#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define ID_ANALIZAR 101
#define ID_LIMPIAR 103
#define ID_LENGUAJE 104
#define ID_ACERCA 105
#define LOG_SIZE 65536
#define MAX_DEPTH 128

typedef enum {
    FIN, IDENTIFICADOR, NUMERO,
    ENTERO, DECIMAL, IMPRIMIR, SI, SINO, MIENTRAS,
    MAS, MENOS, MULTIPLICACION, DIVISION,
    ASIGNACION, IGUAL, DIFERENTE,
    MENOR, MAYOR, MENOR_IGUAL, MAYOR_IGUAL, NEGACION,
    PARENTESIS_A, PARENTESIS_C,
    LLAVE_A, LLAVE_C, PUNTO_COMA, ERROR_TOKEN
} TipoToken;

typedef struct {
    TipoToken tipo;
    char texto[128];
    int linea;
    int columna;
} Token;

typedef struct {
    const char *codigo;
    size_t posicion;
    int linea;
    int columna;
    Token actual;
    int error;
    int profundidad;
    int sentencias;
    char salida[LOG_SIZE];
    size_t longitudSalida;
} Analizador;

static HWND editor, resultados;
static HWND etiquetaCodigo, etiquetaResultado;
static HWND botonAnalizar, botonLimpiar;
static HFONT fuente;


static void registrar(Analizador *a, const char *formato, ...)
{
    va_list argumentos;
    size_t disponible = LOG_SIZE - a->longitudSalida;
    int cantidad;

    if (disponible <= 1) return;

    va_start(argumentos, formato);
    cantidad = vsnprintf(
        a->salida + a->longitudSalida,
        disponible, formato, argumentos
    );
    va_end(argumentos);

    if (cantidad < 0) return;

    if ((size_t)cantidad >= disponible)
        a->longitudSalida = LOG_SIZE - 1;
    else
        a->longitudSalida += (size_t)cantidad;
}

static char ver(Analizador *a)
{
    return a->codigo[a->posicion];
}

static char avanzarCaracter(Analizador *a)
{
    char c = ver(a);

    if (!c) return c;

    a->posicion++;

    if (c == '\n') {
        a->linea++;
        a->columna = 1;
    } else {
        a->columna++;
    }

    return c;
}

static Token crearToken(
    TipoToken tipo, const char *texto, int linea, int columna
)
{
    Token t;
    t.tipo = tipo;
    t.linea = linea;
    t.columna = columna;
    snprintf(t.texto, sizeof(t.texto), "%s", texto);
    return t;
}

