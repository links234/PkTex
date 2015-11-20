#include "PkTexAtlas.hpp"
#include "Image.hpp"
#include "AtlasGenerator.hpp"
#include "Global.hpp"

using namespace MPACK::Global;
using namespace MPACK::Core::StringEx;

namespace PkTex
{
	Atlas::Atlas()
	{
	}

	Atlas::~Atlas()
	{
	}

	int Atlas::Main()
	{
		std::string noCommand = "No command found. Use : pktex-atlas --help for more info of how to use this tool.";
		std::string help = "Help\nUsage: pack width[int] height[int] padding[int] json-path[string] output-path[string] prefix[string] absolute-path-count[integer] sort_type[OPTIONAL integer]\n";
		help+="    -width : an integer representing the maximum width of the atlases\n";
		help+="    -height : an integer representing the maximum height of the atlases\n";
		help+="    -padding : an integer representing the padding between the images in the atlases\n";
		help+="    -json-path : a string representing the path to the textures' Json\n";
		help+="    -output-path : a string representing the maximum width of the atlases\n";
		help+="    -prefix : a string representing the prefix of the generated files' names\n";
		help+="    -absolute-path-count[Optional] : an integer = the length of the common absolute paths\n";
		help+="    -sort-type[Optional] : an optional integer between 1 and 5[default = 5] that set the sorting algorithm \n";
		help+="                    - 1 - WIDTH sorting algorithm\n";
		help+="                    - 2 - HEIGHT sorting algorithm\n";
		help+="                    - 3 - MAX SIDE sorting algorithm\n";
		help+="                    - 4 - AREA sorting algorithm\n";
		help+="                    - 5 - BEST OF ALL sorting algorithm [default]\n";
	    int size = arguments.size();

	    if (size <= 1)
	    {
	      LOGI(noCommand.c_str());
	      return 2;
	    }

		if (size == 2 && (arguments[1] == "--help" || arguments[1] == "-h") )
		{
			LOGI(help.c_str());
			return 0;
		}
		else if (arguments[1] == "pack" && (size == 9 || size == 10))
		{
			bool res;
			int widthAtlas, heightAtlas, padding, count;
			std::string jsonPath, outputPath, prefix;

			res = ToInt(arguments[2], widthAtlas);
			if (!res)
			{
				LOGI("The width parameter is not an integer! Use pktex-atlas --help for more info.");
				return 2;
			}
			if (widthAtlas <= 0)
			{
				LOGI("The width parameter should be a positive integer [>0]");
				return 0;
			}

			res = ToInt(arguments[3], heightAtlas);
			if (!res)
			{
				LOGI("The height parameter is not an integer! Use pktex-atlas --help for more info.");
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
				LOGI("The padding parameter is not an integer! Use pktex-atlas --help for more info.");
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

			res = ToInt(arguments[8], count);
			if (!res)
			{
				LOGI("The absolute-path-count parameter is not an integer! Use pktex-atlas --help for more info.");
				return 2;
			}
			if (count < 0)
			{
				LOGI("The absolute-path-count parameter should be a positive integer [>=0]");
				return 2;
			}

			int sortType = 5;

			if (size == 10)
			{
				res = ToInt(arguments[9], sortType);
				if (!res)
				{
					LOGI("The sort-type parameter is not an integer! Use pktex-atlas --help for more info.");
					return 0;
				}
				if (sortType <= 0 || sortType > 5)
				{
					LOGI("The sort-type parameter should be an integer between 1 and 5[>=1 && <=5]");
					return 0;
				}
			}

			AtlasGenerator::generateAtlas(widthAtlas, heightAtlas, padding, jsonPath, outputPath, prefix, count, sortType);
		}
		else
		{
			LOGI(noCommand.c_str());
			return 2;
		}

		return 0;
	}
}
