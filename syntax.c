#include <math.h>  // permite usar fmin()
#include <stdio.h> // permite usar printf()
#include <stdlib.h>
#include <string.h> // permite usar strlen()
#include <limits.h> // permite usar INT_MAX

#include "syntax.h"

#define MAX_COMMAND_LENGTH 100 // Longitud máxima de un comando
#define INITIAL_CAPACITY 10    // Tamaño inicial del array dinámico de comandos

/*
------------------------------------------------------------------------------------------------
  *    Función encargada de buscar la distancia mínima de edición entre dos cadenas
  -- toma como parámetros dos cadenas de texto y devuelve un entero con la distancia mínima --
             -- es estática ya que no se necesita fuera de este archivo --
           LA EXPLICACIÓN DETALLADA DE ESTA IMPLEMENTACIÓN ESTÁ EN EL README
------------------------------------------------------------------------------------------------
*/
static int distance_of_edition(const char *s1, const char *s2)
{
    // OBTENCIÓN DE LA LONGITUD DE LAS CADENAS
    int len_s1 = strlen(s1);
    int len_s2 = strlen(s2);
    // CREACIÓN DE LA MATRIZ DE DISTANCIAS (DP)
    int dp[len_s1 + 1][len_s2 + 1];
    // CASOS BASE: LLENADO DE LA PRIMERA FILA Y COLUMNA
    for (int y = 0; y <= len_s1; y++)
    {
        dp[y][0] = y;
    }
    for (int x = 0; x <= len_s2; x++)
    {
        dp[0][x] = x;
    }
    // PASO RECURSIVO: LLENADO DE LA MATRIZ
    for (int y = 1; y <= len_s1; y++)
    {
        for (int x = 1; x <= len_s2; x++)
        {
            /* Si los caracteres coinciden, no hay penalización */
            if (s1[y - 1] == s2[x - 1])
            {
                dp[y][x] = dp[y - 1][x - 1];
            }
            /* Si no coinciden, aplicar penalización y tomar el mínimo entre las tres operaciones posibles */
            else
            {
                dp[y][x] = 1 + fmin(fmin(dp[y - 1][x],  // Eliminación
                                         dp[y][x - 1]), // Inserción
                                    dp[y - 1][x - 1]);  // Sustitución
            }
        }
    }
    /* --- El resultado final es el valor de la esquina inferior derecha de la matriz --- */
    return dp[len_s1][len_s2];
}

/*
------------------------------------------------------------------------------------------------
*              Función encargada de sugerir un comando similar al ingresado
  -- toma simplemente un string como parámetro y no devuelve nada, solo imprime en consola --
------------------------------------------------------------------------------------------------
*/
void suggest_command(const char *command)
{
    /* ------------------------------------------ */
    /* --- CARGA DE COMANDOS DESDE UN ARCHIVO --- */
    /* ------------------------------------------ */
    const char *filename = "commands.in"; // Nombre del archivo
    FILE *file = fopen(filename, "r");    // Abrir el archivo
    if (file == NULL)
    {
        perror("Could not open the file");
        exit(EXIT_FAILURE);
    }
    // Inicializar el array dinámico para los comandos
    int capacity = INITIAL_CAPACITY;
    char **valid_commands = malloc(capacity * sizeof(char *));
    int commands_count = 0;
    char buffer[MAX_COMMAND_LENGTH]; // Buffer para almacenar cada comando leído
    // Leer comandos separados por espacios
    while (fscanf(file, "%s", buffer) != EOF)
    {
        // Redimensionar para evitar desbordamiento
        if (commands_count >= capacity)
        {
            capacity *= 2;
            valid_commands = realloc(valid_commands, capacity * sizeof(char *));
        }
        // Copiar el comando leído al array
        valid_commands[commands_count] = strdup(buffer);
        commands_count++;
    }
    fclose(file);
    /* ------------------------------------------ */
    /* ---- BÚSQUEDA DEL COMANDO MÁS CERCANO ---- */
    /* ------------------------------------------ */
    int min_distance = INT_MAX;
    char *suggestion = NULL;
    for (int i = 0; i < commands_count; i++)
    {
        int dist = distance_of_edition(command, valid_commands[i]);
        if (dist < min_distance)
        {
            min_distance = dist;
            suggestion = valid_commands[i];
        }
    }
    // Decidir si sugerir el comando encontrado
    if (min_distance <= 2)
    {
        printf("Did you mean %s?\n", suggestion);
    }
    else
    {
        printf("Suggested command: %s\n", suggestion);
    }
    // Limpiar la memoria utilizada por los comandos
    for (int i = 0; i < commands_count; i++)
    {
        free(valid_commands[i]);
    }
    free(valid_commands);
}
