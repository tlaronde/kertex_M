/*
 * This program takes TeX or MF in C as a single stream on stdin,
 * and it produces several .c and .h files in the current directory
 * as its output.
 *
 * Tim Morgan  September 19, 1987
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int filenumber = 0, ifdef_nesting = 0, has_ini, lines_in_file = 0;
char *output_name = "tex";

#define	TEMPFILE	"temp.c"
#define	MAXLINES	2000

#define	TRUE	1
#define	FALSE	0

char buffer[1024], filename[100];

static int readln(void);

FILE *out, *in, *ini, *temp;

int
main(int argc, char **argv)
{
    if (argc > 1)
	output_name = argv[1];

	in = stdin;	/* stdin is not guaranteed to be a constant */

	/*  Making the header with globals [TL]
	 */
    (void) sprintf(filename, "%sd.h", output_name);
    if (!(out = fopen(filename, "w")))
	perror(filename), exit(EXIT_FAILURE);

    for ((void) fgets(buffer, sizeof(buffer), in);
		!feof(in) && !ferror(in) && strncmp(&buffer[10], "coerce.h", 8);
    	(void) fgets(buffer, sizeof(buffer), in)) {
		if (buffer[0] == '#' || buffer[0] == '\n' || buffer[0] == '}'
	  		|| buffer[0] == '/'
	  		|| buffer[0] == ' ' 
			|| strncmp(buffer, "typedef", 7) == 0)
			;	/* prefix with nothing */
		else 
			(void) fputs("EXTERN ", out);
		(void) fputs(buffer, out);
    }
	if (strncmp(&buffer[10], "coerce.h", 8) != 0) {
		(void)fprintf(stderr, 
			"splitup: FATAL error: data truncated:\n"
			"Expecting 'coerce.h' got '%s'.\n",
			buffer);
		exit(EXIT_FAILURE);
	}
    (void) fputs(buffer, out);

    if (fclose(out)) perror("fclose"), exit(EXIT_FAILURE);

    (void) sprintf(filename, "i%s.c", output_name);
    ini = fopen(filename, "w");
    if (!ini) perror(filename), exit(EXIT_FAILURE);

    (void) fputs("#define EXTERN extern\n", ini);
    (void) fprintf(ini, "#include \"%sd.h\"\n\n", output_name);
    (void) sprintf(filename, "%s0.c", output_name);
    if (!(out = fopen(filename, "w"))) perror(filename), exit(EXIT_FAILURE);
    (void) fputs("#define EXTERN extern\n", out);
    (void) fprintf(out, "#include \"%sd.h\"\n\n", output_name);
    do {
	/* Read one routine into a temp file */
		has_ini = FALSE;
		if ( !(temp = fopen(TEMPFILE, "w+")))
		    perror(TEMPFILE), exit(EXIT_FAILURE);
		while (readln()) {
		    (void) fputs(buffer, temp);
		    if (buffer[0] == '}') break; /* End of procedure */
		}
		while (ifdef_nesting > 0 && readln())
		    (void) fputs(buffer, temp);
		rewind(temp);
		if (has_ini) {	/* Contained "#ifdef INITEX" */
		    while (fgets(buffer, sizeof(buffer), temp))
			(void) fputs(buffer, ini);
		}
		else {			/* Doesn't contain "#ifdef INITEX" */
		    while (fgets(buffer, sizeof(buffer), temp)) {
				(void) fputs(buffer, out);
				lines_in_file++;
		    }
		}
		if (fclose(temp)) perror("fclose"), exit(EXIT_FAILURE);
		if (lines_in_file > MAXLINES) {
		    if (fclose(out))
			perror("fclose"), exit(EXIT_FAILURE);
		    (void) sprintf(filename, "%s%d.c", output_name, ++filenumber);
		    if ( !(out = fopen(filename, "w")))
			perror(filename), exit(EXIT_FAILURE);
		    (void) fputs("#define EXTERN extern\n", out);
		    (void) fprintf(out, "#include \"%sd.h\"\n\n", output_name);
		    lines_in_file = 0;
		}
    } while (!feof(in));

    exit(EXIT_SUCCESS);
}

/*
 * Read a line of input into the buffer, returning FALSE on EOF.
 * If the line is of the form "#ifdef INI...", we set "has_ini"
 * TRUE else FALSE.  We also keep up with the #ifdef/#endif nesting
 * so we know when it's safe to finish writing the current file.
 */
static
int 
readln(void)
{
    if (fgets(buffer, sizeof(buffer), in) == NULL) return(FALSE);
    if (strncmp(buffer, "#ifdef", 6) == 0) {
		++ifdef_nesting;
		if (strncmp(&buffer[7], "INI", 3) == 0) has_ini = TRUE;
    } else if (strncmp(buffer, "#endif", 6) == 0) 
		--ifdef_nesting;
    return(TRUE);
}

