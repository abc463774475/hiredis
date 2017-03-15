#include "instance_cli.h"
#include "my_head.h"
#include <sstream>
#include <vector>
#include <string.h>
#include "instancMgr_cli.h"

redisInstance_cli::redisInstance_cli()
{
	m_type = eReisInstanceType::cli;
}

redisInstance_cli::~redisInstance_cli()
{
}

void redisInstance_cli::reset()
{
	redisInstance_base::reset();
	m_newIp.clear();
	m_newPort = 0;
}

void redisInstance_cli::initial()
{
	redisInstance_base::initial();
	addBackMsg("MOVED", s_moved);
	addBackMsg("CLUSTER SLOTS", s_slots);
	
	for (int i = 0; i < 10; ++i)
	{
		char sz[256];
		sprintf(sz, "set h%d  iiiiiiii", i);
		//asynCommand(sz);
	}
}

void redisInstance_cli::connectCallback(int status)
{
	redisInstance_base::connectCallback(status);
	if ( m_type == eReisInstanceType::cliBase)
	{
		asynCommand("CLUSTER SLOTS");
	}

	loadLua("/home/hxd/redis/hxd.lua");
	m_status = eInstance_commonStatus::haveConnected;
}

void redisInstance_cli::disconnectCallback(int status)
{
	NLog->info("%s  %s newIp %s newPort %d", 
		getUniqueLabel().c_str(), __FUNCTION__,
		m_newIp.c_str(), m_newPort);
	string strIp = m_newIp;
	int  port = m_newPort;
	MSGDEQUE	_curMsg = m_cacheMsg;
	reset();

	if ( strIp.size() > 0 )
	{
		m_ip = strIp;
		m_port = port;
		connectInstanc();
		m_cacheMsg = _curMsg;
	}
}

void redisInstance_cli::run()
{
	startDoMsg();
}

BACKMSG_CPP_DINGYI(redisInstance_cli, moved)
{
	int slotsId, port;

	stringstream _s1(strBack);
	string str0, str1, str2, str3;
	_s1 >> str0 >> str1 >> str2 >> str3;
	slotsId = atoi(str2.c_str());
	string strNewIp(&str3[0], &str3[0] + str3.find(":"));
	string strNewPort(&str3[0] + str3.find(":") + 1, &str3[0] + str3.length());

	port = atoi(strNewPort.c_str());

	NLog->info("slotsId  %d, ip %s, port %d", slotsId, strNewIp.c_str(), port);

	// how to redirect ?
	redisAsyncDisconnect(m_context);
	// 	m_ip = strNewIp;
	// 	m_port = port;
	// 	connectInstanc();
	m_newIp = strNewIp;
	m_newPort = port;
	m_cacheMsg.push_front(stCache);
}


static void parseSlots(const string &strBack, CSLOTSVEC &vSlots)
{
	stringstream ss1(strBack);

	string strLine;
	getline(ss1, strLine);
	while (1)
	{
		if ( strLine.empty())
		{
			break;
		}
		stClusterSlotsInfo *pInfo = NULL;
		if ( strLine[0] != ' ')
		{
			stClusterSlotsInfo st;
			vSlots.push_back(st);
			pInfo = &vSlots[vSlots.size() - 1];
			pInfo->minSlots = atoi(&strLine[0] + strLine.find("(integer)") + 9);
			getline(ss1, strLine);
			pInfo->maxSlots = atoi(&strLine[0] + strLine.find("(integer)") + 9);
			getline(ss1, strLine);
		}
		else if (strLine[0] == ' ' &&
			strLine[1] == ' '&&
			strLine[2] == ' '&&
			strLine[3] != ' ')
		{
			pInfo = &vSlots[vSlots.size() - 1];
			socketInfo *pSock = NULL;
			if ( pInfo->master.ip.empty())
			{
				pSock = &pInfo->master;
			}
			else
			{
				socketInfo st;
				pInfo->slave.push_back(st);
				pSock = &pInfo->slave[pInfo->slave.size()-1];
			}
			pSock->ip = string(strLine.begin() + strLine.find(".")-3, strLine.end());
			getline(ss1, strLine);
		}
		else if (strLine[0] == ' ' &&
			strLine[1] == ' '&&
			strLine[2] == ' '&&
			strLine[3] == ' ')
		{
			pInfo = &vSlots[vSlots.size() - 1];
			socketInfo *pSock = NULL;
			if (pInfo->master.port == 0)
			{
				pSock = &pInfo->master;
			}
			else
			{
				pSock = &pInfo->slave[pInfo->slave.size() - 1];
			}
			pSock->port = atoi(&strLine[0] + strLine.find("(integer)") + 9);
			getline(ss1, strLine);
		}
	}
}

BACKMSG_CPP_DINGYI(redisInstance_cli, slots)
{
	gInstanceMgr_cli->m_slots.clear();
	parseSlots(strBack, gInstanceMgr_cli->m_slots);
	gInstanceMgr_cli->slotsLoadFinish();
}
