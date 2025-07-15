/* 
 * Description: Phase 2 RCT emulator
 */

// system include files
#include <ap_int.h>
#include <array>
#include <cmath>
// #include <cstdint>
#include <cstdlib>  // for rand
#include <iostream>
#include <fstream>
#include <memory>

// user include files
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"

#include "CalibFormats/CaloTPG/interface/CaloTPGTranscoder.h"
#include "CalibFormats/CaloTPG/interface/CaloTPGRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/EcalAlgo/interface/EcalBarrelGeometry.h"
#include "Geometry/HcalTowerAlgo/interface/HcalTrigTowerGeometry.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "DataFormats/HcalDetId/interface/HcalSubdetector.h"
#include "DataFormats/HcalDetId/interface/HcalDetId.h"

// ECAL TPs
#include "DataFormats/EcalDigi/interface/EcalDigiCollections.h"

// HCAL TPs
#include "DataFormats/HcalDigi/interface/HcalTriggerPrimitiveDigi.h"

// Output tower collection
#include "DataFormats/L1TCalorimeterPhase2/interface/CaloCrystalCluster.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/CaloTower.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/DigitizedClusterCorrelator.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/DigitizedTowerCorrelator.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/DigitizedClusterGT.h"

#include "DataFormats/L1Trigger/interface/BXVector.h"
#include "DataFormats/L1Trigger/interface/EGamma.h"

#include "L1Trigger/L1CaloTrigger/interface/ParametricCalibration.h"
#include "L1Trigger/L1TCalorimeter/interface/CaloTools.h"

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "L1Trigger/L1CaloTrigger/interface/Phase2L1CaloEGammaUtils.h"
#include "L1Trigger/L1CaloTrigger/interface/Phase2L1RCT.h"
#include "L1Trigger/L1CaloTrigger/interface/Phase2L1GCT.h"

// RCT IP1 header files and data formats
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP1.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/RCT_IP1.h"

//////////////////////////////////////////////////////////////////////////

// Define algo_top for RCT IP1

void processOutputLinks(p2rctIP1::ecalcluster ecalclustersH1[p2rctIP1::N_CLUSTERS], p2rctIP1::ecalcluster ecalclustersH2[p2rctIP1::N_CLUSTERS], p2rctIP1::ecalcluster ecalclustersH3[p2rctIP1::N_CLUSTERS], p2rctIP1::ecaltower ecaltowers[p2rctIP1::TOWERS_IN_ETA*p2rctIP1::TOWERS_IN_PHI], ap_uint<576> link_out[p2rctIP1::N_OUTPUT_LINKS]){

ap_uint<10> start;
ap_uint<10> end;

for(p2rctIP1::loop i=0; i<p2rctIP1::N_CLUSTERS; i++){
	start=i*64 ; end=start+63;
	link_out[0].range(end, start) = ecalclustersH1[i].getecalcluster() ;
	start=(3+i)*64 ; end=start+63;
	link_out[0].range(end, start) = ecalclustersH2[i].getecalcluster() ;
	start=(6+i)*64 ; end=start+63;
    link_out[0].range(end, start) = ecalclustersH3[i].getecalcluster() ;
}

for(p2rctIP1::loop i=0; i<p2rctIP1::TOWERS_IN_ETA*p2rctIP1::TOWERS_IN_PHI; i++){
	start=i*18 ; end=start+17;
	link_out[1].range(end, start) = ecaltowers[i].getecaltower() ;
}

}

void processInputLinks(ap_uint<576> link_in[p2rctIP1::N_INPUT_LINKS], p2rctIP1::ecalcrystal ecalcrystals[p2rctIP1::CRYSTALS_IN_ETA][p2rctIP1::CRYSTALS_IN_PHI]){

ap_uint<6> wordId;
ap_uint<6> startId;

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA; i++){
    for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
    wordId  = (i/5)*6+(j/5);
    startId = (i%5)*5+(j%5);
    ap_uint<10> start   = startId*16;
    ap_uint<10> end = start + 15;
    ecalcrystals[i][j] = p2rctIP1::ecalcrystal(link_in[wordId].range(end, start));
  }
}

}

