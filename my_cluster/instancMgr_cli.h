#pragma once
#pragma execution_character_set ("utf_8")
#include "instancMgr_base.h"
#include "instance_cli.h"
#include <map>
#include "async.h"

class instancMgr_cli : public instanceMgr_base
{
public:
	typedef map<string, redisInstance_cli*>	CLIBASEMAP;
public:
	instancMgr_cli ();
	~instancMgr_cli ();

public:
	redisInstance_cli*		m_cluster;				// the base  can do cluster slots and other things
	string					m_clusterName;			//
	CLIBASEMAP				m_clientBase;
	CSLOTSVEC				m_slots;

public:
	void				initial(const string &str);
	virtual void		runCommand();
	virtual void		run();
	virtual	void		addCommand(const string &str);

	void				slotsLoadFinish();
	void				resetClient();
	bool				isCanWork();
	
	redisInstance_cli*	getRightCliByKey(const string &strKey);

	static	int			instan_timer(struct aeEventLoop *eventLoop, long long id, void *clientData);
};

extern instancMgr_cli* gInstanceMgr_cli;
