#include "MySqlWrapper.h"

#include "Logger.h"
#include "tool.h"
#include <string.h>

#pragma warning(disable:4996)
#pragma warning(disable:4244)
#pragma warning(disable:4018)

#ifdef _DBGATE_
	extern void DebugLogMsg(const char* szText);
#endif

namespace MySQL
{
    Connection::Connection()
	    : m_bConnected (false)
	    , m_dwLastCheckTimeMS (0)
        , m_bNeedReconnect (false)
    {
	    m_szBuf[0] = '\0';
	    m_szEscapeBuf[0] = '\0';

	    memset (&m_dMYSQL, 0, sizeof (m_dMYSQL));
    }

    Connection::~Connection()
    {
	    Cleanup ();
    }

    bool Connection::Startup (const std::string &strHost, 
                              const std::string &strUser,
                              const std::string &strPassword,
                              const std::string &strDatabaseName,
                              int wPort/* = 53814*/ )
    {
	    if (IsOpened ())
	    {
		    Cleanup ();
	    }

	    m_strHost = strHost;
	    m_strUser = strUser;
	    m_strPassword = strPassword;
        m_strDB = strDatabaseName;
	    do
	    {
		    if(!mysql_init(&m_dMYSQL))
			    break;

		    {
			    char cReconnect = 1;
			    if(mysql_options(&m_dMYSQL,MYSQL_OPT_RECONNECT,&cReconnect) != 0)
				    break;
		    }
		    if(!mysql_real_connect(&m_dMYSQL,m_strHost.c_str(),m_strUser.c_str(),m_strPassword.c_str(),
			    strDatabaseName.c_str(),wPort,NULL,CLIENT_MULTI_STATEMENTS | CLIENT_MULTI_RESULTS | CLIENT_TRANSACTIONS))
            {
               NLog->warning ("连接数据库失败! Error: %s", mysql_error(&m_dMYSQL));
			    break;
            }

		    if(mysql_set_character_set(&m_dMYSQL,"gbk") != 0)
			    OutputError("mysql_set_character_set", true);

		    ExecuteSQL("SET NAMES gbk");

		    return m_bConnected = true, true;
	    }
        while (0);

	    return false;
    }

    bool Connection::Startup (const std::string &strConnection)
    {
	    char szHost[512] = "";
	    char szUser[512] = "";
	    char szPswd[512] = "";
	    char szDB[512] = "";
	    int nPort = 0;

	    if(sscanf(strConnection.c_str(),"host=%s user=%s pwd=%s db=%s port=%d",szHost,szUser,szPswd,szDB,&nPort) != 5)
		    return false;

	    return Startup (szHost, szUser, szPswd, szDB, nPort);
    }

    void Connection::Cleanup()
    {
	    if (m_bConnected)
	    {
		    mysql_close (&m_dMYSQL);
		    m_bConnected = false;
	    }
    }

    bool Connection::IsOpened()
    {
	    return m_bConnected;
    }

    bool Connection::SelectDatabase (const std::string &strDatabaseName)
    {
	    return mysql_select_db (&m_dMYSQL,strDatabaseName.c_str()) == 0;
    }

    bool Connection::ExecuteSQLNonResult (const char* fmt, ...)
    {
	    va_list vl;
	    va_start(vl,fmt);
	    //_vsnprintf_s(m_szBuf,_countof(m_szBuf),fmt,vl);
		vsnprintf(m_szBuf, sizeof(m_szBuf) - 1, fmt, vl);
	    va_end(vl);

	    return ExecuteSQL(m_szBuf);
    }

    bool Connection::ExecuteSQL (const std::string &strSQL)
    {
	    if (! IsOpened ())
		    return false;


	    CleanupResults ();
	    if(mysql_real_query(&m_dMYSQL,strSQL.c_str(),strSQL.length()) != 0)
	    {
		    OutputError(strSQL);
		    return false;
	    }

        return true;
    }

    bool Connection::ExecuteSQL (const std::string& strSQL,Recordset& rs,bool bFetchFirst /*= true*/ )
    {
	    bool bResult = ExecuteSQL(strSQL);
	    if(!bResult)
		    return false;

	    MYSQL_RES* pRes = mysql_store_result(*this);

	    if (! pRes) return true;

	    if (! rs.Initial (pRes)) return false;
    	
        if (bFetchFirst)
		    rs.Fetch();

		return bResult;
	}

