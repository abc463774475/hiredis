#include "instancMgr_slaveAndMaster.h"
#include "my_head.h"
#include "redisInstance_slave.h"

instancMgr_slaveAndMaster *gInstanceMgr_slaveAndMaster = NULL;
instancMgr_slaveAndMaster::instancMgr_slaveAndMaster()
{
}

instancMgr_slaveAndMaster::~instancMgr_slaveAndMaster()
{
}

bool instancMgr_slaveAndMaster::addData(redisInstance_base *_base,const string &_name)
{
	auto it = m_server_data.find(_name);
	if ( it != m_server_data.end())
	{
		return false;
	}
	
	stClusterInfo st;
	st.rIt = _base;
	st.name = _name;

	m_server_data.insert(DATAMAP::value_type(_name, st));
	return true;
}

void instancMgr_slaveAndMaster::runCommand()
{
	lock_guard<mutex> guard(m_lockCommand);
	for (auto itCommand = m_command.begin(); itCommand != m_command.end(); ++itCommand)
	{
		for (auto it:m_server_data)
		{
			it.second.rIt->asynCommand(*itCommand);
		}
	}
	m_command.clear();
}

void instancMgr_slaveAndMaster::run()
{
	runCommand();

	for (auto it = m_server_data.begin(); it != m_server_data.end();)
	{
		auto &_base = it->second.rIt;
		if (_base->m_status == eInstance_commonStatus::needDele)
		{
			NLog->info("erase ip %s port %d", _base->m_ip.c_str(), _base->m_port);
			delete _base;
			it = m_server_data.erase(it);
			continue;
		}
		_base->run();
		++it;
	}
}

instancMgr_slaveAndMaster::stClusterInfo* instancMgr_slaveAndMaster::getClusterInfo(const string &_name)
{
	auto it = m_server_data.find(_name);
	if (it == m_server_data.end())
	{
		return NULL;
	}
	return &it->second;
}

void instancMgr_slaveAndMaster::failover(const string &_name)
{
	NLog->info("start failover serverName %s",_name.c_str());
	redisInstance_slaveOrMaster *pBase = NULL;
	for (auto it:m_server_data)
	{
		if (it.second.name == _name)
		{
			pBase = dynamic_cast<redisInstance_slaveOrMaster*> (it.second.rIt);
			break;
		}
	}
	if ( !pBase)
	{
		NLog->error("failover not find name %s", _name.c_str());
		return;
	}
	
	pBase->getPromotedSlave(true);
}

// void instancMgr_slaveAndMaster::addCommand(const string &str)
// {
// 	lock_guard<mutex> guard(m_lockCommand);
// 	m_command.push_back(str);
// }
int failover_timer(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
	gInstanceMgr_slaveAndMaster->failover("my_1");
	return AE_NOMORE;
}