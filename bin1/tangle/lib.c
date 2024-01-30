/* Concatenation of routines by tangle, from kerTeX_T/lib.
   C) 2010 Thierry Laronde <tlaronde@polynym.com>
   All right reserved and no guarantees.

   These routines were extracted and sometimes adjusted from the 
   various web2c V5.0C files.
 */
#include <string.h>
#include "web.h"

unsigned char nameoffile[FILENAME_MAX + 3];

Boolean
eof(FILE *fp)
{
	register int c;

	if (feof(fp))
		return true;
	else { /* check to see if next is EOF */
		c = getc(fp);
		if (c == EOF)
			return true;
		else {
			(void) ungetc(c,fp);
			return false;
		}
	}
}

Boolean
eoln(FILE *f)
{
    register int c;

    if (feof(f)) return(true);
    c = getc(f);
    if (c != EOF) (void) ungetc(c, f);
    if (c == '\n' || c == EOF) 
		return true;
	else
		return false;
}

/* Open a file; exit on error 
 */
FILE *
openf(char *name, char *mode)
{
    FILE *result;
    char *cp;

    cp = strchr(name, ' ');
    if (cp != NULL) *cp = '\0';
    result = fopen(name, mode);
    if (result) 
		return result;
    perror(name);
    exit(EXIT_FAILURE);
}

/* Return the nth argument in argv[] (global variable put by pp2rc
   in entry module) into the string s, with leading space,
   since it is for Pascal handling. libpascal::reset() takes name
   as the C string starting at s+1, so no trailing spaces, and a
   trailing '\0'.
 */
void 
argfmt(integer n, unsigned char s[])
{
	int i;

	s[0] = ' ';
	for (i=0; i < (FILENAME_MAX + 1) && argv[n][i] != '\0'; i++)
		s[i+1] = argv[n][i];
	s[i+1] = '\0';
}