p2rctIP1::ecalcrystal  bestOf2(const p2rctIP1::ecalcrystal& ecaltp0, const p2rctIP1::ecalcrystal& ecaltp1) {

p2rctIP1::ecalcrystal x;
x = (ecaltp0.energy > ecaltp1.energy)?ecaltp0:ecaltp1;
return x;

}

void getseedEta(p2rctIP1::ecalcrystal crystals[p2rctIP1::CRYSTALS_IN_PHI], p2rctIP1::ecalcrystal &EtaMax){
 
p2rctIP1::ecalcrystal Step1[p2rctIP1::CRYSTALS_IN_PHI/2+1];
p2rctIP1::ecalcrystal Step2[p2rctIP1::CRYSTALS_IN_PHI/4+1];
p2rctIP1::ecalcrystal Step3[p2rctIP1::CRYSTALS_IN_PHI/8+1];

for(p2rctIP1::loop k=0; k<p2rctIP1::CRYSTALS_IN_PHI; k=k+2){
	Step1[k/2]  = bestOf2(crystals[k], crystals[k+1]) ;
}

Step1[15] = Step1[14] ;

for(p2rctIP1::loop k=0; k<p2rctIP1::CRYSTALS_IN_PHI/2+1; k=k+2){
    Step2[k/2]  = bestOf2(Step1[k], Step1[k+1]) ;
}

for(p2rctIP1::loop k=0; k<p2rctIP1::CRYSTALS_IN_PHI/4+1; k=k+2){
    Step3[k/2]  = bestOf2(Step2[k], Step2[k+1]) ;
}

p2rctIP1::ecalcrystal x1 = bestOf2(Step3[0], Step3[1]) ;
p2rctIP1::ecalcrystal x2 = bestOf2(Step3[2], Step3[3]) ;
        
EtaMax  = bestOf2(x1,x2) ;

}

void getseedMax(p2rctIP1::ecalcrystal crystals[p2rctIP1::CRYSTALS_IN_ETA23], p2rctIP1::ecalcrystal &Seed) {

p2rctIP1::ecalcrystal Step1[p2rctIP1::CRYSTALS_IN_ETA23/2+1] ;
p2rctIP1::ecalcrystal Step2[p2rctIP1::CRYSTALS_IN_ETA23/4+1] ;

for(p2rctIP1::loop k=0; k<p2rctIP1::CRYSTALS_IN_ETA23-1; k=k+2){
	Step1[k/2]  = bestOf2(crystals[k+1], crystals[k]) ;
}

Step1[5]=crystals[10] ;

for(p2rctIP1::loop k=0; k<p2rctIP1::CRYSTALS_IN_ETA23/2+1; k=k+2){
    Step2[k/2]  = bestOf2(Step1[k], Step1[k+1]) ;
}

p2rctIP1::ecalcrystal x1 = bestOf2(Step2[0], Step2[1]) ;

Seed  = bestOf2(x1, Step2[2]) ;

}

void getseedposition(p2rctIP1::ecalcrystal crystals[p2rctIP1::CRYSTALS_IN_ETA23][p2rctIP1::CRYSTALS_IN_PHI], p2rctIP1::ecalcrystal &Seed){

p2rctIP1::ecalcrystal EtaSlices[p2rctIP1::CRYSTALS_IN_ETA23];
p2rctIP1::ecalcrystal crystals1D[p2rctIP1::CRYSTALS_IN_PHI];

p2rctIP1::ecalcrystal tmp ;

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
        tmp = crystals[i][j] ;
        crystals1D[j] = tmp ;
	}
	getseedEta(crystals1D, EtaSlices[i]) ;
}

getseedMax(EtaSlices, Seed) ;

}


void getslice(p2rctIP1::ecalcrystal crystals[p2rctIP1::CRYSTALS_IN_PHI+4], const p2rctIP1::ecalcrystal& Seed, ap_uint<12> &value) {

ap_uint<12> tmpValue = 0 ;

p2rctIP1::ecalcrystalmask mask[p2rctIP1::CRYSTALS_IN_PHI+4] ;

for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
    mask[j].energy = 0 ;
}

for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
	if(j == Seed.phi){
		mask[j+0].energy = 1;
        mask[j+1].energy = 1;
        mask[j+2].energy = 1;
        mask[j+3].energy = 1;
        mask[j+4].energy = 1;
    }
else {}
}

