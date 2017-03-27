#include "socket_db.h"
#include "socket_cache.h"
#include "redis_Connect.h"
Socket_db* gSockDb = NULL;
Socket_db::Socket_db(){
	m_isServerStartLoadFinish = false;
}

Socket_db::~Socket_db(){

}

void Socket_db::connectedDo(){
	// need all playerInfo cache ok ?
	if (m_isServerStartLoadFinish){
		return;
	}
 	WorkPacket pack(MSG_REDIS_2_DB_ALLPLAYERINFO);
 	//sendPack(pack);
	saveRedisToDb();
}

void Socket_db::sendPack(WorkPacket &pack){
	DBMsgSaveData	data;
	data.stID = 0;
	data.dbthreadType = eDbThreadType_Common;

	Msg *pMsg = data.Write(pack);

	if (!pMsg){
		//SendMsg(&data);
		sendMsg(&data);
	}
	else{
		//SendMsg(pMsg);
		sendMsg(pMsg);
	}
}

void Socket_db::processMsg(){
	for (auto it : m_recvDeque){
		Msg *pMsg = it;
		Msg_WorkPacket_Msg *data = (Msg_WorkPacket_Msg *)pMsg;
		WorkPacket pack(data->data, data->GetRealLength());
		int messageType = pack.GetType();

		switch (messageType)
		{
		case MSG_DB_2_REDIS_ALLPLAYERINFO:			// recvAllInfo and cache
			{
				handle_dbCache(pack);
			}
			break;
		case MSG_DB_2_REDIS_ALLLOADFINISH:
			{
				handle_dbLoadFinish(pack);
			}
			break;
		}

		delete pMsg;
	}
	
	m_recvDeque.clear();
}

void Socket_db::handle_dbCache(WorkPacket &pack){
// 	int size = 0;
// 	pack >> size;
// 	for (int i = 0; i < size; ++i){
// 		int acountId = 0;
// 		string strCache;
// 		pack >> acountId >> strCache;
// 		
// 		//gCache->addInfo(acountId, strCache);
// 		NLog->info("curPlayerAcId  %d", acountId);
// 	}
// 	NLog->info("totalSize  %d", size);
// 	m_isServerStartLoadFinish = true;
	int acountId = 0;
	string strCache;
	pack >> acountId >> strCache;
	
	redisReply *reply = (redisReply*)redisCommand(gRedisCon->m_context,"hset playerInfo %d %s",acountId, strCache.c_str());
	if ( reply->integer != 1){
		NLog->error("redis hset erro");
	}
}

void Socket_db::handle_dbLoadFinish(WorkPacket &pack){
	m_isServerStartLoadFinish = true;
	NLog->info("load db finish");
}

void Socket_db::saveRedisToDb(){
// 	char *sz[2] = { "hgetall", "playerInfo" };
// 	size_t len[2] = { 7,10 };
// 	int  i = redisAppendCommandArgv(gRedisCon->m_context, 2, (const char**)sz, len);
// 	void *_reply;
// 	redisReply *reply;
// 	redisGetReply(gRedisCon->m_context, &_reply);
// 
// 	reply = (redisReply*)_reply;
// 	NLog->info("type  %d  len %ld", reply->type, (long)reply->elements);
// 	redisReply *reply = (redisReply*)redisCommand(gRedisCon->m_context, "hget hxd h");
// 	NLog->info("type  %d  len %ld", reply->type, (long)reply->elements);
// 
// 	freeReplyObject(reply);
	gRedisCon->fun1();
}

void Socket_db::handle_dbSavePlayerInfo(int accountId, const string &str){
	WorkPacket pack(MSG_REDIS_2_DB_SAVEPLAYERINFO);
	pack << accountId << str;
	sendPack(pack);
}