#include "Socket_CacheListen.h"
#include <stdlib.h>
#include "socket_db.h"
#include "redis_Connect.h"

Socket_Cache* gCache = NULL;

Socket_CacheListen::Socket_CacheListen(){

}
Socket_CacheListen::~Socket_CacheListen(){

}

CSocketCtrl_base* Socket_CacheListen::getNewAcceptNode(){
	CSocketCtrl_base *pNew = new Socket_Cache;
	gCache = (Socket_Cache*)pNew;
	return pNew;
}

Socket_Cache::Socket_Cache(){

}

Socket_Cache::~Socket_Cache(){

}

void Socket_Cache::processMsg(){
	for ( auto it : m_recvDeque){
		Msg *pMsg = it;
		Msg_WorkPacket_Msg *data = (Msg_WorkPacket_Msg *)pMsg;
		WorkPacket pack(data->data, data->GetRealLength());
		int messageType = pack.GetType();

		NLog->info("messageType  %d redisCacheIsLoadFinish %d", messageType, (int)gSockDb->m_isServerStartLoadFinish);
		if ( !gSockDb->m_isServerStartLoadFinish){
			delete pMsg;
			continue;
		}
		switch (messageType)
		{
		case  MSG_WS_REDIS_LOADPLAYERINFO:			// login
		{
			int hallId;
			int  accountId;
			pack >> hallId >> accountId;

			gRedisCon->loadPlayerInfo(hallId, accountId);
		}
		break;
		case MSG_WS_REDIS_SAVEPLAYERINFO:				// exit ok ?
		{
			int accountId = 0;
			string strMsg;
			pack >> strMsg;

			gRedisCon->addSelfCache(pack.GetAccountId(), strMsg);
		}
		break;
		case MSG_WS_REDIS_QUIT:
		{
			gRedisCon->saveAllCacheToDb();
		}
		break;
		default:
			break;
		}

		delete pMsg;
	}
	m_recvDeque.clear();
}


