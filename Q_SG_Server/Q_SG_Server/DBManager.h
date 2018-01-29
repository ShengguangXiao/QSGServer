#pragma once

#include "stdafx.h"
#include "Constants.h"
#include "DataStruct.h"
#include <stdlib.h>
#include <iostream>
#include "mysql_connection.h"
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <string>
using namespace std;

class CDBManager
{
public:
	CDBManager(void);
	~CDBManager(void);
	short RegisterUser ( const STT_REGISTER_USER_INFO &stUserInfo );
	short RegisterShop ( const STT_REGISTER_SHOP_INFO &stShopInfo );
	short UpdateShopProfile ( const STT_UPDATE_SHOP_PROFILE_CMD &stShopProfile);
	short GetShops(LIST_GET_SHOP_SUMMARY &lstShop);
	short GetRecommendedShop(LIST_GET_SHOP_SUMMARY &lstShop);
	short SrchShop(const STT_SRCH_SHOP_CMD &stSrchShopCmd, LIST_GET_SHOP_SUMMARY &lstShop);
	short GetFavoriteShop(const STT_GET_FAVORITE_SHOP_CMD &stGetFavoriteCmd, LIST_GET_SHOP_SUMMARY &lstShop);
	short GetPopularShop( LIST_GET_SHOP_SUMMARY &lstShop);
	short GetShopInfoByID(const int nShopID, STT_GET_SHOP_DETAIL_RPY &stShopDetail);
	short GetShopInfoByEmail(const STT_GET_SHOP_BY_EMAIL_CMD &stCmd, STT_GET_SHOP_DETAIL_RPY &stShopDetail);
	//short HandleQ( const STT_Q_USER_TO_SERVER_INFO &stQInfo );
	short HandleUserLogIn ( const STT_LOGIN_CMD &stLoginCmd, STT_USER_LOGIN_RPY  &stLoginRpy );
	short HandleShopLogIn ( const STT_LOGIN_CMD &stLoginCmd, STT_SHOP_LOGIN_RPY  &stLoginRpy );
	short HandleShopLogOut ( const int nShopID );
	short InqueryUserInfo ( short nUserID, STT_Q_SERVER_TO_SHOP_INFO &stInfo);
	short AddQInfo ( const STT_Q_SHOP_TO_SERVER_RPY &stRpy );
	short AddToQ ( const STT_Q_USER_TO_SERVER_INFO stQInfo, short &nQNo );
	short InqueryQNo ( const int nUserID, const int nShopID, short &nQNo, short &nQInfront );
	short InqueryQNo ( const int nUserID, STT_USER_INQUERY_Q_RPY &nInqueryQRpy );
	short InqueryShopQLen ( const int nShopID, int &nQLen );
	short ShopProcessQ ( const STT_SHOP_PROCESS_Q_CMD &stProcessQCmd );
	//short GetShopQUserList ( const int nShopID, LIST_USER_CLIENT &lstUser);
	short GetShopQUserInfoList ( const int nShopID, LIST_Q_USER_INFO &lstUserInfo );
	short UpdateShopActiveTime(int nShopID, time_t time);
	short GetShopActiveTime(int nShopID, bool &bLogin, time_t &time);
	short GetShopDeviceToken ( int nShopID, STT_DEVICE_TOKEN &stDeviceToken);
	short GetUserDeviceToken ( int nShopID, STT_DEVICE_TOKEN &stDeviceToken);
	short UserCancelQ(const STT_USER_CANCEL_Q_CMD &stCancelQInfo);
	short IsUserCancelQExceedLimit ( short nUserID );
	short GetUserQHistory ( const STT_GET_USER_Q_HISTORY_CMD &stGetQHistoryCmd, LIST_USER_Q_HISTORY &lstUserQHistory );
	short AddShopToFavorite ( const STT_ADD_TO_FAVORITE_CMD &stAddFavoriteCmd );
	short AddFeedBack ( const STT_SEND_FEED_BACK_CMD &stSendFeedBackCmd );
protected:
	short ConnectDB();
	short DisconnectDB();
	void HandleException ( sql::SQLException &e, char *szFile, char *szFunction, int nLine );

protected:
	sql::Driver *m_pDriver;
	sql::Connection *m_pConnection;
	sql::Statement *m_pStatement;
	sql::ResultSet *m_pResultSet;
	bool m_bConnected;
};

