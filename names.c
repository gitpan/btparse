/* ------------------------------------------------------------------------
@NAME       : names.c
@DESCRIPTION: Functions for dealing with BibTeX names and lists of names:
                bt_split_list 
                bt_split_name
@GLOBALS    : 
@CALLS      : 
@CREATED    : 1997/05/05, Greg Ward (as string_util.c)
@MODIFIED   : 1997/05/14-05/16, GW: added all the code to split individual 
                                    names, renamed file to names.c
@VERSION    : $Id: names.c,v 1.9 1997/10/01 12:56:16 greg Rel $
@COPYRIGHT  : Copyright (c) 1996-97 by Gregory P. Ward.  All rights reserved.

              This file is part of the btparse library.  This library is
              free software; you can redistribute it and/or modify it under
              the terms of the GNU Library General Public License as
              published by the Free Software Foundation; either version 2
              of the License, or (at your option) any later version.
-------------------------------------------------------------------------- */

#include "bt_config.h"

#ifdef __GNUC__                         /* this is from the Autoconf docs */
# define alloca __builtin_alloca
#else
# if HAVE_ALLOCA_H
#  include <alloca.h>
# else
#  ifdef _AIX
#   pragma alloca
#  else
#   ifndef alloca                       /* predefined by HP cc +Olibcalls */
char *alloca ();
#   endif
#  endif
# endif
#endif

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "btparse.h"
#include "prototypes.h"
#include "error.h"
#include "my_dmalloc.h"

#define DEBUG 0

#define update_depth(s,offs,depth)              \
switch (s[offs])                                \
{                                               \
   case '{': depth++; break;                    \
   case '}': depth--; break;                    \
}


/* ------------------------------------------------------------------------
@NAME       : bt_split_list()
@INPUT      : string - string to split up; whitespace must be collapsed
                       eg. by postprocess_string()
              delim  - delimiter to use; must be lowercase and should be
                       free of whitespace (code requires that delimiters
                       in string be surrounded by whitespace)
              filename - source of string (for warning messages)
              line     - 1-based line number into file (for warning messages)
              description - what substrings are (eg. "name") (for warning
                            messages); if NULL will use "substring"
@OUTPUT     : substrings (*substrings is allocated by bt_split_list() for you)
@RETURNS    : number of substrings found
@DESCRIPTION: Splits a string using a fixed delimiter, in the BibTeX way:
                * delimiters at beginning or end of string are ignored
                * delimiters in string must be surrounded by whitespace
                * case insensitive
                * delimiters at non-zero brace depth are ignored

              The list of substrings is returned as *substrings, which
              is an array of pointers into a duplicate of string.  This
              duplicate copy has been scribbled on such that there is 
              a nul byte at the end of every substring.  You should
              call bt_free_list() to free both the duplicate copy
              of string and *substrings itself.  Do *not* walk over
              the array free()'ing the substrings yourself, as this is
              invalid -- they were not malloc()'d!
@GLOBALS    : 
@CALLS      : 
@CALLERS    : anyone (exported by library)
@CREATED    : 1997/05/05, GPW
@MODIFIED   : 
-------------------------------------------------------------------------- */
bt_stringlist *
bt_split_list (char *   string,
               char *   delim,
               char *   filename,
               int      line,
               char *   description)
{
   int    depth;                        /* brace depth */
   int    i, j;                         /* offset into string and delim */
   int    inword;                       /* flag telling if prev. char == ws */
   int    string_len;
   int    delim_len;
   int    maxdiv;                       /* upper limit on no. of divisions */
   int    maxoffs;                      /* max offset of delim in string */
   int    numdiv;                       /* number of divisions */
   int *  start;                        /* start of each division */
   int *  stop;                         /* stop of each division */
   bt_stringlist *
          list;                         /* structure to return */

   if (description == NULL)
      description = "substring";

   string_len = strlen (string);
   delim_len = strlen (delim);
   maxdiv = (string_len / delim_len) + 1;
   maxoffs = string_len - delim_len + 1;

   start = (int *) alloca (maxdiv * sizeof (int));
   stop = (int *) alloca (maxdiv * sizeof (int));

   list = (bt_stringlist *) malloc (sizeof (bt_stringlist));

   depth = 0;
   i = j = 0;
   inword = 1;                          /* so leading delim ignored */
   numdiv = 0;
   start[0] = 0;                        /* first substring @ start of string */

   while (i < maxoffs)
   {
      /* does current char. in string match current char. in delim? */
      if (depth == 0 && !inword && tolower (string[i]) == delim[j])
      {
         j++; i++;

         /* have we found an entire delim, followed by a space? */
         if (j == delim_len && string[i] == ' ')
         {

            stop[numdiv] = i - delim_len - 1;
            start[++numdiv] = ++i;
            j = 0;

#if DEBUG
            printf ("found complete delim; i == %d, numdiv == %d: "
                    "stop[%d] == %d, start[%d] == %d\n",
                    i, numdiv, 
                    numdiv-1, stop[numdiv-1],
                    numdiv, start[numdiv]);
#endif
         }
      }
      
      /* no match between string and delim, at non-zero depth, or in a word */
      else
      {
         update_depth (string, i, depth);
         inword = (i < string_len) && (string[i] != ' ');
         i++;
         j = 0;
      }
   }

   stop[numdiv] = string_len;           /* last substring ends just past eos */
   list->num_items = numdiv+1;


   /* 
    * OK, now we know how many divisions there are and where they are --
    * so let's split that string up for real!
    * 
    * list->items will be an array of pointers into a duplicate of
    * `string'; we duplicate `string' so we can safely scribble on it and
    * free() it later (in bt_free_list()).
    */
       
   list->items = (char **) malloc (list->num_items * sizeof (char *));
   list->string = strdup (string);

   for (i = 0; i < list->num_items; i++)
   {
      /* 
       * Possible cases:
       *   - stop < start is for empty elements, e.g. "and and" seen in
       *     input.  (`start' for empty element will be the 'a' of the
       *     second 'and', and its stop will be the ' ' *before* the
       *     second 'and'.)
       *   - stop > start is for anything else between two and's (the usual)
       *   - stop == start should never happen if the loop above is correct 
       */

      if (stop[i] > start[i])           /* the usual case */
      {
         list->string[stop[i]] = 0;
         list->items[i] = list->string+start[i];
      }
      else if (stop[i] < start[i])      /* empty element */
      {
         list->items[i] = NULL;
         name_warning (filename, line, "%s %d is empty", 
                       description, i+1);
      }
      else                              /* should not happen! */
      {
         internal_error ("stop == start for substring %d", i);
      }
   }

   return list;
/*    return num_substrings; */

} /* bt_split_list () */


