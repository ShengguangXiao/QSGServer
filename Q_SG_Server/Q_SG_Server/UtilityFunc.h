#ifndef _UTILITY_FUNC_H_
#define _UTILITY_FUNC_H_

#include <string>
using namespace std;

#define DEREE_TO_RADIAN(degree)		( (degree) * 3.1415926 / 180.f )
double CalculateDistanceOfLocation(double dCurrentLagitude, double dCurrentLongitude, double dShopLagitude, double dShopLongitude);
int SendPushNotificationToShop( const char *token, const char *push_message, short nBadge );
int SendPushNotificationToUser( const char *token, const char *push_message, short nBadge );
string EscapeString ( string &szInput );

#endif