#pragma once
#pragma execution_character_set ("utf_8")

#include <stdint.h>
#include <unistd.h>
#include <string>
#include <deque>
#include <map>
#include <iostream>
using namespace std;

extern "C"
{
#include <hiredis.h>
}
// get a asyn connect to redis ?
class Redis_connect 
{
public:
	Redis_connect();
	~Redis_connect();

public:
	redisContext*		m_context;
	string				m_ip;
	int					m_port;
public:
	void init();

	void saveAllCacheToDb();
	void addSelfCache(int accountId, const string &str);
	void loadPlayerInfo(int hallId, int accountId);
};

extern Redis_connect* gRedisCon;