#ifndef L1Trigger_L1CaloTrigger_HFDumpUtils_h
#define L1Trigger_L1CaloTrigger_HFDumpUtils_h

#include <ap_int.h>

#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/Exception.h"

#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

#include "CommonTools/BaseParticlePropagator/interface/BaseParticlePropagator.h"
#include "CommonTools/BaseParticlePropagator/interface/RawParticle.h"
#include "L1Trigger/L1CaloTrigger/interface/HF_IP1/algo_topIP1_h.h"

namespace hfdump {

// Do not use N_HF_REGIONS here.
// IP1 uses 24 internal regions, while IP2 uses 6 sectors.
static constexpr int N_SECTORS = 6;

static constexpr int N_IP1_INPUT_LINKS = 18;
static constexpr int N_IP1_INPUT_LINKS_PER_SECTOR = 3;

static constexpr int N_IP1_OUTPUT_LINKS = 9;
static constexpr int N_IP1_CLUSTER_LINKS = 6;

static constexpr int IP1_JET_LINK = 6;
static constexpr int IP1_TAU_LINK = 7;
static constexpr int IP1_SUM_LINK = 8;

static constexpr int N_IP1_GLOBAL_OBJECTS = 6;

static constexpr int N_IP2_LINKS = 6;

static constexpr int N_IP1_CLUSTERS_PER_SECTOR = 6;
static constexpr int N_IP2_OBJECTS_PER_SECTOR = 8;

static constexpr int N_TOWERS_ETA = 12;
static constexpr int N_TOWERS_PHI = 72;

static constexpr double PI = 3.14159265358979323846;


// -----------------------------------------------------------------------------
// HF side
// -----------------------------------------------------------------------------

enum class HFSide {
  Plus,
  Minus
};

inline std::string sideName(HFSide side) {
  return side == HFSide::Plus ? "HFPlus" : "HFMinus";
}

inline int sideSign(HFSide side) {
  return side == HFSide::Plus ? 1 : -1;
}


// -----------------------------------------------------------------------------
// Directory helpers
// -----------------------------------------------------------------------------

inline void ensureDir(const std::string& dir) {
  const std::string command = "mkdir -p \"" + dir + "\"";

  const int returnCode = std::system(command.c_str());

  if (returnCode != 0) {
    throw cms::Exception("Phase2L1CaloL1HFEmulator")
        << "Failed to create directory: " << dir;
  }
}

inline std::ofstream openOutputFile(
    const std::string& fileName
) {
  std::ofstream out(fileName);

  if (!out.is_open()) {
    throw cms::Exception(
        "Phase2L1CaloL1HFEmulator"
    ) << "Could not open file: "
      << fileName;
  }

  return out;
}

struct DumpDirs {
  std::string base;

  std::string input;
  std::string output;
  std::string decoded;
  std::string towers;

  std::string inputIP1;
  std::string inputIP2;

  std::string outputIP1;
  std::string outputIP2;

