#include "upk_json.h"

int
main()
{
    char                    string[] =
        "//test comment\n/* more difficult test case */ { \"glossary\": { \"title\": \"example glossary\", \"pageCount\": 100, \"GlossDiv\": { \"title\": \"S\", \"GlossList\": [ { \"ID\": \"SGML\", \"SortAs\": \"SGML\", \"GlossTerm\": \"Standard Generalized Markup Language\", \"Acronym\": \"SGML\", \"Abbrev\": \"ISO 8879:1986\", \"GlossDef\": \"A meta-markup language, used to create markup languages such as DocBook.\", \"GlossSeeAlso\": [\"GML\", \"XML\", \"markup\"] } ] } }, \"foobar\":Null, \"categories\":[ [\"zero-zero\", [\"zero-one-zero\", \"zero-one-one\" ], \"zero-two\"], \"one\", \"two\"], \n# Hello world!\n \"other\":[\"zero\", [\"one-zero\",\"one-one\"], \"two\"] }";

    printf("JSON string: %s\n", string);
    upk_json_parse_string(string);
}
