
# Actividad 2.6: Problema práctico en OpenMP

## Integrantes

- Avalos Oliva Karen Lizeth
- Oliver Daniel Chavez Torres
- Ramírez Arvizu Fernando David

## Descripción de la solución

En esta práctica se desarrolló un programa de búsqueda exhaustiva utilizando C++ y OpenMP.

El programa permite ingresar una clave de prueba y buscarla de dos maneras: mediante una búsqueda secuencial y una búsqueda paralela.

En la versión secuencial, las combinaciones se revisan una por una hasta encontrar la clave. En la versión paralela, el espacio de búsqueda se divide entre varios hilos para que cada uno revise un rango diferente al mismo tiempo.

Cuando uno de los hilos encuentra la clave, se registra el hilo ganador y se comunica el resultado a los demás hilos para detener la búsqueda.

Finalmente, se comparan los tiempos de ambas versiones y se calcula el speedup para analizar el rendimiento obtenido.

## Caracteres utilizados

El espacio de búsqueda utiliza 36 caracteres:

- Letras mayúsculas de la A a la Z.
- Números del 0 al 9.

Conjunto utilizado:

`ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789`

## Algoritmos implementados

- Validación de la clave ingresada por el usuario.
- Cálculo del número total de combinaciones.
- Conversión de una posición numérica a una combinación de caracteres.
- Búsqueda exhaustiva secuencial.
- Búsqueda exhaustiva paralela.
- Distribución equilibrada del espacio de búsqueda entre los hilos.
- Identificación del hilo ganador.
- Comunicación entre los hilos para detener la búsqueda.
- Medición del tiempo de ejecución.
- Cálculo del speedup.

## Directivas y funciones OpenMP utilizadas

- `#pragma omp parallel`: crea la región paralela donde trabajan varios hilos.
- `#pragma omp critical`: evita que varios hilos modifiquen al mismo tiempo la información compartida.
- `#pragma omp flush`: permite que los hilos observen los cambios realizados en la variable que indica que la clave fue encontrada.
- `omp_get_thread_num()`: obtiene el identificador del hilo.
- `omp_get_max_threads()`: obtiene la cantidad máxima de hilos disponibles.
- `omp_get_wtime()`: permite medir el tiempo de ejecución.

## Compilación

El programa necesita un compilador de C++ con soporte para OpenMP.

En Code::Blocks se debe tener habilitada la opción:

`-fopenmp`

También puede compilarse desde una terminal utilizando:

`g++ main.cpp -o programa -fopenmp`

## Ejecución

Al ejecutar el programa:

1. Seleccionar la opción para realizar una búsqueda.
2. Introducir la longitud de la clave.
3. Ingresar una clave de prueba utilizando solamente A-Z y 0-9.
4. Seleccionar la cantidad de hilos.
5. El programa realizará primero la búsqueda secuencial.
6. Después realizará la búsqueda paralela.
7. Finalmente mostrará los tiempos de ejecución, el hilo ganador y el speedup obtenido.

Ejemplo de prueba:

- Longitud: 3
- Clave: `M7Q`
- Hilos: 4

También se realizó una prueba de mayor longitud utilizando:

- Longitud: 5
- Clave: `X7K9Q`
- Hilos: 4
