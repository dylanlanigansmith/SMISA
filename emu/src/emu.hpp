#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <magic_enum/magic_enum.hpp>



#define ERROR(fmt, ...) printf("Error @ '%s'(%i) !!! \n    >>", __FILE__, __LINE__); printf(fmt, __VA_ARGS__); putc('\n', stdout); 