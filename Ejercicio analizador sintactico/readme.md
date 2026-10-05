# Analizador sintáctico gráfico

Aplicación desarrollada en C con Win32 API para comprobar la
estructura sintáctica de un pequeño lenguaje de programación.

## Requisitos

- Windows.
- GCC de MinGW-w64 disponible en el PATH para compilar.
- VS Code u otro editor para modificar el código.

El ejecutable generado funciona como aplicación gráfica.

## Compilación y ejecución

Ejecutar compilar.bat.

También se puede compilar desde la terminal:

```bash
gcc main.c -o AnalizadorSintactico.exe -std=c11 -Wall -Wextra -O2 -mwindows -static -luser32 -lgdi32
```

Al utilizar compilar.bat, el ejecutable se crea en:

dist/AnalizadorSintactico.exe

La terminal se utiliza para compilar. La aplicación final presenta
su editor y sus resultados en una ventana, sin consola.

## Uso

1. Escribir o pegar el código en el editor.
2. Presionar Analizar.
3. Consultar el resultado.
4. Si aparece un error, corregir la línea indicada.
5. Volver a analizar.

Cargar ejemplo introduce un programa válido.
Limpiar elimina el código y los resultados.

Los archivos de ejemplos se prueban copiando su contenido en el editor.

## Lenguaje utilizado para las pruebas

El lenguaje de entrada se llama MiniLenguaje.

El analizador está programado en C, pero el código que se introduce
debe respetar las reglas de MiniLenguaje. No acepta todo el lenguaje C.

### Identificadores

Empiezan con una letra ASCII o un guion bajo.
Después pueden contener letras, números y guiones bajos.

Ejemplos:

```c
contador
precio
total_ventas
_variable
```

Las palabras reservadas no pueden utilizarse como nombres de variables.

### Palabras reservadas

int, float, print, if, else, while.

El lenguaje distingue entre mayúsculas y minúsculas.

### Números

Acepta números enteros y decimales con punto.

```c
10
25.50
0.75
```

Los valores negativos se forman utilizando el operador unario menos.

```c
int temperatura = -5;
```

No acepta notación científica ni números como .5 o 5.

### Declaraciones

```c
int cantidad;
int contador = 0;
float precio = 12.5;
```

Cada declaración permite una sola variable y termina con punto y coma.

### Asignaciones

```c
cantidad = 10;
contador = contador + 1;
```

### Expresiones

Operadores aritméticos:

+  -  *  /

Operadores relacionales:

<  >  <=  >=

Operadores de igualdad:

==  !=

Operadores unarios:

+  -  !

Ejemplo:

```c
cantidad = (5 + 3) * 2;
```

Precedencia, de mayor a menor:

1. Paréntesis.
2. Operadores unarios.
3. Multiplicación y división.
4. Suma y resta.
5. Comparaciones.
6. Igualdad y desigualdad.

Los operadores binarios de un mismo nivel se reconocen de izquierda
a derecha.

### Impresión

```c
print(cantidad);
print(5 + 3);
```

El analizador reconoce la estructura de print, pero no ejecuta
la impresión.

### Condicionales

```c
if (cantidad > 5) {
    print(cantidad);
} else {
    print(0);
}
```

else es opcional. Las llaves son obligatorias.

### Ciclos

```c
while (contador < 10) {
    contador = contador + 1;
}
```

Las llaves son obligatorias.

### Comentarios

```c
// Comentario de una línea

/*
Comentario de varias líneas
*/
```

Los comentarios de bloque no pueden anidarse.

## Gramática

Notación EBNF:
- { X } significa cero o más repeticiones.
- [ X ] significa una parte opcional.
- | indica alternativas.
- Los símbolos entre comillas son elementos del código de entrada.