for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI+4; j++){
    ap_uint<10> energytmp = crystals[j].energy ;
    ap_uint<10> energy = energytmp *  mask[j].energy ;
    ap_uint<12> tmp = tmpValue + energy ;
    tmpValue = tmp  ;
}
value = tmpValue ;
}

void zerrocrystals(p2rctIP1::ecalcrystal ecalcrystals[p2rctIP1::CRYSTALS_IN_ETA23][p2rctIP1::CRYSTALS_IN_PHI], p2rctIP1::ecalcrystal Seed, ap_uint<2> brems) {

p2rctIP1::ecalcrystalmask mask[p2rctIP1::CRYSTALS_IN_ETA23][p2rctIP1::CRYSTALS_IN_PHI] ;
p2rctIP1::ecalcrystalmask maskN[p2rctIP1::CRYSTALS_IN_ETA23][p2rctIP1::CRYSTALS_IN_PHI] ;
p2rctIP1::ecalcrystalmask maskP[p2rctIP1::CRYSTALS_IN_ETA23][p2rctIP1::CRYSTALS_IN_PHI] ;
p2rctIP1::ecalcrystalmask maskI[p2rctIP1::CRYSTALS_IN_ETA23][p2rctIP1::CRYSTALS_IN_PHI] ;

ap_uint<5> eta = Seed.eta ;

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	if(i+1 >= eta && i <= eta+1){
    for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
    	if(j+7 >= Seed.phi && j+3 <= Seed.phi ) maskN[i][j].energy=1 ;
        }
	}
}

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	if(i+1 >= eta && i <= eta+1){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
		if(j+2 >= Seed.phi && j <= Seed.phi+2 ) mask[i][j].energy =1 ;
		}
	}
}

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	if(i+1 >= eta && i <= eta+1){
        for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
        if(j >= Seed.phi+3 && j <= Seed.phi+7 ) maskP[i][j].energy=1 ;
        }
	}
}

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
        maskI[i][j].energy = ((ap_uint<1>)1 - mask[i][j].energy) ;
    }
}

if(brems == 1){
for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
		ap_uint<1> mask0 = maskI[i][j].energy * ((ap_uint<1>)1 - maskN[i][j].energy) ;
        maskI[i][j].energy = mask0 ;
       	}
	}
}

if(brems == 2){
for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
		ap_uint<1> mask0 = maskI[i][j].energy * ((ap_uint<1>)1 - maskP[i][j].energy) ;
		maskI[i][j].energy = mask0 ;
	}
}
}

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
		ap_uint<10> tmp = ecalcrystals[i][j].energy ;
		ap_uint<10> energy = tmp * maskI[i][j].energy  ;
        ecalcrystals[i][j].energy = energy ;
	}
}

}

void getcluster(p2rctIP1::ecalcrystal ecalcrystals[p2rctIP1::CRYSTALS_IN_ETA23][p2rctIP1::CRYSTALS_IN_PHI], p2rctIP1::ecalcrystal Seed, p2rctIP1::ecalcluster &output) {

ap_uint<2> brems = 0 ;
ap_uint<12> NegValue=0, CntrValue=0, PosValue=0 ;
ap_uint<12> s5x5=0, s2x5=0, s2x5n=0, s2x5p=0 ;
ap_uint<12> CntrSlice[5] ;
ap_uint<12> NegSlice[5] ;
ap_uint<12> PosSlice[5] ;

p2rctIP1::ecalcrystal extendedcrystals[p2rctIP1::CRYSTALS_IN_ETA23+4][p2rctIP1::CRYSTALS_IN_PHI+4] ;

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
    for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
        extendedcrystals[i+2][j+2].energy = ecalcrystals[i][j].energy ;
	}
}

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	if(i == Seed.eta){
		for(p2rctIP1::loop k=0; k<5; k++){
		   p2rctIP1::ecalcrystal extendedcrystals1D[p2rctIP1::CRYSTALS_IN_PHI+4] ;
		   	   for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI+4; j++){
		   		   extendedcrystals1D[j].energy = extendedcrystals[i+k][j].energy ;
		   	   }
	       getslice(extendedcrystals1D, Seed, CntrSlice[k]) ;
         }
	}
	else { }
}


