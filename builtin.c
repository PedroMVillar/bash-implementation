#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>
#include <time.h>

#include "tests/syscall_mock.h"
#include "command.h"
#include "builtin.h"

#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

#define PATH_MAX 4096

typedef void (*CommandFunc)(scommand cmd);

// Guardaremos los comandos internos en un struct para hacer que agregar comandos nuevos sea mas simple
typedef struct
{
    // nombre del comando, de la forma que deberá ser ingresado por consola
    const char *name;
    // función de tal comando
    CommandFunc func;
} Command;


/*
------------------------------------------------------------
  *    Función encargada de cambiar de directorio
------------------------------------------------------------
*/
static void cmd_cd(scommand cmd)
{
    // "consumimos" cd
    scommand_pop_front(cmd);
    // caso de que a cd no se le indicó un directorio, nos lleva a Home
    if (scommand_length(cmd) == 0)
    {
        char *home = getenv("HOME");
        chdir(home);
    }
    // a cd sí se le indicó un directorio
    else
    {
        // nos dirigimos hacia donde se nos haya indicado
        int res = chdir(scommand_front(cmd));
        // casos de error
        if (res != 0)
        {
            perror("Directory change failed\n");
        }
        else if (res == -1)
        {
            perror("Directory change failed\n");
        }
    }
}

/*
---------------------------------------------------------------
  *          Función encargada de cerrar myBash
---------------------------------------------------------------
*/
static void cmd_exit(scommand cmd)
{
    exit(0);
}

/*
---------------------------------------------------------------------
  *             Función encargada de mostrar la ayuda
         -- muestra información sobre los comandos internos --
  -- los colores usados están definidos al principio del archivo --
---------------------------------------------------------------------
*/
static void cmd_help(scommand cmd)
{
    printf(BLUE "---   myBash   --- \n" RESET);

    printf(GREEN "Members: \n" RESET);
    printf(CYAN "Pedro Martin Villar \n" RESET);
    printf(CYAN "Gustavo Federico Grisetti \n" RESET);
    printf(CYAN "Fabrizio Mandile \n" RESET);
    printf(CYAN "Victoria Testa \n" RESET);

    printf(GREEN "Commands you might want to use:\n" RESET);

    printf(YELLOW "- cd <path>   " RESET BLUE "- changes the directory you are currently in\n" RESET);
    printf(YELLOW "- help        " RESET BLUE "- will give you some brief explanations of (almost) every command\n" RESET);
    printf(YELLOW "- exit        " RESET BLUE "- closes myBash\n" RESET);
    printf(YELLOW "- pwd         " RESET BLUE "- shows you your current directory\n" RESET);
    printf(YELLOW "- ps          " RESET BLUE "- allows you to view information about the current running processes on your system\n" RESET);
    printf(YELLOW "- echo        " RESET BLUE "- outputs the strings that are passed to it as arguments\n" RESET);
    printf(YELLOW "- kirby       " RESET BLUE "- use at your own risk\n" RESET);
    printf(YELLOW "- cowsay      " RESET BLUE "- makes Lola say whatever you want!\n" RESET);
}

