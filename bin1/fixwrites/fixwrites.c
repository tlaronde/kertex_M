/*
 * This program converts the pascal-type write/writeln's into fprintf's
 * or putc's.
 *
 * Tim Morgan   October 10, 1987
 */
/* Some light modifications by Thierry Laronde, 2010-04-07.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *insert_long(char *cp);
void join(char *cp);
void do_blanks(int indent);
int whole(char *buf);
char *skip_balanced(char *cp);
int bare(char *cp, char c);

#define	TRUE	1
#define	FALSE	0

char buf[BUFSIZ], filename[100], *file, *argp, args[100], *as, *cmd;

char *
insert_long(char *cp)
{
    char tbuf[BUFSIZ];
    register int i;

    for (i=0; &buf[i] < cp; ++i)
	tbuf[i] = buf[i];
    (void) strcpy(&tbuf[i], "(long)");
    (void) strcpy(&tbuf[i+6], cp);
    (void) strcpy(buf, tbuf);
    return(cp+6);
}

void 
join(char *cp)
{
    char temp[BUFSIZ], *tp;

    if (fgets(temp, BUFSIZ, stdin) == NULL) return;
	for (tp = temp; *tp != '\0' && *tp != '\n' && *tp != '\r'; tp++)
		;
	*tp = '\0';	/* erase newline if any */
    *cp++ = ' ';
    for (tp=temp; *tp == ' '; ++tp);
    (void) strcpy(cp, tp);
}

void 
do_blanks(int indent)
{
    register int i;

    for (i=0; i<indent/8; i++)
	(void) putchar('\t');
    indent %= 8;
    for (i=0; i<indent; i++)
	(void) putchar(' ');
}

/*
 * Return true if we have a whole write/writeln statement.  We determine
 * this by matching parens, ignoring those within strings.
 */
int
whole(register char *cp)
{
    register int depth=0;

    while (cp && *cp) {
	switch (*cp) {
	case '(':
		++depth;
		break;
	case ')':
		--depth;
		break;
	case '"':
		for (++cp; cp && *cp && *cp != '"'; ++cp)
			if (*cp == '\\') ++cp;
		break;
	case '\'':
		++cp;
		if (*cp == '\\') ++cp;
		++cp;
		break;
	}
	++cp;
    }
    return(depth <= 0);
}


/* Skips to the next , or ), skipping over balanced paren pairs */
char *
skip_balanced(char *cp)
{
    register int depth = 0;

    while (depth > 0 || (*cp != ',' && *cp != ')')) {
	switch (*cp) {
	case '(':
	    ++depth;
	    break;
	case ')':
	    --depth;
	    break;
	}
	++cp;
    }
    return(cp);
}

/* Return true if c appears, except inside a quoted string */
int
bare(char *cp, char c)
{
    for (; *cp && *cp != c; ++cp) {
	if (*cp == '"') {
	    ++cp;	/* Skip over initial quotation mark */
	    while (*cp && *cp != '"') { /* skip to closing double quote */
		if (*cp == '\\') ++cp;
		++cp;
	    }
	}
	else if (*cp == '\'') {
	    ++cp;	/* Skip to contained char */
	    if (*cp == '\'') ++cp;	/* if backslashed, it's double */
	    ++cp;	/* Skip to closing single-quote mark */
	}
    }
    return(*cp);
}