static Token leerToken(Analizador *a)
{
    int linea, columna;
    size_t inicio, longitud;
    char texto[128], c;
    TipoToken tipo;

    for (;;) {
        while (isspace((unsigned char)ver(a)))
            avanzarCaracter(a);

        if (ver(a) == '/' && a->codigo[a->posicion + 1] == '/') {
            while (ver(a) && ver(a) != '\n')
                avanzarCaracter(a);
            continue;
        }

        if (ver(a) == '/' && a->codigo[a->posicion + 1] == '*') {
            linea = a->linea;
            columna = a->columna;
            avanzarCaracter(a);
            avanzarCaracter(a);

            while (ver(a) &&
                   !(ver(a) == '*' &&
                     a->codigo[a->posicion + 1] == '/'))
                avanzarCaracter(a);

            if (!ver(a))
                return crearToken(
                    ERROR_TOKEN, "Comentario sin cerrar",
                    linea, columna
                );

            avanzarCaracter(a);
            avanzarCaracter(a);
            continue;
        }

        break;
    }

    linea = a->linea;
    columna = a->columna;
    inicio = a->posicion;
    c = ver(a);

    if (!c)
        return crearToken(FIN, "fin del codigo", linea, columna);

    if (isalpha((unsigned char)c) || c == '_') {
        while (isalnum((unsigned char)ver(a)) || ver(a) == '_')
            avanzarCaracter(a);

        longitud = a->posicion - inicio;

        if (longitud >= sizeof(texto))
            return crearToken(
                ERROR_TOKEN, "Identificador demasiado largo",
                linea, columna
            );

        memcpy(texto, a->codigo + inicio, longitud);
        texto[longitud] = '\0';

        tipo = IDENTIFICADOR;
        if (!strcmp(texto, "int")) tipo = ENTERO;
        else if (!strcmp(texto, "float")) tipo = DECIMAL;
        else if (!strcmp(texto, "print")) tipo = IMPRIMIR;
        else if (!strcmp(texto, "if")) tipo = SI;
        else if (!strcmp(texto, "else")) tipo = SINO;
        else if (!strcmp(texto, "while")) tipo = MIENTRAS;

        return crearToken(tipo, texto, linea, columna);
    }

    if (isdigit((unsigned char)c)) {
        while (isdigit((unsigned char)ver(a)))
            avanzarCaracter(a);

        if (ver(a) == '.') {
            avanzarCaracter(a);

            if (!isdigit((unsigned char)ver(a)))
                return crearToken(
                    ERROR_TOKEN, "Decimal incompleto",
                    linea, columna
                );

            while (isdigit((unsigned char)ver(a)))
                avanzarCaracter(a);
        }

        if (isalpha((unsigned char)ver(a)) || ver(a) == '_')
            return crearToken(
                ERROR_TOKEN, "Numero seguido de letras",
                linea, columna
            );

        longitud = a->posicion - inicio;

        if (longitud >= sizeof(texto))
            return crearToken(
                ERROR_TOKEN, "Numero demasiado largo",
                linea, columna
            );

        memcpy(texto, a->codigo + inicio, longitud);
        texto[longitud] = '\0';
        return crearToken(NUMERO, texto, linea, columna);
    }

    avanzarCaracter(a);
    texto[0] = c;
    texto[1] = '\0';

    switch (c) {
        case '+': tipo = MAS; break;
        case '-': tipo = MENOS; break;
        case '*': tipo = MULTIPLICACION; break;
        case '/': tipo = DIVISION; break;
        case '(': tipo = PARENTESIS_A; break;
        case ')': tipo = PARENTESIS_C; break;
        case '{': tipo = LLAVE_A; break;
        case '}': tipo = LLAVE_C; break;
        case ';': tipo = PUNTO_COMA; break;

        case '=':
        case '!':
        case '<':
        case '>':
            if (ver(a) == '=') {
                avanzarCaracter(a);
                texto[1] = '=';
                texto[2] = '\0';

                if (c == '=') tipo = IGUAL;
                else if (c == '!') tipo = DIFERENTE;
                else if (c == '<') tipo = MENOR_IGUAL;
                else tipo = MAYOR_IGUAL;
            } else {
                if (c == '=') tipo = ASIGNACION;
                else if (c == '!') tipo = NEGACION;
                else if (c == '<') tipo = MENOR;
                else tipo = MAYOR;
            }
            break;

        default:
            tipo = ERROR_TOKEN;
            break;
    }

    return crearToken(tipo, texto, linea, columna);
}

static void siguiente(Analizador *a)
{
    if (a->error) return;

    a->actual = leerToken(a);

    if (a->actual.tipo == ERROR_TOKEN) {
        a->error = 1;
        registrar(
            a,
            "\r\nERROR LEXICO\r\n"
            "Linea %d, columna %d\r\n"
            "Token o detalle: %s\r\n",
            a->actual.linea, a->actual.columna, a->actual.texto
        );
    }
}

static void errorSintactico(Analizador *a, const char *esperado)
{
    if (a->error) return;

    a->error = 1;
    registrar(
        a,
        "\r\nERROR SINTACTICO\r\n"
        "Linea %d, columna %d\r\n"
        "Se esperaba: %s\r\n"
        "Se encontro: \"%s\"\r\n",
        a->actual.linea, a->actual.columna,
        esperado, a->actual.texto
    );
}

static int aceptar(Analizador *a, TipoToken tipo)
{
    if (!a->error && a->actual.tipo == tipo) {
        siguiente(a);
        return 1;
    }

    return 0;
}

static void exigir(
    Analizador *a, TipoToken tipo, const char *descripcion
)
{
    if (a->error) return;

    if (!aceptar(a, tipo))
        errorSintactico(a, descripcion);
}

static int entrar(Analizador *a)
{
    if (a->error) return 0;

    if (a->profundidad >= MAX_DEPTH) {
        errorSintactico(a, "menos niveles de anidamiento");
        return 0;
    }

    a->profundidad++;
    return 1;
}

static void expresion(Analizador *a);
static void sentencia(Analizador *a);

static void primario(Analizador *a)
{
    if (a->error) return;

    if (a->actual.tipo == NUMERO ||
        a->actual.tipo == IDENTIFICADOR) {
        siguiente(a);
    } else if (aceptar(a, PARENTESIS_A)) {
        expresion(a);
        exigir(a, PARENTESIS_C, "')'");
    } else {
        errorSintactico(a, "un numero, una variable o '('");
    }
}