/* ------------------------------------------------------------------------
@NAME       : bt_free_list()
@INPUT      : list
              num_items
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Frees the list of strings created by bt_split_list().
@GLOBALS    : 
@CALLS      : 
@CALLERS    : anyone (exported by library)
@CREATED    : 1997/05/06, GPW
@MODIFIED   : 
-------------------------------------------------------------------------- */
void bt_free_list (bt_stringlist *list)
{
   free (list->string);
   free (list->items);
   free (list);
}



/* ----------------------------------------------------------------------
 * Stuff for splitting up a single name
 */

static char *Filename = NULL;           /* set by split_name() and used */
static int   Line = 0;                  /* by others for warning messages */
static int   NameNum = -1;


/* ------------------------------------------------------------------------
@NAME       : find_commas
@INPUT      : name       - string to search for commas
              max_commas - maximum number of commas to allow (if more than 
                           this number are seen, a warning is printed and
                           the excess commas are removed)
@OUTPUT     : commas     - array of offsets into name -- will be filled with
                           positions of commas (caller must allocate with
                           max possible number of commas before calling)
@RETURNS    : number of commas found
@DESCRIPTION: Counts and records positions of commas at brace-depth 0.
              Modifies string in-place to remove whitespace around commas,
              excess commas, and any trailing commas; warns on excess or
              trailing commas.  Excess commas are removed by replacing them
              with space and calling postprocess_string() to collapse 
              whitespace a second time; trailing commas are simply replaced
              with (char) 0 to truncate the string.

              Assumes whitespace has been collapsed (ie. no space at
              beginning or end of string, and all internal strings of
              whitespace reduced to exactly one space).

              Hint: max possible number of commas is just strlen(name)
              (eg. ",,,,,,").
@GLOBALS    : Filename, Line, NameNum (for warning message)
@CALLS      : name_warning() (if too many commas, or commas at end)
@CALLERS    : bt_split_name()
@CREATED    : 1997/05/14, Greg Ward
@MODIFIED   : 
-------------------------------------------------------------------------- */
static int 
find_commas (char *name, int max_commas, int *commas)
{
   int    i, j;
   int    depth;
   int    num_commas;
   int    len;
   int    at_comma;
   int    warned;

   i = j = 0;
   depth = 0;
   num_commas = 0;
   len = strlen (name);
   warned = 0;

   /* First pass to check for and blank out excess commas */

   for (i = 0; i < len; i++)
   {
      if (depth == 0 && name[i] == ',')
      {
         num_commas++;
         if (num_commas > max_commas)
         {
            if (! warned)
            {
               name_warning (Filename, Line, "name %d: "
                             "too many commas in name (removing extras)", 
                             NameNum+1);
               warned = 1;
            }
            name[i] = ' ';
         }
      }
   }

   /* 
    * If we blanked out a comma, better re-collapse whitespace.  (This is
    * a bit of a cop-out -- I could probably adjust i and j appropriately
    * in the above loop to do the collapsing for me, but my brain
    * hurt when I tried to think it through.  Some other time, perhaps.
    */

   if (warned)
      postprocess_string (name, 1, 0, 0);

   /* Now the real comma-finding loop (only if necessary) */

   if (num_commas == 0)
      return 0;
   
   num_commas = 0;
   i = 0;
   while (i < len)
   {
      at_comma = (depth == 0 && name[i] == ',');
      if (at_comma)
      {
         while (j > 0 && name[j-1] == ' ') j--;
         commas[num_commas++] = j;
      }

      update_depth (name, i, depth);
      if (i != j)
         name[j] = name[i];

      i++; j++;
      if (at_comma)
      {
         while (i < len && name[i] == ' ') i++;
      }
   } /* while i */

   if (i != j) name[j] = (char) 0;
   j--;

   if (name[j] == ',') 
   {
      name_warning (Filename, Line, "name %d: "
                    "comma(s) at end of name (removing)", NameNum+1);
      while (name[j] == ',')
      {
         name[j--] = (char) 0;
         num_commas--;
      }
   }

   return num_commas;

} /* find_commas() */


