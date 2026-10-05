# Guía para la exposición (5 a 7 minutos)

## 1. Objetivo (30 segundos)

“Desarrollé una aplicación gráfica en C con Win32 que comprueba si un programa de MiniLenguaje cumple una gramática. La aplicación indica si la sintaxis es correcta o muestra el primer error con su ubicación.”

Aclara la diferencia entre C, usado para desarrollar la aplicación, y MiniLenguaje, usado como entrada.

## 2. Conceptos de análisis sintáctico (1 minuto)

En un compilador, el análisis léxico transforma caracteres en tokens; el análisis sintáctico verifica su organización según una gramática; el análisis semántico revisa significado, tipos y nombres. Este proyecto implementa las primeras dos etapas y no genera código ni ejecuta instrucciones.

Ejemplo: `int edad = 20;` produce los tokens ENTERO, IDENTIFICADOR, ASIGNACION, NUMERO y PUNTO_COMA. El analizador comprueba que coincidan con la regla de declaración.

## 3. Técnica utilizada (1 a 2 minutos)

El descenso recursivo representa reglas de la gramática mediante funciones. `sentencia` elige la regla observando el token actual. `exigir` comprueba un token obligatorio y comunica el error si no aparece. `bloque` reconoce las llaves y llama a `sentencia` para su contenido.

Las expresiones se dividen en niveles: igualdad, comparación, suma, producto, unario y primario. Esto representa la precedencia: en `2 + 3 * 4`, el producto pertenece al segundo operando de la suma. La recursión permite paréntesis y estructuras anidadas; un límite protege ante entradas demasiado profundas.

Recorrido del programa: editor → `leerToken` → `sentencia` y reglas de expresión → resultado gráfico. No se construye un árbol en memoria: las funciones reconocen directamente la estructura.

## 4. Demostración (2 minutos)

1. Abre el ejecutable, pega `ejemplos/01_correcto.txt` y pulsa **Analizar**. Explica el resultado correcto y las 8 sentencias, incluyendo las internas.
2. Pega `ejemplos/02_falta_punto_coma.txt`. Analiza y explica que la falta de `;` se detecta al encontrar `print` en la siguiente línea. Corrige y vuelve a analizar.
3. Escribe `print(2 + );`: se rechaza porque después de `+` falta un operando.
4. Escribe `x = @;`: muestra un error léxico, porque `@` no es un token del lenguaje.
5. Escribe `x = 1 / 0;`: se acepta sintácticamente. Explica que comprobar valores y variables requiere otras etapas.

## 5. Alcance y cierre (30 segundos)

Muestra la gramática y los casos de prueba de la documentación. Resume las construcciones soportadas: variables, expresiones, print, if/else y while. Menciona que se informa un error a la vez y que el lenguaje no es C completo.

## Preguntas posibles

- **¿Por qué descenso recursivo?** Permite expresar esta gramática pequeña con funciones claras y controlar los mensajes de error.
- **¿Qué significa sintaxis correcta?** Que la secuencia de tokens cumple las reglas; no garantiza que el programa tenga sentido o pueda ejecutarse sin problemas.
- **¿Qué diferencia hay entre error léxico y sintáctico?** Un símbolo como `@` no pertenece al vocabulario; una instrucción sin `;` utiliza tokens conocidos pero incumple la estructura.
- **¿El botón Analizar ejecuta while?** No. Reconoce su condición y su bloque una sola vez como texto.
- **¿Cómo se ampliaría?** Añadiendo tokens y reglas, y después tabla de símbolos, árbol sintáctico o un intérprete si se requieren esas etapas.

## Archivos que se entregan

Código fuente `main.c`, ejecutable `dist/AnalizadorSintactico.exe`, script de compilación, documentación, ejemplos y pruebas. No se requiere instalar el compilador para abrir el ejecutable. La publicación en GitHub queda excluida de esta entrega.
