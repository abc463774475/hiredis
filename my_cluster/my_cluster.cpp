#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "redisInstance_base.h"
#include <signal.h>
#include "my_head.h"
#include "Random.h"
#include "instance_cli.h"
#include "instancMgr_slaveAndMaster.h"
#include "instancMgr_cli.h"
#include "hrNetwork.h"
#include <thread>
#include <iostream>
#include "socketMgr.h"
#include "socket_db.h"
#include "socket_cache.h"
#include "redis_Connect.h"
extern "C"
{
#include "lua.h"
}


aeEventLoop *loop = NULL;

static void test1();
static void	test2();
static void test3();
int main(int argc, char *argv[])
{
	signal(SIGPIPE, SIG_IGN);

	InitRandomNumberGenerator();
	NLog = new Logger("cluster");
	NLog->Startup();

	NLog->info("this pid %lld",(long long)getpid());
	loop = aeCreateEventLoop(10240);
	
	test3();
}
void test1()
{
	gInstanceMgr_slaveAndMaster = new instancMgr_slaveAndMaster;

	redisInstance_cli *pNew = new redisInstance_cli;
	redisInstance_cli &_base = *pNew;
	_base.m_ip = "127.0.0.1";
	_base.m_port = 6379;
	_base.m_type = eReisInstanceType::master;
	_base.initial();
	_base.connectInstanc();

	//_base.asynCommand("set h1 222222");

	gInstanceMgr_slaveAndMaster->addData(pNew, "my_1");
	if (aeCreateTimeEvent(loop, 1, instan_timer, NULL, NULL) == AE_ERR)
	{
		NLog->error("createTime erro");
		return ;
	}

	std::thread t1(aeMain, loop);
	t1.detach();
	//aeMain(loop);

	while (1)
	{
		string str;
		/*std::cin >> str;*/
		getline(cin, str);
		gInstanceMgr_slaveAndMaster->addCommand(str);
	}
}

void test2()
{
	lua_State* ls = lua_open();
	lua_gc(ls, LUA_GCSTOP, 0);    /* stop collector during initialization */
	luaL_openlibs(ls);            /* open libraries */
	lua_gc(ls, LUA_GCRESTART, 0);

	lua_register(ls, "getKeysSlots", l_getKeysSlots);

	gInstanceMgr_cli = new instancMgr_cli;
	gInstanceMgr_cli->initial("");

	if (aeCreateTimeEvent(loop, 1, instancMgr_cli::instan_timer, NULL, NULL) == AE_ERR)
	{
		NLog->error("createTime erro");
		return ;
	}

	std::thread t1(aeMain, loop);
	t1.detach();
	//aeMain(loop);

	uint16_t slotsId = getKeysSlots("h1");
	while (1)
	{
		string str;
		/*std::cin >> str;*/
		getline(cin, str);
		if ( str.empty())
		{
			for (auto i = 0; i < 10000; ++i)
			{
				char sz[256];
				sprintf(sz, "set h%d  iiiiiiiiidfsd", i);
				gInstanceMgr_cli->addCommand(sz);
			}
		}
		else
		{
			gInstanceMgr_cli->addCommand(str);
		}
	}
}

void test3()
{
	gSockMgr = new SocketMgr;
	{
		gRedisCon = new Redis_connect;
		gRedisCon->m_ip = "127.0.0.1";
		gRedisCon->m_port = 6379;

		gRedisCon->init();
	}
	{
		Socket_db *pCli = new Socket_db;
		pCli->m_ip = "192.168.23.5";
		pCli->m_port = 60001;
		pCli->m_sockType = CSocketCtrl_base::eSockeType::cli;
		pCli->m_isNeedReConnectWhenLost = true;

		gSockDb = pCli;

		gSockMgr->regist(pCli);
	}
	
	while (1)
	{
		gSockMgr->run();
		usleep(1000);
	}
}
