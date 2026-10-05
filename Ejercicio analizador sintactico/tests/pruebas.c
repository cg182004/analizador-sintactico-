/* Reutiliza exactamente el motor de la aplicacion, sin abrir ventanas. */
#include "../main.c"

static const char *codigoEjemplo =
    "int contador = 0;\r\n"
    "float precio = 12.5;\r\n"
    "\r\n"
    "while (contador < 5) {\r\n"
    "    contador = contador + 1;\r\n"
    "    print(contador);\r\n"
    "}\r\n"
    "\r\n"
    "if (precio >= 10) {\r\n"
    "    print(precio * 2);\r\n"
    "} else {\r\n"
    "    print(0);\r\n"
    "}\r\n";


static int fallos;
static int pruebas;

static void comprobar(const char *codigo, const char *detalle,
                      int sentencias, int linea, int columna)
{
    Analizador *a = calloc(1, sizeof(*a));
    int correcto;
    if (!a) exit(2);
    a->codigo = codigo;
    a->linea = a->columna = 1;
    siguiente(a);
    while (!a->error && a->actual.tipo != FIN) sentencia(a);
    correcto = detalle ? a->error && strstr(a->salida, detalle) != NULL
                       : !a->error && a->sentencias == sentencias;
    if (linea) correcto = correcto && a->actual.linea == linea;
    if (columna) correcto = correcto && a->actual.columna == columna;
    correcto = correcto && a->profundidad == 0;
    pruebas++;
    if (!correcto) {
        fallos++;
        printf("FALLO %d: %s\n%s\n", pruebas, codigo, a->salida);
    }
    free(a);
}

int main(void)
{
    char profundo[512];
    char largo[160];
    comprobar(codigoEjemplo, NULL, 8, 0, 0);
    comprobar("int x;", NULL, 1, 0, 0);
    comprobar("float x = 2.5; print(x);", NULL, 2, 0, 0);
    comprobar("x = -(2 + 3) * 4;", NULL, 1, 0, 0);
    comprobar("if (x != 0) { print(x); } else { print(0); }", NULL, 3, 0, 0);
    comprobar("while (x < 3) { x = x + 1; }", NULL, 2, 0, 0);
    comprobar("if (1) { while (!x) { print(+1); } }", NULL, 3, 0, 0);
    comprobar("if (1) {} else {}", NULL, 1, 0, 0);
    comprobar("/* comentario */ int _x2 = 1; // fin", NULL, 1, 0, 0);
    comprobar("", NULL, 0, 0, 0);
    comprobar("// comentario", NULL, 0, 0, 0);
    comprobar("x = 1 / 0;", NULL, 1, 0, 0);
    comprobar("int x = 1", "';'", 0, 1, 10);
    comprobar("print(1;", "')'", 0, 1, 8);
    comprobar("int x = 0\r\nprint(x);", "';'", 0, 2, 1);
    comprobar("if (1) { print(1);", "'}'", 0, 0, 0);
    comprobar("int = 2;", "un nombre de variable", 0, 0, 0);
    comprobar("x = 2 + ;", "un numero", 0, 0, 0);
    comprobar("else { print(0); }", "ERROR SINTACTICO", 0, 0, 0);
    comprobar("int x = 2.;", "Decimal incompleto", 0, 0, 0);
    comprobar("int x = 12abc;", "Numero seguido de letras", 0, 0, 0);
    comprobar("/* comentario", "Comentario sin cerrar", 0, 1, 1);
    comprobar("x = @;", "ERROR LEXICO", 0, 1, 5);
    comprobar("print();", "ERROR SINTACTICO", 0, 0, 0);
    comprobar("if (1) print(1);", "'{'", 0, 0, 0);
    comprobar("int x; }", "ERROR SINTACTICO", 0, 0, 0);
    strcpy(profundo, "x = ");
    memset(profundo + 4, '-', 200);
    strcpy(profundo + 204, "1;");
    comprobar(profundo, "menos niveles de anidamiento", 0, 0, 0);
    strcpy(profundo, "print(");
    memset(profundo + 6, '(', 200);
    strcpy(profundo + 206, "1;");
    comprobar(profundo, "menos niveles de anidamiento", 0, 0, 0);
    memset(largo, 'a', 128);
    largo[128] = '\0';
    comprobar(largo, "Identificador demasiado largo", 0, 0, 0);
    memset(largo, '1', 128);
    comprobar(largo, "Numero demasiado largo", 0, 0, 0);
    printf("%d pruebas, %d fallos.\n", pruebas, fallos);
    return fallos ? 1 : 0;
}
