#include "InputResource.hpp"

#include "AssetInputResource.hpp"
#include "FileInputResource.hpp"
#include "StringEx.hpp"

using namespace std;

namespace MPACK
{
	namespace Core
	{
		const int PATH_BUFFER_SIZE=256;

		InputResource::InputResource(const char* pPath)
		{
			mPath = new char[strlen(pPath)+2];
			strcpy(mPath,pPath);
		}

		const char* InputResource::GetPath()
		{
			return mPath;
		}

		InputResource::~InputResource()
		{
			delete[] mPath;
		}

		InputResource* GetInputResource(const char* pPath)
		{
			const char* temp = pPath;
	#ifdef ANDROID_PLATFORM
			if(pPath[0]=='@')
			{
				return (InputResource*)(new AssetInputResource(pPath+1));
			}
			if(pPath[0]=='&')
			{
				return (InputResource*)(new FileInputResource(pPath+1));
			}
	#elif	defined(WINDOWS_PLATFORM) || defined(LINUX_PLATFORM)
			char pathBuffer[PATH_BUFFER_SIZE];
			if(pPath[0]=='@')
			{
				strcpy(pathBuffer,"assets/");
				strcat(pathBuffer,pPath+1);
			}
			if(pPath[0]=='&')
			{
				strcpy(pathBuffer,pPath+1);
			}
			return (InputResource*)(new FileInputResource(pathBuffer));
	#endif
			LOGE("LoadInputResource: invalid path %s",pPath);
			return NULL;
		}

		string GetInputResourcePath(string path)
		{
	#ifdef ANDROID_PLATFORM
			if(path[0]=='@')
			{
				return StringEx::Substring(path,1);
			}
			if(path[0]=='&')
			{
				return StringEx::Substring(path,1);
			}
	#elif	defined(WINDOWS_PLATFORM) || defined(LINUX_PLATFORM)
			if(path[0]=='@')
			{
				return string("assets/")+StringEx::Substring(path,1);
			}
			if(path[0]=='&')
			{
				return StringEx::Substring(path,1);
			}
	#endif
			return path;
		}
	}
}
