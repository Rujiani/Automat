%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "variables.h"
#include "actions.h"
#include "types.h"
#include "gc.h"
#include "robot.h"
#include "maze.h"
#include "func.h"
Value last_overview;

void yyerror(const char* msg);

ActionList program_actions;

struct Bin       { Expr l, r; char op; };
struct AssignCtx { char *id; Expr rhs; };
struct PrintCtx  { Expr e; };
struct IfCtx     { Expr cond; ActionList* body; };
struct LoopCtx   { char *id; Expr from, to; ActionList* body; };
struct Un        { Expr x; char op; };

struct IndexCtx {
    Expr array;
    Expr index;
};

enum StmtKind { STMT_ACTION, STMT_EXPR };
typedef struct Stmt {
    enum StmtKind kind;
    union {
        Action act;
        Expr   expr;
    };
    struct Stmt *next;
} Stmt;

struct CallCtx {
    Function *f;
    size_t    argc;
    Expr      argv[16];
};

static Value eval_call(void *ctx)
{
    struct CallCtx *c = ctx;
    return run_function(c->f, c->argv, c->argc);
}

static Action make_expr_stmt(Expr e)
{
    Action a = { NULL, NULL };
    a.fn  = NULL;
    a.ctx = gc_alloc(sizeof(Expr));
    *(Expr*)a.ctx = e;
    return a;
}

static Value eval_array_index(void *ctx) {
    struct IndexCtx *c = ctx;
    Value arr = c->array.fn(c->array.ctx);
    Value idx = c->index.fn(c->index.ctx);

    if (arr.type != VALUE_ARRAY || idx.type != VALUE_INT) {
        fprintf(stderr,"type mismatch\n"); exit(1);
    }
    if ((size_t)idx.i >= arr.array.size) {
        fprintf(stderr,"index out of range\n"); exit(1);
    }
    return arr.array.data[idx.i];
}

struct AssignArrayCtx {
    char *name;
    Expr index;
    Expr rhs;
};

static void run_nested_assign(void *ctx)
{
    struct NestedAssignCtx *c = ctx;

    Value *cur = gc_alloc(sizeof(Value));
    *cur = get_var(c->root);

    for (size_t k = 0; k < c->depth; ++k) {
        Value idv = c->idx[k].fn(c->idx[k].ctx);
        if (cur->type != VALUE_ARRAY || idv.type != VALUE_INT) {
            fprintf(stderr,"type mismatch (nested assign)\n");
            exit(1);
        }
        if ((size_t)idv.i >= cur->array.size) {
            fprintf(stderr,"index out of range (level %zu)\n", k);
            exit(1);
        }
        if (k + 1 == c->depth) {
            cur->array.data[idv.i] = c->rhs.fn(c->rhs.ctx);
        } else {
            cur = &cur->array.data[idv.i];
        }
    }
}

// static Value eval_const(void *ctx) {
//     return *(Value*)ctx;
// }

static Value make_multi_array(size_t dim, const size_t *sizes)
{
    if (dim == 0) return make_unknown();

    Value v = make_array(sizes[0]);
    if (dim > 1) {
        for (size_t i = 0; i < sizes[0]; ++i)
            v.array.data[i] = make_multi_array(dim - 1, sizes + 1);
    }
    return v;
}

static Value eval_var(void *ctx){ return get_var((char*)ctx); }

static Expr make_var(char *name)     { return (Expr){ eval_var, strdup(name) }; }

static Value eval_un(void *ctx) {
    struct Un *u = ctx;
    Value v = u->x.fn(u->x.ctx);

    if (v.type != VALUE_BOOL) {
        fprintf(stderr, "unary '~' expects bool, got %s\n",
                value_type_name(v.type));
        exit(1);
    }
    return make_bool(!v.i);   // «i» – 0/1
}

static Value eval_const_bool(void *ctx)
{
    int *pi = ctx;
    return make_bool(*pi);
}

static Expr make_const_bool(int b)
{
    int *pv = gc_alloc(sizeof(int));
    *pv = b;
    return (Expr){ eval_const_bool, pv };
}


static Expr make_un(Expr x, char op) {
    struct Un *u = malloc(sizeof *u);
    u->x = x; u->op = op;
    return (Expr){ eval_un, u };
}

