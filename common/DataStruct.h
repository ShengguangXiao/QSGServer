#ifndef _DATA_STRUCT_H_
#define _DATA_STRUCT_H_

#include "Constants.h"
#include <list>
using namespace std;

enum CMD_TYPE
{
	CMD_BEGIN,
	CMD_USER_LOGIN,
	CMD_USER_LOGIN_RPY,
	CMD_USER_LOG_OUT,
	CMD_USER_LOG_OUT_RPY,
	CMD_SHOP_LOGIN,
	CMD_SHOP_LOGIN_RPY,
	CMD_SHOP_LOG_OUT,
	CMD_SHOP_LOG_OUT_RPY,
	CMD_REGISTER_USER,
	CMD_REGISTER_USER_RPY,
	CMD_UPDATE_USER_PROFILE,
	CMD_UPDATE_USER_PROFILE_RPY,
	CMD_REGISTER_SHOP,
	CMD_REGISTER_SHOP_RPY,
	CMD_UPDATE_SHOP_PROFILE,
	CMD_UPDATE_SHOP_PROFILE_RPY,
	CMD_GET_ALL_SHOP,
	CMD_GET_ALL_SHOP_RPY,
	CMD_GET_RECOMMENDED_SHOP,
	CMD_GET_RECOMMENDED_SHOP_RPY,
	CMD_Q,
	CMD_Q_RPY,
	CMD_Q_TO_SHOP,
	CMD_Q_TO_SHOP_RPY,
	CMD_INQUERY_Q,
	CMD_INQUERY_Q_RPY,
	CMD_SHOP_PROCESS_Q,
	CMD_SHOP_PROCESS_Q_RPY,
	CMD_GET_SHOP_Q_USER,
	CMD_GET_SHOP_Q_USER_RPY,
	CMD_GET_SHOP_BY_ID,
	CMD_GET_SHOP_BY_ID_RPY,
	CMD_SRCH_SHOP,
	CMD_SRCH_SHOP_RPY,
	CMD_GET_NEAR_BY_SHOP,
	CMD_GET_NEAR_BY_SHOP_RPY,
	CMD_GET_POPULAR_SHOP,
	CMD_GET_POPULAR_SHOP_RPY,
	CMD_SHOP_SEND_IMAGE,
	CMD_SHOP_SEND_IMAGE_RPY,
	CMD_REQUEST_IMAGE,
	CMD_REQUEST_IMAGE_RPY,
	CMD_CHECK_ONLINE,
	CMD_CHECK_ONLINE_RPY,
	CMD_USER_CANCEL_Q,
	CMD_USER_CANCEL_Q_RPY,
	CMD_GET_USER_Q_HISTORY,
	CMD_GET_USER_Q_HISTORY_RPY,
	CMD_ADD_SHOP_TO_FAVORITE,
	CMD_ADD_SHOP_TO_FAVORITE_RPY,
	CMD_GET_FAVORITE_SHOP,
	CMD_GET_FAVORITE_SHOP_RPY,
	CMD_SHOP_ADD_CUSTOMER,
	CMD_SHOP_ADD_CUSTOMER_RPY,
	CMD_SEND_FEED_BACK,
	CMD_SEND_FEED_BACK_RPY,
	CMD_GET_SHOP_BY_EMAIL,
	CMD_GET_SHOP_BY_EMAIL_RPY,
};

#pragma pack(push, 1)

struct STT_USER_CLIENT_SOCKET
{
	int socket;
	int nUserID;
};

typedef list<STT_USER_CLIENT_SOCKET> LIST_USER_CLIENT;

struct STT_SHOP_CLIENT_SOCKET
{
	int socket;
	int nShopID;
};

typedef list<STT_SHOP_CLIENT_SOCKET> LIST_SHOP_CLIENT;

struct STT_MSG_HEADER
{
	short		nCmdID;
	short		nStatus;
	short		nDataSize;
	short		nIsLastMsg;
	short		nReserve[4];
};

