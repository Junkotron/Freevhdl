#include <memory>
#include <string>
#include "gen_design.h"

namespace cxxrtl_design {

// This satisfies the exact undefined reference the linker is hunting for!
std::unique_ptr<bb_p_my__black__box> bb_p_my__black__box::create(
    std::string name, 
    cxxrtl::metadata_map parameters, 
    cxxrtl::metadata_map attributes) {
    
    struct my_impl : public bb_p_my__black__box {
        // Concrete simulation logic block
        bool eval(cxxrtl::performer *performer) override {
            // Read inputs (Note the double underscores Yosys generated)
            uint8_t input_val = p_data__in.get<uint8_t>();
            
            // Perform custom C++ behavior (adding 42)
            uint8_t output_val = input_val + 42;
            
	    std::cout << "Port black box backend called" << std::endl;

	    // Write outputs doing it this way instead of the set
	    // method avoids an extra delay
            p_data__out.curr.data[0]=output_val;
	    // p_data__out.set<uint8_t>(output_val);
            
	    // 4. VIKTIGT: Returnera basklassens hantering eller true
	    // så att CXXRTL vet att värdet har ändrats!
	    return bb_p_my__black__box::eval(performer);
	    //return true; // Notifies the simulation runner of a state change
        }
    };

    // Return the concrete class as a safe unique_ptr mapping
    return std::unique_ptr<my_impl>(new my_impl());
}

} // namespace cxxrtl_design
