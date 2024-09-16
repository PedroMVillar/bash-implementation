/*
------------------------------------------------------------------------------------------------
  *    Módulo encargado de detectar que comando se quiso ejecutar y sugerir uno similar
  -- se agrega un parámetro de compilación en el makefile para poder usar la libreria math --
     -- la implementación se encuentra completamente encapsulada en el archivo syntax.c --
------------------------------------------------------------------------------------------------
*/

#ifndef SYNTAX_H
#define SYNTAX_H

void suggest_command(const char* command);

#endif
