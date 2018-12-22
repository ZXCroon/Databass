%{
#include <string.h>
#include "y.tab.h"
%}
letter ([A-Za-z])
digit ([0-9])
value_int ({digit}+)
value_string ('[^']*')
identifier ([A-Za-z][_0-9A-Za-z]*)
exit ("EXIT"|"exit")
operator ("<="|">="|"<>"|"<"|">"|"="|","|"."|";"|"*"|"("|")")

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

INT ("INT"|"int")
CHAR ("CHAR"|"char")
VARCHAR ("VARCHAR"|"varchar")

whitespace ([ \t]+)
newline (\r|\n|\r\n)
%x comment
%%

"/*"                { BEGIN(comment); }
<comment>[^*]       { /* ignore comment */ }
<comment>"*/"       { BEGIN(INITIAL); }
<comment>\*         { /* ignore * */ }


%%
int yywrap() {
    return 1;
}