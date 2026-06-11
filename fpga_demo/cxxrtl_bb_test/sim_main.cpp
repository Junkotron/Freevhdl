#include <iostream>
#include "gen_design.h"

int main() {
    cxxrtl_design::p_top top_design;

    std::cout << "Verilog top file..." << std::endl;
    
    // Drive top-level inputs (Verify underscore quantity in gen_design.h)
    top_design.p_io__in.set<uint8_t>(10);
    std::cout << "Simulation Input:  " << (int)top_design.p_io__in.get<uint8_t>() << std::endl;
    
    // Evaluate the simulation. This calls your custom C++ blackbox eval() automatically!
    top_design.step();

    // Print the final outputs

    std::cout << "Simulation Output: " << (int)top_design.p_io__out.get<uint8_t>() << std::endl;

    return 0;
}
