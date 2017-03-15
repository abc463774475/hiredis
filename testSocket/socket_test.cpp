#include "socket_test.h"

Socket_test::Socket_test()
{

}

Socket_test::~Socket_test()
{}

void Socket_test::connectedDo()
{
	CSocketCtrl_base::connectedDo();
	
	for (int i = 0; i < 1; ++i)
	{
		Msg msg;
		msg.length = sizeof(Msg);
		sendMsg(&msg);
	}
}