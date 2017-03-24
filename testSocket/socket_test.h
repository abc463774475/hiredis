#pragma once
#pragma execution_character_set ("utf_8")

#include "Socket.h"

class Socket_test : public CSocketCtrl_base
{
public:
	Socket_test() ;
	~Socket_test() ;

	virtual void	connectedDo();
	virtual void	processMsg();
	virtual void	run();

	map<int, Msg*>	m_totalMsg;
};