for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	if(i == Seed.eta){
		for(p2rctIP1::loop k=0; k<3; k++){
		   p2rctIP1::ecalcrystal extendedcrystals1D[p2rctIP1::CRYSTALS_IN_PHI+4] ;
		   for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI-1; j++){
        		extendedcrystals1D[j+5].energy = extendedcrystals[i+k+1][j].energy ;
			}
	        getslice(extendedcrystals1D, Seed, NegSlice[k+1]) ;
		}
	}
	else { }
}

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	if(i == Seed.eta){
		for(p2rctIP1::loop k=0; k<3; k++){
		p2rctIP1::ecalcrystal extendedcrystals1D[p2rctIP1::CRYSTALS_IN_PHI+4] ;
			for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI-1; j++){
        		extendedcrystals1D[j].energy = extendedcrystals[i+k+1][j+5].energy ;
			}
	    getslice(extendedcrystals1D, Seed, PosSlice[k+1]) ;
        }
	}
		else { }
}

CntrValue = CntrSlice[1] + CntrSlice[2] + CntrSlice[3] ;
NegValue = NegSlice[1] + NegSlice[2] + NegSlice[3] ;
PosValue = PosSlice[1] + PosSlice[2] + PosSlice[3] ;
s5x5 = CntrSlice[0] + CntrSlice[1] + CntrSlice[2] + CntrSlice[3] + CntrSlice[4] ;
s2x5n = CntrSlice[1] + CntrSlice[2] ;
s2x5p = CntrSlice[2] + CntrSlice[3] ;

ap_uint<12> clusterEnergyDiv8 = CntrValue >> 3;
ap_uint<12> Total = CntrValue ;

if(NegValue > clusterEnergyDiv8 && NegValue > PosValue) {
	Total = CntrValue + NegValue;
    brems = 1;
}
else if(PosValue > clusterEnergyDiv8){
	Total = CntrValue + PosValue;
    brems  = 2;
}
else { brems = 0;}

s2x5 = (s2x5n > s2x5p) ? s2x5n:s2x5p ;

output.seedEnergy = Seed.energy ;
output.energy = Total ;
output.eta = Seed.eta ;
output.phi = Seed.phi ;
output.timing = Seed.timing ;
output.spike = Seed.spike ;
output.brems = brems ;
output.et2x5 = s2x5 ;
output.et5x5 = s5x5 ;

}

void createClusters1(p2rctIP1::ecalcrystal crystals[p2rctIP1::CRYSTALS_IN_ETA23][p2rctIP1::CRYSTALS_IN_PHI], p2rctIP1::ecalcluster ecalclusters[p2rctIP1::N_CLUSTERS]) {

p2rctIP1::ecalcrystal Seed ;

for(p2rctIP1::loop k=0; k<p2rctIP1::N_CLUSTERS; k++){
	getseedposition(crystals, Seed) ;
	getcluster(crystals,Seed,ecalclusters[k]) ;
	zerrocrystals(crystals, Seed, ecalclusters[k].brems) ;
}

}

void createTowers(p2rctIP1::ecalcrystal crystals[p2rctIP1::CRYSTALS_IN_ETA][p2rctIP1::CRYSTALS_IN_PHI],p2rctIP1::ecaltower ecaltowers[p2rctIP1::TOWERS_IN_ETA*p2rctIP1::TOWERS_IN_PHI]) {

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA; i++){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
	   ap_uint<12> energy = ecaltowers[i/5*6+j/5].energy + crystals[i][j].energy  ;
		ecaltowers[i/5*6+j/5].energy = energy  ;
		ecaltowers[i/5*6+j/5].timing = 0 ;
		ecaltowers[i/5*6+j/5].spike = 0 ;
	}
}
}

