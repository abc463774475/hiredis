#include "redisInstance_slave.h"
#include "my_head.h"
#include <sstream>
#include "instancMgr_slaveAndMaster.h"

redisInstance_slaveOrMaster::redisInstance_slaveOrMaster()
{
	m_type = eReisInstanceType::slave;
}

redisInstance_slaveOrMaster::~redisInstance_slaveOrMaster()
{
}

void redisInstance_slaveOrMaster::reset()
{
	redisInstance_base::reset();
	m_slaveStatus = eInstance_slavePromoteStatus::init;
	m_checkInfoTimes = 0;
	m_isCheckInfoIn = false;
	m_isInTTLMode = false;

	m_master = NULL;
	m_slave.clear();
}

void redisInstance_slaveOrMaster::initial()
{
	redisInstance_base::initial();
	addBackMsg("INFO", s_info);
	addBackMsg("SLAVE", s_slaveof);
}

void redisInstance_slaveOrMaster::disconnectCallback(int status)
{
	m_status = eInstance_commonStatus::discconect;
	/*
	if the type is slave   wo can dele directly
	if master  wo need vote slave  and promote slave to master   but if no slave   what can we do ?
	*/
	if (m_type == eReisInstanceType::slave)
	{
		m_status = eInstance_commonStatus::needDele;
	}
	else
	{
		/*
		this is a transcation  we need do the following things
		a	vote a slave
		b	finish slaveof no one
		c	the other slaves need slaveof newip newport
		we need control slave do what?  and the run must change
		*/
		getPromotedSlave();
	}


	if (status != REDIS_OK)
	{
		NLog->info("disConnect Ok socket %d  addr %s port %d id %lld",
			m_context->c.fd,
			m_ip.c_str(),
			m_port,
			(long long)m_id);
	}
	else
	{
		NLog->error("disConnect Ok socket %d  addr %s port %d id %lld",
			m_context->c.fd,
			m_ip.c_str(),
			m_port,
			(long long)m_id);
	}

	m_context = NULL;
}

void redisInstance_slaveOrMaster::run()
{
	if (m_status == eInstance_commonStatus::needDele)
	{
		return;
	}
	startDoMsg();

	/*
	one opt  bijection one timer so we must  create the related timer
	if no timer the  status function may be wathed so tired
	the following status or other situation may need timer  and out timer opt
	a info     such as 1 minute
	b slave promoting   slaveof no one
	c promoted  slave ip port
	d finish all slaves must change into the promoted slaves in timers
	e vote opt (the next opt may include)
	*/
	if (m_slaveStatus != eInstance_slavePromoteStatus::init &&
		m_slaveStatus != eInstance_slavePromoteStatus::needPromote &&
		m_slaveStatus != eInstance_slavePromoteStatus::promoting)
	{
		return;
	}

	if (m_slaveStatus == eInstance_slavePromoteStatus::needPromote)
	{
		asynCommand("SLAVEOF NO ONE");
		m_slaveStatus = eInstance_slavePromoteStatus::promoting;
		return;
	}

	checkInfo();
	run_timer();

	for (auto it = m_slave.begin(); it != m_slave.end();)
	{
		if ((*it)->m_status == eInstance_commonStatus::needDele)
		{
			NLog->info("delete  %s", (*it)->getUniqueLabel().c_str());
			delete *it;
			it = m_slave.erase(it);
			continue;
		}
		else
		{
			(*it)->run();
			++it;
		}
	}
}

void redisInstance_slaveOrMaster::checkInfo()
{
	int64_t	_curTimes = getTime64();
	if (!m_isCheckInfoIn)
	{
		if (_curTimes - m_checkInfoTimes >= 60000)
		{
			asynCommand("INFO");
			m_checkInfoTimes = _curTimes;
			m_isCheckInfoIn = true;
		}
	}
	else
	{
		if (_curTimes - m_checkInfoTimes >= 60000)
		{
			// time is over   so need actively close socket
			redisAsyncDisconnect(m_context);
			// info back  can not  take effect   because  the  remote server cannot work safely
			m_isInTTLMode = true;
		}
	}
}

