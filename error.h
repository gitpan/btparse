/* ------------------------------------------------------------------------
@NAME       : error.c
@DESCRIPTION: Prototypes for the error-generating functions (i.e. functions
              defined in error.c, and meant only for use elswhere in the
              library).
@CREATED    : Summer 1996, Greg Ward
@MODIFIED   : 
@VERSION    : $Id: error.h,v 1.9 1997/09/26 13:36:25 greg Rel $
@COPYRIGHT  : Copyright (c) 1996-97 by Gregory P. Ward.  All rights reserved.

              This file is part of the btparse library.  This library is
              free software; you can redistribute it and/or modify it under
              the terms of the GNU Library General Public License as
              published by the Free Software Foundation; either version 2
              of the License, or (at your option) any later version.
-------------------------------------------------------------------------- */

#ifndef ERROR_H
#define ERROR_H

#include "btparse.h"                    /* for AST typedef */

#define MAX_ERROR 1024

/*
 * Prototypes for functions exported by error.c but only used within
 * the library -- functions that can be called by outsiders are declared
 * in btparse.h.
 */

void notify (char *format,...);
void content_warning (AST * ast, char *format,...);
void name_warning (char *filename, int line, char *format, ...);
void structural_warning (AST * ast, char *format,...);
void lexical_warning (char *format,...);
void usage_warning (char *format,...);
void lexical_error (char *format,...);
void syntax_error (char *format,...);
void usage_error (char *format,...);
void internal_error (char *format,...);

#endif