	bool Connection::ExecuteSQL(const std::string& strSQL,RecordsetMgr&rs,bool bFetchFirst /* = true */)
	{
		bool bResult = ExecuteSQL(strSQL);
		if(!bResult)
			return false;

		return true;
	}

    bool Connection::ExecuteSQL( const std::vector<std::string>& vtSQLs,bool bTransaction /*= true*/ )
    {
	    bool bSuccessful = true;

	    if(bTransaction) this->BeginTransaction();

	    std::string strSQL;
	    strSQL.reserve(2000*1000);

	    for(size_t i = 0; i < vtSQLs.size(); ++i)
	    {
		    strSQL = strSQL + vtSQLs[i] + ";";
	    }

	    if(strSQL.length() > 0)
	    {
		    bSuccessful = ExecuteSQL(strSQL);
	    }
	    if(bSuccessful)
	    {
		    if(bTransaction) this->CommitTransaction();
	    }
	    else
	    {
		    if(bTransaction) this->RollbackTransaction();
	    }
	    return bSuccessful;
    }

    void Connection::CleanupResults()
    {
	    while(mysql_next_result(&m_dMYSQL) == 0)
	    {
		    MYSQL_RES* pRes = mysql_store_result(&m_dMYSQL);
		    if(pRes)
		    {
			    mysql_free_result(pRes);
		    }
	    }
    }

    bool Connection::BeginTransaction()
    {
	    CleanupResults();
	    return SetAutoCommit(false);
    }

    bool Connection::CommitTransaction()
    {
	    CleanupResults();
	    bool bResult = mysql_commit(&m_dMYSQL) == 0;
	    SetAutoCommit(true);
	    return bResult;
    }

    bool Connection::RollbackTransaction()
    {
	    CleanupResults();
	    bool bResult = mysql_rollback(&m_dMYSQL) == 0;
	    SetAutoCommit(true);
	    return bResult;
    }

    bool Connection::SetAutoCommit( bool bAutoCommit )
    {
	    return mysql_autocommit(&m_dMYSQL,bAutoCommit) == 0;
    }

    bool Connection::QueryStoreProcResult( Recordset& rs )
    {
	    if(!ExecuteSQL("SELECT @result"))
		    return false;
	    if(!rs.Initial(GetMYSQL()))
		    return false;
	    return true;
    }

    int Connection::ExecuteStoreProc( const std::string& strSQL,int nErrorReturn /*= 0xFF*/)
    {
        int n = 3;
        while((n --) > 0)
        {
            if(!ExecuteSQL(strSQL.c_str()))
	        {
                OutputError(strSQL);

                int _errno = mysql_errno(GetMYSQL());
                if(_errno == ER_LOCK_DEADLOCK)
                    continue;
	        }
            break;
        }
	    Recordset rs;
	    if(!QueryStoreProcResult(rs))
	    {
		    OutputError(strSQL);
		    return nErrorReturn;
	    }

	    if(!rs.Fetch())
		    return nErrorReturn;
	    return rs.GetInt32(0);
    }

    void Connection::OutputError( const std::string &sql, bool bReconnect/* = false*/ )
    {
        char msg [8192] = "";
        if ( sql.length() < 7000 )
        {
			snprintf (msg, sizeof (msg), "SQLError(%d): %s \r\nInformatin:%s\r\n", mysql_errno(*this), sql.c_str(), mysql_error(*this));
			NLog->error (msg);
        }
		else
		{
			string str(sql.c_str(), sql.c_str() + 7000);
			snprintf (msg, sizeof (msg), "SQLError(%d)   Informatin:%s : %s \r\n\r\n", mysql_errno(*this),mysql_error(*this), str.c_str());
			NLog->error (msg);

			NLog->error ("") ;
			//G_Log (gLoggerError,"数据库出错 长度出错 %d",sql.length());
		}

        if (bReconnect)
        {
            m_bNeedReconnect = true;
        }
    }

    int64_t Connection::GetLastInsertID()
    {
	    return mysql_insert_id(&m_dMYSQL);
    }

    bool Connection::CheckAndReconnect(int dwCheckInterval)
    {
        if(m_bNeedReconnect)
        {
            Cleanup();
            m_bNeedReconnect = false;
            return this->Startup(m_strHost,m_strUser,m_strPassword,m_strDB);
        }

	    int64_t dwCurrent = getTime64();
	    if(m_dwLastCheckTimeMS == 0 || (dwCurrent - m_dwLastCheckTimeMS) > dwCheckInterval)
	    {
		    m_dwLastCheckTimeMS = dwCurrent;
		    return mysql_ping(&m_dMYSQL) == 0;
	    }
	    return true;
    }