static Value eval_bin(void *ctx)
{
    struct Bin *b = ctx;
    Value a = b->l.fn(b->l.ctx);
    Value c = b->r.fn(b->r.ctx);

    ensure_same(a, c, "binary op");

    switch (b->op) {

        case '+': case '-':
            if (a.type != VALUE_INT) {
                fprintf(stderr,"+, - expect int, got %s\n",
                        value_type_name(a.type));
                exit(1);
            }
            return make_int(b->op=='+' ? a.i + c.i : a.i - c.i);

        case '<': case '>':
            if (a.type != VALUE_INT) {
                fprintf(stderr,"<, > expect int, got %s\n",
                        value_type_name(a.type));
                exit(1);
            }
            return make_bool(b->op=='<' ? a.i < c.i : a.i > c.i);

        case '&': case '|':
            if (a.type != VALUE_BOOL) {
                fprintf(stderr,"&&, || expect bool, got %s\n",
                        value_type_name(a.type));
                exit(1);
            }
            return make_bool(b->op=='&' ? (a.i && c.i)
                                        : (a.i || c.i));
    }

    return make_unknown();
}



static Expr make_bin(Expr l, Expr r, char op) {
    struct Bin *b = malloc(sizeof *b);
    b->l = l; b->r = r; b->op = op;
    return (Expr){ eval_bin, b };
}
extern int sequence_break;
extern int seq_depth;
Value last_overview = {0};


static void run_block(void *ctx)
{
    ActionList *list = (ActionList*)ctx;

    ++seq_depth;
    sequence_break = 0;

    for (ActionNode *n = list->head; n && !sequence_break; n = n->next)
        if (n->a.fn)
            n->a.fn(n->a.ctx);

    --seq_depth;
    last_overview = sequence_flush_overview();
}
// static void act_add_var(void *ctx) { add_var((char*)ctx); }
// static Action make_add_var(char *id) { return (Action){ act_add_var, id }; }

static void act_assign(void *ctx)
{
    struct AssignCtx *c = ctx;
    if (!var_exists(c->id)) {
        fprintf(stderr, "undeclared variable: %s\n", c->id);
        exit(1);
    }
    set_var(c->id, c->rhs.fn(c->rhs.ctx));
}

static Action make_assign(char *id, Expr rhs) {
    struct AssignCtx *c = malloc(sizeof *c);
    c->id = id; c->rhs = rhs;
    return (Action){ act_assign, c };
}

static Value eval_position(void *ctx)
{
    (void)ctx;
    int busy = maze_is_obstacle(robo.x, robo.y);
    return make_cell_xyz(robo.x, robo.y, busy);
}

static Expr make_position_expr(void){
    return (Expr){ eval_position, NULL };
}

static Value make_int_val(long long v)          { return make_int(v); }
static Value eval_const_ll(void *ctx)
{
    long long *pi = ctx;
    return make_int(*pi);
}

static Expr make_const_ll(long long v)
{
    long long *pv = gc_alloc(sizeof(long long));
    *pv = v;
    return (Expr){ eval_const_ll, pv };
}


static void act_print(void *ctx)
{
    struct PrintCtx *c = ctx;
    Value v = c->e.fn(c->e.ctx);

    switch(v.type){
        case VALUE_INT:   printf("%lld\n", v.i); break;
        case VALUE_CELL:  printf("(%lld, %lld) %s\n",
                                v.cell.x, v.cell.y,
                                v.cell.busy ? "busy":"free"); break;
        case VALUE_BOOL:  printf("%s\n", v.i ? "thật" : "sai"); break;
        default:          printf("<unknown>\n");
    }
}


static Action make_print(Expr e) {
    struct PrintCtx *c = malloc(sizeof *c);
    c->e = e;
    return (Action){ act_print, c };
}

static void if_run(void *ctx) {
    struct IfCtx *c = ctx;
    Value cond = c->cond.fn(c->cond.ctx);
    if (cond.type != VALUE_BOOL) {
        fprintf(stderr, "`nếu` condition must be bool, got %s\n", value_type_name(cond.type));
        exit(1);
    }
    if (cond.i)
        action_list_run(c->body);
}


static void loop_run(void *ctx)
{
    struct LoopCtx *c = ctx;
    long long s = c->from.fn(c->from.ctx).i;
    long long e = c->to.fn(c->to.ctx).i;

    if (!var_exists(c->id)) add_var(c->id);

    for(long long i=s;i<=e;++i){
        set_var(c->id, make_int_val(i));
        action_list_run(c->body);
    }
}

