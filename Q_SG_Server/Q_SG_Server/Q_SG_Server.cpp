// Q_SG_Server.cpp : �������̨Ӧ�ó������ڵ㡣
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

