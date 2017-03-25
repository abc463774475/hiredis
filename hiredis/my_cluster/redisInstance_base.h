#pragma once
#pragma execution_character_set ("utf_8")
#include <stdint.h>
#include <unistd.h>
#include <string>
#include <deque>
#include "async.h"
#include <map>

using namespace std;
enum class eReisInstanceType : int
{
	slave = 1,
	master = 2,
	sentinel = 3,
	cliBase = 4,
	cli = 5,
	max ,
};

// 
struct stCacheMsg
{
	int64_t	id;				// uniqueId indicate first
	string str;

	stCacheMsg()
	{
		id = 0;
	}
};

/*
 status   maybe include    just connect   info check  slave check disconnect 
 */
enum class eInstance_commonStatus : int
{
	init,						// first 
	justConnecting,				// is in connect
	haveConnected,				// connected
	discconect,					
	needDele,					// need Del
	mustExist,					
	max,
};

enum class eInstance_slavePromoteStatus : int
{
	init,
	voteing,
	voted,
	needPromote,
	promoting,
	changeSalveof,
	finish,
	max,
};

enum  eTimerType : int
{
	eTimerType_connect,		// connect to server
	eTimerType_voteSlave,
	eTimerType_promoting,	//
	eTimerType_promoted,
	eTimerType_msg_info,
	eTimerType_timerMax,
};

struct  stTimerInfo
{
	int64_t		startTime;		// 
	int64_t		intervalTime;
	int			leftNum;
	
	
	stTimerInfo()
	{
		startTime = 0;
		intervalTime = 0;
		leftNum = 0;
	}
};

/*
 baseinstance  info

 */
class redisInstance_base
{
public:
	typedef deque<stCacheMsg> MSGDEQUE;
	typedef int(*MSG_FUN) (redisInstance_base *pBase,const string &strBack, const stCacheMsg &st);
public:
	struct stBackMsgHandle 
	{
		MSG_FUN		fun;
	};
public:
	typedef map<string, stBackMsgHandle>	BACKMAP;
public:
	redisInstance_base();
	virtual~redisInstance_base();

public:
	pid_t				m_processId;						// redis pross id
	eReisInstanceType	m_type;								// redisType
	redisAsyncContext*	m_context;							// context
	std::string			m_ip;								// ip
	int					m_port;								// port
	int64_t				m_id;								// unique id
	int64_t				m_doMsgId;
	string				m_run_id;							// slave run_id   indicate unique id
	MSGDEQUE			m_cacheMsg;

	eInstance_commonStatus	m_status;							// status
	BACKMAP				m_backMsg;

public:
	/*
		the status timer  and other info
		other info we need create related other info
		only single key  to single value
	*/
	stTimerInfo			m_timer[eTimerType::eTimerType_timerMax];

public:
	bool				connectInstanc();					// must asynconsist
	
	virtual void		connectCallback(int status);		
	virtual void		disconnectCallback(int status);

	void				asynCommand(const string &str);
	void				getMsgback(void *r, void *privdata);
	string				getMsgback_str(void *r, char *prefix);

	virtual void		run();
	void				startDoMsg();

	void				addBackMsg(const string &str,MSG_FUN  fun);
	virtual void		initial();
	virtual void		reset();
	
	void				setTimerData(eTimerType eType, int leftNum, int64_t intervalTime);

	string				getUniqueLabel();
public:
	void				run_timer();
	/*
		the  base aemain maybe contain the connect outTimer function  so we must ensure the way ?
	*/
	void				outTimer_connect();					// connect outTimer
	/*
		the msg info is outTimer we can handle it like ping    
		it may indicate two meanings   
			1 the connect is shutdown 
			2 the server may shutdown
		but wo do it only like the server may shutdown
	*/
	void				outTimer_info();
	/*
		this situation maybe very complex 
	*/
	void				outTimer_voteSlave();				// select promote slave outtimer
	/*
		this situation maybe very complex
	*/
	void				outTimer_promoting();

public:
	virtual void		loadLua(const string &strFile);
};

void redisInstan_connectCallback(const redisAsyncContext *c, int status);
void redisInstan_disconnectCallback(const redisAsyncContext *c, int status);
void redisInstan_getMsgback(redisAsyncContext *c, void *r, void *privdat);

int instan_timer(struct aeEventLoop *eventLoop, long long id, void *clientData);

#define BACKMSG_H_DINGYI(name)					static int  s_##name (redisInstance_base *pBase,const string &strBack, const stCacheMsg &st);\
	void  name(const string &strBack, const stCacheMsg &st)
#define BACKMSG_CPP_DINGYI(Base,name)				int Base::s_##name(redisInstance_base *pBase,const string &strBack, const stCacheMsg &st)\
{\
Base *pClient = (Base*)pBase; \
pClient->name(strBack, st); \
return 1; \
	}\
	void Base::name(const string &strBack, const stCacheMsg &stCache)
