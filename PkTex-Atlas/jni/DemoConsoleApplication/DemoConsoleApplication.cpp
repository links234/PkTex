#include "DemoConsoleApplication.hpp"
#include "Image.hpp"
#include "AtlasGenerator.hpp"
#include "Global.hpp"

using namespace MPACK::Global;
using namespace MPACK::Core::StringEx;

namespace Demo
{
	ConsoleApplication::ConsoleApplication()
	{
	}

	ConsoleApplication::~ConsoleApplication()
	{
	}

	int ConsoleApplication::Main()
	{
  	//LOGI("Demo::ConsoleApplication is working!!!");
		//LOGD("Demo::ConsoleApplication has a bug!");
		//LOGW("Demo::ConsoleApplication this should not be happening, falling back to older version!");
		//LOGE("Demo::ConsoleApplication error, could not recover");

		std::string noCommand = "No command found. Use : atlas -help for more info of how to use this script.";
		std::string help = "Help\nUsage: atlas width[int] height[int] padding[int] Json_Path[string] output_Path[string] prefix[string] sort_Type[OPTIONAL integer]\n";
		help+="    -width : an integer representing the maximum width of the atlases\n";
		help+="    -height : an integer representing the maximum height of the atlases\n";
		help+="    -padding : an integer representing the padding between the images in the atlases\n";
		help+="    -json_Path : a string representing the path to the textures' Json\n";
		help+="    -output_Path : a string representing the maximum width of the atlases\n";
		help+="    -prefix : a string representing the prefix of the generated files' names\n";
		help+="    -sort_Type[Optional] : an optional integer between 1 and 5[default = 5] that set the sorting algorithm \n";
		help+="                    - 1 - WIDTH sorting algorithm\n";
		help+="                    - 2 - HEIGHT sorting algorithm\n";
		help+="                    - 3 - MAX SIDE sorting algorithm\n";
		help+="                    - 4 - AREA sorting algorithm\n";
		help+="                    - 5 - BEST OF ALL sorting algorithm [default]\n";
    int size = arguments.size();

    if (size <= 1)
    {
      LOGI(noCommand.c_str());
      return 0;
    }

		if (MPACK::Global::arguments[1] != "atlas" || (size != 3 && size != 8 && size != 9))
		{
			LOGI(noCommand.c_str());
			return 0;
		}	

		
		switch (size)
		{
			case 3:
			{
				if (arguments[2] != "-help") 
					LOGI(noCommand.c_str());
				else
					LOGI(help.c_str());
				break;
			}
			case 8:
			{
				bool res;
				int widthAtlas, heightAtlas, padding; 
				std::string jsonPath, outputPath, prefix;

				res = ToInt(arguments[2], widthAtlas);
				if (!res) 
				{
					LOGI("The width parameter is not an integer! Use atlas -help for more info.");
					return 0;
				}
				if (widthAtlas <= 0)
				{
					LOGI("The width parameter should be a positive integer [>0]");
					return 0;
				}

				res = ToInt(arguments[3], heightAtlas);
				if (!res) 
				{
					LOGI("The height parameter is not an integer! Use atlas -help for more info.");
					return 0;
				}
				if (heightAtlas <= 0)
				{
					LOGI("The height parameter should be a positive integer [>0]");
					return 0;
				}

				res = ToInt(arguments[4], padding);
				if (!res) 
				{
					LOGI("The padding parameter is not an integer! Use atlas -help for more info.");
					return 0;
				}
				if (padding < 0)
				{
					LOGI("The padding parameter should be a positive integer [>=0]");
					return 0;
				}

				jsonPath = arguments[5];
				outputPath = arguments[6];
				prefix = arguments[7];

				AtlasGenerator::generateAtlas(widthAtlas, heightAtlas, padding, jsonPath, outputPath, prefix);
				break;
			}
			case 9:
			{
				bool res;
				int widthAtlas, heightAtlas, padding, sortType; 
				std::string jsonPath, outputPath, prefix;

				res = ToInt(arguments[2], widthAtlas);
				if (!res) 
				{
					LOGI("The width parameter is not an integer! Use atlas -help for more info.");
					return 0;
				}
				if (widthAtlas <= 0)
				{
					LOGI("The width parameter should be a positive integer [>0]");
					return 0;
				}

				res = ToInt(arguments[3], heightAtlas);
				if (!res) 
				{
					LOGI("The height parameter is not an integer! Use atlas -help for more info.");
					return 0;
				}
				if (heightAtlas <= 0)
				{
					LOGI("The height parameter should be a positive integer [>0]");
					return 0;
				}

				res = ToInt(arguments[4], padding);
				if (!res) 
				{
					LOGI("The padding parameter is not an integer! Use atlas -help for more info.");
					return 0;
				}
				if (padding < 0)
				{
					LOGI("The padding parameter should be a positive integer [>=0]");
					return 0;
				}

				jsonPath = arguments[5];
				outputPath = arguments[6];
				prefix = arguments[7];

				res = ToInt(arguments[8], sortType);
				if (!res) 
				{
					LOGI("The sortType parameter is not an integer! Use atlas -help for more info.");
					return 0;
				}
				if (sortType <= 0 || sortType > 5)
				{
					LOGI("The sortType parameter should be an integer between 1 and 5[>=1 && <=5]");
					return 0;
				}

				AtlasGenerator::generateAtlas(widthAtlas, heightAtlas, padding, jsonPath, outputPath, prefix, sortType);
				break;
			}
		}

		return 0;
	}
}
