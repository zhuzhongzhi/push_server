
#ifndef _X_UTIL_H_
#define _X_UTIL_H_
#include <list>
#include "zbasedef.h"

char * safe_copy_str(char *dst, const char* src, int maxlen);
bool create_dir(const char* pszPath);
bool create_dir(const TPath& path);
int  zsnprintf(char*& str, size_t size, const char *format, ...)FORMAT_CHECK(printf, 3, 4);
int  zvsnprintf(char*& str, size_t size, const char *format, va_list& ap);
void trim_space(STLString& str);
int read_line(int fd, STLString& strData);
int split_string(const char * str, char filter, std::list<STLString>& out_list);

#endif

