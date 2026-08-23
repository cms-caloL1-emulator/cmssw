#ifndef L1Trigger_L1CaloTrigger_RCTDumpUtils_h
#define L1Trigger_L1CaloTrigger_RCTDumpUtils_h

#include <ap_int.h>

#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "CommonTools/BaseParticlePropagator/interface/BaseParticlePropagator.h"
#include "CommonTools/BaseParticlePropagator/interface/RawParticle.h"
#include "L1Trigger/L1CaloTrigger/interface/RCT_IO.h"
#include "L1Trigger/L1CaloTrigger/interface/RCT_IP3_h.h"

namespace rctdump {

// This part of the code is added to create inpute and output directories
inline void ensureDir(const std::string& dir) {
  std::string cmd = "mkdir -p \"" + dir + "\"";
  int ret = std::system(cmd.c_str());
  if (ret != 0) {
    throw cms::Exception("Phase2L1CaloL1RCTEmulator")
        << "Failed to create directory: " << dir;
  }
}

struct DumpDirs {
  std::string base;

  std::string input;
  std::string output;
  std::string decoded;
  std::string text;
  std::string crystals;

  std::string inputIP1;
  std::string inputIP21;
  std::string inputIP22;
  std::string inputIP3;

  std::string outputIP1;
  std::string outputIP21;
  std::string outputIP22;
  std::string outputIP3;

  std::string decodedIP1;
  std::string decodedIP21;
  std::string decodedIP22;
  std::string decodedIP3;
};

inline DumpDirs makeDumpDirs(int card, unsigned long long eventId) {
  DumpDirs d;

  std::string eventDir = "rct_IO/event_" + std::to_string(eventId);

  d.base = eventDir + "/card_" + std::to_string(card);

  d.input    = d.base + "/input";
  d.output   = d.base + "/output";
  d.decoded  = d.base + "/decoded";
  d.crystals = d.base + "/crystals";

  d.inputIP1  = d.input  + "/IP1";
  d.inputIP21 = d.input  + "/IP21";
  d.inputIP22 = d.input  + "/IP22";
  d.inputIP3  = d.input  + "/IP3";

  d.outputIP1  = d.output + "/IP1";
  d.outputIP21 = d.output + "/IP21";
  d.outputIP22 = d.output + "/IP22";
  d.outputIP3  = d.output + "/IP3";

  d.decodedIP1 = d.decoded + "/IP1";
  d.decodedIP21 = d.decoded + "/IP21";
  d.decodedIP22 = d.decoded + "/IP22";
  d.decodedIP3  = d.decoded + "/IP3";

  return d;
}

inline void ensureDumpDirs(const DumpDirs& d) {
  ensureDir(d.base);

  ensureDir(d.input);
  ensureDir(d.output);
  ensureDir(d.decoded);
  ensureDir(d.crystals);

  ensureDir(d.inputIP1);
  ensureDir(d.inputIP21);
  ensureDir(d.inputIP22);
  ensureDir(d.inputIP3);

  ensureDir(d.outputIP1);
  ensureDir(d.outputIP21);
  ensureDir(d.outputIP22);
  ensureDir(d.outputIP3);

  ensureDir(d.decodedIP1);
  ensureDir(d.decodedIP21);
  ensureDir(d.decodedIP22);
  ensureDir(d.decodedIP3);
}


inline std::string hexIndex(unsigned int i) {
  std::ostringstream os;
  os << std::hex << std::nouppercase << i;
  return os.str();
}

inline std::string formatApUint576(const ap_uint<576>& x) {
  if (x == 0) return "0";
  std::string s = x.to_string(16);
  if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) return s;
  return "0x" + s;
}


struct FWClusterDecoded {
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
};

struct FWTowerDecoded {
  ap_uint<12> energy;
  ap_uint<5> timing;
  ap_uint<1> spike;
};

// Needed particularly for IP21, IP22
struct FWRctClusterDecoded {
  ap_uint<10> seedEnergy;
  ap_uint<12> energy;
  ap_uint<7> eta;
  ap_uint<5> phi;
  ap_uint<10> et5x5;
  ap_uint<7> wps;
  ap_uint<5> timing;
  ap_uint<1> spike;
  ap_uint<1> satur;
  ap_uint<2> brems;
  ap_uint<3> spare;
};