void p2rctIP1::algo_top(ap_uint<576> link_in[p2rctIP1::N_INPUT_LINKS], ap_uint<576> link_out[p2rctIP1::N_OUTPUT_LINKS]){

p2rctIP1::ecalcluster ecalclusters[p2rctIP1::N_CLUSTERS] ;
p2rctIP1::ecalcluster ecalclustersH1[p2rctIP1::N_CLUSTERS] ;
p2rctIP1::ecalcluster ecalclustersH2[p2rctIP1::N_CLUSTERS] ;
p2rctIP1::ecalcluster ecalclustersH3[p2rctIP1::N_CLUSTERS] ;
p2rctIP1::ecalcrystal ecalcrystals[p2rctIP1::CRYSTALS_IN_ETA][p2rctIP1::CRYSTALS_IN_PHI];
p2rctIP1::ecalcrystal ecalcrystalsH1[p2rctIP1::CRYSTALS_IN_ETA23][p2rctIP1::CRYSTALS_IN_PHI];
p2rctIP1::ecalcrystal ecalcrystalsH2[p2rctIP1::CRYSTALS_IN_ETA23][p2rctIP1::CRYSTALS_IN_PHI];
p2rctIP1::ecalcrystal ecalcrystalsH3[p2rctIP1::CRYSTALS_IN_ETA23][p2rctIP1::CRYSTALS_IN_PHI];

p2rctIP1::ecaltower ecaltowers[p2rctIP1::TOWERS_IN_ETA*p2rctIP1::TOWERS_IN_PHI];

//creating 25x30 crystals matrix

processInputLinks(link_in, ecalcrystals) ;
          
// 25 crystals in eta are divided into 3 regions
// to create 3 regions, 11 eta each, with 2 eta overlap
//      9+2    -      2+7+2       -     2+9
//  0...8 9.10 - 7.8 9...15 16.17 - 14.15 16...24 
  
for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
		ecalcrystalsH1[i][j] = ecalcrystals[i][j];
		ecalcrystalsH1[i][j].eta = i;
		ecalcrystalsH1[i][j].phi = j;
	}
}

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
		ecalcrystalsH2[i][j] = ecalcrystals[p2rctIP1::CRYSTALS_IN_ETA23+i-4][j];
		ecalcrystalsH2[i][j].eta = i;
		ecalcrystalsH2[i][j].phi = j;
	}
}

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23; i++){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
		ecalcrystalsH3[i][j] = ecalcrystals[p2rctIP1::CRYSTALS_IN_ETA23+i+3][j];
		ecalcrystalsH3[i][j].eta = i;
		ecalcrystalsH3[i][j].phi = j;
	}
}

// each region is treated in the same way and later we combine them back in one
//
createClusters1(ecalcrystalsH1, ecalclustersH1);
createClusters1(ecalcrystalsH2, ecalclustersH2);
createClusters1(ecalcrystalsH3, ecalclustersH3);

for(p2rctIP1::loop i=0; i<p2rctIP1::N_CLUSTERS; i++){
	ap_uint<5> eta = ecalclustersH2[i].eta + 7;
	ecalclustersH2[i].eta = eta ;
}

for(p2rctIP1::loop i=0; i<p2rctIP1::N_CLUSTERS; i++){
	ap_uint<5> eta = ecalclustersH3[i].eta + 14;
	ecalclustersH3[i].eta = eta ;
}

//  0...8 9.10 - 7.8 9...15 16.17 - 14.15 16...24 
//  clean double counted clusters 
//
for(p2rctIP1::loop i=0; i<p2rctIP1::N_CLUSTERS; i++){
	for(p2rctIP1::loop k=0; k<p2rctIP1::N_CLUSTERS; k++){
		if(ecalclustersH1[i].eta == ecalclustersH2[k].eta && ecalclustersH1[i].phi == ecalclustersH2[k].phi){
		if(ecalclustersH1[i].eta >= 9) { ecalclustersH1[i].energy = 0 ; }
		else {ecalclustersH2[k].energy = 0 ;}
		}
	}
}

for(p2rctIP1::loop i=0; i<p2rctIP1::N_CLUSTERS; i++){
	for(p2rctIP1::loop k=0; k<p2rctIP1::N_CLUSTERS; k++){
		if(ecalclustersH3[i].eta == ecalclustersH2[k].eta && ecalclustersH3[i].phi == ecalclustersH2[k].phi){
			if(ecalclustersH3[i].eta <= 15) { ecalclustersH3[i].energy = 0 ; }
		else {ecalclustersH2[k].energy = 0 ;}
		}
	}
}

// unique regions are combined 
//
for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23-2; i++){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
		ecalcrystals[i][j] = ecalcrystalsH1[i][j] ;
	}
}

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23-4; i++){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
		ecalcrystals[p2rctIP1::CRYSTALS_IN_ETA23-2+i][j] = ecalcrystalsH2[i+2][j] ;
	}
}

