#pragma once

#include "shared.hpp"
//General

#define ASM_COMMENT ';'
#define ASM_LABEL ':'
#define ASM_ADDR '$'



//Symbol Stuff

const std::vector<const char*> symbols_reg = {
    "RA", "RB","RC"
};

bool symbol_exists(const char* to_find, const std::vector<const char*>& symbols){
    
            auto it = std::find_if(symbols.begin(), symbols.end(), [to_find](const char *element)
                                   { return strcmp(element, to_find) == 0; });
            return it != symbols.end();
}

//Label Stuff

#define LABEL_INVALID (size_t)0xbeef

typedef std::pair<std::string, uint16_t> label_t;
typedef std::pair<label_t, std::vector<size_t> > label_entry_t;
typedef std::pair<std::string, label_entry_t>  label_idx_t;
typedef std::unordered_map<std::string, label_entry_t> label_map_t;

std::optional<label_idx_t> label_exists(const std::string &to_find, const label_map_t &labels){
    auto it = std::find_if(labels.begin(), labels.end(), [to_find](const label_idx_t &element)
                           { return to_find == element.second.first.first; });
    if (it != labels.end())
        return std::optional<label_idx_t>(*it);
    return std::optional<label_idx_t>();
}

std::optional<label_idx_t> label_line_exists(const std::string &to_find, const label_map_t &labels){
    auto it = std::find_if(labels.begin(), labels.end(), [to_find](const label_idx_t &element)
                           { return to_find == element.first; });
    if (it != labels.end())
        return std::optional<label_idx_t>(*it);
    return std::nullopt;
}



std::string label_str(const label_idx_t& label, bool dump_refs = false){
    char buf[512];                                      //label name, label target line, label addr label res size
    snprintf(buf, 512, "%s: '%s' = 0x%4hx [%li refs] ", label.second.first.first.c_str(), label.first.c_str(), label.second.first.second, label.second.second.size());
    auto ret = std::string(buf);
    if(dump_refs){
       
        ret.append(" ->res/refs:");
        for(const auto& ref : label.second.second){
            snprintf(buf, 512, "\n    - [%i] %lx", 0, ref);
            ret.append(buf);
        }   
       return ret;
        
    }
    return ret;
}


void print_labels(const label_map_t& labels, bool dr = false){
    puts("-LABELS-");
    for(auto& label : labels){
        std::cout << " -> " << label_str(label, dr) << std::endl;
    }
}



//Parsing Stuff


std::vector<std::string> split(const std::string& str) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;

    while (std::getline(ss, token, ' ')) {
        size_t start = 0, end = 0;
        while ((end = token.find(',', start)) != std::string::npos) {
            if (end != start) {
                tokens.push_back(token.substr(start, end - start));
            }
            start = end + 1;
        }
        if (start < token.size()) {
            tokens.push_back(token.substr(start));
        }
    }

    return tokens;
}
template <typename T>
T hexstr_to(const std::string& hex_str){
    T addr;
    std::stringstream ss;
    ss << std::hex << hex_str;
    ss >> addr;
    return addr; 
}

bool is_hexstr(const std::string& str) {
    if (str.size() > 2 && str[0] == '0' && str[1] == 'x') {
        return std::all_of(str.begin() + 2, str.end(), [](unsigned char c) {
            return std::isxdigit(c);
        });
    } else if (str.length()){
        return std::all_of(str.begin(), str.end(), [](unsigned char c) {
            return std::isxdigit(c);
        });
    }
    return false;
}


std::string remove_comments(std::string& line){
    int idx = 0;
    for(auto& c : line){
        if(c == ';'){
            line = line.substr(0, idx); break;
        }
        idx++;
    }
    return line;
}

//Error Stuff

int assembly_warning(const char* fmt, ...)
{
    set_logclr(LOG_YELLOW);
    va_list va;
    va_start(va, fmt);
    printf("!!! Assembler Warning: \n    >");
    vprintf(fmt, va);
    va_end(va);
    putc('\n', stdout);
    reset_logclr();
    return 0;
}

enum ASM_ERROR_LEVELS : int
{
    ASM_OKAY = 0,
    ASM_WARNING,
    ASM_MINOR,
    ASM_FATAL,
    ASM_INTERNAL, //when it aint the program being assembled's problem
    ASM_ERROR, //generic
};

int assembly_error(int err, const char* fmt, ...)
{
    set_logclr( (err != ASM_INTERNAL) ?  LOG_RED : LOG_CYAN);
    va_list va;
    va_start(va, fmt);

    printf("Assembler Error [%s]:  \n    >", magic_enum::enum_name<ASM_ERROR_LEVELS>((ASM_ERROR_LEVELS)err).data() );
    vprintf(fmt, va);
    va_end(va);

    putc('\n', stdout);
    reset_logclr();
    return err;
}

