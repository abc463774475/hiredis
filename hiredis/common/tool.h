#pragma once
#pragma execution_character_set ("utf_8")

#include <stdint.h>
#include <string>
using namespace std;
extern "C"
{
#include "lua.h"
}

//  msTime 64bytes
int64_t	getTime64();

/*
 * getRandom64
 */
int64_t getRandom64();

uint16_t crc16(const char *buf, int len);
uint16_t getKeysSlots(const string &strKey);

int l_getKeysSlots(lua_State *ls);