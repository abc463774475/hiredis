#pragma once
#pragma execution_character_set ("utf_8")

#include "hrNetwork.h"

// base socketctrl
class CSocketCtrl_base
{
public:
	enum  class eSockeType:int
	{
		server = 1,
		cli,
	};
private:
	void			SetDefaultOpt();
public:
	CSocketCtrl_base() : m_sockfd(-1)	{
		Close();
	}
	virtual ~CSocketCtrl_base()	{}
	eSockeType		getSockType(){ return m_sockType;  }
	void			setSockType(eSockeType _type){ m_sockType = _type; }

	bool			ShutDown();
	bool			CanWrite();
	bool			HasData();

	bool			Create (unsigned short uPort, const char* addr = NULL);
	bool			Create();

	bool			Connect( const char* lpszHostAddress, unsigned short nHostPort,int dwTimeout =0);
	void			Close();

	bool			Listen( int nBackLog );

	bool			RecvMsg( char *sBuf );
	int				SendMsg( char *sBuf,unsigned short stSize );

	bool			GetHostName( char szHostName[], int nNameLength );
	bool			GetPeerName( in_addr &addr );
	bool			IsClosed( void );

    SOCKET&			GetSockfd(){ return m_sockfd; }

	bool			Release();

    sockaddr_in    *GetSockInAddr ();
    string          GetRemoteIp ();
    uint16_t        GetRemotePort ();
	
	void			run();
	void			accept_node();
	
	void			tryRecv();
	
protected:
	SOCKET			m_sockfd;
    sockaddr_in     m_addr;
	eSockeType		m_sockType;
	bool			m_isListen;
	string			m_ip;
	uint16_t		m_port;
};