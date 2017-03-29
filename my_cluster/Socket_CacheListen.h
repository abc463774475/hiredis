#pragma once
#pragma execution_character_set ("utf_8")

#include "Socket.h"
extern "C"
{
#include <hiredis.h>
}

class Socket_CacheListen : public CSocketCtrl_base
{
public:
	Socket_CacheListen();
	~Socket_CacheListen();

	virtual CSocketCtrl_base* getNewAcceptNode();

};

class Socket_Cache : public CSocketCtrl_base
{
public:
	Socket_Cache();
	~Socket_Cache();

	virtual void processMsg();

};

extern Socket_Cache* gCache;