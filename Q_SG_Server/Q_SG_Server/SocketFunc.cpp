#include "stdafx.h"
#include <WinSock2.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <process.h>
#include "Constants.h"
#include "LogFunc.h"
#include "DataStruct.h"
#include "DBManager.h"
#include "Status.h"
#include "UtilityFunc.h"
#include <fstream>
#include <direct.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>

LIST_USER_CLIENT glistUserClient;
CRITICAL_SECTION gcsUserClient;
LIST_SHOP_CLIENT glistShopClient;
CRITICAL_SECTION gcsShopClient;

using namespace std;

static short AddUserInfoToList ( const short nUserID, const SOCKET socket )
{
	STT_USER_CLIENT_SOCKET stClient;
	stClient.socket = socket;
	stClient.nUserID = nUserID;

	EnterCriticalSection ( &gcsUserClient );

	LIST_USER_CLIENT::iterator it = glistUserClient.begin();
	while ( it != glistUserClient.end() )	{
		if ( it->nUserID == nUserID || it->socket == socket )	{
			it = glistUserClient.erase ( it );
		}else	{
			++ it;
		}
	}
	glistUserClient.push_back ( stClient );

	LeaveCriticalSection ( &gcsUserClient );
	return OK;
}

static short AddShopInfoToList ( const int nShopID, const SOCKET socket )
{
	STT_SHOP_CLIENT_SOCKET stClient;
	stClient.socket = socket;
	stClient.nShopID = nShopID;

	EnterCriticalSection ( &gcsShopClient );

	LIST_SHOP_CLIENT::iterator it = glistShopClient.begin();
	while ( it != glistShopClient.end() )	{
		if ( it->nShopID == nShopID || it->socket == socket )	{
			it = glistShopClient.erase ( it );
		}else	{
			++ it;
		}
	}
	glistShopClient.push_back ( stClient );

	LeaveCriticalSection ( &gcsShopClient );
	return OK;
}

static void RemoveUserInfoFromList ( const SOCKET socket )
{
	EnterCriticalSection ( &gcsUserClient );
	LIST_USER_CLIENT::iterator it = glistUserClient.begin();
	while ( it != glistUserClient.end() )	{
		if ( it->socket == socket )	{
			it = glistUserClient.erase ( it );
		}else	{
			++ it;
		}
	}
	LeaveCriticalSection ( &gcsUserClient );
}

static void RemoveShopInfoFromList ( const SOCKET socket )
{
	EnterCriticalSection ( &gcsShopClient );
	LIST_SHOP_CLIENT::iterator it = glistShopClient.begin();
	while ( it != glistShopClient.end() )	{
		if ( it->socket == socket )	{
			it = glistShopClient.erase ( it );
		}else	{
			++ it;
		}
	}
	LeaveCriticalSection ( &gcsShopClient );
}

static short HandleUserLogInData ( SOCKET socket, char *buffer, int nSize)
{
	STT_LOGIN_CMD stLoginCmd;
	int size = recv ( socket, buffer, sizeof ( stLoginCmd ), 0 );
	if ( sizeof ( stLoginCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stLoginCmd ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stLoginCmd, buffer, sizeof ( stLoginCmd ) );

	STT_USER_LOGIN_RPY  stLoginRpy;
	char szMsg[200];
	_snprintf ( szMsg, 200, "Log in with email %s, password %s", stLoginCmd.szEmail, stLoginCmd.szPassWord );
	LogToFile ( szMsg );
	short nStatus = OK;
	CDBManager dbManager;

	nStatus = dbManager.HandleUserLogIn ( stLoginCmd, stLoginRpy );
	if ( OK == nStatus )	{
		AddUserInfoToList ( stLoginRpy.nID, socket );
	}
	_snprintf ( szMsg, 200, "User log in status %d, UserID %d", nStatus, stLoginRpy.nID );
	LogToFile ( szMsg );

	if ( ! stLoginCmd.bNeedReply )
		return nStatus;

	STT_MSG_HEADER stMsgHeader;
	stMsgHeader.nCmdID = CMD_USER_LOGIN_RPY;
	stMsgHeader.nIsLastMsg = TRUE;
	stMsgHeader.nDataSize = sizeof ( stLoginRpy );
	stMsgHeader.nStatus = nStatus;

	char sendBuffer[BUFFER_SIZE];
	memcpy ( sendBuffer, &stMsgHeader, MSG_HEADER_SIZE );
	memcpy ( sendBuffer + MSG_HEADER_SIZE, &stLoginRpy, sizeof ( stLoginRpy ) );
	send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stLoginRpy ), 0 );
		
	return OK;
}

static short HandleShopLogInData ( SOCKET socket, char *buffer, int nSize)
{
	STT_LOGIN_CMD stLoginCmd;
	int size = recv ( socket, buffer, sizeof ( stLoginCmd ), 0 );
	if ( sizeof ( stLoginCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stLoginCmd ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stLoginCmd, buffer, sizeof ( stLoginCmd ) );

	STT_SHOP_LOGIN_RPY  stLoginRpy;
	char szMsg[200];
	_snprintf ( szMsg, 200, "Log in with email %s, password %s", stLoginCmd.szEmail, stLoginCmd.szPassWord );
	LogToFile ( szMsg );
	short nStatus = OK;
	CDBManager dbManager;

	nStatus = dbManager.HandleShopLogIn ( stLoginCmd, stLoginRpy );
	if ( OK == nStatus )	{
		int nQLen = 0;
		dbManager.InqueryShopQLen ( stLoginRpy.nID, nQLen );
		time_t tCurrentTime = time ( NULL );
		dbManager.UpdateShopActiveTime ( stLoginRpy.nID, tCurrentTime );
		AddShopInfoToList ( stLoginRpy.nID, socket );
	}
	_snprintf ( szMsg, 200, "Shop log in status %d, Result ShopID %d", nStatus, stLoginRpy.nID );
	LogToFile ( szMsg );

	if ( ! stLoginCmd.bNeedReply )
		return nStatus;

	STT_MSG_HEADER stMsgHeader;
	stMsgHeader.nCmdID = CMD_SHOP_LOGIN_RPY;
	stMsgHeader.nIsLastMsg = TRUE;
	stMsgHeader.nDataSize = 0;
	stMsgHeader.nStatus = nStatus;

	char sendBuffer[BUFFER_SIZE];
	memcpy ( sendBuffer, &stMsgHeader, MSG_HEADER_SIZE );
	memcpy ( sendBuffer + MSG_HEADER_SIZE, &stLoginRpy, sizeof ( stLoginRpy ) );
	send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stLoginRpy ), 0 );
		
	return OK;
}

