#include "instancMgr_cli.h"
#include "my_head.h"
#include <sstream>
instancMgr_cli* gInstanceMgr_cli = NULL;
instancMgr_cli::instancMgr_cli()
{
}

instancMgr_cli::~instancMgr_cli()
{
}

void instancMgr_cli::initial(const string &str)
{
	m_cluster = new redisInstance_cli;
	m_clusterName = str;

	redisInstance_cli &_base = *m_cluster;
	_base.m_ip = "127.0.0.1";
	_base.m_port = 6379;
	_base.initial();
	_base.m_type = eReisInstanceType::cliBase;
	_base.connectInstanc();
}

void instancMgr_cli::addCommand(const string &str)
{
	instanceMgr_base::addCommand(str);
}

static string getValueKey(const string &str)
{
	// twice   such as set h1 
	stringstream ss(str);
	string strKey;
	ss >> strKey >> strKey;

	return strKey;
}

void instancMgr_cli::runCommand()
{
	lock_guard<mutex> guard(m_lockCommand);
	for (auto itCommand = m_command.begin(); itCommand != m_command.end(); ++itCommand)
	{

		getRightCliByKey(getValueKey(*itCommand))->asynCommand(*itCommand);
	}
	m_command.clear();
}

void instancMgr_cli::run()
{
	runCommand();

	m_cluster->run();
	for (auto it = m_clientBase.begin(); it != m_clientBase.end();)
	{
		auto &_base = it->second;
		_base->run();
		++it;
	}
}

void instancMgr_cli::resetClient()
{
	m_clientBase.clear();
}

void instancMgr_cli::slotsLoadFinish()
{
	//resetClient();
	for ( auto it: m_slots)
	{
		char sz[1024];
		sprintf(sz, "%s:%d", it.master.ip.c_str(),it.master.port);
		auto itTemp = m_clientBase.find(sz);
		if ( itTemp != m_clientBase.end())
		{
			continue;
		}

		redisInstance_cli *cli = new redisInstance_cli;
		cli->initial();
		cli->m_ip = it.master.ip;
		cli->m_port = it.master.port;
		cli->connectInstanc();
		m_clientBase.insert(CLIBASEMAP::value_type(string(sz), cli));
	}
}

bool instancMgr_cli::isCanWork()
{
	if ( m_cluster->m_status != eInstance_commonStatus::haveConnected)
	{
		return false;
	}
	for ( auto it : m_clientBase)
	{
		if ( it.second->m_status != eInstance_commonStatus::haveConnected)
		{
			return false;
		}
	}
	return true;
}

redisInstance_cli* instancMgr_cli::getRightCliByKey(const string &strKey)
{
	int slotId = getKeysSlots(strKey);

	char sz[256];
	for (auto it : m_slots)
	{
		if (slotId >= it.minSlots && slotId <= it.maxSlots)
		{
			sprintf(sz, "%s:%d", it.master.ip.c_str(), it.master.port);
			break;
		}
	}
	auto it = m_clientBase.find(sz);
	if ( it != m_clientBase.end())
	{
		return it->second;
	}

	return NULL;
}

int instancMgr_cli::instan_timer(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
	gInstanceMgr_cli->run();
	return 1;
}