/* ------------------------------------------------------------------------
@NAME       : find_tokens
@INPUT      : name       - string to tokenize
              commas     - offsets of commas in name
@OUTPUT     : comma_token- number of token immediately preceding each comma
                           (again, caller must allocate)
@RETURNS    : number of tokens
@DESCRIPTION: Finds tokens in a string; delimiter is space or comma at
              brace-depth zero.  Assumes whitespace has been collapsed
              and find_commas has been run on the string to remove
              whitespace around commas and any trailing commas.
@GLOBALS    : 
@CALLS      : 
@CALLERS    : bt_split_name()
@CREATED    : 1997/05/14, Greg Ward
@MODIFIED   : 
-------------------------------------------------------------------------- */
static bt_stringlist *
find_tokens (char *  name,
             int *   comma_token)
{
   int    i;                            /* index into name */
   int    num_tok;
   int    in_boundary;                  /* previous char was ' ' or ',' */
   int    cur_comma;                    /* index into comma_token */
   int    len;
   int    depth;
   bt_stringlist *
          tokens;

   i = 0;
   in_boundary = 1;                     /* so first char will start a token */
   cur_comma = 0;
   len = strlen (name);
   depth = 0;

   tokens = (bt_stringlist *) malloc (sizeof (bt_stringlist));
   tokens->string = name;
   num_tok = 0;
   tokens->items = NULL;

   if (len == 0)                        /* empty string? */
      return tokens;                    /* return empty token list */

   tokens->items = (char **) malloc (sizeof (char *) * len);

   while (i < len)
   {
      if (depth == 0 && in_boundary)    /* at start of a new token */
      {
         tokens->items[num_tok++] = name+i;
      }

      if (depth == 0 && (name[i] == ' ' || name[i] == ','))
      {
         /* if we're at a comma, record the token preceding the comma */

         if (name[i] == ',')
         {
            comma_token[cur_comma++] = num_tok-1;
         }

         /* 
          * if already in a boundary zone, we have an empty token
          * (caused by multiple consecutive commas)
          */
         if (in_boundary)
         {
            tokens->items[num_tok-1] = NULL;
         }

         /* in any case, mark the end of one token and prepare for the 
          * start of the next
          */
         name[i] = (char) 0;
         in_boundary = 1;
      }
      else
      {
         in_boundary = 0;               /* inside a token */
      }

      update_depth (name, i, depth);
      i++;

   } /* while i */

   tokens->num_items = num_tok;
   return tokens;

} /* find_tokens() */