#define MSG_HEADER_SIZE			(sizeof ( struct STT_MSG_HEADER ))

struct STT_COMMON_RPY	{
	short		nStatus;
	short		nDataSize;
	short		nIsLastReply;
	short		nReserve[4];
};

struct STT_LOGIN_CMD	{
	char	szEmail[50];
	char	szPassWord[50];
	bool	bDeviceTokenValid;
	char	szDeviceToken[QSG_DEVICE_TOKEN_LEN];
	bool	bNeedReply;
	short	nClientDeviceType;		//Reserved. iOS or Android
	char	szReserve[200];
};

struct STT_USER_LOGIN_RPY	{
	int		nID;
	char	szEmail[50];
	char	szName[50];
	char	szPhone[50];
	char	szAddress[200];
	short	nCreditPoint;
	char	szReserve[100];
};

struct STT_SHOP_LOGIN_RPY	{
	int		nID;
	char	szEmail[50];
	char	szName[100];
	char	szPhone[50];
	char	szAddress[100];
	char	szPostcode[20];
	char	szIntroduction[300];
	char	szOpenHours[200];
	char	szReserve[100];
};

struct STT_REGISTER_USER_INFO	{
	char	szEmail[50];
	char	szPassWord[50];
	char	szName[50];
	char	szPhone[50];
	char	szAddress[200];
	char	szReserve[1000];
};

struct STT_REGISTER_SHOP_INFO	{
	char	szEmail[50];
	char	szPassWord[50];
	char	szName[100];
	char	szPhone[50];
	char	szAddress[100];
	char	szPostcode[20];
	char	szIntroduction[300];
	short	nCategory;
	double	dLatidute;
	double	dLongitude;
	char	szOpenHours[200];
	char	szReserve[500];
};

struct STT_GET_SHOP_DETAIL_RPY	{
	int		nID;
	char	szEmail[50];
	char	szName[100];
	char	szPhone[50];
	char	szAddress[100];
	char	szPostcode[20];
	char	szIntroduction[300];
	short	nCategory;
	double	dLatidute;
	double	dLongitude;
	bool	bOnline;
	int		nQLen;					//Currently how many people are Queqing
	int		nTotalQNum;
	float	fDistance;
	char	szOpenHours[200];
	char	szReserve[200];
};

struct STT_GET_SHOP_SUMMARY_RPY	{
	int		nID;
	char	szEmail[50];
	char	szName[100];
	char	szPhone[50];
	char	szAddress[100];
	char	szPostcode[20];
	short	nCategory;
	double	dLatidute;
	double	dLongitude;
	bool	bOnline;
	int		nQLen;					//Currently how many people are Queqing
	int		nTotalQNum;
	float	fDistance;
	char	szReserve[100];
};

typedef list<STT_GET_SHOP_SUMMARY_RPY> LIST_GET_SHOP_SUMMARY;

struct STT_Q_USER_INFO	{
	int		nID;
	int		nQNo;
	char	szEmail[50];
	char	szName[50];
	char	szPhone[50];
	char	szQTime[50];
	short	nCustomerNum;
	short	nCreditPoint;
	char	szReserve[1000];
};

typedef list<STT_Q_USER_INFO> LIST_Q_USER_INFO;

struct STT_CATEGORY	{
	int		nID;
	char	szName[50];
	char	szDestription[200];
	char	szReserve[250];
};

struct STT_Q_USER_TO_SERVER_INFO	{
	int		nShopID;
	int		nUserID;
	short	nCustomerNum;
	char	szReserve[500];
};

struct STT_USER_CANCEL_Q_CMD	{
	int		nShopID;
	int		nUserID;
	short	nQNo;
	char	szUserName[50];
	char	szReserve[100];
};

struct STT_Q_SERVER_TO_USER_RPY	{
	int		nShopID;
	int		nUserID;
	bool	bAccept;	//Only bAccept is true, then has the QNo
	short	nQNo;
	short	nQInfront;
	char	szShopName[100];
	char	szReserve[500];
};

