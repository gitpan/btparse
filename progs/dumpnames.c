/* ------------------------------------------------------------------------
@NAME       : dumpnames.c
@INPUT      : 
@OUTPUT     : 
@RETURNS    : 
@DESCRIPTION: Parses a BibTeX file, splitting `author' and `editor' 
              fields into lists of names, and then splitting the
              individual names, and dumping everything.
@GLOBALS    : 
@CALLS      : 
@CALLERS    : 
@CREATED    : 1997/09/29, Greg Ward
@MODIFIED   : 
@VERSION    : $Id: dumpnames.c,v 1.1 1997/10/05 17:49:00 greg Rel $
-------------------------------------------------------------------------- */

#include <stdio.h>
#include "btparse.h"

char *Usage = "usage: dumpnames file\n";


void dump_component (char *comp, char **tokens, int num_tokens, char *tail)
{
   int  i;

   if (num_tokens > 0)
   {
      printf ("%s: ", comp);
      for (i = 0; i < num_tokens-1; i++)
      {
         printf ("%s/", tokens[i]);
      }
      printf ("%s%s", tokens[num_tokens-1], tail ? tail : "");
   }

} /* dump_component () */


void dump_names (AST * entry)
{
   AST *     field;
   char *    fname;
   char *    value;
   int       i;
   bt_stringlist *
             namelist;
   bt_name * name;
      

   if (bt_entry_metatype (entry) != BTE_REGULAR)
   {
      printf ("skipping %s entry\n", bt_entry_type (entry));
      return;
   }
   else
   {
      printf ("%s: %s\n", bt_entry_key (entry), bt_entry_type (entry));
   }

   field = NULL;
   while ((field = bt_next_field (entry, field, &fname)))
   {
      if (strcmp (fname, "author") == 0 ||
          strcmp (fname, "editor") == 0)
      {
         value = bt_get_text (field);

         printf ("field: %s:\n", fname);
         printf ("  %s\n", value);

         namelist = bt_split_list (value, "and", NULL, 0, "name");
         printf ("  splits into %d names:\n", namelist->num_items);
         for (i = 0; i < namelist->num_items; i++)
         {
            printf ("    %s\n", namelist->items[i]);

            name = bt_split_name (namelist->items[i], NULL, 0, i);
            printf ("      ");
            dump_component ("first", name->first, name->n_first, "; ");
            dump_component ("von", name->von, name->n_von, "; ");
            dump_component ("last", name->last, name->n_last, "; ");
            dump_component ("jr", name->jr, name->n_jr, NULL);
            printf ("\n");
         }
      }
   }

} /* dump_names () */


boolean process_file (char *filename)
{
   FILE *  infile;
   AST *   entry;
   boolean status, 
           overall_status;

   infile = fopen (filename, "r");
   if (infile == NULL)
   {
      perror (filename);
      return FALSE;
   }

   overall_status = TRUE;                  /* assume success */
   while (1)
   {
      entry = bt_parse_entry (infile, filename, 0, &status);
      overall_status &= status;
      if (!entry) break;
      dump_names (entry);
      bt_free_ast (entry);
   }
   
   return overall_status;

} /* process_file () */


int main (int argc, char **argv)
{
   char *  filename;
   boolean ok;
   
   if (argc != 2) 
   {
      fprintf (stderr, Usage);
      fprintf (stderr, "Wrong number of arguments\n");
      exit (1);
   }

   bt_initialize ();
   filename = argv[1];
   ok = process_file (filename);
   bt_cleanup ();
   exit (ok ? 0 : 1);
}
