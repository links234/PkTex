#include "DemoConsoleApplication.hpp"
#include "Image.hpp"
#include "AtlasGenerator.hpp"

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
		LOGI("Demo::ConsoleApplication is working!!!");
		LOGD("Demo::ConsoleApplication has a bug!");
		LOGW("Demo::ConsoleApplication this should not be happening, falling back to older version!");
		LOGE("Demo::ConsoleApplication error, could not recover");

		DOM *dom = new DOM();
		dom->AddString("@Sprites/Ships/enemy0.png", "1");
		dom->AddString("@Sprites/Ships/enemy1.png", "1");
		dom->AddString("@Sprites/Ships/enemy2.png", "1");
		dom->AddString("@Sprites/Ships/enemy3.png", "-1");
		dom->AddString("@Sprites/Ships/Player.png", "-1");

		JSONParser jsonParser;
		jsonParser.Save("assets/local/textures.dom", dom, JSONParser::STYLE_PRETTY);

		AtlasGenerator::generateAtlas(2048, 2048, 0, AtlasGenerator::BEST_OF_ALL, "@local/textures.dom", "assets/local", "atlas");

		return 0;
	}
}
