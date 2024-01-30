/* This is a concatenation of headers of kerTeX_T/lib
   C) 2010 Thierry Laronde <tlaronde@polynym.com>
   All right reserved and no guarantees.
 */
/* Header of libpascal.
 */
#include <stdio.h>
#include <stdlib.h>

/* The types as macros. Some have default values that shall not be
   changed. Others can be adjusted for optimization depending on the
   target machine.
   For integer, real and glueration, the values are set (typedef) after
   configuration in "kertex.h".

   As K&R A8.2 specifies that "char" does not mean "signed char",
   if a char is unsigned, the parser writes "unsigned char". If
   the char is signed, the parser writes explicitely "signed char".
   There is hence no more macro for this. 
 */
#ifdef T_TETRA
#define integer T_TETRA
#else
#define integer long
#endif

/* Pascal ISO 74185:1990 specifies Boolean with a leading capital,
   while all the others have a leading minuscule. In the programs,
   there is the minuscule version. Hence the define to make Boolean
   an alias of boolean specified.
 */
#define boolean char
#define Boolean boolean

#ifdef true
#undef true
#endif
#define true '\1'

#ifdef false
#undef false
#endif
#define false '\0'

/* These ones are direct mapping.
 */
typedef FILE	*text, *file_ptr;
typedef unsigned char	*ccharpointer;

/* pp2rc(1) translates Pascal entry point "program" into main(). argc
 * and argv are defined as global variables, accessible for scanning
 * arguments.
 */
extern int argc;
extern char **argv;

/* macros. */
#define	chr(x)		(x)
#define get(f)		(void) getc(f)
#define	odd(x)		((x) % 2)
#define	ord(x)		(x)
#define pred(x)		((x) - 1)	/* !!! no checking */
#define	readln(f)	{register int c; while ((c=getc(f)) != '\n' && c != EOF);}
/* read() implementation is limited and ad hoc.
 */
#define	read(f, c)	c = getc(f)
#define read2(f,c1,c2)	c1 = getc(f); c2 = getc(f)
#define	rewrite(f,n)	f = openf((char *)n+1, "wb")
#define	reset(f,n)	if (f != NULL) {\
	fclose(f);\
	f = openf((char *)n+1, "rb");\
} else\
	f = openf((char *)n+1, "rb")

#define succ(x)		((x) + 1)	/* !!! no checking */
#define	trunc(x)	( (integer) (x) )	/* K&R A6.3 alinea 1 */
#define round(x)	( ((x) >= 0.0) ? trunc((x)+0.5) : trunc((x)-0.5) )

/* Prototypes. */

extern Boolean eof(FILE *);
extern Boolean eoln(FILE *);
extern FILE *openf(char *name,char *mode);	/* exit() on error */

/* Header for libweb.
 */
#include <string.h>

/* protect from not C89 names present in C89 headers and conflicting
   with Web routines (hence the leading 'w'
 */
#define strtonum strtonum_	/* reported on BSD */
#define remainder remainder_	/* reported on Mac */
#define getline getline_	/* reported on Mac: stdio.h:getline() */

/* The maximum length of a pathname including a directory specifier.
   Use C FILENAME_MAX (frequently 1024) but Pascal doesn't want `_'
   in identifiers (declared as constant in ppdef).
*/
#define	FILENAMEMAX	FILENAME_MAX
extern unsigned char nameoffile[];

/* PASCAL-H routines.
 */
#define hbreak(f) (void) fflush(f)
extern void hbreakin(FILE *stream, Boolean val);
extern void argfmt(integer n, unsigned char s[]);

/* Non Pascal or PASCAL-H, but WEB common.
 */
#define cexit(d) exit(d)
#define decr(x) --(x)
#define	fabs(x)		((x>=0.0)?(x):(-(x)))
#define incr(x) ++(x)
#define	toint(x)	((integer) (x))
