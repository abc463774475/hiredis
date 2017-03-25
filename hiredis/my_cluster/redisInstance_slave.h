#pragma once
#pragma execution_character_set ("utf_8")

#include "redisInstance_base.h"

class  redisInstance_slaveOrMaster : public redisInstance_base
{
public:
	typedef deque<redisInstance_slaveOrMaster*> SLAVEDEQUE;
public:
	redisInstance_slaveOrMaster();
	~redisInstance_slaveOrMaster();

	virtual void		disconnectCallback(int status);
	virtual void		run();
	virtual void		reset();
	virtual	void		initial();

public:
	SLAVEDEQUE			m_slave;							// slave not want use recursive function so we must think another way
	//SLAVEDEQUE			m_needChangeSlave;					// needChangeSlave

	/*
	maybe only one status cannot enough		 especially in slavePromot
	one status contain so much status  and the control is very complex
	*/
	
	eInstance_slavePromoteStatus	m_slaveStatus;

	int64_t				m_checkInfoTimes;					// need  time over  opt
	bool				m_isCheckInfoIn;					// isInCheckInfo
	bool				m_isInTTLMode;						// is in ttl mode

	redisInstance_base* m_master;

public:
	void				checkInfo();
	void				checkInstanceInsIn(const string &strIp, int port);

	redisInstance_slaveOrMaster*	getPromotedSlave(bool isFailover = false);
	
	BACKMSG_H_DINGYI(info);
	BACKMSG_H_DINGYI(slaveof);
};