static short HandleShopLogOut ( SOCKET socket, char *buffer, int nSize)
{
	STT_GET_SHOP_INFO_CMD stShopInfo;
	int size = recv ( socket, buffer, sizeof ( stShopInfo ), 0 );
	if ( sizeof ( stShopInfo ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stShopInfo ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stShopInfo, buffer, sizeof ( stShopInfo ) );

	short nStatus = OK;
	CDBManager dbManager;

	RemoveShopInfoFromList ( socket );
	nStatus = dbManager.HandleShopLogOut ( stShopInfo.nShopID );
	
	char szMsg[200];
	_snprintf ( szMsg, 200, "Shop log out status %d, ShopID %d", nStatus, stShopInfo.nShopID );
	printf ( "%s\n", szMsg );
	LogToFile ( szMsg );

	STT_MSG_HEADER stMsgHeader;
	stMsgHeader.nCmdID = CMD_SHOP_LOG_OUT_RPY;
	stMsgHeader.nIsLastMsg = TRUE;
	stMsgHeader.nDataSize = 0;
	stMsgHeader.nStatus = nStatus;

	char sendBuffer[BUFFER_SIZE];
	memcpy ( sendBuffer, &stMsgHeader, MSG_HEADER_SIZE );
	send ( socket, sendBuffer, MSG_HEADER_SIZE, 0 );

	return OK;
}

static bool IsShopOnline( const int nShopID )
{
	bool bOnline = false;
	
	EnterCriticalSection ( &gcsShopClient );
	LIST_SHOP_CLIENT::iterator it = glistShopClient.begin();
	while ( it != glistShopClient.end() )	{
		if ( it->nShopID == nShopID )	{
			bOnline = true;
			break;
		}
		++ it;
	}
	LeaveCriticalSection ( &gcsShopClient );
	
	if ( ! bOnline )	{
		CDBManager dbManager;
		time_t tActiveTime = 0;
		bool bLogginIn = false;
		dbManager.GetShopActiveTime ( nShopID, bLogginIn, tActiveTime );
		if ( ! bLogginIn )
			return false;

		time_t tCurrent = time ( NULL );
		//If last active time is recently, then still consider shop is online
		if ( tCurrent - tActiveTime < QSG_SHOP_OFFLINE_TIME_THRES )
			bOnline = true;
	}
	return bOnline;
}

static short GetSocketByUserID ( const short nUserID, STT_USER_CLIENT_SOCKET &stUserClient )
{
	short nStatus = NOK;
	EnterCriticalSection ( &gcsUserClient );
	for ( LIST_USER_CLIENT::iterator it = glistUserClient.begin(); it != glistUserClient.end(); ++ it )
	{
		if ( nUserID == it->nUserID )	{
			stUserClient = *it;
			nStatus = OK;
			break;
		}
	}
	LeaveCriticalSection ( &gcsUserClient );
	return nStatus;
}

static short GetSocketByShopID ( const short nShopID, STT_SHOP_CLIENT_SOCKET &stShopClient )
{
	short nStatus = NOK;
	EnterCriticalSection ( &gcsShopClient );
	for ( LIST_SHOP_CLIENT::iterator it = glistShopClient.begin(); it != glistShopClient.end(); ++ it )
	{
		if ( nShopID == it->nShopID )	{
			stShopClient = *it;
			nStatus = OK;
			break;
		}
	}
	LeaveCriticalSection ( &gcsShopClient );
	return nStatus;
}

static short HandleRegisterUser ( int socket, char *buffer, int nSize )
{
	STT_REGISTER_USER_INFO stRegisterUserInfo;
	int size = recv ( socket, buffer, sizeof ( stRegisterUserInfo ), 0 );
	if ( sizeof ( stRegisterUserInfo ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stRegisterUserInfo ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stRegisterUserInfo, buffer, sizeof ( stRegisterUserInfo ) );

	CDBManager dbManager;
	short nStatus = dbManager.RegisterUser ( stRegisterUserInfo );

	STT_MSG_HEADER stMsgHeader;
	stMsgHeader.nCmdID = CMD_REGISTER_USER_RPY;
	stMsgHeader.nIsLastMsg = TRUE;
	stMsgHeader.nDataSize = 0;
	stMsgHeader.nStatus = nStatus;

	send ( socket, (char *)&stMsgHeader, sizeof ( STT_MSG_HEADER ), 0 );
	return OK;
}

static short HandleRegisterShop ( int socket, char *buffer, int nSize )
{
	STT_REGISTER_SHOP_INFO stRegisterShopInfo;
	Sleep(200);
	int size = recv ( socket, buffer, sizeof ( stRegisterShopInfo ), 0 );
	if ( sizeof ( stRegisterShopInfo ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stRegisterShopInfo ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stRegisterShopInfo, buffer, sizeof ( stRegisterShopInfo ) );

	CDBManager dbManager;
	short nStatus = dbManager.RegisterShop ( stRegisterShopInfo );

	STT_MSG_HEADER stMsgHeader;
	stMsgHeader.nCmdID = CMD_REGISTER_SHOP_RPY;
	stMsgHeader.nIsLastMsg = TRUE;
	stMsgHeader.nDataSize = 0;
	stMsgHeader.nStatus = nStatus;

	send ( socket, (char *)&stMsgHeader, sizeof ( STT_MSG_HEADER ), 0 );
	return OK;
}

static short HandleUpdateShopProfile ( int socket, char *buffer, int nSize )
{
	STT_UPDATE_SHOP_PROFILE_CMD stShopProfile;
	int size = recv ( socket, buffer, sizeof ( stShopProfile ), 0 );
	if ( sizeof ( stShopProfile ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stShopProfile ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stShopProfile, buffer, sizeof ( stShopProfile ) );

	short nStatus = OK;
	CDBManager dbManager;
	if ( stShopProfile.nID <= 0 || strlen ( stShopProfile.szAddress ) < 1 || strlen ( stShopProfile.szName ) < 1 )	{
		nStatus = QSG_STATUS_INVALID_PARAM;
		goto EXIT;
	}
	nStatus = dbManager.UpdateShopProfile ( stShopProfile );

	time_t tCurrentTime = time ( NULL );
	dbManager.UpdateShopActiveTime ( stShopProfile.nID, tCurrentTime );

EXIT:
	STT_MSG_HEADER stMsgHeader;
	stMsgHeader.nCmdID = CMD_UPDATE_SHOP_PROFILE_RPY;
	stMsgHeader.nIsLastMsg = TRUE;
	stMsgHeader.nDataSize = 0;
	stMsgHeader.nStatus = nStatus;

	send ( socket, (char *)&stMsgHeader, MSG_HEADER_SIZE, 0 );
	return OK;
}

static short HandleGetAllShop ( int socket, char *buffer, int nSize )
{
	CDBManager dbManager;
	LIST_GET_SHOP_SUMMARY lstShop;
	short nStatus = dbManager.GetShops ( lstShop );

	char sendBuffer[BUFFER_SIZE];
	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_GET_ALL_SHOP_RPY;
	stHeader.nStatus = nStatus;
	stHeader.nIsLastMsg = FALSE;
	if ( OK == nStatus ){
		for ( LIST_GET_SHOP_SUMMARY::iterator it = lstShop.begin(); it != lstShop.end(); ++ it )	{
			STT_GET_SHOP_SUMMARY_RPY stGetShopInfo = *it;
			
			stGetShopInfo.bOnline = IsShopOnline ( stGetShopInfo.nID );
			if ( stGetShopInfo.bOnline )	{
				int nQLen = 0;
				dbManager.InqueryShopQLen ( stGetShopInfo.nID, nQLen );
				stGetShopInfo.nQLen = nQLen;
			}else	{
				stGetShopInfo.nQLen = 0;
			}
			stHeader.nDataSize = sizeof ( stGetShopInfo );
			if ( it == std::prev ( lstShop.end() ) )
				stHeader.nIsLastMsg = TRUE;
			memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
			memcpy ( sendBuffer + MSG_HEADER_SIZE, &stGetShopInfo, sizeof ( stGetShopInfo ) );

			send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stGetShopInfo ), 0 );
		}
	}else	{
		stHeader.nIsLastMsg = TRUE;
		stHeader.nDataSize = 0;
		memcpy ( sendBuffer, &stHeader, sizeof ( STT_MSG_HEADER ) );
		send ( socket, sendBuffer, sizeof ( STT_MSG_HEADER ), 0 );
	}

	return OK;
}

static short HandleGetRecommendedShop ( int socket, char *buffer, int nSize )
{
	CDBManager dbManager;
	LIST_GET_SHOP_SUMMARY lstShop;
	short nStatus = dbManager.GetRecommendedShop ( lstShop );

	char sendBuffer[BUFFER_SIZE];
	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_GET_RECOMMENDED_SHOP_RPY;
	stHeader.nStatus = nStatus;
	stHeader.nIsLastMsg = FALSE;
	if ( OK == nStatus ){
		for ( LIST_GET_SHOP_SUMMARY::iterator it = lstShop.begin(); it != lstShop.end(); ++ it )	{
			STT_GET_SHOP_SUMMARY_RPY stGetShopInfo = *it;
			
			stGetShopInfo.bOnline = IsShopOnline ( stGetShopInfo.nID );
			if ( stGetShopInfo.bOnline )	{
				int nQLen = 0;
				dbManager.InqueryShopQLen ( stGetShopInfo.nID, nQLen );
				stGetShopInfo.nQLen = nQLen;
			}else	{
				stGetShopInfo.nQLen = 0;
			}
			stHeader.nDataSize = sizeof ( STT_GET_SHOP_SUMMARY_RPY );
			if ( it == std::prev ( lstShop.end() ) )
				stHeader.nIsLastMsg = TRUE;
			memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
			memcpy ( sendBuffer + sizeof ( STT_MSG_HEADER ), &stGetShopInfo, sizeof ( struct STT_GET_SHOP_SUMMARY_RPY ) );
			send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( struct STT_GET_SHOP_SUMMARY_RPY ), 0 );
			Sleep(1);
		}
	}else	{
		stHeader.nIsLastMsg = TRUE;
		stHeader.nDataSize = 0;
		memcpy ( sendBuffer, &stHeader, sizeof ( STT_MSG_HEADER ) );
		send ( socket, sendBuffer, sizeof ( STT_MSG_HEADER ), 0 );
	}

	return OK;
}

static short HandleGetShop ( int socket, char *buffer, int nSize )
{
	STT_GET_SHOP_INFO_CMD stGetShopInfoCmd;
	int size = recv ( socket, buffer, sizeof ( stGetShopInfoCmd ), 0 );
	if ( sizeof ( stGetShopInfoCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stGetShopInfoCmd ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stGetShopInfoCmd, buffer, sizeof ( stGetShopInfoCmd ) );

	CDBManager dbManager;
	STT_GET_SHOP_DETAIL_RPY stShopDetail;
	short nStatus = dbManager.GetShopInfoByID ( stGetShopInfoCmd.nShopID, stShopDetail );

	stShopDetail.bOnline = IsShopOnline ( stShopDetail.nID );
	if ( stShopDetail.bOnline )	{
		int nQLen = 0;
		dbManager.InqueryShopQLen ( stShopDetail.nID, nQLen );
		stShopDetail.nQLen = nQLen;
	}else	{
		stShopDetail.nQLen = 0;
	}

	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_GET_SHOP_BY_ID_RPY;
	stHeader.nIsLastMsg = TRUE;
	stHeader.nDataSize = sizeof ( stShopDetail );

	char sendBuffer [ BUFFER_SIZE ];
	memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
	memcpy ( sendBuffer + MSG_HEADER_SIZE, &stShopDetail, sizeof ( stShopDetail ) );

	send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stShopDetail ), 0 );
	return OK;
}