void redisInstance_slaveOrMaster::checkInstanceInsIn(const string &strIp, int port)
{
	bool bRet = false;
	for (auto it : m_slave)
	{
		if ((*it).m_ip == strIp &&
			(*it).m_port == port)
		{
			bRet = true;
			break;
		}
	}
	if (!bRet)
	{
		auto *pBase = new redisInstance_slaveOrMaster;
		pBase->m_ip = strIp;
		pBase->m_port = port;
		pBase->m_type = eReisInstanceType::slave;
		pBase->connectInstanc();

		m_slave.push_back(pBase);
	}
}

redisInstance_slaveOrMaster* redisInstance_slaveOrMaster::getPromotedSlave(bool isFailover)
{
	redisInstance_slaveOrMaster *pRet = NULL;
	for (auto it : m_slave)
	{
		if (it->m_status == eInstance_commonStatus::haveConnected)
		{
			pRet = it;
			break;
		}
	}

	if (!pRet)
	{
		NLog->error("can not find the promotedSlave the ip is  %s port is %d", m_ip.c_str(), m_port);
		m_status = eInstance_commonStatus::mustExist;
		return pRet;
	}
	for (auto it : m_slave)
	{
		if (it != pRet)
		{
			pRet->m_slave.push_back(it);
		}
	}

	//
	m_slave.clear();

	NLog->info("promoted ip %s port %d", pRet->m_ip.c_str(), pRet->m_port);
	instancMgr_slaveAndMaster::stClusterInfo* pInfo = gInstanceMgr_slaveAndMaster->getClusterInfo("my_1");
	pInfo->rIt = pRet;
	pRet->m_slave.push_back(this);
	// directly delet maybe not good    later i'll change other status to monitor
	if (isFailover)
	{
		// m_status =
		// change mgr  base
	}
	else
	{
		m_status = eInstance_commonStatus::needDele;
	}

	pRet->m_slaveStatus = eInstance_slavePromoteStatus::needPromote;
	return NULL;
}

BACKMSG_CPP_DINGYI(redisInstance_slaveOrMaster, info)
{
	m_isCheckInfoIn = false;
	stringstream	_stream(strBack);
	while (1)
	{
		string str;
		_stream >> str;
		if (str.size() == 0)
		{
			break;
		}
		int ipos = str.find("run_id:");
		if (str.find("run_id:") != string::npos)
		{
			str.erase(0, str.find("run_id:") + 7);
			m_run_id = str;
			NLog->info("ip %s  port %d runid   %s", m_ip.c_str(), m_port, str.c_str());
			continue;
		}
		else if (
			str.find("ip=") != string::npos &&
			str.find(",port=") != string::npos)
		{
			string strIp(&str[0] + str.find("ip=") + 3,
				&str[0] + str.find(",port="));
			string strPort(&str[0] + str.find(",port=") + 6,
				&str[0] + str.find(",state"));

			int port = atoi(strPort.c_str());
			// check the slave is in
			checkInstanceInsIn(strIp, port);
		}
	}
}

BACKMSG_CPP_DINGYI(redisInstance_slaveOrMaster, slaveof)
{
	if (strBack != string("OK"))
	{
		/*
		what can we do ?
		*/
		NLog->error("this is a critical erro ");
		return;
	}
	if (m_slaveStatus == eInstance_slavePromoteStatus::promoting)
	{
		m_slaveStatus == eInstance_slavePromoteStatus::init;
		/*
		*/
		for (auto it : m_slave)
		{
			char sz[256];
			sprintf(sz, "SLAVEOF %s %d", m_ip.c_str(), m_port);
			it->asynCommand(sz);
			it->m_slaveStatus = eInstance_slavePromoteStatus::changeSalveof;
			it->m_master = this;

			NLog->info("cur ip %s port %d  need slaveof ip %s port %d",
				it->m_ip.c_str(), it->m_port, m_ip.c_str(), m_port);
		}
	}
	else
	{
		/*
		needAllSlave return ok
		if all return ok we need change needChangeSlave to slave deque   how to notify the master ?
		the slave must have pointer to master otherwise the master cannot know the slave if finish

		m_needChangeSlave  need change into slave
		*/
		NLog->info("slave ip  %s  port %d change ok", m_ip.c_str(), m_port);
		m_slaveStatus = eInstance_slavePromoteStatus::init;

		asynCommand("keys *");
	}
}