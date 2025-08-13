#ifndef L1Trigger_L1CaloTrigger_bitonicSort32_h_h
#define L1Trigger_L1CaloTrigger_bitonicSort32_h_h

#include <iostream>
#include "ap_int.h"
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP21_h.h"

namespace p2rctIP21 {

static constexpr int N = 32;

class GreaterSmaller{
public:
    ecalcluster greater, smaller;
};

typedef ap_uint<6> dloop_t ;

void bitonicSort32(ecalcluster in[N], ecalcluster out[N]);

} // namespace bitonicSort32

#endif
