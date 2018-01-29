// Q_SG_Server.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "SocketFunc.h"
#include "..\..\common\LogFunc.h"

int _tmain(int argc, _TCHAR* argv[])
{
	SetLogFileName ( ".\\Log\\QSGServer.log" );
	StartSocket();
	return 0;
}

