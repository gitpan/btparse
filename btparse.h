/* ------------------------------------------------------------------------
@NAME       : btparse.h
@DESCRIPTION: Declarations and types for users of the btparse library.
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1997/01/19, Greg Ward
@MODIFIED   : 
@VERSION    : $Id: btparse.h,v 1.16 1997/09/10 02:02:07 greg Exp $
@COPYRIGHT  : Copyright (c) 1996-97 by Gregory P. Ward.  All rights reserved.

              This file is part of the btparse library.  This library is
              free software; you can redistribute it and/or modify it under
              the terms of the GNU Library General Public License as
              published by the Free Software Foundation; either version 2
              of the License, or (at your option) any later version.
-------------------------------------------------------------------------- */
#ifndef BTPARSE_H
#define BTPARSE_H

#include <sys/types.h>                  /* probably supplies 'ushort' */
#include <stdio.h>

/*
 * Every Unix variant I have access to (Linux, IRIX, SunOS) has a typedef
 * for 'ushort' in <sys/types.h>.  If this isn't true for some Unix, then
 * compilation will fail shortly (I use 'ushort' extensively in this 
 * header), and you'll have to adjust the #ifdef to reflect your
 * system.  *Please* tell me if you have to do so!
 */
#ifdef __unix
# define HAVE_USHORT
#endif


#ifndef HAVE_BOOLEAN
typedef int boolean;
#endif

#ifndef TRUE
# define TRUE 1
# define FALSE 0
#endif

/* Parsing (and post-processing) options */

#define BTO_DELQUOTES 1
#define BTO_EXPAND    2
#define BTO_PASTE     4
#define BTO_COLLAPSE  8

#define BTO_NOSTORE   16

#define BTO_FULL (BTO_DELQUOTES | BTO_EXPAND | BTO_PASTE | BTO_COLLAPSE)
#define BTO_MACRO (BTO_DELQUOTES | BTO_EXPAND | BTO_PASTE)
#define BTO_MINIMAL BTO_DELQUOTES

#define BTO_STRINGMASK (BTO_DELQUOTES | BTO_EXPAND | BTO_PASTE | BTO_COLLAPSE)

typedef enum
{
   BTE_UNKNOWN,
   BTE_REGULAR,
   BTE_COMMENT,
   BTE_PREAMBLE,
   BTE_MACRODEF
/*
   BTE_ALIAS,
   BTE_MODIFY
*/
} bt_metatype_t;

#define NUM_METATYPES ((int) BTE_MACRODEF + 1)

typedef enum 
{ 
   BTAST_BOGUS,                           /* to detect uninitialized nodes */
   BTAST_ENTRY,
   BTAST_KEY,
   BTAST_FIELD,
   BTAST_STRING,
   BTAST_NUMBER,
   BTAST_MACRO
} bt_nodetype_t;

typedef enum 
{
   E_NOTIFY,                    /* notification about next action */
   E_CONTENT,                   /* warning about the content of a record */
   E_STRUCTURAL,                /* warning about the structure of a record */
   E_LEXWARN,                   /* warning in lexical analysis */
   E_LEXERR,                    /* error in lexical analysis */
   E_SYNTAX,                    /* error in parser */
   E_FATAL,                     /* bigger problem (but still user's fault) */
   E_INTERNAL                   /* my fault */
} errclass_t;

#define NUM_ERRCLASSES ((int) E_INTERNAL + 1)

#define USER_DEFINED_AST 1

#define zzcr_ast(ast,attr,tok,txt)              \
{                                               \
   (ast)->line = (attr)->line;                  \
   (ast)->offset = (attr)->offset;              \
   (ast)->text = strdup ((attr)->text);         \
/* printf ("zzcr_ast: dup'd string = %p (%s)\n", (ast)->text, (ast)->text); */\
}

#define zzd_ast(ast)                            \
/* printf ("zzd_ast: free'ing ast node with string %p (%s)\n", \
           (ast)->text, (ast)->text); */ \
   if ((ast)->text != NULL) free ((ast)->text);


#ifdef USER_DEFINED_AST
typedef struct _ast 
{
   struct _ast *right, *down;
   int              line;
   int              offset;
   bt_nodetype_t    nodetype;
   bt_metatype_t    metatype;
   char *           text;
} AST;
#endif /* USER_DEFINED_AST */


#ifndef HAVE_USHORT                     /* needed for various bitmaps */
typedef unsigned short ushort;
#endif


typedef struct
{
   /* 
    * `string' is the string that has been split; items[0] ...
    * items[num_items-1] are pointers into `string', or NULL for empty
    * substrings.  Note that `string' is actually a copy of the string
    * passed in to bt_split_list() with NULs inserted between substrings.
    */

   char *  string;
   int     num_items;
   char ** items;
} bt_stringlist;

typedef struct
{
   bt_stringlist * tokens;
   char ** first;
   char ** von;
   char ** last;
   char ** jr;
   int     n_first, n_von, n_last, n_jr;
} bt_name;


typedef struct tex_tree_s
{
   char * start;
   int    len;
   struct tex_tree_s
        * child,
        * next;
} bt_tex_tree;


/* Function prototypes: */

/* init.c */
void  bt_initialize (void);
void  bt_free_ast (AST *ast);
void  bt_cleanup (void);

/* input.c */
void    bt_set_stringopts (bt_metatype_t metatype, ushort options);
AST * bt_parse_entry_s (char *    entry_text,
                        char *    filename,
                        int       line,
                        ushort    options,
                        boolean * status);
AST * bt_parse_entry   (FILE *    infile,
                        char *    filename,
                        ushort    options,
                        boolean * status);
AST * bt_parse_file    (char *    filename, 
                        ushort    options, 
                        boolean * overall_status);

/* error.c */
void  reset_error_counts (void);
int   get_error_count (errclass_t errclass);
int  *get_error_counts (int *counts);
ushort error_status (int *saved_counts);

/* traversal.c */
AST *bt_next_entry (AST *entry_list, AST *prev_entry);
bt_metatype_t bt_entry_metatype (AST *entry);
char *bt_entry_type (AST *entry);
char *bt_cite_key (AST *entry);
AST *bt_next_field (AST *entry, AST *prev, char **name);
AST *bt_next_macro (AST *entry, AST *prev, char **name);
AST *bt_next_value (AST *head, 
                    AST *prev,
                    bt_nodetype_t *nodetype,
                    char **text);
char *bt_get_text (AST *node);

/* names.c */
bt_stringlist * bt_split_list (char *   string,
                               char *   delim,
                               char *   filename,
                               int      line,
                               char *   description);
void bt_free_list (bt_stringlist *list);
bt_name * bt_split_name (char *  name,
                         char *  filename, 
                         int     line,
                         int     name_num);

/* tex_tree.c */
bt_tex_tree * bt_build_tex_tree (char * string);
void          bt_free_tex_tree (bt_tex_tree **top);
void          bt_dump_tex_tree (bt_tex_tree *node, int depth, FILE *stream);
char *        bt_flatten_tex_tree (bt_tex_tree *top);


#endif /* BTPARSE_H */