#pragma once
#pragma execution_character_set ("utf_8")

#include "Socket.h"
extern "C"
{
#include <hiredis.h>
}

class Socket_Cache : public CSocketCtrl_base
{
public:
	Socket_Cache();
	~Socket_Cache();

	void init();
	virtual void processMsg();

public:
	string		m_redisIp;
	int			m_redisPort;
	
	redisContext* m_context;

	void		addInfo(int accountId, const string &str, bool isServerFirstLoad = false);
	void		loadInfo(int accountId, const string &str, bool isServerFirstLoad = false);

	void		saveAllToDb();
};

extern Socket_Cache* gCache;