struct FWHcalTowerDecoded {
  ap_uint<10> energy;
  ap_uint<6> fb;
};

struct FWRctTowerDecoded {
  ap_uint<10> energy;
  ap_uint<4> hoe;
  ap_uint<2> flags;
};

inline std::string formatApUint64(const ap_uint<64>& x) {
  if (x == 0) return "0";
  std::string s = x.to_string(16);
  if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) return s;
  return "0x" + s;
}

inline std::string formatApUint18(const ap_uint<18>& x) {
  if (x == 0) return "0";
  std::string s = x.to_string(16);
  if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) return s;
  return "0x" + s;
}

inline std::string formatApUint16(const ap_uint<16>& x) {
  if (x == 0) return "0";
  std::string s = x.to_string(16);
  if (s.rfind("0x", 0) == 0 || s.rfind("0X", 0) == 0) return s;
  return "0x" + s;
}

inline FWClusterDecoded unpackFWCluster(ap_uint<64> word) {
  FWClusterDecoded c;
  c.seedEnergy = word.range(9, 0);
  c.energy     = word.range(21, 10);
  c.eta        = word.range(26, 22);
  c.phi        = word.range(31, 27);
  c.et5x5      = word.range(41, 32);
  c.et2x5      = word.range(51, 42);
  c.timing     = word.range(56, 52);
  c.spike      = word.range(57, 57);
  c.satur      = word.range(58, 58);
  c.brems      = word.range(60, 59);
  c.spare      = word.range(63, 61);
  return c;
}

inline FWRctClusterDecoded unpackFWRctCluster(ap_uint<64> word) {
  FWRctClusterDecoded c;

  c.seedEnergy = word.range(9, 0);
  c.energy     = word.range(21, 10);
  c.eta        = word.range(28, 22);
  c.phi        = word.range(33, 29);
  c.et5x5      = word.range(43, 34);
  c.wps        = word.range(50, 44);
  c.timing     = word.range(55, 51);
  c.spike      = word.range(56, 56);
  c.satur      = word.range(57, 57);
  c.brems      = word.range(59, 58);
  c.spare      = word.range(62, 60);

  return c;
}

inline FWTowerDecoded unpackFWTower(ap_uint<18> word) {
  FWTowerDecoded t;
  t.energy = word.range(11, 0);
  t.timing = word.range(16, 12);
  t.spike  = word.range(17, 17);
  return t;
}

inline FWHcalTowerDecoded unpackFWHcalTower(ap_uint<16> word) {
  FWHcalTowerDecoded t;
  t.energy = word.range(9, 0);
  t.fb     = word.range(15, 10);
  return t;
}

inline FWRctTowerDecoded unpackFWRctTower(ap_uint<16> word) {
  FWRctTowerDecoded t;
  t.energy = word.range(9, 0);
  t.hoe    = word.range(13, 10);
  t.flags  = word.range(15, 14);
  return t;
}

inline void dumpCardCrystalCSV(
    std::ofstream& out,
    int card,
    float crystalGrid[p2rctIO::N_TOWERS_ETA * p2rctIO::CRYSTALS_IN_TOWER_ETA]
                     [p2rctIO::N_TOWERS_PHI * p2rctIO::CRYSTALS_IN_TOWER_PHI]) {

  out << "card,ieta,iphi,energy,tower_ieta,tower_iphi,crystal_in_tower_ieta,crystal_in_tower_iphi\n";

  for (int iEta = 0; iEta < p2rctIO::N_TOWERS_ETA * p2rctIO::CRYSTALS_IN_TOWER_ETA; ++iEta) {
    for (int iPhi = 0; iPhi < p2rctIO::N_TOWERS_PHI * p2rctIO::CRYSTALS_IN_TOWER_PHI; ++iPhi) {
      int tower_iEta = iEta / p2rctIO::CRYSTALS_IN_TOWER_ETA;
      int tower_iPhi = iPhi / p2rctIO::CRYSTALS_IN_TOWER_PHI;
      int crystal_iEta_in_tower = iEta % p2rctIO::CRYSTALS_IN_TOWER_ETA;
      int crystal_iPhi_in_tower = iPhi % p2rctIO::CRYSTALS_IN_TOWER_PHI;

      out << card << ","
          << iEta << ","
          << iPhi << ","
          << crystalGrid[iEta][iPhi] << ","
          << tower_iEta << ","
          << tower_iPhi << ","
          << crystal_iEta_in_tower << ","
          << crystal_iPhi_in_tower
          << "\n";
    }
  }
}

