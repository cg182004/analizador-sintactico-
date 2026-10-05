# Pruebas de la aplicación

Abre `dist/AnalizadorSintactico.exe`, copia una entrada en el editor y pulsa **Analizar**. Usa **Limpiar** entre pruebas. También puedes copiar los archivos de `ejemplos/` para probar entradas válidas e inválidas.

| Entrada | Resultado esperado |
| --- | --- |
| `int x;` | Sintaxis correcta, 1 sentencia |
| `float x = 2.5; print(x);` | Sintaxis correcta, 2 sentencias |
| `x = -(2 + 3) * 4;` | Sintaxis correcta |
| `if (x != 0) { print(x); } else { print(0); }` | Sintaxis correcta, 3 sentencias |
| `while (x < 3) { x = x + 1; }` | Sintaxis correcta, 2 sentencias |
| `int x = 1` | Error sintáctico: falta `;`, línea 1, columna 10 |
| `print(1;` | Error sintáctico: falta `)`, línea 1, columna 8 |
| `if (1) { print(1);` | Error sintáctico: falta `}` |
| `int = 2;` | Error sintáctico: falta nombre de variable |
| `x = 2 + ;` | Error sintáctico: falta operando |
| `else { print(0); }` | Error sintáctico: `else` sin `if` |
| `int x = 2.;` | Error léxico: decimal incompleto |
| `int x = 12abc;` | Error léxico: número seguido de letras |
| `/* comentario` | Error léxico: comentario sin cerrar |
| `x = @;` | Error léxico: carácter no reconocido |
| Entrada vacía o `// comentario` | Sintaxis correcta, programa vacío |
| `x = 1 / 0;` | Sintaxis correcta; no se evalúa ni verifica semántica |

## Verificación automática

`probar.bat` compila y ejecuta `tests/pruebas.c` con el mismo analizador de `main.c`. Comprueba aceptación, rechazo, clasificación de errores, posición de dos errores, conteo de sentencias y protección ante anidamiento excesivo. Devuelve un código distinto de cero si falla alguna comprobación. Es una herramienta de desarrollo, no la interfaz del proyecto.

## Comprobación visual para la presentación

1. Abrir el ejecutable y verificar que aparece la ventana con el editor vacío.
2. Pegar `ejemplos/01_correcto.txt` y pulsar **Analizar**: el ejemplo debe aceptarse y reconocer 8 sentencias.
3. Pegar `ejemplos/02_falta_punto_coma.txt`: falta un `;`. Analizar, corregirlo y volver a analizar.
4. Revisar las opciones de **Ayuda**.
5. Pulsar **Limpiar**, pegar otro ejemplo y redimensionar la ventana.

La prueba automática valida el motor; esta lista permite comprobar la interacción gráfica en el equipo de presentación.
