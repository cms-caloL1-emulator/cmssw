//------------------------------------
// IP1 Logic for Phase2L1CaloL1RCTEmulator.cc
//------------------------------------
#ifndef L1Trigger_L1CaloTrigger_RCT_IO
#define L1Trigger_L1CaloTrigger_RCT_IO

#include <iostream>
#include "ap_int.h"
#include <algorithm>
#include <utility>
#include <stdint.h>

namespace p2rctIO {

static constexpr int N_BITS_CRYSTAL = 16;
static constexpr int N_BITS_ENERGY = 10;
static constexpr int N_BITS_TIMING = 5;
static constexpr int N_BITS_SPIKE = 1;
static constexpr float LSB_ENERGY = 0.5;

class linkECAL {
    private:
    ap_uint<576> data;

    public:
    // constructor
    linkECAL() {
        data = (ap_uint<576>)0;
    }

    inline ap_uint<576> Data(void) {return data;}

    inline void setCrystal(float energy, float timing, int spike, int iEta, int iPhi) {
        // Clear the 16 bits for this crystal
        clearCrystal(iEta, iPhi);

        // Build the 16 bits for this crystal
        ap_uint<16> crystalData;
        crystalData = 
        ((ap_uint<16>)(energy/LSB_ENERGY) & 0x3FF) |
        (((ap_uint<16>)(timing)<<N_BITS_ENERGY) & 0x1F) |
        (((ap_uint<16>)(spike)<<(N_BITS_ENERGY+N_BITS_TIMING)) & 0x1) ;

        // Put this crystal's 16 bits into the 576 bit data
        int startId = (iEta%5)*5+(iPhi%5);
        int start = startId * N_BITS_CRYSTAL;
        ap_uint<576> shiftedCrystalData = (ap_uint<576>)crystalData<<start;

        this->data = this->data | shiftedCrystalData;
    }

    inline void clearCrystal(int iEta, int iPhi) {
        // Find first bit corresponding to crystal at iEta, iPhi
        int startId = (iEta%5)*5+(iPhi%5);
        int start = startId * N_BITS_CRYSTAL;

        ap_uint<576> spaceHolder = (ap_uint<576>)(0xFFFF)<<start; //16 1s, shifted to start at start

        this->data = this->data & ~spaceHolder; //Turn the 16 bits for this crystal to 0
    }
};

class ecalcrystal{
    public:
    ap_uint<10> energy;
    ap_uint<5> timing;
    ap_uint<1> spike;
    ap_uint<5> eta;
    ap_uint<5> phi;

    ecalcrystal(){
        energy = 0;
        timing = 0;
        spike = 0;
        eta = 0;
        phi = 0;
    }

    inline ap_uint<16> getecalcrystal(void){
    	ap_uint<16> data;
      data  = 
      ((ap_uint<16>)energy & 0x3FF) |
      (((ap_uint<16>)timing)<<10 & 0x1F) |
      (((ap_uint<16>)spike)<<15 & 0x1) ;
      return data ;
    }

    ecalcrystal(ap_uint<16> i){
    	this->energy = i.range(9, 0);
    	this->timing = i.range(14, 10);
    	this->spike = i.range(15, 15);
    }

    inline void setecalcrystal(ap_uint<16> i, ap_uint<5> k, ap_uint<5> j){
    	this->energy = i.range(9, 0);
    	this->timing = i.range(14, 10);
    	this->spike = i.range(15, 15);
    	this->eta = k;
    	this->phi = j;
    }

    ecalcrystal(const ecalcrystal& rhs){
    energy=rhs.energy;
    timing=rhs.timing;
    spike=rhs.spike;
    eta=rhs.eta;
    phi=rhs.phi;
    }

    ecalcrystal& operator=(const ecalcrystal& rhs){
    this->energy=rhs.energy;
    this->timing=rhs.timing;
    this->spike=rhs.spike;
    this->eta=rhs.eta;
    this->phi=rhs.phi;
        return *this;
    }

    inline ap_uint<10> Energy(void) {return energy;}
    inline ap_uint<5> Eta(void) {return eta;}
    inline ap_uint<5> Phi(void) {return phi;}
};


class ecalcluster{
    public:
    ap_uint<10> seedEnergy;
    ap_uint<12> energy;
    ap_uint<5> eta;
    ap_uint<5> phi;
    ap_uint<10> et5x5;
    ap_uint<10> et2x5;
    ap_uint<5> timing;
    ap_uint<1> spike;
    ap_uint<1> satur;
    ap_uint<2> brems;
    ap_uint<3> spare;
    ap_uint<64> data;

