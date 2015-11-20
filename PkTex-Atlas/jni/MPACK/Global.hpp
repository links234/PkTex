#ifndef MPACK_GLOBAL_HPP
#define MPACK_GLOBAL_HPP

#include "Types.hpp"

#define HUD_LAYER		500.0f

namespace MPACK
{
	namespace Graphics
	{
		class Camera2D;
		class TextureMappedFont;
	}
}

namespace MPACK
{
	namespace Global
	{
		extern std::vector<std::string>		arguments;

		extern Graphics::TextureMappedFont	*pFont;
	}
}

#endif