    std::string Connection::RealEscapeStringEx( const char* szBuf,unsigned long nLen /*= 0xFFFFFFFF*/ )
    {
	    if(nLen == 0xFFFFFFFF)
		    nLen = strlen(szBuf);

	    m_szEscapeBuf[0] = '\0';
	    unsigned long n = mysql_real_escape_string(&m_dMYSQL,m_szEscapeBuf,szBuf,nLen);
	    m_szEscapeBuf[n] = '\0';
    	
	    return std::string(m_szEscapeBuf);
    }

    Recordset::Recordset( MYSQL* pConn )
	    : m_pFields(NULL)
	    , m_nFieldCount(0)
	    , m_nRowCount(0)
	    , m_nCurrentRow(0)
	    , m_pRecordset(NULL)
    {
	    Initial(pConn);
    }

    Recordset::Recordset(MYSQL_RES* pRes)
        : m_pFields(NULL)
        , m_nFieldCount(0)
        , m_nRowCount(0)
        , m_nCurrentRow(0)
        , m_pRecordset(NULL)
    {
	    Initial(pRes);
    }

    Recordset::Recordset()
        : m_pFields(NULL)
        , m_nFieldCount(0)
        , m_nRowCount(0)
        , m_nCurrentRow(0)
        , m_pRecordset(NULL)
    {
    }

    Recordset::~Recordset()
    {
	    if(m_pRecordset)
	    {
		    mysql_free_result(m_pRecordset);
		    m_pRecordset = NULL;
	    }
    }

    bool Recordset::FetchAll()
    {
	    while(m_nRowCount > m_vtRows.size())
	    {
		    if(!Fetch())
			    return false;
	    }
	    return true;
    }

    bool Recordset::Fetch()
    {
	    if(!m_pRecordset)
		    return false;

	    MYSQL_ROW pRow = mysql_fetch_row(m_pRecordset);		// 检索一个结果集合的下一行
	    unsigned long* pLengths = mysql_fetch_lengths(m_pRecordset);	// 函数取得一行中每个字段的内容的长度
	    if(pRow == NULL || pLengths == NULL)
		    return false;
	    m_nCurrentRow = m_vtRows.size();
	    m_vtRows.push_back(pRow);
	    m_vtLengths.push_back(pLengths);
	    return true;
    }

    unsigned long Recordset::GetFieldByName( const char* szName )
    {
	    if(szName == NULL)
		    return 0xFFFFFFFF;

	    for(unsigned long i = 0; i < m_nFieldCount; ++i)
	    {
		    if(strcmp(m_pFields[i].name,szName) == 0)
			    return i;
	    }
	    return 0xFFFFFFFF;
    }

    unsigned long Recordset::GetFieldCount()
    {
	    return m_nFieldCount;
    }

	char * Recordset::GetFieldName(unsigned long nIndex )
	{
		if ( nIndex >= m_nFieldCount)
		{
			return NULL;
		}

		return m_pFields[nIndex].name;
	}

    unsigned long Recordset::GetCount()
    {
	    return m_nRowCount;
    }

    char* Recordset::Get( unsigned long field )
    {
	    if(m_nCurrentRow >= m_vtRows.size())
		    return "";
	    if(field >= m_nFieldCount)
		    return "";

	    char* pResult =  m_vtRows[m_nCurrentRow][field];
	    if(pResult != NULL)
		    return pResult;
	    return "";
    }

    char* Recordset::Get( const char* szFieldName )
    {
	    return Get(GetFieldByName(szFieldName));
    }

    int Recordset::GetInt32( unsigned long field )
    {
	    char* szVal = Get(field);
	    if(szVal == NULL)
		    return 0;
	    return atoi(szVal);
    }

    int64_t Recordset::GetInt64( unsigned long field )
    {
	    char* szVal = Get(field);
	    if(szVal == NULL)
		    return 0;
		int64_t id = 0;
		sscanf(szVal, "%lld", &id);
		return id;
    }

    Blob Recordset::GetBlob( unsigned long field )
    {
	    Blob objResult;
	    char* szVal = Get(field);
	    if(szVal != NULL)
	    {
		    objResult.buf = szVal;
		    objResult.len = m_vtLengths[m_nCurrentRow][field];
	    }
	    return objResult;
    }

