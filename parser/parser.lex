%{
#include <string.h>
#include <cstdlib>
#include "SemValue.h"
#define YYSTYPE SemValue
#include "parser.tab.h"
void yyerror(const char*);
%}

%option noyywrap

DIGIT ([0-9])
NUMBER ({DIGIT}+)
VALUE_INT ([+\-]?{NUMBER})
VALUE_STRING ('(\\.|[^'\\])*')

IDENTIFIER ([A-Za-z][_0-9A-Za-z]*)
EXIT ("EXIT"|"exit")
LE ("<=")
GE (">=")
NE ("<>")
LIKE ("like"|"LIKE")
OPERATOR ("<"|">"|"="|","|"."|";"|"*"|"("|")")

DATABASE ("database"|"DATABASE")
DATABASES ("databases"|"DATABASES")
TABLE ("table"|"TABLE")
TABLES ("tables"|"TABLES")
SHOW ("show"|"SHOW")
CREATE ("create"|"CREATE")
DROP ("drop"|"DROP")
USE ("use"|"USE")
PRIMARY ("primary"|"PRIMARY")
KEY ("key"|"KEY")
NOT ("not"|"NOT")
NUL ("null"|"NULL")
INSERT ("insert"|"INSERT")
INTO ("into"|"INTO")
VALUES ("values"|"VALUES")
DELETE ("delete"|"DELETE")
FROM ("from"|"FROM")
WHERE ("where"|"WHERE")
UPDATE ("update"|"UPDATE")
SET ("set"|"SET")
SELECT ("select"|"SELECT")
IS ("is"|"IS")
TYPE_INT ("int"|"INT")
TYPE_VARCHAR ("varchar"|"VARCHAR")
DESC ("desc"|"DESC")
REFERENCES ("references"|"REFERENCES")
INDEX ("index"|"INDEX")
AND ("and"|"AND")
TYPE_DATE ("DATE")
TYPE_FLOAT ("float"|"FLOAT")
FOREIGN ("foreign"|"FOREIGN")

TYPE_CHAR ("char"|"CHAR")

WHITESPACE ([ \t]+)
NEWLINE (\r|\n|\r\n)
%x COMMENT
%%

"/*"                                    { BEGIN(COMMENT); }
<COMMENT>[^*]                           { /* ignore comment */ }
<COMMENT>"*/"                           { BEGIN(INITIAL); }
<COMMENT>\*                             { /* ignore * */ }
{WHITESPACE}                            { /* do nothing */ }
{NEWLINE}                               { /* do nothing */ }
{EXIT}                                  { return EXIT; }

{DATABASE}                              { return SemValue::keyword(DATABASE); }
{DATABASES}                             { return SemValue::keyword(DATABASES); }
{TABLE}                                 { return SemValue::keyword(TABLE); }
{TABLES}                                { return SemValue::keyword(TABLES); }
{SHOW}                                  { return SemValue::keyword(SHOW); }
{CREATE}                                { return SemValue::keyword(CREATE); }
{DROP}                                  { return SemValue::keyword(DROP); }
{USE}                                   { return SemValue::keyword(USE); }
{PRIMARY}                               { return SemValue::keyword(PRIMARY); }
{KEY}                                   { return SemValue::keyword(KEY); }
{NOT}                                   { return SemValue::keyword(NOT); }
{NUL}                                   { return SemValue::keyword(NUL); }
{INSERT}                                { return SemValue::keyword(INSERT); }
{INTO}                                  { return SemValue::keyword(INTO); }
{VALUES}                                { return SemValue::keyword(VALUES); }
{DELETE}                                { return SemValue::keyword(DELETE); }
{FROM}                                  { return SemValue::keyword(FROM); }
{WHERE}                                 { return SemValue::keyword(WHERE); }
{UPDATE}                                { return SemValue::keyword(UPDATE); }
{SET}                                   { return SemValue::keyword(SET); }
{SELECT}                                { return SemValue::keyword(SELECT); }
{IS}                                    { return SemValue::keyword(IS); }
{TYPE_INT}                              { return SemValue::keyword(TYPE_INT); }
{TYPE_VARCHAR}                          { return SemValue::keyword(TYPE_VARCHAR); }
{DESC}                                  { return SemValue::keyword(DESC); }
{REFERENCES}                            { return SemValue::keyword(REFERENCES); }
{INDEX}                                 { return SemValue::keyword(INDEX); }
{AND}                                   { return SemValue::keyword(AND); }
{TYPE_DATE}                             { return SemValue::keyword(TYPE_DATE); }
{TYPE_FLOAT}                            { return SemValue::keyword(TYPE_FLOAT); }
{FOREIGN}                               { return SemValue::keyword(FOREIGN); }

{TYPE_CHAR}                             { return SemValue::keyword(TYPE_CHAR); }

{LE}                                    { return SemValue::keyword(LE); }
{GE}                                    { return SemValue::keyword(GE); }
{NE}                                    { return SemValue::keyword(NE); }
{LIKE}                                  { return SemValue::keyword(LIKE); }
{OPERATOR}                              { return *yytext; }

{IDENTIFIER}                            {
                                            yylval = SemValue();
                                            yylval.code = IDENTIFIER;
                                            int len = strlen(yytext);
                                            yylval.id = new char[len + 1];
                                            memcpy(yylval.id, yytext, len);
                                            yylval.id[len] = '\0';
                                            return IDENTIFIER;
                                        }

{VALUE_INT}                             {
                                            yylval = SemValue();
                                            yylval.code = VALUE_INT;
                                            yylval.value = (void*)(new int(atoi(yytext)));
                                            return VALUE_INT;
                                        }

{VALUE_INT}\.{NUMBER}                   {
                                            yylval = SemValue();
                                            yylval.code = VALUE_FLOAT;
                                            yylval.value = (void*)(new float(atof(yytext)));
                                            return VALUE_FLOAT;
                                        }

{VALUE_INT}\.{NUMBER}[Ee]{VALUE_INT}    {
                                            yylval = SemValue();
                                            yylval.code = VALUE_FLOAT;
                                            yylval.value = (void*)(new float(atof(yytext)));
                                            return VALUE_FLOAT;
                                        }

{VALUE_STRING}                          {
                                            yylval = SemValue();
                                            yylval.code = VALUE_STRING;
                                            int len = strlen(yytext) - 2;
                                            char *chars = new char[len + 1];
                                            memcpy(chars, yytext + 1, len);
                                            chars[len] = '\0';
                                            yylval.value = (void*)chars;
                                            return VALUE_STRING;
                                        }

.                                       {
                                            return 256;
                                        }
%%