/* ------------------------------------------------------------------------
@NAME       : find_lc_tokens()
@INPUT      : tokens
@OUTPUT     : first_lc
              last_lc
@RETURNS    : 
@DESCRIPTION: Finds the first contiguous string of lowercase tokens in
              `name'.  The string must already be tokenized by
              find_tokens(), and the input args num_tok, tok_start, and
              tok_stop are the return value and the two same-named output
              arguments from find_tokens().
@GLOBALS    : 
@CALLS      : 
@CALLERS    : bt_split_name()
@CREATED    : 1997/05/14, Greg Ward
@MODIFIED   : 
-------------------------------------------------------------------------- */
static void
find_lc_tokens (bt_stringlist * tokens,
                int *           first_lc,
                int *           last_lc)
{
   int    i;                            /* iterate over token list this time */
   int    in_lc_sequence;               /* in contig. sequence of lc tokens? */

   *first_lc = *last_lc = -1;           /* haven't found either yet */
   in_lc_sequence = 0;

   i = 0;
   while (i < tokens->num_items)
   {
      if (*first_lc == -1 && islower (tokens->items[i][0]))
      {
         *first_lc = i;

         i++;
         while (i < tokens->num_items && islower (tokens->items[i][0]))
            i++;

         *last_lc = i-1;
      }
      else
      {
         i++;
      }
   }
} /* find_lc_tokens() */


/* ------------------------------------------------------------------------
@NAME       : resolve_token_range()
@INPUT      : tokens     - structure containing the token list
              tok_range  - two-element array with start and stop token number
@OUTPUT     : *component - set to point to first token in range, or NULL
                           if empty range
              *num_tok   - number of tokens in the range
@RETURNS    : 
@DESCRIPTION: Given a list of tokens and a range of token numbers (as a 
              two-element array, tok_range), computes the number of tokens 
              in the range.  If this is >= 0, sets *component to point
              to the first token in the range; otherwise, sets *component
              to NULL.
@CALLERS    : 
@CREATED    : May 1997, GPW
@MODIFIED   : 
-------------------------------------------------------------------------- */
static void
resolve_token_range (bt_stringlist *tokens,
                     int *          tok_range,
                     char ***       component,
                     int *          num_tok)
{
   *num_tok = (tok_range[1] - tok_range[0]) + 1;
   if (*num_tok <= 0)
   {
      *num_tok = 0;
      *component = NULL;
   }
   else
   {
      *component = tokens->items + tok_range[0];
   }   
} /* resolve_token_range() */