static void unario(Analizador *a)
{
    if (!entrar(a)) return;

    if (a->actual.tipo == MAS ||
        a->actual.tipo == MENOS ||
        a->actual.tipo == NEGACION) {
        siguiente(a);
        unario(a);
    } else {
        primario(a);
    }

    a->profundidad--;
}

static void producto(Analizador *a)
{
    unario(a);

    while (!a->error &&
           (a->actual.tipo == MULTIPLICACION ||
            a->actual.tipo == DIVISION)) {
        siguiente(a);
        unario(a);
    }
}

static void suma(Analizador *a)
{
    producto(a);

    while (!a->error &&
           (a->actual.tipo == MAS || a->actual.tipo == MENOS)) {
        siguiente(a);
        producto(a);
    }
}

static void comparacion(Analizador *a)
{
    suma(a);

    while (!a->error &&
           (a->actual.tipo == MENOR ||
            a->actual.tipo == MAYOR ||
            a->actual.tipo == MENOR_IGUAL ||
            a->actual.tipo == MAYOR_IGUAL)) {
        siguiente(a);
        suma(a);
    }
}

static void expresion(Analizador *a)
{
    if (!entrar(a)) return;

    comparacion(a);

    while (!a->error &&
           (a->actual.tipo == IGUAL ||
            a->actual.tipo == DIFERENTE)) {
        siguiente(a);
        comparacion(a);
    }

    a->profundidad--;
}

static void bloque(Analizador *a)
{
    exigir(a, LLAVE_A, "'{' para iniciar el bloque");

    while (!a->error &&
           a->actual.tipo != LLAVE_C &&
           a->actual.tipo != FIN)
        sentencia(a);

    exigir(a, LLAVE_C, "'}' para cerrar el bloque");
}

static void sentencia(Analizador *a)
{
    int linea;
    const char *nombre = "";

    if (!entrar(a)) return;

    linea = a->actual.linea;

    switch (a->actual.tipo) {
        case ENTERO:
        case DECIMAL:
            nombre = "Declaracion";
            siguiente(a);
            exigir(a, IDENTIFICADOR, "un nombre de variable");

            if (aceptar(a, ASIGNACION))
                expresion(a);

            exigir(a, PUNTO_COMA, "';' al finalizar la declaracion");
            break;

        case IDENTIFICADOR:
            nombre = "Asignacion";
            siguiente(a);
            exigir(a, ASIGNACION, "'='");
            expresion(a);
            exigir(a, PUNTO_COMA, "';' al finalizar la asignacion");
            break;

        case IMPRIMIR:
            nombre = "Sentencia print";
            siguiente(a);
            exigir(a, PARENTESIS_A, "'(' despues de print");
            expresion(a);
            exigir(a, PARENTESIS_C, "')'");
            exigir(a, PUNTO_COMA, "';' despues de print");
            break;

        case SI:
            nombre = "Condicional if";
            siguiente(a);
            exigir(a, PARENTESIS_A, "'(' despues de if");
            expresion(a);
            exigir(a, PARENTESIS_C, "')' despues de la condicion");
            bloque(a);

            if (aceptar(a, SINO))
                bloque(a);
            break;

        case MIENTRAS:
            nombre = "Ciclo while";
            siguiente(a);
            exigir(a, PARENTESIS_A, "'(' despues de while");
            expresion(a);
            exigir(a, PARENTESIS_C, "')' despues de la condicion");
            bloque(a);
            break;

        default:
            errorSintactico(
                a, "una declaracion, asignacion, print, if o while"
            );
            break;
    }

    if (!a->error) {
        a->sentencias++;
        registrar(a, "[OK] Linea %d: %s\r\n", linea, nombre);
    }

    a->profundidad--;
}

