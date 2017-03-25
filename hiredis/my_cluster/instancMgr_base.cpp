#include "instancMgr_base.h"
#include "my_head.h"

instanceMgr_base::instanceMgr_base()
{

}

instanceMgr_base::~instanceMgr_base()
{

}

void instanceMgr_base::runCommand()
{
}

void instanceMgr_base::run()
{
	runCommand();
}

void instanceMgr_base::addCommand(const string &str)
{
	lock_guard<mutex> guard(m_lockCommand);
	m_command.push_back(str);
}