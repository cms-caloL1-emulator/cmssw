/* 
 * Description: Phase 2 HF Emulator
 */

// system include files
//#include <ap_int.h>
#include <cmath>
// #include <cstdint>
#include <cstdlib>  // for rand
#include <iostream>
#include <fstream>
#include <memory>

#include <bitset>

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


// HF CMSSW - firmware interface
#include "DataFormats/HcalDigi/interface/HcalDigiCollections.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/HF_output.h"


//HF IP1 header files and data formats
#include "L1Trigger/L1CaloTrigger/interface/HF_IP1/algo_topIP1_h.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP1/algo_topIP1_cpp.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP1/hfcommon_cpp.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP1/bubl_sorter_h.h"

//HF IP2 header files and data formats
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/algo_topIP2_h.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/algo_topIP2_cpp.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/common/bit_encoding.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/common/datatypes.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/common/hls_reg.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/common/layer1_multiplicities.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/common/layer1_objs.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/common/pairwise_sum_reduce.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/common/pf.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/common/puppi.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/common/firmware/linpuppi.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/common/firmware/linpuppi_bits.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP2/common/firmware/linpuppi_cpp.h"
////////////////////////////////////////////////////////////////////////////////

// Declare the Phase2L1CaloL1HFEmulator class and its methods 




class Phase2L1CaloL1HFEmulator : public edm::stream::EDProducer<> {
public:
  explicit Phase2L1CaloL1HFEmulator(const edm::ParameterSet&);
  ~Phase2L1CaloL1HFEmulator() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions&);

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  edm::EDGetTokenT<HcalTrigPrimDigiCollection> hfToken_;
};


////////////////////////////////////////////////////////////////////////////////


//Phase2L1CaloL1HFEmulator intializer, destructor, and produce methods

Phase2L1CaloL1HFEmulator::Phase2L1CaloL1HFEmulator(const edm::ParameterSet& iConfig)
    : hfToken_(consumes<HcalTrigPrimDigiCollection>(iConfig.getParameter<edm::InputTag>("hcalDigis"))) {
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP1Ch0");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP1Ch1");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP1Ch2");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP1Ch3");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP1Ch4");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP1Ch5");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP1Ch6");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP1Ch7");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP1Ch8");
  
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP2Ch0");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP2Ch1");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP2Ch2");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP2Ch3");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP2Ch4");
  produces<l1tp2::hfOutputLinkCollection>("LinkOutIP2Ch5");
}


