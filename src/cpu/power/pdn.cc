#include "pdn.hh"
#include "debug/PPredStat.hh"
#include <iostream>


pdn::pdn(double _L, double _C, double _R, double _VDC, double _CLK) :
L(_L),
C(_C),
R(_R),
VDC(_VDC),
CLK(_CLK)
{
    ts = 1 / CLK;
    LmulC = L*C;
    LdivR = L/R;
    vout_2_cycle_ago = VDC;
    vout_1_cycle_ago = VDC;
    iout_1_cycle_ago = 0;
}

double 
pdn::get_voltage(double power){
    double current = power/VDC;
    
    double vout = VDC*(ts*ts)/(LmulC) 
        + vout_1_cycle_ago*(2 - ts/(LdivR))
        + vout_2_cycle_ago*(ts/(LdivR) 
        - 1 - (ts*ts)/(LmulC)) 
        - current*R*(ts*ts)/(LmulC) 
        - (1/C)*ts*(current - iout_1_cycle_ago);
        
    vout_2_cycle_ago = vout_1_cycle_ago;
    vout_1_cycle_ago = vout;
    iout_1_cycle_ago = current;

    return vout;
}

double
pdn::get_current(double power){
    return power/VDC;
}

void
pdn::clk_throttle(double throttled_CLK){
    this->CLK = throttled_CLK;
    this->ts = 1/throttled_CLK;
}

void
pdn::print_params(){
    // std::cout << "---PDN L = :" << this->L << "\n";
    // std::cout << "---PDN C = :" << this->C << "\n";
    // std::cout << "---PDN R = :" << this->R << "\n";
    std::cout << "---PDN Clk = :" << this->CLK << "\n";
}