static short HandleGetShopByEmail ( int socket, char *buffer, int nSize )
{
	STT_GET_SHOP_BY_EMAIL_CMD stGetShopByEmailCmd;
	int size = recv ( socket, buffer, sizeof ( stGetShopByEmailCmd ), 0 );
	if ( sizeof ( stGetShopByEmailCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stGetShopByEmailCmd ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stGetShopByEmailCmd, buffer, sizeof ( stGetShopByEmailCmd ) );

	CDBManager dbManager;
	STT_GET_SHOP_DETAIL_RPY stShopDetail;
	short nStatus = dbManager.GetShopInfoByEmail ( stGetShopByEmailCmd, stShopDetail );

	stShopDetail.bOnline = IsShopOnline ( stShopDetail.nID );
	if ( stShopDetail.bOnline )	{
		int nQLen = 0;
		dbManager.InqueryShopQLen ( stShopDetail.nID, nQLen );
		stShopDetail.nQLen = nQLen;
	}else	{
		stShopDetail.nQLen = 0;
	}

	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_GET_SHOP_BY_EMAIL_RPY;
	stHeader.nIsLastMsg = TRUE;
	stHeader.nDataSize = sizeof ( stShopDetail );
	stHeader.nStatus = nStatus;

	char sendBuffer [ BUFFER_SIZE ];
	memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
	memcpy ( sendBuffer + MSG_HEADER_SIZE, &stShopDetail, sizeof ( stShopDetail ) );

	send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stShopDetail ), 0 );
	return OK;
}

static short HandleSrchShop ( int socket, char *buffer, int nSize )
{
	STT_SRCH_SHOP_CMD stSrchShopCmd;
	int size = recv ( socket, buffer, sizeof ( stSrchShopCmd ), 0 );
	if ( sizeof ( stSrchShopCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stSrchShopCmd ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stSrchShopCmd, buffer, sizeof ( stSrchShopCmd ) );

	if ( strlen ( stSrchShopCmd.szSrchText ) <= 0 )	{
		return NOK;
	}

	CDBManager dbManager;
	LIST_GET_SHOP_SUMMARY lstShop;
	short nStatus = dbManager.SrchShop ( stSrchShopCmd, lstShop );

	if ( lstShop.size() <= 0 )
		nStatus = QSG_STATUS_NO_MATCH_RESULT;

	char sendBuffer[BUFFER_SIZE];
	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_SRCH_SHOP_RPY;
	stHeader.nStatus = nStatus;
	stHeader.nIsLastMsg = FALSE;
	if ( OK == nStatus ){
		for ( LIST_GET_SHOP_SUMMARY::iterator it = lstShop.begin(); it != lstShop.end(); ++ it )	{
			STT_GET_SHOP_SUMMARY_RPY stGetShopInfo = *it;
			
			stGetShopInfo.bOnline = IsShopOnline ( stGetShopInfo.nID );
			if ( stGetShopInfo.bOnline )	{
				int nQLen = 0;
				dbManager.InqueryShopQLen ( stGetShopInfo.nID, nQLen );
				stGetShopInfo.nQLen = nQLen;
			}else	{
				stGetShopInfo.nQLen = 0;
			}
			stHeader.nDataSize = sizeof ( STT_GET_SHOP_SUMMARY_RPY );
			if ( it == std::prev ( lstShop.end() ) )
				stHeader.nIsLastMsg = TRUE;
			memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
			memcpy ( sendBuffer + MSG_HEADER_SIZE, &stGetShopInfo, sizeof ( stGetShopInfo ) );

			send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stGetShopInfo ), 0 );
		}
	}else	{
		stHeader.nIsLastMsg = TRUE;
		stHeader.nDataSize = 0;
		memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
		send ( socket, sendBuffer, MSG_HEADER_SIZE, 0 );
	}
	return OK;
}

static short HandleGetNearByShop ( int socket, char *buffer, int nSize )
{
	STT_GET_NEAR_BY_SHOP_CMD stGetNearByShopCmd;
	int size = recv ( socket, buffer, sizeof ( stGetNearByShopCmd ), 0 );
	if ( sizeof ( stGetNearByShopCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stGetNearByShopCmd ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stGetNearByShopCmd, buffer, sizeof ( stGetNearByShopCmd ) );

	CDBManager dbManager;
	LIST_GET_SHOP_SUMMARY lstShop;
	short nStatus = dbManager.GetShops ( lstShop );

	if ( lstShop.size() <= 0 )
		nStatus = QSG_STATUS_NO_MATCH_RESULT;

	vector<_STT_SHOP_DISTANCE> vecShopDistance;
	vector<STT_GET_SHOP_SUMMARY_RPY> vecShop;
	int nIndex = 0;
	for ( LIST_GET_SHOP_SUMMARY::iterator it = lstShop.begin(); it != lstShop.end(); ++ it )	{
		_STT_SHOP_DISTANCE stShopDistance;
		stShopDistance.dDistance = CalculateDistanceOfLocation ( stGetNearByShopCmd.dCurrentLatitude, stGetNearByShopCmd.dCurrentLongitude,
			it->dLatidute, it->dLongitude );
		printf("Shop ID %d, distance %f\n", it->nID, stShopDistance.dDistance );
		stShopDistance.nShopIndex = nIndex;
		vecShopDistance.push_back ( stShopDistance );
		it->fDistance = (float)stShopDistance.dDistance;
		vecShop.push_back ( *it );
		++ nIndex;
	}

	sort ( vecShopDistance.begin(), vecShopDistance.end() );

	LIST_GET_SHOP_SUMMARY lstNearByShop;
	for ( size_t nShop = 0; nShop < vecShopDistance.size() && nShop < QSG_MAX_NEAR_BY_SHOP_NUM; ++ nShop )	{
		_STT_SHOP_DISTANCE stShopDistance = vecShopDistance[nShop];
		short nShopIndx = stShopDistance.nShopIndex;
		lstNearByShop.push_back ( vecShop[ nShopIndx ] );
		printf("Output Shop ID %d, distance %f\n", vecShop[ nShopIndx ].nID, stShopDistance.dDistance );
	}

	char sendBuffer[BUFFER_SIZE];
	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_GET_NEAR_BY_SHOP_RPY;
	stHeader.nStatus = nStatus;
	stHeader.nIsLastMsg = FALSE;
	if ( OK == nStatus ){
		for ( LIST_GET_SHOP_SUMMARY::iterator it = lstNearByShop.begin(); it != lstNearByShop.end(); ++ it )	{
			STT_GET_SHOP_SUMMARY_RPY stGetShopInfo = *it;
			
			stGetShopInfo.bOnline = IsShopOnline ( stGetShopInfo.nID );
			if ( stGetShopInfo.bOnline )	{
				int nQLen = 0;
				dbManager.InqueryShopQLen ( stGetShopInfo.nID, nQLen );
				stGetShopInfo.nQLen = nQLen;
			}else	{
				stGetShopInfo.nQLen = 0;
			}
			stHeader.nDataSize = sizeof ( STT_GET_SHOP_SUMMARY_RPY );
			if ( it == std::prev ( lstNearByShop.end() ) )
				stHeader.nIsLastMsg = TRUE;
			memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
			int nSizeOfShop = sizeof ( stGetShopInfo );
			memcpy ( sendBuffer + MSG_HEADER_SIZE, &stGetShopInfo, nSizeOfShop );
			send ( socket, sendBuffer, MSG_HEADER_SIZE + nSizeOfShop, 0 );
			Sleep ( 1 );
		}
	}else	{
		stHeader.nIsLastMsg = TRUE;
		stHeader.nDataSize = 0;
		memcpy ( sendBuffer, &stHeader, sizeof ( STT_MSG_HEADER ) );
		send ( socket, sendBuffer, sizeof ( STT_MSG_HEADER ), 0 );
	}
	return OK;
}

static short HandleGetPopularShop ( int socket, char *buffer, int nSize )
{
	CDBManager dbManager;
	LIST_GET_SHOP_SUMMARY lstShop;
	short nStatus = dbManager.GetPopularShop ( lstShop );

	if ( lstShop.size() <= 0 )
		nStatus = QSG_STATUS_NO_MATCH_RESULT;

	char sendBuffer[BUFFER_SIZE];
	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_GET_POPULAR_SHOP_RPY;
	stHeader.nStatus = nStatus;
	stHeader.nIsLastMsg = FALSE;

	if ( OK == nStatus ){
		for ( LIST_GET_SHOP_SUMMARY::iterator it = lstShop.begin(); it != lstShop.end(); ++ it )	{
			STT_GET_SHOP_SUMMARY_RPY stGetShopInfo = *it;
			
			stGetShopInfo.bOnline = IsShopOnline ( stGetShopInfo.nID );
			if ( stGetShopInfo.bOnline )	{
				int nQLen = 0;
				dbManager.InqueryShopQLen ( stGetShopInfo.nID, nQLen );
				stGetShopInfo.nQLen = nQLen;
			}else	{
				stGetShopInfo.nQLen = 0;
			}

			stHeader.nDataSize = sizeof ( STT_GET_SHOP_SUMMARY_RPY );
			if ( it == std::prev ( lstShop.end() ) )
				stHeader.nIsLastMsg = TRUE;
			memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
			memcpy ( sendBuffer + MSG_HEADER_SIZE, &stGetShopInfo, sizeof ( stGetShopInfo ) );

			send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stGetShopInfo ), 0 );
		}
	}else	{
		stHeader.nIsLastMsg = TRUE;
		stHeader.nDataSize = 0;
		memcpy ( sendBuffer, &stHeader, sizeof ( STT_MSG_HEADER ) );
		send ( socket, sendBuffer, sizeof ( STT_MSG_HEADER ), 0 );
	}
	return OK;
}

