%option nounput noinput
%option noyywrap

%{
#include "parser.h"
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "gc.h"
%}

%%
"số nguyên"             return KEYWORD_INT;
"boolean"               return KEYWORD_BOOL;
"vuông"                 return KEYWORD_CELL;
"sắp hàng"              return KEYWORD_ARRAY;
"kích thước"            return KEYWORD_DIM;
"kiểm tra loại"         return KEYWORD_TYPECHECK;
"nếu"                   return KEYWORD_IF;
"chu kỳ"                return KEYWORD_LOOP;
"bắt đầu"               return KEYWORD_BEGIN;
"đuôi"                  return KEYWORD_END;
"hoạt động"             return KEYWORD_FUNC;

"="     return ASSIGN;
"+"     return PLUS;
"-"     return MINUS;
"<"     return LT;
">"     return GT;
"~"     return NOT;
"&&"    return AND;
"||"    return OR;
":"     return COLON;
";"     return SEMICOLON;
","     return COMMA;
"\("     return LPAREN;
"\)"     return RPAREN;
"{"     return LBRACE;
"}"     return RBRACE;
"["     return LBRACKET;
"]"     return RBRACKET;
"=>"    return FIELD_ACCESS;

"^_0"   return SENSE_FORWARD;
"v_0"   return SENSE_BACKWARD;
"<_0"   return SENSE_LEFT;
">_0"   return SENSE_RIGHT;

"^_^"   return MOVE_FORWARD;
"v_v"   return MOVE_BACKWARD;
"<_<"   return MOVE_LEFT;
">_>"   return MOVE_RIGHT;
">_<"   return STOP_IF_OBSTACLE;
"*_*"   return GET_POSITION;

"thật" return TRUE_LITERAL;
"sai"   return FALSE_LITERAL;

[0-9]+ {
    yylval.num = atoll(yytext);
    return INT_LITERAL;
}

[a-zA-Z_][a-zA-Z0-9_]*  {
        size_t len = strlen(yytext) + 1;  /* +1 под '\0' */
        char  *s   = gc_alloc(len);       /* память отдаём GC          */
        memcpy(s, yytext, len);           /* копируем сам текст        */
        yylval.str = s;                   /* парсеру отдаём готовую    */
        return IDENTIFIER;
}


[ \t\r\n]+   {;}

<<EOF>>      { return END_OF_FILE; }

.            { return UNKNOWN; }
