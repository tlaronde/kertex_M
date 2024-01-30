/* Common header for the whole kerTeX package. */

#ifndef KERTEX_H
#define KERTEX_H

/* TYPEDEFS.  DON'T CHANGE! */

/* These types can be basically anything, so they don't need to put in
   site.h.  Despite the dire warning above, probably nothing bad will
   happen if you change them -- but you shouldn't need to.
 */
typedef unsigned char boolean;
typedef double real;

/* Character separator for pathnames on the system.
 */
#define PATHNAME_SEPARATOR '/'

/* The type `integer' must be a signed integer capable of holding at
   least the range of numbers (-2^31+1)..(2^31-1).  The ANSI C
   standard says that `long' meets this requirement, but if you don't
   have an ANSI C compiler, you might have to change this definition.
   Please note that this is used only with the KERTEX_M compiled
   binaries that run on the matrix. KERTEX_T will build the actual
   programs that run on the TARGET and this is done, then, differently.
 */
typedef long integer;

#endif /* KERTEX_H */