static void make_lvalue_assign(Expr *lhs, Expr rhs, Action *out)
{
    if (lhs->fn == eval_var) {
        *out = make_assign((char*)lhs->ctx, rhs);
        return;
    }

    struct NestedAssignCtx *na = gc_alloc(sizeof *na);
    na->depth = 0;
    na->rhs   = rhs;

    Expr *cur = lhs;
    while (cur->fn == eval_array_index) {
        struct IndexCtx *ix = cur->ctx;
        if (na->depth >= MAX_NEST) {
            fprintf(stderr,"too many nested indices\n"); exit(1);
        }
        na->idx[na->depth++] = ix->index;
        cur = &ix->array;
    }

    if (cur->fn != eval_var) {
        fprintf(stderr,"unsupported lvalue shape\n"); exit(1);
    }
    na->root = (char*)cur->ctx;

    out->fn  = run_nested_assign;
    out->ctx = na;
}

extern int var_exists(const char*);
int yylex(void);
void yyerror(const char* msg);
%}

%code requires {
  #include "types.h"
  #include "actions.h"
  #include "func.h"
}

%union {
    long long  num;
    char      *str;
    Expr        expr;
    Action      act;
    ActionList *list;
    struct {
        size_t dims[MAX_DIMS];
        size_t n;
    } dimlist;
    struct { size_t n; Expr a[16]; } callargs;
    ParamList *params;
}


%glr-parser
%type <params> param_list_opt param_list
%type <num> type
%type <callargs> arg_list
%type <expr>    expr
%type <act>     statement
%type <list>    program
%type <dimlist> dims
%type <expr> lvalue
%type <list>   block_opt
%type <callargs> arg_list_opt
%type <list> program_main statement_list


%left OR
%left AND
%right NOT
%nonassoc LT GT
%left PLUS MINUS
%nonassoc PAREN
%right FIELD_ACCESS
%left LBRACKET

%token <num> INT_LITERAL
%token <str> IDENTIFIER
%token KEYWORD_INT
%token KEYWORD_BOOL
%token KEYWORD_CELL
%token KEYWORD_ARRAY
%token KEYWORD_DIM
%token KEYWORD_TYPECHECK
%token KEYWORD_IF
%token KEYWORD_LOOP
%token KEYWORD_BEGIN
%token KEYWORD_END
%token KEYWORD_FUNC
%token ASSIGN
%token PLUS
%token MINUS
%token LT
%token GT
%token NOT
%token AND
%token OR
%token SEMICOLON
%token COLON
%token COMMA
%token LPAREN
%token RPAREN
%token LBRACKET RBRACKET
%token LBRACE
%token RBRACE
%token FIELD_ACCESS
%token MOVE_FORWARD
%token MOVE_BACKWARD
%token MOVE_LEFT
%token MOVE_RIGHT
%token SENSE_FORWARD
%token SENSE_BACKWARD
%token SENSE_LEFT
%token SENSE_RIGHT
%token STOP_IF_OBSTACLE
%token GET_POSITION
%token TRUE_LITERAL
%token FALSE_LITERAL
%token UNKNOWN
%token PAREN
%token END_OF_FILE
%type <list> block block_statements
%nonassoc BLOCK_STMT

%start program_file

%%

program_file
    : top_items END_OF_FILE
    ;

top_items
    : /* empty */
    | top_items top_item
    ;

top_item
    : function_def
    | statement
        {
            if ($1.fn) action_list_add(&program_actions, $1);
        }
    ;


program
    : function_def              { $$ = &program_actions; }
    | statement                 { $$ = &program_actions; if ($1.fn) action_list_add($$, $1); }
    | program function_def      { $$ = $1; }
    | program statement         { $$ = $1; if ($2.fn) action_list_add($$, $2); }
    ;


function_list
    : /* empty */
    | function_def
    | function_list function_def
    ;


program_main
    : statement_list                { $$ = $1; }
    ;

statement_list
    : statement                     { $$ = &program_actions; if ($1.fn) action_list_add($$, $1); }
    | statement_list statement      { $$ = $1; if ($2.fn) action_list_add($$, $2); }
    ;

program_body
    : program
    ;


