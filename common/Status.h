#ifndef _STATUS_H_
#define _STATUS_H_

struct QSG_STATUS
{
	short	nStatus;
	char	szDescription[100];
};

enum
{
	QSG_STATUS_OK,
	QSG_STATUS_SOCKET_ERROR,
	QSG_STATUS_INVALID_PARAM,
	QSG_STATUS_USER_PSW_WRONG,
	QSG_STATUS_WRONG_USER_ID_FOR_Q,
	QSG_STATUS_ALREADY_HAVE_Q,
	QSG_STATUS_SHOP_OFFLINE,
	QSG_STATUS_NO_MATCH_RESULT,
	QSG_STATUS_USER_NOT_IN_Q,
	QSG_STATUS_NO_USER_IS_QING,
	QSG_STATUS_EMAIL_ALREADY_REGISTERED,
	QSG_STATUS_CANCEL_Q_EXCEED_LMT,
	QSG_STATUS_IMAGE_NOT_EXIST,
	QSG_STATUS_NO_Q_HISTORY,
	QSG_STATUS_ALREADY_ADDED,
};

//const QSG_STATUS gaQsgStatus[] = 
//{
//	//{0,		"OK"}
//	//{1,		"Invalid Parameter"}
//	//{2,		"User name or password is wrong"}
//};
#endif