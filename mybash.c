#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pwd.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#include "command.h"
#include "execute.h"
#include "parser.h"
#include "parsing.h"
#include "builtin.h"

#include "obfuscated.h"

static void show_prompt(void)
{
    const char *reset = "\x1b[0m";
    const char *green = "\x1b[32m";
    const char *blue = "\x1b[34m";
    const char *yellow = "\x1b[33m";

    char hostname[1024];
    gethostname(hostname, sizeof(hostname));
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));

    time_t now;
    time(&now);
    const char *user = getenv("USER");

    printf("%s[%s at %s@%s%s %s]%s $ ",
           green, user, blue, hostname, reset, cwd,
           yellow);
    fflush(stdout);
}

int main(int argc, char *argv[])
{
    pipeline pipe;
    pipe = pipeline_new();
    Parser input;

    char *line = NULL;    // Cadena para almacenar la línea de entrada
    size_t len = 0;       // Tamaño del buffer para getline
    ssize_t read = 0;     // Cantidad de caracteres leídos

    while (true)
    {
        ping_pong_loop("ArticBlueWombat");
        show_prompt();
        // obtengo la entrada y luego se la paso a parse new
        // Leer la entrada del usuario (getline se encarga de gestionar el tamaño del buffer)
        read = getline(&line, &len, stdin);

        // Verificar si se ingresó Ctrl-D (EOF)
        if (read == -1)
        {
            printf("\n");
            break;
        }

        // Mostrar la línea de entrada antes de pasarla al parser
        // printf("Entrada recibida: %s", line);
        if(strchr(line, '<')){
            // printf("Redirección de salida\n");
            set_flag_out_true();
        }
        if(strchr(line, '>')){
            // printf("Redirección de entrada\n");
            set_flag_in_true();
        }

        input = parser_new(fmemopen(line, read, "r"));
        pipe = parse_pipeline(input);
        // verificamos si se ingreso ctrl-d, en tal caso cerramos myBash
        if (parser_at_eof(input))
        {
            printf("\n");
            return EXIT_SUCCESS;
        }
        execute_pipeline(pipe);
        pipe = pipeline_destroy(pipe);
        parser_destroy(input);
    }

    if (input != NULL)
    {
        parser_destroy(input);
        input = NULL;
    }
    if (pipe != NULL)
    {
        pipeline_destroy(pipe);
    }
    return EXIT_SUCCESS;
}
