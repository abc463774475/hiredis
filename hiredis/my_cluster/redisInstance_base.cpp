#include "redisInstance_base.h"
#include "my_head.h"
#include <sstream>
#include <string.h>
#include <fstream>
#include "instancMgr_slaveAndMaster.h"

#define getCommond(msg,msglen,pat)	\
do	\
{	\
	va_list ap;	\
	memset(msg, 0, msglen);	\
	va_start(ap, pat);		\
	vsnprintf(msg, msglen - 1, pat, ap);	\
	va_end(ap);	\
}while (0)

redisInstance_base::redisInstance_base()
{
	m_context = NULL;
	reset();
}

redisInstance_base::~redisInstance_base()
{
	reset();
}

void redisInstance_base::initial()
{
}

string redisInstance_base::getUniqueLabel()
{
	char sz[256];
	sprintf(sz, "server %s port %d id %lld", m_ip.c_str(), m_port, (long long)m_id);
	return string(sz);
}
void redisInstance_base::reset()
{
	m_doMsgId = 0;
	if ( m_context)
	{
		m_context = NULL;
	}
	m_ip.clear();
	m_port = 0;
	m_id = getRandom64();
	m_run_id.clear();
	m_cacheMsg.clear();
	
	m_status = eInstance_commonStatus::init;

	int64_t curLeftTimes[eTimerType::eTimerType_timerMax] =
	{2000,2000,2000,2000,2000};
	for (int i = 0; i < eTimerType::eTimerType_timerMax; ++i)
	{
		m_timer[i].intervalTime = curLeftTimes[i];
	}
}

void redisInstance_base::addBackMsg(const string &str, redisInstance_base::MSG_FUN fun)
{
	auto it = m_backMsg.find(str);
	if ( it != m_backMsg.end())
	{
		NLog->error("addBackMsg Is Dup   %s", str.c_str());
		return;
	}
	stBackMsgHandle st;
	st.fun = fun;
	
	m_backMsg.insert(BACKMAP::value_type(str, st));
}

bool redisInstance_base::connectInstanc()
{
	m_context = redisAsyncConnect(m_ip.c_str(), m_port);
	if ( m_context->err)
	{
		NLog->error("redisConnectErro");
		return false;
	}
	m_context->data = this;

	// now i dont sure  whether the loop contains the connect out timer
	redisAeAttach(loop, m_context);
	redisAsyncSetConnectCallback(m_context, redisInstan_connectCallback);
	redisAsyncSetDisconnectCallback(m_context, redisInstan_disconnectCallback);

	m_status = eInstance_commonStatus::justConnecting;
	NLog->info("create asyn text   %p   port %d", m_context, m_port);
	return true;
}

void redisInstance_base::connectCallback(int status)
{
	if (status != REDIS_OK)
	{
		NLog->error("connectErro %s socket %d  addr %s port %d id %lld", m_context->errstr,
			m_context->c.fd,
			m_ip.c_str(),
			m_port,
			(long long)m_id);
		
		m_status = eInstance_commonStatus::discconect;
		return;
	}
	NLog->info("redisConnect Ok socket %d  addr %s port %d", 
		m_context->c.fd,
		m_ip.c_str(),
		m_port);
	m_status = eInstance_commonStatus::haveConnected;
}

void redisInstance_base::disconnectCallback(int status)
{
	
}

void redisInstance_base::asynCommand(const string &str)
{
	// can no do like this ....  one commond one reply,,,so we must pushbakc to deque to cache  only just last command back ..wo start the next?
	//redisAsyncCommand(m_context, redisInstan_getMsgback, NULL, str.c_str());
	stCacheMsg st;
	st.str = str;
	st.id = getRandom64();

	m_cacheMsg.push_back(st);
}

void redisInstance_base::getMsgback(void *r, void *privdata)
{
	redisReply *reply = static_cast<redisReply*>(r);
	if (reply == NULL) return;
	//NLog->info("argv[%s]: %s", (char*)privdata, reply->str);
	if ( m_cacheMsg.size() == 0)
	{
		NLog->error("curDequeMsg len is zero");
		return;
	}
// 	if ( m_isInTTLMode)
// 	{
// 		m_msg.pop_front();
// 		return;
// 	}

	m_doMsgId = 0;
	stCacheMsg st = m_cacheMsg.front();
	m_cacheMsg.pop_front();

	string strBack = getMsgback_str(r, "");
	
	string strName;
	if (strBack.find("MOVED") != string::npos)
	{
		strName = "MOVED";
	}
	else if (st.str.find("CLUSTER SLOTS") != string::npos)
	{
		strName = "CLUSTER SLOTS";
	}
	auto it = m_backMsg.find(strName);
	if (it != m_backMsg.end())
	{
		it->second.fun(this, strBack, st);
	}

	if (st.str != string("INFO"))
	{
		NLog->info("src  %s  \nback\n%s", st.str.c_str(), strBack.c_str());
	}
	return;
// 	if (/*reply->type != REDIS_REPLY_ARRAY*/1)
// 	{
// 		//string strBack(reply->str);
// 		if (st.str == string("INFO"))
// 		{
// 			// prase come back msg  getAllInfo
// 			// check slave Info   if  the slave  gone away  we need close this
// 			//paraseInfo(strBack);
// 		}
// 		else if (st.str.find("SLAVE") != string::npos)
// 		{
// 			//paraseSlaveof(strBack);
// 		}
// 		else if (strBack.find("MOVED") != string::npos)
// 		{
// 			//paraseMoved(strBack, st);
// 		}
// 
// 		if (st.str != string("INFO"))
// 		{
// 			NLog->info("src  %s  \nback\n%s", st.str.c_str(), strBack.c_str());
// 		}
// 	}
}

