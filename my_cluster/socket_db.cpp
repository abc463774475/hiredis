#include "socket_db.h"
#include "Socket_CacheListen.h"
#include "redis_Connect.h"
#include <stdlib.h>
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
		case MSG_DB_REDIS_QUIT:
			{
				handle_quit();
			}
			break;
		default:
			NLog->error("not handle msg %d", messageType);
			break;
		}

		delete pMsg;
	}
	
	m_recvDeque.clear();
}

void Socket_db::handle_dbCache(WorkPacket &pack){
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

void Socket_db::handle_quit(){
	NLog->info("exit now ");
	exit(1);
}

void Socket_db::sendQuit(){
	WorkPacket pack(MSG_REDIS_DB_QUIT);
	sendPack(pack);
}