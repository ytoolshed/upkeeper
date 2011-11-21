/* ***************************************************************************
 * Copyright (c) 2011 Yahoo! Inc. All rights reserved. Licensed under the
 * Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License
 * at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable
 * law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 * See accompanying LICENSE file. 
 ************************************************************************** */

#include <upkeeper/upk_json.h>
#include <stdio.h>

int
main()
{
    char                    string[] =
        "//test comment\n/* more difficult test case */ { \"glossary\": { \"title\": \"example glossary\", \"pageCount\": 100, \"GlossDiv\": { \"title\": \"S\", \"GlossList\": [ { \"ID\": \"SGML\", \"SortAs\": \"SGML\", \"GlossTerm\": \"Standard Generalized Markup Language\", \"Acronym\": \"SGML\", \"Abbrev\": \"ISO 8879:1986\", \"GlossDef\": \"A meta-markup language, used to create markup languages such as DocBook.\", \"GlossSeeAlso\": [\"GML\", \"XML\", \"markup\"] } ] } }, \"foobar\":Null, \"categories\":[ [\"zero-zero\", [\"zero-one-zero\", \"zero-one-one\" ], \"zero-two\"], \"one\", \"two\"], \n# Hello world!\n \"other\":[\"zero\", [\"one-zero\",\"one-one\"], \"two\"] }";

    printf("JSON string: %s\n", string);
    upk_json_parse_string(string);
}