inline std::vector<std::string> makeIP21InputSources() {
  return {
    "SLR3_out0", "SLR2_out0", "SLR1_out0", "SLR0_out0",
    "SLR3_out1", "SLR2_out1", "SLR1_out1", "SLR0_out1"
  };
}

inline void writeCSVHeader(std::ofstream& out) {
  out << "slr,block_type,object_type,index,raw_hex,"
      << "seedEnergy,clusterEnergy,eta,phi,et5x5,et2x5,"
      << "timing,spike,satur,brems,spare,towerEnergy\n";
}

// For IP21, IP22
inline void writeRctClusterCSVHeader(std::ofstream& out) {
  out << "card,event,component,direction,link_index,object_type,index,raw_hex,"
      << "seedEnergy,clusterEnergy,eta,phi,et5x5,wps,"
      << "timing,spike,satur,brems,spare\n";
}

inline void dumpDecodedIP1CSV(std::ofstream& out,
                               const std::string& slrLabel,
                               const std::string& blockType,
                               int nClusters,
                               int nTowers,
                               ap_uint<576> (&links)[2]) {
  // clusters from links[0]
  for (int i = 0; i < nClusters; ++i) {
    int start = i * 64;
    ap_uint<64> raw = (ap_uint<64>)links[0].range(start + 63, start);
    FWClusterDecoded c = unpackFWCluster(raw);

    out << slrLabel << ","
        << blockType << ","
        << "cluster" << ","
        << i << ","
        << formatApUint64(raw) << ","
        << (unsigned)c.seedEnergy << ","
        << (unsigned)c.energy << ","
        << (unsigned)c.eta << ","
        << (unsigned)c.phi << ","
        << (unsigned)c.et5x5 << ","
        << (unsigned)c.et2x5 << ","
        << (unsigned)c.timing << ","
        << (unsigned)c.spike << ","
        << (unsigned)c.satur << ","
        << (unsigned)c.brems << ","
        << (unsigned)c.spare << ","
        << ""
        << "\n";
  }

  // towers from links[1]
  for (int i = 0; i < nTowers; ++i) {
    int start = i * 18;
    ap_uint<18> raw = (ap_uint<18>)links[1].range(start + 17, start);
    FWTowerDecoded t = unpackFWTower(raw);

    int towerEta = i / 6;
    int towerPhi = i % 6;

    out << slrLabel << ","
        << blockType << ","
        << "tower" << ","
        << i << ","
        << formatApUint18(raw) << ","
        << "" << ","
        << "" << ","
        << towerEta << ","
        << towerPhi << ","
        << "" << ","
        << "" << ","
        << (unsigned)t.timing << ","
        << (unsigned)t.spike << ","
        << "" << ","
        << "" << ","
        << "" << ","
        << (unsigned)t.energy
        << "\n";
  }
}
// Needed specifically for IP21, IP22

inline void dumpDecodedEcalClusterLinksCSV(
    std::ofstream& out,
    int card,
    unsigned long long eventId,
    const std::string& component,
    const std::string& direction,
    const ap_uint<576>* links,
    const std::vector<int>& linkIndices,
    int nClustersPerLink
) {
  out << "card,event,component,direction,link_index,object_type,index,raw_hex,"
      << "seedEnergy,clusterEnergy,eta,phi,et5x5,et2x5,"
      << "timing,spike,satur,brems,spare,towerEnergy\n";

  for (int linkIdx : linkIndices) {
    for (int i = 0; i < nClustersPerLink; ++i) {
      int start = i * 64;

      ap_uint<64> raw =
          (ap_uint<64>)links[linkIdx].range(start + 63, start);

      FWClusterDecoded c = unpackFWCluster(raw);

      out << card << ","
          << eventId << ","
          << component << ","
          << direction << ","
          << linkIdx << ","
          << "cluster" << ","
          << i << ","
          << formatApUint64(raw) << ","
          << (unsigned)c.seedEnergy << ","
          << (unsigned)c.energy << ","
          << (unsigned)c.eta << ","
          << (unsigned)c.phi << ","
          << (unsigned)c.et5x5 << ","
          << (unsigned)c.et2x5 << ","
          << (unsigned)c.timing << ","
          << (unsigned)c.spike << ","
          << (unsigned)c.satur << ","
          << (unsigned)c.brems << ","
          << (unsigned)c.spare << ","
          << ""
          << "\n";
    }
  }
}