/* ------------------------------------------------------------------------
@NAME       : split_simple_name()
@INPUT      : name
              first_lc
              last_lc
@OUTPUT     : name
@RETURNS    : 
@DESCRIPTION: Splits up a name (represented as a string divided into
              non-overlapping, whitespace-separated tokens) according
              to the BibTeX rules for names without commas.  Specifically:
                * tokens up to (but not including) the first lowercase
                  token, or the last token of the string if there 
                  are no lowercase tokens, become the `first' component
                * the earliest contiguous sequence of lowercase tokens,
                  up to (but not including) the last token of the string,
                  becomes the `von' component
                * the tokens following the von component, or the last
                  single token if there is no von component, become
                  the `last' component
                * there is no `jr' component
@GLOBALS    : Filename, Line, NameNum (for warning message)
@CALLS      : name_warning() (if last lc token taken as lastname)
              resolve_token_range()
@CALLERS    : bt_split_name()
@CREATED    : 1997/05/15, Greg Ward
@MODIFIED   : 
-------------------------------------------------------------------------- */
static void
split_simple_name (bt_name * name,
                   int       first_lc,
                   int       last_lc)
{
   int   first_t[2], von_t[2], last_t[2];
   int   end;

   end = name->tokens->num_items-1;     /* token number of last token */

   if (first_lc > -1)                   /* any lowercase tokens at all? */
   {
      first_t[0] = 0;                   /* first name goes from beginning */
      first_t[1] = first_lc-1;          /* to just before first lc token */

      if (last_lc == end)               /* sequence of lowercase tokens */
      {                                 /* goes all the way to end of string */
         last_lc--;                     /* -- roll it back by one so we */
                                        /* still have a lastname */
         name_warning (Filename, Line, "name %d: "
                       "no capitalized token at end of name; "
                       "using \"%s\" as lastname",
                       NameNum+1,
                       name->tokens->items[end]);
      }

      von_t[0] = first_lc;              /* von component covers sequence of */
      von_t[1] = last_lc;               /* lowercase tokens */
      last_t[0] = last_lc+1;            /* lastname from after von to end */
      last_t[1] = end;                  /* of string */
   }
   else                                 /* no lowercase tokens */
   {
      von_t[0] = 0;                     /* empty von component */
      von_t[1] = -1;
      first_t[0] = 0;                   /* first goes from first to second */
      first_t[1] = end-1;               /* last token */
      last_t[0] = last_t[1] = end;      /* and last is just the last token */
   }

   resolve_token_range (name->tokens, first_t, &name->first, &name->n_first);
   resolve_token_range (name->tokens, von_t, &name->von, &name->n_von);
   resolve_token_range (name->tokens, last_t, &name->last, &name->n_last);
   name->n_jr = 0;                      /* no jr component possible */
   name->jr = NULL;

} /* split_simple_name() */


/* ------------------------------------------------------------------------
@NAME       : split_general_name()
@INPUT      : name
              num_commas
              comma_token
              first_lc
              last_lc
@OUTPUT     : name
@RETURNS    : 
@DESCRIPTION: Splits a name according to the BibTeX rules for names 
              with 1 or 2 commas (> 2 commas is handled elsewhere,
              namely by bt_split_name() calling find_commas() with
              max_commas == 2).  Specifically:
                * an initial string of lowercase tokens, up to (but not 
                  including) the token before the first comma, becomes
                  the `von' component
                * tokens from immediately after the `von' component, 
                  or from the beginning of the string if no `von',
                  up to the first comma become the `last' component

              if one comma:
                * all tokens following the sole comma become the 
                  `first' component

              if two commas:
                * tokens between the two commas become the `jr' component
                * all tokens following the second comma become the 
                  `first' component
@GLOBALS    : Filename, Line, NameNum (for warning message)
@CALLS      : name_warning() (if last lc token taken as lastname)
              resolve_token_range()
@CALLERS    : bt_split_name()
@CREATED    : 1997/05/15, Greg Ward
@MODIFIED   : 
-------------------------------------------------------------------------- */
static void
split_general_name (bt_name *  name,
                    int        num_commas,
                    int *      comma_token,
                    int        first_lc,
                    int        last_lc)
{
   int   first_t[2], von_t[2], last_t[2], jr_t[2];
   int   end;

   end = name->tokens->num_items-1;     /* last token number */

   if (first_lc == 0)                   /* we have an initial string of */
   {                                    /* lowercase tokens */
      if (last_lc == comma_token[0])    /* lc string ends at first comma */
      {
         name_warning (Filename, Line, "name %d: "
                       "no capitalized tokens before first comma",
                       NameNum+1);
         last_lc--;
      }
      
      von_t[0] = first_lc;              /* von covers the sequence of */
      von_t[1] = last_lc;               /* lowercase tokens */
   }
   else                                 /* no lowercase tokens at start */
   {
      von_t[0] = 0;                     /* empty von component */
      von_t[1] = -1;
   }

   last_t[0] = von_t[1] + 1;            /* start right after end of von */
   last_t[1] = comma_token[0];          /* and end at first comma */

   if (num_commas == 1)
   {
      first_t[0] = comma_token[0]+1;    /* start right after comma */
      first_t[1] = end;                 /* stop at end of string */
      jr_t[0] = 0;                      /* empty jr component */
      jr_t[1] = -1;
   }
   else                                 /* more than 1 comma */
   {
      jr_t[0] = comma_token[0]+1;       /* start after first comma */
      jr_t[1] = comma_token[1];         /* stop at second comma */
      first_t[0] = comma_token[1]+1;    /* start after second comma */
      first_t[1] = end;                 /* and go to end */
   }

   resolve_token_range (name->tokens, first_t, &name->first, &name->n_first);
   resolve_token_range (name->tokens, von_t, &name->von, &name->n_von);
   resolve_token_range (name->tokens, last_t, &name->last, &name->n_last);
   resolve_token_range (name->tokens, jr_t, &name->jr, &name->n_jr);

} /* split_general_name() */


