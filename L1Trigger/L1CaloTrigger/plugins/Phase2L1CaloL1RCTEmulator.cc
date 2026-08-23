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

// RCT CMSSW - firmware interface
#include "L1Trigger/L1CaloTrigger/interface/RCT_IO.h"

// RCT header files and data formats
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP1_5x6_h.h"
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP1_5x6_cpp.h"
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP1_2x6_h.h"
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP1_2x6_cpp.h"
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP21_h.h"
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP21_cpp.h"
#include "L1Trigger/L1CaloTrigger/interface/bubl_sorter_h.h"
#include "L1Trigger/L1CaloTrigger/interface/bubl_sorter_cpp.h"
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP22_h.h"
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP22_cpp.h"
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP3_h.h"
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP3_cpp.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/RCT_output.h"


// Adding the RCT IO Dump utlity header file
#include "L1Trigger/L1CaloTrigger/interface/RCT_IO_DumpUtils.h"
using namespace rctdump;

///////////////////////////////////////////////////////////////////////////////

// Declare the Phase2L1CaloL1RCTEmulator class and its methods

class Phase2L1CaloL1RCTEmulator : public edm::stream::EDProducer<> {
public:
  explicit Phase2L1CaloL1RCTEmulator(const edm::ParameterSet&);
  ~Phase2L1CaloL1RCTEmulator() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions&);

private:
  void produce(edm::Event&, const edm::EventSetup&) override;

  edm::EDGetTokenT<EcalEBTrigPrimDigiCollection> ecalTPEBToken_;
  edm::EDGetTokenT<edm::SortedCollection<HcalTriggerPrimitiveDigi>> hcalTPToken_;
  edm::EDGetTokenT<std::vector<reco::GenParticle>> genParticleToken_; // For Gen Particles

  edm::ESGetToken<CaloTPGTranscoder, CaloTPGRecord> decoderTag_;

  edm::ESGetToken<CaloGeometry, CaloGeometryRecord> caloGeometryTag_;
  const CaloSubdetectorGeometry* ebGeometry;
  const CaloSubdetectorGeometry* hbGeometry;
  edm::ESGetToken<HcalTopology, HcalRecNumberingRecord> hbTopologyTag_;
  const HcalTopology* hcTopology_;
};

//////////////////////////////////////////////////////////////////

// Phase2L1CaloL1RCTEmulator initializer, destructor, and produce methods

Phase2L1CaloL1RCTEmulator::Phase2L1CaloL1RCTEmulator(const edm::ParameterSet& iConfig)
    : ecalTPEBToken_(consumes<EcalEBTrigPrimDigiCollection>(iConfig.getParameter<edm::InputTag>("ecalTPEB"))),
      hcalTPToken_(consumes<edm::SortedCollection<HcalTriggerPrimitiveDigi>>(iConfig.getParameter<edm::InputTag>("hcalTP"))),
      genParticleToken_(consumes<std::vector<reco::GenParticle>>(iConfig.getParameter<edm::InputTag>("genParticles"))),
      decoderTag_(esConsumes<CaloTPGTranscoder, CaloTPGRecord>(edm::ESInputTag("", ""))),
      caloGeometryTag_(esConsumes<CaloGeometry, CaloGeometryRecord>(edm::ESInputTag("", ""))),
      hbTopologyTag_(esConsumes<HcalTopology, HcalRecNumberingRecord>(edm::ESInputTag("", ""))) {
  produces<l1tp2::rctOutputLinkCollection>("LinkOut0");
  produces<l1tp2::rctOutputLinkCollection>("LinkOut1");
  produces<l1tp2::rctOutputLinkCollection>("LinkOut2");
  produces<l1tp2::rctOutputLinkCollection>("LinkOut3");
}