/*
---------------------------------------------------------------
  *  Función encargada de mostrar a Kirby bailando (EXTRA)
    -- se usó la libreria time para setear un cronómetro --
---------------------------------------------------------------
*/
static void cmd_kirby(scommand cmd)
{
    time_t start_time = time(NULL); // Obtener el tiempo de inicio

    printf("\033[H\033[J");
    printf("\n \n");
    while (1) {
        if (difftime(time(NULL), start_time) >= 10) {
            break;
        }
        /* Frame #1 */
        printf("                                          \n                 ..^~^^^~~!!!^.                   \n            .77~?J7777!!7??JYY5?~~                \n       ^?~:::~^:7J???7!~^^^~!!7??JJ!              \n     ^YBB7~~^    ^^!?YJJ?7!~^^~~!77J7             \n   ?B#BPYJ???7!~^^^:^^~?7JJ?7!~^~~!!7?:           \n   #&BPYJ?!~!!~~^^^^^::::^!JJ?77~^^~~!7:          \n  7BGPPPJ~7!~~^^^^^^^^^^^::^^~777!7JYJ^^:         \n Y&#GGY7?7!~~^^^^^^^^^^^^^^:::^^^~YB##Y^^.        \n^&&##G?777!~~^^^^^^^^^^^^^^::^~??77?B##57~:       \nY&&&5???7!~~~^^^^^^^^^^^^^^^75P5J7!~!5B#5~:^:     \n.&&&5?J??7!~~^^^^^^^^^^^^^^PBPY?!^:^YYP#&Y!~~^.   \n :JYYJJ?!?!~~~~^^^^^^^^^^^^GGPY7~^^??~!BPPG?^~7   \n    Y?YY?G7!!~^7!^^^^^^^^^^PBBP55PP57~JJJP?^~^^   \n    Y?P&&#77P7:5?^^^^^^^^^^G##GPGGBBPYY5PGGJ?~    \n    JY5##Y7?B&&&J^^^^^^^^^^JB&#P5Y5BBB#BPPG5?~^.  \n    .7Y55?77JB&P7~^^^^^^^^^:^?G##BGB##P555J!^:^^^ \n     ~?JY5PY5YJ7777!~^^^^^^^::^?55YYYJJJY7^:^^^^^.\n     ~YY?J5YYJ???JY?7~~^^^^^^^::::^^^^^^^::^^^^^^:\n     YBGPYJ???JJ???77!!!~~~^^^^^^^^!7!!!!~~!!!~.  \n      7#BBG5JJJ?JJJJ???77!!!!~~~~!?JP5J^^^~~~:    \n       :^YYYJ?!YYJ?JJJJ?77???J?Y5PGGGGP7.         \n               ... :^^^  .!PGGGGGPPPPPPB~   \n ");
        printf("\033[H");
        usleep(100000);

        /* Frame #2 */
        printf("                                                  \n           ^JJBP55P7::^7!77~~!?7^~~..             \n         :P#B!^.         ::7J7~~!7?JJ~!.          \n       ~YBGY77.      .....  ^?Y?7~^^!7?Y:         \n     5BGGY?7?77!~^^::^^^^^::::~?Y?!~^~!!7.        \n   .7GPYJJ?7!~~~^^^^^^^^^^^^^^:^~?J?!~~!7!:       \n  :#&BP5Y?!!!~~^^^^^^^^^^^^^^^^^::^!~?GBB:        \n !&##BPJ77!!~~^^^^^^^^^^^^^^^:::^^^!^^?B#Y:       \n:&&&&5??77!~^^^^^^^^^^^^^^^^^^^!?YJ?!~^7G&P^.     \n:&@&&5????^~^^^^^^^^^^^^^^^^:J5Y??7^::!77P#Y^.    \n :J#GY?J5Y?~~~!:~!~^^^^^^^^^:JP5J~:::!J7?B#?~~~   \n   :J??P@&5~!PY^JY~^^^^^^^^^7PGPJ7YYY?~^PBY5P?^~  \n   .5YJG&G7!?&@@&?^^^^^^^^:^GBBP?JPGGY7J?J557~~!  \n   .YYY55?777P##J~^:^^^^^^^^?PBB5J?YBGPPPGGY77?.  \n   .YJJJY5P55Y7!!!!~^^^^^^^^:^J#BP5PB#BGPPGG5JY   \n    ^?JJ?JYJ??777?!?!^^^^^^^^:^J#BGGGGP55YY?~~^.  \n     :!JJ????7777!!~~^^^^^^^^^:^~~^~YYJJJ!^^::^:  \n      ^5JJJ?J??777!!!~~~^^^^^^^::^~^^^^^:::^^^^:  \n      !#PJ?J???7777!!!!~~~~~^^^^:^77!~~~^^^^^^^^. \n     ^P#BG5YJJ????77777!!!~~~~^^~!JYJ??77!!!!~~.  \n      ~G#BBBGGP5YJJJ?J??J?7?7?JYPPGGPPP57^^^^     \n        :.!7::::.........^GGGGGGPPPPPPPG^         \n                          .PBGPPPPPPPPPG^      \n");
        printf("\033[H");
        usleep(100000);

        /* Frame #3 */
        printf("             .G##BPPPGGGP7!!!~^^!:^^!             \n          .:JGBBG^              :Y?!7!^~:.        \n        .YBG5Y7~:............     .~?!~!7J?.      \n      .5G5J?77!~~^^^^^^^^^^^^^:::...~?7~^~?^      \n     ~5B5YYJ7~~^^^^^^^^^^^^^^^^^^^^::^~~?7!7~     \n   :B#BG5J?~~~~^^^^^^^^^^^^^^^^^^^:^^^^^7P#?.     \n   #&##P?!!!~~^^:^^^^^^^^^^^^^^:^^^7J!?!^7G#5     \n   #&&BY777~^~^^~~!!^^^^^^^^^^:~?J?77~^:^^?#P::   \n   #&&P775#P5~~Y~.~?~^^^^^^^^:~JYYJ~::.~JY~J#5!   \n   .7JJ?J#&B!~7&B5BJ:^^^^^^^^^JP557?7!7?!7P#5~~~  \n    ^J55J5J!~~J&@&J^^^^^^^^^:JGPPY7YGGJ~:!JY5PJ!: \n    YJJJ?J?7!!7YY!^^^^^^^^^^^^JBGP?77GPJ77JGP?77  \n    YJ?777Y5YY?~~7??7~^^^^^^^:!5GG5??BBBGGBB57J?  \n    ^7??777!!!!~~~!7!~^^^^^^^^:7BBB55GGP5PGBGYY.  \n     !J????77!!!~~~^^^^^^^^^^^^^!JJGBGGGP5PJ~^    \n     :!J?????77!!!~~~^^^^^^^^^^:::^~~~JYJ!~:.     \n       !J??????7!!!~~~~^^^^^^^^^~!~~~^^:::^^^.    \n      ^GYJJ??????77!!~~~~~^^^^^~?77!~~~^^^^^^:.   \n      ~#BGG5JJ?????777!!~~~~~~^!???77!!~~^^^^:.   \n      ^##BBBGG5YYYJJJ????77!!7?JJ????77!~~^^^.    \n       :Y#BBBBBY??::~~~~~!55PGPGP5YJJ?77!!~~^     \n         :::::.           JGBGGGPGPPPYYY!::.      \n                           YBBGGGGPPPPGG^        \n");
        printf("\033[H");
        usleep(100000);

        /* Frame #4 */
        printf("            .JBB5?:              .7J?!!?7.        \n         :YGPPY7:.......::::....   .!?!~~!?~      \n        :BPJ?77!~^^^^^^^^^^^^^^^^:.. ^7!^~!~:     \n      .JG5J??7~^^^^^^^^^^^^^^^^^^^^^:...~J??7     \n     .J#GP5J!~^^^^^^^^^^^^^^^^^^^^::^^^:^5&J      \n    P##BG57~~!~^^^^^:^^^^^^^^^^::^!~77!7~7BG?     \n   .&&&#P7!~!^^^:^^~^^^^^^^^^^:^!??7!!^^~^?#G^.   \n   .&&&G?!?P~?~^!^.^?^^^^^^^^^:!5JJ~^:.:?J~J#5!   \n    G&BJ7?G@#7~!B?~YJ:^^^^^^^:!55J?!~^~J7?YGG7~.  \n     ~?J?J&#?~~P@@&5^^^^^^^^^^JGPY75G5J~^?P5Y?!?: \n     ~J55?Y!~~~P&#5~^^^^^^^^:!YGP5?7YG5!?J?5GY7J^ \n   :!7JJJ?5?77!!?7~^^^^^^^^^^:7GGPJ?7GP5YYPP?7?!. \n .?!^!J?77?555J!~!?JJ!^^^^^^^^^?BBPYJB###BBGJ?5   \n .J!~!J?7777!!~!~~!77~^^^^^^^^:7#BBPPGPYY5GBGY7   \n ~?!~~7J??7777!!!~~~~^^^^^^^^^^^755B#BGGPPYJ!     \n 7?7!~!?????7777!!~~~~~^^^^^^^^:::^~!!YP7~~.      \n  .777~!??????777!!~~~~^^^^^^^^^^^^::::::^:       \n   ... 7Y????J??77!!~~~~~~~~^^^^^^^^^^^^^^^.      \n      ~#BPY???J??777!!!!!~~~~!!~^^^^^^^^^^^.      \n      ~#BBBPYYJ?????77777!!!!77!~~~^^^^^^^^.      \n      ^B#BBBGGPYYYJ?JJJJJ????YJ7!!~~~^^^^^:       \n       .?BBBBBBJ!!..^::::^PPGGGPJ7!~^^:^:         \n         ......           7GBGGGGP?7???P~         \n                           J#BGGGGPPGGGG!         \n");
        printf("\033[H");
        usleep(100000);

        /* Frame #5 */
        printf("           .P#BJ:          :J?7!~^~7?~!:          \n         ~PG55!     ....... ..7Y?!~^~77J!         \n       ^5B5?77J~:..^^^^^^^^^^^:^7?7!^^~!?.        \n      ^PPJ?77!^~^^^^^^^^^^^^^^^^:^!7!~~!!^.       \n     5BPYYJ7~~~^^^^^^^^^^^^^^^^^^::^^?G#P:        \n   ^##BGPY!~~~~^^^^^^^^^^^^^^^^:^^^~^^?B#7        \n   &&#BGJ!!!~~^^^^^^^^^^^^^^^:^!JYJ?!~~5B#J.      \n   &&&P?777!~^^^^^:^^^^^^^^^:!JYYY?^::7!YBG?~     \n   &&&P77?7:~^^~~!~^^^^^^^^^:J5YJ~:..~?JJGB7~^    \n   ?#&P?JGP5~~?^.!?^^^^^^^^^^?PG5~!77?^^GGYPJ^~   \n     ?J?P@&Y!7#GYGJ:^^^^^^^^5GGPJJGG5!^7?J5P7~7   \n    :75Y5&P!!7&@@G~^^^^^^^^:5BBGY??5G5?7JP57!~:   \n .^77?Y555?777P#P!^^:^^^^^^^^5BBGYJJBBBBBBB5?!    \n .!7!?JJJY555Y?7!!7!~^^^^^^^:^Y#BBPP#BBP5PBGY^    \n   77?JJJJJYYJ?77??J?~^^^^^^^:!P#BBBBBGGGPY~      \n   7?7?JJJJ???777!!!~~~^^^^^^^:^~~~!Y55YY7^.      \n   .^7??JJJJJ???777!!!~~~~^^^^^^^^^:::::::^.      \n      7BP?JJJ?????7777!!~~~^^^^~~^^^^^^^^^^::     \n      ~#BPJJ?JJJ????777777!!!~!7!~~~^^^^^^^^^     \n     ^5#BBGPYJJ??????77777777!7?77!!~~^^^^^^^     \n     .!G##BBBBGP5YJJJJJJJJ??JJ55J?7!!!~^^^^.      \n        ::!?::::::.......^GBBGGGPPJ77!~~^.        \n                          .PBGGGGGG5J?75~         \n");
        printf("\033[H");
        usleep(100000);

        /* Frame #6 */
        printf("          :5BB5~~!: .!~JY7~~~~!7?J5?.             \n         ^PG??.        .!!JJ7~~~~!7??Y^           \n       :JBP!~!.    .......:!JJ?!~^^~!7J.          \n     ~YBGJ77?~!~^::^^^^^^^^:^!?J?7~:~!!~.         \n     YB5J?7^~^^^^^^^^^^^^^^^^:^~777!~~J^.         \n   .GG5Y5?~~~~^^^^^^^^^^^^^^^^^::^^~PB#J.         \n  .B#BGG57!!~~^^^^^^^^^^^^^^^^:^^~~!JB##?         \n :&&##BY?7!!~^^^^^^^^^^^^^^^:^~?JJJ?~~P#BJ^.      \n :@&&BY?77?~^^^^:^^^^^^^^^^^!JJY5?~::7?P#5!:.     \n :&&&P??JJ~~^^^!!^^^^^^^^^^:J5YJ!:..~JJP#P7!~:    \n  .?#P??5P!~~~~:!!^^^^^^^^^:JGG5!!7??~^GP5PY~7.   \n     7JY#@B!!PJ^YJ^^^^^^^^^7PGPJJGBP?~???PY~^!.   \n    .7J5#&Y!7#@&&J^^^^^^^^^G#BGYJY5BGYJYPGY~7Y.   \n    JY55YY?775&&5~^^^^^^^^^~?G#GPYYG#B##BBBPYY.   \n    JJJJJJJJ??J?!~~^^^^^^^^::7G&#BGGBGP55PPY~     \n    YJJJJJYG5Y777?JJ7^^^^^^^^:!YP##BBGGGG57.      \n    ^7JJ?JJ?????777!~~~^^^^^^^::^!!!!~~~~^:::     \n      !GY?JJJ??J??777!~~~^^^^^^^!!~~~^^^:^^^^     \n     ^Y#GY??JJJJJJ???777!!~~~~^~77!!~~^^^^^^^^    \n     Y#BBG5YJJ??JJJJ????7777!!!7??77!!~~^^^^:~.   \n      ~P#BBBBGP5YJJJJJJ?7?777?JY5??777!~~^^^^     \n        :~YYYYYYJ!^~~~~~~!GGGGGGPPY??7!!~~^:      \n                          7GBGGGGGPPPJJ?~..       \n");
        printf("\033[H");
        usleep(100000);

        /* Frame #7 */
        printf("              .7!~^7?!!!!77JY5?7~                 \n         :!?P?7?!!~?JJ?!~~~!!7?JY?~~.             \n       .?##?.       .~!JJ?7!~^^~!7?YY             \n     :?BG?P7::: ...::.:^~?JJ7!^^~!!777.           \n    P##GY?7???7^^^^^^^^^^:^?JJ7!~^~~!7^           \n   .&#PYJ?7!~~^^^^^^^^^^^^::^7J?7!~^7?!           \n   P#G555J!!~~^^^^^^^^^^^^^^::^77^~5#B?^.         \n .G#BGG5?7!!~~^^^^^^^^^^^^^^^::^^^~JB##7:         \n ~&##BPJ77!!~~^^^^^^^^^^^^^::^!???J7?B#P!^.       \n.#&&&P??77!~~~^^^^^^^^^^^^^^?YY5J7^:75B#P!~:      \n ^@&&P??J?!~~^^:^^^^^^^^^^^5P5Y?~^:^5Y5#G?!!~:    \n  Y#&P?YY~7~~~~7~^^^^^^^^^^PP5Y7~~^J7~YPJ5P?^^    \n   .JY?PB7?!!J:^7~^^^^^^^^^PBG5J5G5Y7~Y?YGJ^!~    \n    .7?P@&?!Y#!J57^^^^^^^^^P##P5PGBG5?7JPBY!?~    \n    ~?JP&#77J&&@G!^^^^^^^^^JB#BPYJ5BBBBBB#GYY7    \n     !5YYJ777B&#?^^^^^^^^^^:^5##GPP##BP55PPY?.    \n     !J?YG5JYYJ7!!!~~^^^^^^^:~5&&#BBBGGGGP?^^     \n     .^JJJ5Y55?7?JYJ7^^^^^^^^:~777?77!!!!~:^^.    \n      :PJ?????J???7!!~~~^^^^^^::^~!~~^^:::^^^:.   \n     !PBPJ??J?JJJJ??77!!!~~~^^^^~?7!!~^^^^^^^^^:  \n     JBBGG5J???JJJJJ????77!!!~~~!J?77!!~~^^^^^~   \n      :5BBBBPPYYYJJJJ?JJ??77777?YPJ??77!!~~^^:.   \n        :??????77!^7??7^^~55PGPPGGPYJJ?77!!^^:    \n                          !GGGGPPGGGPPYY7:.       \n");
        printf("\033[H");
        usleep(100000);
    }
    printf("\033[H\033[J");
}

