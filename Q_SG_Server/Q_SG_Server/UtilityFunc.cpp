#include "UtilityFunc.h"
#include <stdlib.h>
#include <math.h>
#include "apn.h"
#include "version.h"
#include "Constants.h"

#define DEREE_TO_RADIAN(degree)		( (degree) * 3.1415926 / 180.f )
double CalculateDistanceOfLocation(double dCurrentLagitude, double dCurrentLongitude, double dShopLagitude, double dShopLongitude)
{
	double dRadius = 6371000; // metres of earth
	double dCurrentLagitudeRadian = DEREE_TO_RADIAN ( dCurrentLagitude );
	double dShopLagitudeRadian = DEREE_TO_RADIAN ( dShopLagitude );
	double dLatDiffRadian =  DEREE_TO_RADIAN ( dShopLagitude- dCurrentLagitude );
	double dLongDiff = DEREE_TO_RADIAN ( dShopLongitude - dCurrentLongitude );

	double a = sin( dLatDiffRadian / 2 ) * sin( dLatDiffRadian / 2) +
		cos(dCurrentLagitudeRadian) * cos(dShopLagitudeRadian) *
		sin(dLongDiff/2) * sin(dLongDiff/2);
	double c = 2 * atan2( sqrt ( a ),sqrt ( 1 - a ) );

	double dResult = dRadius * c;
	return dResult;
}

int SendPushNotification( const char *token, const char *push_message, const char *cert_path, const char *key_path, short nBadge )
{
	apn_payload_ctx_ref payload_ctx = NULL;
    apn_ctx_ref ctx = NULL;
    apn_error_ref error = NULL;

    time_t time_now = 0;
    if(apn_init(&ctx, cert_path, key_path, QSG_NOTIFY_PASSPHRASE, &error) == APN_ERROR){
        printf("%s: %d\n", apn_error_message(error),  APN_ERR_CODE_WITHOUT_CLASS(apn_error_code(error)));
        apn_error_free(&error);
        return 1;
    }
    apn_set_mode(ctx, APN_MODE_SANDBOX, NULL);
    apn_add_token(ctx, token, NULL);
  
    if(apn_connect(ctx, &error) == APN_ERROR) {
       printf("Could not connected to Apple Push Notification Servece: %s (%d)\n", apn_error_message(error), APN_ERR_CODE_WITHOUT_CLASS(apn_error_code(error)));
       apn_payload_free(&payload_ctx);
       //apn_free(&ctx);
       apn_error_free(&error);
       return 1;
    }
    
	if(apn_payload_init(&payload_ctx, &error) == APN_ERROR) {
        printf("%s: %d\n", apn_error_message(error), APN_ERR_CODE_WITHOUT_CLASS(apn_error_code(error)));
        apn_free(&ctx);
        apn_error_free(&error);
        return 1;
    }
    time(&time_now);
    
    apn_payload_set_badge(payload_ctx, nBadge, NULL);
    apn_payload_set_body(payload_ctx, push_message, NULL);
    apn_payload_set_expiry(payload_ctx, time_now + 3600, NULL);
    apn_payload_set_sound(payload_ctx, "default",  NULL);
    apn_payload_add_custom_property_integer(payload_ctx, "int_property", 20, NULL);

    if(apn_send(ctx, payload_ctx, &error) == APN_ERROR) {
       printf("Could not sent push: %s (%d)\n", apn_error_message(error),  APN_ERR_CODE_WITHOUT_CLASS(apn_error_code(error)));
       
       if(APN_ERR_CODE_WITHOUT_CLASS(apn_error_code(error)) == APN_ERR_TOKEN_INVALID) {
           printf("Invalid token: %s\n", apn_error_invalid_token(error));
       }
       
       apn_close(ctx);
       apn_payload_free(&payload_ctx);
       apn_free(&ctx);
       apn_error_free(&error);
       return 1;
    } 
    apn_close(ctx);
    apn_payload_free(&payload_ctx);
    apn_free(&ctx);
    
    return 0;
}

//apn_payload_ctx_ref payload_ctx = NULL;
//apn_ctx_ref ctx = NULL;
//apn_error_ref error = NULL;
//short InitPushNotification()
//{
//	time_t time_now = 0;
//	if(apn_init(&ctx, QSG_SHOP_NOTIFY_CERT, QSG_SHOP_NOTIFY_KEY, QSG_NOTIFY_PASSPHRASE, &error) == APN_ERROR){
//		printf("%s: %d\n", apn_error_message(error),  APN_ERR_CODE_WITHOUT_CLASS(apn_error_code(error)));
//		apn_error_free(&error);
//		return 1;
//	}
//	apn_set_mode(ctx, APN_MODE_SANDBOX, NULL);
//	apn_add_token(ctx, token, NULL);
//
//	if(apn_connect(ctx, &error) == APN_ERROR) {
//		printf("Could not connected to Apple Push Notification Servece: %s (%d)\n", apn_error_message(error), APN_ERR_CODE_WITHOUT_CLASS(apn_error_code(error)));
//		apn_payload_free(&payload_ctx);
//		//apn_free(&ctx);
//		apn_error_free(&error);
//		return 1;
//	}
//}

int SendPushNotificationToUser( const char *token, const char *push_message, short nBadge )
{
	return SendPushNotification ( token, push_message, QSG_USER_NOTIFY_CERT, QSG_USER_NOTIFY_KEY, nBadge );
}

int SendPushNotificationToShop( const char *token, const char *push_message, short nBadge )
{
	return SendPushNotification ( token, push_message, QSG_SHOP_NOTIFY_CERT, QSG_SHOP_NOTIFY_KEY, nBadge );
}

bool ReplaceString ( string &szInput, const string &szOriginal, const string &szNew) 
{
	if ( szOriginal.length() <= 0 )
		return false;

	if ( szOriginal == szNew )
		return false;

	size_t nPos = 0;
	while ( ( nPos = szInput.find ( szOriginal, nPos ) ) != string::npos )	{
		szInput.replace ( nPos, szOriginal.length(), szNew );
		nPos = szNew.length();
	}
	return true;
}

#define QSG_SPECIAL_CHAR_NEED_TO_REPLACE		(7)

const string gszNeedToReplace[QSG_SPECIAL_CHAR_NEED_TO_REPLACE] = 
{
	"\\",
	"\x00",
	"\n",
	"\r",
	"'",
	"\"",
	"\x1a",
};

const string gszReplaceValue[QSG_SPECIAL_CHAR_NEED_TO_REPLACE] = 
{
	"\\\\",
	"\\0",
	"\\n",
	"\\r",
	"\\'",
	"\\\"",
	"\\z",
};

string EscapeString ( string &szInput )
{
	for ( short i = 0; i < QSG_SPECIAL_CHAR_NEED_TO_REPLACE; ++ i )	{
		ReplaceString ( szInput, gszNeedToReplace[i], gszReplaceValue[i] );
	}
	return szInput;
}