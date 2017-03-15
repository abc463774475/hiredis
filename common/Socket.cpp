#include "Socket.h"
#include <stdio.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <fcntl.h>
#include <errno.h>
#include "socketMgr.h"

CSocketCtrl_base::CSocketCtrl_base() : m_sockfd(-1)
{
	Close();
	m_isDele = false;
	m_isNeedReConnectWhenLost = false;
}

CSocketCtrl_base::~CSocketCtrl_base()
{
	Close();
}

bool CSocketCtrl_base::GetHostName( char szHostName[], int nNameLength )
{
	if( gethostname( szHostName, nNameLength ) != -1 )
		return true;
	return false;
}


bool CSocketCtrl_base::hasAccept()
{
	int e;
	fd_set set;
	timeval tout;
	tout.tv_sec = 0;
	tout.tv_usec = 0;
	
	FD_ZERO(&set);
	FD_SET(m_sockfd,&set);
	e=::select(m_sockfd+1,&set,NULL,NULL,&tout);
	if(e==-1) 
	{
		return false;
	}
	if(e>0) return true	;
	return false;
}

void CSocketCtrl_base::haveRecvOrSend()
{
	int e;
	fd_set rf,wf;
	timeval tout;
	tout.tv_sec = 0;
	tout.tv_usec = 0;

	
	FD_ZERO(&rf);
	FD_ZERO(&wf);
	FD_SET(m_sockfd, &rf);
	FD_SET(m_sockfd, &wf);
	e = ::select(m_sockfd+1, &rf, &wf, NULL, &tout);
	if (e == -1)
	{
		NLog->error("select Erro %s", strerror(errno));
		goto errEnd;
	}
	else if (e == 0)
	{
		return ;
	}
	else
	{
		if ( FD_ISSET (m_sockfd, &rf))
		{
			if ( !tryRecv())
			{
				goto errEnd;
			}
		}
		if ( FD_ISSET (m_sockfd, &wf))
		{
			if ( !trySend())
			{
				goto errEnd;
			}
		}
	}
	return;
errEnd:
	Close();
}

bool CSocketCtrl_base::Create()
{
	Close();
	m_sockfd = ::socket(PF_INET, SOCK_STREAM, 0);
	if (m_sockfd == -1)
	{
		NLog->error("create socket erro  %s",strerror(errno));
		return false;
	}
	SetDefaultOpt();
	return true;
}

bool CSocketCtrl_base::Create(unsigned short uPort, const char* addr /* = NULL */)
{
	Close();
	m_sockfd = ::socket(PF_INET, SOCK_STREAM, 0);
	if (m_sockfd == -1)
		return false;

	sockaddr_in	SockAddr;
	memset(&SockAddr, 0, sizeof(SockAddr));
	SockAddr.sin_family = AF_INET;

	SetDefaultOpt();
	if (addr)
	{
		SockAddr.sin_addr.s_addr = inet_addr(addr);
	}
	else
	{
		SockAddr.sin_addr.s_addr = INADDR_ANY;
	}

	SockAddr.sin_port = ::htons(uPort);
	if (!::bind(m_sockfd, (sockaddr*)&SockAddr, sizeof(SockAddr)))
	{
		// 在绑定之前设置    客户端在连接之前设置
		return true;
	}

	Close();
	NLog->error("create erro   %s",strerror(errno));
	return false;
}

void CSocketCtrl_base::Close()
{
	if ( m_sockfd != -1 )
	{
		::shutdown(m_sockfd, SHUT_WR) ;
		
		int iret = ::close( m_sockfd );

		if ( iret == -1)
		{
			NLog->error("关闭socket 失败  erro %d",errno) ;
		}
	}
    m_sockfd = -1;

	if ( m_sockfd != -1 )
	{
		//assert(FALSE);
		NLog->error ("SysErro 非常严重的同步bug");
	}
	
	reset();
}

void CSocketCtrl_base::reset()
{
	m_isInListen = false;
	m_recvMsg.Reset();
	m_sendBuff.Reset();

	for ( auto it : m_recvDeque)
	{
		delete it;
	}
	m_recvDeque.clear();

	for ( auto it : m_sendDeque)
	{
		delete it;
	}
	m_sendDeque.clear();

	m_isConnect = false;
}

