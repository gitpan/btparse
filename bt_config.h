/* ------------------------------------------------------------------------
@NAME       : bt_config.h
@DESCRIPTION: Site-specific options for the btparse library.
@GLOBALS    : 
@CREATED    : 1997/09/06, Greg Ward
@MODIFIED   : 
@VERSION    : $Id: bt_config.h 1.3 1997/09/07 14:12:50 greg Exp $
-------------------------------------------------------------------------- */

#ifndef CONFIG_H
#define CONFIG_H

/* 
 * strlwr() and strupr() generally seem to be available with MS-DOS
 * compilers, but I've never seen them on a Unix system.  I have no idea
 * about other Microsloth operating systems, 'though I suspect they would
 * be there.  Thus, this is a complete stab in the dark at guessing if we
 * need them (until I write a proper configure script, at least).
 */

#ifdef __MSDOS
# define HAVE_STRLWR
# define HAVE_STRUPR
#endif


/* 
 * The dmalloc library is handy for finding memory leaks and other malloc
 * errors; I use it in the development version of the library.  Here, we
 * just turn on DMALLOC_FUNC_CHECK (to do even more checking); we don't
 * include <dmalloc.h> until we get to my_dmalloc.h.  That's because
 * <dmalloc.h> is supposed to be the *last* header included, but I like to
 * make config.h the first.  Ugh.
 */

#ifdef DMALLOC
# define DMALLOC_FUNC_CHECK
# define strdup(str) xstrdup(str)
#endif

#endif /* CONFIG_H */
