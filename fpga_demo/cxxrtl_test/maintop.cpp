 #include <iostream>
 #include "vectop_gen.cpp"
    
 using namespace std;
    
 int main()
 {
     cxxrtl_design::p_myvectop top;
    
     bool prev_led = 0;
    
     top.step();
     for(int cycle=0;cycle<16;++cycle){

       top.p_vec.set<uint32_t>(cycle);
       top.step();
    
       bool result        = top.p_y.get<bool>();
    
       cout << "cycle " << cycle << ", y: " << result << endl;
         
     }
 }