/* ------------------------------------------------------------------------
@NAME       : bt_split_name()
@INPUT      : name
              filename
              line
              name_num
@OUTPUT     : name       - modified in place (excess and trailing commas
                           removed, whitespace around commas stripped,
                           NULs inserted between tokens)
@RETURNS    : bt_name structure containing the four components as token-lists
@DESCRIPTION: Splits a name according to the BibTeX rules.  There are 
              actually two sets of rules: one for names with no commas,
              and one for names with 1 or 2 commas.  (If a name has 
              more than 2 commas, the extras are removed and it's treated
              as though it had just the first 2.)

              See split_simple_name() for the no-comma rules, and
              split_general_name() for the 1-or-2-commas rules.
@GLOBALS    : 
@CALLS      : 
@CALLERS    : anyone (exported by library)
@CREATED    : 1997/05/14, Greg Ward
@MODIFIED   : 
-------------------------------------------------------------------------- */
bt_name *
bt_split_name (char *  name,
               char *  filename,
               int     line,
               int     name_num)
{
   int *  commas;
   bt_stringlist *
          tokens;
   int *  comma_token;
   int    len;
   int    max_commas;
   int    num_commas;
   int    first_lc, last_lc;
   bt_name * split_name;
#if DEBUG
   int    i;
#endif

   split_name = (bt_name *) malloc (sizeof (bt_name));

#define EMPTY(n,c) n->c = NULL; n->n_##c = 0

   if (name == NULL)                    /* non-existent string? */
   {
      EMPTY (split_name, first);
      EMPTY (split_name, von);
      EMPTY (split_name, last);
      EMPTY (split_name, jr);
      return split_name;
   }

   Filename = filename;                 /* so called functions can generate */
   Line = line;                         /* decent warning messages */
   NameNum = name_num;

   len = strlen (name);

   /* worst case for num_commas is ",,,,," -> num_commas = length of string */
   max_commas = len;

   commas = (int *) alloca (sizeof (int) * max_commas);

#if DEBUG
   printf ("name = >%s<\n", name);
#endif

   num_commas = find_commas (name, 2, commas);
   assert (num_commas <= max_commas);

#if DEBUG
   printf ("found %d commas: ", num_commas);
   for (i = 0; i < num_commas; i++)
      printf ("%d ", commas[i]);
   printf ("\nnow name = >%s<\n", name);
#endif

   if (num_commas > 0)
      comma_token = (int *) alloca (sizeof (int) * num_commas);
   else
      comma_token = NULL;
   tokens = find_tokens (name, comma_token);

#if DEBUG
   printf ("found %d tokens:\n", tokens->num_items);
   for (i = 0; i < tokens->num_items; i++)
   {
      printf ("  %d: ", i);

      if (tokens->items[i])             /* non-empty token? */
      {
         printf (">%s<\n", tokens->items[i]);
      }
      else 
      {
         printf ("(empty)\n");
      }
   }
#endif

#if DEBUG
   printf ("comma tokens: ");
   for (i = 0; i < num_commas; i++)
      printf ("%d ", comma_token[i]);
   printf ("\n");
#endif

   find_lc_tokens (tokens, &first_lc, &last_lc);
#if DEBUG
   printf ("(first,last) lc tokens = (%d,%d)\n", first_lc, last_lc);
#endif

   if (strlen (name) == 0)              /* name now empty? */
   {
      EMPTY (split_name, first);
      EMPTY (split_name, von);
      EMPTY (split_name, last);
      EMPTY (split_name, jr);
   }
   else
   {
      split_name->tokens = tokens;
      if (num_commas == 0)              /* no commas -- "simple" format */
      {
         split_simple_name (split_name, 
                            first_lc, last_lc);
      }
      else
      {
         split_general_name (split_name,
                             num_commas, comma_token,
                             first_lc, last_lc);
      }
   }

   return split_name;
}
