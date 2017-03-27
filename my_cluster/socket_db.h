#pragma once
#pragma execution_character_set ("utf_8")

#include "Socket.h"

class Socket_db : public CSocketCtrl_base
{
public:
	Socket_db();
	~Socket_db();

	virtual void	connectedDo();
	virtual void	processMsg();
	virtual void	sendPack(WorkPacket &pack);

	void			handle_dbCache(WorkPacket &pack);
	void			handle_dbLoadFinish(WorkPacket &pack);

	void			handle_dbSavePlayerInfo(int accountId, const string &str);

	void			saveRedisToDb();
private:
	bool			m_isServerStartLoadFinish;
};

extern Socket_db*  gSockDb;