static void analizarCodigo(void)
{
    int longitud = GetWindowTextLengthA(editor);
    char *codigo = (char *)malloc((size_t)longitud + 1);
    Analizador *a = (Analizador *)calloc(1, sizeof(Analizador));

    if (!codigo || !a) {
        free(codigo);
        free(a);
        MessageBoxA(
            NULL, "Memoria insuficiente.", "Error",
            MB_OK | MB_ICONERROR
        );
        return;
    }

    GetWindowTextA(editor, codigo, longitud + 1);
    a->codigo = codigo;
    a->linea = 1;
    a->columna = 1;

    registrar(
        a,
        "ANALIZADOR SINTACTICO\r\n"
        "Metodo: descenso recursivo\r\n"
        "----------------------------------------\r\n"
    );

    siguiente(a);

    while (!a->error && a->actual.tipo != FIN)
        sentencia(a);

    if (a->error) {
        registrar(
            a,
            "\r\nRESULTADO: CODIGO RECHAZADO\r\n"
            "Corrige el error y vuelve a analizar.\r\n"
        );
    } else {
        registrar(
            a,
            "\r\nRESULTADO: SINTAXIS CORRECTA\r\n"
            "Sentencias reconocidas: %d\r\n",
            a->sentencias
        );

        if (!a->sentencias)
            registrar(a, "El programa esta vacio.\r\n");
    }

    SetWindowTextA(resultados, a->salida);
    free(a);
    free(codigo);
}

static void ajustarVentana(HWND ventana)
{
    RECT r;
    int ancho, alto, altoEditor, yResultado;

    GetClientRect(ventana, &r);
    ancho = r.right;
    alto = r.bottom;

    if (ancho < 100 || alto < 200) return;

    altoEditor = (alto - 140) * 3 / 5;
    yResultado = altoEditor + 90;

    MoveWindow(etiquetaCodigo, 15, 12, ancho - 30, 22, TRUE);
    MoveWindow(editor, 15, 38, ancho - 30, altoEditor, TRUE);

    MoveWindow(botonAnalizar, 15, altoEditor + 48, 130, 30, TRUE);
    MoveWindow(botonLimpiar, 155, altoEditor + 48, 130, 30, TRUE);

    MoveWindow(
        etiquetaResultado, 15, yResultado, ancho - 30, 22, TRUE
    );
    MoveWindow(
        resultados, 15, yResultado + 25,
        ancho - 30, alto - yResultado - 40, TRUE
    );
}