inline void dumpDecodedTowerLinksCSV(
    std::ofstream& out,
    int card,
    unsigned long long eventId,
    const std::string& component,
    const std::string& direction,
    const ap_uint<576>* links,
    const std::vector<int>& linkIndices,
    int nTowersPerLink
) {
  out << "card,event,component,direction,link_index,object_type,index,raw_hex,"
      << "seedEnergy,clusterEnergy,eta,phi,et5x5,et2x5,"
      << "timing,spike,satur,brems,spare,towerEnergy\n";

  for (int linkIdx : linkIndices) {
    for (int i = 0; i < nTowersPerLink; ++i) {
      int start = i * 18;

      ap_uint<18> raw =
          (ap_uint<18>)links[linkIdx].range(start + 17, start);

      FWTowerDecoded t = unpackFWTower(raw);

      out << card << ","
          << eventId << ","
          << component << ","
          << direction << ","
          << linkIdx << ","
          << "tower" << ","
          << i << ","
          << formatApUint18(raw) << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << (unsigned)t.timing << ","
          << (unsigned)t.spike << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << (unsigned)t.energy
          << "\n";
    }
  }
}
// Needed for IP3
inline void dumpDecodedIP3OutputCSV(
    std::ofstream& out,
    int card,
    unsigned long long eventId,
    ap_uint<576> (&links)[p2rctIP3::N_OUTPUT_LINKS]
) {
  out << "card,event,link_index,object_type,index,raw_hex,"
      << "seedEnergy,clusterEnergy,eta,phi,et5x5,wps,"
      << "timing,spike,satur,brems,spare,"
      << "towerEnergy,hoe,flags,towerPhiGroup,towerEtaIndex\n";

  // link 0: RCT clusters, 64-bit words
  for (int i = 0; i < p2rctIP3::N_CLUSTERS; ++i) {
    int start = i * 64;

    ap_uint<64> raw =
        (ap_uint<64>)links[0].range(start + 63, start);

    FWRctClusterDecoded c = unpackFWRctCluster(raw);

    out << card << ","
        << eventId << ","
        << 0 << ","
        << "rct_cluster" << ","
        << i << ","
        << formatApUint64(raw) << ","
        << (unsigned)c.seedEnergy << ","
        << (unsigned)c.energy << ","
        << (unsigned)c.eta << ","
        << (unsigned)c.phi << ","
        << (unsigned)c.et5x5 << ","
        << (unsigned)c.wps << ","
        << (unsigned)c.timing << ","
        << (unsigned)c.spike << ","
        << (unsigned)c.satur << ","
        << (unsigned)c.brems << ","
        << (unsigned)c.spare << ","
        << "" << ","
        << "" << ","
        << "" << ","
        << "" << ","
        << ""
        << "\n";
  }

  // links 1,2,3: RCT towers, 16-bit words.
  // Each link holds two phi columns:
  // link 1 -> phi 0 and 1
  // link 2 -> phi 2 and 3
  // link 3 -> phi 4 and 5
  for (int linkIdx = 1; linkIdx <= 3; ++linkIdx) {
    int phiA = 2 * (linkIdx - 1);
    int phiB = phiA + 1;

    for (int iEta = 0; iEta < p2rctIP3::N_TOWERS_IN_ETA; ++iEta) {
      int startA = iEta * 16;
      ap_uint<16> rawA =
          (ap_uint<16>)links[linkIdx].range(startA + 15, startA);

      FWRctTowerDecoded tA = unpackFWRctTower(rawA);

      out << card << ","
          << eventId << ","
          << linkIdx << ","
          << "rct_tower" << ","
          << iEta << ","
          << formatApUint16(rawA) << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << (unsigned)tA.energy << ","
          << (unsigned)tA.hoe << ","
          << (unsigned)tA.flags << ","
          << phiA << ","
          << iEta
          << "\n";

      int startB = iEta * 16 + 272;
      ap_uint<16> rawB =
          (ap_uint<16>)links[linkIdx].range(startB + 15, startB);

      FWRctTowerDecoded tB = unpackFWRctTower(rawB);

      out << card << ","
          << eventId << ","
          << linkIdx << ","
          << "rct_tower" << ","
          << iEta + p2rctIP3::N_TOWERS_IN_ETA << ","
          << formatApUint16(rawB) << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << "" << ","
          << (unsigned)tB.energy << ","
          << (unsigned)tB.hoe << ","
          << (unsigned)tB.flags << ","
          << phiB << ","
          << iEta
          << "\n";
    }
  }
}

