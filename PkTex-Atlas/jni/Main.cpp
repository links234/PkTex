#include "MPACK.hpp"
#include "DemoConsoleApplication.hpp"

int result = 0;

MPACK_MAIN
{
	MPACK_INITIALIZE;

	MPACK_RUN(Demo::ConsoleApplication, result);

    MPACK_SHUTDOWN;

    MPACK_RETURN(result);
}
