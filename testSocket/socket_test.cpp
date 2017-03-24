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
		int stId = pMsg->dwType;
		
		auto it1 = m_totalMsg.find(stId);
		if (it1 == m_totalMsg.end()){
			assert(false);
		}

		NLog->info("procMsgLen %d  dwType %d leftSize %ld", len, stId, (long)m_totalMsg.size() - 1);
		
		for (int i = sizeof(Msg); i < len; ++i){
			char *s1 = (char*)pMsg;
			char *s2 = (char*)it1->second;

			if (s1[i] != s2[i]){
				assert(false);
				NLog->error("curMsgLen is %d   erro pos %d  left %d", len, i, i%4920);
				break;
			}
		}

		delete it1->second;
		m_totalMsg.erase(it1);
		delete pMsg;
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
		if ( times++ >= 1000){
			return;
		}
		for (int i = 0; i < 100; ++i){
			int len = /*RandomUInt(const_iSendbufSize,const_iSendbufSize * 100)*/RandomUInt(sizeof(Msg), const_iSendbufSize *10 );
			char *pSz = new char[len];
			memset(pSz, 0, sizeof(char)*len);
			Msg *pMsg = (Msg*)pSz;
			pMsg->length = len;
			pMsg->dwType = ++id;
			
			for (int i = sizeof(Msg); i < len; ++i){
				pSz[i] = RandomUInt(0,128);
			}

			sendMsg(pMsg);
			//NLog->info("sendMsg len %d  stdId %d", len, pMsg->dwType);
			
			auto it = m_totalMsg.find(pMsg->dwType);
			if (it != m_totalMsg.end()){
				assert(false);
			}
			
			m_totalMsg.insert(make_pair(pMsg->dwType, pMsg));
		}
		_curTime = getTime64();
	}
}