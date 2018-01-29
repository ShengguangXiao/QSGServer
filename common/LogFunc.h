#ifndef _LOG_FUNC_H_
#define _LOG_FUNC_H_

void SetLogFileName(char *szName);
void _LogToFile(const char *szMsg, char *file, int line);

#define LogToFile(szMsg)			_LogToFile ( szMsg, __FILE__, __LINE__ )

#endif