#include <assert.h>   // permite usar la funcion assert()
#include <stdio.h>    // permite usar printf()
#include <stdlib.h>   // permite usar exit()
#include <unistd.h>   // permite usar fork(), getpid()
#include <sys/wait.h> // permite usar wait()
#include <fcntl.h>    // permite usar open() y otras constantes
#include <string.h>   // permite usar strdup()

#include "execute.h"            // contiene los prototipos de las funcines
#include "command.h"            // definicion del tipo 'pipeline' y permite llamar a las funciones del TAD
#include "builtin.h"            // permite llamar a las funciones de builtin
#include "tests/syscall_mock.h" // requisito para pasar los tests
#include "syntax.h"

/*
 * Módulo que maneja el redireccionamiento de entrada ('<') del comando simple
 */
static void redirection_in(char *redir_in)
{
    if (redir_in)
    {                                               // si redir_in != NULL ...
        int in = open(redir_in, O_RDONLY, S_IRWXU); // lo abre (con permisos de lectura)
        dup2(in, STDIN_FILENO);                     // duplica el valor de 'in' en el descriptor 0 (STDIN_FILENO)
        close(in);                                  // cierra 'in' por no ser necesario de nuevo
    }
}

/*
 * Módulo que maneja el redireccionamiento de salida ('>') del comando simple
 */
static void redirection_out(char *redir_out)
{
    if (redir_out)
    {                                                                     // si redir_out != NULL ...
        int out = open(redir_out, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU); // lo crea (con permisos de escritura)
        dup2(out, STDOUT_FILENO);                                         // duplica el valor de 'out' en el descriptor 1 (STDOUT_FILENO)
        close(out);                                                       // cierra 'out' por no ser necesario de nuevo
    }
}

/*
 * Módulo encargado de ejecutar cada comando externo simple
 * Verifica la presencia de redireccionamientos de entrada y salida
 */
static void execute_simple_command(scommand cmd)
{

    redirection_in(scommand_get_redir_in(cmd)); // revisa si debe obtener el valor 'in_redir' de 'cmd'

    redirection_out(scommand_get_redir_out(cmd)); // revisa si debe obtener el valor 'out_redir' de 'cmd'

    unsigned int cmd_len = scommand_length(cmd); // obtengo la cantidad de argumentos del comando

    char **myargs = calloc(cmd_len + 1, sizeof(char *)); // declara un arreglo de 'strings' de tamaño 'cmd_len+1' para los argumentos de execvp()

    for (size_t j = 0; j < cmd_len; j++) // itera cada argumento del comando...
    {
        myargs[j] = strdup(scommand_front(cmd)); // guardandolos en el arreglo
        scommand_pop_front(cmd);                 // y borrándolos 1 a 1 con cada ciclo
    }

    myargs[cmd_len] = NULL; // NULL marca el final del array para execvp()

    execvp(myargs[0], myargs); // ejecuta el comando con sus argumentos (si los hay)

    // el proceso no debe llegar hasta aqui de ejecutarse correctamente
    printf("%s : command not found\n", myargs[0]);
    suggest_command(myargs[0]);
    exit(EXIT_FAILURE);
}

/*
 * Módulo encargado de redirigir la entrada del pipe
 */
static void redirect_pipe_in(int descriptor_in)
{
    if (descriptor_in != STDIN_FILENO)
    {
        dup2(descriptor_in, STDIN_FILENO); // Redirige stdin al pipe
        close(descriptor_in);              // Cierra el descriptor de entrada auxiliar
    }
}

/*
 * Módulo encargado de redirigir la salida del pipe
 */
static void redirect_pipe_out(int descriptores[])
{
    close(descriptores[0]);               // Cierra el extremo de entrada del pipe
    dup2(descriptores[1], STDOUT_FILENO); // Redirige stdout al pipe
    close(descriptores[1]);               // Cierra el extremo de salida del pipe
}

/*
 * Módulo que controla que el proceso 'parent' cierre correctamente el descriptor auxiliar
 */
static void descriptor_in_close(int descriptor_in)
{
    if (descriptor_in != STDIN_FILENO)
    { // nunca en el primero, en donde descriptor_in empieza inicializado en STDIN_FILENO
        close(descriptor_in);
        descriptor_in = -1; // Invalida descriptor_in para evitar cierres innecesarios en futuros ciclos
    }
}

