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
* \brief 消息标志
*        压缩策略：对消息头不做处理，只对消息实际数据压缩，既是：
*        Msg + 压缩后的消息实体
*
*        加密策略：对消息体不做处理，只对消息头加密，既是：
*        加密过的Msg + 消息实体
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