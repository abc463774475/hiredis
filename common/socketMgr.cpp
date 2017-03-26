#include "socketMgr.h"

SocketMgr *gSockMgr = NULL;
SocketMgr::SocketMgr(){
}

SocketMgr::~SocketMgr(){
}

void SocketMgr::run(){
	for ( auto it : m_node){
		if ( it->m_isDele){
			continue;
		}
		it->run();
	}
	for (auto it = m_node.begin(); it != m_node.end(); ){
		if ( (*it)->m_isDele){
			NLog->info("deleNode  fd %d", (*it)->m_sockfd);
			delete *it;
			it = m_node.erase(it);
		}
		else{
			it++;
		}
	}
}

void SocketMgr::regist(CSocketCtrl_base *_base){
	m_node.push_back(_base);
	NLog->info("add sock fd  %d", _base->m_sockfd);
}