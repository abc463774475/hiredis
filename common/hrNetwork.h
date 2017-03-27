#pragma once
#pragma execution_character_set ("utf_8")
#include <stdint.h>
#include <string>
#include <unistd.h>
#include <iostream>
#include <netinet/in.h>
#include <sys/select.h>
#include "WorkPacket.h"
#include "msgbase.h"
using namespace std;

#define MAXMSGERRORCOUNT					5     //收到的错误消息序列最大数（指合法消息）
#define DEFAULT_SSL_CONNECT_TIMEOUT			5000
#define MAX_SERVER_MSG_SIZE					10000 //服务器之间可以接收的消息的最大值
#define MAX_CLIENT_MSG_NUM					400	  //客户端最多保存的消息队列的长度

#define MAX_RECV_CACHESIZE					1024

typedef int SOCKET;

struct stRecvBuff
{
public:
	uint32_t	m_nMsgLength;						// 每条消息的需要的长度
	uint32_t	m_nCurLength;						// 当前长度
	char		m_data[MSGCANCUTMAXSIZE];			// 最大长度是这个

	stRecvBuff()
	{
		Reset();
	}
	void Reset()
	{
		m_nMsgLength = 0;
		m_nCurLength = 0;
		memset(m_data, 0, sizeof(m_data));
	}

	void CopyData(const char *pData, uint32_t length)
	{
		memmove(m_data + m_nCurLength, pData, length);
		m_nCurLength += length;
	}
};

struct	stSendBuff
{
	int8_t			m_sendBuff[const_iSendbufSize];
	uint16_t		m_sendBuffTotalSize;
	/*uint16_t		m_sendBuffCurSize;*/

	stSendBuff()
	{
		Reset();
	}
	void			Reset()
	{
		memset(m_sendBuff, 0, sizeof (m_sendBuff));
		m_sendBuffTotalSize = 0;
	}

	void			Copy(int _isize)
	{
		//memcpy(m_sendBuff, m_sendBuff + _isize, m_sendBuffTotalSize - _isize);
		memmove(m_sendBuff, m_sendBuff + _isize, m_sendBuffTotalSize - _isize);
		m_sendBuffTotalSize -= _isize;
	}
};

