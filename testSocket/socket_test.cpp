#include "socket_test.h"
#include "Random.h"

Socket_test::Socket_test()
{

}

Socket_test::~Socket_test()
{}

void Socket_test::connectedDo()
{
	CSocketCtrl_base::connectedDo();
	
	m_isNeedReConnectWhenLost = false;
	for (int i = 0; i < 100; ++i)
	{
		int len = RandomUInt(sizeof(Msg), 100 * sizeof(Msg));
		char *pSz = new char[len];
		memset(pSz, 0, sizeof(char)*len);
		Msg *pMsg = (Msg*)pSz;
		pMsg->length = len;
		sendMsg(pMsg);

		delete []pSz;

		NLog->info("sendMsg len %d", len);
	}
}