/* ------------------------------------------------------------------------
@NAME       : macros.c
@DESCRIPTION: Front-end to the standard PCCTS symbol table code (sym.c)
              to abstract my "macro table".
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1997/01/12, Greg Ward
@MODIFIED   : 
@VERSION    : $Id: macros.c,v 1.10 1997/09/10 02:02:07 greg Exp $
@COPYRIGHT  : Copyright (c) 1996-97 by Gregory P. Ward.  All rights reserved.

              This file is part of the btparse library.  This library is
              free software; you can redistribute it and/or modify it under
              the terms of the GNU Library General Public License as
              published by the Free Software Foundation; either version 2
              of the License, or (at your option) any later version.
-------------------------------------------------------------------------- */
#include "bt_config.h"
#include <stdlib.h>
#include <string.h>
#include "sym.h"
#include "prototypes.h"
#include "error.h"
#include "my_dmalloc.h"

#define DEBUG 0
#define NUM_MACROS 547
#define STRING_SIZE 4096

Sym *AllMacros = NULL;                  /* `scope' so we can get back list */
                                        /* of all macros when done */


void init_macros (void)
{
   zzs_init (NUM_MACROS, STRING_SIZE);
   zzs_scope (&AllMacros);
}


void done_macros (void)
{
   Sym  *cur, *next;

   /* 
    * Use the current `scope' (same one for all macros) to get access to
    * a linked list of all macros.  Then traverse the list, free()'ing
    * both the text (which was strdup()'d in add_macro(), below) and 
    * the records themselves (which are calloc()'d by zzs_new()).
    */

   cur = zzs_rmscope (&AllMacros);
   while (cur != NULL)
   {
#if DEBUG > 1
      printf ("done_macros(): freeing macro \"%s\" (%p=\"%s\") at %p\n",
              cur->symbol, cur->text, cur->text, cur);
#endif

      next = cur->scope;
      if (cur->text != NULL) free (cur->text);
      free (cur);
      cur = next;
   }
   
   zzs_done ();
}


void add_macro (AST *assignment, ushort options)
{
   Sym   *new_rec;
   AST   *value;
   char  *macro, *text;

   if (assignment == NULL || assignment->down == NULL) return;
   value = assignment->down;

   /* 
    * If the options that were used to process the macro's expansion text 
    * are anything other than BTO_MACRO, then we'll have to do it ourselves.
    */

   if ((options & BTO_STRINGMASK) != BTO_MACRO)
   {
      text = postprocess_field (assignment, BTO_MACRO, FALSE);
   }
   else
   {
      /* 
       * First a sanity check to make sure that the presumed post-processing
       * had the desired effect.
       */

      int len = strlen (value->text);

      if (value->nodetype != BTAST_STRING || value->right != NULL ||
          value->text[0] == '"' || value->text[0] == '{' ||
          value->text[len-1] == '"' || value->text[len-1] == '}')
      {
         internal_error ("add_macro: macro value was not " 
                         "correctly preprocessed");
      }

      text = assignment->down->text;
      if (text != NULL)
         text = strdup (text);          /* so the AST and macro hash can */
                                        /* free() the text independently */
   }

   macro = assignment->text;

#if DEBUG == 1
   printf ("adding macro \"%s\" = \"%s\"\n", macro, text);
#elif DEBUG > 1
   printf ("add_macro: macro = %p (%s)\n"
           "            text = %p (%s)\n",
           macro, macro, text, text);
#endif

   if (zzs_get (macro))
   {
      content_warning (assignment, 
                       "overriding existing definition of macro \"%s\"", 
                       macro);
   }

   new_rec = zzs_newadd (macro);
   new_rec->text = text;
#if DEBUG > 1
   printf ("           saved = %p (%s)\n", new_rec->text, new_rec->text);
#endif
}


int macro_length (char *macro)
{
   Sym   *sym;

#if DEBUG > 1
   printf ("macro_length: looking up \"%s\"\n", macro);
#endif

   sym = zzs_get (macro);
   if (sym)
      return strlen (sym->text);
   else
      return 0;   
}


char *macro_text (AST *macro_use)
{
   char  *macro;
   Sym   *sym;

   macro = macro_use->text;
   sym = zzs_get (macro);
   if (!sym)
   {
      content_warning (macro_use, "undefined macro \"%s\"", macro);
      return NULL;
   }

   return sym->text;
}
