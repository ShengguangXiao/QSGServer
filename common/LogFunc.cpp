#include <time.h>
#include <stdio.h>
#include <string.h>
#include "LogFunc.h"
#include <string>
using namespace std;

#ifndef MAX_PATH
#define MAX_PATH				255
#endif

const size_t MAX_FILE_SIZE =		2 * 1024 * 1024;		//Max file size 2M, other wise backup

static char gszFileName[MAX_PATH] = "";

void SetLogFileName(char *szName)
{
	_snprintf_s(gszFileName, MAX_PATH, szName );
}

void _LogToFile(const char *szMsg, char *szFile, int line)
{
	if ( strlen ( gszFileName ) <= 0 )
		return;

	char szText[500];

	time_t tTime;
	tm *tmTemp;
	time ( &tTime );
	tmTemp = localtime ( &tTime );
	char szTime[100];
	_snprintf ( szTime, 100, "%d-%d-%d, %02d:%02d:%02d", tmTemp->tm_year + 1900,tmTemp->tm_mon + 1, tmTemp->tm_mday,
		tmTemp->tm_hour, tmTemp->tm_min, tmTemp->tm_sec );
	_snprintf ( szText, 500, "%s \t Sys Time: \t %s File %s Line %d\n", szMsg, szTime, szFile, line );

	FILE *pFile = fopen(gszFileName, "a");
	if ( NULL == pFile )
		return;
	fseek ( pFile, 0, SEEK_END );
	fprintf ( pFile, szText );

	size_t nFileSize = ftell ( pFile );
	bool bNeedToBackUp = false;
	if ( nFileSize > MAX_FILE_SIZE )
		bNeedToBackUp = true;

	printf ( szText );
	fclose ( pFile );

	if ( bNeedToBackUp )	{
		string szBackupFilePath = gszFileName;
		size_t nPos = szBackupFilePath.find_last_of('.');
		string szFileName = szBackupFilePath.substr(0, nPos );
		string szFileExt = szBackupFilePath.substr(nPos, szBackupFilePath.length() );
		_snprintf ( szTime, 100, "_%d-%d-%d %02d_%02d_%02d", tmTemp->tm_year + 1900,tmTemp->tm_mon + 1, tmTemp->tm_mday,
		tmTemp->tm_hour, tmTemp->tm_min, tmTemp->tm_sec );
		szBackupFilePath = szFileName + szTime + szFileExt;
		rename ( gszFileName, szBackupFilePath.c_str() );
	}
}