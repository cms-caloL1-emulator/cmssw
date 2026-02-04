#ifndef L1Trigger_L1CaloTrigger_algo_topIP1_cpp_h
#define L1Trigger_L1CaloTrigger_algo_topIP1_cpp_h


#include "algo_topIP1.h"
#include "bitonicSort16.h"
#include "common/firmware/linpuppi.h"
#include "common/layer1_objs.h"



namespace p2hfIP2 {


inline void processInputLinks(ap_uint<576> link_in[N_INPUT_LINKS], hftower HFTowers[TOWERS_ETA][TOWERS_PHI+4]){
// -2 because last 2 are 4x towers 
// we have initally 10 A and 10 B towers that we need to split in 2 each
// and after that 11th A and 11th B we need to split in 4

    for(loop j=0; j<TOWERS_PHI; j=j+4){
    for(loop i=0; i<TOWERS_ETA-2; i++){
    ap_uint<10> start   = i*10;
    ap_uint<10> end = start + 9;
// j=0 and 1 we keep for overlap later
    ap_uint<10> tmp = link_in[j>>2].range(end, start) ;
    HFTowers[i][j+2].hftower1(tmp); // j/4 because we have 72 towers in 18 links
    HFTowers[i][j+3].hftower2(tmp);
  }}

    for(loop j=0; j<TOWERS_PHI; j=j+4){
    for(loop i=0; i<TOWERS_ETA-2; i++){
    ap_uint<10> start   = i*10;
    ap_uint<10> end = start + 9;
    ap_uint<10> tmp1 = link_in[j>>2].range(end+110, start+110) ;
    HFTowers[i][j+4].hftower1(tmp1);
    HFTowers[i][j+5].hftower2(tmp1);
  }}

    for(loop j=0; j<TOWERS_PHI; j=j+4){
    ap_uint<10> tmp = link_in[j>>2].range(109, 100) ;
    HFTowers[10][j+2].hftower11(tmp);
    HFTowers[10][j+3].hftower12(tmp);
    HFTowers[10][j+4].hftower13(tmp);
    HFTowers[10][j+5].hftower14(tmp);
  }
    for(loop j=0; j<TOWERS_PHI; j=j+4){
    ap_uint<10> tmp1 = link_in[j>>2].range(219, 210) ;
    HFTowers[11][j+2].hftower11(tmp1);
    HFTowers[11][j+3].hftower12(tmp1);
    HFTowers[11][j+4].hftower13(tmp1);
    HFTowers[11][j+5].hftower14(tmp1);
  }

// we create an array that has 2 extra phi's on each side for later overlap

 for(loop i=0; i<TOWERS_ETA; i++){
    HFTowers[i][0] = HFTowers[i][TOWERS_PHI];
    HFTowers[i][1] = HFTowers[i][TOWERS_PHI+1];
    HFTowers[i][TOWERS_PHI+2] = HFTowers[i][2];
    HFTowers[i][TOWERS_PHI+3] = HFTowers[i][3];
  }

}

void calculateST(hftower towers[TOWERS_ETA][TOWERS_PHI+4], stower stowers[STOWERS_ETA][STOWERS_PHI+4]) {
	for(loop i=0; i<STOWERS_ETA; i++){
		for(loop k=0; k<STOWERS_PHI; k++){
			ap_uint<12> tmp0 = towers[3*i][3*k+2].energy + towers[3*i+1][3*k+2].energy + towers[3*i+2][3*k+2].energy ;
			ap_uint<12> tmp1 = towers[3*i][3*k+3].energy + towers[3*i+1][3*k+3].energy + towers[3*i+2][3*k+3].energy ;
			ap_uint<12> tmp2 = towers[3*i][3*k+4].energy + towers[3*i+1][3*k+4].energy + towers[3*i+2][3*k+4].energy ;
			stowers[i][k+2].energy = tmp0 + tmp1 + tmp2 ;
			stowers[i][k+2].SeedEnergy = towers[3*i+1][3*k+3].energy;
			stowers[i][k+2].eta = i ;
			stowers[i][k+2].phi = k+2 ;
	}}

	for(loop i=0; i<STOWERS_ETA; i++){
		for(loop k=0; k<2; k++){
			stowers[i][k] = stowers[i][STOWERS_PHI+k] ;
			stowers[i][k].phi = k ;
			stowers[i][STOWERS_PHI+2+k] = stowers[i][k+2] ;
			stowers[i][STOWERS_PHI+2+k].phi = STOWERS_PHI+2+k ;
	}}

	//	for(loop i=0; i<STOWERS_ETA; i++){
	//#pragma HLS UNROLL
	//		for(loop k=0; k<STOWERS_PHI+4; k++){
	//#pragma HLS UNROLL
	//    cout << " stowers et " << stowers[i][k].energy << " eta " <<
	//    stowers[i][k].eta << " phi " <<
	//    stowers[i][k].phi << endl  ;
	//	}}
}
//	convert PFcluster to HadcaloObj for PUPPI function
namespace l1ct {
    HadCaloObj convertPFclusterToHadCaloObj(const PFcluster& pfCluster) {
        HadCaloObj hadCalo;
        hadCalo.hwPt = pfCluster.energy;
        hadCalo.hwEta = pfCluster.eta;
        hadCalo.hwPhi = pfCluster.phi;
        hadCalo.hwEmPt = pfCluster.ECAL;
        hadCalo.hwEmID = 0;
        hadCalo.hwSrrTot = 0;
        hadCalo.hwMeanZ = 0;
        hadCalo.hwHoe = pfCluster.hoe;
        return hadCalo;
    }
}

//convert PuppiObj to PFCluster
PFcluster convertPuppiToPF(const l1ct::PuppiObj &puppi) {
    PFcluster pf;

    pf.energy = puppi.hwPt;
    pf.eta    = puppi.hwEta;
    pf.phi    = puppi.hwPhi;
    pf.hoe    = 0; // needed
    pf.ECAL   = 0; // needed
    pf.HCAL   = 0;
    pf.spare  = 0;
    pf.data   = 0;
    return pf;
}

void fillOutputLink(ap_uint<576>& linkOut, PFcluster pfclusters[16]){
	linkOut = 0;
	ap_uint<10> start ;
	ap_uint<10> end ;

	for(loop j=15; j>=7; j--)
		{
		start=(15-j)*64 ; end=start+63;
		linkOut.range(end, start) = pfclusters[j].getPFcluster() ;
	    }
}


void copy_to_output(ap_uint<576> link_tmp[N_OUTPUT_LINKS_CL1+N_OUTPUT_LINKS_MIX], ap_uint<576> link_out1[N_OUTPUT_LINKS_CL1+N_OUTPUT_LINKS_MIX]){
	for(loop i=0; i<N_OUTPUT_LINKS_CL1+N_OUTPUT_LINKS_MIX; i++)
		link_out1[i]=link_tmp[i];
	}

// sync function to fix ap_vld synchronization
void sync_output_links(ap_uint<576> link_out1[N_OUTPUT_LINKS_CL1+N_OUTPUT_LINKS_MIX], ap_uint<576> link_out[N_OUTPUT_LINKS_CL1+N_OUTPUT_LINKS_MIX]) {

    for (int i = 0; i < N_OUTPUT_LINKS_CL1+N_OUTPUT_LINKS_MIX; i++) {
		link_out[i] = link_out1[i];
		}
	}

void algo_topIP1(ap_uint<576> link_in[N_INPUT_LINKS], ap_uint<576> link_out[N_OUTPUT_LINKS_CL1+N_OUTPUT_LINKS_MIX]){
	// we define the towers 12x72 eta phi and create 24 regions
	// 6x6 after adding 2 overlap becomes 10x10, where eta additions
	// and phi are from correspondent towers

	ap_uint<576> link_tmp[N_OUTPUT_LINKS_CL1+N_OUTPUT_LINKS_MIX] ;
	ap_uint<576> link_out1[N_OUTPUT_LINKS_CL1+N_OUTPUT_LINKS_MIX];
	hftower HFTowers[TOWERS_ETA][TOWERS_PHI+4] ;
	stower HFSTowers[STOWERS_ETA][STOWERS_PHI+4] ; // for jets and taus
	stower HFSTowersJets[STOWERS_ETA][STOWERS_PHI+4] ; // for jets and taus
	hfregion HFRegions[N_HF_REGIONS] ; // for PF clusters
	sums HFSums ; // only one variable for now

	processInputLinks(link_in, HFTowers) ;
	// we readout all links and have eta 12 x phi 72+4 grid of towers
	//std::cout << " Read HFTowera 5:0 " << HFTowers[5][0].energy << " 10:10 " << HFTowers[10][10].energy << endl ;

	createSums(HFTowers, HFSums) ;
	// in hfcommon
	//std::cout << " Read sum " << HFSums.energy << endl ;

	//	Initialize tmp link to zero.
	for(loop i=0; i<N_OUTPUT_LINKS_CL1+N_OUTPUT_LINKS_MIX; i++){
		link_tmp[i] = 0 ;
		}

	for(loop i=0; i<N_HF_REGIONS; i=i+2){
		// here we build the regions 2+6+2 in eta where 2 and 2 are 0s
		// and 2+6+2 in phi
		// within each region eta and phi run 0-9, but since in eta we have only 2 regions
		// to take into account the overlap we need to remember that eta=0 and 1 in region 0, 2, 4 etc is 0
		// while eta 8 and 9 are 0 in region 1, 3, 5 etc
		// lets fill 0s first with eta and phi coordinates

		for(loop k=0; k<2; k++) {
			for(loop m=0; m<N_HF_TOWERS_REGION_PHI+4; m++) {
				HFRegions[i].hftowers[k][m].eta = k ;
				HFRegions[i].hftowers[k][m].phi = m ;
				HFRegions[i+1].hftowers[k+N_HF_TOWERS_REGION_ETA+2][m].eta = k+N_HF_TOWERS_REGION_ETA+2 ;
				HFRegions[i+1].hftowers[k+N_HF_TOWERS_REGION_ETA+2][m].phi = m ;
		}}
		// fill et and eta phi

		for(loop k=0; k<N_HF_TOWERS_REGION_ETA+2; k++) {
			for(loop m=0; m<N_HF_TOWERS_REGION_PHI+4; m++) {
				HFRegions[i].hftowers[k+2][m].eta = k+2 ;
				HFRegions[i].hftowers[k+2][m].phi = m ;
				HFRegions[i].hftowers[k+2][m].energy = HFTowers[k][m+(i/2)*6].energy ;
				HFRegions[i].hftowers[k+2][m].fb = HFTowers[k][m+(i/2)*6].fb ;
				HFRegions[i+1].hftowers[k][m].eta = k ;
				HFRegions[i+1].hftowers[k][m].phi = m ;
				HFRegions[i+1].hftowers[k][m].energy = HFTowers[k+4][m+(i/2)*6].energy ;
				HFRegions[i+1].hftowers[k][m].fb = HFTowers[k+4][m+(i/2)*6].fb ;
				//cout << " i " << i << " k " << k << " m " << m << " HF tower i " << HFRegions[i].hftowers[k+2][m].Energy() << " i+1 " << HFRegions[i+1].hftowers[k][m].Energy() << endl ;
		}}
	}

	// PF clusters from towers
	for(loop i=0; i<N_HF_REGIONS; i++){
		getPFClusters(HFRegions[i].hftowers,HFRegions[i].pfclusters) ;
		}

// print pfclusters
//#ifndef __SYNTHESIS__
//#include <iostream>
//using namespace std;
//
//for(int i=0; i<N_HF_REGIONS; i++){
//    cout << "=== sub Region " << i << " ===" << endl;
//
//    // print output PFclusters
//    for(int j=0; j<N_HF_REGIONS; j++){  // adjust this constant to actual size
//        auto &cl = HFRegions[i].pfclusters[j];
//        cout << " PFcluster[" << j << "] "
//             << "E="     << (int)cl.energy
//             << " eta="  << (int)cl.eta
//             << " phi="  << (int)cl.phi
//            << endl;
//    }
//}
//#endif

//	declaring pfSelectedNeutrals
	l1ct::PuppiObj pfSelNe[N_SECTORS_PF][NNEUTRALS];
//=== define the  sector boundaries and overlaps ========================================
	l1ct::PFRegion region[N_SECTORS_PF];
	getPFClustringChain:
	for(int i=0 ; i < N_SECTORS_PF ; i++){
		region[i].hwEtaCenter = l1ct::glbeta_t(6);
		region[i].hwPhiCenter = l1ct::glbphi_t(12*i+5);

		region[i].hwEtaExtra = l1ct::eta_t(0);
		region[i].hwPhiExtra = l1ct::phi_t(2);

		region[i].hwEtaHalfWidth = l1ct::eta_t(7);
		region[i].hwPhiHalfWidth = l1ct::phi_t(6);

//            #ifndef __SYNTHESIS__
//            if(DEBUG_LEVEL > 9)
//                {
//                    std::cout<<"Sector "<<i<<" | eta center : "<<region[i].hwEtaCenter<<" , phi center : "<<region[i].hwPhiCenter
//                         <<" [ eta HW : "<<region[i].hwEtaHalfWidth<<" phi HW : "<< region[i].hwPhiHalfWidth  <<" ] "
//                         <<"\n";
//                }
//            #endif
	        }
//=======================================================================================
	l1ct::HadCaloObj hadCaloClusters[N_SECTORS_PF][NCALO];
	for (loop k=0; k<N_HF_REGIONS; k=k+4) {
	    for (loop i=0; i<4; i++) {
	        int sector = k >> 2;

	        hadCaloClusters[sector][i]      = l1ct::convertPFclusterToHadCaloObj(HFRegions[k].pfclusters[i]);
	        hadCaloClusters[sector][i + 4]  = l1ct::convertPFclusterToHadCaloObj(HFRegions[k+1].pfclusters[i]);
	        hadCaloClusters[sector][i + 8]  = l1ct::convertPFclusterToHadCaloObj(HFRegions[k+2].pfclusters[i]);
	        hadCaloClusters[sector][i + 12] = l1ct::convertPFclusterToHadCaloObj(HFRegions[k+3].pfclusters[i]);
	    }
	}

	#ifndef __SYNTHESIS__
	// Debug print BEFORE PUPPI
	std::cout << "=== PF Clusters to HadCaloObj ===" << std::endl;
	for (int i = 0; i < N_SECTORS_PF; i++) {
		std::cout << "--- Region " << i << " ---" << std::endl;
		for (int j = 0; j < NCALO; j++) {
			std::cout << " HadCaloClusters[" << j << "]: "
					  << "Pt="  << hadCaloClusters[i][j].hwPt
					  << " Eta=" << hadCaloClusters[i][j].hwEta
					  << " Phi=" << hadCaloClusters[i][j].hwPhi
					  << std::endl;
		}
	}
	#endif

//	PUPPI ALgorithm
	for (int i = 0; i < N_SECTORS_PF; i++) {
	    fwdlinpuppi(region[i], hadCaloClusters[i], pfSelNe[i]);
	}
	#ifndef __SYNTHESIS__
		for (int i = 0; i < N_SECTORS_PF; i++) {
			std::cout << "--- Region " << i << " ---" << std::endl;
			for (int j = 0; j < NNEUTRALS; j++) {
				std::cout << " PUPPI_obj[" << j << "]: "
				<< "Pt=" << pfSelNe[i][j].hwPt
				<< " Eta=" << pfSelNe[i][j].hwEta
				<< " Phi=" << pfSelNe[i][j].hwPhi << std::endl;
			}
		}
	#endif

	PFcluster pfclustersIn[N_SECTORS_PF][16] ;
	PFcluster pfclustersOut[N_SECTORS_PF][16] ;

	//convert PuppiObj to PFcluster
	for (int i = 0; i < N_SECTORS_PF; i++) {

//	    convert PUPPI obj to PFcluster
	    for (int j = 0; j < NNEUTRALS; j++) {
	        pfclustersIn[i][j] = convertPuppiToPF(pfSelNe[i][j]);

	#ifndef __SYNTHESIS__
	        std::cout << "Region " << i << " PFClustersIn " << j
	                  << " Pt="  << pfclustersIn[i][j].energy
	                  << " Eta=" << pfclustersIn[i][j].eta
	                  << " Phi=" << pfclustersIn[i][j].phi
	                  << std::endl;
	#endif
	    }

	    for (int j = NNEUTRALS; j < 16; j++) {
			pfclustersIn[i][j] = PFcluster();
	    	}
	    //Step:2 Bitonic sort 8 PFclusters
	   bitonicSort16(pfclustersIn[i], pfclustersOut[i]);

	#ifndef __SYNTHESIS__
	    std::cout << " Region " << i << " sorted 16 PFclustersOut:" << std::endl;
	    for (int j = 15; j >=0; j--) {
	        std::cout << "  PF[" << j << "]: "
	                  << "Pt="  << pfclustersOut[i][j].energy
	                  << " Eta=" << pfclustersOut[i][j].eta
	                  << " Phi=" << pfclustersOut[i][j].phi
	                  << std::endl;
	    }
	#endif

	    // Step 3: write to link
	    fillOutputLink(link_tmp[i], pfclustersOut[i]);

	}


	tau HFTaus[N_TAUS];
	jet HFJets[N_JETS];

	calculateST(HFTowers, HFSTowers) ;

	for(loop i=0; i<STOWERS_ETA; i++){
		for(loop k=0; k<STOWERS_PHI+4; k++){
			HFSTowersJets[i][k] = HFSTowers[i][k];
		}
	}

	createTaus(HFSTowers, HFTaus) ;
	createJets(HFSTowersJets, HFJets) ;

	ap_uint<10> start ;
	ap_uint<10> end ;

	for(loop j=0; j<6; j++){
		start=j*64 ; end=start+63; 
		link_tmp[6].range(end, start) = HFSums.getSum() ;
		link_tmp[7].range(end, start) = HFTaus[j].getTau() ;
		link_tmp[8].range(end, start) = HFJets[j].getJet() ;
		}

	copy_to_output(link_tmp, link_out1) ;
//#ifndef __SYNTHESIS__
//for (int j = 0; j < N_SECTORS_PF; j++) {
//    std::cout << "LINK_out1[" << j << "]: "
//              << link_out1[j].to_string(16)
//              << std::endl;
//}
//#endif
	sync_output_links(link_out1, link_out);
//#ifndef __SYNTHESIS__
//for (int j = 0; j < N_SECTORS_PF; j++) {
//    std::cout << "LINK_out[" << j << "]: "
//              << link_out[j].to_string(16)
//              << std::endl;
//}
//#endif
}

}

#endif