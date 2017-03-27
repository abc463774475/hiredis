#include "lib_ea.h"

/*
	To test the library, include "lib_ea.h" from an application project
	and call lib_eaTest().
	
	Do not forget to add the library to Project Dependencies in Visual Studio.
*/

static int s_Test = 0;

extern "C" int lib_eaTest();

int lib_eaTest()
{
	return ++s_Test;
}