#pragma once
#pragma execution_character_set ("utf_8")

#include <stdint.h>
#include <string.h>

#define MSGMAXSIZE 512


#define MSGCANCUTMAXSIZE (MSGMAXSIZE*10)
#define MSG_NEED_COMPRESS (1024 * 256)

#define const_iSendbufSize	(MSGMAXSIZE*10)
#define const_iRecvBufSizeForIOC (MSGMAXSIZE*100)

struct Msg;
#define MSG_HEADER_LEN (sizeof (Msg))

#pragma pack(push,1) 

#define CUT_MSG_CHILDBEGIN	-1
#define CUT_MSG_CHILD		-2

#define CALC_FULL_MSG_LEN                       \
	(sizeof (*this))

static const unsigned int DATA_STREAM_SIZE = 0xffff;
/**
* \brief ��Ϣ��־
*        ѹ�����ԣ�����Ϣͷ��������ֻ����Ϣʵ������ѹ�������ǣ�
*        Msg + ѹ�������Ϣʵ��
*
*        ���ܲ��ԣ�����Ϣ�岻������ֻ����Ϣͷ���ܣ����ǣ�
*        ���ܹ���Msg + ��Ϣʵ��
*/
enum  eMsgFlag
{
	eMsgFlag_Non			= 0x00000001,
	eMsgFlag_Intergrated	= 0x00000002,
	eMsgFlag_Father			= 0x00000004,
	eMsgFlag_Child			= 0x00000008,
};

/**
 * \brief in fact, this is msg header.
 *        18 bytes in size.
 */
struct Msg
{
	Msg()
	: length(0)
	, stID(-1)
	, dwType(0)
	, flag(eMsgFlag_Non)
	, origLen(0)
	{}

	uint32_t length;
	int  flag;
	int16_t  stID;
	uint32_t dwType;
	uint32_t origLen;

	inline uint32_t GetType()             { return dwType; }
	inline uint32_t GetLength()           { return length; }
	inline void   SetLength(uint32_t len) { length = len; }
	inline int32_t  GetFlag()             { return flag; }
	inline void   SetFlag(int f)        { flag |= f; }
	inline uint32_t GetOrigLen()          { return origLen; }
	inline void   SetOrigLen(uint32_t len) { origLen = len; }
};


struct Msg_WorkPacket_Msg : public Msg
{
	Msg_WorkPacket_Msg()
	{
		memset(data, 0, sizeof(data));
		dataDup = NULL;
		length = CALC_FULL_MSG_LEN;
		dwType = 1001;
	}
	~Msg_WorkPacket_Msg()
	{
		if (dataDup)
		{
			delete[]dataDup;
			dataDup = NULL;
		}
	}

	uint8_t	data[DATA_STREAM_SIZE];
	uint8_t*	dataDup;					// ���͵����ݳ���  ����ʱ��ʱ�洢  ����һ�����Կռ�
	Msg*	Write(WorkPacket &pack)
	{
		length = sizeof(Msg) + pack.GetNeedLen();
		if (!IsDataLenEnough(pack))
		{
			dataDup = new uint8_t[length];
			memmove(dataDup, this, sizeof(Msg));			// �ȰѰ�ͷ������ȥ

			memmove(dataDup + sizeof(Msg), pack.m_value, sizeof(pack.m_value));
			memmove(dataDup + sizeof(Msg) + sizeof(pack.m_value), pack.contents(), pack.size());
			return (Msg*)dataDup;
		}
		else
		{
			memmove(data, pack.m_value, sizeof(pack.m_value));
			if (pack.size() != 0)
			{
				memmove(data + sizeof(pack.m_value), pack.contents(), pack.size());
			}
			return NULL;
		}
	}

	bool	IsDataLenEnough(WorkPacket &pack)
	{
		return pack.GetNeedLen() <= DATA_STREAM_SIZE;
	}