/*
 * Módulo encargado de ejecutar los comandos externos
 * Itera el ciclo de ejecuciones por cada comando del 'pipeline'
 */
static void execute_external_command(pipeline apipe)
{

    unsigned int apipe_len = pipeline_length(apipe); // cuenta cuantos comandos hay separados por '|'

    int *child_pid = malloc(apipe_len * sizeof(int)); // se crea un array para contener los 'process ID' de todos los 'child' creados

    // para conectar cada comando del pipeline se crean descriptores de archivos
    int descriptores[2];              // descriptores de archivo para el pipe
    int descriptor_in = STDIN_FILENO; // descriptor auxiliar para la entrada, se inicializa como STDIN_FILENO

    for (size_t i = 0; i < apipe_len; ++i)
    { // itera el ciclo con fork() por cada comando individual del 'apipe' según 'apipe_len'

        if (i < apipe_len - 1)
        { // Si no es el último comando, crea un pipe para conectar las entradas y salidas
            pipe(descriptores);
        }

        // a partir de aqui, parte del código es tomado del capítulo 5 de OSTEP
        int rc = fork(); // llama a fork() y crea 2 procesos iguales ('parent' y 'child'), excepto por el valor de 'rc'

        if (rc < 0)
        { // rc < 0, significa que el fork() falló
            fprintf(stderr, "fork failed\n");
            exit(EXIT_FAILURE);
        }

        else if (rc == 0) // rc == 0, significa que el proceso es el 'child'
        {

            redirect_pipe_in(descriptor_in); // Redirección de la entrada del pipe (si no es el primer comando)

            if (i < apipe_len - 1)
            { // Redirección de la salida del pipe (si no es el último comando)
                redirect_pipe_out(descriptores);
            }

            execute_simple_command(pipeline_front(apipe)); // obtiene el primer comando de 'apipe' y llama a la función para ejecutarlo
        }

        else // rc > 0, significa que es el 'parent'
        {
            // solo cierra los descriptores si es necesario (si hubo manipulacion de archivos) estas 2 condiciones son necesarias para pasar los 'tests':

            descriptor_in_close(descriptor_in); // 1- el padre sólo cierra el descriptor de entrada si se manipulo en este ciclo

            if (i < apipe_len - 1)
            {                                    // 2- excepto en el último comando, el padre debe siempre cerrar el descriptor de salida y asignar la siguiente entrada.
                close(descriptores[1]);          // El padre no necesita escribir en el pipe
                descriptor_in = descriptores[0]; // asigna el valor de descriptor_in para el siguiente ciclo
            }

            child_pid[i] = rc; // como 'rc' en el proceso del 'parent' tiene el valor de PID del 'child', se lo puede guardar en el arreglo de PIDs

            pipeline_pop_front(apipe); // elimina el comando ya usado de 'apipe'
        }
    }
    // modificado para que pase los 'tests' (antes sólo el proceso 'parent' esperaba)
    // el proceso que llega hasta esta línea solo espera segun el valor de pipeline_get_wait(), que normalmente es 'true'
    if (pipeline_get_wait(apipe))
    {

        for (unsigned int j = 0; j < apipe_len; ++j) // se itera según la cantidad de comandos del 'pipeline'

            waitpid(child_pid[j], NULL, 0); // se espera hasta que cada 'child' haya terminado su proceso.
    }
}

/*
 * Ejecuta un 'pipeline', identificando comandos nulos, vacíos, internos o externos
 */
void execute_pipeline(pipeline apipe)
{
    // Caso 1 - el 'pipeline' es NULL
    assert(apipe != NULL); // por consigna ' Requires: apipe!=NULL '

    // Caso 2 - el 'pipeline' es vacio
    if (!pipeline_is_empty(apipe)) // si el 'pipeline' esta vacio, termina la función, sólo lo ejecuta si tiene contenido
    {
        // Caso 3 - el 'pipeline' es un comando simple presente en builtin.c
        if (builtin_alone(apipe))
        {
            builtin_run(pipeline_front(apipe)); // y en ese caso, simplemente obtiene el comando y lo ejecuta el módulo builtin.c
        }
        // Caso 4 - el 'pipeline' es un comando externo
        else
        {
            execute_external_command(apipe); // llamada a la función que ejecute los comandos externos (simples y múltiples)
        }
    }
}