void Phase2L1CaloL1RCTEmulator::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;

  // Output collections
  std::unique_ptr<l1tp2::rctOutputLinkCollection> link_out0(make_unique<l1tp2::rctOutputLinkCollection>());
  std::unique_ptr<l1tp2::rctOutputLinkCollection> link_out1(make_unique<l1tp2::rctOutputLinkCollection>());
  std::unique_ptr<l1tp2::rctOutputLinkCollection> link_out2(make_unique<l1tp2::rctOutputLinkCollection>());
  std::unique_ptr<l1tp2::rctOutputLinkCollection> link_out3(make_unique<l1tp2::rctOutputLinkCollection>());

  std::cout << "Starting the RCT Emulator..." << std::endl;

  const unsigned long long eventId = iEvent.id().event();


  // Detector geometry
  const auto& caloGeometry = iSetup.getData(caloGeometryTag_);
  ebGeometry = caloGeometry.getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  hbGeometry = caloGeometry.getSubdetectorGeometry(DetId::Hcal, HcalBarrel);

  const auto& hbTopology = iSetup.getData(hbTopologyTag_);
  hcTopology_ = &hbTopology;
  HcalTrigTowerGeometry theTrigTowerGeometry(hcTopology_);

  const auto& decoder = iSetup.getData(decoderTag_);

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

  // Initialize array of energies to all 0s
  float crystalEnergies[p2rctIO::N_CARDS][p2rctIO::N_TOWERS_ETA*p2rctIO::CRYSTALS_IN_TOWER_ETA][p2rctIO::N_TOWERS_PHI*p2rctIO::CRYSTALS_IN_TOWER_PHI] = {{{ 0 }}};

  for (const auto& hit : *pcalohits.product()) {
    if (hit.encodedEt() > 0)  // hit.encodedEt() returns an int corresponding to 2x the crystal Et
    {
      // Et is 10 bit, by keeping the ADC saturation Et at 120 GeV it means that you have to multiply by 0.125 (input LSB)
      float et = hit.encodedEt() * 0.125;
      // if (et < 0.5) {
      //   continue;  // Reject hits with < 500 MeV ET
      // }

      // std::cout << "pcalohit energy: " << et << std::endl;

      // Get cell coordinates and info
      auto cell = ebGeometry->getGeometry(hit.id());

      p2rctIO::SimpleCaloHit ehit;
      ehit.setPosition(GlobalVector(cell->getPosition().x(), cell->getPosition().y(), cell->getPosition().z()));
      ehit.setEt(et);

      // std::cout << "ehit energy: " << ehit.et() << std::endl;
      // std::cout << "ehit eta: " << ehit.position().eta() << std::endl;
      // std::cout << "ehit phi: " << ehit.position().phi() << std::endl;

      // Find the card that this hit is in
      for(int cc = 0; cc < p2rctIO::N_CARDS; cc++) {
        if (ehit.isInCard(cc)) {

          // Get the crystal iEta and iPhi, relative to the bottom left corner of the card
          int local_iEta = ehit.crystalLocaliEta(cc);
          int local_iPhi = ehit.crystalLocaliPhi(cc);

          // std::cout << "Local iEta: " << local_iEta << std::endl;
          // std::cout << "Local iPhi: " << local_iPhi << std::endl;

          crystalEnergies[cc][local_iEta][local_iPhi] = crystalEnergies[cc][local_iEta][local_iPhi] + ehit.et();

          // std::cout << "crystalEnergies local_iEta+1: " << crystalEnergies[cc][(local_iEta+1)%85][local_iPhi] << std::endl;
          // std::cout << "crystalEnergies (local_iEta, local_iPhi): " << crystalEnergies[cc][local_iEta][local_iPhi] << std::endl;
          // std::cout << "crystalEnergies local_iPhi+1: " << crystalEnergies[cc][local_iEta][(local_iPhi+1)%85] << std::endl;

        }
      }
      // break;
    }
  }

  //***************************************************//
  // Get the HCAL hits
  //***************************************************//
  edm::Handle<edm::SortedCollection<HcalTriggerPrimitiveDigi>> hbhecoll;
  iEvent.getByToken(hcalTPToken_, hbhecoll);

  // Initialize array of energies to all 0s
  //float HCALtowerEnergies[p2rctIO::N_CARDS][p2rctIO::TOWERS_IN_REGION_ETA*p2rctIO::REGIONS_IN_CARD_ETA][p2rctIO::TOWERS_IN_REGION_PHI*p2rctIO::REGIONS_IN_CARD_PHI] = {{{ 0 }}};
  float HCALtowerEnergies[p2rctIO::N_REGIONS_ETA][p2rctIO::N_REGIONS_PHI][p2rctIO::TOWERS_IN_REGION_ETA][p2rctIO::TOWERS_IN_REGION_PHI] = {{{{ 0 }}}};

  for (const auto& hit : *hbhecoll.product()) {
    float et = decoder.hcaletValue(hit.id(), hit.t0());
    // same thing as SOI_compressedEt() in HcalTriggerPrimitiveDigi.h///
    if (et <= 0)
      continue;

    if (!(hcTopology_->validHT(hit.id()))) {
      LogError("Phase2L1CaloL1RCTEmulator")
          << " -- Hcal hit DetID not present in HCAL Geom: " << hit.id() << std::endl;
      throw cms::Exception("Phase2L1CaloL1RCTEmulator");
      continue;
    }
    const std::vector<HcalDetId>& hcId = theTrigTowerGeometry.detIds(hit.id());
    if (hcId.empty()) {
      LogError("Phase2L1CaloL1RCTEmulator") << "Cannot find any HCalDetId corresponding to " << hit.id() << std::endl;
      throw cms::Exception("Phase2L1CaloL1RCTEmulator");
      continue;
    }
    if (hcId[0].subdetId() > 1) {
      continue;
    }
    GlobalVector hcal_tp_position = GlobalVector(0., 0., 0.);
    for (const auto& hcId_i : hcId) {
      if (hcId_i.subdetId() > 1) {
        continue;
      }
      // get the first HCAL TP/ cell
      auto cell = hbGeometry->getGeometry(hcId_i);
      if (cell == nullptr) {
        continue;
      }
      GlobalVector tmpVector = GlobalVector(cell->getPosition().x(), cell->getPosition().y(), cell->getPosition().z());
      hcal_tp_position = tmpVector;

      break;
    }
    p2rctIO::SimpleCaloHit hhit;
    hhit.setPosition(hcal_tp_position);
    hhit.setEt(et);

    int BCP_iEta = hhit.BCPcardiEta();
    int BCP_iPhi = hhit.BCPcardiPhi();
    int iEtaInBCP = hhit.iEtaInBCPcard();
    int iPhiInBCP = hhit.iPhiInBCPcard();

    HCALtowerEnergies[BCP_iEta][BCP_iPhi][iEtaInBCP][iPhiInBCP] = HCALtowerEnergies[BCP_iEta][BCP_iPhi][iEtaInBCP][iPhiInBCP] + hhit.et();

  }

  // Iterate through cells of crystalEnergies and fill a vector of p2rctIO::RCTcardECAL objects

  std::vector<p2rctIO::RCTcardECAL> cardsECAL(p2rctIO::N_CARDS);
  for(int cc=0; cc < p2rctIO::N_CARDS; cc++) {
    for(int iEtaCrystalCard=0; iEtaCrystalCard < p2rctIO::N_TOWERS_ETA*p2rctIO::CRYSTALS_IN_TOWER_ETA; iEtaCrystalCard++) {
      for(int iPhiCrystalCard=0; iPhiCrystalCard < p2rctIO::N_TOWERS_PHI*p2rctIO::CRYSTALS_IN_TOWER_PHI; iPhiCrystalCard++) {
        float thisCrystalEnergy = crystalEnergies[cc][iEtaCrystalCard][iPhiCrystalCard];
        cardsECAL[cc].addHit(thisCrystalEnergy, 0.0, 0, iEtaCrystalCard, iPhiCrystalCard);

        // if (thisCrystalEnergy > 0.0) {
          // std::cout << "Link local_iEta: " << (bitset<576>)cardsECAL[cc].getLink(iEtaCrystalCard/5, iPhiCrystalCard/5).Data() << std::endl;
        // }
      }
    }
  }

  // Iterate through towers of HCALtowerEnergies and fill an array of HCAL links via a BCPcardsHCAL object

  p2rctIO::BCPcardsHCAL cardsHCAL;
  for(int BCPiEta=0; BCPiEta < p2rctIO::N_REGIONS_ETA; BCPiEta++) {
    for(int BCPiPhi=0; BCPiPhi < p2rctIO::N_REGIONS_PHI; BCPiPhi++) {
      for(int iEtaInBCP=0; iEtaInBCP < p2rctIO::TOWERS_IN_REGION_ETA; iEtaInBCP++) {
        for(int iPhiInBCP=0; iPhiInBCP < p2rctIO::TOWERS_IN_REGION_PHI; iPhiInBCP++) {
          float thisTowerEnergy = HCALtowerEnergies[BCPiEta][BCPiPhi][iEtaInBCP][iPhiInBCP];
          cardsHCAL.addHit(thisTowerEnergy, 0, BCPiEta, BCPiPhi, iEtaInBCP, iPhiInBCP);
        }
      }
    }
  }
  
  // std::cout << "Card 0, link (0,0): " << (bitset<576>)cardsECAL[0].getLink(0, 0).Data() << std::endl;
  // std::cout << "Card 23, link (16,5): " << (bitset<576>)cardsECAL[23].getLink(16, 5).Data() << std::endl;

  // Dumping the propagated GEN-electron information
  {
    const std::string eventDir =
        "rct_IO/event_" + std::to_string(eventId);

    ensureDir(eventDir);

    std::ofstream genCSV(
        eventDir + "/event_" +
        std::to_string(eventId) +
        "_gen_electrons.csv"
    );

    if (!genCSV.is_open()) {
      throw cms::Exception("Phase2L1CaloL1RCTEmulator")
          << "Could not open GEN CSV in " << eventDir;
    }

    dumpGenElectronsCSV(
        genCSV,
        iEvent,
        genParticleToken_
    );
  } 

  // Loop through cards
  for(int cc=0; cc < p2rctIO::N_CARDS; cc++) {
    DumpDirs dirs = makeDumpDirs(cc, eventId);
    ensureDumpDirs(dirs);

    const std::string prefix =
        "card_" + std::to_string(cc) + "_";

    {
      std::ofstream crystalCSV(
          dirs.crystals + "/" +
          prefix + "input_crystals.csv"
      );

      if (!crystalCSV.is_open()) {
        throw cms::Exception("Phase2L1CaloL1RCTEmulator")
            << "Could not open crystal CSV for card " << cc;
      }

      dumpCardCrystalCSV(
          crystalCSV,
          cc,
          crystalEnergies[cc]
      );
    }
    //////////////////////////// IP1 ////////////////////////////

    // Separate out the hits into the 5x6 and 2x6 areas
    std::vector<ap_uint<576>> link_in_SLR3_vec = cardsECAL[cc].getAx6(5, 0);
    std::vector<ap_uint<576>> link_in_SLR2_vec = cardsECAL[cc].getAx6(5, 5);
    std::vector<ap_uint<576>> link_in_SLR1_vec = cardsECAL[cc].getAx6(5, 10);
    std::vector<ap_uint<576>> link_in_SLR0_vec = cardsECAL[cc].getAx6(2, 15);

    ap_uint<576>* link_in_SLR3 = &link_in_SLR3_vec[0];
    ap_uint<576>* link_in_SLR2 = &link_in_SLR2_vec[0];
    ap_uint<576>* link_in_SLR1 = &link_in_SLR1_vec[0];
    ap_uint<576>* link_in_SLR0 = &link_in_SLR0_vec[0];

    // Initialize output links for IP1
    ap_uint<576> link_outIP1_SLR3[p2rctIP1_5x6::N_OUTPUT_LINKS];
    ap_uint<576> link_outIP1_SLR2[p2rctIP1_5x6::N_OUTPUT_LINKS];
    ap_uint<576> link_outIP1_SLR1[p2rctIP1_5x6::N_OUTPUT_LINKS];
    ap_uint<576> link_outIP1_SLR0[p2rctIP1_2x6::N_OUTPUT_LINKS];

    // Apply IP1 algo_top
    p2rctIP1_5x6::algo_top(link_in_SLR3, link_outIP1_SLR3);
    p2rctIP1_5x6::algo_top(link_in_SLR2, link_outIP1_SLR2);
    p2rctIP1_5x6::algo_top(link_in_SLR1, link_outIP1_SLR1);
    p2rctIP1_2x6::algo_top(link_in_SLR0, link_outIP1_SLR0);

    // Dumping all the I/O related to IP1
    writeRawLinksCSVFileInDir(dirs.inputIP1, cc, eventId, "IP1_SLR3_5x6", "input", link_in_SLR3, static_cast<int>(link_in_SLR3_vec.size()));
    writeRawLinksCSVFileInDir(dirs.inputIP1, cc, eventId, "IP1_SLR2_5x6", "input", link_in_SLR2, static_cast<int>(link_in_SLR2_vec.size()));
    writeRawLinksCSVFileInDir(dirs.inputIP1, cc, eventId, "IP1_SLR1_5x6", "input", link_in_SLR1, static_cast<int>(link_in_SLR1_vec.size()));
    writeRawLinksCSVFileInDir(dirs.inputIP1, cc, eventId, "IP1_SLR0_2x6", "input", link_in_SLR0, static_cast<int>(link_in_SLR0_vec.size()));

    writeRawLinksCSVFileInDir(dirs.outputIP1, cc, eventId, "IP1_SLR3_5x6", "output", link_outIP1_SLR3, p2rctIP1_5x6::N_OUTPUT_LINKS);
    writeRawLinksCSVFileInDir(dirs.outputIP1, cc, eventId, "IP1_SLR2_5x6", "output", link_outIP1_SLR2, p2rctIP1_5x6::N_OUTPUT_LINKS);
    writeRawLinksCSVFileInDir(dirs.outputIP1, cc, eventId, "IP1_SLR1_5x6", "output", link_outIP1_SLR1, p2rctIP1_5x6::N_OUTPUT_LINKS);
    writeRawLinksCSVFileInDir(dirs.outputIP1, cc, eventId, "IP1_SLR0_2x6", "output", link_outIP1_SLR0, p2rctIP1_2x6::N_OUTPUT_LINKS);
    
    // Decoded IP1 outputs.
    {
      std::ofstream csv(dirs.decodedIP1 + "/" + prefix + "SLR3_emu_decoded.csv");
      writeCSVHeader(csv);
      dumpDecodedIP1CSV(csv,"SLR3", "5x6",9,30, link_outIP1_SLR3);
    }

    {
      std::ofstream csv(dirs.decodedIP1 + "/" + prefix + "SLR2_emu_decoded.csv");
      writeCSVHeader(csv);
      dumpDecodedIP1CSV(csv,"SLR2","5x6",9,30,link_outIP1_SLR2);
    }

    {
      std::ofstream csv(dirs.decodedIP1 + "/" + prefix + "SLR1_emu_decoded.csv");
      writeCSVHeader(csv);
      dumpDecodedIP1CSV(csv,"SLR1","5x6",9,30,link_outIP1_SLR1);
    }

    {
      std::ofstream csv(dirs.decodedIP1 + "/" + prefix + "SLR0_emu_decoded.csv");

      writeCSVHeader(csv);
      dumpDecodedIP1CSV(csv,"SLR0","2x6",3,12,link_outIP1_SLR0);
    }


    // Reorganize links to be input links for IP21
    ap_uint<576> link_inIP21[p2rctIP1_5x6::N_OUTPUT_LINKS*3 + p2rctIP1_2x6::N_OUTPUT_LINKS];
    for(int i=0; i<2; i++) {
      link_inIP21[i*4] = link_outIP1_SLR3[i];
      link_inIP21[i*4 + 1] = link_outIP1_SLR2[i];
      link_inIP21[i*4 + 2] = link_outIP1_SLR1[i];
      link_inIP21[i*4 + 3] = link_outIP1_SLR0[i];
    }

    // Initialize output links for IP21
    ap_uint<576> link_outIP21[p2rctIP21::N_OUTPUT_LINKS];

    // Apply IP21 algo_top (using ss vars from firmware IP21 test bench)
    p2rctIP21::algo_top(link_inIP21, link_outIP21,126, 125, 125, 124, 123, 123, 122, 122, 121, 121, 121, 121, 121, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120);

    // I/O for IP21
    const std::vector<std::string> ip21Sources = makeIP21InputSources();

    writeRawLinksCSVFileInDir(dirs.inputIP21, cc, eventId, "IP21", "input", link_inIP21, p2rctIP21::N_INPUT_LINKS, &ip21Sources);
    writeRawLinksCSVFileInDir(dirs.outputIP21, cc, eventId, "IP21", "output", link_outIP21,p2rctIP21::N_OUTPUT_LINKS);
    // IP21 input ECAL clusters: SLR3, SLR2, and SLR1.
    {
      std::ofstream csv(dirs.decodedIP21 + "/" + prefix + "IP21_input_clusters_decoded.csv");
      dumpDecodedEcalClusterLinksCSV(csv, cc, eventId, "IP21", "input", link_inIP21, {0, 1, 2}, 9);
    }
    // IP21 input ECAL clusters: SLR0.
    {
      std::ofstream csv(dirs.decodedIP21 + "/" +prefix + "IP21_input_clusters_SLR0_decoded.csv");
      dumpDecodedEcalClusterLinksCSV(csv, cc, eventId, "IP21", "input", link_inIP21, {3}, 3);
    }
    // IP21 input towers: SLR3, SLR2, and SLR1.
    {
      std::ofstream csv(dirs.decodedIP21 + "/" + prefix + "IP21_input_towers_5x6_decoded.csv");
      dumpDecodedTowerLinksCSV(csv, cc, eventId, "IP21", "input", link_inIP21, {4, 5, 6}, 30);
    }
    // IP21 input towers: SLR0.
    {
      std::ofstream csv(dirs.decodedIP21 + "/" + prefix + "IP21_input_towers_SLR0_decoded.csv");
      dumpDecodedTowerLinksCSV(csv, cc, eventId, "IP21", "input", link_inIP21, {7}, 12);
    }
    // IP21 output cluster link.
    {
      std::ofstream csv(dirs.decodedIP21 + "/" + prefix + "IP21_output_rct_clusters_decoded.csv");
      dumpDecodedRctClusterLinkCSV(csv, cc, eventId, "IP21", "output", link_outIP21, 0, p2rctIP21::N_CLUSTERS_OUT);
    }
    {
      std::ofstream csv(dirs.decodedIP21 + "/" + prefix + "IP21_output_rejected_clusters_decoded.csv");
      dumpDecodedEcalClusterLinksCSV(csv, cc, eventId, "IP21", "output", link_outIP21, {1, 2, 3}, 9);
    }

    // IP21 output links 4-6:
    // ECAL tower links corresponding to the three 5x6 SLRs.
    {
      std::ofstream csv(dirs.decodedIP21 + "/" + prefix + "IP21_output_towers_5x6_decoded.csv");
      dumpDecodedTowerLinksCSV(csv, cc, eventId, "IP21", "output", link_outIP21, {4, 5, 6}, 30);
    }

    // IP21 output link 7:
    // ECAL towers from the SLR0 2x6 region.
    {
      std::ofstream csv(dirs.decodedIP21 + "/" + prefix + "IP21_output_towers_SLR0_decoded.csv");
      dumpDecodedTowerLinksCSV(csv, cc, eventId, "IP21", "output", link_outIP21,{7}, 12);
    }


    // Initialize output links for IP22
    ap_uint<576> link_outIP22[p2rctIP22::N_OUTPUT_LINKS];

    // Apply IP22 algo_top (link_outIP21 = link_inIP22)
    p2rctIP22::algo_top(link_outIP21, link_outIP22);

    writeRawLinksCSVFileInDir(dirs.inputIP22, cc, eventId, "IP22", "input", link_outIP21, p2rctIP22::N_INPUT_LINKS);
    writeRawLinksCSVFileInDir(dirs.outputIP22, cc, eventId, "IP22", "output", link_outIP22, p2rctIP22::N_OUTPUT_LINKS);

    {
      std::ofstream csv(dirs.decodedIP22 + "/" + prefix + "IP22_input_rct_clusters_decoded.csv");
      dumpDecodedRctClusterLinkCSV(csv, cc, eventId, "IP22", "input", link_outIP21, 0, p2rctIP21::N_CLUSTERS_OUT);
    }
    // Decode the RCT cluster output carried by IP22 output link 0.
    {
      std::ofstream csv(dirs.decodedIP22 + "/" + prefix + "IP22_output_rct_clusters_decoded.csv");
      dumpDecodedRctClusterLinkCSV(csv, cc, eventId, "IP22", "output", link_outIP22, 0, p2rctIP22::N_CLUSTERS_OUT);
    }

    // IP22 input links 1-3:
    // rejected ECAL clusters from IP21.
    {
      std::ofstream csv(dirs.decodedIP22 + "/" + prefix + "IP22_input_rejected_clusters_decoded.csv");
      dumpDecodedEcalClusterLinksCSV(csv, cc, eventId, "IP22", "input", link_outIP21, {1, 2, 3}, 9);
    }

    // IP22 input links 4-6:
    // ECAL tower links from the three 5x6 SLRs.
    {
      std::ofstream csv(dirs.decodedIP22 + "/" + prefix + "IP22_input_towers_5x6_decoded.csv");
      dumpDecodedTowerLinksCSV(csv, cc, eventId, "IP22", "input", link_outIP21, {4, 5, 6}, 30);
    }

    // IP22 input link 7:
    // ECAL towers from SLR0 2x6.
    {
      std::ofstream csv(dirs.decodedIP22 + "/" + prefix + "IP22_input_towers_SLR0_decoded.csv");
      dumpDecodedTowerLinksCSV(csv, cc, eventId, "IP22", "input", link_outIP21, {7}, 12);
    }
    // IP22 output link 0:
    // IP22 output links 1-3:
    // ECAL towers sent to IP3.
    {
      std::ofstream csv(dirs.decodedIP22 + "/" + prefix + "IP22_output_towers_5x6_decoded.csv");
      dumpDecodedTowerLinksCSV(csv, cc, eventId, "IP22", "output", link_outIP22, {1, 2, 3}, 30);
    }
    // IP22 output link 4:
    // ECAL towers from the SLR0 2x6 region.
    {
      std::ofstream csv(dirs.decodedIP22 + "/" + prefix + "IP22_output_towers_SLR0_decoded.csv");
      dumpDecodedTowerLinksCSV(csv, cc, eventId, "IP22", "output", link_outIP22, {4}, 12);
    }

    // Append HCAL links to link_outIP22 to form link_inIP3
    ap_uint<576> link_inIP3[p2rctIP3::N_INPUT_LINKS];
    // IP22 output links
    for(int i=0; i<p2rctIP22::N_OUTPUT_LINKS; i++) {
      link_inIP3[i] = link_outIP22[i];
    }
    // HCAL links
    bool secondhalfstarts = (((cc + 3) % 4) > 1); //True for cards 0,3,4,7,etc.
    int BCPiPhiLow = (3*(cc / 2)) / 2;
    int BCPiPhiHigh = BCPiPhiLow + 1;
    if((cc % 2) == 1) {
      // Positive eta
      int BCPiEtaLow = 2;
      int BCPiEtaHigh = 3;

      if (!secondhalfstarts) {
        link_inIP3[5] = cardsHCAL.getLink(BCPiEtaLow,BCPiPhiLow).Data();
        link_inIP3[6] = cardsHCAL.getLink(BCPiEtaHigh,BCPiPhiLow).Data();
        link_inIP3[7] = cardsHCAL.getLink(BCPiEtaLow,BCPiPhiHigh).Data();
        link_inIP3[8] = cardsHCAL.getLink(BCPiEtaHigh,BCPiPhiHigh).Data();
      }
      else {
        link_inIP3[5] = cardsHCAL.getLink(BCPiEtaLow,BCPiPhiHigh).Data();
        link_inIP3[6] = cardsHCAL.getLink(BCPiEtaHigh,BCPiPhiHigh).Data();
        link_inIP3[7] = cardsHCAL.getLink(BCPiEtaLow,BCPiPhiLow).Data();
        link_inIP3[8] = cardsHCAL.getLink(BCPiEtaHigh,BCPiPhiLow).Data();
      }
    }
    else {
      // Negative eta
      int BCPiEtaLow = 0;
      int BCPiEtaHigh = 1;

      if (!secondhalfstarts) {
        link_inIP3[5] = cardsHCAL.getLink(BCPiEtaHigh,BCPiPhiHigh).Data();
        link_inIP3[6] = cardsHCAL.getLink(BCPiEtaLow,BCPiPhiHigh).Data();
        link_inIP3[7] = cardsHCAL.getLink(BCPiEtaHigh,BCPiPhiLow).Data();
        link_inIP3[8] = cardsHCAL.getLink(BCPiEtaLow,BCPiPhiLow).Data();
      }
      else {
        link_inIP3[5] = cardsHCAL.getLink(BCPiEtaHigh,BCPiPhiLow).Data();
        link_inIP3[6] = cardsHCAL.getLink(BCPiEtaLow,BCPiPhiLow).Data();
        link_inIP3[7] = cardsHCAL.getLink(BCPiEtaHigh,BCPiPhiHigh).Data();
        link_inIP3[8] = cardsHCAL.getLink(BCPiEtaLow,BCPiPhiHigh).Data();
      }
    }

    // Initialize output links for IP3
    ap_uint<576> link_outIP3[p2rctIP3::N_OUTPUT_LINKS];


    // I/O for IP3
    writeRawLinksCSVFileInDir(dirs.inputIP3, cc, eventId, "IP3", "input", link_inIP3, p2rctIP3::N_INPUT_LINKS);
    
    // Decode the ECAL/RCT cluster input from IP22.
    {
      std::ofstream csv(dirs.decodedIP3 + "/" + prefix + "IP3_input_rct_clusters_decoded.csv");
      dumpDecodedRctClusterLinkCSV(csv, cc, eventId, "IP3", "input", link_inIP3, 0, p2rctIP3::N_CLUSTERS);
    }
    // Decode HCAL links 5–8.
    {
      std::ofstream csv(dirs.decodedIP3 + "/" + prefix + "IP3_input_hcal_towers_decoded.csv");
      dumpDecodedHcalTowerLinksCSV(csv, cc, eventId, "IP3", "input", link_inIP3, {5, 6, 7, 8}, p2rctIO::TOWERS_IN_REGION_ETA * p2rctIO::TOWERS_IN_REGION_PHI);
    }

    // IP3 input links 1-3:
    // ECAL towers from the three 5x6 SLR regions.
    {
      std::ofstream csv(dirs.decodedIP3 + "/" + prefix + "IP3_input_ecal_towers_5x6_decoded.csv");
      dumpDecodedTowerLinksCSV(csv, cc, eventId, "IP3", "input", link_inIP3, {1, 2, 3}, 30);
    }

    // IP3 input link 4:
    // ECAL towers from SLR0 2x6.
    {
      std::ofstream csv(dirs.decodedIP3 + "/" + prefix + "IP3_input_ecal_towers_SLR0_decoded.csv");
      dumpDecodedTowerLinksCSV(csv, cc, eventId, "IP3", "input", link_inIP3, {4}, 12);
    }

    // Apply IP3 algo_top, or algo_top_HCALsecondhalfstarts
    if (!secondhalfstarts) {
      p2rctIP3::algo_top(link_inIP3, link_outIP3);
    }
    else {
      p2rctIP3::algo_top_HCALsecondhalfstarts(link_inIP3, link_outIP3);
    }

    writeRawLinksCSVFileInDir(dirs.outputIP3, cc, eventId, "IP3", "output", link_outIP3, p2rctIP3::N_OUTPUT_LINKS);
    {
      std::ofstream csv(dirs.decodedIP3 + "/" + prefix + "IP3_output_decoded.csv");
      dumpDecodedIP3OutputCSV(csv, cc, eventId, link_outIP3);
    }

    // // Print outputs for comparison purposes
    // std::cout << "------------------------------" << std::endl;
    // std::cout << "New emulator RCT outputs of card: " << cc << std::endl;

    // // Print output clusters
    // std::cout << "Clusters:" << std::endl;
    // p2rctIO::printIP3OutputClusters(link_outIP3[0]);

    // // Print towers
    // std::cout << "Tower iPhi 0 and 1:" << std::endl;
    // p2rctIO::printIP3OutputTowers(link_outIP3[1]);
    // std::cout << "Tower iPhi 2 and 3:" << std::endl;
    // p2rctIO::printIP3OutputTowers(link_outIP3[2]);
    // std::cout << "Tower iPhi 4 and 5:" << std::endl;
    // p2rctIO::printIP3OutputTowers(link_outIP3[3]);

    // Move outputs into output vectors
    link_out0->push_back(link_outIP3[0]);
    link_out1->push_back(link_outIP3[1]);
    link_out2->push_back(link_outIP3[2]);
    link_out3->push_back(link_outIP3[3]);

  }

  iEvent.put(std::move(link_out0), "LinkOut0");
  iEvent.put(std::move(link_out1), "LinkOut1");
  iEvent.put(std::move(link_out2), "LinkOut2");
  iEvent.put(std::move(link_out3), "LinkOut3");
}

//////////////////////////////////////////////////////////////////////////

void Phase2L1CaloL1RCTEmulator::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  // l1tPhase2L1CaloL1RCTEmulator
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("ecalTPEB", edm::InputTag("simEcalEBTriggerPrimitiveDigis"));
  desc.add<edm::InputTag>("hcalTP", edm::InputTag("simHcalTriggerPrimitiveDigis"));
  desc.add<edm::InputTag>("genParticles", edm::InputTag("genParticles"));

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
