#pragma once

#include <cstdint>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <functional>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <cstdarg>



#include <magic_enum/magic_enum.hpp>


#define DEBUG 1
#define ERROR(fmt, ...) printf("Error @ '%s'(%i) !!! \n    >>", __FILE__, __LINE__); printf(fmt, __VA_ARGS__); putc('\n', stdout); 


enum LOG_CLR : uint8_t{
    LOG_DEFAULT_CLR = 0u,
    LOG_RED = 31u,
    LOG_GREEN = 32u,
    LOG_YELLOW = 33u,
    LOG_BLUE = 34u,
    LOG_MAGENTA = 35u,
    LOG_CYAN = 36u,
    LOG_WHITE = 37u,
};

void reset_logclr(){
     std::cout << "\033[0m";
}

void set_logclr(uint8_t clr)
{

    /* https://stackoverflow.com/questions/2616906/how-do-i-output-coloured-text-to-a-linux-terminal
    cout << "\033[1;31mbold red text\033[0m\n";
Here, \033 is the ESC character, ASCII 27. It is followed by [, then zero or more numbers separated by ;, and finally the letter m. The numbers describe the colour and format to switch to from that point onwards.
             foreground background
black        30         40
red          31         41
green        32         42
yellow       33         43
blue         34         44
magenta      35         45
cyan         36         46
white        37         47

reset             0  (everything back to normal)
bold/bright       1  (often a brighter shade of the same colour)
underline         4
inverse           7  (swap foreground and background colours)
bold/bright off  21
underline off    24
inverse off      27
    
    
    
    
    */

    if(clr == LOG_DEFAULT_CLR) return reset_logclr();


    char clrs[] = {"\033[1;%2dm"};
    char buf[32];
    snprintf(buf, 32, clrs, clr);
   // buf[0] = '!';
    std::cout << buf;
}