function_def
    : KEYWORD_FUNC type IDENTIFIER LPAREN param_list_opt RPAREN
      KEYWORD_BEGIN block_statements KEYWORD_END
      {
          Function *f = gc_alloc(sizeof *f);
          f->ret_type = $2;
          f->name     = $3;
          f->params   = $5;
          f->body     = $8;
          add_function(f);
      }
    ;


param_list_opt
    : /* empty */                { $$ = create_param_list(); }
    | param_list
    ;
param_list
    : type IDENTIFIER                    { $$ = single_param($1,$2); }
    | param_list COMMA type IDENTIFIER   { $$ = append_param($1,$3,$4); }
    ;

type
      : KEYWORD_INT   { $$ = VALUE_INT;  }
      | KEYWORD_BOOL  { $$ = VALUE_BOOL; }
      | KEYWORD_CELL  { $$ = VALUE_CELL; }
      ;

statement
    : KEYWORD_INT   IDENTIFIER SEMICOLON { add_var_with_type($2, VALUE_INT);  $$ = (Action){0}; }
    | KEYWORD_BOOL  IDENTIFIER SEMICOLON { add_var_with_type($2, VALUE_BOOL); $$ = (Action){0}; }
    | KEYWORD_CELL  IDENTIFIER SEMICOLON { add_var_with_type($2, VALUE_CELL); $$ = (Action){0}; }
    | KEYWORD_CELL  IDENTIFIER ASSIGN expr SEMICOLON
        {
            add_var_with_type($2, VALUE_CELL);
            $$ = make_assign($2, $4);
        }
    | lvalue ASSIGN expr SEMICOLON
    {
        Expr tmp = $1;
        make_lvalue_assign(&tmp, $3, &$$);
    }
    | expr SEMICOLON                         { $$ = make_print($1); }
    | block SEMICOLON        { $$ = (Action){ run_block, $1 }; }
    | KEYWORD_IF expr block
        {
            struct IfCtx *c = malloc(sizeof *c);
            c->cond = $2; c->body = $3;
            $$ = (Action){ if_run, c };
        }

    | KEYWORD_LOOP IDENTIFIER ASSIGN expr COLON expr block
        {
            struct LoopCtx *c = malloc(sizeof *c);
            c->id=$2; c->from=$4; c->to=$6; c->body=$7;
            $$ = (Action){ loop_run, c };
        }
    | KEYWORD_ARRAY IDENTIFIER ASSIGN LBRACE dims RBRACE SEMICOLON
        {
            Value arr = make_multi_array($5.n, $5.dims);
            add_var($2);
            set_var($2, arr);
            $$ = (Action){0};
        }
    | MOVE_FORWARD  SEMICOLON { $$ = make_move_action( 0, -1); }
    | MOVE_BACKWARD SEMICOLON { $$ = make_move_action( 0,  1); }
    | MOVE_LEFT     SEMICOLON { $$ = make_move_action(-1,  0); }
    | MOVE_RIGHT    SEMICOLON { $$ = make_move_action( 1,  0); }

    | SENSE_FORWARD  SEMICOLON { $$ = make_sense_action( 0,-1); }
    | SENSE_BACKWARD SEMICOLON { $$ = make_sense_action( 0, 1); }
    | SENSE_LEFT     SEMICOLON { $$ = make_sense_action(-1,0); }
    | SENSE_RIGHT    SEMICOLON { $$ = make_sense_action( 1,0); }
    | STOP_IF_OBSTACLE SEMICOLON { $$ = make_stop_if_obstacle(); }
    | expr                %prec BLOCK_STMT
      { $$ = make_expr_stmt($1); }
;

arg_list_opt
      : /* empty */          { $$.n = 0; }
      | arg_list
      ;

arg_list
      : expr                 { $$.n = 1; $$.a[0] = $1; }
      | arg_list COMMA expr  { $$=$1; $$.a[$$.n++]=$3; }
      ;

lvalue
    : IDENTIFIER
        { $$ = make_var($1); }
    | lvalue LBRACKET expr RBRACKET
        {
            struct IndexCtx *c = gc_alloc(sizeof *c);
            c->array = $1;
            c->index = $3;
            $$ = (Expr){ eval_array_index, c };
        }
;

block_opt
    : /* empty */
      {
          $$ = gc_alloc(sizeof(ActionList));
          action_list_init($$);
      }
    | block
      { $$ = $1; }
;

