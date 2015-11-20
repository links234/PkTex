#include "MPACK.hpp"
#include "PkTexAtlas.hpp"

int result = 0;

MPACK_MAIN
{
	MPACK_INITIALIZE;

	MPACK_RUN(PkTex::Atlas, result);

    MPACK_SHUTDOWN;

    MPACK_RETURN(result);
}
