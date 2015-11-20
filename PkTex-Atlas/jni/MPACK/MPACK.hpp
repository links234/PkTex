#ifndef MPACK_HPP
#define MPACK_HPP

#include "MPACKLinux.hpp"
#include "MPACKWindows.hpp"

#if 	defined(WINDOWS_PLATFORM)
	#define MPACK_MAIN	MPACK_WINDOWS_MAIN
	#define MPACK_INITIALIZE MPACK_WINDOWS_INITIALIZE
	#define MPACK_RUN(pApp,result) MPACK_WINDOWS_RUN(pApp,result)
	#define MPACK_RETURN(x) MPACK_WINDOWS_RETURN(x)
#elif	defined(LINUX_PLATFORM)
	#define MPACK_MAIN	MPACK_LINUX_MAIN
	#define MPACK_INITIALIZE MPACK_LINUX_INITIALIZE
	#define MPACK_RUN(pApp,result) MPACK_LINUX_RUN(pApp,result)
	#define MPACK_RETURN(x) MPACK_LINUX_RETURN(x)
#endif

#define MPACK_SHUTDOWN 						\
		MPACK::Core::Console::Reset();			\
		MPACK::Core::Console::Cleanup()
#endif