inline void dumpDecodedRctClusterLinkCSV(
    std::ofstream& out,
    int card,
    unsigned long long eventId,
    const std::string& component,
    const std::string& direction,
    const ap_uint<576>* links,
    int linkIdx,
    int nClusters
) {
  writeRctClusterCSVHeader(out);

  for (int i = 0; i < nClusters; ++i) {
    int start = i * 64;

    ap_uint<64> raw =
        (ap_uint<64>)links[linkIdx].range(start + 63, start);

    FWRctClusterDecoded c = unpackFWRctCluster(raw);

    out << card << ","
        << eventId << ","
        << component << ","
        << direction << ","
        << linkIdx << ","
        << "rct_cluster" << ","
        << i << ","
        << formatApUint64(raw) << ","
        << (unsigned)c.seedEnergy << ","
        << (unsigned)c.energy << ","
        << (unsigned)c.eta << ","
        << (unsigned)c.phi << ","
        << (unsigned)c.et5x5 << ","
        << (unsigned)c.wps << ","
        << (unsigned)c.timing << ","
        << (unsigned)c.spike << ","
        << (unsigned)c.satur << ","
        << (unsigned)c.brems << ","
        << (unsigned)c.spare
        << "\n";
  }
}

inline void dumpDecodedHcalTowerLinksCSV(
      std::ofstream& out,
      int card,
      unsigned long long eventId,
      const std::string& component,
      const std::string& direction,
      const ap_uint<576>* links,
      const std::vector<int>& linkIndices,
      int nTowersPerLink
  ) {
    out << "card,event,component,direction,link_index,object_type,index,raw_hex,"
        << "hcalEnergy,fb\n";

    for (int linkIdx : linkIndices) {
      for (int i = 0; i < nTowersPerLink; ++i) {
        int start = i * 16;

        ap_uint<16> raw =
            (ap_uint<16>)links[linkIdx].range(start + 15, start);

        FWHcalTowerDecoded t = unpackFWHcalTower(raw);

        out << card << ","
            << eventId << ","
            << component << ","
            << direction << ","
            << linkIdx << ","
            << "hcal_tower" << ","
            << i << ","
            << formatApUint16(raw) << ","
            << (unsigned)t.energy << ","
            << (unsigned)t.fb
            << "\n";
      }
    }
  }

inline void dumpRawLinksCSV(std::ofstream& out,
                     int card,
                     unsigned long long eventId,
                     const std::string& component,
                     const std::string& direction,
                     const ap_uint<576>* links,
                     int nLinks,
                     const std::vector<std::string>* sourceLabels = nullptr) {
  out << "card,event,component,direction,link_index_dec,link_index_hex";
  if (sourceLabels) out << ",source";
  out << ",raw_hex\n";

  for (int i = 0; i < nLinks; ++i) {
    out << card << ","
        << eventId << ","
        << component << ","
        << direction << ","
        << i << ","
        << hexIndex(i);

    if (sourceLabels) {
      if (i < (int)sourceLabels->size()) out << "," << (*sourceLabels)[i];
      else out << ",UNKNOWN";
    }

    out << "," << formatApUint576(links[i]) << "\n";
  }
}

inline void writeRawLinksCSVFileInDir(const std::string& dir,
                               int card,
                               unsigned long long eventId,
                               const std::string& component,
                               const std::string& direction,
                               const ap_uint<576>* links,
                               int nLinks,
                               const std::vector<std::string>* sourceLabels = nullptr) {
  std::string fileName = dir + "/card_" + std::to_string(card)
                       + "_event_" + std::to_string(eventId)
                       + "_" + component + "_" + direction + "_links.csv";

  std::ofstream out(fileName);
  dumpRawLinksCSV(out, card, eventId, component, direction, links, nLinks, sourceLabels);
}