for(p2rctIP1::loop i=0; i<p2rctIP1::CRYSTALS_IN_ETA23-2; i++){
	for(p2rctIP1::loop j=0; j<p2rctIP1::CRYSTALS_IN_PHI; j++){
		ecalcrystals[p2rctIP1::CRYSTALS_IN_ETA23+5+i][j] = ecalcrystalsH3[i+2][j] ;
	}
}

createTowers(ecalcrystals, ecaltowers) ;

/*---------------------------------link 0------------------------------------*/
        
link_out[0] = 0;
link_out[1] = 0;

processOutputLinks(ecalclustersH1, ecalclustersH2, ecalclustersH3, ecaltowers, link_out);

}

//////////////////////////////////////////////////////////////////////////

// Declare the Phase2L1CaloL1RCTEmulator class and its methods

class Phase2L1CaloL1RCTEmulator : public edm::stream::EDProducer<> {
public:
  explicit Phase2L1CaloL1RCTEmulator(const edm::ParameterSet&);
  ~Phase2L1CaloL1RCTEmulator() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions&);

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  edm::EDGetTokenT<EcalEBTrigPrimDigiCollection> ecalTPEBToken_;
  edm::ESGetToken<CaloTPGTranscoder, CaloTPGRecord> decoderTag_;

  edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeometryTag_;
  const CaloSubdetectorGeometry* ebGeometry;
};

//////////////////////////////////////////////////////////////////////////

// Phase2L1CaloL1RCTEmulator initializer, destructor, and produce methods

Phase2L1CaloL1RCTEmulator::Phase2L1CaloL1RCTEmulator(const edm::ParameterSet& iConfig)
    : ecalTPEBToken_(consumes<EcalEBTrigPrimDigiCollection>(iConfig.getParameter<edm::InputTag>("ecalTPEB"))),
      decoderTag_(esConsumes<CaloTPGTranscoder, CaloTPGRecord>(edm::ESInputTag("", ""))),
      caloGeometryTag_(esConsumes<CaloGeometry, CaloGeometryRecord>(edm::ESInputTag("", ""))) {
  produces<l1tp2::rctIP1OutputLinkCollection>("LinkOut");
}

void Phase2L1CaloL1RCTEmulator::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  // Output collections
  std::unique_ptr<l1tp2::rctIP1OutputLinkCollection> link_out(make_unique<l1tp2::rctIP1OutputLinkCollection>());

  // Detector geometry
  const auto& caloGeometry = iSetup.getData(caloGeometryTag_);
  ebGeometry = caloGeometry.getSubdetectorGeometry(DetId::Ecal, EcalBarrel);

  // const auto& decoder = iSetup.getData(decoderTag_);

  //***************************************************//
  // Declare RCT output collections
  //***************************************************//

  auto L1EGXtalClusters = std::make_unique<l1tp2::CaloCrystalClusterCollection>();
  auto L1CaloTowers = std::make_unique<l1tp2::CaloTowerCollection>();

  //***************************************************//
  // Get the ECAL hits
  //***************************************************//
  edm::Handle<EcalEBTrigPrimDigiCollection> pcalohits;
  iEvent.getByToken(ecalTPEBToken_, pcalohits);

  std::vector<p2eg::SimpleCaloHit> ecalhits;

  for (const auto& hit : *pcalohits.product()) {
    if (hit.encodedEt() > 0)  // hit.encodedEt() returns an int corresponding to 2x the crystal Et
    {
      // Et is 10 bit, by keeping the ADC saturation Et at 120 GeV it means that you have to multiply by 0.125 (input LSB)
      float et = hit.encodedEt() * 0.125;
      if (et < p2eg::cut_500_MeV) {
        continue;  // Reject hits with < 500 MeV ET
      }

      // Get cell coordinates and info
      auto cell = ebGeometry->getGeometry(hit.id());

      p2eg::SimpleCaloHit ehit;
      ehit.setId(hit.id());
      std::cout << "Trying to show positions in phi, eta, r space" << std::endl;
      std::cout << "Phi: " << cell->getPosition().phi() << std::endl;
      std::cout << "Eta: " << cell->getPosition().eta() << std::endl;
      // std::cout << "R: " << cell->getPosition().R() << std::endl;
      std::cout << "End of cell" << std::endl;
      ehit.setPosition(GlobalVector(cell->getPosition().x(), cell->getPosition().y(), cell->getPosition().z()));
      ehit.setEnergy(et);
      ehit.setEt_uint(
          (ap_uint<10>)hit.encodedEt() >>
          2);  // also save the uint Et, this is to convert between 0.125 (in MC production) and 0.5 (in firmware based code)
      ehit.setPt();
      ecalhits.push_back(ehit);
    }
  }

  // Here, need to separate out the hits into the 5x6 and 2x6 areas

  // std::vector<ap_uint<576>> link_out[p2rctIP1::N_OUTPUT_LINKS];

  // In here, transform ECAL hits to input link format, then apply p2rctIP1::algo_top using link_out

  // iEvent.put(std::move(link_out), "LinkOut");
}

