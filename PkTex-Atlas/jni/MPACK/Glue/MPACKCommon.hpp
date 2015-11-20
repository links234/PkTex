#ifndef MPACKCOMMON_HPP
#define MPACKCOMMON_HPP

#include "Global.hpp"

#include "Algorithm.hpp"
#include "Core.hpp"
#include "Debug.hpp"
#include "Graphics.hpp"
#include "Math.hpp"
#include "Resources.hpp"
#include "Misc.hpp"


#define MPACK_FORCE_SEMICOLON 							\
	do 									\
	{ 									\
										\
	} while(0)

#define MPACK_RUN_COMMON(UserApplication,result) 				\
	UserApplication *MPACK_pUserApplication = new UserApplication(); 	\
	result = MPACK_pUserApplication->Main();				\
	delete MPACK_pUserApplication

#endif
