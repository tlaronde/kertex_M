#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "kertex.h"

#define	TRUE	1
#define	FALSE	0

#define max_line_length (78)
#define max_strings (20000)
#define hash_prime (101)
#define sym_table_size (3000)
#define unused (271828)
#define ex_32 (2)
#define ex_real (3)
#define max(a,b) ((a>b)?a:b)

extern int indent;
extern int line_pos;
extern int last_brace;
extern int block_level;
extern int ii;
extern int last_tok;

extern char safe_string[80];
extern char var_list[200];
extern char field_list[200];
extern char last_id[80];
extern char z_id[80];
extern char next_temp[];

extern long last_i_num;
extern int ii, l_s;
extern long lower_bound, upper_bound;
extern FILE *std;
extern int pf_count;

#include "symtab.h"

extern char strings[max_strings];
extern int hash_list[hash_prime];
extern short global;
extern struct sym_entry sym_table[sym_table_size];
extern int next_sym_free, next_string_free;
extern int mark_sym_free, mark_string_free;

extern int yyparse(void);

void find_next_temp(void);
void normal(void);
void new_line(void);
void indent_line(void);
void my_output(char *);
void semicolon(void);
void yyerror(char *);
int hash(char *);
int search_table(char *);
int add_to_table(char *);
void remove_locals(void);
void mark(void);
void initialize(void);