bool CSocketCtrl_base::Connect( const char* lpszHostAddress, unsigned short nHostPort,int dwTimeout )
{
	if(m_sockfd==-1) 
	{
		Create();
		if( m_sockfd==-1 )
			return false;
	}
	
	int er1 = 0;

	sockaddr_in sockAddr;
	
	memset(&sockAddr,0,sizeof(sockAddr));
	const char* lpszAscii=lpszHostAddress;
	sockAddr.sin_family=AF_INET;
	sockAddr.sin_addr.s_addr=inet_addr(lpszAscii);
	if(sockAddr.sin_addr.s_addr==INADDR_NONE)
	{
		hostent * lphost;
		lphost = ::gethostbyname(lpszAscii);
		if(lphost!=NULL)
			sockAddr.sin_addr.s_addr = ((in_addr *)lphost->h_addr)->s_addr;
		else return false;
	}
	sockAddr.sin_port = htons((u_short)nHostPort);
	
	int r=::connect(m_sockfd,(sockaddr*)&sockAddr,sizeof(sockAddr));
	if(r!=-1) 
		return true;
	if ( errno != EINPROGRESS)
	{
		goto errEnd;
	}

	timeval	tv;
	tv.tv_sec = 0;
	tv.tv_usec = 0;
	fd_set wset;

	FD_ZERO(&wset);
	FD_SET(m_sockfd, &wset);

	if (select(m_sockfd + 1, NULL, &wset,NULL, &tv) < 0)
	{
		goto errEnd;
	}
	
	if (FD_ISSET(m_sockfd, &wset)) {
		socklen_t len = sizeof(er1);
		if (getsockopt(m_sockfd, SOL_SOCKET, SO_ERROR, &er1, &len) < 0 ) {
			goto errEnd;
		}
		return true;
	}

	return false;
errEnd:
	Close();
	return false;
}

bool CSocketCtrl_base::Listen( int nBackLog )
{
	if( m_sockfd == -1 ) 
		return false;
	if( 0 == listen( m_sockfd, nBackLog) ) 
		return true;
	return false;
}

void	CSocketCtrl_base::SetDefaultOpt()
{
	int flags;

	/* Set the socket blocking (if non_block is zero) or non-blocking.
	* Note that fcntl(2) for F_GETFL and F_SETFL can't be
	* interrupted by a signal. */
	if ((flags = fcntl(m_sockfd, F_GETFL)) == -1) {
		NLog->error("fcntl(F_GETFL): %s", strerror(errno));
	}

	if (1)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;

	if (fcntl(m_sockfd, F_SETFL, flags) == -1) {
		NLog->error( "fcntl(F_SETFL,O_NONBLOCK): %s", strerror(errno));
	}

	int bNoDelay = true;
	if (setsockopt(m_sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&bNoDelay, sizeof(int)) == -1){
		NLog->error("setsock tcpNodelay erro %s", strerror(errno));
	}

	int on = 1;
	if (setsockopt(m_sockfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on)) == -1){
		NLog->error("setsock reuseaddr erro %s", strerror(errno));
	}
	
}

bool CSocketCtrl_base::GetPeerName( in_addr &addr )
{
	sockaddr_in localaddr;
	socklen_t iSize;
	//m_sock
	iSize = sizeof( localaddr );
	if( 0 == getpeername( m_sockfd,(sockaddr*)&localaddr,&iSize ) )
	{
		addr = localaddr.sin_addr;
		return true;
	}
	return false;
}

bool CSocketCtrl_base::isClosed( void )
{
	return -1 == m_sockfd;
}

bool CSocketCtrl_base::Release()
{
	delete this;
	return true;
}

sockaddr_in *CSocketCtrl_base::GetSockInAddr ()
{
    return (&m_addr);
}

string CSocketCtrl_base::GetRemoteIp ()
{
	char *ip = (char *)inet_ntoa (m_addr.sin_addr);
	if (ip != NULL)
    {
		return string (ip);
    }
	else
    {
        return "";
    }
}

uint16_t CSocketCtrl_base::GetRemotePort()
{
    return ntohs (m_addr.sin_port);
}

void CSocketCtrl_base::run()
{
	if ( m_sockType == eSockeType::server)
	{
		if ( !m_isInListen)
		{
			if ( Create(m_port, m_ip.c_str()) &&
				Listen(5) )
			{
				m_isInListen = true;
				NLog->info("bind %s:%d ok",m_ip.c_str(), m_port);
			}
			else
			{
				NLog->error("bind socket Erro %s : %d", m_ip.c_str(), m_port);
				exit(1);
			}
		}
		if ( m_isInListen)
		{
			while (hasAccept())
			{
				// one client 
				accept_node();
			}
		}
	}
	else if (m_sockType == eSockeType::cli)
	{
		if ( isClosed())
		{
			if ( !m_isNeedReConnectWhenLost)
			{
				m_isDele = true;
			}
			else
			{
				if ( Connect(m_ip.c_str(),m_port))
				{
					connectedDo();
				}
			}
			return;
		}
		haveRecvOrSend();
		processMsg();
	}
}

void CSocketCtrl_base::accept_node()
{
	sockaddr_in addr;
	socklen_t len = sizeof (addr);
	int sockfd = accept(m_sockfd, (sockaddr *)&addr, &len);
	if (sockfd == -1)
	{
		NLog->error("accept Erro  %s", strerror(errno));
		return;
	}

	CSocketCtrl_base *pBase = new CSocketCtrl_base;
	pBase->setSockType(eSockeType::cli);
	pBase->GetSockfd() = sockfd;
	pBase->m_isConnect = true;
	
	pBase->SetDefaultOpt();
	gSockMgr->regist(pBase);
}

