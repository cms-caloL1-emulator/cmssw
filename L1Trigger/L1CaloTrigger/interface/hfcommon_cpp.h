#ifndef L1Trigger_L1CaloTrigger_hfcommon_cpp_h
#define L1Trigger_L1CaloTrigger_hfcommon_cpp_h


#include "algo_topIP1.h"

namespace p2hfIP1 {

//
    stower  bestOf2(const stower& ecaltp0, const stower& ecaltp1) {
        stower x;
        x = (ecaltp0.energy > ecaltp1.energy)?ecaltp0:ecaltp1;
        return x;
    }

    
    hftower  bestOf2(const hftower& ecaltp0, const hftower& ecaltp1) {
        hftower x;
        x = (ecaltp0.energy > ecaltp1.energy)?ecaltp0:ecaltp1;
        return x;
    }

void findmaxof10(hftower towers[TEN], hftower &MaxTEN){

	hftower Step1[5] ;
	hftower Step2[2] ;

	   for(loop k=0; k<TEN; k=k+2){
        Step1[k>>1]  = bestOf2(towers[k], towers[k+1]) ;
        }

	   for(loop k=0; k<4; k=k+2){
        Step2[k>>1]  = bestOf2(Step1[k], Step1[k+1]) ;
        }

        hftower x1  = bestOf2(Step2[0], Step2[1]) ;

	MaxTEN  = bestOf2(x1,Step1[4]) ;
}

void findmaxPhi(stower stowers[STOWERS_PHI], stower &MaxPhi){

	stower Step1[12] ;
	stower Step2[6] ;
	stower Step3[3] ;

	   for(loop k=0; k<STOWERS_PHI; k=k+2){
        Step1[k>>1]  = bestOf2(stowers[k], stowers[k+1]) ;
        }

	   for(loop k=0; k<12; k=k+2){
        Step2[k>>1]  = bestOf2(Step1[k], Step1[k+1]) ;
        }
 
	   for(loop k=0; k<6; k=k+2){
        Step3[k>>1]  = bestOf2(Step2[k], Step2[k+1]) ;
	}

        stower x1 = bestOf2(Step3[0], Step3[1]) ;

	MaxPhi  = bestOf2(x1,Step3[2]) ;
}

void findmaxPhiJ(stower stowers[STOWERS_PHI+4], stower &MaxPhi){

	stower Step1[14] ;
	stower Step2[7] ;
	stower Step3[3] ;

	   for(loop k=0; k<STOWERS_PHI+4; k=k+2){
        Step1[k>>1]  = bestOf2(stowers[k], stowers[k+1]) ;
        }

	   for(loop k=0; k<14; k=k+2){
        Step2[k>>1]  = bestOf2(Step1[k], Step1[k+1]) ;
        }
 
	   for(loop k=0; k<6; k=k+2){
        Step3[k>>1]  = bestOf2(Step2[k], Step2[k+1]) ;
	}

        stower x1 = bestOf2(Step3[0], Step3[1]) ;
        stower x2 = bestOf2(Step3[2], Step2[6]) ;

	MaxPhi  = bestOf2(x1,x2) ;
}

void findmaxEta(stower stowers[STOWERS_ETA], stower &MaxEta){

	stower Step1[2] ;

	   for(loop k=0; k<STOWERS_ETA; k=k+2){
        Step1[k>>1]  = bestOf2(stowers[k], stowers[k+1]) ;
        }

        MaxEta = bestOf2(Step1[0], Step1[1]) ;
}

void findmaxEtaJ(stower stowers[STOWERS_ETA], stower &MaxEta){

	stower Step1[2] ;

	   for(loop k=0; k<STOWERS_ETA; k=k+2){
        Step1[k>>1]  = bestOf2(stowers[k], stowers[k+1]) ;
        }

        MaxEta = bestOf2(Step1[0], Step1[1]) ;
}

void getseedpositionPF(hftower towers[N_HF_TOWERS_REGION_ETA+4][N_HF_TOWERS_REGION_PHI+4], hftower &Seed){

        hftower EtaSlicesOUT[N_HF_TOWERS_REGION_ETA+4] ;

        for(loop i=0; i<N_HF_TOWERS_REGION_ETA+4; i++){
        hftower towers1D[N_HF_TOWERS_REGION_PHI+4] ;
                        for(loop j=0; j<N_HF_TOWERS_REGION_PHI+4; j++){
                        hftower tmp = towers[i][j] ;
//cout << " towers " << towers[i][j].Energy() << " eta " << i << " phi " << j << endl ;
                        towers1D[j] = tmp ;
                        }
        findmaxof10(towers1D, EtaSlicesOUT[i]) ;
        }

        findmaxof10(EtaSlicesOUT, Seed) ;

}

void getseedposition(stower stowers[STOWERS_ETA][STOWERS_PHI+4], stower &Seed){

        stower EtaSlicesOUT[STOWERS_ETA] ;

        for(loop i=0; i<STOWERS_ETA; i++){

        stower towers1D[STOWERS_PHI] ;
                        for(loop j=0; j<STOWERS_PHI; j++){
                        stower tmp = stowers[i][j+2] ;
                        towers1D[j] = tmp ;
                        }
        findmaxPhi(towers1D, EtaSlicesOUT[i]) ;
        }

        findmaxEta(EtaSlicesOUT, Seed) ;
}

void getseedpositionJet(stower stowers[STOWERS_ETA+4][STOWERS_PHI+4], stower &Seed){

        stower EtaSlicesOUT[STOWERS_ETA] ;

        for(loop i=0; i<STOWERS_ETA; i++){
        stower towers1D[STOWERS_PHI+4] ;
                        for(loop j=0; j<STOWERS_PHI+4; j++){
                        stower tmp = stowers[i+2][j] ;
                        towers1D[j] = tmp ;
                        }
        findmaxPhiJ(towers1D, EtaSlicesOUT[i]) ;
        }

        findmaxEtaJ(EtaSlicesOUT, Seed) ;
}

void getclusterPF(hftower towers[N_HF_TOWERS_REGION_ETA+4][N_HF_TOWERS_REGION_PHI+4], hftower Seed, PFcluster &output) {

	towermask mask[N_HF_TOWERS_REGION_ETA+4+2][N_HF_TOWERS_REGION_PHI+4+2] ;

	ap_uint<5> eta = Seed.eta ;
	ap_uint<7> phi = Seed.phi ;

        for(loop i=0; i<N_HF_TOWERS_REGION_ETA+4; i++){
        if(i >= eta && i <= eta+2){
        for(loop j=0; j<N_HF_TOWERS_REGION_PHI+4; j++){
        if(j >= phi && j <= phi+2 ) mask[i][j].energy =1 ;
        }}}

        ap_uint<12> tmpValue = 0  ;

           for(loop i=0; i<N_HF_TOWERS_REGION_ETA+4; i++){
           	for(loop j=0; j<N_HF_TOWERS_REGION_PHI+4; j++){
        	ap_uint<12> energytmp = towers[i][j].energy ;
        	ap_uint<12> energy = energytmp *  mask[i+1][j+1].energy ;
        	ap_uint<12> tmp = tmpValue + energy ;
        	tmpValue = tmp  ;
		}}

//cout << " tmp " << tmpValue << " eta " << Seed.eta << " phi " <<  Seed.phi <<  endl ;
	if(Seed.eta < 2 || Seed.eta > 7 || Seed.phi < 2 || Seed.phi > 7) tmpValue  = 0 ;
//cout << " tmp1 " << tmpValue << endl ;
	output.energy = tmpValue ;
	output.eta = Seed.eta ;
	output.phi = Seed.phi ;
 }

void zerrotowersPF(hftower towers[N_HF_TOWERS_REGION_ETA+4][N_HF_TOWERS_REGION_PHI+4], hftower Seed) {

	towermask mask[N_HF_TOWERS_REGION_ETA+4+2][N_HF_TOWERS_REGION_PHI+4+2] ;
	towermask maskI[N_HF_TOWERS_REGION_ETA+4+2][N_HF_TOWERS_REGION_PHI+4+2] ;
\
	ap_uint<5> eta = Seed.eta ;
	ap_uint<7> phi = Seed.phi ;


        for(loop i=0; i<N_HF_TOWERS_REGION_ETA+4; i++){
	if(i+1 >= eta && i <= eta+1){
        for(loop j=0; j<N_HF_TOWERS_REGION_PHI+4; j++){
	if(j+1 >= phi && j <= phi+1 ) mask[i+1][j+1].energy =1 ;
	}}}

        	for(loop i=0; i<N_HF_TOWERS_REGION_ETA+4+2; i++){
        	for(loop j=0; j<N_HF_TOWERS_REGION_PHI+4+2; j++){
        	maskI[i][j].energy = ((ap_uint<1>)1 - mask[i][j].energy) ;
         	}}

         for(loop i=0; i<N_HF_TOWERS_REGION_ETA+4; i++){
           for(loop j=0; j<N_HF_TOWERS_REGION_PHI+4; j++){
		ap_uint<12> tmp = towers[i][j].energy ;
		ap_uint<12> energy = tmp * maskI[i+1][j+1].energy  ;
        	towers[i][j].energy = energy ;
		}}

}

void getPFClusters(hftower towers[N_HF_TOWERS_REGION_ETA+4][N_HF_TOWERS_REGION_PHI+4], PFcluster pfclusters[N_HF_PFCLUSTERS_REGION]) {

hftower  Seed ;

        getseedpositionPF(towers, Seed) ;
//cout << " Seed " << Seed.energy << " eta " << Seed.eta << " phi " << Seed.phi <<  endl ;

	getclusterPF(towers,Seed,pfclusters[0]) ;
	zerrotowersPF(towers, Seed) ;

        getseedpositionPF(towers, Seed) ;
//cout << " Seed " << Seed.energy << " eta " << Seed.eta << " phi " << Seed.phi <<  endl ;
	getclusterPF(towers,Seed,pfclusters[1]) ;
	zerrotowersPF(towers, Seed) ;

        getseedpositionPF(towers, Seed) ;
//cout << " Seed " << Seed.energy << " eta " << Seed.eta << " phi " << Seed.phi <<  endl ;
	getclusterPF(towers,Seed,pfclusters[2]) ;
	zerrotowersPF(towers, Seed) ;

        getseedpositionPF(towers, Seed) ;
//cout << " Seed " << Seed.energy << " eta " << Seed.eta << " phi " << Seed.phi <<  endl ;
	getclusterPF(towers,Seed,pfclusters[3]) ;

}

void createSums(hftower towers[TOWERS_ETA][TOWERS_PHI+4] , sums& sum) {

	ap_uint<14> energy[TOWERS_ETA] ;

         for(loop i=0; i<TOWERS_ETA; i++){
		energy[i] = 0 ;
           for(loop j=2; j<TOWERS_PHI+2; j++){
		energy[i] += towers[i][j].energy ;
		}} 

	ap_uint<16> energySum = 0 ;
         for(loop i=0; i<TOWERS_ETA; i++){
		energySum += energy[i] ;
		} 
	if( energySum >= 4095 ) energySum = 4095 ;
	sum.energy = energySum ;

	}

void gettau(stower stowers[STOWERS_ETA][STOWERS_PHI+4], stower Seed, tau &output) {

	towermask mask[STOWERS_ETA][STOWERS_PHI] ;
	towermask maskDel[STOWERS_ETA][STOWERS_PHI] ;

	ap_uint<4> eta = Seed.eta ;
	ap_uint<5> phi = Seed.phi ;

        for(loop i=0; i<STOWERS_ETA; i++){
        for(loop j=0; j<STOWERS_PHI; j++){
        if(i == eta && j+2 == phi) {mask[i][j].energy =1 ; maskDel[i][j].energy = 0; }
	else maskDel[i][j].energy =1 ;
        }}

        ap_uint<12> tmpValue = 0  ;

           for(loop i=0; i<STOWERS_ETA; i++){
           	for(loop j=0; j<STOWERS_PHI; j++){
        	ap_uint<12> energytmp = stowers[i][j+2].energy ;
        	ap_uint<12> energy = energytmp *  mask[i][j].energy ;
        	ap_uint<12> tmp = tmpValue + energy ;
        	tmpValue = tmp  ;
        	stowers[i][j+2].energy = energytmp * maskDel[i][j].energy ;
		}}

	output.energy = tmpValue ;
	output.eta = Seed.eta ;
	output.phi = Seed.phi-2 ;
}

void createTaus(stower stowers[STOWERS_ETA][STOWERS_PHI+4], tau taus[N_TAUS]) {

stower  Seed ;

        getseedposition(stowers, Seed) ;
	gettau(stowers,Seed,taus[0]) ;

        getseedposition(stowers, Seed) ;
	gettau(stowers,Seed,taus[1]) ;

        getseedposition(stowers, Seed) ;
	gettau(stowers,Seed,taus[2]) ;

        getseedposition(stowers, Seed) ;
	gettau(stowers,Seed,taus[3]) ;

        getseedposition(stowers, Seed) ;
	gettau(stowers,Seed,taus[4]) ;

        getseedposition(stowers, Seed) ;
	gettau(stowers,Seed,taus[5]) ;
}

void getJet(stower stowers[STOWERS_ETA+4][STOWERS_PHI+4], stower Seed, jet &output) {

	towermask mask[STOWERS_ETA+4][STOWERS_PHI+4] ;

	ap_uint<4> eta = Seed.eta+2 ;
	ap_uint<5> phi = Seed.phi ;

        for(loop i=0; i<STOWERS_ETA+4; i++){
        if(i+1 >= eta && i <= eta+1){
        for(loop j=0; j<STOWERS_PHI+4; j++){
        if(j+1 >= phi && j <= phi+1 ) mask[i][j].energy =1 ;
        }}}

        ap_uint<12> tmpValue = 0  ;

           for(loop i=0; i<STOWERS_ETA+4; i++){
           	for(loop j=0; j<STOWERS_PHI+4; j++){
//	cout << " jet st value " << stowers[i][j].energy << " mask " << mask[i][j].energy << " i " << i << " j " << j << endl ;
        	ap_uint<12> energytmp = stowers[i][j].energy ;
        	ap_uint<12> energy = energytmp *  mask[i][j].energy ;
        	ap_uint<12> tmp = tmpValue + energy ;
        	tmpValue = tmp  ;
		}}

//	cout << " jet value " << tmpValue << endl ;
	if(eta < 2 || eta > 5 || phi < 2 || phi > 25) tmpValue  = 0 ;
	output.energy = tmpValue ;
	output.eta = Seed.eta ;
	output.phi = Seed.phi ;
 }

void zerrostowersJet(stower stowers[STOWERS_ETA+4][STOWERS_PHI+4], stower Seed) {

	towermask mask[STOWERS_ETA+4][STOWERS_PHI+4] ;
	towermask maskI[STOWERS_ETA+4][STOWERS_PHI+4] ;

	ap_uint<4> eta = Seed.eta+2 ;
	ap_uint<5> phi = Seed.phi ;


        for(loop i=0; i<STOWERS_ETA+4; i++){
	if(i+1 >= eta && i <= eta+1){
        for(loop j=0; j<STOWERS_PHI+4; j++){
	if(j+1 >= phi && j <= phi+1 ) mask[i][j].energy =1 ;
	}}}

        	for(loop i=0; i<STOWERS_ETA+4; i++){
        	for(loop j=0; j<STOWERS_PHI+4; j++){
        	maskI[i][j].energy = ((ap_uint<1>)1 - mask[i][j].energy) ;
         	}}

         for(loop i=0; i<STOWERS_ETA+4; i++){
           for(loop j=0; j<STOWERS_PHI+4; j++){
		ap_uint<12> tmp = stowers[i][j].energy ;
		ap_uint<12> energy = tmp * maskI[i][j].energy  ;
        	stowers[i][j].energy = energy ;
		}}

}

void createJets(stower stowers[STOWERS_ETA][STOWERS_PHI+4], jet jets[N_JETS]) {

        stower stowersExtended[STOWERS_ETA+4][STOWERS_PHI+4] ;

         for(loop i=0; i<STOWERS_ETA; i++){
           for(loop j=0; j<STOWERS_PHI+4; j++){
		stowersExtended[i+2][j] = stowers[i][j] ;
	}}

stower  Seed ;

        getseedpositionJet(stowersExtended, Seed) ;
	getJet(stowersExtended,Seed,jets[0]) ;
	zerrostowersJet(stowersExtended, Seed) ;

        getseedpositionJet(stowersExtended, Seed) ;
	getJet(stowersExtended,Seed,jets[1]) ;
	zerrostowersJet(stowersExtended, Seed) ;

        getseedpositionJet(stowersExtended, Seed) ;
	getJet(stowersExtended,Seed,jets[2]) ;
	zerrostowersJet(stowersExtended, Seed) ;

        getseedpositionJet(stowersExtended, Seed) ;
	getJet(stowersExtended,Seed,jets[3]) ;
	zerrostowersJet(stowersExtended, Seed) ;

        getseedpositionJet(stowersExtended, Seed) ;
	getJet(stowersExtended,Seed,jets[4]) ;
	zerrostowersJet(stowersExtended, Seed) ;

        getseedpositionJet(stowersExtended, Seed) ;
	getJet(stowersExtended,Seed,jets[5]) ;

}
 
}

#endif