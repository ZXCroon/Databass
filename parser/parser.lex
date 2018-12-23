%{
#include <string.h>
#include <cstdlib>
#include "SemValue.h"
#define YYSTYPE SemValue
#include "Parser.tab.h"
void yyerror(const char*);
%}
DIGIT ([0-9])
NUMBER ({DIGIT}+)
VALUE_INT ([+\-]?{NUMBER})
VALUE_STRING ('[^']*')

IDENTIFIER ([A-Za-z][_0-9A-Za-z]*)
EXIT ("EXIT"|"exit")
OPERATOR ("<="|">="|"<>"|"<"|">"|"="|","|"."|";"|"*"|"("|")")

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
NULL ("null"|"NULL")
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
INT ("int"|"INT")
VARCHAR ("varchar"|"VARCHAR")
DESC ("desc"|"DESC")
REFERENCES ("references"|"REFERENCES")
INDEX ("index"|"INDEX")
AND ("and"|"AND")
DATE ("date"|"DATE")
FLOAT ("float"|"FLOAT")
FOREIGN ("foreign"|"FOREIGN")

IS ("is"|"IS")

INT ("INT"|"int")
CHAR ("CHAR"|"char")
VARCHAR ("VARCHAR"|"varchar")

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
{NULL}                                  { return SemValue::keyword(NULL); }
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
{INT}                                   { return SemValue::keyword(INT); }
{VARCHAR}                               { return SemValue::keyword(VARCHAR); }
{DESC}                                  { return SemValue::keyword(DESC); }
{REFERENCES}                            { return SemValue::keyword(REFERENCES); }
{INDEX}                                 { return SemValue::keyword(INDEX); }
{AND}                                   { return SemValue::keyword(AND); }
{DATE}                                  { return SemValue::keyword(DATE); }
{FLOAT}                                 { return SemValue::keyword(FLOAT); }
{FOREIGN}                               { return SemValue::keyword(FOREIGN); }

{IS}                                    { return SemValue::keyword(IS); }

{INT}                                   { return SemValue::keyword(INT); }
{CHAR}                                  { return SemValue::keyword(CHAR); }
{VARCHAR}                               { return SemValue::keyword(VARCHAR); }

{OPERATOR}                              {
                                            yylval = SemValue();
                                            yylval.code = OPERATOR;
                                            yylval.opt = SemValue::opt(yytext);
                                            return OPERATOR;
                                        }

{IDENTIFIER}                            {
                                            yylval = SemValue();
                                            yylval.code = IDENTIFIER;
                                            yylval.id = yytext;
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
                                            yylval.value = (void*)yytext;
                                            return VALUE_STRING;
                                        }
%%
int yywrap() {
    return 1;
}