	uint32_t	GetRealLength()
	{
		return GetLength() - sizeof(Msg);
	}
};

enum
{
	/**
	* Server <----> Client
	*
	*/
	GAMEWORLD_MSG_BEGIN = 1000,
	GAMEWORLD_MSG_END = 1500,

	PLATE_MSG_BEGIN = 2000,
	PLATE_MSG_END = 2100,

	DB_MSG_BEGIN = 4000,
	DB_MSG_END = 4200,

	WS_MSG_BEGIN = 4300,
	WS_MSG_END = 4600,

	CLIENTMSG_BEGIN = 5000,
	CLIENTMSG_END = 8000,

	DB_REDIS_BEGIN = 9000,
	DB_REDIS_END = 10000,

};

enum
{
	//���ݿ����
	DBMSG_DATA = DB_MSG_BEGIN,
	DBMSG_MAX,
};

enum
{
	MSG_DB_REDIS_START = DB_REDIS_BEGIN,
	MSG_REDIS_2_DB_ALLPLAYERINFO,
	MSG_DB_2_REDIS_ALLPLAYERINFO,
	MSG_DB_2_REDIS_ALLLOADFINISH,

	MSG_REDIS_2_DB_SAVEPLAYERINFO,

	MSG_WS_REDIS_LOADPLAYERINFO,
	MSG_WS_REDIS_SAVEPLAYERINFO,
	MSG_REDIS_WS_LOADPLAYERINFO,

	MSG_REDIS_DB_LOADONEPLAYERINFO,
	MSG_DB_REDIS_LOADONEPLAYERINFO,

	MSG_WS_REDIS_QUIT,
	MSG_REDIS_DB_QUIT,
	MSG_DB_REDIS_QUIT,


};


enum
{
	eDbThreadType_Common = 1,					// db Common ����
	eDbThreadType_Log,							// ��־�߳�
	//eDbThreadType_PlayerLog,					// �����־
	eDbThreadType_OpDb,							//�������ݿ�
	eDbThreadType_Max,
};

#pragma pack(push,1) 
struct DBMsg :public Msg
{
	//��ID�����һ���ж�Ψһ�Ķ���
	//CDBInterface��DispatchMsgת����Ϣʱ�ж�
	int32_t ustLifeCode;
	short shGameWorldId;
	uint8_t dbthreadType;

	DBMsg()
	{
		dbthreadType = eDbThreadType_Common;
		ustLifeCode = 0;
		shGameWorldId = 0;
	}
};

struct DBMsgSaveData :public DBMsg
{
	DBMsgSaveData()
	{
		length = CALC_FULL_MSG_LEN;
		dwType = DBMSG_DATA;
	}
	uint8_t	data[DATA_STREAM_SIZE];
	uint8_t*	dataDup;
	Msg*	Write(WorkPacket &pack)
	{
		length = sizeof(DBMsg) + pack.GetNeedLen();

		if (!IsDataLenEnough(pack))
		{
			dataDup = new uint8_t[length];
			memmove(dataDup, this, sizeof(DBMsg));			// �ȰѰ�ͷ������ȥ

			memmove(dataDup + sizeof(DBMsg), pack.m_value, sizeof(pack.m_value));
			memmove(dataDup + sizeof(DBMsg) + sizeof(pack.m_value), pack.contents(), pack.size());
			return (Msg*)dataDup;
		}
		else
		{
			memmove(data, pack.m_value, sizeof(pack.m_value));
			if (pack.size() != 0)
			{
				memmove(data + sizeof(pack.m_value), pack.contents(), pack.size());
			}

			return NULL;
		}
		//length = sizeof(DBMsg) + pack.size() + sizeof(pack.m_value);
	}

	bool	IsDataLenEnough(WorkPacket &pack)
	{
		return pack.GetNeedLen() <= DATA_STREAM_SIZE;
	}

	int32_t	GetRealLength()
	{
		return GetLength() - sizeof(DBMsg);
	}
};