int
main(int argc, char **argv)
{
    register char *cp;
    int blanks_done, indent;

	if (argc != 1) {
		(void) fprintf(stderr, "usage: fixwrites <stdin\n");
		exit(EXIT_FAILURE);
	}

    while (fgets(buf, BUFSIZ, stdin) != NULL) {
		for (cp = buf; *cp != '\0' && *cp != '\n' && *cp != '\r'; cp++)
			;
		*cp = '\0';	/* erase new line if any */
	blanks_done = FALSE;
	for (cp=buf; *cp; ++cp);
	while (*--cp == ' ');
	while (*cp == '.') {
	    join(cp+1);
	    while (*cp) ++cp;
	    while (*--cp == ' ');
	}
	for (cp=buf, indent=0; *cp == ' ' || *cp == '\t'; ++cp) {
		if (*cp == ' ') indent++;
		else indent += 8;
	}
	if (!*cp) {	/* All blanks, possibly with "{" */
	    (void) puts(buf);
	    continue;
	}
	if (*cp == '{') {
	    do_blanks(indent);
	    (void) putchar('{');
	    ++cp;
	    while (*cp == ' ' || *cp == '\t') ++cp;
	    blanks_done = TRUE;
	    if (!*cp) {
		(void) putchar('\n');
		continue;
	    }
	}
	if (!blanks_done) do_blanks(indent);
	if (strncmp(cp, "read ( input", 12) == 0) {
	    char variable_name[20];
	    if (sscanf(cp, "read ( input , %s )", variable_name) != 1) {
			(void) fprintf(stderr, "sscanf failed\n");
			exit(EXIT_FAILURE);
		}
	    (void) printf("%s = getint();\n", variable_name);
	    continue;
	}
	if (strncmp(cp, "lab", 3) == 0 && strchr(cp, ':')) {
	    do {
		(void) putchar(*cp);
	    } while (*cp++ != ':');
	    while (*cp == ' ') ++cp;
	    (void) putchar(' ');
	}
	if (strncmp(cp, "else write", 10) == 0) {
	    (void) puts("else");
	    do_blanks(indent);
	    cp += 5;
	    while (*cp == ' ') ++cp;
	}
	if (bare(cp, '{')) {
	    while (*cp != '{') {
		(void) putchar(*cp);
		++cp;
	    }
	    ++cp;
	    (void) puts("{");
	    indent += 4;
	    do_blanks(indent);
	    while (*cp == ' ') ++cp;
	}
	if (strncmp(cp, "write (", 7) && strncmp(cp, "writeln (", 9)) {
	    /* if not a write/writeln, just copy it to stdout and continue */
	    (void) puts(cp);
	    continue;
	}
	/* Some writes directive */
	cmd = cp;
	while (!whole(buf))	{ /* Make sure we have whole stmt */
		char *p;
	    (void) fgets(&buf[strlen(buf)], BUFSIZ - strlen(buf), stdin);
		for (p = &buf[strlen(buf)]; *p != '\0' && *p != '\n' && *p != '\r'; p++)
			;
		*p = '\0';	/* erase new line if any */
	}
	while (*cp != '(') ++cp;
	++cp;
	while (*(cp+1) == ' ') ++cp;
	/* Stream to write to.
	 */
	if (*(cp+1) == '"' || *(cp+1) == '\'' ||
	  strncmp(cp+1, "buffer", 6) == 0 ||
	  strncmp(cp+1, "dig", 3) == 0 ||
	  strncmp(cp+1, "xchr", 4) == 0 ||
	  strncmp(cp+1, "k ,", 3) == 0 ||
	  strncmp(cp+1, "s ,", 3) == 0)
	    (void) strcpy(filename, "stdout");
	else {
	    file = filename;
	    while (*cp != ',' && *cp != ')')
			*file++ = *cp++;
	    *file = '\0';
	}
	/* If no format, just add a newline and continue.
	 */
	if (*cp == ')') {
	    (void) printf("(void) putc('\\n', %s);\n", filename);
	    continue;
	}
	argp = ++cp;
	as = args;
	while (*cp==' ') ++cp;
	while (*cp != ')') {
	    if (*cp == '\'' || strncmp(cp, "xchr", 4) == 0
	      || strncmp(cp, "ASCII04", 7) == 0
	      || strncmp(cp, "ASCII1", 6) == 0
	      || strncmp(cp, "nameoffile", 10) == 0
	      || strncmp(cp, "months", 6) == 0) {
		*as++ = '%';
		*as++ = 'c';
		if (*cp == '\'')
			cp += 2;
	    } else if (*cp == '"') {
			*as++ = '%';
			*as++ = 's';
			while (*++cp != '"') /* skip to end of string */
		    	if (*cp == '\\') ++cp;	/* allow \" in string */
	    } else {
			*as++ = '%';
			*as++ = 'l';
			*as++ = 'd';
			cp = insert_long(cp);
			cp = skip_balanced(cp);/* It's a numeric expression */
	    }
	    while (*cp != ',' && *cp != ')') ++cp;
	    while (*cp == ',' || *cp == ' ' || *cp == '\n') ++cp;
	}
	if (strncmp(cmd, "writeln", 7) == 0) {
	    *as++ = '\\';
	    *as++ = 'n';
	}
	*as = '\0';
	if (strcmp(args, "%c") == 0) {
	    for (as = argp; *as; ++as);
	    while (*--as != ')');
	    *as = '\0';
	    (void) printf("(void) putc(%s, %s);\n", argp, filename);
	} else if (strcmp(args, "%s") == 0) {
	    (void) printf("(void) fputs_(%s, %s\n", filename, argp);
	} else {
	    (void) printf("(void) fprintf(%s, \"%s\", %s\n",
			  filename, args, argp);
	}
    }
    exit(EXIT_SUCCESS);
}
