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
	sendPack(pack);
	//saveRedisToDb();
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
	int hallId;
	int acountId = 0;
	string strCache;
	pack >> hallId >> acountId >> strCache;

	gRedisCon->addSelfCache(acountId, strCache);

	if (hallId != -1){
		// sendTodb
		WorkPacket sendPack(MSG_REDIS_WS_LOADPLAYERINFO);
		sendPack << hallId << acountId << strCache;
		gCache->sendPack(sendPack);
	}
}

void Socket_db::handle_dbLoadFinish(WorkPacket &pack){
	m_isServerStartLoadFinish = true;
	NLog->info("load db finish");
}

void Socket_db::saveRedisToDb(){
	gRedisCon->saveAllCacheToDb();
}

void Socket_db::handle_dbSavePlayerInfo(int accountId, const string &str){
	WorkPacket pack(MSG_REDIS_2_DB_SAVEPLAYERINFO);
	pack << accountId << str;
	sendPack(pack);
}