#pragma once
#pragma execution_character_set ("utf_8")

#include "redisInstance_base.h"
#include <vector>

struct socketInfo
{
	string	ip;
	int port;
	socketInfo()
	{
		port = 0;
	}
};
struct stClusterSlotsInfo
{
	int minSlots;
	int maxSlots;
	socketInfo master;
	vector<socketInfo> slave;

	stClusterSlotsInfo()
	{
		minSlots = -1;
		maxSlots = -1;
	}
};

typedef vector<stClusterSlotsInfo>	CSLOTSVEC;

class  redisInstance_cli : public redisInstance_base
{
public:
	redisInstance_cli();
	~redisInstance_cli();

	virtual void		connectCallback(int status);
	virtual void		disconnectCallback(int status);
	virtual void		run();
	virtual void		reset();
	virtual void		initial();
public:
	string				m_newIp;
	int					m_newPort;
	
public:
	BACKMSG_H_DINGYI(moved);
	BACKMSG_H_DINGYI(slots);

public:
};