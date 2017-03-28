#include "socket_cache.h"
#include <stdlib.h>
#include "socket_db.h"
#include "redis_Connect.h"

Socket_Cache* gCache = NULL;

Socket_Cache::Socket_Cache(){

}
Socket_Cache::~Socket_Cache(){

}

void Socket_Cache::init(){
	m_context = redisConnect(m_redisIp.c_str(), m_redisPort);
	if (!m_context){
		NLog->error("redisCannot Connect ip %s port %d", m_redisIp.c_str(), m_redisPort);
		exit(1);
	}
}

void Socket_Cache::processMsg(){
	for ( auto it : m_recvDeque){
		Msg *pMsg = it;
		Msg_WorkPacket_Msg *data = (Msg_WorkPacket_Msg *)pMsg;
		WorkPacket pack(data->data, data->GetRealLength());
		int messageType = pack.GetType();

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
		default:
			break;
		}
	}
}

void Socket_Cache::addInfo(int accountId, const string &str, bool isServerFirstLoad){
	redisReply* reply = (redisReply*)redisCommand(m_context, "hset playerInfo %d %s", accountId, str.c_str());
	string strBack(reply->str);
	if (strBack == "nil"){
		// needSendToDb handle
		NLog->error("hset playerInfo erro %d", accountId);
	}
}

void Socket_Cache::saveAllToDb(){
}