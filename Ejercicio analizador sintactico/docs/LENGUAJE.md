# Lenguaje del proyecto

Hay dos lenguajes distintos: **C11** es el lenguaje usado para programar la aplicación; **MiniLenguaje** es el lenguaje que el usuario escribe para que la aplicación compruebe su sintaxis. MiniLenguaje toma elementos de C, pero no acepta programas completos de C, funciones `main`, directivas `#include` ni `printf`.

## Reglas léxicas

- Palabras reservadas: `int`, `float`, `print`, `if`, `else`, `while`, en minúsculas.
- Identificadores: empiezan por una letra ASCII o `_` y continúan con letras, dígitos o `_`. Máximo: 127 caracteres. Ejemplos: `edad`, `_total`, `valor2`. Las palabras reservadas no pueden ser nombres de variables.
- Números: uno o más dígitos, opcionalmente seguidos de punto y uno o más dígitos. Ejemplos: `0`, `123`, `12.5`. El signo se reconoce como operador unario. No se admiten `.5`, `5.`, exponentes o más de 127 caracteres por número.
- Se ignoran espacios, tabulaciones, saltos de línea y comentarios `//` o `/* ... */`. Los comentarios de bloque no se anidan.
- Se distingue entre mayúsculas y minúsculas: `contador` y `Contador` son identificadores distintos; `INT` no es la palabra reservada `int`.

## Instrucciones

```text
int contador;
float precio = 12.5;
contador = 2 + 3 * 4;
print(contador);
if (contador >= 10) {
    print(precio);
} else {
    print(0);
}
while (contador > 0) {
    contador = contador - 1;
}
```

Cada declaración admite una variable y una inicialización opcional. Declaraciones, asignaciones y `print` terminan en `;`. `print` requiere exactamente una expresión. `if` y `while` requieren una condición entre paréntesis y un bloque entre llaves, incluso para una sola sentencia. `else` es opcional y requiere su propio bloque. Se permiten bloques vacíos y estructuras anidadas; `else if` se expresa con un `if` dentro del bloque de `else`.

## Gramática EBNF

En esta notación, `{ regla }` significa repetición cero o más veces, `[ regla ]` significa opcional y los símbolos entre comillas son texto literal. `FIN` es el final de la entrada, no una palabra que se deba escribir.

```text
programa     = { sentencia }, FIN ;
sentencia    = declaracion | asignacion | impresion | condicional | ciclo ;
declaracion  = ("int" | "float"), identificador, [ "=", expresion ], ";" ;
asignacion   = identificador, "=", expresion, ";" ;
impresion    = "print", "(", expresion, ")", ";" ;
condicional = "if", "(", expresion, ")", bloque, [ "else", bloque ] ;
ciclo        = "while", "(", expresion, ")", bloque ;
bloque       = "{", { sentencia }, "}" ;
expresion    = comparacion, { ("==" | "!="), comparacion } ;
comparacion  = suma, { ("<" | ">" | "<=" | ">="), suma } ;
suma         = producto, { ("+" | "-"), producto } ;
producto     = unario, { ("*" | "/"), unario } ;
unario       = ("+" | "-" | "!"), unario | primario ;
primario     = numero | identificador | "(", expresion, ")" ;
identificador = (letra | "_"), { letra | digito | "_" } ;
numero       = digito, { digito }, [ ".", digito, { digito } ] ;
letra        = "A" ... "Z" | "a" ... "z" ;
digito       = "0" ... "9" ;
```

Los identificadores excluyen las palabras reservadas. Los espacios y comentarios se eliminan durante el análisis léxico antes de aplicar las reglas sintácticas.

## Precedencia, de mayor a menor

| Nivel | Operadores o elementos |
| --- | --- |
| 1 | Paréntesis, números, identificadores |
| 2 | Unarios `+`, `-`, `!` |
| 3 | `*`, `/` |
| 4 | `+`, `-` binarios |
| 5 | `<`, `>`, `<=`, `>=` |
| 6 | `==`, `!=` |

Por ejemplo, `2 + 3 * 4` se reconoce como una suma cuyo segundo operando es un producto. Las repeticiones permiten cadenas como `a < b < c`; se aceptan sintácticamente, pero no se evalúa su significado.

## Alcance y límites

- Se valida la estructura, sin comprobar declaración previa de variables, tipos, valores, división por cero o terminación de ciclos. `x = 1 / 0;` es sintácticamente válido.
- No hay cadenas, arreglos, funciones, `for`, `return`, `&&`, `||`, `++`, `%` ni asignaciones dentro de expresiones.
- Una entrada vacía o con solo comentarios es válida; se informa que el programa está vacío.
- Se detiene en el primer error léxico o sintáctico; tras corregirlo se analiza de nuevo.
- El editor permite hasta 1 MiB de texto. El registro tiene 65 536 bytes y puede truncarse con programas grandes. Cada identificador o número admite hasta 127 caracteres.
- El límite de profundidad es 128 entradas recursivas internas compartidas entre sentencias, expresiones y unarios; no equivale a 128 pares de llaves. Evita un anidamiento excesivo.
- Las líneas y columnas empiezan en 1. Un tabulador cuenta como un carácter. El error señala el token donde se detecta el problema: un `;` ausente puede detectarse al comenzar la línea siguiente.
- El contador incluye sentencias dentro de bloques y sus estructuras externas. Los mensajes `[OK]` de una sentencia interna pueden aparecer antes que el de su `if` o `while`.