```text
programa = { sentencia } ;

sentencia = declaracion, ";"
          | asignacion, ";"
          | impresion, ";"
          | condicional
          | ciclo ;

declaracion = ("int" | "float"), ID, [ "=", expresion ] ;

asignacion = ID, "=", expresion ;

impresion = "print", "(", expresion, ")" ;

condicional = "if", "(", expresion, ")", bloque,
              [ "else", bloque ] ;

ciclo = "while", "(", expresion, ")", bloque ;

bloque = "{", { sentencia }, "}" ;

expresion = comparacion, { ("==" | "!="), comparacion } ;

comparacion = suma, { ("<" | ">" | "<=" | ">="), suma } ;

suma = producto, { ("+" | "-"), producto } ;

producto = unario, { ("*" | "/"), unario } ;

unario = ("+" | "-" | "!"), unario
       | primario ;

primario = NUMERO
         | ID
         | "(", expresion, ")" ;
```

ID representa un identificador y NUMERO un literal numérico.

El programa vacío y los bloques vacíos son válidos.

## Funcionamiento interno

### 1. Análisis léxico

leerToken recorre los caracteres y reconoce palabras reservadas,
identificadores, números, operadores y delimitadores.

También ignora espacios y comentarios, y registra líneas y columnas.

### 2. Análisis sintáctico

El proyecto utiliza descenso recursivo.

Cada función reconoce una parte de la gramática:

- sentencia: declaraciones, asignaciones, print, if y while.
- bloque: sentencias entre llaves.
- expresion: igualdad y desigualdad.
- comparacion: operadores relacionales.
- suma: suma y resta.
- producto: multiplicación y división.
- unario: signos y negación.
- primario: números, identificadores y paréntesis.

exigir comprueba que aparezca el token necesario.
Si falta, se informa qué se esperaba y qué se encontró.

### 3. Resultado

Si se consume todo el código sin errores, se muestra:

RESULTADO: SINTAXIS CORRECTA

Si aparece un error, el análisis se detiene en el primero y muestra
su línea, columna y descripción.

Las sentencias interiores se registran antes que el bloque que
las contiene, porque se validan antes de completar ese bloque.

## Pruebas propuestas

Probar cada fragmento por separado.

| Código | Resultado esperado |
|---|---|
| int x = 5; | Sintaxis correcta |
| float p = 2.5; | Sintaxis correcta |
| x = (5 + 3) * 2; | Sintaxis correcta |
| print(x); | Sintaxis correcta |
| if (x > 0) { print(x); } | Sintaxis correcta |
| while (x < 5) { x = x + 1; } | Sintaxis correcta |
| int x = 5 | Error: falta ; |
| int = 5; | Error: falta identificador |
| x = ; | Error: falta expresión |
| print(x; | Error: falta ) |
| if (x > 0) { print(x); | Error: falta } |
| x = 5 + * 2; | Error: falta operando |
| int x = 2. ; | Error léxico: decimal incompleto |
| @ | Error léxico: símbolo no permitido |

Esta tabla describe resultados esperados; las pruebas deben
ejecutarse en Windows después de compilar.

## Alcance y limitaciones

- Comprueba sintaxis, sin ejecutar el código.
- No verifica si una variable fue declarada.
- No comprueba compatibilidad de tipos.
- No detecta división entre cero.
- No acepta cadenas, funciones, arreglos, for, ++, && ni ||.
- Informa el primer error encontrado.
- Limita el anidamiento interno para evitar recursión excesiva.

Por ejemplo, print(variableNoDeclarada); es sintácticamente válido,
aunque un analizador semántico podría rechazarlo.

## Guion para la presentación

1. El análisis sintáctico verifica que los tokens estén organizados
   según la gramática del lenguaje.
2. El análisis léxico identifica los tokens.
3. El parser utiliza esos tokens para reconocer declaraciones,
   expresiones, condicionales y ciclos.
4. El método utilizado es descenso recursivo, con funciones que
   representan reglas de la gramática.
5. Mostrar el programa del botón Cargar ejemplo y analizarlo.
6. Eliminar un punto y coma y mostrar el error.
7. Corregirlo y volver a analizar.
8. Explicar que la sintaxis correcta no garantiza que el programa
   tenga significado correcto: eso corresponde al análisis semántico.

## Entrega en GitHub

Después de compilar, incluir:

- main.c
- compilar.bat
- README.md
- .gitignore
- ejemplos/correcto.txt
- ejemplos/incorrecto.txt
- dist/AnalizadorSintactico.exe

El archivo .gitignore permite incluir el ejecutable en la entrega.