static short HandleGetFavoriteShop ( int socket, char *buffer, int nSize )
{
	STT_GET_FAVORITE_SHOP_CMD stGetFavoriteCmd;
	int size = recv ( socket, buffer, sizeof ( stGetFavoriteCmd ), 0 );
	if ( sizeof ( stGetFavoriteCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stGetFavoriteCmd ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stGetFavoriteCmd, buffer, sizeof ( stGetFavoriteCmd ) );

	CDBManager dbManager;
	LIST_GET_SHOP_SUMMARY lstShop;
	short nStatus = dbManager.GetFavoriteShop ( stGetFavoriteCmd, lstShop );

	if ( lstShop.size() <= 0 )
		nStatus = QSG_STATUS_NO_MATCH_RESULT;

	char sendBuffer[BUFFER_SIZE];
	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_GET_FAVORITE_SHOP_RPY;
	stHeader.nStatus = nStatus;
	stHeader.nIsLastMsg = FALSE;

	if ( OK == nStatus ){
		for ( LIST_GET_SHOP_SUMMARY::iterator it = lstShop.begin(); it != lstShop.end(); ++ it )	{
			STT_GET_SHOP_SUMMARY_RPY stGetShopInfo = *it;
			
			stGetShopInfo.bOnline = IsShopOnline ( stGetShopInfo.nID );
			if ( stGetShopInfo.bOnline )	{
				int nQLen = 0;
				dbManager.InqueryShopQLen ( stGetShopInfo.nID, nQLen );
				stGetShopInfo.nQLen = nQLen;
			}else	{
				stGetShopInfo.nQLen = 0;
			}

			stHeader.nDataSize = sizeof ( stGetShopInfo );
			if ( it == std::prev ( lstShop.end() ) )
				stHeader.nIsLastMsg = TRUE;
			memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
			memcpy ( sendBuffer + MSG_HEADER_SIZE, &stGetShopInfo, sizeof ( stGetShopInfo ) );

			send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stGetShopInfo ), 0 );
			Sleep ( 1 );
		}
	}else	{
		stHeader.nIsLastMsg = TRUE;
		stHeader.nDataSize = 0;
		memcpy ( sendBuffer, &stHeader, sizeof ( STT_MSG_HEADER ) );
		send ( socket, sendBuffer, sizeof ( STT_MSG_HEADER ), 0 );
	}
	return OK;
}

static short HandleQ ( int socket, char *buffer, int nSize )
{
	STT_Q_USER_TO_SERVER_INFO stQInfo;
	int size = recv ( socket, buffer, sizeof ( stQInfo ), 0 );
	if ( sizeof ( stQInfo ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stQInfo ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stQInfo, buffer, sizeof ( stQInfo ) );

	char sendBuffer[BUFFER_SIZE];
	STT_MSG_HEADER stHeader;

	STT_Q_SERVER_TO_USER_RPY stQServerToUserInfo;
	stQServerToUserInfo.nUserID = stQInfo.nUserID;
	stQServerToUserInfo.nShopID = stQInfo.nShopID;

	CDBManager dbManager;
	short nStatus = OK;
	int nQNo = 0;
	STT_USER_INQUERY_Q_RPY stInqueryQRpy;
	nStatus = dbManager.InqueryQNo ( stQInfo.nUserID, stInqueryQRpy );
	//Already in Q
	if ( OK == nStatus )	{
		stQServerToUserInfo.bAccept = false;
		stQServerToUserInfo.nQNo = stInqueryQRpy.nQNo;
		_snprintf ( stQServerToUserInfo.szShopName, sizeof ( stQServerToUserInfo.szShopName), stInqueryQRpy.szShopName  );

		stHeader.nCmdID = CMD_Q_RPY;
		stHeader.nStatus = QSG_STATUS_ALREADY_HAVE_Q;
		stHeader.nIsLastMsg = TRUE;

		memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
		memcpy ( sendBuffer + MSG_HEADER_SIZE, &stQServerToUserInfo, sizeof ( stQServerToUserInfo ) );
		send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stQServerToUserInfo ), 0 );
		return NOK;
	}

	if ( ! IsShopOnline ( stQInfo.nShopID ) )	{
		nStatus = QSG_STATUS_SHOP_OFFLINE;
		goto EXIT;
	}

	nStatus = dbManager.IsUserCancelQExceedLimit ( stQInfo.nUserID );
	if ( nStatus != OK )
		goto EXIT;

	nStatus = dbManager.AddToQ ( stQInfo, stQServerToUserInfo.nQNo );
	if ( nStatus != OK )
		goto EXIT;

	nStatus = dbManager.InqueryQNo ( stQInfo.nUserID, stQInfo.nShopID, stQServerToUserInfo.nQNo, stQServerToUserInfo.nQInfront );
	if ( nStatus != OK )
		goto EXIT;

	stQServerToUserInfo.bAccept = TRUE;

	//Send user information is shop is shop client socket is valid
	STT_Q_SERVER_TO_SHOP_INFO stQServerToShopInfo;
	stQServerToShopInfo.nShopID = stQInfo.nShopID;
	stQServerToShopInfo.nUserID = stQInfo.nUserID;
	stQServerToShopInfo.nCustomerNum = stQInfo.nCustomerNum;
	stQServerToShopInfo.nQNo = stQServerToUserInfo.nQNo;
	time_t tTime = 0;
	time( &tTime );
	tm *pstTime = localtime( &tTime );

	//tm_year   The number of years since 1900.
	_snprintf ( stQServerToShopInfo.szQTime, 50, "%d-%02d-%02d %02d:%02d:%02d", pstTime->tm_year + 1900, pstTime->tm_mon,
		pstTime->tm_mday, pstTime->tm_hour, pstTime->tm_min, pstTime->tm_sec );

	dbManager.InqueryUserInfo ( stQInfo.nUserID, stQServerToShopInfo );

	STT_SHOP_CLIENT_SOCKET stShopClient;
	if ( OK == GetSocketByShopID ( stQInfo.nShopID, stShopClient ) )	{
		stHeader.nCmdID = CMD_Q_TO_SHOP;
		stHeader.nStatus = 0;
		stHeader.nIsLastMsg = TRUE;
		memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
		memcpy ( sendBuffer + MSG_HEADER_SIZE, &stQServerToShopInfo, sizeof ( stQServerToShopInfo ) );
		send ( stShopClient.socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stQServerToShopInfo ), 0 );
	}

