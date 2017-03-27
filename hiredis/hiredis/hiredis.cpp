#include "hiredis.h"

/*
	To test the library, include "hiredis.h" from an application project
	and call hiredisTest().
	
	Do not forget to add the library to Project Dependencies in Visual Studio.
*/

static int s_Test = 0;

extern "C" int hiredisTest();

int hiredisTest()
{
	return ++s_Test;
}