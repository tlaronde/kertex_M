%{
#include "pp2rc.h"
#include "parser.h"

#ifdef yywrap
#undef yywrap
#endif

int yylex(void);
int yywrap(void);

#ifdef FLEX_SCANNER
#define YY_NO_UNISTD_H 1 /* for flex non-ANSI use */
#ifdef isatty
#undef isatty
#endif
int isatty_(int fd);
#define isatty(fd) isatty_(fd)
#ifdef fileno
#undef fileno
#endif
int fileno_(FILE *stream);
#define fileno(stream) fileno_(stream)
#endif /* FLEX_SCANNER */

char conditional[20], negbuf[2], temp[20];
extern boolean doing_statements;
%}
DIGIT		[0-9]
ALPHA		[a-zA-Z]
ALPHANUM	({DIGIT}|{ALPHA})
IDENTIFIER	({ALPHA}{ALPHANUM}*)
NUMBER		({DIGIT}+)
SIGN		("+"|"-")
SIGNED		({SIGN}?{NUMBER})
WHITE		[ \n\t]+
REAL		({NUMBER}"."{NUMBER}("e"{SIGNED})?)|({NUMBER}"e"{SIGNED})
COMMENT		(("{"[^}]*"}")|("(*"([^*]|"*"[^)])*"*)"))
W		({WHITE}|"packed ")+
WW		({WHITE}|{COMMENT}|"packed ")*

%%
{W}				;
"{"				{while (input() != '}');
				}
"#"				{
				    register int c;
				    (void) putc('#', std);
				    while ((c = input()) && c != ';')
					(void) putc(c, std);
				    (void) putc('\n', std);
				}
"ifdef("			{register int c;
				 register char *cp=conditional;
				 new_line();
				 (void) input();
				 while ((c = input()) != '\'')
#ifdef	MS_DOS
				    *cp++ = (char) c;
#else
				    *cp++ = c;
#endif
				 *cp = '\0';
				 (void) input();
				 if (doing_statements) fputs("\t;\n", std);
				 (void) fprintf(std,
					"#ifdef %s\n", conditional);
				}
"endif("			{register int c;
				 new_line();
				 fputs("#endif /* ", std);
				 (void) input();
				 while ((c = input()) != '\'')
					(void) putc(c, std);
				 (void) input();
				 conditional[0] = '\0';
				 fputs(" */\n", std);
				}
"procedure "[a-z]+";"[ \n\t]*"forward;"	;
"function "[(),:a-z]+";"[ \n\t]*"forward;"	;
"@define"			return(last_tok=define_tok);
"@field"			return(last_tok=field_tok);
"and"				return(last_tok=and_tok) ;
"array"				return(last_tok=array_tok) ;
"begin"				return(last_tok=begin_tok) ;
"case"				return(last_tok=case_tok) ;
"const"				return(last_tok=const_tok) ;
"div"				return(last_tok=div_tok) ;
"do"				return(last_tok=do_tok) ;
"downto"			return(last_tok=downto_tok) ;
"else"				return(last_tok=else_tok) ;
"end"				return(last_tok=end_tok) ;
"file"				return(last_tok=file_tok) ;
"for"				return(last_tok=for_tok) ;
"function"			return(last_tok=function_tok) ;
"goto"				return(last_tok=goto_tok) ;
"if"				return(last_tok=if_tok) ;
"label"				return(last_tok=label_tok) ;
"mod"				return(last_tok=mod_tok) ;
"not"				return(last_tok=not_tok) ;
"of"				return(last_tok=of_tok) ;
"or"				return(last_tok=or_tok) ;
"procedure"			return(last_tok=procedure_tok) ;
"program"			return(last_tok=program_tok) ;
"record"			return(last_tok=record_tok) ;
"repeat"			return(last_tok=repeat_tok) ;
"then"				return(last_tok=then_tok) ;
"to"				return(last_tok=to_tok) ;
"type"				return(last_tok=type_tok) ;
"until"				return(last_tok=until_tok) ;
"var"				return(last_tok=var_tok) ;
"while"				return(last_tok=while_tok) ;
"others"			return(last_tok=others_tok) ;
{REAL}				{		
				  (void) sprintf(temp, "%s%s", negbuf, yytext);
				  negbuf[0] = '\0';
				  return(last_tok=r_num_tok) ;
				}
{NUMBER}			{
				  (void) sprintf(temp, "%s%s", negbuf, yytext);
				  negbuf[0] = '\0';
				  return(last_tok=i_num_tok) ;
				}
("'"([^']|"''")"'")		return(last_tok=single_char_tok) ;
("'"([^']|"''")*"'")		return(last_tok=string_literal_tok) ;
"+"				{ if ((last_tok>=undef_id_tok &&
				      last_tok<=field_id_tok) ||
				      last_tok==i_num_tok ||
				      last_tok==r_num_tok ||
				      last_tok==')' ||
				      last_tok==']')
				   return(last_tok='+') ;
				else return(last_tok=unary_plus_tok) ; }
"-"				{ if ((last_tok>=undef_id_tok &&
				      last_tok<=field_id_tok) ||
				      last_tok==i_num_tok ||
				      last_tok==r_num_tok ||
				      last_tok==')' ||
				      last_tok==']')
				   return(last_tok='-') ;
				else {
				  int c;
				  while ((c = input()) == ' ' || c == '\t');
#ifdef	MS_DOS
				  unput((char) c);
#else
				  unput(c);
#endif
				  if (c < '0' || c > '9') {
					return(last_tok = unary_minus_tok);
				  }
				  negbuf[0] = '-';
				}}
"*"				return(last_tok='*') ;
"/"				return(last_tok='/') ;
"="				return(last_tok='=') ;
"<>"				return(last_tok=not_eq_tok) ;
"<"				return(last_tok='<') ;
">"				return(last_tok='>') ;
"<="				return(last_tok=less_eq_tok) ;
">="				return(last_tok=great_eq_tok) ;
"("				return(last_tok='(') ;
")"				return(last_tok=')') ;
"["				return(last_tok='[') ;
"]"				return(last_tok=']') ;
":="				return(last_tok=assign_tok) ;
".."				return(last_tok=two_dots_tok) ;
"."				return(last_tok='.') ;
","				return(last_tok=',') ;
";"				return(last_tok=';') ;
":"				return(last_tok=':') ;
"^"				return(last_tok='^') ;
{IDENTIFIER}			{ (void) strcpy(last_id,yytext) ; 
				  l_s=search_table(last_id) ;
				  if (l_s == -1) return(last_tok=undef_id_tok);
				  return(last_tok=sym_table[l_s].typ);
				}
.				return(last_tok=unknown_tok) ;
%%
int
yywrap(void)
{
	return 1;
}

#ifdef FLEX_SCANNER

/* ARGUSED */
int
isatty_(int fd)
{
	return 0;
}

/* ARGUSED */
int
fileno_(FILE *stream)
{
	return 0;
}
#endif /* FLEX_SCANNER */
