#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "parsing.h"
#include "parser.h"
#include "command.h"

bool flag_in = false;
bool flag_out = false;

void set_flag_in_true(){
    flag_in = true;
}

void set_flag_out_true(){
    flag_out = true;
}

/*
---------------------------------------------------------------------------------------------------------
*                      Función encargada de parsear un comando simple y devolverlo
 -- toma un parser como parámetro y devuelve un scommand, es estática ya que no se necesita por fuera --
---------------------------------------------------------------------------------------------------------
*/
static scommand parse_scommand(Parser p)
{
    scommand cmd = scommand_new();
    char *arg = NULL;
    arg_kind_t arg_type;

    // Parsear los argumentos del comando
    while ((arg = parser_next_argument(p, &arg_type)) != NULL)
    {
        // printf("Entra al while\n");
        // printf("Arg: %s\n", arg);
        if (arg_type == ARG_NORMAL)
        {
            scommand_push_back(cmd, arg);
        }
        else if (arg_type == ARG_INPUT)
        {
            scommand_set_redir_in(cmd, arg);
        }
        else if (arg_type == ARG_OUTPUT)
        {
            // printf("Redir out: %s\n", arg);
            scommand_set_redir_out(cmd, arg);
        }
        // Comentado porque terminaba liberando la memoria del comando ingresado
        // free(arg); // Liberar el argumento ya que scommand lo copia internamente
    }

    if(flag_in && (scommand_get_redir_in(cmd) == NULL)){
        printf("Error: Redirección de entrada sin archivo\n");
        scommand_destroy(cmd);
        cmd = scommand_new();
        flag_in = false;
        return cmd;
    }
    if(flag_out && (scommand_get_redir_out(cmd) == NULL)){
        printf("Error: Redirección de salida sin archivo\n");
        scommand_destroy(cmd);
        cmd = scommand_new();
        flag_out = false;
        return cmd;
    }

    // Verificar si el comando está vacío
    if (scommand_is_empty(cmd))
    {
        scommand_destroy(cmd);
        cmd = scommand_new();
    }

    return cmd;
}

/* ------------------------------------------------------------------------------------------------
*               Función encargada de parsear un pipeline completo y devolverlo
       -- toma un parser como parámetro y devuelve un pipeline, que se utilizará en el main --
------------------------------------------------------------------------------------------------ */

pipeline parse_pipeline(Parser p)
{
    pipeline result = pipeline_new();
    scommand cmd = NULL;
    bool error = false, another_pipe = true; // Indica si hay otro comando por parsear
    pipeline_set_wait(result, true);         // Establecer en true por defecto
    parser_skip_blanks(p);                   // Saltar blancos antes de empezar

    cmd = parse_scommand(p);
    error = (cmd == NULL); // Error si no se pudo parsear el primer comando

    while (another_pipe && !error)
    {
        if (scommand_is_empty(cmd))
        {
            break;
        }
        pipeline_push_back(result, cmd); // Agregar el comando al pipeline
        bool is_pipe = false;            // Indica si hay otro comando por parsear
        parser_skip_blanks(p);           // Saltar blancos antes de intentar leer un pipe
        parser_op_pipe(p, &is_pipe);     // Intentar leer un pipe
        if (is_pipe)
        {
            cmd = parse_scommand(p);
            error = (cmd == NULL); // Error si no se pudo parsear el siguiente comando
        }
        else
        {
            another_pipe = false;
        }
    }

    // Manejo del operador de background '&'
    bool is_background = false;
    parser_skip_blanks(p); // Saltar blancos antes de intentar leer un background
    parser_op_background(p, &is_background);
    if (is_background)
    {
        pipeline_set_wait(result, false);
    }

    // Consumir posibles blancos restantes y el \n final
    parser_skip_blanks(p);
    bool garbage = false;
    parser_garbage(p, &garbage);
    if (garbage)
    {
        pipeline_destroy(result);
        result = NULL;
    }

    if (error)
    {
        pipeline_destroy(result);
        result = NULL;
    }

    return result;
}