bool CSocketCtrl_base::tryRecv()
{
	if ( isClosed()){
		return false;
	}
	char sz[MAX_RECV_CACHESIZE];
	char *sBuf = sz;
	while (1){
		int iSize = recv(m_sockfd, sBuf, MAX_RECV_CACHESIZE, 0);
		if (iSize == 0){
			return false;
		}
		else if (iSize == -1){
			int e1 = errno;
			if (e1 != EAGAIN){
				// maybe the rst 
				NLog->error("cur cannot enter  %s", strerror(e1));
				return false;
			}
			return true;
		}
		else{
			uint32_t	stMsgSize;
			char		sMsgBuf[MSGMAXSIZE];
			int			iNeed;
			if (m_recvMsg.m_nCurLength < sizeof (uint32_t)){
				if (m_recvMsg.m_nCurLength + iSize < sizeof (uint32_t)){
					m_recvMsg.CopyData(sBuf, iSize);
					return true;
				}
				else if (m_recvMsg.m_nCurLength + iSize == sizeof (uint32_t)){
					iNeed = sizeof (uint32_t)-m_recvMsg.m_nCurLength;
					m_recvMsg.CopyData(sBuf, iNeed);
					return true;
				}
				else{
					iNeed = sizeof (uint32_t)-m_recvMsg.m_nCurLength;
					m_recvMsg.CopyData(sBuf, iNeed);
					iSize -= iNeed;
					sBuf += iNeed;
				}
			}

			stMsgSize = *(uint32_t*)m_recvMsg.m_data;

			// 超长数据很少用得到   超过 100M 的数据都丢弃  怕的是服务器之间的同步 需要那么大的数据
			if (stMsgSize >= MSGCANCUTMAXSIZE || stMsgSize < sizeof(Msg)){
				m_recvMsg.Reset();
				NLog->error("msghead erro %d", stMsgSize);
				return false;
			}

			if (m_recvMsg.m_nCurLength + iSize >= stMsgSize){
				// 完整包
				iNeed = stMsgSize - m_recvMsg.m_nCurLength;
				if (iNeed < 0){
					NLog->error("如果这里都出错的话，我就无语了 %d", iNeed);
					return false;
				}
				m_recvMsg.CopyData(sBuf, iNeed);
				m_recvMsg.m_nMsgLength = stMsgSize;
				Msg *pMsg = (Msg*)m_recvMsg.m_data;

				pushRecvMsg();
				m_recvMsg.Reset();
				// 数据都拷贝完成了
				if (iSize > iNeed){
					sBuf += iNeed;
					iSize -= iNeed;
					continue;
				}
				else{
					break;;
				}
			}
			else{
				// 不完整包
				m_recvMsg.CopyData(sBuf, iSize);
				break;
			}
		}
	}

	return true;
}

void CSocketCtrl_base::pushRecvMsg(){
	char* sBuf = new char[m_recvMsg.m_nMsgLength];
	memcpy(sBuf,m_recvMsg.m_data,m_recvMsg.m_nMsgLength);
	m_recvDeque.push_back((Msg*)sBuf);
}

bool CSocketCtrl_base::trySend(){
	if ( isClosed()){
		return false;
	}
	// check the send cache
	while (1){
		// push the msgDeuq onto buff
		while (1){
			if ( m_sendDeque.size () == 0){
				break;
			}
			Msg *pMsg = m_sendDeque.front();
			if ( pMsg->length + m_sendBuff.m_sendBuffTotalSize < const_iSendbufSize){
				
				memcpy(m_sendBuff.m_sendBuff + m_sendBuff.m_sendBuffTotalSize, 
					(char*)pMsg, pMsg->length);
				m_sendBuff.m_sendBuffTotalSize += pMsg->length;

				delete pMsg;
				m_sendDeque.pop_front();
			}
			else{
				break;
			}
		}
		// curNotHaveBuf to send 
		if (m_sendBuff.m_sendBuffTotalSize == 0){
			return true;
		}
		int iSize = send(m_sockfd, m_sendBuff.m_sendBuff, m_sendBuff.m_sendBuffTotalSize, 0);
		if ( iSize == 0){
			return false;
		}
		else if (iSize  < 0){
			int e1 = errno;
			if (e1 != EAGAIN){
				// maybe the rst 
				NLog->error("cur cannot enter  %s", strerror(e1));
				return false;
			}
			return true;
		}
		else{
			m_sendBuff.Copy(iSize);
			continue;
		}
	}

	return true;
}

void CSocketCtrl_base::sendMsg(Msg *pMsg)
{
	if ( isClosed())
	{
		return;
	}
	char *sBuf = new char[pMsg->length];
	memcpy(sBuf, (char*)pMsg, pMsg->length);
	
	m_sendDeque.push_back((Msg*)sBuf);
}

void CSocketCtrl_base::processMsg()
{
	for ( auto it : m_recvDeque)
	{
		Msg *pMsg = it;

		// proMsg
		delete pMsg;
	}
}

void CSocketCtrl_base::connectedDo()
{
	m_isConnect = true;
}