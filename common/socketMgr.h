#pragma once
#pragma execution_character_set ("utf_8")
#include "Socket.h"
#include <map>

class  SocketMgr
{
public:
	typedef vector <CSocketCtrl_base*> SOCKVEC;
public:
	 SocketMgr();
	~SocketMgr();

private:
	SOCKVEC				m_node;
public:
	void				run();
	void				regist(CSocketCtrl_base *_base);

};

extern SocketMgr *gSockMgr;