void Phase2L1CaloL1HFEmulator::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  // Initialize the 18 input links for the HF emulator
  ap_uint<576> link_in[N_INPUT_LINKS];
  for (int i = 0; i < N_INPUT_LINKS; i++) {
    link_in[i] = 0;
  }

  edm::Handle<HcalTrigPrimDigiCollection> hfHandle;
  iEvent.getByToken(hfToken_, hfHandle);

  ap_uint<10> tp[TOWERS_ETA][TOWERS_PHI] = {};
  for (const auto& hit : *hfHandle.product()) {
    int ieta = hit.id().ieta();
    int iphi = hit.id().iphi();
    if (ieta < 30 || ieta > 41) continue; // only HF
    if (hit.id().version() != 1) continue; // only version 1
    tp[ieta - 30][iphi - 1] = ap_uint<10>(hit.SOI_compressedEt()) | (ap_uint<10>(hit.SOI_fineGrain(0)) << 8) |
                              (ap_uint<10>(hit.SOI_fineGrain(1)) << 9); // get the 10-bit energy value
  }

  // link inde = 3 * sector + chunk, with per-link bitfields [A(0...109), B(110...219)]
  for (int sector = 0; sector < N_INPUT_LINKS / 3; ++sector) {
    for (int i = 0; i < TOWERS_ETA - 2; ++i) {
      const ap_uint<10> startA = i * 10;
      const ap_uint<10> endA = startA + 9;
      const ap_uint<10> startB = startA + 110;
      const ap_uint<10> endB = startB + 9;

      for (int chunk = 0; chunk < 3; ++chunk) {
        const int linkIdx = 3 * sector + chunk;
        const int phiBase = 12 * sector + 4 * chunk;
        link_in[linkIdx].range(endA, startA) = tp[i][phiBase];       // A side
        link_in[linkIdx].range(endB, startB) = tp[i][phiBase + 2];  // B side
      }
    }

    for (int chunk = 0; chunk < 3; ++chunk) {
      const int linkIdx = 3 * sector + chunk;
      const int phiBase = 12 * sector + 4 * chunk;
      link_in[linkIdx].range(109, 100) = tp[10][phiBase];  // row 10, A side
      link_in[linkIdx].range(219, 210) = tp[11][phiBase];  // row 11, B side
    }
  }

  
  /////////////// Run the HF Emulator firmware ///////////////////////////

  std::cout << "Starting the HF Emulator..." << std::endl;

  // Run algo_top (firmware code)
  ap_uint<576> link_out_ip1[N_OUTPUT_LINKS_CL1 + N_OUTPUT_LINKS_MIX];
  algo_topIP1(link_in, link_out_ip1);

  ap_uint<576> link_in_ip2[N_HF_REGIONS];
  ap_uint<576> link_out_ip2[N_HF_REGIONS];

  // Copy the output of IP1 to the input of IP2
  for (int i = 0; i < N_HF_REGIONS; i++) {
    link_in_ip2[i] = link_out_ip1[i];
  }
  // Run the IP2 emulator
  algo_topIP2(link_in_ip2, link_out_ip2);


  // put into output collections
  auto link_out_ip1_0 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip1_1 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip1_2 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip1_3 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip1_4 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip1_5 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip1_6 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip1_7 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip1_8 = std::make_unique<l1tp2::hfOutputLinkCollection>();

  auto link_out_ip2_0 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip2_1 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip2_2 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip2_3 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip2_4 = std::make_unique<l1tp2::hfOutputLinkCollection>();
  auto link_out_ip2_5 = std::make_unique<l1tp2::hfOutputLinkCollection>();

  link_out_ip1_0->push_back(l1tp2::hfOutputLink(link_out_ip1[0]));
  link_out_ip1_1->push_back(l1tp2::hfOutputLink(link_out_ip1[1]));
  link_out_ip1_2->push_back(l1tp2::hfOutputLink(link_out_ip1[2]));
  link_out_ip1_3->push_back(l1tp2::hfOutputLink(link_out_ip1[3]));
  link_out_ip1_4->push_back(l1tp2::hfOutputLink(link_out_ip1[4]));
  link_out_ip1_5->push_back(l1tp2::hfOutputLink(link_out_ip1[5]));
  link_out_ip1_6->push_back(l1tp2::hfOutputLink(link_out_ip1[6]));
  link_out_ip1_7->push_back(l1tp2::hfOutputLink(link_out_ip1[7]));
  link_out_ip1_8->push_back(l1tp2::hfOutputLink(link_out_ip1[8]));

  link_out_ip2_0->push_back(l1tp2::hfOutputLink(link_out_ip2[0]));
  link_out_ip2_1->push_back(l1tp2::hfOutputLink(link_out_ip2[1]));
  link_out_ip2_2->push_back(l1tp2::hfOutputLink(link_out_ip2[2]));
  link_out_ip2_3->push_back(l1tp2::hfOutputLink(link_out_ip2[3]));
  link_out_ip2_4->push_back(l1tp2::hfOutputLink(link_out_ip2[4]));
  link_out_ip2_5->push_back(l1tp2::hfOutputLink(link_out_ip2[5]));

  iEvent.put(std::move(link_out_ip1_0), "LinkOutIP1Ch0");
  iEvent.put(std::move(link_out_ip1_1), "LinkOutIP1Ch1");
  iEvent.put(std::move(link_out_ip1_2), "LinkOutIP1Ch2");
  iEvent.put(std::move(link_out_ip1_3), "LinkOutIP1Ch3");
  iEvent.put(std::move(link_out_ip1_4), "LinkOutIP1Ch4");
  iEvent.put(std::move(link_out_ip1_5), "LinkOutIP1Ch5");
  iEvent.put(std::move(link_out_ip1_6), "LinkOutIP1Ch6");
  iEvent.put(std::move(link_out_ip1_7), "LinkOutIP1Ch7");
  iEvent.put(std::move(link_out_ip1_8), "LinkOutIP1Ch8");

  iEvent.put(std::move(link_out_ip2_0), "LinkOutIP2Ch0");
  iEvent.put(std::move(link_out_ip2_1), "LinkOutIP2Ch1");
  iEvent.put(std::move(link_out_ip2_2), "LinkOutIP2Ch2");
  iEvent.put(std::move(link_out_ip2_3), "LinkOutIP2Ch3");
  iEvent.put(std::move(link_out_ip2_4), "LinkOutIP2Ch4");
  iEvent.put(std::move(link_out_ip2_5), "LinkOutIP2Ch5");

}


///////////////////////////////////////////////////////////////////////////////////


void Phase2L1CaloL1HFEmulator::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("hcalDigis", edm::InputTag("simHcalTriggerPrimitiveDigis"));
  descriptions.addWithDefaultLabel(desc);
}


//define this as a plug-in
DEFINE_FWK_MODULE(Phase2L1CaloL1HFEmulator);