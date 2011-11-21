
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
 ************************************************************************** */

#ifndef _UPK_UTIL_H
#define _UPK_UTIL_H
/* upkeeper/upk_util.c */
extern bool             upk_numeric_string(const char *string, long *num);
extern bool             upk_boolean_string(const char *string, bool * val);
extern void             upk_replace_string(char **haystack, const char *needle, const char *repl);
extern struct timeval   upk_double_to_timeval(long double rational);
int                     upk_rm_rf(char *start_path);

#define _stringify(A) #A
#define stringify(A) _stringify(A)


#endif