inline void dumpGenElectronsCSV(
    std::ofstream& out,
    const edm::Event& iEvent,
    const edm::EDGetTokenT<std::vector<reco::GenParticle>>& genParticleToken
) {
  out << "run,lumi,event,"
      << "gen_idx,pdgId,status,"
      << "gen_pt,gen_eta,gen_phi,gen_energy,"
      << "gen_vx,gen_vy,gen_vz,"
      << "gen_ecal_pt,gen_ecal_eta,gen_ecal_phi,"
      << "gen_rct_card,"
      << "gen_crystal_iEta,gen_crystal_iPhi,"
      << "gen_local_iEta,gen_local_iPhi"
      << "\n";

  edm::Handle<std::vector<reco::GenParticle>> genParticles;
  iEvent.getByToken(genParticleToken, genParticles);

  if (!genParticles.isValid()) {
    std::cout << "WARNING: genParticles collection not found. GEN CSV will be empty."
              << std::endl;
    return;
  }

  int nGenParticles = genParticles->size();
  int nGenElectrons = 0;
  int nEtaPreselected = 0;
  int nPropagated = 0;
  int nAssignedToRctCard = 0;

  for (size_t iGen = 0; iGen < genParticles->size(); ++iGen) {

    const reco::GenParticle& p = genParticles->at(iGen);

    const int pdgId = p.pdgId();
    const int absPdgId = std::abs(pdgId);

    if (absPdgId != 11) continue;
    nGenElectrons++;

    // Loose preselection before propagation.
    // This keeps barrel-ish electrons while allowing bending.
    // if (std::abs(p.eta()) > 2.0) continue;
    nEtaPreselected++;

    RawParticle particle(p.p4());

    particle.setVertex(
        p.vertex().x(),
        p.vertex().y(),
        p.vertex().z(),
        0.0
    );

    // CMSSW four-vectors are in GeV.
    particle.setMass(0.000511);

    // PDG convention: e- has pdgId = 11 and charge = -1.
    if (pdgId > 0) {
      particle.setCharge(-1.0);
    }
    else {
      particle.setCharge(1.0);
    }

    const float field_z = 4.0;

    BaseParticlePropagator prop(particle, 0.0, 0.0, field_z);
    prop.propagateToEcalEntrance();

    if (prop.getSuccess() == 0) continue;
    nPropagated++;

    GlobalVector propagated_ecal_position(
        prop.particle().vertex().x(),
        prop.particle().vertex().y(),
        prop.particle().vertex().z()
    );

    p2rctIO::SimpleCaloHit genHit;
    genHit.setPosition(propagated_ecal_position);
    genHit.setEt(p.pt());

    int genRctCard = -1;
    int genCrystalIEta = -999;
    int genCrystalIPhi = -999;
    int genLocalIEta = -999;
    int genLocalIPhi = -999;

    for (int cc = 0; cc < p2rctIO::N_CARDS; ++cc) {
      if (genHit.isInCard(cc)) {
        genRctCard = cc;

        genCrystalIEta = genHit.crystaliEta();
        genCrystalIPhi = genHit.crystaliPhi();

        genLocalIEta = genHit.crystalLocaliEta(cc);
        genLocalIPhi = genHit.crystalLocaliPhi(cc);

        break;
      }
    }

    if (genRctCard >= 0) nAssignedToRctCard++;

    out << iEvent.id().run() << ","
        << iEvent.id().luminosityBlock() << ","
        << iEvent.id().event() << ","
        << iGen << ","
        << pdgId << ","
        << p.status() << ","
        << p.pt() << ","
        << p.eta() << ","
        << p.phi() << ","
        << p.energy() << ","
        << p.vertex().x() << ","
        << p.vertex().y() << ","
        << p.vertex().z() << ","
        << prop.particle().Pt() << ","
        << propagated_ecal_position.eta() << ","
        << propagated_ecal_position.phi() << ","
        << genRctCard << ","
        << genCrystalIEta << ","
        << genCrystalIPhi << ","
        << genLocalIEta << ","
        << genLocalIPhi
        << "\n";
  }

  std::cout << "GEN information for event: " << iEvent.id().event()
            << " | nGenParticles = " << nGenParticles
            << " | nGenElectrons = " << nGenElectrons
            << " | nEtaPreselected = " << nEtaPreselected
            << " | nPropagated = " << nPropagated
            << " | nAssignedToRctCard = " << nAssignedToRctCard
            << std::endl;
}



}  

#endif