    ecalcluster(){
        seedEnergy = 0;
        energy = 0;
        eta = 0;
        phi = 0;
        et5x5 = 0;
        et2x5 = 0;
        timing = 0;
        spike = 0;
        satur = 0;
        brems = 0;
        spare = 0;
        data = 0;
    }

    ecalcluster& operator=(const ecalcluster& rhs){
        seedEnergy = rhs.seedEnergy;
        energy = rhs.energy;
        eta = rhs.eta;
        phi = rhs.phi;
        et5x5 = rhs.et5x5;
        et2x5 = rhs.et2x5;
        timing = rhs.timing;
        spike = rhs.spike;
        satur = rhs.satur;
        brems = rhs.brems;
        spare = rhs.spare;
        data = rhs.data;
        return *this;
    }

    inline ap_uint<64> getecalcluster(void){
      data = (seedEnergy) | 
      (((ap_uint<64>) energy)  << 10) | 
      (((ap_uint<64>) eta)  << 22) | 
      (((ap_uint<64>) phi)  << 27) | 
      (((ap_uint<64>) et5x5)       << 32) |
      (((ap_uint<64>) et2x5)       << 42) |
      (((ap_uint<64>) timing)       << 52) |
      (((ap_uint<64>) spike)       << 57) |
      (((ap_uint<64>) satur)       << 58) |
      (((ap_uint<64>) brems)       << 59) |
      (((ap_uint<64>) spare)       << 61);
    return data ;
    }

    inline void fillecalcluster(ap_uint<64> i){
    	this->seedEnergy = i.range(9, 0);
    	this->energy = i.range(21, 10);
    	this->eta = i.range(26, 22);
    	this->phi = i.range(31, 27);
    	this->et5x5 = i.range(41, 32);
    	this->et2x5 = i.range(51, 42);
    	this->timing = i.range(56, 52);
    	this->spike = i.range(57, 57);
    	this->satur = i.range(58, 58);
    	this->brems = i.range(60, 59);
    }

    ecalcluster(ap_uint<10> seedEnergy, ap_uint<12> energy, ap_uint<5> eta, ap_uint<5> phi, ap_uint<10> et5x5, ap_uint<10> et2x5, ap_uint<5> timing, ap_uint<1> spike, ap_uint<1> satur, ap_uint<2> brems, ap_uint<3> spare){
        data = (seedEnergy) | 
      (((ap_uint<64>) energy)  << 10) | 
      (((ap_uint<64>) eta)  << 22) | 
      (((ap_uint<64>) phi)  << 27) | 
      (((ap_uint<64>) et5x5)       << 32) |
      (((ap_uint<64>) et2x5)       << 42) |
      (((ap_uint<64>) timing)       << 52) |
      (((ap_uint<64>) spike)       << 57) |
      (((ap_uint<64>) satur)       << 58) |
      (((ap_uint<64>) brems)       << 59) |
      (((ap_uint<64>) spare)       << 61);
    }

    inline ap_uint<12> Energy(void) {return energy;}
    inline ap_uint<5> Eta(void) {return eta;}
    inline ap_uint<5> Phi(void) {return phi;}
    inline ap_uint<64> Data(void) {return data;}


};


class ecaltower{
    public:
    ap_uint<12> energy;
    ap_uint<5> timing;
    ap_uint<1> spike;
    ap_uint<18> data;

    ecaltower(){
        energy = 0;
        timing = 0;
        spike = 0;
        data = 0;
    }

    ecaltower& operator=(const ecaltower& rhs){
        energy = rhs.energy;
        timing = rhs.timing;
        spike = rhs.spike;
        data = rhs.data;
        return *this;
    }

    inline ap_uint<18> getecaltower(void){
      data = (((ap_uint<18>) energy)  | 
      (((ap_uint<18>) timing) << 12) |
      (((ap_uint<18>) spike)  << 17)) ;
    return data ;
    }

};

void algo_top(ap_uint<576> link_in[N_INPUT_LINKS], ap_uint<576> link_out[N_OUTPUT_LINKS]);

} // namespace p2rctIP1

#endif
