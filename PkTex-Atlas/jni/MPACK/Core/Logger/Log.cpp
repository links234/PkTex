#include "Log.hpp"

#include <stdarg.h>
#include <cstdio>

#include "Console.hpp"

#if	defined(WINDOWS_PLATFORM)
#include <windows.h>
#endif

#define LOG_TAG 		"MPACK"
#define	BUFFERSIZE 		128*1024

namespace MPACK
{
	namespace Core
	{
		void Log::Initialize()
		{
		}

		void Log::Destroy()
		{
		}

		void Log::Info(const char* pMessage, ...)
		{
			char buffer[BUFFERSIZE];
			va_list lVarArgs;
			va_start(lVarArgs, pMessage);
			vsprintf (buffer,pMessage,lVarArgs);
			va_end(lVarArgs);

			Console::SetFGColor(Console::FG_WHITE);
			printf("[  INFO ]   %s\n", buffer);
			Console::Reset();
		}

		void Log::Error(const char* pMessage, ...)
		{
			char buffer[BUFFERSIZE];
			va_list lVarArgs;
			va_start(lVarArgs, pMessage);
			vsprintf (buffer,pMessage,lVarArgs);
			va_end(lVarArgs);

			Console::SetFGColor(Console::FG_RED);
			printf("[ ERROR ]   %s\n", buffer);
			Console::Reset();
		}

		void Log::Warn(const char* pMessage, ...)
		{
			char buffer[BUFFERSIZE];
			va_list lVarArgs;
			va_start(lVarArgs, pMessage);
			vsprintf (buffer,pMessage,lVarArgs);
			va_end(lVarArgs);

			Console::SetFGColor(Console::FG_YELLOW);
			printf("[WARNING]   %s\n", buffer);
			Console::Reset();
		}

		void Log::Debug(const char* pMessage, ...)
		{
			char buffer[BUFFERSIZE];
			va_list lVarArgs;
			va_start(lVarArgs, pMessage);
			vsprintf (buffer,pMessage,lVarArgs);
			va_end(lVarArgs);

			Console::SetFGColor(Console::FG_MAGENTA);
			printf("[ DEBUG ]   %s\n", buffer);
			Console::Reset();
		}
	}
}
