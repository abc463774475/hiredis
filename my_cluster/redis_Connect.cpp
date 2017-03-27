#include "redis_Connect.h"
#include "my_head.h"
#include "socket_db.h"

Redis_connect* gRedisCon = NULL;

Redis_connect::Redis_connect(){

}

Redis_connect::~Redis_connect(){

}

void Redis_connect::init(){
	m_context = redisConnect(m_ip.c_str(), m_port);
	if ( !m_context){
		NLog->error("cannot connect redis Server ip %s port %d", m_ip.c_str(), m_port);
	}

	NLog->info("connect redis Server ip %s port %d connect  %p", m_ip.c_str(), m_port, m_context);
}

void Redis_connect::fun1(){
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