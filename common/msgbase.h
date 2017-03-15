#pragma once
#pragma execution_character_set ("utf_8")

#include <stdint.h>

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


/**
* \brief ��Ϣ��־
*        ѹ�����ԣ�����Ϣͷ����������ֻ����Ϣʵ������ѹ�������ǣ�
*        Msg + ѹ�������Ϣʵ��
*
*        ���ܲ��ԣ�����Ϣ�岻��������ֻ����Ϣͷ���ܣ����ǣ�
*        ���ܹ���Msg + ��Ϣʵ��
*/
enum
{
	MSG_FLAG_NONE = 0x00000000,
	MSG_FLAG_COMPRESS = 0x00000001, /* ѹ�� */
	MSG_FLAG_ENCRYPT = 0x00000002, /* ���� */

	MSG_FLAG_ALL = 0xffffffff,
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
	, flag(MSG_FLAG_NONE)
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