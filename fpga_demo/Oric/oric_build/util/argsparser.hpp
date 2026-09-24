#pragma once
#include <string>
#include <vector>

namespace KMVArgs {
    extern bool verbose;
    extern unsigned long long glesning;
    extern bool replay; 
    extern std::string load_custom; // 🚀 NY GLOBAL FLAGGA FÖR VHDL-TOMSPOLE-LÄGET!

    void parse(int argc, char* argv[], const std::vector<std::string>& allowed_extra_flags = {});
    bool has_flag(const std::string& flag);
    void print_usage(const char* prog_name, const std::vector<std::string>& allowed_extra_flags);
}
