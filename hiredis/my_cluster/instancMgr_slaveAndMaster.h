#pragma once
#pragma execution_character_set ("utf_8")

#include "instancMgr_base.h"

class instancMgr_slaveAndMaster : public instanceMgr_base
{
public:
	struct stClusterInfo
	{
		string	name;
		redisInstance_base* rIt;
		stClusterInfo()
		{
			rIt = NULL;
		}
	};
public:
	typedef map<string, stClusterInfo> DATAMAP;
	
public:
	instancMgr_slaveAndMaster();
	virtual ~instancMgr_slaveAndMaster();

public:
	DATAMAP			m_server_data;
public:
	bool				addData(redisInstance_base *_base, const string &_name);
	virtual void		runCommand();
	virtual void		run();

	stClusterInfo*	getClusterInfo(const string &_name);
	
	void		failover(const string &_name);
};


extern instancMgr_slaveAndMaster* gInstanceMgr_slaveAndMaster;
int failover_timer(struct aeEventLoop *eventLoop, long long id, void *clientData);
