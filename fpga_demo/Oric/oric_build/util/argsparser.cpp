#include "argsparser.hpp"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <algorithm>

namespace KMVArgs {

    bool verbose = false;
    unsigned long long glesning = 1;
    bool replay = false; 
    std::string load_custom = ""; // 🚀 Startar stabilt som falsk!

    static std::vector<std::string> active_args;

   void print_usage(const char* prog_name, const std::vector<std::string>& allowed_extra_flags) {
        std::cout << "📋 KMV Simulator Engine (Oric Atmos)\n"
                  << "Användning: " << prog_name << " [options]\n\n"
                  << "Gemensamma optioner:\n"
                  << "  -v, --verbose       Aktivera fullstoppad loggström på bussen\n"
                  << "  -n <antal>          Glesa ut loggen, visa bara var N:te instruktion\n"
                  << "  -r, --replay        Aktivera binär HUGO-uppspelning mot kislis.trace\n"
                  << "  -l <filnamn>        Ladda en anpassad ROM/test-image till kisel-minnet\n" // 🚀 Ny info!
                  << "  -h, --help          Visa denna hjälptext\n";
        // ... (resten av print_usage är oförändrad)
    }


    bool has_flag(const std::string& flag) {
        return std::find(active_args.begin(), active_args.end(), flag) != active_args.end();
    }

    void parse(int argc, char* argv[], const std::vector<std::string>& allowed_extra_flags) {
        for (int i = 1; i < argc; ++i) {
            active_args.push_back(argv[i]);
        }

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];

            if (arg == "-v" || arg == "--verbose") {
                verbose = true;
            } 
            else if (arg == "-r" || arg == "--replay") {
                replay = true; 
            }
            else if (arg == "-l" || arg == "--load") { 
                if (i + 1 < argc) {
                    load_custom = argv[++i]; // Kliv fram ett steg och nyp filnamnet!
                } else {
                    std::cerr << "❌ Fel: Flaggan -l kräver ett filnamn (t.ex. -l viashake.bin)!\n";
                    std::exit(1);
                }
            }
            else if (arg == "-n") {
                if (i + 1 < argc) {
                    glesning = std::stoull(argv[++i]);
                } else {
                    std::cerr << "❌ Fel: Flaggan -n kräver ett numeriskt värde!\n";
                    std::exit(1);
                }
            } 
            else if (arg == "-h" || arg == "--help") {
                print_usage(argv[0], allowed_extra_flags);
                std::exit(0);
            }
            else {
                auto it = std::find(allowed_extra_flags.begin(), allowed_extra_flags.end(), arg);
                if (it == allowed_extra_flags.end()) {
                    std::cerr << "❌ Okänd eller felaktig flagga: " << arg << "\n"
                              << "Skriv '" << argv[0] << " --help' för att se giltiga argument." << std::endl;
                    std::exit(1);
                }
            }
        }
    }
}
