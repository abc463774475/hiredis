#include "redis_Connect.h"
#include "my_head.h"
#include "socket_db.h"
#include "socket_cache.h"

Redis_connect* gRedisCon = NULL;

Redis_connect::Redis_connect(){

}

Redis_connect::~Redis_connect(){

}

void Redis_connect::init(){
	m_context = redisConnect(m_ip.c_str(), m_port);
	if ( !m_context || m_context->err){
		NLog->error("cannot connect redis Server ip %s port %d", m_ip.c_str(), m_port);
	}

	NLog->info("connect redis Server ip %s port %d connect  %p", m_ip.c_str(), m_port, m_context);
}

void Redis_connect::addSelfCache(int accountId, const string &str){
	redisReply *reply = (redisReply*)redisCommand(m_context, "hset playerInfo %d %s", accountId, str.c_str());
	if (!reply || reply->type == REDIS_REPLY_ERROR){
		NLog->error("%s   reply %p",__FUNCTION__, reply);
	}
	// if need saveToDb  to disk save ?  later do it 
}

void Redis_connect::saveAllCacheToDb(){
	redisReply *reply = (redisReply*)redisCommand(m_context, "hgetall playerInfo");
	NLog->info("type  %d  len %ld", reply->type, (long)reply->elements);

	// need

	for (int i = 0; i < reply->elements;){
		int accountId = atoi(reply->element[i]->str);
		string strMsg(reply->element[i + 1]->str);

		i += 2;
		gSockDb->handle_dbSavePlayerInfo(accountId, strMsg);
	}
	freeReplyObject(reply);
}

void Redis_connect::loadPlayerInfo(int hallId, int accountId){
	redisReply *reply = (redisReply*)redisCommand(m_context, "hget playerInfo %d",accountId);
	if ( reply){

	}

	string strBack(reply->str);
	if (strBack == "nil" || strBack.size() == 0){
		// needSendToDb
		WorkPacket pack(MSG_REDIS_DB_LOADONEPLAYERINFO);
		pack << accountId;
		gSockDb->sendPack(pack);
		return;
	}
	else{
		WorkPacket pack(MSG_REDIS_WS_LOADPLAYERINFO);
		pack << hallId <<  accountId << strBack;
		
		gCache->sendPack(pack);
	}
	
}

