#include "socket_cache.h"
#include <stdlib.h>
#include "socket_db.h"

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
		case  10000:			// login
		{
			int  accountId;
			pack >> accountId;
			
			
		}
		break;
		case 10001:				// exit ok ?
		{
			int accountId = 0;
			string strMsg;
			pack >> strMsg;
			
			addInfo(accountId, strMsg);
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

void Socket_Cache::loadInfo(int accountId, const string &str, bool isServerFirstLoad /* = false */){
	redisReply*reply = (redisReply*)redisCommand(m_context, "hget playerInfo %d", accountId);
	string strBack(reply->str);
	if (strBack == "nil"){
		// needSendToDb handle
		WorkPacket sendp(111);
		gSockDb->sendPack(sendp);
	}
	else{
		WorkPacket sendp(222);
		sendPack(sendp);
	}
}

void Socket_Cache::saveAllToDb(){
	redisReply*reply = (redisReply*)redisCommand(m_context, "hgetall playerInfo");
	// array load and 
}