//////////////////////////////////////////////////////////////////////////

void Phase2L1CaloL1RCTEmulator::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  // l1tPhase2L1CaloL1RCTEmulator
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("ecalTPEB", edm::InputTag("simEcalEBTriggerPrimitiveDigis"));
  desc.add<edm::InputTag>("hcalTP", edm::InputTag("simHcalTriggerPrimitiveDigis"));
  {
    edm::ParameterSetDescription psd0;
    psd0.add<std::vector<double>>("etaBins",
                                  {
                                      0.087,
                                      0.174,
                                      0.261,
                                      0.348,
                                      0.435,
                                      0.522,
                                      0.609,
                                      0.696,
                                      0.783,
                                      0.87,
                                      0.957,
                                      1.044,
                                      1.131,
                                      1.218,
                                      1.305,
                                      1.392,
                                      1.479,
                                  });
    psd0.add<std::vector<double>>("ptBins",
                                  {
                                      12,
                                      20,
                                      30,
                                      40,
                                      55,
                                      90,
                                      1000000.0,
                                  });
    psd0.add<std::vector<double>>("scale",
                                  {
                                      1.298,  1.287,
                                      1.309,  1.298,
                                      1.309,  1.309,
                                      1.309,  1.298,
                                      1.309,  1.298,
                                      1.309,  1.309,
                                      1.309,  1.32,
                                      1.309,  1.32,
                                      1.309,  1.1742,
                                      1.1639, 1.1639,
                                      1.1639, 1.1639,
                                      1.1639, 1.1639,
                                      1.1742, 1.1742,
                                      1.1639, 1.1639,
                                      1.1742, 1.1639,
                                      1.1639, 1.1742,
                                      1.1742, 1.1536000000000002,
                                      1.11,   1.11,
                                      1.11,   1.11,
                                      1.11,   1.11,
                                      1.11,   1.11,
                                      1.11,   1.11,
                                      1.11,   1.11,
                                      1.11,   1.11,
                                      1.11,   1.11,
                                      1.1,    1.09,
                                      1.09,   1.09,
                                      1.09,   1.09,
                                      1.09,   1.09,
                                      1.09,   1.09,
                                      1.09,   1.09,
                                      1.09,   1.09,
                                      1.09,   1.09,
                                      1.09,   1.09,
                                      1.07,   1.07,
                                      1.07,   1.07,
                                      1.07,   1.07,
                                      1.07,   1.08,
                                      1.07,   1.07,
                                      1.08,   1.08,
                                      1.07,   1.08,
                                      1.08,   1.08,
                                      1.08,   1.06,
                                      1.06,   1.06,
                                      1.06,   1.05,
                                      1.05,   1.06,
                                      1.06,   1.06,
                                      1.06,   1.06,
                                      1.06,   1.06,
                                      1.06,   1.06,
                                      1.06,   1.06,
                                      1.04,   1.04,
                                      1.04,   1.04,
                                      1.05,   1.04,
                                      1.05,   1.05,
                                      1.05,   1.05,
                                      1.05,   1.05,
                                      1.05,   1.05,
                                      1.05,   1.05,
                                      1.05,
                                  });
    desc.add<edm::ParameterSetDescription>("calib", psd0);
  }
  descriptions.addWithDefaultLabel(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(Phase2L1CaloL1RCTEmulator);