    int Recordset::GetInt32ByName (const char *filed)
    {
        if (! filed) return 0xFFFFFFFF;
        
        int idx = GetFieldByName (filed);
        if (idx == 0xFFFFFFFF)
		{
			NLog->error("不确定的列  %s",filed);
			return 0xFFFFFFFF;
		}

        return GetInt32 (idx);
    }

    int64_t Recordset::GetInt64ByName (const char *filed)
    {
        if (! filed) return ErrorSQLRS;

        int idx = GetFieldByName (filed);
        if (idx == ErrorSQLRS)
		{
			NLog->error("不确定的列  %s",filed);
            return ErrorSQLRS;
		}
        return GetInt64 (idx);
    }

    char *Recordset::GetStrByName (const char *filed)
    {
        if (! filed) return 0;
        
        int idx = GetFieldByName (filed);
        if (idx == ErrorSQLRS)
		{
			NLog->error("不确定的列  %s",filed);
            return 0;
		}
        return GetStr (idx);
    }

    void Recordset::GetBlobByName (const char *filed, Blob &ret)
    {
        if (! filed) return;

        int idx = GetFieldByName (filed);
        if (idx == 0xFFFFFFFF)
            return;

        char *str = Get (idx);
        if (str)
        {
            ret.buf = str;
            ret.len = m_vtLengths[m_nCurrentRow][idx];
        }
    }

    bool Recordset::Goto( unsigned long nCurrent )
    {
	    if(m_nCurrentRow >= m_vtRows.size())
		    return false;
	    m_nCurrentRow = nCurrent;
	    return true;
    }

    char* Recordset::GetStr( unsigned long field )
    {
	    return Get(field);
    }

    bool Recordset::UpdateBlobField( Connection& connection,
                                    const char* szTable,
                                    const char* szKeyField,
                                    const char* szValueField,
                                    int64_t nKeyValue,
                                    const void* pBuf,
                                    unsigned long cLen,
                                    const char* szAddUpdate)
    {
        char szBuf[100 * 1024] = {0};

        unsigned long n = mysql_hex_string(szBuf,(const char*)pBuf,cLen);
        
	    return connection.ExecuteSQLNonResult("UPDATE %s SET %s=0x%s %s WHERE %s=%I64d",
            szTable,szValueField,szBuf,szAddUpdate,szKeyField,nKeyValue);
    }

    bool Recordset::Initial( MYSQL* pConn )
    {
	    return Initial(mysql_store_result(pConn));
    }

    bool Recordset::Initial( MYSQL_RES* pRes )
    {
	    if(pRes == m_pRecordset)
		    return false;
	    if(m_pRecordset)
	    {
		    mysql_free_result(m_pRecordset);
		    m_pRecordset = NULL;
	    }
	    m_vtRows.clear();
	    m_vtLengths.clear();
	    m_pFields = NULL;
	    m_nFieldCount = 0;
	    m_nRowCount = 0;
	    m_nCurrentRow = 0;
	    m_pRecordset = pRes;

	    if(m_pRecordset)
	    {
		    m_pFields = mysql_fetch_fields(m_pRecordset);
		    m_nFieldCount = mysql_num_fields(m_pRecordset);
		    m_nRowCount = mysql_num_rows(m_pRecordset);
	    }
	    return true;
    }

    bool Recordset::IsOpen()
    {
	    return m_nRowCount > 0;
    }

	bool Recordset::IsEmpty()
	{
		return m_nRowCount == 0;
	}

	bool	RecordsetMgr::HaveNextResult(MYSQL *mmysql)
	{
		MYSQL_RES *result;
		if ( step != 0 )
		{
			int  status = mysql_next_result(mmysql);
			if ( status != 0 )
			{
				return false;
			}
			result = mysql_store_result(mmysql);
		}
		else
		{

			result = mysql_store_result(mmysql);
			step = 1;
		}
		
// 		int	status = 0;
// 		 do
// 		 {
// 		   if (result)
// 		   {
// 			   
// 		   }
// 		   else          /* no result set or error */
// 		   {
// 			   
// 		   }
// 		   /* more results? -1 = no, >0 = error, 0 = yes (keep looping) */
// 		   if ((status = mysql_next_result(mmysql)) > 0)
// 				printf("Could not execute statement\n");
// 		 }while(status == 0);
		 if ( result == NULL )
		 {
			 // 出错了   这里的逻辑
			 return false;
		 }
		 m_Record.Initial(result);
		 m_Record.Fetch();
		 return true;
	}


	
}
