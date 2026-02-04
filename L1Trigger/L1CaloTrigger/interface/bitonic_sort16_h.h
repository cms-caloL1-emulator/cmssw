
#ifndef L1Trigger_L1CaloTrigger_interface_bitonic_sort16_h_h
#define L1Trigger_L1CaloTrigger_interface_bitonic_sort16_h_h

#include <iostream>
#include "ap_int.h"
#include "L1Trigger/L1CaloTrigger/interface/algo_topIP1_h.h"

namespace p2hfIP1 {

static constexpr int Nbclusters = 16;

class GreaterSmaller{
public:
    PFcluster greater, smaller;
};

typedef ap_uint<6> dloop_t ;

void bitonicSort16(PFcluster in[Nbclusters], PFcluster out[Nbclusters]);

}

#endif
