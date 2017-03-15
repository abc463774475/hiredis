#pragma once
#pragma execution_character_set ("utf_8")

extern "C"
{
#include "adapters/ae.h"
#include "hiredis.h"
#include "async.h"
}

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "Logger.h"
#include "tool.h"

#include <unistd.h>
#include <iostream>
#include <thread>
#include <stdlib.h>
#include <string.h>
using namespace std;

extern aeEventLoop *loop;