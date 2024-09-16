#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <assert.h>
#include <string.h>
#include "strextra.h"

#include "command.h"

struct scommand_s {
    GSList* args;
    char* in_redir;
    char* out_redir;
};



scommand scommand_new(void){
    scommand new_cmd = malloc (sizeof(struct scommand_s));
    assert(new_cmd != NULL);
    new_cmd->args = NULL;
    new_cmd->in_redir = NULL;
    new_cmd->out_redir = NULL;
    

    assert((new_cmd != NULL) && scommand_is_empty(new_cmd) && (scommand_get_redir_in(new_cmd) == NULL) && (scommand_get_redir_out (new_cmd) == NULL));
    return new_cmd;
}


scommand scommand_destroy(scommand self) {
    assert(self != NULL);

    g_slist_free_full(self->args, free);
    self->args = NULL;

    free(self->in_redir);
    self->in_redir = NULL; //Me aseguro de que apunten a NULL
    free(self->out_redir);
    self->out_redir = NULL;//Me aseguro de que apunten a NULL

    free(self);
    self = NULL; //Me aseguro de que apunten a NULL

    assert(self == NULL);

    return self;
}
void scommand_push_back(scommand self, char * argument){
    
    assert(self!=NULL && argument!=NULL);
    self->args = g_slist_append(self->args, argument); // Esta funcion agrega un elemento al final de la lista
    

}

void scommand_pop_front(scommand self){

    assert(self!=NULL && !scommand_is_empty(self));
    
    char* head = g_slist_nth_data(self->args, 0u); // Esta funcion devuelve el dato en la posicion 0 (front)
    self->args = g_slist_remove(self->args, head); // Esta funcion elimina el dato en la posicion a la que apunta head
    free(head); // Olvide liberar la memoria del dato eliminado, causante de los memory leaks en scommand
    
}

void scommand_set_redir_in(scommand self, char * filename){
    assert(self!=NULL);
    
    free(self->in_redir);
    self->in_redir = filename;
}

void scommand_set_redir_out(scommand self, char * filename){
    assert(self!=NULL);
    
    free(self->out_redir);
    self->out_redir = filename;
}

bool scommand_is_empty(const scommand self){
    assert(self!=NULL);
    return (self->args == NULL);
}

unsigned int scommand_length(const scommand self){
    assert(self!=NULL);
    return (g_slist_length(self->args)); // Esta funcion devuelve el largo de la lista
    //Notar que esta g_slist_length devuelve un guint, que es exactamente un unsigned int
}

char * scommand_front(const scommand self){
    
    assert(self!=NULL && !scommand_is_empty(self));
    
    return g_slist_nth_data(self->args, 0u);
}

char * scommand_get_redir_in(const scommand self){
    assert(self!=NULL);
    return self->in_redir;
}

char * scommand_get_redir_out(const scommand self){
    assert(self!=NULL);
    return self->out_redir;
}

char *scommand_to_string(const scommand self) {
    assert(self != NULL);
    GString *gstr = g_string_new(NULL); // Crea un nuevo string vacío
    
    // Tamaño de la lista a recorrer
    unsigned int n = g_slist_length(self->args);
    
    for (unsigned int i = 0; i < n; i++) {
        // dato en la pos i
        char *data = (char *)g_slist_nth_data(self->args, i);
        gstr = g_string_append(gstr, data);
        if (i < n - 1) {
            gstr = g_string_append_c(gstr, ' ');
        }
    }
    
    if (self->in_redir) {
        gstr = g_string_append(gstr, " < ");
        gstr = g_string_append(gstr, self->in_redir);
    }
    if (self->out_redir) {
        gstr = g_string_append(gstr, " > ");
        gstr = g_string_append(gstr, self->out_redir);
    }
    
    return g_string_free(gstr, FALSE); // FALSE indica que no se debe liberar la cadena resultante
}

struct pipeline_s{
    GSList *cmds; 
    bool fg; 
};


// Función auxiliar para liberar memoria de un scommand y que se pueda usar con g_slist_free_full ya que recibe
// un puntero a void y no a scommand (no podria usar scommand_destroy directamente)
// sin esta funcion g_slist_free_full no liberaba correctamente la memoria de los scommand, lo que causaba memory leaks en pipeline
static void scommand_free(void* self) {
    scommand aux = self; 
    scommand_destroy(aux);
}

pipeline pipeline_new(void){
    pipeline result = malloc (sizeof(struct pipeline_s));
    result->cmds = NULL;
    result->fg = true;
    assert(result != NULL && pipeline_is_empty(result) && pipeline_get_wait(result));
    return result;
}

pipeline pipeline_destroy(pipeline self) {
    assert(self != NULL);

    g_slist_free_full(self->cmds, scommand_free); // Utilizamos la funcion auxiliar
    self->cmds = NULL;
    free(self);
    self = NULL;

    assert(self == NULL);
    return self;
}

void pipeline_push_back(pipeline self, scommand sc){
    assert(self != NULL && sc != NULL);
    self->cmds = g_slist_append(self->cmds, sc);
}

void pipeline_pop_front(pipeline self) {
    assert(self != NULL && !pipeline_is_empty(self));

    scommand head = g_slist_nth_data(self->cmds, 0u);
    self->cmds = g_slist_remove(self->cmds, head);

    scommand_destroy(head);
    
}

void pipeline_set_wait(pipeline self, const bool w){
    assert(self != NULL);
    self->fg = w;
}

bool pipeline_is_empty(const pipeline self){
    assert(self != NULL);
    return (self->cmds == NULL);
}

unsigned int pipeline_length(const pipeline self){
    assert(self != NULL);
    return (g_slist_length(self->cmds));
}

scommand pipeline_front(const pipeline self){
    assert(self != NULL && !pipeline_is_empty(self));
    return g_slist_nth_data(self->cmds, 0u);
}

bool pipeline_get_wait(const pipeline self){
    assert(self != NULL);
    return self->fg;
}

char *pipeline_to_string(const pipeline self) {
    assert(self != NULL);

    // Verificar si el pipeline está vacío
    if (self->cmds == NULL) {
        return strdup("");  // Devuelve una cadena vacía
    }

    GString *gstr = g_string_new(NULL);
    
    // Tamaño de la lista a recorrer
    unsigned int n = g_slist_length(self->cmds);
    
    for (unsigned int i = 0; i < n; i++) {
        // dato en la pos i
        scommand cmd = (scommand)g_slist_nth_data(self->cmds, i);
        gstr = g_string_append(gstr, scommand_to_string(cmd));
        if (i < n - 1) {
            gstr = g_string_append(gstr, " | ");
        }
    }
    
    if (!self->fg) {
        gstr = g_string_append(gstr, " &");
    }
    
    return g_string_free(gstr, FALSE); // FALSE indica que no se debe liberar la cadena resultante
}
