/* ------------------------------------------------------------------------
@NAME       : prototypes.h
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Prototype declarations for functions from various places.
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1997/10/12, Greg Ward
@MODIFIED   : 
@VERSION    : $Id: prototypes.h,v 1.8 1997/09/06 22:57:22 greg Rel $
@COPYRIGHT  : Copyright (c) 1996-97 by Gregory P. Ward.  All rights reserved.

              This file is part of the btparse library.  This library is
              free software; you can redistribute it and/or modify it under
              the terms of the GNU Library General Public License as
              published by the Free Software Foundation; either version 2
              of the License, or (at your option) any later version.
-------------------------------------------------------------------------- */

#ifndef PROTOTYPES_H
#define PROTOTYPES_H

#include <stdio.h>
#include "btparse.h"                    /* for types */

/* util.c */
#if !HAVE_STRLWR
char *strlwr (char *s);
#endif
#if !HAVE_STRUPR
char *strupr (char *s);
#endif

/* parse_auxiliary.c */
void  fix_token_names (void);
void show_stack (char *label);           /* for debugging */

/* post_parse.c */
void  postprocess_string (char *s,
                          int   collapse_whitespace,
                          int   delete_quotes,
                          int   convert_quotes);
char *postprocess_value (AST *field, ushort options, int replace);
char *postprocess_field (AST *field, ushort options, int replace);
void  postprocess_entry (AST *top, ushort options);

/* macros.c */
void  init_macros (void);
void  done_macros (void);
void  add_macro (AST *macrodef, ushort options);
int   macro_length (char *macro);
char *macro_text (AST *macro_use);

/* bibtex_ast.c */
void dump_ast (char *msg, AST *root);

#endif /* PROTOTYPES_H */