block
    : KEYWORD_BEGIN block_statements KEYWORD_END
      {
          $$ = $2;
      }
    | LBRACE block_statements RBRACE
        { $$ = $2; }
;

block_statements
    : /* empty */             { $$ = malloc(sizeof(ActionList));
                                action_list_init($$); }
    | block_statements statement
        { $$ = $1; if($2.fn) action_list_add($$, $2); }
;

dims
    : expr
        {
            long long v = $1.fn($1.ctx).i;
            if (v <= 0) { fprintf(stderr,"dimension must be >0\n"); exit(1); }
            $$.dims[0] = (size_t)v;
            $$.n = 1;
        }
    | dims COMMA expr
        {
            if ($1.n >= MAX_DIMS) { fprintf(stderr,"too many dimensions\n"); exit(1); }
            long long v = $3.fn($3.ctx).i;
            if (v <= 0) { fprintf(stderr,"dimension must be >0\n"); exit(1); }
            $$ = $1;
            $$.dims[$$.n++] = (size_t)v;
        }
;

expr
    : IDENTIFIER LPAREN arg_list_opt RPAREN
      {
          Function *f = get_function($1);
          if (!f) { fprintf(stderr,"unknown func %s\n",$1); YYERROR; }

          struct CallCtx *cc = gc_alloc(sizeof *cc);
          cc->f = f; cc->argc = $3.n;
          memcpy(cc->argv, $3.a, cc->argc*sizeof(Expr));

          $$ = (Expr){ eval_call, cc };
      }
    | '(' expr ')'               { $$ = $2; }
    | NOT expr                   { $$ = make_un($2, '~'); }
    | expr AND expr              { $$ = make_bin($1,$3,'&'); }
    | expr OR  expr              { $$ = make_bin($1,$3,'|'); }
    | GET_POSITION               { $$ = make_position_expr(); }

    | KEYWORD_DIM IDENTIFIER
        {
            if (!var_exists($2)) {
                fprintf(stderr, "undeclared variable: %s\n", $2);
                exit(1);
            }
            Value v = get_var($2);
            if (v.type != VALUE_ARRAY) {
                fprintf(stderr, "kich thuoc on non-array: %s\n", $2);
                exit(1);
            }
            $$ = make_const_ll((long long)v.array.size);
        }

    | KEYWORD_TYPECHECK LBRACE expr expr RBRACE
        { $$ = make_typecheck_expr($3, $4); }
    | expr FIELD_ACCESS IDENTIFIER
        {
            enum CellField fld;
            if (strcmp($3, "x") == 0)       fld = CELL_X;
            else if (strcmp($3, "y") == 0)  fld = CELL_Y;
            else if (strcmp($3, "busy")==0) fld = CELL_BUSY;
            else {
                fprintf(stderr, "unknown cell field: %s\n", $3);
                YYERROR;
            }
            $$ = make_field_expr($1, fld);
        }
    
    | LBRACE expr COMMA expr COMMA expr RBRACE
        { $$ = make_cell_lit($2, $4, $6); }



    | INT_LITERAL                { $$ = make_const_ll($1); }
    | TRUE_LITERAL               { $$ = make_const_bool(1); }
    | FALSE_LITERAL              { $$ = make_const_bool(0); }

    | IDENTIFIER                 { $$ = make_var($1); }

    | expr PLUS  expr            { $$ = make_bin($1,$3,'+'); }
    | expr MINUS expr            { $$ = make_bin($1,$3,'-'); }
    | expr LT expr               { $$ = make_bin($1,$3,'<'); }
    | expr GT expr               { $$ = make_bin($1,$3,'>'); }
    | expr LBRACKET expr RBRACKET
      {
          struct IndexCtx *c = gc_alloc(sizeof *c);
          c->array = $1;
          c->index = $3;
          $$ = (Expr){ eval_array_index, c };
      }
    ;

%%

void yyerror(const char* msg) { fprintf(stderr, "syntax error: %s\n", msg); }

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <maze.txt> < program.robot\n", argv[0]);
        return 1;
    }
    read_maze(argv[1]);
    robot_init(robo.x, robo.y);
   extern int yydebug;
   yydebug = 1;
    action_list_init(&program_actions);
    yyparse();
    action_list_run(&program_actions);
    action_list_free(&program_actions);
    gc_collect();
    maze_free();
    return 0;
}
