#include <iostream>
#include "socketMgr.h"
#include "Logger.h"
#include "Random.h"
#include <signal.h>
#include "socket_test.h"
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "MySQL/MySqlWrapper.h"

using namespace std;
using namespace MySQL;

int main(int argc, char *argv[])
{
	signal(SIGPIPE, SIG_IGN);

	InitRandomNumberGenerator();
	NLog = new Logger("cluster");
	NLog->Startup();

	gSockMgr = new SocketMgr;
	Socket_test *pCli = new Socket_test;
	pCli->m_ip = "192.168.23.254";
	pCli->m_port = 20000;
	pCli->m_sockType = CSocketCtrl_base::eSockeType::cli;
	pCli->m_isNeedReConnectWhenLost = true;

	gSockMgr->regist(pCli);

	MySQL::Connection con;

	while (1)
	{
		gSockMgr->run();
		usleep(1000);
	}
}