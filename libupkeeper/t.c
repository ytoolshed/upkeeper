
/****************************************************************************
 * Copyright (c) 2011 Yahoo! Inc. All rights reserved. Licensed under the
 * Apache License, Version 2.0 (the "License"); you may not use this file
 * except in compliance with the License. You may obtain a copy of the License
 * at http://www.apache.org/licenses/LICENSE-2.0 Unless required by applicable
 * law or agreed to in writing, software distributed under the License is
 * distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the specific language
 * governing permissions and limitations under the License.
 * See accompanying LICENSE file. 
 ***************************************************************************/

#include <upkeeper.h>
#include "upkeeper/upk_json.h"

int
main()
{
    char                    string[] =
        "//test comment\n/* more difficult test case */ { \"glossary\": { \"title\": \"example glossary\", \"pageCount\": 100, \"GlossDiv\": { \"title\": \"S\", \"GlossList\": [ { \"ID\": \"SGML\", \"SortAs\": \"SGML\", \"GlossTerm\": \"Standard Generalized Markup Language\", \"Acronym\": \"SGML\", \"Abbrev\": \"ISO 8879:1986\", \"GlossDef\": \"A meta-markup language, used to create markup languages such as DocBook.\", \"GlossSeeAlso\": [\"GML\", \"XML\", \"markup\"] } ] } }, \"foobar\":Null, \"categories\":[ [\"zero-zero\", [\"zero-one-zero\", \"zero-one-one\" ], \"zero-two\"], \"/one\", \"two\"], \n// Hello world!\n \"other\":[\"zero\", [\"one-zero\",\"one-one\"], \"two\"] }";
    struct json_object     *obj = NULL;

    /* char svcstring[] =
       "{\"baz::foobar\":{\"Provides\":Null,\"UUID\":Null,\"ShortDescription\":Null,\"LongDescription\":Null,\"Prerequisites\":Null,\"StartPriority\":Null,\"BuddyShutdownTimeout\":Null,\"KillTimeout\":Null,\"MaxConsecutiveFailures\":Null,\"UserMaxRestarts\":Null,\"UserRestartWindow\":Null,\"UserRateLimit\":Null,\"RandomizeRateLimit\":Null,\"SetUID\":Null,\"SetGID\":Null,\"RingbufferSize\":Null,\"ReconnectRetries\":Null,\"ExecStart\":\"bash 
       -c \\\"while true; sleep 1;
       done\\\"\",\"StartScript\":Null,\"ExecStop\":Null,\"StopScript\":Null,\"ExecReload\":Null,\"ReloadScript\":Null,\"PipeStdoutScript\":Null,\"PipeStderrScript\":Null,\"RedirectStdout\":Null,\"RedirectStderr\":Null,\"InitialState\":\"running\",\"UnconfigureOnFileRemoval\":Null,\"PreferBuddyStateForStopped\":true,\"PreferBuddyStateForRunning\":Null,}, 
       \"quux::doo\":{\"ExecStart\":\"bash -c \\\"while true; sleep 1;
       done\\\"\",\"InitialState\":\"stopped\",\"PreferBuddyStateForRunning\":true,}}";

       upk_controller_config_t cfg; upk_svc_desc_head_t *svclist = NULL; */

    printf("%s\n", upk_default_configuration_str);
    return 0;
    upk_diag_verbosity = UPK_DIAGLVL_DEBUG1;
    upk_json_data_output_opts_t opts = {.pad = " ",.indent = "\t",.sep = "\n" };
    char                   *cp;

    obj = upk_json_parse_string(string);
    // upk_json_obj_to_stream(obj, stdout, opts);
    json_object_put(obj);

    obj = upk_json_parse_string(upk_default_configuration_str);
    char                   *p = upk_json_obj_to_string(obj, opts);

    json_object_put(obj);
    // printf("%s\n", p);

    obj = upk_json_parse_string(p);
    free(p);

    // upk_json_obj_to_stream(obj, stdout, opts);
    json_object_put(obj);

    upk_ctrl_load_config();

    printf("default: %lf\n", upk_default_configuration.BuddyPollingInterval);
    printf("default: %s\n", upk_default_configuration.UpkBuddyPath);
    printf("file: %lf\n", upk_file_configuration.BuddyPollingInterval);
    printf("file: %s\n", upk_file_configuration.UpkBuddyPath);
    printf("runtime: %lf\n", upk_runtime_configuration.BuddyPollingInterval);
    printf("runtime: %s\n", upk_runtime_configuration.UpkBuddyPath);
    cp = upk_json_serialize_svc_config(&upk_default_configuration.ServiceDefaults, opts);
    // printf("%s\n", cp);
    free(cp);

    cp = upk_json_serialize_svc_config(&upk_runtime_configuration.ServiceDefaults, opts);
    // printf("%s\n", cp);
    free(cp);

    upk_load_runtime_services();

    obj = upk_svclist_to_json_obj(upk_runtime_configuration.svclist);
    upk_json_obj_to_stream(obj, stdout, opts);
    json_object_put(obj);


    /* 
       cfg.svclist = calloc(1, sizeof(*cfg.svclist)); svclist = calloc(1, sizeof(*svclist));

       upk_svcconf_pack(&cfg, svcstring); UPKLIST_FOREACH(cfg.svclist) { UPKLIST_APPEND(svclist);
       upk_svc_desc_clear(svclist->thisp); upk_finalize_svc_desc(svclist->thisp, cfg.svclist->thisp); } obj = 
       upk_svclist_to_json_obj(svclist); upk_json_obj_to_stream(obj, stdout, opts); json_object_put(obj);

       upk_svclist_free(svclist); UPKLIST_FREE(svclist); UPKLIST_FREE(cfg.svclist); */

    upk_ctrl_free_config();
}
