#include "stdafx.h"
#include "DBManager.h"
#include "Constants.h"
#include "LogFunc.h"
#include "Status.h"
#include "UtilityFunc.h"
#include <Windows.h>
#include <time.h>

CDBManager::CDBManager(void)
{
	m_pDriver = NULL;
	m_pConnection = NULL;
	m_pStatement = NULL;
	m_pResultSet = NULL;
	if ( OK == ConnectDB() )
		m_bConnected = true;
	else
		m_bConnected = false;
}


CDBManager::~CDBManager(void)
{
	DisconnectDB();
}

short CDBManager::ConnectDB()
{
	try {
		/* Create a connection */
		m_pDriver = get_driver_instance();
		m_pConnection = m_pDriver->connect( QSG_DB_SERVER, QSG_DB_USER, QSG_DB_PASSWORD );

		//m_pConnection = new sql::mysql::MySQL_Connection ( m_pDriver, m_Proxy, QSG_DB_SERVER, QSG_DB_USER, QSG_DB_PASSWORD );

		/* Connect to the MySQL test database */
		m_pConnection->setSchema ( QSG_DB_NAME );

		m_pStatement = m_pConnection->createStatement();

		m_pStatement->execute( "set names gbk" );

	} catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::DisconnectDB()
{
	if ( NULL != m_pConnection )
		delete m_pConnection;
	if ( NULL != m_pStatement )
		delete m_pStatement;
	if ( NULL != m_pResultSet )
		delete m_pResultSet;
	return OK;
}

short CDBManager::RegisterUser ( const STT_REGISTER_USER_INFO &stUserInfo )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql;
	szSql = "select ID from ";
	szSql += QSG_TABLE_USER;
	szSql += " where email = ";
	szSql += " \"" + EscapeString ( string ( stUserInfo.szEmail ) ) + "\";";
	LogToFile ( szSql.c_str() );

	try	{
		m_pResultSet = m_pStatement->executeQuery ( szSql );
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		if ( m_pResultSet->rowsCount() > 0 )
			return QSG_STATUS_EMAIL_ALREADY_REGISTERED;
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}

	szSql = "insert into ";
	szSql += QSG_TABLE_USER;
	szSql += " (email, password, name, phone, address ) VALUES ( ";
	szSql += " \"" + EscapeString ( string ( stUserInfo.szEmail ) ) + "\", ";
	szSql += " \"" + EscapeString ( string ( stUserInfo.szPassWord ) ) + "\", ";
	szSql += " \"" + EscapeString ( string ( stUserInfo.szName ) ) + "\", ";
	szSql += " \"" + EscapeString ( string ( stUserInfo.szPhone ) ) + "\", ";
	szSql += " \"" + EscapeString ( string ( stUserInfo.szAddress ) ) + "\" ); ";

	LogToFile ( szSql.c_str() );

	try
	{
		m_pStatement->execute ( szSql );
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::RegisterShop ( const STT_REGISTER_SHOP_INFO &stShopInfo )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql;
	szSql = "select ID from ";
	szSql += QSG_TABLE_SHOP;
	szSql += " where email = ";
	szSql += " \"" + EscapeString ( string ( stShopInfo.szEmail ) ) + "\";";
	LogToFile ( szSql.c_str() );

	try	{
		m_pResultSet = m_pStatement->executeQuery ( szSql );
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		if ( m_pResultSet->rowsCount() > 0 )
			return QSG_STATUS_EMAIL_ALREADY_REGISTERED;
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}

	szSql = "insert into ";
	szSql += QSG_TABLE_SHOP;
	szSql += " (email, password, name, phone, address, postcode, introduction, open_hours, category, latitude, longitude ) VALUES ( ";
	szSql += " \"" + EscapeString ( string ( stShopInfo.szEmail ) ) + "\", ";
	szSql += " \"" + EscapeString ( string ( stShopInfo.szPassWord ) ) + "\", ";
	szSql += " \"" + EscapeString ( string ( stShopInfo.szName ) ) + "\", ";
	szSql += " \"" + EscapeString ( string ( stShopInfo.szPhone ) ) + "\", ";
	szSql += " \"" + EscapeString ( string ( stShopInfo.szAddress ) ) + "\", ";
	szSql += " \"" + EscapeString ( string ( stShopInfo.szPostcode ) ) + "\", ";
	szSql += " \"" + EscapeString ( string ( stShopInfo.szIntroduction ) ) + "\", ";
	szSql += " \"" + EscapeString ( string ( stShopInfo.szOpenHours ) ) + "\", ";
	char szTemp[100];
	_snprintf ( szTemp, 100, "%d, ", stShopInfo.nCategory );
	szSql += szTemp;

	_snprintf ( szTemp, 100, "%f, ", stShopInfo.dLatidute );
	szSql += szTemp;

	_snprintf ( szTemp, 100, "%f ", stShopInfo.dLongitude );
	szSql += szTemp;
	szSql += ");";

	LogToFile ( szSql.c_str() );

	try
	{
		m_pStatement->execute( szSql );
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::UpdateShopProfile ( const STT_UPDATE_SHOP_PROFILE_CMD &stShopProfile)
{
	if ( ! m_bConnected )
		return NOK;

	string szSql;
	szSql = "update ";
	szSql += QSG_TABLE_SHOP;
	szSql += " set name = ";
	szSql += "\"" + string ( stShopProfile.szName ) + "\", ";
	szSql += "Phone = ";
	szSql += "\"" + string ( stShopProfile.szPhone ) + "\", ";
	szSql += "Address = ";
	szSql += "\"" + string ( stShopProfile.szAddress ) + "\", ";
	szSql += "Postcode = ";
	szSql += "\"" + string ( stShopProfile.szPostcode ) + "\", ";
	szSql += "open_hours = ";
	szSql += "\"" + string ( stShopProfile.szOpenHours ) + "\", ";
	szSql += "introduction = ";
	szSql += "\"" + string ( stShopProfile.szIntroduction ) + "\" ";
	szSql += "where ID = ";
	char szTemp[100];
	_snprintf ( szTemp, 100, "%d", stShopProfile.nID );
	szSql += szTemp;

	LogToFile ( szSql.c_str() );

	try
	{
		m_pStatement->execute( szSql );
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

void CDBManager::HandleException ( sql::SQLException &e, char *szFile, char *szFunction, int nLine )
{
	string szException = "# ERR: SQLException in ";
	szException += szFile;
	szException += "(" + string ( szFunction ) + ") on line ";
	char szTemp[100];
	_snprintf ( szTemp, 100, "%d; ", nLine );
	szException += szTemp;
	szException += " ERR: " + string ( e.what() );
	_snprintf ( szTemp, 100, " (MySQL error code: %d, SQLState: %d", e.getErrorCode(), e.getSQLState() );
	szException += szTemp;
	cout << szException << endl;
	LogToFile ( szException.c_str() );
}

short CDBManager::GetShops(LIST_GET_SHOP_SUMMARY &lstShop)
{
	if ( ! m_bConnected )
		return NOK;

	try {
		string szSql = "select ID, email, name, phone, address, postcode, category, latitude, longitude from qsg_shop";
		LogToFile ( szSql.c_str() );

		m_pResultSet = m_pStatement->executeQuery(szSql);
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		while ( m_pResultSet->next() ) {
			STT_GET_SHOP_SUMMARY_RPY stShopInfo;
			stShopInfo.nID = m_pResultSet->getInt ( "ID" );
			_snprintf ( stShopInfo.szEmail, sizeof (stShopInfo.szEmail), m_pResultSet->getString ( "email" ).c_str() );
			_snprintf ( stShopInfo.szName, sizeof (stShopInfo.szName), m_pResultSet->getString ( "name" ).c_str() );
			_snprintf ( stShopInfo.szPhone, sizeof (stShopInfo.szPhone), m_pResultSet->getString ( "phone" ).c_str() );
			_snprintf ( stShopInfo.szAddress, sizeof (stShopInfo.szAddress), m_pResultSet->getString ( "address" ).c_str() );
			_snprintf ( stShopInfo.szPostcode, sizeof (stShopInfo.szPostcode), m_pResultSet->getString ( "postcode" ).c_str() );
			stShopInfo.nCategory = m_pResultSet->getInt ( "category" );
			stShopInfo.dLatidute = m_pResultSet->getDouble("latitude");
			stShopInfo.dLongitude = m_pResultSet->getDouble("longitude");
			lstShop.push_back ( stShopInfo );
		}
	} catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::GetRecommendedShop(LIST_GET_SHOP_SUMMARY &lstShop)
{
	if ( ! m_bConnected )
		return NOK;

	try {
		char achSql[200];
		_snprintf ( achSql, sizeof ( achSql ), "select ID, email, name, phone, address, postcode, category, latitude, longitude from qsg_shop where recommended = 1 limit %d",
			QSG_MAX_RECOMMENDED_SHOP_NUM);
		LogToFile ( achSql );

		m_pResultSet = m_pStatement->executeQuery ( achSql );
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;

		if ( m_pResultSet->rowsCount() <= 0 )
			return QSG_STATUS_NO_MATCH_RESULT;

		while ( m_pResultSet->next() ) {
			STT_GET_SHOP_SUMMARY_RPY stShopInfo;
			stShopInfo.nID = m_pResultSet->getInt ( "ID" );
			_snprintf ( stShopInfo.szEmail, sizeof (stShopInfo.szEmail), m_pResultSet->getString ( "email" ).c_str() );
			_snprintf ( stShopInfo.szName, sizeof (stShopInfo.szName), m_pResultSet->getString ( "name" ).c_str() );
			_snprintf ( stShopInfo.szPhone, sizeof (stShopInfo.szPhone), m_pResultSet->getString ( "phone" ).c_str() );
			_snprintf ( stShopInfo.szAddress, sizeof (stShopInfo.szAddress), m_pResultSet->getString ( "address" ).c_str() );
			_snprintf ( stShopInfo.szPostcode, sizeof (stShopInfo.szPostcode), m_pResultSet->getString ( "postcode" ).c_str() );
			stShopInfo.nCategory = m_pResultSet->getInt ( "category" );
			stShopInfo.dLatidute = m_pResultSet->getDouble("latitude");
			stShopInfo.dLongitude = m_pResultSet->getDouble("longitude");
			lstShop.push_back ( stShopInfo );
		}
	} catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::SrchShop(const STT_SRCH_SHOP_CMD &stSrchShopCmd, LIST_GET_SHOP_SUMMARY &lstShop)
{
	if ( ! m_bConnected )
		return NOK;

	try {
		string szSql = "select ID, email, name, phone, address, postcode, category, latitude, longitude from qsg_shop where name like \"%";

		szSql += string ( stSrchShopCmd.szSrchText ) + "%\"";
		LogToFile ( szSql.c_str() );

		m_pResultSet = m_pStatement->executeQuery ( szSql );
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		while ( m_pResultSet->next() ) {
			STT_GET_SHOP_SUMMARY_RPY stShopInfo;
			stShopInfo.nID = m_pResultSet->getInt ( "ID" );
			_snprintf ( stShopInfo.szEmail, sizeof (stShopInfo.szEmail), m_pResultSet->getString ( "email" ).c_str() );
			_snprintf ( stShopInfo.szName, sizeof (stShopInfo.szName), m_pResultSet->getString ( "name" ).c_str() );
			_snprintf ( stShopInfo.szPhone, sizeof (stShopInfo.szPhone), m_pResultSet->getString ( "phone" ).c_str() );
			_snprintf ( stShopInfo.szAddress, sizeof (stShopInfo.szAddress), m_pResultSet->getString ( "address" ).c_str() );
			_snprintf ( stShopInfo.szPostcode, sizeof (stShopInfo.szPostcode), m_pResultSet->getString ( "postcode" ).c_str() );
			stShopInfo.nCategory = m_pResultSet->getInt ( "category" );
			stShopInfo.dLatidute = m_pResultSet->getDouble("latitude");
			stShopInfo.dLongitude = m_pResultSet->getDouble("longitude");
			lstShop.push_back ( stShopInfo );
		}
	} catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::GetFavoriteShop(const STT_GET_FAVORITE_SHOP_CMD &stGetFavoriteCmd, LIST_GET_SHOP_SUMMARY &lstShop)
{
	if ( ! m_bConnected )
		return NOK;

	try {
		char achSql[300];
		_snprintf ( achSql, sizeof ( achSql ),"select qsg_shop.ID, qsg_shop.email, qsg_shop.name, qsg_shop.phone, qsg_shop.address, qsg_shop.postcode, qsg_shop.category, qsg_shop.latitude, qsg_shop.longitude from qsg_shop left join qsg_favorite on qsg_shop.ID = qsg_favorite.shop_id where qsg_favorite.user_id = %d;", 
			stGetFavoriteCmd.nUserID );

		LogToFile ( achSql );

		m_pResultSet = m_pStatement->executeQuery ( achSql );
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		while ( m_pResultSet->next() ) {
			STT_GET_SHOP_SUMMARY_RPY stShopInfo;
			stShopInfo.nID = m_pResultSet->getInt ( 1 );
			_snprintf ( stShopInfo.szEmail, sizeof (stShopInfo.szEmail), m_pResultSet->getString ( 2 ).c_str() );
			_snprintf ( stShopInfo.szName, sizeof (stShopInfo.szName), m_pResultSet->getString ( 3 ).c_str() );
			_snprintf ( stShopInfo.szPhone, sizeof (stShopInfo.szPhone), m_pResultSet->getString ( 4 ).c_str() );
			_snprintf ( stShopInfo.szAddress, sizeof (stShopInfo.szAddress), m_pResultSet->getString ( 5 ).c_str() );
			_snprintf ( stShopInfo.szPostcode, sizeof (stShopInfo.szPostcode), m_pResultSet->getString ( 6 ).c_str() );
			stShopInfo.nCategory = m_pResultSet->getInt ( 7 );
			stShopInfo.dLatidute = m_pResultSet->getDouble(8);
			stShopInfo.dLongitude = m_pResultSet->getDouble(9);
			lstShop.push_back ( stShopInfo );
		}
	} catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::GetPopularShop( LIST_GET_SHOP_SUMMARY &lstShop)
{
	if ( ! m_bConnected )
		return NOK;

	try {
		time_t tCurrentTime = time ( NULL );
		time_t tOneWeekAgo = tCurrentTime - 7 * 24 * 3600;
		struct tm *pTimeInfo = localtime ( &tOneWeekAgo );
		char szOneWeeekAgoTime[50];

		_snprintf ( szOneWeeekAgoTime, sizeof ( szOneWeeekAgoTime ), "%d-%02d-%02d %s", pTimeInfo->tm_year + QSG_TIME_START_YEAR,
			pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday, QSG_Q_START_TIME );

		string szSql = "select qsg_shop.ID, qsg_shop.email, qsg_shop.name, qsg_shop.phone, qsg_shop.address, qsg_shop.postcode, qsg_shop.category, qsg_shop.latitude, qsg_shop.longitude, count(*) as c from qsg_shop left join qsg_q_table on qsg_shop.ID = qsg_q_table.shop_id where qsg_q_table.time > ";

		char szTemp[200];
		_snprintf ( szTemp, sizeof ( szTemp ), "\'%s\' and qsg_q_table.status = %d group by qsg_shop.ID order by c desc limit %d;", szOneWeeekAgoTime, Q_PROCESSED, QSG_MAX_POPULAR_SHOP_NUM );
		szSql += szTemp;

		LogToFile ( szSql.c_str() );

		m_pResultSet = m_pStatement->executeQuery ( szSql );
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		while ( m_pResultSet->next() ) {
			STT_GET_SHOP_SUMMARY_RPY stShopInfo;
			stShopInfo.nID = m_pResultSet->getInt ( 1 );
			_snprintf ( stShopInfo.szEmail, sizeof (stShopInfo.szEmail), m_pResultSet->getString ( 2 ).c_str() );
			_snprintf ( stShopInfo.szName, sizeof (stShopInfo.szName), m_pResultSet->getString ( 3 ).c_str() );
			_snprintf ( stShopInfo.szPhone, sizeof (stShopInfo.szPhone), m_pResultSet->getString ( 4 ).c_str() );
			_snprintf ( stShopInfo.szAddress, sizeof (stShopInfo.szAddress), m_pResultSet->getString ( 5 ).c_str() );
			_snprintf ( stShopInfo.szPostcode, sizeof (stShopInfo.szPostcode), m_pResultSet->getString ( 6 ).c_str() );
			stShopInfo.nCategory = m_pResultSet->getInt (7);
			stShopInfo.dLatidute = m_pResultSet->getDouble(8);
			stShopInfo.dLongitude = m_pResultSet->getDouble(9);
			stShopInfo.nTotalQNum = m_pResultSet->getInt ( 10 );
			lstShop.push_back ( stShopInfo );
		}
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::GetShopInfoByID(const int nShopID, STT_GET_SHOP_DETAIL_RPY &stShopDetail)
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select ID, email, name, phone, address, postcode, introduction, open_hours, category, latitude, longitude from qsg_shop where ID = ";
	char szTemp[10];
	_snprintf ( szTemp, 10, "%d; ", nShopID );
	szSql += szTemp;
	LogToFile ( szSql.c_str() );

	try {
		m_pResultSet = m_pStatement->executeQuery(szSql);
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		while ( m_pResultSet->next() ) {
			stShopDetail.nID = m_pResultSet->getInt ( "ID" );
			_snprintf ( stShopDetail.szEmail, sizeof (stShopDetail.szEmail), m_pResultSet->getString ( "email" ).c_str() );
			_snprintf ( stShopDetail.szName, sizeof (stShopDetail.szName), m_pResultSet->getString ( "name" ).c_str() );
			_snprintf ( stShopDetail.szPhone, sizeof (stShopDetail.szPhone), m_pResultSet->getString ( "phone" ).c_str() );
			_snprintf ( stShopDetail.szAddress, sizeof (stShopDetail.szAddress), m_pResultSet->getString ( "address" ).c_str() );
			_snprintf ( stShopDetail.szPostcode, sizeof (stShopDetail.szPostcode), m_pResultSet->getString ( "postcode" ).c_str() );
			_snprintf ( stShopDetail.szIntroduction, sizeof (stShopDetail.szIntroduction), m_pResultSet->getString ( "introduction" ).c_str() );
			_snprintf ( stShopDetail.szOpenHours, sizeof (stShopDetail.szOpenHours), m_pResultSet->getString ( "open_hours" ).c_str() );
			stShopDetail.nCategory = m_pResultSet->getInt ( "category" );
			stShopDetail.dLatidute = m_pResultSet->getDouble("latitude");
			stShopDetail.dLongitude = m_pResultSet->getDouble("longitude");
		}
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::GetShopInfoByEmail(const STT_GET_SHOP_BY_EMAIL_CMD &stCmd, STT_GET_SHOP_DETAIL_RPY &stShopDetail)
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select ID, email, name, phone, address, postcode, introduction, open_hours, category, latitude, longitude from qsg_shop where email = \'";
	szSql += EscapeString ( string ( stCmd.szEmail ) );
	szSql += "\';";
	LogToFile ( szSql.c_str() );

	try {
		m_pResultSet = m_pStatement->executeQuery(szSql);
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		while ( m_pResultSet->next() ) {
			stShopDetail.nID = m_pResultSet->getInt ( "ID" );
			_snprintf ( stShopDetail.szEmail, sizeof (stShopDetail.szEmail), m_pResultSet->getString ( "email" ).c_str() );
			_snprintf ( stShopDetail.szName, sizeof (stShopDetail.szName), m_pResultSet->getString ( "name" ).c_str() );
			_snprintf ( stShopDetail.szPhone, sizeof (stShopDetail.szPhone), m_pResultSet->getString ( "phone" ).c_str() );
			_snprintf ( stShopDetail.szAddress, sizeof (stShopDetail.szAddress), m_pResultSet->getString ( "address" ).c_str() );
			_snprintf ( stShopDetail.szPostcode, sizeof (stShopDetail.szPostcode), m_pResultSet->getString ( "postcode" ).c_str() );
			_snprintf ( stShopDetail.szIntroduction, sizeof (stShopDetail.szIntroduction), m_pResultSet->getString ( "introduction" ).c_str() );
			_snprintf ( stShopDetail.szOpenHours, sizeof (stShopDetail.szOpenHours), m_pResultSet->getString ( "open_hours" ).c_str() );
			stShopDetail.nCategory = m_pResultSet->getInt ( "category" );
			stShopDetail.dLatidute = m_pResultSet->getDouble("latitude");
			stShopDetail.dLongitude = m_pResultSet->getDouble("longitude");
		}
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::HandleUserLogIn ( const STT_LOGIN_CMD &stLoginCmd, STT_USER_LOGIN_RPY  &stLoginRpy )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select ID, email, name, phone, address, credit_point from qsg_user where email = ";
	string szEmail ( stLoginCmd.szEmail );
	EscapeString ( szEmail );
	szSql += "\"" + szEmail + "\" and password = ";

	string szPassword ( stLoginCmd.szPassWord );
	EscapeString ( szPassword );
	szSql += "\"" + szPassword + "\";";
	LogToFile ( szSql.c_str() );

	try
	{
		m_pResultSet = m_pStatement->executeQuery(szSql);
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		if ( m_pResultSet->rowsCount() <= 0 )
			return QSG_STATUS_USER_PSW_WRONG;
		if ( m_pResultSet->next() )	{
			stLoginRpy.nID = m_pResultSet->getInt("ID");
			_snprintf ( stLoginRpy.szEmail, sizeof ( stLoginRpy.szEmail ), m_pResultSet->getString ( "email" ).c_str() );
			_snprintf ( stLoginRpy.szName, sizeof ( stLoginRpy.szName ), m_pResultSet->getString ( "name" ).c_str() );
			_snprintf ( stLoginRpy.szPhone, sizeof ( stLoginRpy.szPhone ), m_pResultSet->getString ( "phone" ).c_str() );
			_snprintf ( stLoginRpy.szAddress, sizeof ( stLoginRpy.szAddress ), m_pResultSet->getString ( "address" ).c_str() );
			stLoginRpy.nCreditPoint = m_pResultSet->getInt("credit_point");
			szSql = "update qsg_user set login_time = now(), ";
			char szTemp[300];
			string szDeviceToken;
			if ( ! stLoginCmd.bDeviceTokenValid )
				szDeviceToken = "";
			else
				szDeviceToken = stLoginCmd.szDeviceToken;
			_snprintf ( szTemp, 300, "valid_device_token = %d, device_token = \"%s\"",
				stLoginCmd.bDeviceTokenValid, szDeviceToken.data() );
			szSql += szTemp;

			szSql += " where ID = ";
			szSql +=  m_pResultSet->getString("ID").c_str();
			m_pStatement->execute ( szSql );
		}
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}

	return OK;
}

short CDBManager::HandleShopLogIn ( const STT_LOGIN_CMD &stLoginCmd, STT_SHOP_LOGIN_RPY  &stLoginRpy )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select ID, email, name, phone, address, postcode, open_hours, introduction from qsg_shop where email = ";
	szSql += "\"" + string ( stLoginCmd.szEmail ) + "\" and password = ";
	szSql += "\"" + string ( stLoginCmd.szPassWord ) + "\"";
	LogToFile ( szSql.c_str() );

	try
	{
		m_pResultSet = m_pStatement->executeQuery(szSql);
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		if ( m_pResultSet->rowsCount() <= 0 )
			return QSG_STATUS_USER_PSW_WRONG;
		if ( m_pResultSet->next() )	{
			stLoginRpy.nID = m_pResultSet->getInt("ID");
			_snprintf ( stLoginRpy.szEmail, sizeof ( stLoginRpy.szEmail ), m_pResultSet->getString ( "email" ).c_str() );
			_snprintf ( stLoginRpy.szName, sizeof ( stLoginRpy.szName ), m_pResultSet->getString ( "name" ).c_str() );
			_snprintf ( stLoginRpy.szPhone, sizeof ( stLoginRpy.szPhone ), m_pResultSet->getString ( "phone" ).c_str() );
			_snprintf ( stLoginRpy.szAddress, sizeof ( stLoginRpy.szAddress ), m_pResultSet->getString ( "address" ).c_str() );
			_snprintf ( stLoginRpy.szPostcode, sizeof ( stLoginRpy.szPostcode ), m_pResultSet->getString ( "postcode" ).c_str() );
			_snprintf ( stLoginRpy.szOpenHours, sizeof ( stLoginRpy.szOpenHours ), m_pResultSet->getString ( "open_hours" ).c_str() );
			_snprintf ( stLoginRpy.szIntroduction, sizeof ( stLoginRpy.szIntroduction ), m_pResultSet->getString ( "introduction" ).c_str() );

			szSql = "update qsg_shop set login_time = now(), logged_in = true, ";

			char szTemp[300];
			string szDeviceToken;
			if ( ! stLoginCmd.bDeviceTokenValid )
				szDeviceToken = "";
			else
				szDeviceToken = stLoginCmd.szDeviceToken;
			_snprintf ( szTemp, 300, "valid_device_token = %d, device_token = \"%s\"",
				stLoginCmd.bDeviceTokenValid, szDeviceToken.data() );
			szSql += szTemp;

			szSql += " where ID = ";
			szSql +=  m_pResultSet->getString("ID").c_str();

			LogToFile ( szSql.c_str() );
			m_pStatement->execute ( szSql );
		}
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}

	return OK;
}

short CDBManager::HandleShopLogOut ( const int nShopID )
{
	if ( ! m_bConnected )
		return NOK;

	char szSql[200];
	_snprintf ( szSql, 200, "update qsg_shop set logged_in = false where ID = %d;", nShopID );
	LogToFile ( szSql );

	try
	{
		m_pStatement->execute ( szSql );
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}


short CDBManager::InqueryUserInfo ( short nUserID, STT_Q_SERVER_TO_SHOP_INFO &stInfo )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select name, phone from qsg_user where id = ";
	char szTemp[10];
	_snprintf ( szTemp, 10, "%d", nUserID );
	szSql += szTemp;
	LogToFile ( szSql.c_str() );

	try
	{
		m_pResultSet = m_pStatement->executeQuery(szSql);
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		if ( m_pResultSet->rowsCount() <= 0 )
			return QSG_STATUS_WRONG_USER_ID_FOR_Q;
		if ( m_pResultSet->next() )	{
			_snprintf ( stInfo.szUserName, sizeof ( stInfo.szUserName ), m_pResultSet->getString ( "name" ).c_str() );
			_snprintf ( stInfo.szPhone, sizeof ( stInfo.szPhone ), m_pResultSet->getString ( "phone" ).c_str() );
		}
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}

	return OK;
}

//short CDBManager::HandleQ( const STT_Q_USER_TO_SERVER_INFO &stQInfo )
//{
//	if ( ! m_bConnected )
//		return NOK;
//
//	char szValue[20];
//	string szSql;
//	szSql = "insert into ";
//	szSql += QSG_TABLE_Q;
//	szSql += " (user_id, shop_id, time ) VALUES ( ";
//	_snprintf ( szValue, 20, "%d, ", stQInfo.nUserID );
//	szSql += szValue;
//
//	_snprintf ( szValue, 20, "%d, ", stQInfo.nShopID );
//	szSql += szValue;
//
//	szSql += "now() )";
//
//	LogToFile ( szSql.c_str() );
//
//	try
//	{
//		m_pStatement->execute( szSql );
//	}catch (sql::SQLException &e) {
//		HandleException ( e, AT );
//		return NOK;
//	}
//	return OK;
//}

short CDBManager::AddQInfo ( const STT_Q_SHOP_TO_SERVER_RPY &stRpy )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql;
	szSql = "insert into ";
	szSql += QSG_TABLE_Q;
	szSql += " (user_id, shop_id, time ) VALUES ( ";
	char szTemp[100];
	_snprintf ( szTemp, 100, "%d, ", stRpy.nUserID );
	szSql += szTemp;
	_snprintf ( szTemp, 100, "%d, ", stRpy.nShopID );
	szSql += szTemp;
	szSql += "now() )";

	LogToFile ( szSql.c_str() );

	try
	{
		m_pStatement->execute( szSql );
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::AddToQ ( const STT_Q_USER_TO_SERVER_INFO stQInfo, short &nQNo )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql;
	char szTemp[100];

	time_t tCurrentTime = time ( NULL );
	struct tm *pTimeInfo = localtime ( &tCurrentTime );
	char szTodayQStartTime[50];
	if ( pTimeInfo->tm_hour < 4 )	{
		--pTimeInfo->tm_mday;
	}

	_snprintf ( szTodayQStartTime, sizeof ( szTodayQStartTime ), "%d-%02d-%02d %s", pTimeInfo->tm_year + QSG_TIME_START_YEAR,
		pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday, QSG_Q_START_TIME );
	szSql = "select q_no from ";
	szSql += QSG_TABLE_Q;
	_snprintf ( szTemp, sizeof ( szTemp ), " where shop_id = %d and time > \'%s\' order by time desc;", stQInfo.nShopID, szTodayQStartTime );
	szSql += szTemp;
	try
	{
		LogToFile ( szSql.c_str() );
		m_pResultSet = m_pStatement->executeQuery( szSql );
		nQNo = m_pResultSet->rowsCount() + 1;
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	
	szSql = "insert into ";
	szSql += QSG_TABLE_Q;
	szSql += " (user_id, shop_id, q_no, customer_num, time ) VALUES ( ";
	_snprintf ( szTemp, 100, "%d, %d, %d, %d, ", stQInfo.nUserID, stQInfo.nShopID, nQNo, stQInfo.nCustomerNum );
	szSql += szTemp;
	szSql += "now() );";

	string szSqlAddQNum;
	szSqlAddQNum = "update ";
	szSqlAddQNum += QSG_TABLE_SHOP;
	szSqlAddQNum += " SET current_q_num = current_q_num + 1 where ID = ";
	_snprintf ( szTemp, 100, "%d;", stQInfo.nShopID );
	szSqlAddQNum += szTemp;

	try
	{
		LogToFile ( szSql.c_str() );
		m_pStatement->execute( szSql );

		LogToFile ( szSqlAddQNum.c_str() );
		m_pStatement->execute( szSqlAddQNum );
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::UserCancelQ(const STT_USER_CANCEL_Q_CMD &stCancelQInfo)
{
	if ( ! m_bConnected )
		return NOK;

	char achSql[200];
	_snprintf ( achSql, sizeof ( achSql ), "update %s set status = %d where shop_id = %d and user_id = %d;", QSG_TABLE_Q,
		Q_CANCELED, stCancelQInfo.nShopID, stCancelQInfo.nUserID );

	try
	{
		LogToFile ( achSql );
		m_pStatement->execute( achSql );
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::IsUserCancelQExceedLimit ( short nUserID )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select q_no from ";
	szSql += QSG_TABLE_Q;
	szSql += " where user_id = ";

	time_t tCurrentTime = time ( NULL );
	time_t t24HourBefore = tCurrentTime - 24 * 3600;
	struct tm *pTimeInfo = localtime ( &t24HourBefore );
	char sz24HourBefore[50];
	
	_snprintf ( sz24HourBefore, sizeof ( sz24HourBefore ), "%d-%02d-%02d %02d:%02d:%02d", pTimeInfo->tm_year + QSG_TIME_START_YEAR,
		pTimeInfo->tm_mon + 1, pTimeInfo->tm_mday, pTimeInfo->tm_hour, pTimeInfo->tm_min, pTimeInfo->tm_sec );

	char szTemp[100];
	_snprintf ( szTemp, 100, "%d and status = %d and time > \'%s\';", nUserID, Q_CANCELED, sz24HourBefore );
	szSql += szTemp;
	LogToFile ( szSql.c_str() );

	try
	{
		m_pResultSet = m_pStatement->executeQuery(szSql);
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		if ( m_pResultSet->rowsCount() >= QSG_CANCEL_Q_LIMIT )
			return QSG_STATUS_CANCEL_Q_EXCEED_LMT;
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::InqueryQNo ( const int nUserID, const int nShopID, short &nQNo, short &nQInfront )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select user_id, q_no from ";
	szSql += QSG_TABLE_Q;
	szSql += " where shop_id = ";
	char szTemp[100];
	_snprintf ( szTemp, 100, "%d and status = %d;", nShopID, Q_SUBMITTED );
	szSql += szTemp;
	LogToFile ( szSql.c_str() );
	nQInfront = 0;
	bool bInQueue = false;

	try
	{
		m_pResultSet = m_pStatement->executeQuery(szSql);
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		if ( m_pResultSet->rowsCount() <= 0 )
			return QSG_STATUS_WRONG_USER_ID_FOR_Q;
		
		while ( m_pResultSet->next() )	{
			int nTempUserID = m_pResultSet->getInt("user_id");
			if ( nUserID == nTempUserID )	{
				nQNo = m_pResultSet->getInt("q_no");
				bInQueue = true;
				break;
			}
			++ nQInfront;
		}
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}

	if ( bInQueue )
		return OK;
	else
		return QSG_STATUS_USER_NOT_IN_Q;
}

//User want to know what's his current Q number
short CDBManager::InqueryQNo ( const int nUserID, STT_USER_INQUERY_Q_RPY &stInqueryQRpy  )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select shop_id from ";
	szSql +=QSG_TABLE_Q;
	szSql += " where user_id = ";
	char szTemp[100];
	_snprintf ( szTemp, 100, "%d and status = %d;", nUserID, Q_SUBMITTED );
	szSql += szTemp;

	LogToFile ( szSql.c_str() );

	try
	{
		m_pResultSet = m_pStatement->executeQuery(szSql);
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		if ( m_pResultSet->rowsCount() <= 0 )
			return QSG_STATUS_WRONG_USER_ID_FOR_Q;
		if ( m_pResultSet->next() )	{
			stInqueryQRpy.nShopID = m_pResultSet->getInt("shop_id");
		}else	{
			return QSG_STATUS_WRONG_USER_ID_FOR_Q;
		}

		short nStatus = InqueryQNo ( nUserID, stInqueryQRpy.nShopID, stInqueryQRpy.nQNo, stInqueryQRpy.nQInfront );
		if ( nStatus != OK )
			return nStatus;

		//Just to get the shop name for the inquiry
		STT_GET_SHOP_DETAIL_RPY stShopDetail;
		nStatus = GetShopInfoByID ( stInqueryQRpy.nShopID, stShopDetail );
		if ( nStatus != OK )
			return nStatus;

		_snprintf ( stInqueryQRpy.szShopName, sizeof ( stInqueryQRpy.szShopName ), stShopDetail.szName );
		
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}

	return OK;
}

short CDBManager::InqueryShopQLen ( const int nShopID, int &nQLen )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select user_id from ";
	szSql +=QSG_TABLE_Q;
	szSql += " where shop_id = ";
	char szTemp[100];
	_snprintf ( szTemp, 100, "%d and status = %d", nShopID, Q_SUBMITTED );
	szSql += szTemp;
	LogToFile ( szSql.c_str() );
	nQLen = 0;

	try
	{
		m_pResultSet = m_pStatement->executeQuery(szSql);
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		nQLen = m_pResultSet->rowsCount();
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}

	return OK;
}

short CDBManager::ShopProcessQ ( const STT_SHOP_PROCESS_Q_CMD &stProcessQCmd )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql;
	szSql = "update ";
	szSql += QSG_TABLE_Q;
	char szTemp[200];
	_snprintf ( szTemp, 200, " set status = %d, process_time = now() where shop_id = %d and user_id = %d and q_no = %d;", 
		stProcessQCmd.nStatus, stProcessQCmd.nShopID, stProcessQCmd.nUserID, stProcessQCmd.nQNo );
	szSql += szTemp;

	LogToFile ( szSql.c_str() );

	try
	{
		m_pStatement->execute( szSql );

		char achSqlUpdateUserCredit[100];
		_snprintf ( achSqlUpdateUserCredit, sizeof ( achSqlUpdateUserCredit ), "update %s set credit_point = credit_point %s 1 where ID = %d;",
			QSG_TABLE_USER, stProcessQCmd.nStatus == Q_PROCESSED?"+":"-", stProcessQCmd.nUserID );
		LogToFile ( achSqlUpdateUserCredit );
		m_pStatement->execute( achSqlUpdateUserCredit );
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

//short CDBManager::GetShopQUserList ( const int nShopID, LIST_USER_CLIENT &lstUser)
//{
//	if ( ! m_bConnected )
//		return NOK;
//
//	string szSql = "select user_id from ";
//	szSql +=QSG_TABLE_Q;
//	szSql += " where shop_id = ";
//	char szTemp[100];
//	_snprintf ( szTemp, 100, "%d and status = %d;", nShopID, Q_SUBMITTED );
//	szSql += szTemp;
//	LogToFile ( szSql.c_str() );
//	short nQLen = 0;
//
//	try
//	{
//		m_pResultSet = m_pStatement->executeQuery(szSql);
//		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
//		nQLen = m_pResultSet->rowsCount();
//		cout << "Rows selected " << nQLen << endl;
//		while ( m_pResultSet->next() ) {
//			STT_USER_CLIENT_SOCKET stUserClient;
//			stUserClient.nUserID = m_pResultSet->getInt ( "user_id" );
//			lstUser.push_back ( stUserClient );
//		}
//	}catch (sql::SQLException &e) {
//		HandleException ( e, AT );
//		return NOK;
//	}
//
//	return OK;
//}

short CDBManager::GetShopQUserInfoList ( const int nShopID, LIST_Q_USER_INFO &lstUserInfo )
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select qsg_user.ID, qsg_user.email, qsg_user.name, qsg_user.phone, qsg_user.credit_point, qsg_q_table.time, qsg_q_table.q_no, qsg_q_table.customer_num from qsg_user left join qsg_q_table on qsg_user.ID = qsg_q_table.user_id where qsg_q_table.shop_id = ";
	char szTemp[100];
	_snprintf ( szTemp, 100, "%d and status = %d order by qsg_q_table.time;", nShopID, Q_SUBMITTED );
	szSql += szTemp;
	LogToFile ( szSql.c_str() );
	short nQLen = 0;

	try
	{
		m_pResultSet = m_pStatement->executeQuery(szSql);
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		nQLen = m_pResultSet->rowsCount();
		while ( m_pResultSet->next() ) {
			STT_Q_USER_INFO stQUserInfo;
			stQUserInfo.nID = m_pResultSet->getInt ( 1 );
			_snprintf ( stQUserInfo.szEmail, sizeof (stQUserInfo.szEmail), m_pResultSet->getString ( 2 ).c_str() );
			_snprintf ( stQUserInfo.szName, sizeof (stQUserInfo.szName), m_pResultSet->getString ( 3 ).c_str() );
			_snprintf ( stQUserInfo.szPhone, sizeof (stQUserInfo.szPhone), m_pResultSet->getString ( 4 ).c_str() );
			stQUserInfo.nCreditPoint = m_pResultSet->getInt ( 5 );
			_snprintf ( stQUserInfo.szQTime, sizeof (stQUserInfo.szQTime), m_pResultSet->getString ( 6 ).c_str() );
			stQUserInfo.nQNo = m_pResultSet->getInt ( 7 );
			stQUserInfo.nCustomerNum = m_pResultSet->getInt ( 8 );
			lstUserInfo.push_back ( stQUserInfo );
		}
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}

	return OK;
}

short CDBManager::UpdateShopActiveTime(int nShopID, time_t tActiveTime)
{
	if ( ! m_bConnected )
		return NOK;

	string szSql;
	szSql = "update ";
	szSql += QSG_TABLE_SHOP;
	char szTemp[100];
	_snprintf ( szTemp, 100, " set active_time = %d where ID = %d;", (int)tActiveTime, nShopID );
	szSql += szTemp;

	LogToFile ( szSql.c_str() );

	try
	{
		m_pStatement->execute( szSql );
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::GetShopActiveTime(int nShopID, bool &bLogin, time_t &time)
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select active_time, logged_in from ";
	szSql += QSG_TABLE_SHOP;
	szSql += " where ID = ";
	char szTemp[15];
	_snprintf ( szTemp, 15, "%d;", nShopID );
	szSql += szTemp;
	LogToFile ( szSql.c_str() );

	try
	{
		m_pResultSet = m_pStatement->executeQuery(szSql);
		
		if ( m_pResultSet->rowsCount() > 0 )	{
			m_pResultSet->next();
			time = m_pResultSet->getInt ( "active_time" );
			bLogin = m_pResultSet->getInt ( "logged_in" );
		}
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}

	return OK;
}

short CDBManager::GetShopDeviceToken ( int nShopID, STT_DEVICE_TOKEN &stDeviceToken)
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select valid_device_token, device_token from ";
	szSql += QSG_TABLE_SHOP;
	szSql += " where ID = ";
	char szTemp[15];
	_snprintf ( szTemp, 15, "%d;", nShopID );
	szSql += szTemp;
	LogToFile ( szSql.c_str() );

	try
	{
		m_pResultSet = m_pStatement->executeQuery(szSql);
		
		if ( m_pResultSet->rowsCount() > 0 )	{
			m_pResultSet->next();
			stDeviceToken.bDeviceTokenValid = m_pResultSet->getInt ( "valid_device_token" );
			_snprintf ( stDeviceToken.szDeviceToken, sizeof ( stDeviceToken.szDeviceToken ), m_pResultSet->getString ( "device_token" ).c_str() );
		}
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::GetUserDeviceToken ( int nShopID, STT_DEVICE_TOKEN &stDeviceToken)
{
	if ( ! m_bConnected )
		return NOK;

	string szSql = "select valid_device_token, device_token from ";
	szSql += QSG_TABLE_USER;
	szSql += " where ID = ";
	char szTemp[15];
	_snprintf ( szTemp, 15, "%d;", nShopID );
	szSql += szTemp;
	LogToFile ( szSql.c_str() );

	try
	{
		m_pResultSet = m_pStatement->executeQuery(szSql);
		
		if ( m_pResultSet->rowsCount() > 0 )	{
			m_pResultSet->next();
			stDeviceToken.bDeviceTokenValid = m_pResultSet->getInt ( "valid_device_token" );
			_snprintf ( stDeviceToken.szDeviceToken, sizeof ( stDeviceToken.szDeviceToken ), m_pResultSet->getString ( "device_token" ).c_str() );
		}
	}catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::GetUserQHistory ( const STT_GET_USER_Q_HISTORY_CMD &stGetQHistoryCmd, LIST_USER_Q_HISTORY &lstUserQHistory )
{
	if ( ! m_bConnected )
		return NOK;

	try {
		char achSql[300];
		_snprintf ( achSql, sizeof ( achSql ), "select qsg_shop.ID, qsg_shop.name, qsg_q_table.time from qsg_shop left join qsg_q_table on qsg_shop.ID = qsg_q_table.shop_id where qsg_q_table.user_id = %d and qsg_q_table.status = %d order by qsg_q_table.time desc limit 10;",
			stGetQHistoryCmd.nUserID, Q_PROCESSED );
		LogToFile ( achSql );

		m_pResultSet = m_pStatement->executeQuery ( achSql );
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		if ( m_pResultSet->rowsCount() <= 0 )
			return QSG_STATUS_NO_Q_HISTORY;
		while ( m_pResultSet->next() ) {
			STT_GET_USER_Q_HISTORY_RPY stQHistoryRpy;
			stQHistoryRpy.nShopID = m_pResultSet->getInt ( 1 );
			_snprintf ( stQHistoryRpy.szShopName, sizeof (stQHistoryRpy.szShopName), m_pResultSet->getString ( 2 ).c_str() );
			_snprintf ( stQHistoryRpy.szQTime, sizeof ( stQHistoryRpy.szQTime ), m_pResultSet->getString ( 3 ).c_str() );	
			lstUserQHistory.push_back ( stQHistoryRpy );
		}
	} catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::AddShopToFavorite ( const STT_ADD_TO_FAVORITE_CMD &stAddFavoriteCmd )
{
	if ( ! m_bConnected )
		return NOK;

	try {
		char achSql[300];
		_snprintf ( achSql, sizeof ( achSql ), "select ID from %s where user_id = %d and shop_id = %d;",
			QSG_TABLE_FAVORITE, stAddFavoriteCmd.nUserID, stAddFavoriteCmd.nShopID );
		LogToFile ( achSql );
		m_pResultSet = m_pStatement->executeQuery ( achSql );
		cout << "Rows selected " << m_pResultSet->rowsCount() << endl;
		if ( m_pResultSet->rowsCount() > 0 )
			return QSG_STATUS_ALREADY_ADDED;

		_snprintf ( achSql, sizeof ( achSql ), "insert into %s (user_id, shop_id) VALUES ( %d, %d );",
			QSG_TABLE_FAVORITE, stAddFavoriteCmd.nUserID, stAddFavoriteCmd.nShopID );
		LogToFile ( achSql );
		m_pStatement->execute ( achSql );
	} catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}

short CDBManager::AddFeedBack ( const STT_SEND_FEED_BACK_CMD &stSendFeedBackCmd )
{
	if ( ! m_bConnected )
		return NOK;

	try {
		string szSql;
		szSql = "insert into ";
		szSql += QSG_TABLE_FEED_BACK;

		char achTemp[100];
		_snprintf ( achTemp, sizeof ( achTemp ), " (client_type, comment, time) values (%d, \'", stSendFeedBackCmd.nClientType );
		szSql += achTemp;
		szSql += EscapeString ( string ( stSendFeedBackCmd.szFeedBack ) );
		szSql += "\', now() );";
		LogToFile ( szSql.c_str() );
		m_pStatement->execute ( szSql );
	} catch (sql::SQLException &e) {
		HandleException ( e, AT );
		return NOK;
	}
	return OK;
}