EXIT:
	stHeader.nCmdID = CMD_Q_RPY;
	stHeader.nStatus = nStatus;
	stHeader.nIsLastMsg = TRUE;
	memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
	memcpy ( sendBuffer + MSG_HEADER_SIZE, &stQServerToUserInfo, sizeof ( stQServerToUserInfo ) );
	send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stQServerToUserInfo ), 0 );

	//Because send push notification need longer time, so put it at the end
	if ( OK == nStatus )	{
		STT_DEVICE_TOKEN stDeviceToken;
		nStatus = dbManager.GetShopDeviceToken ( stQInfo.nShopID, stDeviceToken );
		if ( OK == nStatus && stDeviceToken.bDeviceTokenValid )	{
			char szPushMsg[200];
			_snprintf ( szPushMsg, 200, "You have new customer %s", stQServerToShopInfo.szUserName );
			SendPushNotificationToShop ( stDeviceToken.szDeviceToken, szPushMsg, 1 );
			char szLog[300];
			_snprintf ( szLog, 300, "Send notification to shop, device token %s", stDeviceToken.szDeviceToken );
			LogToFile ( szLog );
		}
	}
	return OK;
}

static short HandleShopAddCustomer ( int socket, char *buffer, int nSize )
{
	struct STT_Q_USER_TO_SERVER_INFO stQInfo;
	int size = recv ( socket, buffer, sizeof ( stQInfo ), 0 );
	if ( sizeof ( stQInfo ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stQInfo ) );
		LogToFile ( szLog );
		return NOK;
	}

	memcpy ( &stQInfo, buffer, sizeof ( stQInfo ) );

	struct STT_MSG_HEADER stMsgHeader;
	struct STT_SHOP_ADD_CUSTOMER_RPY stAddCustomerRpy;

	CDBManager dbManager;
	short nStatus = dbManager.AddToQ ( stQInfo, stAddCustomerRpy.nQNo );

	stMsgHeader.nCmdID = CMD_SHOP_ADD_CUSTOMER_RPY;
	stMsgHeader.nStatus = nStatus;
	stMsgHeader.nIsLastMsg = TRUE;

	char sendBuffer[BUFFER_SIZE];
	memcpy ( sendBuffer, &stMsgHeader, MSG_HEADER_SIZE );
	memcpy ( sendBuffer + MSG_HEADER_SIZE, &stAddCustomerRpy, sizeof ( stAddCustomerRpy ) );
	send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stAddCustomerRpy ), 0 );

	return OK;
}

static short HandleUserCancelQ( int socket, char *buffer, int nSize )
{
	STT_USER_CANCEL_Q_CMD stCancelQCmd;
	int size = recv ( socket, buffer, sizeof ( stCancelQCmd ), 0 );
	if ( sizeof ( stCancelQCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stCancelQCmd ) );
		LogToFile ( szLog );
		return NOK;
	}

	memcpy ( &stCancelQCmd, buffer, sizeof ( stCancelQCmd ) );

	char sendBuffer[BUFFER_SIZE];
	STT_MSG_HEADER stHeader;
	CDBManager dbManager;
	short nStatus = OK;
	nStatus = dbManager.UserCancelQ ( stCancelQCmd );

	if ( OK != nStatus )
		goto EXIT;

	STT_SHOP_CLIENT_SOCKET stShopClient;
	if ( OK == GetSocketByShopID ( stCancelQCmd.nShopID, stShopClient ) )	{
		stHeader.nCmdID = CMD_USER_CANCEL_Q;
		stHeader.nStatus = 0;
		stHeader.nIsLastMsg = TRUE;
		memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
		memcpy ( sendBuffer + MSG_HEADER_SIZE, &stCancelQCmd, sizeof ( struct STT_USER_CANCEL_Q_CMD ) );
		send ( stShopClient.socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( struct STT_USER_CANCEL_Q_CMD ), 0 );
	}

EXIT:
	stHeader.nCmdID = CMD_USER_CANCEL_Q_RPY;
	stHeader.nStatus = nStatus;
	stHeader.nIsLastMsg = TRUE;
	send ( socket, (const char*)&stHeader, MSG_HEADER_SIZE, 0 );

	//Because send push notification need longer time, so put it at the end
	if ( OK == nStatus )	{
		STT_DEVICE_TOKEN stDeviceToken;
		nStatus = dbManager.GetShopDeviceToken ( stCancelQCmd.nShopID, stDeviceToken );
		if ( OK == nStatus && stDeviceToken.bDeviceTokenValid )	{
			char szPushMsg[200];
			_snprintf ( szPushMsg, 200, "Customer %s has canceled Q No A%04d", stCancelQCmd.szUserName, stCancelQCmd.nQNo );
			SendPushNotificationToShop ( stDeviceToken.szDeviceToken, szPushMsg, 1 );
			char szLog[300];
			_snprintf ( szLog, 300, "Send notification to shop, device token %s", stDeviceToken.szDeviceToken );
			LogToFile ( szLog );
		}
	}
	return OK;
}

static short HandleQReplyFromShop ( int socket, char *buffer, int nSize )
{
	//short nStatus = OK;
	//STT_MSG_HEADER stHeader;
	//memcpy ( &stHeader, buffer, MSG_HEADER_SIZE );

	//STT_Q_SHOP_TO_SERVER_RPY stRpy;
	//memcpy ( &stRpy, buffer + MSG_HEADER_SIZE, sizeof ( struct STT_Q_SHOP_TO_SERVER_RPY ) );
	//int nQNo = 0;
	////If Q success, need to put the Q information into data base and inquire the Q No
	//if ( stRpy.bAccept )	{
	//	CDBManager dbManager;
	//	nStatus = dbManager.AddQInfo ( stRpy );
	//	if ( nStatus != OK )
	//		return nStatus;
	//	nStatus = dbManager.InqueryQNo ( stRpy.nUserID, stRpy.nShopID, nQNo );
	//	if ( nStatus != OK )
	//		return nStatus;
	//}

	//STT_Q_SERVER_TO_USER_RPY stQServerToUserInfo;
	//stQServerToUserInfo.nUserID = stRpy.nUserID;
	//stQServerToUserInfo.nShopID = stRpy.nShopID;
	//stQServerToUserInfo.bAccept = stRpy.bAccept;
	//stQServerToUserInfo.nQNo = nQNo;

	//char sendBuffer[BUFFER_SIZE];
	//stHeader.nCmdID = CMD_Q_RPY;
	//stHeader.nStatus = 0;
	//stHeader.nIsLastMsg = TRUE;

	//SOCKET socketUser;
	//GetSocketByUserID ( stRpy.nUserID, socketUser );
	//if ( nStatus != OK )
	//	return nStatus;

	//memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
	//memcpy ( sendBuffer + MSG_HEADER_SIZE, &stQServerToUserInfo, sizeof ( struct STT_Q_SERVER_TO_USER_RPY ) );
	//send ( socketUser, sendBuffer, BUFFER_SIZE, 0 );

	return OK;
}