static LRESULT CALLBACK procedimientoVentana(
    HWND ventana, UINT mensaje, WPARAM wParam, LPARAM lParam
)
{
    switch (mensaje) {
        case WM_CREATE:
        {
            HINSTANCE instancia =
                ((LPCREATESTRUCTA)lParam)->hInstance;
            HWND controles[6];
            int i;

            etiquetaCodigo = CreateWindowExA(
                0, "STATIC", "Escribe el codigo:",
                WS_CHILD | WS_VISIBLE,
                0, 0, 0, 0, ventana, NULL, instancia, NULL
            );

            editor = CreateWindowExA(
                WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                ES_MULTILINE | ES_WANTRETURN |
                ES_AUTOVSCROLL | ES_AUTOHSCROLL |
                WS_VSCROLL | WS_HSCROLL,
                0, 0, 0, 0, ventana, NULL, instancia, NULL
            );

            botonAnalizar = CreateWindowExA(
                0, "BUTTON", "Analizar",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                BS_DEFPUSHBUTTON,
                0, 0, 0, 0, ventana,
                (HMENU)(INT_PTR)ID_ANALIZAR, instancia, NULL
            );

            botonLimpiar = CreateWindowExA(
                0, "BUTTON", "Limpiar",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP,
                0, 0, 0, 0, ventana,
                (HMENU)(INT_PTR)ID_LIMPIAR, instancia, NULL
            );

            etiquetaResultado = CreateWindowExA(
                0, "STATIC", "Resultado:",
                WS_CHILD | WS_VISIBLE,
                0, 0, 0, 0, ventana, NULL, instancia, NULL
            );

            resultados = CreateWindowExA(
                WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP |
                ES_MULTILINE | ES_READONLY |
                ES_AUTOVSCROLL | ES_AUTOHSCROLL |
                WS_VSCROLL | WS_HSCROLL,
                0, 0, 0, 0, ventana, NULL, instancia, NULL
            );

            fuente = CreateFontA(
                -17, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                FIXED_PITCH | FF_MODERN, "Consolas"
            );

            controles[0] = editor;
            controles[1] = resultados;
            controles[2] = etiquetaCodigo;
            controles[3] = etiquetaResultado;
            controles[4] = botonAnalizar;
            controles[5] = botonLimpiar;

            for (i = 0; i < 6; i++)
                SendMessageA(
                    controles[i], WM_SETFONT, (WPARAM)fuente, TRUE
                );

            SendMessageA(editor, EM_SETLIMITTEXT, 1024 * 1024, 0);
            SendMessageA(resultados, EM_SETLIMITTEXT, LOG_SIZE, 0);

            ajustarVentana(ventana);
            return 0;
        }

        case WM_SIZE:
            ajustarVentana(ventana);
            return 0;

        case WM_GETMINMAXINFO:
        {
            MINMAXINFO *info = (MINMAXINFO *)lParam;
            info->ptMinTrackSize.x = 540;
            info->ptMinTrackSize.y = 460;
            return 0;
        }

        case WM_COMMAND:
            if (HIWORD(wParam) == BN_CLICKED) {
                switch (LOWORD(wParam)) {
                    case ID_ANALIZAR:
                        analizarCodigo();
                        return 0;

                    case ID_LIMPIAR:
                        SetWindowTextA(editor, "");
                        SetWindowTextA(resultados, "");
                        SetFocus(editor);
                        return 0;

                    case ID_LENGUAJE:
                        MessageBoxA(ventana,
                            "MiniLenguaje (sensible a mayusculas)\r\n\r\n"
                            "Declaraciones: int edad = 20; float precio = 12.5;\r\n"
                            "Asignacion: edad = edad + 1;\r\n"
                            "Salida: print(edad);\r\n"
                            "Condicional: if (edad >= 18) { print(edad); } else { print(0); }\r\n"
                            "Ciclo: while (edad < 30) { edad = edad + 1; }\r\n\r\n"
                            "Operadores: + - * / < > <= >= == != !\r\n"
                            "Comentarios: // una linea y /* varias lineas */\r\n"
                            "Las declaraciones, asignaciones y print terminan en ;\r\n"
                            "Los bloques de if, else y while requieren llaves.\r\n\r\n"
                            "Valida sintaxis; no ejecuta el programa ni verifica tipos.\r\n"
                            "Consulta docs/LENGUAJE.md para la gramatica completa.",
                            "Ayuda del lenguaje", MB_OK | MB_ICONINFORMATION);
                        return 0;

                    case ID_ACERCA:
                        MessageBoxA(ventana,
                            "Analizador sintactico de MiniLenguaje\r\n"
                            "Implementado en C11 con interfaz Win32.\r\n\r\n"
                            "1. El analizador lexico reconoce tokens.\r\n"
                            "2. El descenso recursivo comprueba la gramatica.\r\n"
                            "3. Muestra las sentencias reconocidas o el primer error,\r\n"
                            "   con linea, columna y elemento esperado.\r\n\r\n"
                            "El boton Analizar no ejecuta las instrucciones print o while.",
                            "Funcionamiento", MB_OK | MB_ICONINFORMATION);
                        return 0;
                }
            }
            break;

        case WM_DESTROY:
            if (fuente) DeleteObject(fuente);
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcA(ventana, mensaje, wParam, lParam);
}

int WINAPI WinMain(
    HINSTANCE instancia,
    HINSTANCE instanciaAnterior,
    LPSTR argumentos,
    int mostrar
)
{
    WNDCLASSA clase;
    HWND ventana;
    HMENU menu, ayuda;
    MSG mensaje;
    int estado;

    (void)instanciaAnterior;
    (void)argumentos;

    memset(&clase, 0, sizeof(clase));
    clase.lpfnWndProc = procedimientoVentana;
    clase.hInstance = instancia;
    clase.lpszClassName = "AnalizadorSintactico";
    clase.hCursor = LoadCursor(NULL, IDC_ARROW);
    clase.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    clase.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);

    if (!RegisterClassA(&clase)) return 1;

    ventana = CreateWindowExA(
        0, clase.lpszClassName,
        "Analizador Sintactico - C / Win32",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 900, 720,
        NULL, NULL, instancia, NULL
    );

    if (!ventana) return 1;

    menu = CreateMenu();
    ayuda = CreatePopupMenu();
    AppendMenuA(ayuda, MF_STRING, ID_LENGUAJE, "Lenguaje y ejemplos");
    AppendMenuA(ayuda, MF_STRING, ID_ACERCA, "Funcionamiento del analizador");
    AppendMenuA(menu, MF_POPUP, (UINT_PTR)ayuda, "Ayuda");
    SetMenu(ventana, menu);

    ShowWindow(ventana, mostrar);
    UpdateWindow(ventana);

    while ((estado = GetMessageA(&mensaje, NULL, 0, 0)) > 0) {
        if (!IsDialogMessageA(ventana, &mensaje)) {
            TranslateMessage(&mensaje);
            DispatchMessageA(&mensaje);
        }
    }

    return estado == -1 ? 1 : (int)mensaje.wParam;
}
