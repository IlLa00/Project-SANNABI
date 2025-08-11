#pragma once

#include <iostream>
#include <windows.h>
#include <vector>
#include <set>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <functional>	// std::function
#include <algorithm>	// std::find_if, std::sort

// C++ 20
#include <format>		// std::format("x:{0}, y:{1}"
#include <numbers>  
#include <filesystem>
#include <fstream>
namespace fs = std::filesystem;

#include "json_fwd.hpp" // nlohmann/json 헤더 파일
using json = nlohmann::json;

// magic_enum
// #include "magic_enum/magic_enum.hpp"


// transparent Blt
#pragma comment(lib, "msimg32.lib")

using namespace std;

#define SAFE_DELETE(ptr)		\
	if (ptr)					\
	{							\
		delete ptr;				\
		ptr = nullptr;			\
	}

// 메모리 릭. json 라이브러리 뒤쪽에서 해야지 new 키워드랑 충돌하지 않는다.
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include <cstdlib>

#ifdef _DEBUG
#define new new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#endif

// 공용 선언들 
#include "Defines.h"