string redisInstance_base::getMsgback_str(void *_r, char *prefix)
{
	redisReply *r = static_cast<redisReply*>(_r);
	
	string out;
	char sz[1024];
	switch (r->type) {
	case REDIS_REPLY_ERROR:
		sprintf(sz, "(error) %s\n", r->str);
		out += sz;
		break;
	case REDIS_REPLY_STATUS:
		out += r->str;
		out += "\n";
		break;
	case REDIS_REPLY_INTEGER:
		sprintf(sz, "(integer) %lld\n", r->integer);
		out += sz;
		break;
	case REDIS_REPLY_STRING:
		/* If you are producing output for the standard output we want
		* a more interesting output with quoted characters and so forth */
		out += r->str;
		out += "\n";
		break;
	case REDIS_REPLY_NIL:
		out += "(nil)\n";
		break;
	case REDIS_REPLY_ARRAY:
		if (r->elements == 0) {
			out += "(empty list or set)\n";
		}
		else {
			unsigned int i, idxlen = 0;
			char _prefixlen[16];
			char _prefixfmt[16];
			char _prefix[1024] = {0};
			string tmp;

			/* Calculate chars needed to represent the largest index */
			i = r->elements;
			do {
				idxlen++;
				i /= 10;
			} while (i);

			/* Prefix for nested multi bulks should grow with idxlen+2 spaces */
			memset(_prefixlen, ' ', idxlen + 2);
			_prefixlen[idxlen + 2] = '\0';
			//strcat(_prefix, prefix);
			strcat(_prefix, prefix);
			strcat(_prefix, _prefixlen);
			

			/* Setup prefix format for every entry */
			snprintf(_prefixfmt, sizeof(_prefixfmt), "%%s%%%dd) ", idxlen);

			for (i = 0; i < r->elements; i++) {
				if ( i == 0)
				{
					sprintf(sz,_prefixfmt, "",i+1);
				}
				else
				{
					sprintf(sz,_prefixfmt, prefix, i + 1);
				}
				out += sz;
				/* Format the multi bulk entry */
				tmp = getMsgback_str((void*)r->element[i], _prefix);
				/*out = sdscatlen(out, tmp, sdslen(tmp));*/
				out += tmp;
			}
		}
		break;
	default:
		fprintf(stderr, "Unknown reply type: %d\n", r->type);
		exit(1);
	}
	return out;
}


void redisInstance_base::startDoMsg()
{
	if ( m_cacheMsg.size() == 0 || m_doMsgId != 0)
	{
		return;
	}
	stCacheMsg &st = m_cacheMsg.front();
	m_doMsgId = st.id;
	redisAsyncCommand(m_context, redisInstan_getMsgback, NULL, st.str.c_str());
	NLog->info("asyn msg %s", st.str.c_str());
}


void redisInstance_base::run()
{
}

void redisInstance_base::run_timer()
{
	int64_t	_curTime = getTime64();
	for (int i = 0; i < eTimerType::eTimerType_timerMax; ++i)
	{
	}
}

void redisInstance_base::setTimerData(eTimerType eType, int leftNum, int64_t intervalTime)
{
}

void redisInstance_base::loadLua(const string &strFile)
{
	string strMsg;
	
	fstream outfile;
	outfile.open(strFile.c_str());
	if ( !outfile)
	{
		NLog->error("luafile not exist  %s", strFile.c_str());
		return;
	}
	string str1;
	while ( getline(outfile,str1))
	{
		strMsg += str1 + "\n";
	}

	string strArgv[3];
	strArgv[0] = "eval";
	strArgv[1] = strMsg;
	strArgv[2] = "0";
	
	const char* sz[3];
	sz[0] = strArgv[0].c_str();
	sz[1] = strArgv[1].c_str();
	sz[2] = strArgv[2].c_str();
	int *iArgvs = new int(1);

	redisAsyncCommandArgv(m_context, redisInstan_getMsgback, iArgvs, 3, (const char **)sz, NULL);
}

void redisInstan_connectCallback(const redisAsyncContext *c, int status)
{
	redisInstance_base *pBase = static_cast<redisInstance_base*>(c->data);
	pBase->connectCallback(status);
}

void redisInstan_disconnectCallback(const redisAsyncContext *c, int status)
{
	redisInstance_base *pBase = static_cast<redisInstance_base*>(c->data);
	pBase->disconnectCallback(status);
}

void redisInstan_getMsgback(redisAsyncContext *c, void *r, void *privdat)
{
	redisInstance_base *pBase = static_cast<redisInstance_base*>(c->data);
	NLog->info("pridata  %p",privdat);
	if ( !privdat)
	{
		pBase->getMsgback(r, privdat);
	}
	else
	{
		delete privdat;
	}
}


int instan_timer(struct aeEventLoop *eventLoop, long long id, void *clientData)
{
	gInstanceMgr_slaveAndMaster->run();
	return 1;
}