static short HandleShopProcessQ ( int socket, char *buffer, int nSize )
{
	STT_SHOP_PROCESS_Q_CMD stProcessQCmd;
	int size = recv ( socket, buffer, sizeof ( stProcessQCmd ), 0 );
	if ( sizeof ( stProcessQCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stProcessQCmd ) );
		LogToFile ( szLog );
		return NOK;
	}

	memcpy ( &stProcessQCmd, buffer, sizeof ( stProcessQCmd ) );
	
	short nStatus = OK;
	STT_MSG_HEADER stHeader;
	CDBManager dbManager;
	LIST_Q_USER_INFO lstUserInfo;
	nStatus = dbManager.ShopProcessQ ( stProcessQCmd );
	if ( OK != nStatus )
		goto EXIT;
	
	nStatus = dbManager.GetShopQUserInfoList ( stProcessQCmd.nShopID, lstUserInfo );
	if ( OK != nStatus )
		goto EXIT;

	time_t tCurrentTime = time ( NULL );
	dbManager.UpdateShopActiveTime ( stProcessQCmd.nShopID, tCurrentTime );

	STT_DEVICE_TOKEN stDeviceToken;

	short nSequenceNo = 0;
	char sendBuffer[BUFFER_SIZE];
	for ( LIST_Q_USER_INFO::iterator it = lstUserInfo.begin(); it != lstUserInfo.end(); ++ it )
	{
		++ nSequenceNo;
		if ( QSG_REMIND_USER_Q_NO == nSequenceNo - 1 )	{
			nStatus = dbManager.GetUserDeviceToken (  it->nID, stDeviceToken );
			if ( OK == nStatus && stDeviceToken.bDeviceTokenValid )	{
				char szMsg[200];
				_snprintf ( szMsg, sizeof ( szMsg), "Your Q No A%04d at %s only has 3 people in front of you , please proceed to the merchant",
					it->nQNo, stProcessQCmd.szShopName );
				SendPushNotificationToUser ( stDeviceToken.szDeviceToken, szMsg, 1 );
				char szLog[300];
				_snprintf ( szLog, 300, "Send notification to user, device token %s", stDeviceToken.szDeviceToken );
				LogToFile ( szLog );
			}
		}
		STT_USER_CLIENT_SOCKET stUserClient;
		nStatus = GetSocketByUserID ( it->nID, stUserClient );
		if ( OK != nStatus )
			continue;

		stHeader.nCmdID = CMD_INQUERY_Q_RPY;
		stHeader.nStatus = 0;
		stHeader.nIsLastMsg = TRUE;

		STT_USER_INQUERY_Q_RPY stInqueryQRpy;
		stInqueryQRpy.nQNo = it->nQNo;
		stInqueryQRpy.nShopID = stProcessQCmd.nShopID;
		stInqueryQRpy.nUserID = it->nID;
		stInqueryQRpy.nQInfront = nSequenceNo - 1;
		_snprintf ( stInqueryQRpy.szShopName, sizeof ( stInqueryQRpy.szShopName ), stProcessQCmd.szShopName );
		memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
		memcpy ( sendBuffer + MSG_HEADER_SIZE, &stInqueryQRpy, sizeof ( stInqueryQRpy ) );
		send ( stUserClient.socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stInqueryQRpy ), 0 );
	}

EXIT:
	nStatus = dbManager.GetUserDeviceToken (  stProcessQCmd.nUserID, stDeviceToken );
	if ( OK == nStatus && stDeviceToken.bDeviceTokenValid )	{
		char szMsg[200];
		_snprintf ( szMsg, sizeof ( szMsg ), "%s: You Q A%04d has been processed", stProcessQCmd.szShopName, stProcessQCmd.nQNo );
		SendPushNotificationToUser ( stDeviceToken.szDeviceToken, szMsg, 1 );
		char szLog[300];
		_snprintf ( szLog, 300, "Send Q processed notification to user, device token %s", stDeviceToken.szDeviceToken );
		LogToFile ( szLog );
	}

	stHeader.nCmdID = CMD_SHOP_PROCESS_Q_RPY;
	stHeader.nStatus = nStatus;
	stHeader.nIsLastMsg = TRUE;
	memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
	send ( socket, sendBuffer, MSG_HEADER_SIZE, 0 );

	return OK;
}

static short HandleInqueryQ ( int socket, char *buffer, int nSize )
{
	short nStatus = OK;

	STT_USER_INQUERY_Q_CMD stInqueryQInfo;
	int size = recv ( socket, buffer, sizeof ( stInqueryQInfo ), 0 );
	if ( sizeof ( stInqueryQInfo ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stInqueryQInfo ) );
		LogToFile ( szLog );
		return NOK;
	}

	memcpy ( &stInqueryQInfo, buffer, sizeof ( stInqueryQInfo ) );

	STT_USER_INQUERY_Q_RPY stInqueryQRpy;
	CDBManager dbManager;
	nStatus = dbManager.InqueryQNo ( stInqueryQInfo.nUserID, stInqueryQRpy );

	STT_MSG_HEADER stHeader;
	stHeader.nStatus = nStatus;
	stHeader.nCmdID = CMD_INQUERY_Q_RPY;	
	stInqueryQRpy.nUserID = stInqueryQInfo.nUserID;

	char sendBuffer[BUFFER_SIZE];
	memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
	memcpy ( sendBuffer + MSG_HEADER_SIZE, &stInqueryQRpy, sizeof ( stInqueryQRpy ) );
	send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stInqueryQRpy ), 0 );

	return OK;
}

static short HandleGetShopQUser ( int socket, char *buffer, int nSize )
{
	STT_GET_SHOP_INFO_CMD stGetShopQUserInfo;
	int size = recv ( socket, buffer, sizeof ( stGetShopQUserInfo ), 0 );
	if ( sizeof ( stGetShopQUserInfo ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stGetShopQUserInfo ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stGetShopQUserInfo, buffer, sizeof ( stGetShopQUserInfo ) );

	short nStatus = OK;
	STT_MSG_HEADER stHeader;
	LIST_Q_USER_INFO lstQUserInfo;
	CDBManager dbManager;
	nStatus = dbManager.GetShopQUserInfoList ( stGetShopQUserInfo.nShopID, lstQUserInfo );	
	
	if ( lstQUserInfo.size() > 0 )	{
		char sendBuffer[BUFFER_SIZE];
		for ( LIST_Q_USER_INFO::iterator it = lstQUserInfo.begin(); it != lstQUserInfo.end(); ++ it )
		{
			stHeader.nCmdID = CMD_GET_SHOP_Q_USER_RPY;
			stHeader.nStatus = 0;
			stHeader.nIsLastMsg = FALSE;

			if ( it == std::prev ( lstQUserInfo.end() ) )
				stHeader.nIsLastMsg = TRUE;

			STT_Q_USER_INFO stQUserInfo = *it;
			memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
			memcpy ( sendBuffer + MSG_HEADER_SIZE, &stQUserInfo, sizeof ( stQUserInfo ) );
			send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stQUserInfo ), 0 );
		}
	}else	{
		stHeader.nCmdID = CMD_GET_SHOP_Q_USER_RPY;
		stHeader.nStatus = QSG_STATUS_NO_USER_IS_QING;
		stHeader.nIsLastMsg = TRUE;
		send ( socket, (char *)&stHeader, MSG_HEADER_SIZE, 0 );
	}
	return nStatus;
}

static short SendShopImage ( int socket, char *szImagePath, const STT_REQUEST_IMAGE_CMD &stRequestImgCmd )
{
	char szLog[300];

	ifstream is ( szImagePath, ifstream::binary );

	if ( ! is.is_open() )	{
		_snprintf ( szLog, 300, "Failed to open file %s", szImagePath );
		LogToFile ( szLog );
		return NOK;
	}

	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_REQUEST_IMAGE_RPY;
	stHeader.nStatus = 0;
	stHeader.nIsLastMsg = FALSE;

	STT_IMAGE_HEADER stImageHeader;
	stImageHeader.nShopID = stRequestImgCmd.nShopID;
	stImageHeader.nImageID = stRequestImgCmd.nImageID;
	stImageHeader.nImageType = stRequestImgCmd.nImageType;

	//Seek to end to get file size
	is.seekg (0, is.end);
    int length = (int)is.tellg();
	stImageHeader.nImageSize = length;

	is.seekg (0, is.beg);

	short nMaxFrameDataSize = BUFFER_SIZE - MSG_HEADER_SIZE - sizeof ( struct STT_IMAGE_HEADER );

	int nFrameNo = 0;
	while ( is.tellg() < length )	{
		int nLeftOverFileSize = length - (int)is.tellg();
		short nReadDataSize = nMaxFrameDataSize;
		if ( nLeftOverFileSize < nMaxFrameDataSize )	{
			nReadDataSize = nLeftOverFileSize;
			stHeader.nIsLastMsg = TRUE;
		}
		stImageHeader.nFrameNo = nFrameNo;
		stImageHeader.nDataSize = nReadDataSize;

		char readBuffer[ BUFFER_SIZE ];
		is.read ( readBuffer, nReadDataSize );

		stHeader.nDataSize = IMAGE_HEADER_SIZE + nReadDataSize;

		char sendBuffer [ BUFFER_SIZE ];
		memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );
		memcpy ( sendBuffer + MSG_HEADER_SIZE, &stImageHeader, IMAGE_HEADER_SIZE );
		memcpy ( sendBuffer + MSG_HEADER_SIZE + IMAGE_HEADER_SIZE, readBuffer, nReadDataSize );

		int nSendedData = send ( socket, sendBuffer, MSG_HEADER_SIZE + IMAGE_HEADER_SIZE + nReadDataSize, 0 );

		_snprintf ( szLog, 300, "Send frame %d frame size %d, send data size %d", nFrameNo, nReadDataSize, nSendedData );
		LogToFile ( szLog );
		++ nFrameNo;
		Sleep ( 20 );
	}

	is.close();
	return OK;
}

