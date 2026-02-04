ifndef L1Trigger_L1CaloTrigger_algo_top_tblIP1_cpp_h
#define L1Trigger_L1CaloTrigger_algo_top_tblIP1_cpp_h


#include "algo_topIP1.h"
#include <cstdlib>

namespace p2hfIP1 {

int main(){

  srand((unsigned)time(0));

ap_uint<576> link_in[N_INPUT_LINKS] ;
ap_uint<576> link_out[N_OUTPUT_LINKS_CL1+N_OUTPUT_LINKS_MIX];

        for(loop j=0; j<N_INPUT_LINKS; j++){
          link_in[j] = 0 ;
	}
        for(loop j=0; j<N_OUTPUT_LINKS_CL1+N_OUTPUT_LINKS_MIX; j++){
          link_out[j] = 0 ;
	}


        for(loop j=0; j<1; j++){
        for(loop i=0; i<2; i++){
        ap_uint<10> start = i*10;
        ap_uint<10> end = start+9;
          link_in[j].range(end, start) = 2*i+10 ;
        }
        for(loop i=0; i<2; i++){
        ap_uint<10> start = i*10+110;
        ap_uint<10> end = start+9;
          link_in[j].range(end, start) = 2*i+50 ;
	}}

        for(loop i=0; i<2; i++){
        ap_uint<10> start = i*10;
        ap_uint<10> end = start+9;
          link_in[17].range(end, start) = 2*i+20 ;
        }
        for(loop i=0; i<2; i++){
        ap_uint<10> start = i*10+110;
        ap_uint<10> end = start+9;
          link_in[17].range(end, start) = 2*i+60 ;
	}

        for(loop i=7; i<8; i++){
        ap_uint<10> start = i*10;
        ap_uint<10> end = start+9;
          link_in[10].range(end, start) = 2*i+5 ;
        }
        for(loop i=7; i<8; i++){
        ap_uint<10> start = i*10+110;
        ap_uint<10> end = start+9;
          link_in[10].range(end, start) = 2*i+30 ;
	}


algo_topIP1(link_in, link_out) ;

       PFcluster cluster ;
//6 cluster, sums, taus, and jets
        for(loop i = 0; i < N_OUTPUT_LINKS_CL1 ; i++){
        for(loop j = 0; j < 8; j++){
        ap_uint<10> start = j*64;
        ap_uint<10> end = start+63;
        cluster.fillPFcluster(((ap_uint<64>) link_out[i].range(end, start))) ;
      cout << " region " << i << " cluster " << j << " energy " <<  cluster.Energy() << endl ;
        }}

        for(loop j = 0; j < 6; j++){
        ap_uint<10> start = j*64;
        ap_uint<10> end = start+63;
        cluster.fillPFcluster(((ap_uint<64>) link_out[6].range(end, start))) ;
      cout << " sums " <<  cluster.Energy() << endl ;
        }

        for(loop j = 0; j < 6; j++){
        ap_uint<10> start = j*64;
        ap_uint<10> end = start+63;
        cluster.fillPFcluster(((ap_uint<64>) link_out[7].range(end, start))) ;
      cout << " taus " <<  cluster.Energy() << endl ;
        }

        for(loop j = 0; j < 6; j++){
        ap_uint<10> start = j*64;
        ap_uint<10> end = start+63;
        cluster.fillPFcluster(((ap_uint<64>) link_out[8].range(end, start))) ;
      cout << " jets " <<  cluster.Energy() << endl ;
        }

        cout <<"link_in[0]" << std::hex <<link_in[0] << endl ;
		cout <<"link_in[10]" << std::hex <<link_in[10] << endl;
		cout <<"link_in[17]" << std::hex <<link_in[17] << endl;


		cout <<"link_out[0]" << std::hex <<link_out[0]<<endl;
		cout <<"link_out[1]" << std::hex <<link_out[1]<<endl;
		cout <<"link_out[2]" << std::hex <<link_out[2]<<endl;
		cout <<"link_out[3]" << std::hex <<link_out[3]<<endl;
		cout <<"link_out[4]" << std::hex <<link_out[4]<<endl;
		cout <<"link_out[5]" << std::hex <<link_out[5]<<endl;
		cout <<"link_out[6]" << std::hex <<link_out[6]<<endl;
		cout <<"link_out[7]" << std::hex <<link_out[7]<<endl;
		cout <<"link_out[8]" << std::hex <<link_out[8]<<endl;


  return 0;
}

}