/*
---------------------------------------------------------------
  * Función encargada de mostrar el directorio actual (EXTRA)
---------------------------------------------------------------
*/
static void cmd_pwd(scommand cmd)
{
    char cwd[1024];
    if (getcwd(cwd, sizeof(cwd)) != NULL)
    {
        printf("%s\n", cwd);
    }
    else
    {
        perror("getcwd");
    }
}

/*
---------------------------------------------------------------
* Función encargada de mostrar una vaca diciendo algo (EXTRA)
---------------------------------------------------------------
*/
static void cmd_cowsay(scommand cmd)
{
    scommand_pop_front(cmd);
    char message[256] = "";
    while (!scommand_is_empty(cmd))
    {
        strcat(message, scommand_front(cmd));
        strcat(message, " ");
        scommand_pop_front(cmd);
    }
    printf("  _______\n");
    printf(" < %s >\n", message);
    printf("  -------\n");
    printf("        \\   ^^\n");
    printf("         \\  (oo)\\________\n");
    printf("            (  )\\       )\\/\\\n");
    printf("                ||----w |\n");
    printf("                ||     ||\n");
}

/*
---------------------------------------------------------------
  *   Función encargada de imprimir lo ingresado (EXTRA)
---------------------------------------------------------------
*/
static void cmd_echo(scommand cmd)
{
    scommand_pop_front(cmd);
    while (!scommand_is_empty(cmd))
    {
        printf("%s ", scommand_front(cmd));
        scommand_pop_front(cmd);
    }
    printf("\n");
}
/*
------------------------------------------------------------------
* Función encargada de mostrar los procesos en ejecución (EXTRA)
------------------------------------------------------------------
*/
static void cmd_ps(scommand cmd)
{
    DIR *proc_dir;                     // Directorio de /proc
    struct dirent *entry;              // Entrada de directorio
    char path[PATH_MAX], cmdline[256]; // Buffer para el path y el comando
    FILE *fp;                          // File pointer
    /* Abrir el directorio /proc */
    proc_dir = opendir("/proc");
    if (!proc_dir)
    {
        perror("opendir");
        return;
    }
    printf("\033[0;31mPID\tCOMMAND\033[0m\n");
    printf("===\t=======\n");
    /* Leer el directorio /proc */
    while ((entry = readdir(proc_dir)) != NULL)
    {
        /* Si el nombre del directorio es un número, es un proceso */
        if (entry->d_type == DT_DIR && strspn(entry->d_name, "0123456789") == strlen(entry->d_name))
        {
            snprintf(path, sizeof(path), "/proc/%s/cmdline", entry->d_name);
            /* Leer el comando del proceso */
            fp = fopen(path, "r");
            if (fp)
            {
                if (fgets(cmdline, sizeof(cmdline), fp) != NULL)
                {
                    printf("%s\t%s\n", entry->d_name, cmdline[0] ? cmdline : "[kernel process]"); // Imprimir el PID y el comando
                }
                fclose(fp); // Cerrar el archivo
            }
        }
    }

    closedir(proc_dir);
}

