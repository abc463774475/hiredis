#ifndef _MYSQL_WRAPPER_H_
#define _MYSQL_WRAPPER_H_

extern "C"
{
#include "mysql5/include/mysql.h"
#include "mysql5/include/mysqld_error.h"
}
#include <string>
#include <memory>
#include <vector>
#include <iosfwd>
#include <map>
#include <iostream>
#include <stdint.h>
using namespace std;

/* catch error record set result. */
#define ErrorSQLRS (0xFFFFFFFF)

namespace MySQL
{
	class Recordset;
	class RecordsetMgr;
	/**
	 * \brief connection.
	 *
	 */
	class Connection
	{
	private:
		MYSQL m_dMYSQL;				//Mysql的基本信息。
		std::string m_strHost;		
		std::string m_strUser;
		std::string m_strPassword;
		std::string m_strDB;
		bool m_bConnected;
		bool m_bNeedReconnect;
		char m_szBuf[100*1000];
		char m_szEscapeBuf[100*1000];
		int64_t m_dwLastCheckTimeMS;

	public:
		Connection();
		virtual ~Connection();
	public:
		bool Startup(const std::string &strHost, const std::string &strUser,
			const std::string &strPassword,const std::string& strDatabaseName,int wPort = 3306);
		bool Startup(const std::string &strConnection);
		void Cleanup();
		void CleanupResults();
		void OutputError(const std::string& strSQL,bool bReconnect = false);
	public:
		bool IsOpened();
		bool CheckAndReconnect(int dwCheckInterval = 1000);
	public:
		bool BeginTransaction();
		bool CommitTransaction();
		bool RollbackTransaction();
	public:
		bool SetAutoCommit(bool bAutoCommit);
		bool SelectDatabase(const std::string& strDatabaseName);
	public:
		bool ExecuteSQLNonResult(const char* fmt,...);
		std::string RealEscapeStringEx(const char* szBuf,unsigned long nLen = 0xFFFFFFFF);
	public:
		bool ExecuteSQL(const std::string& strSQL);
		bool ExecuteSQL(const std::string& strSQL,Recordset& rs,bool bFetchFirst = true);
		bool ExecuteSQL(const std::vector<std::string>& vtSQLs,bool bTransaction = true);
		bool ExecuteSQL(const std::string& strSQL,RecordsetMgr&rs,bool bFetchFirst = true);
		int ExecuteStoreProc(const std::string& strSQL,int nErrorReturn = 0xFF);
		bool QueryStoreProcResult(Recordset& rs);
		int64_t GetLastInsertID();
	public:
		operator MYSQL*()
		{
			return &m_dMYSQL;
		}

		MYSQL* GetMYSQL()
		{
			return &m_dMYSQL;
		}
	};

	struct Blob
	{
		char* buf;
		unsigned long len;

		Blob(const void* _buf = NULL,unsigned long _len = 0)
		{
			buf = (char*)_buf; len = _len;
		}
	};

	/**
	 * \brief record sets.
	 *
	 */
	class Recordset
	{
	public:
		Recordset();
		Recordset(MYSQL* pConn);
		Recordset(MYSQL_RES* pRes);
		~Recordset();
	public:
		bool FetchAll();
		bool Fetch();
		bool Goto(unsigned long nCurrent);
	public:
		unsigned long GetFieldByName(const char* szName);
		unsigned long GetCount();
		unsigned long GetFieldCount();
		char *GetFieldName(unsigned long nIndex );
	public:
		bool Initial(MYSQL* pConn);
		bool Initial(MYSQL_RES* pRes);
	public:
		bool IsOpen();
		bool IsEmpty();
	public:
		char* Get(unsigned long field);
		char* Get(const char* szFieldName);

	public:
		int	GetInt32(unsigned long field);
		char* GetStr(unsigned long field);
		int64_t GetInt64(unsigned long field);
		Blob GetBlob(unsigned long field);

		/**
		 * get record value by name.
		 *
		 */
		int   GetInt32ByName (const char *filed);
		int64_t GetInt64ByName (const char *filed);
		char *GetStrByName (const char *filed);
		void GetBlobByName (const char *filed, Blob &ret);

	public:
		static bool UpdateBlobField(Connection& connection,
			const char* szTable,
			const char* szKeyField,
			const char* szValueField,
			int64_t nKeyValue,
			const void* pBuf,unsigned long cLen,
			const char* szAddUpdate = "");
	private:
		typedef std::vector<MYSQL_ROW> MYSQL_ROWS;
		typedef std::vector<unsigned long*> MYSQL_LENGTHS;

		MYSQL_ROWS m_vtRows;
		MYSQL_LENGTHS m_vtLengths;
		MYSQL_FIELD* m_pFields;
		MYSQL_RES* m_pRecordset;
		unsigned long m_nFieldCount;
		unsigned long m_nRowCount;
		unsigned long m_nCurrentRow;
	};

	class CleanupResults
	{
	public:
		CleanupResults(Connection* pConnection)
		{
			m_pConnection = pConnection;
		}
		~CleanupResults()
		{
			if(m_pConnection)
			{
				m_pConnection->CleanupResults();
			}
		}
	private:
		Connection* m_pConnection;
	};

	class	RecordsetMgr
	{
	private:
		Recordset	m_Record;
	public:
		RecordsetMgr()
		{
			step = 0 ;
		}
		~RecordsetMgr(){}
		bool	HaveNextResult(MYSQL *mmysql);	

		Recordset *GetRecord(){ return &m_Record;}

		int  step ;
	};

}

#endif // _MYSQL_WRAPPER_H_
