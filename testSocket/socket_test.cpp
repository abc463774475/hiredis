#include "socket_test.h"
#include "Random.h"
#include "tool.h"
#include <stdlib.h>

Socket_test::Socket_test()
{

}

Socket_test::~Socket_test()
{}

void Socket_test::connectedDo()
{
	CSocketCtrl_base::connectedDo();
	
	m_isNeedReConnectWhenLost = false;
	
}

void Socket_test::processMsg(){
	for (auto it : m_recvDeque){
		Msg *pMsg = it;
		int len = pMsg->length;
		int stId = pMsg->flag;
		
		delete pMsg;

		auto it1 = m_totalMsg.find(stId);
		if (it1 == m_totalMsg.end()){
			assert(false);
		}
		delete it1->second;
		m_totalMsg.erase(it1);

		NLog->info("procMsg %d  stdId %d leftSize %ld", len, stId, (long)m_totalMsg.size());
	}

	m_recvDeque.clear();
}

void Socket_test::run(){
	CSocketCtrl_base::run();
	if ( isClosed()){
		return;
	}

	static int64_t _curTime = getTime64();
	static int times = 0;
	
	static int id = 0;

	if ( getTime64() - _curTime >= 10){
		if ( times++ >= 1){
			return;
		}
		for (int i = 0; i < 1; ++i){
			int len = RandomUInt(sizeof(Msg)*100, 1000 * sizeof(Msg));
			char *pSz = new char[len];
			memset(pSz, 0, sizeof(char)*len);
			Msg *pMsg = (Msg*)pSz;
			pMsg->length = len;
			pMsg->flag = ++id;
			sendMsg(pMsg);

			NLog->info("sendMsg len %d  stdId %d", len, pMsg->flag);
			
			auto it = m_totalMsg.find(pMsg->flag);
			if (it != m_totalMsg.end()){
				assert(false);
			}
			
			m_totalMsg.insert(make_pair(pMsg->flag, pMsg));
		}
		_curTime = getTime64();
	}
}