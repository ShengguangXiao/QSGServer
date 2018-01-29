#ifndef _CONSTANTS_H_
#define _CONSTANTS_H_

#define OK								(0)
#define NOK								(-1)
#define QSG_PORT						(6666)
#define BUFFER_SIZE						8192

#define QSG_SERVER                      (@"222.165.115.192")
#define QSG_SERVER_NAME					("www.queuesg.com")
#define QSG_DB_SERVER					("tcp://127.0.0.1:3306")
#define QSG_DB_NAME						("qsg_utf8")
#define QSG_DB_USER						("xsgluozi")
#define QSG_DB_PASSWORD					("xsgluozi123")
#define QSG_TABLE_USER					("qsg_user")
#define QSG_TABLE_SHOP					("qsg_shop")
#define QSG_TABLE_Q						("qsg_q_table")
#define QSG_TABLE_FAVORITE				("qsg_favorite")
#define QSG_TABLE_FEED_BACK				("QSG_FEED_BACK")
#define QSG_SHOP_EMAIL                  (@"QSG_SHOP_EMAIL")
#define QSG_SHOP_PASSWORD               (@"QSG_SHOP_PASSWORD")
#define QSG_USER_EMAIL                  (@"QSG_USER_EMAIL")
#define QSG_USER_PASSWORD               (@"QSG_USER_PASSWORD")
#define QSG_MAX_NEAR_BY_SHOP_NUM		(10)
#define QSG_MAX_POPULAR_SHOP_NUM		(10)
#define QSG_MAX_RECOMMENDED_SHOP_NUM	(10)
#define	QSG_UI_BACKGROUND_COLOR_RGB		(220.f)
#define QSG_USER_NOTIFY_CERT			("MommyBonusQSGCert.pem")
#define QSG_USER_NOTIFY_KEY				("MommyBonusQSGKey.pem")
#define QSG_SHOP_NOTIFY_CERT			("MommyBonusQSGShopCert.pem")
#define QSG_SHOP_NOTIFY_KEY				("MommyBonusQSGShopKey.pem")
#define QSG_NOTIFY_PASSPHRASE			("xsgluozi123")
#define QSG_DEVICE_TOKEN_LEN			(100)
#define QSG_SHOP_OFFLINE_TIME_THRES		( 60 * 60 )		//60 minutes not active, then consider shop is offline
#define AT								__FILE__, __FUNCTION__, __LINE__
#define QSG_REMIND_USER_Q_NO			(3)	//When Q No advance to this number, send notification to user
#define QSG_Q_START_TIME				("06:00:00")
#define QSG_TIME_START_YEAR				(1900)
#define QSG_CANCEL_Q_LIMIT				(3)
#define QSG_WALKIN_CUSTOMER_ID			(1)

enum IMAGE_TYPE	{
	IMAGE_TYPE_PNG,
	IMAGE_TYPE_JPG,
	IMAGE_TYPE_BMP,
	IMAGE_TYPE_END,
};

const static char *gszImageType[] =
{
	"png",
	"jpg",
	"bmp",
};

enum Q_STATUS	{
	Q_SUBMITTED,
	Q_PROCESSED,
	Q_CANCELED,
	Q_DISCARDED,
};

enum CLIENT_TYPE	{
	CLIENT_USER,
	CLIENT_SHOP,
};

#endif