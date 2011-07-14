#ifndef _UPK_UTIL_H
#define _UPK_UTIL_H
/* upkeeper/upk_util.c */
extern bool upk_numeric_string(const char *string, long *num);
extern bool upk_boolean_string(const char *string, bool *val);
void upk_replace_string(char **haystack, const char *needle, const char *repl);

#endif