static short HandleRequestShopImage ( int socket, char *buffer, int nSize )
{
	STT_REQUEST_IMAGE_CMD stRequestImgCmd;
	int size = recv ( socket, buffer, sizeof ( stRequestImgCmd ), 0 );
	if ( sizeof ( stRequestImgCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stRequestImgCmd ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stRequestImgCmd, buffer, sizeof ( stRequestImgCmd ) );

	char szImagePath[255], szLog[300];
	_snprintf ( szImagePath, sizeof ( szImagePath ), "./Image/Shop_%d/Image_%d.%s", stRequestImgCmd.nShopID, stRequestImgCmd.nImageID, gszImageType[stRequestImgCmd.nImageType] );
	_snprintf ( szLog, sizeof ( szLog ),"Request Image: %s", szImagePath );
	LogToFile ( szLog );

	short nStatus;
	struct stat statbuf;
    if ( stat( szImagePath, &statbuf ) == -1) {
		nStatus = QSG_STATUS_IMAGE_NOT_EXIST;
		goto EXIT;
    }
	
	if ( ! stRequestImgCmd.bHasImage || statbuf.st_mtime > stRequestImgCmd.dImageModifyDate )	{
		nStatus = SendShopImage ( socket, szImagePath, stRequestImgCmd );
	}

EXIT:
	return nStatus;
}

static short RecvShopImage ( int socket, char *buffer, int nSize )
{
	STT_MSG_HEADER stHeader;
	memcpy ( &stHeader, buffer, MSG_HEADER_SIZE );

	Sleep(200);
	int size = recv ( socket, buffer, stHeader.nDataSize, 0 );
	if ( stHeader.nDataSize != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, stHeader.nDataSize );
		LogToFile ( szLog );
		return NOK;
	}

	STT_IMAGE_HEADER stImageHeader;
	memcpy ( &stImageHeader, buffer, sizeof ( struct STT_IMAGE_HEADER ) );

	string szFilePath = "./Image";
	_mkdir ( szFilePath.data() );

	char szPath[255], szLog[300];
	_snprintf ( szPath, 255, "/Shop_%d", stImageHeader.nShopID );
	szFilePath += szPath;
	_mkdir ( szFilePath.data() );

	_snprintf ( szPath, 255, "/Image_%d.%s", stImageHeader.nImageID, gszImageType[stImageHeader.nImageType] );
	szFilePath += szPath;

	if ( 0 == stImageHeader.nFrameNo )	{
		remove ( szFilePath.data() );
		//_snprintf ( szLog, 300, "Receive file %s, total size %d", szFilePath.data(), stImageHeader.nImageSize );
		//LogToFile ( szLog );
	}
	//_snprintf ( szLog, 300, "Receive file %s, frame %d, data size %d", szFilePath.data(), stImageHeader.nFrameNo, stImageHeader.nDataSize );
	//LogToFile ( szLog );

	fstream file ( szFilePath, fstream::binary | fstream::out | fstream::ate | fstream::app );

	if ( ! file.is_open() )	{
		_snprintf ( szLog, 300, "Failed to open file %s", szPath );
		LogToFile ( szLog );
		return NOK;
	}
	
	char dataBuffer [ BUFFER_SIZE ];
	memcpy ( dataBuffer, buffer + IMAGE_HEADER_SIZE, stImageHeader.nDataSize );
	file.write ( dataBuffer, stImageHeader.nDataSize );

	file.close();

	return OK;
}

