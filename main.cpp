#include <iostream>
#include <cstring>
#include <iomanip>
#include <omp.h>

using namespace std;

class BusquedaExhaustiva
{
private:
    // Conjunto de caracteres permitido por la practica:
    // 26 letras + 10 numeros = 36 caracteres.
    char* caracteres;
    int cantidadCaracteres;

    // Guarda la clave que introduce el usuario.
    char* claveObjetivo;

    // Aqui se guardara la clave cuando sea encontrada.
    char* claveEncontrada;

    int longitud;

public:

    // Constructor de la clase.
    BusquedaExhaustiva(int longitudClave)
    {
        longitud = longitudClave;
        cantidadCaracteres = 36;

        // Se utiliza memoria dinamica, como pide la practica.
        caracteres = new char[cantidadCaracteres + 1];

        strcpy(caracteres, "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

        // +1 porque las cadenas terminan con el caracter '\0'.
        claveObjetivo = new char[longitud + 1];
        claveEncontrada = new char[longitud + 1];

        claveObjetivo[0] = '\0';
        claveEncontrada[0] = '\0';
    }

    // Destructor.
    // Libera la memoria dinamica utilizada por el programa.
    ~BusquedaExhaustiva()
    {
        delete[] caracteres;
        delete[] claveObjetivo;
        delete[] claveEncontrada;
    }

    // Calcula 36 elevado a la longitud de la clave.
    // Esto nos indica cuantas combinaciones posibles existen.
    unsigned long long calcularEspacioBusqueda()
    {
        unsigned long long total = 1;

        for (int i = 0; i < longitud; i++)
        {
            total *= cantidadCaracteres;
        }

        return total;
    }

    // Revisa si un caracter pertenece al conjunto permitido.
    bool caracterPermitido(char caracter)
    {
        for (int i = 0; i < cantidadCaracteres; i++)
        {
            if (caracteres[i] == caracter)
            {
                return true;
            }
        }

        return false;
    }

    // Permite que el usuario introduzca la clave.
    // Sigue preguntando hasta que la clave sea valida.
    void pedirClave()
    {
        char* entrada = new char[100];

        bool valida = false;

        while (!valida)
        {
            cout << "\nIngresa una clave de prueba de "
                 << longitud << " caracteres: ";

            cin >> entrada;

            // Primero comprobamos que tenga la longitud correcta.
            if (strlen(entrada) != longitud)
            {
                cout << "La clave debe tener exactamente "
                     << longitud << " caracteres.\n";

                continue;
            }

            valida = true;

            // Revisamos cada caracter.
            for (int i = 0; i < longitud; i++)
            {
                if (!caracterPermitido(entrada[i]))
                {
                    valida = false;
                    break;
                }
            }

            if (!valida)
            {
                cout << "Solo puedes utilizar letras A-Z y numeros 0-9.\n";
            }
        }

        strcpy(claveObjetivo, entrada);

        delete[] entrada;
    }

    /*
        Convierte una posicion numerica en una combinacion.

        Por ejemplo, para una clave de 3 caracteres:

        indice 0  -> AAA
        indice 1  -> AAB
        indice 2  -> AAC

        Se utiliza un sistema parecido a base 36 porque
        tenemos 36 caracteres disponibles.
    */
    void indiceAClave(unsigned long long indice, char* resultado)
    {
        // Vamos llenando la cadena desde la ultima posicion.
        for (int i = longitud - 1; i >= 0; i--)
        {
            int posicion = indice % cantidadCaracteres;

            resultado[i] = caracteres[posicion];

            indice /= cantidadCaracteres;
        }

        resultado[longitud] = '\0';
    }

    // Version secuencial.
    // Un solo hilo revisa las combinaciones una por una.
    double busquedaSecuencial(unsigned long long& combinacionesRevisadas)
    {
        unsigned long long total = calcularEspacioBusqueda();

        combinacionesRevisadas = 0;

        // Guardamos temporalmente cada combinacion generada.
        char* combinacion = new char[longitud + 1];

        double inicio = omp_get_wtime();

        for (unsigned long long i = 0; i < total; i++)
        {
            indiceAClave(i, combinacion);

            combinacionesRevisadas++;

            // strcmp devuelve 0 cuando las cadenas son iguales.
            if (strcmp(combinacion, claveObjetivo) == 0)
            {
                strcpy(claveEncontrada, combinacion);
                break;
            }
        }

        double fin = omp_get_wtime();

        delete[] combinacion;

        return fin - inicio;
    }

    /*
        Version paralela.

        El espacio completo se divide entre varios hilos.
        Cada hilo recibe un rango diferente de combinaciones.
    */
    double busquedaParalela(
        int numeroHilos,
        int& hiloGanador,
        unsigned long long& combinacionesRevisadas)
    {
        unsigned long long total = calcularEspacioBusqueda();

        hiloGanador = -1;
        combinacionesRevisadas = 0;

        /*
            Esta variable es compartida entre todos los hilos.

            0 = nadie ha encontrado la clave.
            1 = la clave ya fue encontrada.
        */
        int encontrada = 0;

        /*
            Arreglos dinamicos para guardar informacion
            correspondiente a cada hilo.
        */
        unsigned long long* inicioRango =
            new unsigned long long[numeroHilos];

        unsigned long long* finRango =
            new unsigned long long[numeroHilos];

        unsigned long long* cantidadRango =
            new unsigned long long[numeroHilos];

        unsigned long long* revisadasPorHilo =
            new unsigned long long[numeroHilos];

        // Inicializamos las posiciones.
        for (int i = 0; i < numeroHilos; i++)
        {
            inicioRango[i] = 0;
            finRango[i] = 0;
            cantidadRango[i] = 0;
            revisadasPorHilo[i] = 0;
        }

        /*
            Calculamos una division equilibrada.

            Si el total no se puede dividir exactamente,
            los primeros hilos reciben una combinacion extra.
        */
        unsigned long long cantidadBase = total / numeroHilos;
        unsigned long long sobrantes = total % numeroHilos;

        unsigned long long posicionActual = 0;

        for (int i = 0; i < numeroHilos; i++)
        {
            cantidadRango[i] = cantidadBase;

            if ((unsigned long long)i < sobrantes)
            {
                cantidadRango[i]++;
            }

            inicioRango[i] = posicionActual;

            finRango[i] =
                inicioRango[i] + cantidadRango[i] - 1;

            posicionActual = finRango[i] + 1;
        }

        cout << "\nDistribucion del trabajo\n";

        // Mostramos la primera y ultima combinacion de cada hilo.
        for (int i = 0; i < numeroHilos; i++)
        {
            char* primera = new char[longitud + 1];
            char* ultima = new char[longitud + 1];

            indiceAClave(inicioRango[i], primera);
            indiceAClave(finRango[i], ultima);

            cout << "Hilo " << i
                 << " -> Inicio: " << primera
                 << " | Fin: " << ultima
                 << " | Cantidad: " << cantidadRango[i]
                 << endl;

            delete[] primera;
            delete[] ultima;
        }

        double inicioTiempo = omp_get_wtime();

        /*
            OpenMP crea aqui los hilos.

            num_threads indica cuantos hilos queremos utilizar.
            Las variables indicadas en shared son compartidas.
        */
        #pragma omp parallel num_threads(numeroHilos) shared(encontrada, hiloGanador, claveEncontrada, revisadasPorHilo)
        {
            // Cada hilo obtiene su identificador.
            int idHilo = omp_get_thread_num();

            // Cada hilo crea su propia combinacion.
            char* combinacion = new char[longitud + 1];

            // critical evita que varios hilos impriman al mismo tiempo.
            #pragma omp critical(salida)
            {
                cout << "[Hilo " << idHilo
                     << "] Iniciando busqueda.\n";
            }

            /*
                Cada hilo solamente recorre el rango
                que le fue asignado.
            */
            for (unsigned long long i = inicioRango[idHilo];
                 i <= finRango[idHilo];
                 i++)
            {
                /*
                    flush permite que el hilo consulte el valor
                    actualizado de la variable compartida encontrada.
                */
                #pragma omp flush(encontrada)

                // Si otro hilo ya encontro la clave, dejamos de buscar.
                if (encontrada)
                {
                    break;
                }

                indiceAClave(i, combinacion);

                revisadasPorHilo[idHilo]++;

                if (strcmp(combinacion, claveObjetivo) == 0)
                {
                    /*
                        Solo un hilo puede entrar a esta seccion
                        a la vez.

                        Esto evita una condicion de carrera sobre
                        encontrada, hiloGanador y claveEncontrada.
                    */
                    #pragma omp critical(resultado)
                    {
                        if (!encontrada)
                        {
                            encontrada = 1;

                            hiloGanador = idHilo;

                            strcpy(claveEncontrada, combinacion);

                            cout << "\nClave encontrada por el hilo: "
                                 << idHilo << endl;

                            cout << "Clave encontrada: "
                                 << claveEncontrada << endl;
                        }
                    }

                    /*
                        Hacemos visible a los demas hilos
                        que la clave ya fue encontrada.
                    */
                    #pragma omp flush(encontrada)

                    break;
                }
            }

            /*
                Comprobamos el estado final del hilo.
                La impresion tambien se protege para que sea legible.
            */
            #pragma omp critical(salida)
            {
                if (idHilo == hiloGanador)
                {
                    cout << "[Hilo " << idHilo
                         << "] Finalizado - encontro la clave.\n";
                }
                else if (encontrada)
                {
                    cout << "[Hilo " << idHilo
                         << "] Finalizado - busqueda detenida.\n";
                }
                else
                {
                    cout << "[Hilo " << idHilo
                         << "] Finalizado - clave no encontrada en su rango.\n";
                }
            }

            // Cada hilo libera la memoria que utilizo.
            delete[] combinacion;
        }

        double finTiempo = omp_get_wtime();

        // Sumamos cuanto trabajo realizaron todos los hilos.
        for (int i = 0; i < numeroHilos; i++)
        {
            combinacionesRevisadas += revisadasPorHilo[i];
        }

        // Liberamos los arreglos dinamicos.
        delete[] inicioRango;
        delete[] finRango;
        delete[] cantidadRango;
        delete[] revisadasPorHilo;

        return finTiempo - inicioTiempo;
    }

    // Ejecuta ambas versiones usando exactamente la misma clave.
    void ejecutar(int numeroHilos)
    {
        unsigned long long total = calcularEspacioBusqueda();

        cout << "\nClave de prueba: " << claveObjetivo << endl;
        cout << "Longitud: " << longitud << endl;
        cout << "Caracteres disponibles: " << cantidadCaracteres << endl;
        cout << "Combinaciones posibles: " << total << endl;
        cout << "Hilos utilizados: " << numeroHilos << endl;


        // BUSQUEDA SECUENCIAL

        cout << "\nBusqueda secuencial\n";

        unsigned long long revisadasSecuencial = 0;

        double tiempoSecuencial =
            busquedaSecuencial(revisadasSecuencial);

        cout << "Clave encontrada: "
             << claveEncontrada << endl;

        cout << "Combinaciones revisadas: "
             << revisadasSecuencial << endl;

        cout << fixed << setprecision(6);

        cout << "Tiempo secuencial: "
             << tiempoSecuencial
             << " segundos\n";


        // BUSQUEDA PARALELA

        cout << "\nBusqueda paralela\n";

        int hiloGanador = -1;

        unsigned long long revisadasParalelo = 0;

        double tiempoParalelo =
            busquedaParalela(
                numeroHilos,
                hiloGanador,
                revisadasParalelo
            );

        cout << "\nResultado de la busqueda paralela\n";

        cout << "Clave encontrada: "
             << claveEncontrada << endl;

        cout << "Hilo ganador: "
             << hiloGanador << endl;

        cout << "Combinaciones revisadas entre los hilos: "
             << revisadasParalelo << endl;

        cout << "Tiempo paralelo: "
             << tiempoParalelo
             << " segundos\n";


        // COMPARACION

        cout << "\nComparacion de rendimiento\n";

        cout << "Tiempo secuencial: "
             << tiempoSecuencial
             << " segundos\n";

        cout << "Tiempo paralelo: "
             << tiempoParalelo
             << " segundos\n";

        if (tiempoParalelo > 0)
        {
            double speedup =
                tiempoSecuencial / tiempoParalelo;

            cout << "Speedup: "
                 << speedup << endl;

            /*
                Si el speedup es mayor a 1,
                la version paralela fue mas rapida.
            */
            if (speedup > 1)
            {
                cout << "La version paralela fue aproximadamente "
                     << speedup
                     << " veces mas rapida.\n";
            }
            else
            {
                cout << "En esta ejecucion el paralelismo "
                     << "no produjo una mejora de tiempo.\n";
            }
        }
    }
};


int main()
{
    int opcion;

    cout << "Actividad 2.6 - Busqueda exhaustiva con OpenMP\n";
    cout << "Conjunto utilizado: A-Z y 0-9\n";

    do
    {
        cout << "\n1. Realizar una busqueda\n";
        cout << "0. Salir\n";
        cout << "Opcion: ";

        cin >> opcion;

        if (opcion == 1)
        {
            int longitud;

            cout << "\nLongitud de la clave: ";
            cin >> longitud;

            /*
                El ejercicio permite trabajar con distintas
                longitudes. Limitamos el programa hasta 10
                porque es la longitud maxima propuesta.
            */
            while (longitud < 1 || longitud > 10)
            {
                cout << "Ingresa una longitud entre 1 y 10: ";
                cin >> longitud;
            }

            /*
                Creamos el objeto encargado de realizar
                todas las operaciones de la busqueda.
            */
            BusquedaExhaustiva busqueda(longitud);

            busqueda.pedirClave();

            int maximoHilos = omp_get_max_threads();
            int numeroHilos;

            cout << "\nTu equipo permite hasta "
                 << maximoHilos
                 << " hilos mediante OpenMP.\n";

            cout << "Cantidad de hilos a utilizar: ";
            cin >> numeroHilos;

            while (numeroHilos < 1 ||
                   numeroHilos > maximoHilos)
            {
                cout << "Ingresa una cantidad entre 1 y "
                     << maximoHilos << ": ";

                cin >> numeroHilos;
            }

            busqueda.ejecutar(numeroHilos);
        }

        else if (opcion != 0)
        {
            cout << "Opcion no valida.\n";
        }

    } while (opcion != 0);

    cout << "\nPrograma finalizado.\n";

    return 0;
}