static const Command internal_commands[] = {
    {"cd", cmd_cd},
    {"exit", cmd_exit},
    {"help", cmd_help},
    {"kirby", cmd_kirby},
    {"cowsay", cmd_cowsay},
    {"pwd", cmd_pwd},
    {"echo", cmd_echo},
    {"ps", cmd_ps},
    {NULL, NULL}};

bool builtin_is_internal(scommand cmd)
{
    if (scommand_is_empty(cmd))
    {
        return false;
    }
    char *command = scommand_front(cmd);
    // buscamos si el comando ingresado está dentro de nuestro arreglo de comandos internos
    for (int i = 0; internal_commands[i].name != NULL; i++)
    {
        if (strcmp(command, internal_commands[i].name) == 0)
        {
            return true;
        }
    }
    return false;
}

bool builtin_alone(pipeline p)
{
    if (p == NULL)
    {
        return false;
    }
    return (pipeline_length(p) == 1) && builtin_is_internal(pipeline_front(p));
}

void builtin_run(scommand cmd)
{
    assert(builtin_is_internal(cmd));
    char *command = scommand_front(cmd);
    // ejecutamos la función del comando ingresado, en caso de que esté dentro de nuestro arreglo de comandos internos
    for (int i = 0; internal_commands[i].name != NULL; i++)
    {
        if (strcmp(command, internal_commands[i].name) == 0)
        {
            internal_commands[i].func(cmd);
            return;
        }
    }
    // caso en el que ingresamos un comando que no existe
    fprintf(stderr, "Command not found: %s\n", command);
}