  std::string decodedIP1;
  std::string decodedIP2;
};


inline DumpDirs makeDumpDirs(
    unsigned long long eventId,
    HFSide side,
    int sector
) {
  DumpDirs dirs;

  dirs.base =
      "hf_IO/event_" + std::to_string(eventId) +
      "/" + sideName(side) +
      "/sector_" + std::to_string(sector);

  dirs.input = dirs.base + "/input";
  dirs.output = dirs.base + "/output";
  dirs.decoded = dirs.base + "/decoded";
  dirs.towers = dirs.base + "/towers";

  dirs.inputIP1 = dirs.input + "/IP1";
  dirs.inputIP2 = dirs.input + "/IP2";

  dirs.outputIP1 = dirs.output + "/IP1";
  dirs.outputIP2 = dirs.output + "/IP2";

  dirs.decodedIP1 = dirs.decoded + "/IP1";
  dirs.decodedIP2 = dirs.decoded + "/IP2";

  return dirs;
}


inline void ensureDumpDirs(const DumpDirs& dirs) {
  ensureDir(dirs.base);

  ensureDir(dirs.input);
  ensureDir(dirs.output);
  ensureDir(dirs.decoded);
  ensureDir(dirs.towers);

  ensureDir(dirs.inputIP1);
  ensureDir(dirs.inputIP2);

  ensureDir(dirs.outputIP1);
  ensureDir(dirs.outputIP2);

  ensureDir(dirs.decodedIP1);
  ensureDir(dirs.decodedIP2);
}

struct GlobalDumpDirs {
  std::string sideBase;
  std::string global;
  std::string globalIP1;
  std::string globalIP1Raw;
  std::string globalIP1Decoded;
};


inline GlobalDumpDirs makeGlobalDumpDirs(
    unsigned long long eventId,
    HFSide side
) {
  GlobalDumpDirs dirs;

  dirs.sideBase =
      "hf_IO/event_" +
      std::to_string(eventId) +
      "/" +
      sideName(side);

  dirs.global =
      dirs.sideBase + "/global";

  dirs.globalIP1 =
      dirs.global + "/IP1";

  dirs.globalIP1Raw =
      dirs.globalIP1 + "/raw";

  dirs.globalIP1Decoded =
      dirs.globalIP1 + "/decoded";

  return dirs;
}


inline void ensureGlobalDumpDirs(
    const GlobalDumpDirs& dirs
) {
  ensureDir(dirs.sideBase);
  ensureDir(dirs.global);
  ensureDir(dirs.globalIP1);
  ensureDir(dirs.globalIP1Raw);
  ensureDir(dirs.globalIP1Decoded);
}


// -----------------------------------------------------------------------------
// Formatting helpers
// -----------------------------------------------------------------------------

inline std::string hexIndex(unsigned int index) {
  std::ostringstream stream;

  stream << std::hex
         << std::nouppercase
         << index;

  return stream.str();
}


template <int N>
inline std::string formatApUint(const ap_uint<N>& value) {
  if (value == 0) {
    return "0";
  }

  std::string text = value.to_string(16);

  if (
      text.rfind("0x", 0) == 0 ||
      text.rfind("0X", 0) == 0
  ) {
    return text;
  }

  return "0x" + text;
}


inline double wrapPhi(double phi) {
  while (phi >= PI) {
    phi -= 2.0 * PI;
  }

  while (phi < -PI) {
    phi += 2.0 * PI;
  }

  return phi;
}


// Approximate conversion from physical phi to the 72-bin HCAL iphi index.
inline int phiToIPhi(double phi) {
  const double wrappedPhi = wrapPhi(phi);

  int iphi = static_cast<int>(
      std::floor(
          (wrappedPhi + PI) /
          (2.0 * PI) *
          72.0
      )
  ) + 1;

  if (iphi < 1) {
    iphi = 1;
  }

  if (iphi > 72) {
    iphi = 72;
  }

  return iphi;
}


inline int iphiToSector(int iphi) {
  int normalized = iphi;

  while (normalized < 1) {
    normalized += 72;
  }

  while (normalized > 72) {
    normalized -= 72;
  }

  return (normalized - 1) / 12;
}


// -----------------------------------------------------------------------------
// Decoded object formats
// -----------------------------------------------------------------------------

struct HFTowerDecoded {
  ap_uint<8> energy;
  ap_uint<2> fineGrain;
};


struct IP1ClusterDecoded {
  ap_uint<12> pt;
  ap_int<8> eta;
  ap_int<7> phi;
  ap_uint<4> hoe;
  ap_uint<12> emPt;
  ap_uint<12> auxiliary;
  ap_uint<9> spare;
};

struct IP1GlobalObjectDecoded {
  ap_uint<12> pt;
  ap_int<8> eta;
  ap_int<7> phi;
  ap_uint<4> flags;
  ap_uint<12> ecal;
  ap_uint<12> seedEt;
  ap_uint<9> spare;
};


struct IP2HadCaloDecoded {
  ap_uint<12> pt;
  ap_int<8> eta;
  ap_int<7> phi;
  ap_uint<4> hoe;
  ap_uint<12> emPt;
  ap_uint<21> unused;
};


struct IP2PuppiDecoded {
  ap_uint<14> pt;
  ap_int<12> eta;
  ap_int<11> phi;
  ap_uint<3> particleId;
  ap_uint<24> data;
};


// -----------------------------------------------------------------------------
// Unpackers
// -----------------------------------------------------------------------------

inline HFTowerDecoded unpackHFTower(
    const ap_uint<10>& word
) {
  HFTowerDecoded tower;

  tower.energy = word.range(7, 0);
  tower.fineGrain = word.range(9, 8);

  return tower;
}


inline IP1ClusterDecoded unpackIP1Cluster(
    const ap_uint<64>& word
) {
  IP1ClusterDecoded cluster;

  cluster.pt = word.range(11, 0);

  ap_int<8> eta;
  eta.range(7, 0) = word.range(19, 12);
  cluster.eta = eta;

  ap_int<7> phi;
  phi.range(6, 0) = word.range(26, 20);
  cluster.phi = phi;

  cluster.hoe = word.range(30, 27);
  cluster.emPt = word.range(42, 31);
  cluster.auxiliary = word.range(54, 43);
  cluster.spare = word.range(63, 55);

  return cluster;
}

inline IP1GlobalObjectDecoded unpackIP1GlobalObject(
    const ap_uint<64>& word
) {
  IP1GlobalObjectDecoded object;

  object.pt = word.range(11, 0);

  ap_int<8> eta;
  eta.range(7, 0) = word.range(19, 12);
  object.eta = eta;

  ap_int<7> phi;
  phi.range(6, 0) = word.range(26, 20);
  object.phi = phi;

  object.flags = word.range(30, 27);
  object.ecal = word.range(42, 31);
  object.seedEt = word.range(54, 43);
  object.spare = word.range(63, 55);

  return object;
}

inline void dumpIP1JetTauGlobalCoordinatesCSV(
    std::ofstream& out,
    unsigned long long eventId,
    HFSide side,
    const ap_uint<576> ip1Output[N_IP1_OUTPUT_LINKS],
    const HFGlobalCoordDebug jetGlobal[N_JETS],
    const HFGlobalCoordDebug tauGlobal[N_TAUS]
) {
    out << "event,side,object_type,index,"
        << "pt_raw,"
        << "local_eta,local_phi,"
        << "global_eta_st,global_phi_st\n";


    // Jets
    for (int i = 0; i < N_JETS; ++i) {

        const int start = 64 * i;

        const IP1GlobalObjectDecoded object =
            unpackIP1GlobalObject(
                ip1Output[IP1_JET_LINK].range(
                    start + 63,
                    start
                )
            );

        out << eventId << ","
            << sideName(side) << ","
            << "jet" << ","
            << i << ","
            << static_cast<unsigned>(object.pt) << ","
            << object.eta.to_int() << ","
            << object.phi.to_int() << ","
            << jetGlobal[i].eta << ","
            << jetGlobal[i].phi
            << "\n";
    }


    // Taus
    for (int i = 0; i < N_TAUS; ++i) {

        const int start = 64 * i;

        const IP1GlobalObjectDecoded object =
            unpackIP1GlobalObject(
                ip1Output[IP1_TAU_LINK].range(
                    start + 63,
                    start
                )
            );

        out << eventId << ","
            << sideName(side) << ","
            << "tau" << ","
            << i << ","
            << static_cast<unsigned>(object.pt) << ","
            << object.eta.to_int() << ","
            << object.phi.to_int() << ","
            << tauGlobal[i].eta << ","
            << tauGlobal[i].phi
            << "\n";
    }
}



inline IP2HadCaloDecoded unpackIP2HadCalo(
    const ap_uint<64>& word
) {
  IP2HadCaloDecoded object;

  object.pt = word.range(11, 0);

  ap_int<8> eta;
  eta.range(7, 0) = word.range(19, 12);
  object.eta = eta;

  ap_int<7> phi;
  phi.range(6, 0) = word.range(26, 20);
  object.phi = phi;

  object.hoe = word.range(30, 27);
  object.emPt = word.range(42, 31);
  object.unused = word.range(63, 43);

  return object;
}


inline IP2PuppiDecoded unpackIP2Puppi(
    const ap_uint<64>& word
) {
  IP2PuppiDecoded object;

  object.pt = word.range(13, 0);

  ap_int<12> eta;
  eta.range(11, 0) = word.range(25, 14);
  object.eta = eta;

  ap_int<11> phi;
  phi.range(10, 0) = word.range(36, 26);
  object.phi = phi;

  object.particleId = word.range(39, 37);
  object.data = word.range(63, 40);

  return object;
}


// -----------------------------------------------------------------------------
// Raw-link dumping
// -----------------------------------------------------------------------------

inline void dumpRawLinksCSV(
    std::ofstream& out,
    unsigned long long eventId,
    HFSide side,
    int sector,
    const std::string& component,
    const std::string& direction,
    const ap_uint<576>* links,
    const std::vector<int>& linkIndices
) {
  out << "event,side,sector,"
      << "component,direction,"
      << "local_link_index,"
      << "global_link_index_dec,"
      << "global_link_index_hex,"
      << "raw_hex\n";

  for (
      std::size_t localIndex = 0;
      localIndex < linkIndices.size();
      ++localIndex
  ) {
    const int globalIndex = linkIndices.at(localIndex);

    out << eventId << ","
        << sideName(side) << ","
        << sector << ","
        << component << ","
        << direction << ","
        << localIndex << ","
        << globalIndex << ","
        << hexIndex(globalIndex) << ","
        << formatApUint(links[globalIndex])
        << "\n";
  }
}


inline void writeRawLinksCSVFile(
    const std::string& directory,
    unsigned long long eventId,
    HFSide side,
    int sector,
    const std::string& component,
    const std::string& direction,
    const ap_uint<576>* links,
    const std::vector<int>& linkIndices
) {
  const std::string fileName =
      directory +
      "/event_" + std::to_string(eventId) +
      "_" + sideName(side) +
      "_sector_" + std::to_string(sector) +
      "_" + component +
      "_" + direction +
      "_links.csv";

  std::ofstream out =
    openOutputFile(fileName);

  dumpRawLinksCSV(
      out,
      eventId,
      side,
      sector,
      component,
      direction,
      links,
      linkIndices
  );
}

inline void dumpIP1GlobalRawCSV(
    std::ofstream& out,
    unsigned long long eventId,
    HFSide side,
    const ap_uint<576> ip1Output[N_IP1_OUTPUT_LINKS]
) {
  out << "event,side,"
      << "link_index,object_type,raw_hex\n";

  out << eventId << ","
      << sideName(side) << ","
      << IP1_JET_LINK << ","
      << "jets" << ","
      << formatApUint(ip1Output[IP1_JET_LINK])
      << "\n";

  out << eventId << ","
      << sideName(side) << ","
      << IP1_TAU_LINK << ","
      << "taus" << ","
      << formatApUint(ip1Output[IP1_TAU_LINK])
      << "\n";

  out << eventId << ","
      << sideName(side) << ","
      << IP1_SUM_LINK << ","
      << "sums" << ","
      << formatApUint(ip1Output[IP1_SUM_LINK])
      << "\n";
}


// -----------------------------------------------------------------------------
// Original trigger-tower grid
// -----------------------------------------------------------------------------

inline void dumpHFTowersCSV(
    std::ofstream& out,
    unsigned long long eventId,
    HFSide side,
    int sector,
    const ap_uint<10> towers[N_TOWERS_ETA][N_TOWERS_PHI]
) {
  out << "event,side,sector,"
      << "eta_index,phi_index,"
      << "ieta,iphi,"
      << "raw_hex,energy,fine_grain\n";

  const int phiStart = sector * 12;
  const int phiEnd = phiStart + 12;

  for (int etaIndex = 0;
       etaIndex < N_TOWERS_ETA;
       ++etaIndex) {
    for (int phiIndex = phiStart;
         phiIndex < phiEnd;
         ++phiIndex) {
      const ap_uint<10> raw =
          towers[etaIndex][phiIndex];

      const HFTowerDecoded tower =
          unpackHFTower(raw);

      const int absIEta = etaIndex + 30;
      const int ieta = sideSign(side) * absIEta;
      const int iphi = phiIndex + 1;

      out << eventId << ","
          << sideName(side) << ","
          << sector << ","
          << etaIndex << ","
          << phiIndex << ","
          << ieta << ","
          << iphi << ","
          << formatApUint(raw) << ","
          << static_cast<unsigned>(tower.energy) << ","
          << static_cast<unsigned>(tower.fineGrain)
          << "\n";
    }
  }
}


// -----------------------------------------------------------------------------
// IP1 input decoding
// -----------------------------------------------------------------------------

inline void dumpDecodedIP1InputCSV(
    std::ofstream& out,
    unsigned long long eventId,
    HFSide side,
    int sector,
    const ap_uint<576>* links
) {
  out << "event,side,sector,"
      << "local_link_index,global_link_index,"
      << "link_side,word_index,"
      << "eta_index,phi_index,"
      << "ieta,iphi,"
      << "raw_hex,energy,fine_grain\n";

  for (int localLink = 0;
       localLink < N_IP1_INPUT_LINKS_PER_SECTOR;
       ++localLink) {
    const int globalLink =
        sector * N_IP1_INPUT_LINKS_PER_SECTOR +
        localLink;

    const int phiBase =
        sector * 12 +
        localLink * 4;

    for (int etaIndex = 0;
         etaIndex < 10;
         ++etaIndex) {
      const int start = etaIndex * 10;

      const ap_uint<10> raw =
          links[globalLink].range(
              start + 9,
              start
          );

      const HFTowerDecoded tower =
          unpackHFTower(raw);

      const int ieta =
          sideSign(side) *
          (etaIndex + 30);

      const int iphi =
          phiBase + 1;

      out << eventId << ","
          << sideName(side) << ","
          << sector << ","
          << localLink << ","
          << globalLink << ","
          << "A" << ","
          << etaIndex << ","
          << etaIndex << ","
          << phiBase << ","
          << ieta << ","
          << iphi << ","
          << formatApUint(raw) << ","
          << static_cast<unsigned>(tower.energy) << ","
          << static_cast<unsigned>(tower.fineGrain)
          << "\n";
    }

    {
      const ap_uint<10> raw =
          links[globalLink].range(109, 100);

      const HFTowerDecoded tower =
          unpackHFTower(raw);

      out << eventId << ","
          << sideName(side) << ","
          << sector << ","
          << localLink << ","
          << globalLink << ","
          << "A" << ","
          << 10 << ","
          << 10 << ","
          << phiBase << ","
          << sideSign(side) * 40 << ","
          << phiBase + 1 << ","
          << formatApUint(raw) << ","
          << static_cast<unsigned>(tower.energy) << ","
          << static_cast<unsigned>(tower.fineGrain)
          << "\n";
    }

    for (int etaIndex = 0;
         etaIndex < 10;
         ++etaIndex) {
      const int start =
          110 + etaIndex * 10;

      const ap_uint<10> raw =
          links[globalLink].range(
              start + 9,
              start
          );

      const HFTowerDecoded tower =
          unpackHFTower(raw);

      const int ieta =
          sideSign(side) *
          (etaIndex + 30);

      const int iphi =
          phiBase + 3;

      out << eventId << ","
          << sideName(side) << ","
          << sector << ","
          << localLink << ","
          << globalLink << ","
          << "B" << ","
          << etaIndex << ","
          << etaIndex << ","
          << phiBase + 2 << ","
          << ieta << ","
          << iphi << ","
          << formatApUint(raw) << ","
          << static_cast<unsigned>(tower.energy) << ","
          << static_cast<unsigned>(tower.fineGrain)
          << "\n";
    }

    {
      const ap_uint<10> raw =
          links[globalLink].range(219, 210);

      const HFTowerDecoded tower =
          unpackHFTower(raw);

      out << eventId << ","
          << sideName(side) << ","
          << sector << ","
          << localLink << ","
          << globalLink << ","
          << "B" << ","
          << 10 << ","
          << 11 << ","
          << phiBase + 2 << ","
          << sideSign(side) * 41 << ","
          << phiBase + 3 << ","
          << formatApUint(raw) << ","
          << static_cast<unsigned>(tower.energy) << ","
          << static_cast<unsigned>(tower.fineGrain)
          << "\n";
    }
  }
}


// -----------------------------------------------------------------------------
// IP1 PF-cluster output decoding
// -----------------------------------------------------------------------------

inline void dumpDecodedIP1OutputCSV(
    std::ofstream& out,
    unsigned long long eventId,
    HFSide side,
    int sector,
    const ap_uint<576>& link
) {
  out << "event,side,sector,"
      << "link_index,object_type,index,raw_hex,"
      << "pt_raw,eta_raw,phi_raw,"
      << "hoe_raw,em_pt_raw,"
      << "auxiliary_raw,spare\n";

  for (int index = 0;
       index < N_IP1_CLUSTERS_PER_SECTOR;
       ++index) {
    const int start = index * 64;

    const ap_uint<64> raw =
        link.range(start + 63, start);

    const IP1ClusterDecoded cluster =
        unpackIP1Cluster(raw);

    out << eventId << ","
        << sideName(side) << ","
        << sector << ","
        << sector << ","
        << "pfcluster" << ","
        << index << ","
        << formatApUint(raw) << ","
        << static_cast<unsigned>(cluster.pt) << ","
        << cluster.eta.to_int() << ","
        << cluster.phi.to_int() << ","
        << static_cast<unsigned>(cluster.hoe) << ","
        << static_cast<unsigned>(cluster.emPt) << ","
        << static_cast<unsigned>(cluster.auxiliary) << ","
        << static_cast<unsigned>(cluster.spare)
        << "\n";
  }
}

inline void dumpDecodedIP1GlobalObjectsCSV(
    std::ofstream& out,
    unsigned long long eventId,
    HFSide side,
    int linkIndex,
    const std::string& objectType,
    const ap_uint<576>& link,
    int numberOfObjects
) {
  out << "event,side,"
      << "link_index,object_type,index,raw_hex,"
      << "pt_raw,eta_raw,phi_raw,"
      << "flags_raw,ecal_raw,seed_et_raw,spare\n";

  for (int index = 0;
       index < numberOfObjects;
       ++index) {
    const int start = index * 64;

    const ap_uint<64> raw =
        link.range(start + 63, start);

    const IP1GlobalObjectDecoded object =
        unpackIP1GlobalObject(raw);

    out << eventId << ","
        << sideName(side) << ","
        << linkIndex << ","
        << objectType << ","
        << index << ","
        << formatApUint(raw) << ","
        << static_cast<unsigned>(object.pt) << ","
        << object.eta.to_int() << ","
        << object.phi.to_int() << ","
        << static_cast<unsigned>(object.flags) << ","
        << static_cast<unsigned>(object.ecal) << ","
        << static_cast<unsigned>(object.seedEt) << ","
        << static_cast<unsigned>(object.spare)
        << "\n";
  }
}

inline void dumpDecodedIP1SumCSV(
    std::ofstream& out,
    unsigned long long eventId,
    HFSide side,
    const ap_uint<576>& link
) {
  const ap_uint<64> raw =
      link.range(63, 0);

  const IP1GlobalObjectDecoded object =
      unpackIP1GlobalObject(raw);

  bool allCopiesMatch = true;

  for (int index = 1;
       index < N_IP1_GLOBAL_OBJECTS;
       ++index) {
    const int start = index * 64;

    const ap_uint<64> copy =
        link.range(start + 63, start);

    if (copy != raw) {
      allCopiesMatch = false;
    }
  }

  out << "event,side,"
      << "link_index,object_type,raw_hex,"
      << "pt_raw,eta_raw,phi_raw,"
      << "flags_raw,ecal_raw,seed_et_raw,spare,"
      << "all_six_copies_match\n";

  out << eventId << ","
      << sideName(side) << ","
      << IP1_SUM_LINK << ","
      << "sum" << ","
      << formatApUint(raw) << ","
      << static_cast<unsigned>(object.pt) << ","
      << object.eta.to_int() << ","
      << object.phi.to_int() << ","
      << static_cast<unsigned>(object.flags) << ","
      << static_cast<unsigned>(object.ecal) << ","
      << static_cast<unsigned>(object.seedEt) << ","
      << static_cast<unsigned>(object.spare) << ","
      << static_cast<int>(allCopiesMatch)
      << "\n";
}

inline void dumpIP1JetTauGlobalCoordinates(
    const edm::Event& event,
    HFSide side,
    const ap_uint<576> ip1Output[N_IP1_OUTPUT_LINKS],
    const HFGlobalCoordDebug jetGlobal[N_JETS],
    const HFGlobalCoordDebug tauGlobal[N_TAUS]
) {
    const unsigned long long eventId =
        event.id().event();

    const GlobalDumpDirs dirs =
        makeGlobalDumpDirs(eventId, side);

    ensureGlobalDumpDirs(dirs);

    const std::string fileName =
        dirs.globalIP1Decoded +
        "/event_" +
        std::to_string(eventId) +
        "_" +
        sideName(side) +
        "_IP1_jets_taus_global_coordinates.csv";

    std::ofstream out =
        openOutputFile(fileName);

    dumpIP1JetTauGlobalCoordinatesCSV(
        out,
        eventId,
        side,
        ip1Output,
        jetGlobal,
        tauGlobal
    );
}
// -----------------------------------------------------------------------------
// IP2 input decoding
// -----------------------------------------------------------------------------

inline void dumpDecodedIP2InputCSV(
    std::ofstream& out,
    unsigned long long eventId,
    HFSide side,
    int sector,
    const ap_uint<576>& link
) {
  out << "event,side,sector,"
      << "link_index,object_type,index,raw_hex,"
      << "pt_raw,eta_raw,phi_raw,"
      << "hoe_raw,em_pt_raw,unused_raw_hex\n";

  for (int index = 0;
       index < N_IP2_OBJECTS_PER_SECTOR;
       ++index) {
    const int start = index * 64;

    const ap_uint<64> raw =
        link.range(start + 63, start);

    const IP2HadCaloDecoded object =
        unpackIP2HadCalo(raw);

    out << eventId << ","
        << sideName(side) << ","
        << sector << ","
        << sector << ","
        << "hadcalo" << ","
        << index << ","
        << formatApUint(raw) << ","
        << static_cast<unsigned>(object.pt) << ","
        << object.eta.to_int() << ","
        << object.phi.to_int() << ","
        << static_cast<unsigned>(object.hoe) << ","
        << static_cast<unsigned>(object.emPt) << ","
        << formatApUint(object.unused)
        << "\n";
  }
}


// -----------------------------------------------------------------------------
// IP2 PUPPI output decoding
// -----------------------------------------------------------------------------

inline void dumpDecodedIP2OutputCSV(
    std::ofstream& out,
    unsigned long long eventId,
    HFSide side,
    int sector,
    const ap_uint<576>& link
) {
  out << "event,side,sector,"
      << "link_index,object_type,index,raw_hex,"
      << "pt_raw,eta_raw,phi_raw,"
      << "particle_id,data_raw_hex\n";

  for (int index = 0;
       index < N_IP2_OBJECTS_PER_SECTOR;
       ++index) {
    const int start = index * 64;

    const ap_uint<64> raw =
        link.range(start + 63, start);

    const IP2PuppiDecoded object =
        unpackIP2Puppi(raw);

    out << eventId << ","
        << sideName(side) << ","
        << sector << ","
        << sector << ","
        << "puppi" << ","
        << index << ","
        << formatApUint(raw) << ","
        << static_cast<unsigned>(object.pt) << ","
        << object.eta.to_int() << ","
        << object.phi.to_int() << ","
        << static_cast<unsigned>(object.particleId) << ","
        << formatApUint(object.data)
        << "\n";
  }
}


// -----------------------------------------------------------------------------
// GEN jets
// -----------------------------------------------------------------------------

inline void dumpGenJetsCSV(
    std::ofstream& out,
    const edm::Event& event,
    const edm::EDGetTokenT<reco::GenJetCollection>& genJetToken,
    HFSide side,
    double minPt = 10.0,
    double etaMin = 3.0,
    double etaMax = 5.2
) {
  out << "run,lumi,event,"
      << "side,"
      << "genjet_index,"
      << "pt,eta,phi,energy,mass,"
      << "px,py,pz,n_constituents,"
      << "iphi,sector,"
      << "passes_pt,"
      << "passes_eta,"
      << "selected\n";

  edm::Handle<reco::GenJetCollection> genJets;

  event.getByToken(
      genJetToken,
      genJets
  );

  if (!genJets.isValid()) {
    std::cout
        << "WARNING: GEN-jet collection not found for event "
        << event.id().event()
        << std::endl;

    return;
  }

  for (std::size_t index = 0;
       index < genJets->size();
       ++index) {
    const reco::GenJet& jet =
        genJets->at(index);

    const bool passesPt =
        jet.pt() >= minPt;

    bool passesEta = false;

    if (side == HFSide::Plus) {
      passesEta =
          jet.eta() >= etaMin &&
          jet.eta() <= etaMax;
    } else {
      passesEta =
          jet.eta() <= -etaMin &&
          jet.eta() >= -etaMax;
    }

    const int iphi =
        phiToIPhi(jet.phi());

    const int sector =
        iphiToSector(iphi);

    const bool selected =
        passesPt &&
        passesEta;

    out << event.id().run() << ","
        << event.id().luminosityBlock() << ","
        << event.id().event() << ","
        << sideName(side) << ","
        << index << ","
        << jet.pt() << ","
        << jet.eta() << ","
        << jet.phi() << ","
        << jet.energy() << ","
        << jet.mass() << ","
        << jet.px() << ","
        << jet.py() << ","
        << jet.pz() << ","
        << jet.numberOfDaughters() << ","
        << iphi << ","
        << sector << ","
        << static_cast<int>(passesPt) << ","
        << static_cast<int>(passesEta) << ","
        << static_cast<int>(selected)
        << "\n";
  }
}


// -----------------------------------------------------------------------------
// GEN particles used by the reference analyzer
// -----------------------------------------------------------------------------

struct PropagatedGenParticle {
  bool attempted = false;
  bool success = false;
  int successCode = 0;
  double pt = -999.0;
  double eta = -999.0;
  double phi = -999.0;
  double energy = -999.0;
  double x = -999.0;
  double y = -999.0;
  double z = -999.0;
};

// Propagate a stable charged GEN particle to the VFCAL (HF) entrance.
// BaseParticlePropagator does not model dE/dx, bremsstrahlung,
// multiple scattering, or hadronic interactions; this is a geometric
// propagation in the magnetic field, analogous to the electron propagation
// used by L1TCaloEGammaAnalyzer but with propagateToVFcalEntrance().
inline PropagatedGenParticle propagateToHFEntrance(
    const reco::GenParticle& genParticle,
    double magneticFieldTesla = 4.0
) {
  PropagatedGenParticle result;

  // Propagation to the detector is meaningful for final-state particles.
  if (genParticle.status() != 1) {
    return result;
  }

  result.attempted = true;

  RawParticle particle(genParticle.p4());
  particle.setVertex(
      genParticle.vertex().x(),
      genParticle.vertex().y(),
      genParticle.vertex().z(),
      0.0
  );
  particle.setMass(genParticle.mass());
  particle.setCharge(genParticle.charge());

  BaseParticlePropagator propagator(
      particle,
      0.0,
      0.0,
      magneticFieldTesla
  );

  const bool propagated = propagator.propagateToVFcalEntrance();
  result.successCode = propagator.getSuccess();
  result.success = propagated && result.successCode > 0;

  if (!result.success) {
    return result;
  }

  const RawParticle& propagatedParticle = propagator.particle();

  result.pt = propagatedParticle.Pt();
  result.eta = propagatedParticle.eta();
  result.phi = propagatedParticle.phi();
  result.energy = propagatedParticle.E();
  result.x = propagatedParticle.X();
  result.y = propagatedParticle.Y();
  result.z = propagatedParticle.Z();

  return result;
}

inline bool passesHFSideEta(
    double eta,
    HFSide side,
    double etaMin,
    double etaMax
) {
  if (side == HFSide::Plus) {
    return eta >= etaMin && eta <= etaMax;
  }

  return eta <= -etaMin && eta >= -etaMax;
}

inline void dumpGenPionsCSV(
    std::ofstream& out,
    const edm::Event& event,
    const edm::EDGetTokenT<reco::GenParticleCollection>& genParticleToken,
    HFSide side,
    double minPt = 0.0,
    double etaMin = 3.0,
    double etaMax = 5.2,
    double magneticFieldTesla = 4.0
) {
  out << "run,lumi,event,side,"
      << "gen_index,pdg_id,status,charge,"
      << "pt,eta,phi,energy,mass,"
      << "vx,vy,vz,"
      << "propagation_attempted,propagation_success,propagation_success_code,"
      << "prop_pt,prop_eta,prop_phi,prop_energy,prop_x,prop_y,prop_z,"
      << "prop_iphi,prop_sector,"
      << "passes_pt,passes_original_eta,passes_propagated_eta,selected\n";

  edm::Handle<reco::GenParticleCollection> genParticles;
  event.getByToken(genParticleToken, genParticles);

  if (!genParticles.isValid()) {
    std::cout << "WARNING: GEN-particle collection not found for event "
              << event.id().event() << std::endl;
    return;
  }

  for (std::size_t index = 0; index < genParticles->size(); ++index) {
    const reco::GenParticle& p = genParticles->at(index);

    if (std::abs(p.pdgId()) != 211) {
      continue;
    }

    const PropagatedGenParticle propagated =
        propagateToHFEntrance(p, magneticFieldTesla);

    const bool passesPt = p.pt() >= minPt;
    const bool passesOriginalEta =
        passesHFSideEta(p.eta(), side, etaMin, etaMax);
    const bool passesPropagatedEta =
        propagated.success &&
        passesHFSideEta(propagated.eta, side, etaMin, etaMax);

    const int propagatedIPhi =
        propagated.success ? phiToIPhi(propagated.phi) : -1;
    const int propagatedSector =
        propagated.success ? iphiToSector(propagatedIPhi) : -1;

    // For detector-level matching use the propagated HF intersection.
    const bool selected =
        passesPt &&
        p.status() == 1 &&
        passesPropagatedEta;

    out << event.id().run() << ","
        << event.id().luminosityBlock() << ","
        << event.id().event() << ","
        << sideName(side) << ","
        << index << ","
        << p.pdgId() << ","
        << p.status() << ","
        << p.charge() << ","
        << p.pt() << ","
        << p.eta() << ","
        << p.phi() << ","
        << p.energy() << ","
        << p.mass() << ","
        << p.vertex().x() << ","
        << p.vertex().y() << ","
        << p.vertex().z() << ","
        << static_cast<int>(propagated.attempted) << ","
        << static_cast<int>(propagated.success) << ","
        << propagated.successCode << ","
        << propagated.pt << ","
        << propagated.eta << ","
        << propagated.phi << ","
        << propagated.energy << ","
        << propagated.x << ","
        << propagated.y << ","
        << propagated.z << ","
        << propagatedIPhi << ","
        << propagatedSector << ","
        << static_cast<int>(passesPt) << ","
        << static_cast<int>(passesOriginalEta) << ","
        << static_cast<int>(passesPropagatedEta) << ","
        << static_cast<int>(selected)
        << "\n";
  }
}

inline void dumpGenElectronsCSV(
    std::ofstream& out,
    const edm::Event& event,
    const edm::EDGetTokenT<reco::GenParticleCollection>& genParticleToken,
    HFSide side,
    double minPt = 0.0,
    double etaMin = 3.0,
    double etaMax = 5.2,
    double magneticFieldTesla = 4.0
) {
  out << "run,lumi,event,side,"
      << "gen_index,pdg_id,status,charge,"
      << "pt,eta,phi,energy,mass,"
      << "propagation_attempted,propagation_success,propagation_success_code,"
      << "prop_pt,prop_eta,prop_phi,prop_energy,"
      << "prop_iphi,prop_sector,"
      << "passes_pt,passes_original_eta,passes_propagated_eta,selected\n";

  edm::Handle<reco::GenParticleCollection> genParticles;
  event.getByToken(genParticleToken, genParticles);

  if (!genParticles.isValid()) {
    std::cout << "WARNING: GEN-particle collection not found for event "
              << event.id().event() << std::endl;
    return;
  }

  for (std::size_t index = 0; index < genParticles->size(); ++index) {
    const reco::GenParticle& p = genParticles->at(index);

    if (std::abs(p.pdgId()) != 11) {
      continue;
    }

    const PropagatedGenParticle propagated =
        propagateToHFEntrance(p, magneticFieldTesla);

    const bool passesPt = p.pt() >= minPt;
    const bool passesOriginalEta =
        passesHFSideEta(p.eta(), side, etaMin, etaMax);
    const bool passesPropagatedEta =
        propagated.success &&
        passesHFSideEta(propagated.eta, side, etaMin, etaMax);

    const int propagatedIPhi =
        propagated.success ? phiToIPhi(propagated.phi) : -1;
    const int propagatedSector =
        propagated.success ? iphiToSector(propagatedIPhi) : -1;

    const bool selected =
        passesPt &&
        p.status() == 1 &&
        passesPropagatedEta;

    out << event.id().run() << ","
        << event.id().luminosityBlock() << ","
        << event.id().event() << ","
        << sideName(side) << ","
        << index << ","
        << p.pdgId() << ","
        << p.status() << ","
        << p.charge() << ","
        << p.pt() << ","
        << p.eta() << ","
        << p.phi() << ","
        << p.energy() << ","
        << p.mass() << ","
        << static_cast<int>(propagated.attempted) << ","
        << static_cast<int>(propagated.success) << ","
        << propagated.successCode << ","
        << propagated.pt << ","
        << propagated.eta << ","
        << propagated.phi << ","
        << propagated.energy << ","
        << propagatedIPhi << ","
        << propagatedSector << ","
        << static_cast<int>(passesPt) << ","
        << static_cast<int>(passesOriginalEta) << ","
        << static_cast<int>(passesPropagatedEta) << ","
        << static_cast<int>(selected)
        << "\n";
  }
}

inline void dumpGenTausCSV(
    std::ofstream& out,
    const edm::Event& event,
    const edm::EDGetTokenT<reco::GenParticleCollection>& genParticleToken,
    HFSide side,
    double minPt = 0.0,
    double etaMin = 3.0,
    double etaMax = 5.2
) {
  out << "run,lumi,event,side,"
      << "gen_index,pdg_id,status,charge,"
      << "pt,eta,phi,energy,mass,"
      << "iphi,sector,passes_pt,passes_eta,selected\n";

  edm::Handle<reco::GenParticleCollection> genParticles;
  event.getByToken(genParticleToken, genParticles);

  if (!genParticles.isValid()) {
    std::cout << "WARNING: GEN-particle collection not found for event "
              << event.id().event() << std::endl;
    return;
  }

  for (std::size_t index = 0; index < genParticles->size(); ++index) {
    const reco::GenParticle& p = genParticles->at(index);

    // Same tau definition as the reference analyzer.
    if (p.status() != 23 || std::abs(p.pdgId()) != 15) {
      continue;
    }

    const bool passesPt = p.pt() >= minPt;
    const bool passesEta =
        passesHFSideEta(p.eta(), side, etaMin, etaMax);
    const int iphi = phiToIPhi(p.phi());
    const int sector = iphiToSector(iphi);
    const bool selected = passesPt && passesEta;

    out << event.id().run() << ","
        << event.id().luminosityBlock() << ","
        << event.id().event() << ","
        << sideName(side) << ","
        << index << ","
        << p.pdgId() << ","
        << p.status() << ","
        << p.charge() << ","
        << p.pt() << ","
        << p.eta() << ","
        << p.phi() << ","
        << p.energy() << ","
        << p.mass() << ","
        << iphi << ","
        << sector << ","
        << static_cast<int>(passesPt) << ","
        << static_cast<int>(passesEta) << ","
        << static_cast<int>(selected)
        << "\n";
  }
}

inline void dumpGenQuarksCSV(
    std::ofstream& out,
    const edm::Event& event,
    const edm::EDGetTokenT<reco::GenParticleCollection>& genParticleToken,
    HFSide side,
    double minPt = 0.0,
    double etaMin = 3.0,
    double etaMax = 5.2
) {
  out << "run,lumi,event,side,"
      << "gen_index,pdg_id,status,charge,"
      << "pt,eta,phi,energy,mass,"
      << "iphi,sector,passes_pt,passes_eta,selected\n";

  edm::Handle<reco::GenParticleCollection> genParticles;
  event.getByToken(genParticleToken, genParticles);

  if (!genParticles.isValid()) {
    std::cout << "WARNING: GEN-particle collection not found for event "
              << event.id().event() << std::endl;
    return;
  }

  for (std::size_t index = 0; index < genParticles->size(); ++index) {
    const reco::GenParticle& p = genParticles->at(index);
    const int absId = std::abs(p.pdgId());

    // Same hard-process quark definition as the reference analyzer.
    if (p.status() != 23 || absId >= 9) {
      continue;
    }

    const bool passesPt = p.pt() >= minPt;
    const bool passesEta =
        passesHFSideEta(p.eta(), side, etaMin, etaMax);
    const int iphi = phiToIPhi(p.phi());
    const int sector = iphiToSector(iphi);
    const bool selected = passesPt && passesEta;

    out << event.id().run() << ","
        << event.id().luminosityBlock() << ","
        << event.id().event() << ","
        << sideName(side) << ","
        << index << ","
        << p.pdgId() << ","
        << p.status() << ","
        << p.charge() << ","
        << p.pt() << ","
        << p.eta() << ","
        << p.phi() << ","
        << p.energy() << ","
        << p.mass() << ","
        << iphi << ","
        << sector << ","
        << static_cast<int>(passesPt) << ","
        << static_cast<int>(passesEta) << ","
        << static_cast<int>(selected)
        << "\n";
  }
}

inline void dumpGenParticles(
    const edm::Event& event,
    HFSide side,
    const edm::EDGetTokenT<reco::GenParticleCollection>& genParticleToken,
    double genParticleMinPt = 0.0,
    double etaMin = 3.0,
    double etaMax = 5.2,
    double magneticFieldTesla = 4.0
) {
  const unsigned long long eventId = event.id().event();

  const std::string genDirectory =
      "hf_IO/event_" + std::to_string(eventId) +
      "/" + sideName(side) + "/gen";

  ensureDir(genDirectory);

  {
    std::ofstream out = openOutputFile(
        genDirectory + "/event_" + std::to_string(eventId) +
        "_" + sideName(side) + "_genpions.csv"
    );
    dumpGenPionsCSV(
        out, event, genParticleToken, side,
        genParticleMinPt, etaMin, etaMax, magneticFieldTesla
    );
  }

  {
    std::ofstream out = openOutputFile(
        genDirectory + "/event_" + std::to_string(eventId) +
        "_" + sideName(side) + "_genelectrons.csv"
    );
    dumpGenElectronsCSV(
        out, event, genParticleToken, side,
        genParticleMinPt, etaMin, etaMax, magneticFieldTesla
    );
  }

  {
    std::ofstream out = openOutputFile(
        genDirectory + "/event_" + std::to_string(eventId) +
        "_" + sideName(side) + "_gentaus.csv"
    );
    dumpGenTausCSV(
        out, event, genParticleToken, side,
        genParticleMinPt, etaMin, etaMax
    );
  }

  {
    std::ofstream out = openOutputFile(
        genDirectory + "/event_" + std::to_string(eventId) +
        "_" + sideName(side) + "_genquarks.csv"
    );
    dumpGenQuarksCSV(
        out, event, genParticleToken, side,
        genParticleMinPt, etaMin, etaMax
    );
  }
}

// -----------------------------------------------------------------------------
// Top-level per-sector dump
// -----------------------------------------------------------------------------

inline void dumpSector(
    const edm::Event& event,
    HFSide side,
    int sector,
    const ap_uint<10> towers[N_TOWERS_ETA][N_TOWERS_PHI],
    const ap_uint<576> ip1Input[N_IP1_INPUT_LINKS],
    const ap_uint<576> ip1Output[N_IP1_OUTPUT_LINKS],
    const ap_uint<576> ip2Input[N_IP2_LINKS],
    const ap_uint<576> ip2Output[N_IP2_LINKS]
) {
  if (
      sector < 0 ||
      sector >= N_SECTORS
  ) {
    throw cms::Exception("Phase2L1CaloL1HFEmulator")
        << "Invalid HF sector: " << sector;
  }

  const unsigned long long eventId =
      event.id().event();
  if (ip1Output[sector] != ip2Input[sector]) {
    std::cerr
        << "WARNING: IP1 output does not match IP2 input"
        << " event=" << eventId
        << " side=" << sideName(side)
        << " sector=" << sector
        << std::endl;
    }

  const DumpDirs dirs =
      makeDumpDirs(
          eventId,
          side,
          sector
      );

  ensureDumpDirs(dirs);

  const std::vector<int> ip1InputIndices = {
      3 * sector,
      3 * sector + 1,
      3 * sector + 2
  };

  const std::vector<int> sectorLink = {
      sector
  };

  writeRawLinksCSVFile(
      dirs.inputIP1,
      eventId,
      side,
      sector,
      "IP1",
      "input",
      ip1Input,
      ip1InputIndices
  );

  writeRawLinksCSVFile(
      dirs.outputIP1,
      eventId,
      side,
      sector,
      "IP1",
      "output",
      ip1Output,
      sectorLink
  );

  writeRawLinksCSVFile(
      dirs.inputIP2,
      eventId,
      side,
      sector,
      "IP2",
      "input",
      ip2Input,
      sectorLink
  );

  writeRawLinksCSVFile(
      dirs.outputIP2,
      eventId,
      side,
      sector,
      "IP2",
      "output",
      ip2Output,
      sectorLink
  );

  {
    const std::string fileName =
        dirs.towers +
        "/event_" + std::to_string(eventId) +
        "_" + sideName(side) +
        "_sector_" + std::to_string(sector) +
        "_hf_towers.csv";

    std::ofstream out =
    openOutputFile(fileName);

    dumpHFTowersCSV(
        out,
        eventId,
        side,
        sector,
        towers
    );
  }

  {
    const std::string fileName =
        dirs.decodedIP1 +
        "/event_" + std::to_string(eventId) +
        "_" + sideName(side) +
        "_sector_" + std::to_string(sector) +
        "_IP1_input_towers_decoded.csv";

    std::ofstream out =
    openOutputFile(fileName);

    dumpDecodedIP1InputCSV(
        out,
        eventId,
        side,
        sector,
        ip1Input
    );
  }

  {
    const std::string fileName =
        dirs.decodedIP1 +
        "/event_" + std::to_string(eventId) +
        "_" + sideName(side) +
        "_sector_" + std::to_string(sector) +
        "_IP1_output_clusters_decoded.csv";

    std::ofstream out =
    openOutputFile(fileName);

    dumpDecodedIP1OutputCSV(
        out,
        eventId,
        side,
        sector,
        ip1Output[sector]
    );
  }

  {
    const std::string fileName =
        dirs.decodedIP2 +
        "/event_" + std::to_string(eventId) +
        "_" + sideName(side) +
        "_sector_" + std::to_string(sector) +
        "_IP2_input_hadcalo_decoded.csv";

    std::ofstream out =
    openOutputFile(fileName);

    dumpDecodedIP2InputCSV(
        out,
        eventId,
        side,
        sector,
        ip2Input[sector]
    );
  }

  {
    const std::string fileName =
        dirs.decodedIP2 +
        "/event_" + std::to_string(eventId) +
        "_" + sideName(side) +
        "_sector_" + std::to_string(sector) +
        "_IP2_output_puppi_decoded.csv";

    std::ofstream out =
    openOutputFile(fileName);

    dumpDecodedIP2OutputCSV(
        out,
        eventId,
        side,
        sector,
        ip2Output[sector]
    );
  }
}

inline void dumpIP1Global(
    const edm::Event& event,
    HFSide side,
    const ap_uint<576> ip1Output[N_IP1_OUTPUT_LINKS]
) {
  const unsigned long long eventId =
      event.id().event();

  const GlobalDumpDirs dirs =
      makeGlobalDumpDirs(
          eventId,
          side
      );

  ensureGlobalDumpDirs(dirs);

  // Raw links 6, 7 and 8
  {
    const std::string fileName =
        dirs.globalIP1Raw +
        "/event_" +
        std::to_string(eventId) +
        "_" +
        sideName(side) +
        "_IP1_global_output_links.csv";

    std::ofstream out =
    openOutputFile(fileName);

    dumpIP1GlobalRawCSV(
        out,
        eventId,
        side,
        ip1Output
    );
  }

  // Decoded jets from link 6
  {
    const std::string fileName =
        dirs.globalIP1Decoded +
        "/event_" +
        std::to_string(eventId) +
        "_" +
        sideName(side) +
        "_IP1_jets_decoded.csv";

    std::ofstream out =
    openOutputFile(fileName);

    dumpDecodedIP1GlobalObjectsCSV(
        out,
        eventId,
        side,
        IP1_JET_LINK,
        "jet",
        ip1Output[IP1_JET_LINK],
        N_IP1_GLOBAL_OBJECTS
    );
  }

  // Decoded taus from link 7
  {
    const std::string fileName =
        dirs.globalIP1Decoded +
        "/event_" +
        std::to_string(eventId) +
        "_" +
        sideName(side) +
        "_IP1_taus_decoded.csv";

    std::ofstream out =
    openOutputFile(fileName);

    dumpDecodedIP1GlobalObjectsCSV(
        out,
        eventId,
        side,
        IP1_TAU_LINK,
        "tau",
        ip1Output[IP1_TAU_LINK],
        N_IP1_GLOBAL_OBJECTS
    );
  }

  // Decoded sum from link 8
  {
    const std::string fileName =
        dirs.globalIP1Decoded +
        "/event_" +
        std::to_string(eventId) +
        "_" +
        sideName(side) +
        "_IP1_sum_decoded.csv";

    std::ofstream out =
    openOutputFile(fileName);

    dumpDecodedIP1SumCSV(
        out,
        eventId,
        side,
        ip1Output[IP1_SUM_LINK]
    );
  }
}

inline void dumpGenJets(
    const edm::Event& event,
    HFSide side,
    const edm::EDGetTokenT<reco::GenJetCollection>& genJetToken,
    double genJetMinPt = 10.0,
    double genJetEtaMin = 3.0,
    double genJetEtaMax = 5.2
) {
  const unsigned long long eventId =
      event.id().event();

  const std::string sideDirectory =
      "hf_IO/event_" +
      std::to_string(eventId) +
      "/" +
      sideName(side);

  ensureDir(sideDirectory);

  const std::string fileName =
      sideDirectory +
      "/event_" +
      std::to_string(eventId) +
      "_" +
      sideName(side) +
      "_genjets.csv";

  std::ofstream out =
    openOutputFile(fileName);

  dumpGenJetsCSV(
      out,
      event,
      genJetToken,
      side,
      genJetMinPt,
      genJetEtaMin,
      genJetEtaMax
  );
}

// -----------------------------------------------------------------------------
// Dump all six sectors
// -----------------------------------------------------------------------------

inline void dumpEvent(
    const edm::Event& event,
    HFSide side,
    const ap_uint<10> towers[N_TOWERS_ETA][N_TOWERS_PHI],
    const ap_uint<576> ip1Input[N_IP1_INPUT_LINKS],
    const ap_uint<576> ip1Output[N_IP1_OUTPUT_LINKS],
    const ap_uint<576> ip2Input[N_IP2_LINKS],
    const ap_uint<576> ip2Output[N_IP2_LINKS],
    const edm::EDGetTokenT<reco::GenJetCollection>& genJetToken,
    const edm::EDGetTokenT<reco::GenParticleCollection>& genParticleToken,
    double genJetMinPt = 10.0,
    double genJetEtaMin = 3.0,
    double genJetEtaMax = 5.2,
    double genParticleMinPt = 0.0,
    double magneticFieldTesla = 4.0
) {
  // Dump IP1 links 6–8 once per event and side.

    dumpIP1Global(
      event,
      side,
      ip1Output
  );
    
  // Dump GEN jets once per event and side.

    dumpGenJets(
        event,
        side,
        genJetToken,
        genJetMinPt,
        genJetEtaMin,
        genJetEtaMax
    );

  // Dump GEN particles used by the reference analyzer.
    dumpGenParticles(
        event,
        side,
        genParticleToken,
        genParticleMinPt,
        genJetEtaMin,
        genJetEtaMax,
        magneticFieldTesla
    );

  // Dump sector-dependent IP1/IP2 information.
  for (int sector = 0;
       sector < N_SECTORS;
       ++sector) {
    dumpSector(
        event,
        side,
        sector,
        towers,
        ip1Input,
        ip1Output,
        ip2Input,
        ip2Output
    );
  }
}

}  // namespace hfdump


#endif