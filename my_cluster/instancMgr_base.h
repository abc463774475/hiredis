#pragma once
#pragma execution_character_set ("utf_8")
#include "redisInstance_base.h"
#include <map>
#include <mutex>
#include <deque>
using namespace std;

class instanceMgr_base
{
public:
	typedef deque<string>	DATA_COMMAND;
	DATA_COMMAND	m_command;
	mutex			m_lockCommand;

	virtual void		runCommand();
	virtual void		run();
	virtual void		addCommand(const string &str);

public:
	instanceMgr_base();
	virtual ~instanceMgr_base();
};