struct STT_Q_SERVER_TO_SHOP_INFO	{
	int		nShopID;
	int		nUserID;
	char	szUserName[50];
	char	szPhone[50];
	char	szQTime[50];
	int		nQNo;
	short	nCustomerNum;
	char	szReserve[500];
};

struct STT_Q_SHOP_TO_SERVER_RPY	{
	int		nShopID;
	int		nUserID;
	bool	bAccept;
	char	szReserve[500];
};

struct STT_SHOP_PROCESS_Q_CMD	{
	int		nShopID;
	int		nUserID;
	char	szShopName[100];
	int		nQNo;
	short	nStatus;
	char	szReserve[500];
};

struct STT_USER_INQUERY_Q_CMD	{
	int		nUserID;
	char	szReserve[500];
};

struct STT_USER_INQUERY_Q_RPY	{
	int		nUserID;
	int		nShopID;
	short	nQNo;
	short	nQInfront;
	char	szShopName[100];
	char	szReserve[500];
};

struct STT_GET_SHOP_INFO_CMD	{
	int		nShopID;
	char	szReserve[500];
};

struct STT_SRCH_SHOP_CMD	{
	char	szSrchText[100];
	char	szReserve[500];
};

struct STT_IMAGE_HEADER
{
	int		nShopID;
	short	nImageID;
	short	nImageType;
	int		nImageSize;
	short	nFrameNo;
	short	nDataSize;
	short	nReserve[10];
};

#define IMAGE_HEADER_SIZE	( sizeof ( struct STT_IMAGE_HEADER ) )

struct STT_GET_NEAR_BY_SHOP_CMD
{
	double	dCurrentLatitude;
	double	dCurrentLongitude;
	short	nReserve[100];
};

struct STT_UPDATE_SHOP_PROFILE_CMD
{
	int		nID;
	char	szName[100];
	char	szPhone[50];
	char	szAddress[100];
	char	szPostcode[20];
	double	dLatidute;
	double	dLongitude;
	char	szIntroduction[300];
	char	szOpenHours[200];
	char	szReserve[200];
};

struct _STT_SHOP_DISTANCE	{
	int nShopIndex;
	double dDistance;

	bool operator > ( const _STT_SHOP_DISTANCE &stShopDistance)	{
		return dDistance > stShopDistance.dDistance;
	}

	bool operator < ( const _STT_SHOP_DISTANCE &stShopDistance)	{
		return dDistance < stShopDistance.dDistance;
	}
};

struct STT_DEVICE_TOKEN
{
	bool bDeviceTokenValid;
	char szDeviceToken[QSG_DEVICE_TOKEN_LEN];
};

struct STT_REQUEST_IMAGE_CMD
{
	int		nShopID;
	short	nImageID;
	short	nImageType;
	bool	bHasImage;
	double	dImageModifyDate;		
	short	nReserve[10];
};

struct STT_GET_USER_Q_HISTORY_CMD	{
	int		nUserID;
	char	szReserve[100];
};

struct STT_GET_USER_Q_HISTORY_RPY	{
	int		nShopID;
	char	szShopName[100];
	char	szQTime[50];
	char	szReserve[500];
};

typedef list < STT_GET_USER_Q_HISTORY_RPY > LIST_USER_Q_HISTORY;

struct STT_ADD_TO_FAVORITE_CMD	{
	int		nUserID;
	int		nShopID;
	char	szReserve[100];
};

struct STT_GET_FAVORITE_SHOP_CMD	{
	int		nUserID;
	char	szReserve[100];
};

struct STT_SHOP_ADD_CUSTOMER_RPY	{
	short	nQNo;
	char	szReserve[100];
};

struct STT_SEND_FEED_BACK_CMD	{
	short nClientType;
	char szFeedBack[1000];
};

struct STT_GET_SHOP_BY_EMAIL_CMD	{
	char szEmail[50];
	char szReserve[200];
};

#pragma pack(pop)

#endif