static short HandleCheckOnline ( int socket, char *buffer, int nSize )
{
	STT_GET_SHOP_INFO_CMD stGetShopInfoCmd;
	int size = recv ( socket, buffer, sizeof ( stGetShopInfoCmd ), 0 );
	if ( sizeof ( stGetShopInfoCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stGetShopInfoCmd ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stGetShopInfoCmd, buffer, sizeof ( stGetShopInfoCmd ) );

	time_t tCurrent = time ( NULL );

	CDBManager dbManager;
	short nStatus = dbManager.UpdateShopActiveTime ( stGetShopInfoCmd.nShopID, tCurrent );

	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_CHECK_ONLINE_RPY;
	stHeader.nIsLastMsg = TRUE;
	char sendBuffer [ BUFFER_SIZE ];
	memcpy ( sendBuffer, &stHeader, MSG_HEADER_SIZE );

	send ( socket, sendBuffer, MSG_HEADER_SIZE, 0 );
	return OK;
}

static short HandleGetUserQHistory ( int socket, char *buffer, int nSize )
{
	STT_GET_USER_Q_HISTORY_CMD stGetQHistoryCmd;
	int size = recv ( socket, buffer, sizeof ( stGetQHistoryCmd ), 0 );
	if ( sizeof ( stGetQHistoryCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stGetQHistoryCmd ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stGetQHistoryCmd, buffer, sizeof ( stGetQHistoryCmd ) );

	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_GET_USER_Q_HISTORY_RPY;
	stHeader.nIsLastMsg = FALSE;
	char sendBuffer [ BUFFER_SIZE ];

	CDBManager dbManager;
	LIST_USER_Q_HISTORY lstUserQHistory;
	short nStatus = dbManager.GetUserQHistory ( stGetQHistoryCmd, lstUserQHistory );
	if ( OK != nStatus )	{
		stHeader.nStatus = nStatus;
		send ( socket, (const char *)&stHeader, MSG_HEADER_SIZE, 0 );
		return nStatus;
	}
	
	stHeader.nStatus = OK;
	for ( LIST_USER_Q_HISTORY::iterator it = lstUserQHistory.begin(); it != lstUserQHistory.end(); ++ it )	{
		STT_GET_USER_Q_HISTORY_RPY stQHistoryRpy = *it;

		if ( it == std::prev ( lstUserQHistory.end() ) )
			stHeader.nIsLastMsg = TRUE;
		memcpy ( sendBuffer, &stHeader, sizeof ( STT_MSG_HEADER ) );
		memcpy ( sendBuffer + MSG_HEADER_SIZE, &stQHistoryRpy, sizeof ( stQHistoryRpy ) );
		send ( socket, sendBuffer, MSG_HEADER_SIZE + sizeof ( stQHistoryRpy ), 0 );
		Sleep ( 1 );
	}
	return OK;
}

static short HandleAddToFavorite ( int socket, char *buffer, int nSize )
{
	STT_ADD_TO_FAVORITE_CMD stAddFavoriteCmd;
	int size = recv ( socket, buffer, sizeof ( stAddFavoriteCmd ), 0 );
	if ( sizeof ( stAddFavoriteCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stAddFavoriteCmd ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stAddFavoriteCmd, buffer, sizeof ( stAddFavoriteCmd ) );

	CDBManager dbManager;
	LIST_USER_Q_HISTORY lstUserQHistory;
	short nStatus = dbManager.AddShopToFavorite ( stAddFavoriteCmd );

	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_ADD_SHOP_TO_FAVORITE_RPY;
	stHeader.nIsLastMsg = TRUE;
	stHeader.nStatus = nStatus;
	send ( socket, (const char *)&stHeader, MSG_HEADER_SIZE, 0 );
	return nStatus;
}

static short HandleSendFeedBack ( int socket, char *buffer, int nSize )
{
	STT_SEND_FEED_BACK_CMD stSendFeedBackCmd;
	Sleep(10);	//Wait all the message send done.
	int size = recv ( socket, buffer, sizeof ( stSendFeedBackCmd ), 0 );
	if ( sizeof ( stSendFeedBackCmd ) != size )	{
		char szLog[200];
		_snprintf ( szLog, sizeof ( szLog ), "Read data size %d not equal to data struct size %d", size, sizeof ( stSendFeedBackCmd ) );
		LogToFile ( szLog );
		return NOK;
	}
	memcpy ( &stSendFeedBackCmd, buffer, sizeof ( stSendFeedBackCmd ) );

	CDBManager dbManager;
	short nStatus = dbManager.AddFeedBack ( stSendFeedBackCmd );

	STT_MSG_HEADER stHeader;
	stHeader.nCmdID = CMD_SEND_FEED_BACK_RPY;
	stHeader.nIsLastMsg = TRUE;
	stHeader.nStatus = nStatus;
	send ( socket, (const char *)&stHeader, MSG_HEADER_SIZE, 0 );
	return nStatus;
}

short HandleData ( SOCKET socket, char *buffer, int nSize )
{
	if ( NULL == buffer || nSize <= 0 )
		return NOK;

	short nStatus = OK;
	STT_MSG_HEADER stHeader;
	memcpy ( &stHeader, buffer, MSG_HEADER_SIZE );
	printf ("Data received, size : %d, nCmdID %d\n", nSize, stHeader.nCmdID );
	switch ( stHeader.nCmdID )
	{
	case CMD_USER_LOGIN:
		nStatus = HandleUserLogInData ( socket, buffer, nSize );
		break;

	case CMD_USER_LOG_OUT:
		RemoveUserInfoFromList ( socket );
		break;

	case CMD_SHOP_LOGIN:
		nStatus = HandleShopLogInData ( socket, buffer, nSize );
		break;	

	case CMD_SHOP_LOG_OUT:
		HandleShopLogOut ( socket, buffer, nSize );
		break;

	case CMD_REGISTER_USER:
		nStatus = HandleRegisterUser ( socket, buffer, nSize );
		break;

	case CMD_REGISTER_SHOP:
		nStatus = HandleRegisterShop ( socket, buffer, nSize );
		break;

	case CMD_UPDATE_SHOP_PROFILE:
		nStatus = HandleUpdateShopProfile ( socket, buffer, nSize );
		break;

	case CMD_GET_ALL_SHOP:
		nStatus = HandleGetAllShop ( socket, buffer, nSize );
		break;

	case CMD_GET_RECOMMENDED_SHOP:
		nStatus = HandleGetRecommendedShop ( socket, buffer, nSize );
		break;

	case CMD_Q:
		nStatus = HandleQ ( socket, buffer, nSize );
		break;

	case CMD_Q_TO_SHOP_RPY:
		nStatus = HandleQReplyFromShop ( socket, buffer, nSize );
		break;

	case CMD_SHOP_PROCESS_Q:
		nStatus = HandleShopProcessQ ( socket, buffer, nSize );
		break;

	case CMD_INQUERY_Q:
		nStatus = HandleInqueryQ ( socket, buffer, nSize );
		break;

	case CMD_GET_SHOP_Q_USER:
		nStatus = HandleGetShopQUser ( socket, buffer, nSize );
		break;

	case CMD_GET_SHOP_BY_ID:
		nStatus = HandleGetShop ( socket, buffer, nSize );
		break;

	case CMD_SRCH_SHOP:
		nStatus = HandleSrchShop ( socket, buffer, nSize );
		break;

	case CMD_GET_NEAR_BY_SHOP:
		nStatus = HandleGetNearByShop ( socket, buffer, nSize );
		break;

	case CMD_GET_POPULAR_SHOP:
		HandleGetPopularShop  ( socket, buffer, nSize );
		break;

	case CMD_SHOP_SEND_IMAGE:
		nStatus = RecvShopImage ( socket, buffer, nSize );
		break;

	case CMD_REQUEST_IMAGE:
		nStatus = HandleRequestShopImage ( socket, buffer, nSize );
		break;

	case CMD_CHECK_ONLINE:
		nStatus = HandleCheckOnline( socket, buffer, nSize );
		break;

	case CMD_USER_CANCEL_Q:
		nStatus = HandleUserCancelQ ( socket, buffer, nSize );
		break;

	case CMD_GET_USER_Q_HISTORY:
		nStatus = HandleGetUserQHistory ( socket, buffer, nSize );
		break;

	case CMD_ADD_SHOP_TO_FAVORITE:
		nStatus = HandleAddToFavorite ( socket, buffer, nSize );
		break;

	case CMD_GET_FAVORITE_SHOP:
		nStatus = HandleGetFavoriteShop ( socket, buffer, nSize );
		break;

	case CMD_SHOP_ADD_CUSTOMER:
		nStatus = HandleShopAddCustomer ( socket, buffer, nSize );
		break;

	case CMD_SEND_FEED_BACK:
		nStatus = HandleSendFeedBack ( socket, buffer, nSize );
		break;

	case CMD_GET_SHOP_BY_EMAIL:
		nStatus = HandleGetShopByEmail ( socket, buffer, nSize );
		break;

	default:
		break;
	}
	return nStatus;
}

void ThreadReadMsg(void *pParam)
{
	SOCKET socket = *(SOCKET *)pParam;
	char szMsg[100];
	_snprintf ( szMsg, 100, "Start read thread for socket %d", socket );
	LogToFile ( szMsg );
	while(1)
	{
		char szDataBuffer[BUFFER_SIZE];
		int size = 0;

		//fd_set Sockets;
		//FD_ZERO(&Sockets);
		//FD_SET(stClient.socket, &Sockets);

		//struct timeval tv;
		//tv.tv_sec = 1;
		//tv.tv_usec = 0;

		//size = select ( stClient.socket + 1, &Sockets, NULL, NULL, &tv );
		//if ( size <= 0 )
		//	continue;
		size = recv ( socket, szDataBuffer, MSG_HEADER_SIZE, 0 );
		if ( size > 0 )
		{
			HandleData ( socket, szDataBuffer, size );
		}else	{
			_snprintf ( szMsg, 100, "Socket %d closed, end thread of it", socket );
			RemoveShopInfoFromList ( socket );
			RemoveUserInfoFromList ( socket );
			LogToFile ( szMsg );
			break;
		}

		Sleep(10);
	}
}

short StartSocket()
{
	int listenfd = 0, connfd = 0;
    struct sockaddr_in serv_addr;
	short nStatus = OK;

	char sendBuff[2015];
	//time_t ticks;

	// Initialize Winsock
	WSADATA wsaData;
    nStatus = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (nStatus != NO_ERROR) {
        wprintf(L"Error at WSAStartup()\n");
        return 1;
    }

	InitializeCriticalSection ( &gcsUserClient );
	InitializeCriticalSection ( &gcsShopClient );

	listenfd = socket ( AF_INET, SOCK_STREAM, 0 );
	if ( listenfd < 0 )
	{
		return NOK;
	}
	LogToFile ( "Success to open socket" );
	memset (&serv_addr, 0, sizeof ( sockaddr_in ) );
	memset ( sendBuff, '0', sizeof (sendBuff) );

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	//serv_addr.sin_addr.S_un.S_addr = inet_addr("222.165.95.78");

	serv_addr.sin_port = htons ( QSG_PORT );

	nStatus = ::bind ( listenfd, (struct sockaddr *)&serv_addr, sizeof ( serv_addr ) );

	if ( nStatus < 0 )
	{
		return NOK;
	}

	listen ( listenfd, 5);

	printf("Start to listen for connection\n");

	while(1)
	{
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
		printf ( "After accept connection connfd %d\n", connfd );

		//STT_CLIENT_SOCKET stClient;
		//stClient.socket = connfd;
		//stClient.nUserID = -1;
		//stClient.nShopID = -1;

		if ( connfd > 0 )
			_beginthread ( ThreadReadMsg, 0, &connfd );

		//EnterCriticalSection ( &gcsUserClient );
		//glistClient.push_back ( stClient );
		//LeaveCriticalSection ( &gcsClient );

		//time(&ticks);
		//_snprintf_s(sendBuff, sizeof(sendBuff), sizeof(sendBuff), "%d\r\n", ticks );
		//send ( connfd, sendBuff, strlen(sendBuff), 0 );

		//shutdown ( connfd, 0 );
		Sleep(10);
	}

	WSACleanup();

	return OK;
}
