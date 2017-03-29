#pragma once
#pragma execution_character_set ("utf_8")

#include "hrNetwork.h"

class CSocketCtrl_base
{
public:
	typedef deque<Msg*>	 MSGDEQUE;
public:
	enum  class eSockeType:int
	{
		server = 1,
		cli,
	};
private:
	void			SetDefaultOpt();
public:
	CSocketCtrl_base();
	virtual ~CSocketCtrl_base();
	eSockeType		getSockType(){ return m_sockType;  }
	void			setSockType(eSockeType _type){ m_sockType = _type; }

	void			reset();
	bool			ShutDown();
	bool			hasAccept();
	void			haveRecvOrSend();

	bool			Create (unsigned short uPort, const char* addr = NULL);
	bool			Create();

	bool			Connect( const char* lpszHostAddress, unsigned short nHostPort,int dwTimeout =0);
	void			Close();

	bool			Listen( int nBackLog );

	bool			GetHostName( char szHostName[], int nNameLength );
	bool			GetPeerName( in_addr &addr );
	bool			isClosed( void );

    SOCKET&			GetSockfd(){ return m_sockfd; }

	bool			Release();

    sockaddr_in    *GetSockInAddr ();
    string          GetRemoteIp ();
    uint16_t        GetRemotePort ();
	
	virtual void	run();
	virtual void	accept_node();
	virtual CSocketCtrl_base*	getNewAcceptNode();
	
	bool			tryRecv();
	virtual void	processMsg();
	bool			trySend();
	
	void			pushRecvMsg();
	void			sendMsg(Msg *pMsg);
	virtual	void	connectedDo();
	
	void			pushRecvTempMsg(Msg *pMsg);
	
	virtual void	sendPack(WorkPacket &pack);
public:
	SOCKET			m_sockfd;
    sockaddr_in     m_addr;
	eSockeType		m_sockType;
	bool			m_isInListen;
	string			m_ip;
	uint16_t		m_port;
	
	bool			m_isNeedReConnectWhenLost;
	bool			m_isDele;
	bool			m_isConnect;

	stRecvBuff		m_recvMsg;
	stSendBuff		m_sendBuff;

	MSGDEQUE		m_recvDeque;
	MSGDEQUE		m_sendDeque;
	MSGDEQUE		m_recvTempDeque;
};