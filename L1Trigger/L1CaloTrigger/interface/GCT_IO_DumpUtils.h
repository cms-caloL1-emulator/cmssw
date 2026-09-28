#ifndef TestingFramework_GCTDumpUtils_h
#define TestingFramework_GCTDumpUtils_h

#include <ap_int.h>

#include <array>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Utilities/interface/EDGetToken.h"

#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/JetReco/interface/GenJet.h"
#include "DataFormats/JetReco/interface/GenJetCollection.h"

#include "CommonTools/BaseParticlePropagator/interface/BaseParticlePropagator.h"
#include "CommonTools/BaseParticlePropagator/interface/RawParticle.h"

#include "L1Trigger/L1CaloTrigger/interface/algo_topIP1gct_h.h"
#include "L1Trigger/L1CaloTrigger/interface/algo_topIP2gct_h.h"

namespace gctdump {

constexpr double kPi = 3.14159265358979323846;
constexpr double kBarrelEtaMax = 1.4841;
constexpr double kEnergyLSB = 0.5;
constexpr int kNGCTCards = 3;
constexpr int kNGCTSLRs = 6;

inline void ensureDir(const std::string& path) {
  if (path.empty()) return;
  ::mkdir(path.c_str(), 0775);
}

inline std::string formatEvent(const edm::Event& event) {
  return std::to_string(event.id().event());
}

template <int N>
inline std::string formatApUint(const ap_uint<N>& value) {
  if (value == 0) return "0x0";
  std::string text = value.to_string(16);
  if (text.rfind("0x", 0) == 0 || text.rfind("0X", 0) == 0) return text;
  return "0x" + text;
}

inline double wrapPhi(double phi) {
  while (phi >= kPi) phi -= 2.0 * kPi;
  while (phi < -kPi) phi += 2.0 * kPi;
  return phi;
}

struct EventDirs {
  std::string eventBase;
  std::string gen;
};

inline EventDirs makeEventDirs(const edm::Event& event, const std::string& baseDir = "gct_IO") {
  ensureDir(baseDir);
  EventDirs dirs;
  dirs.eventBase = baseDir + "/event_" + formatEvent(event);
  dirs.gen = dirs.eventBase + "/gen";
  ensureDir(dirs.eventBase);
  ensureDir(dirs.gen);
  return dirs;
}

inline std::string makeBoundaryDir(const edm::Event& event,
                                   const std::string& label,
                                   const std::string& stage,
                                   const std::string& direction,
                                   const std::string& kind,
                                   const std::string& baseDir = "gct_IO") {
  const EventDirs dirs = makeEventDirs(event, baseDir);
  const std::string objectDir = dirs.eventBase + "/" + label;
  const std::string stageDir = objectDir + "/" + stage;
  const std::string directionDir = stageDir + "/" + direction;
  const std::string finalDir = directionDir + "/" + kind;
  ensureDir(objectDir);
  ensureDir(stageDir);
  ensureDir(directionDir);
  ensureDir(finalDir);
  return finalDir;
}

// -----------------------------------------------------------------------------
// Raw boundary links
// -----------------------------------------------------------------------------

template <std::size_t N>
inline void dumpRawLinks(const edm::Event& event,
                         const std::string& label,
                         const std::string& stage,
                         const std::string& direction,
                         const std::array<ap_uint<576>, N>& links,
                         const std::string& baseDir = "gct_IO") {
  const std::string dir = makeBoundaryDir(event, label, stage, direction, "raw", baseDir);
  const std::string path = dir + "/event_" + formatEvent(event) + "_" + label + "_" + stage + "_" + direction + "_links.csv";
  std::ofstream out(path);
  out << "run,lumi,event,label,stage,direction,link_index_dec,link_index_hex,raw_hex\n";
  for (std::size_t i = 0; i < N; ++i) {
    std::ostringstream idxHex;
    idxHex << std::hex << i;
    out << event.id().run() << ','
        << event.id().luminosityBlock() << ','
        << event.id().event() << ','
        << label << ',' << stage << ',' << direction << ','
        << i << ",0x" << idxHex.str() << ',' << formatApUint(links[i]) << '\n';
  }
}

// -----------------------------------------------------------------------------
// IP1 output decoding
// -----------------------------------------------------------------------------

inline void writeEG64(std::ofstream& out,
                      const edm::Event& event,
                      const std::string& label,
                      int link,
                      int slot,
                      const ap_uint<64>& raw,
                      const std::string& source) {
  EGcluster obj;
  obj.fillEGcluster(raw);
  out << event.id().event() << ',' << label << ',' << link << ',' << slot
      << ",EG," << source << ',' << formatApUint(raw) << ','
      << static_cast<unsigned>(obj.energy) << ','
      << static_cast<int>(obj.eta) << ','
      << static_cast<int>(obj.phi) << ','
      << static_cast<unsigned>(obj.hoe) << ','
      << static_cast<unsigned>(obj.hoeWP) << ','
      << static_cast<unsigned>(obj.iso) << ','
      << static_cast<unsigned>(obj.isoWP) << ','
      << static_cast<unsigned>(obj.FB) << ','
      << static_cast<unsigned>(obj.timing) << ','
      << static_cast<unsigned>(obj.shapeWP) << ','
      << static_cast<unsigned>(obj.brems) << ",,,\n";
}

inline void writePF64(std::ofstream& out,
                      const edm::Event& event,
                      const std::string& label,
                      int link,
                      int slot,
                      const ap_uint<64>& raw,
                      const std::string& source) {
  PFcluster obj;
  obj.fillPFcluster(raw);
  out << event.id().event() << ',' << label << ',' << link << ',' << slot
      << ",PF," << source << ',' << formatApUint(raw) << ','
      << static_cast<unsigned>(obj.energy) << ','
      << static_cast<int>(obj.eta) << ','
      << static_cast<int>(obj.phi) << ','
      << static_cast<unsigned>(obj.hoe) << ",,,,,,,,,"
      << static_cast<unsigned>(obj.ECAL) << ','
      << static_cast<unsigned>(obj.HCAL) << "\n";
}

inline void dumpPostIP1Decoded(const edm::Event& event,
                               const std::string& label,
                               const std::array<ap_uint<576>, N_OUTPUT_LINKS>& links,
                               const std::string& baseDir = "gct_IO") {
  const std::string dir = makeBoundaryDir(event, label, "IP1", "output", "decoded", baseDir);

  // Object links 0..12.  This mapping follows the checked-in IP1 packing literally,
  // including the mixed EG/PF link 3 and the duplicated PF subsets in links 3/4 and 7/8.
  {
    const std::string path = dir + "/event_" + formatEvent(event) + "_" + label + "_IP1_output_objects.csv";
    std::ofstream out(path);
    out << "event,label,link_index,slot,object_type,source,raw_hex,energy_raw,eta_raw,phi_raw,"
           "hoe_raw,hoe_wp_raw,iso_raw,iso_wp_raw,fb_raw,timing_raw,shape_wp_raw,brems_raw,ecal_raw,hcal_raw\n";

    auto eg = [&](int link, int slot, const std::string& src) {
      writeEG64(out, event, label, link, slot, links[link].range(64 * slot + 63, 64 * slot), src);
    };
    auto pf = [&](int link, int slot, const std::string& src) {
      writePF64(out, event, label, link, slot, links[link].range(64 * slot + 63, 64 * slot), src);
    };

    for (int i = 0; i < 8; ++i) eg(0, i + 1, "R1_EG" + std::to_string(i));
    for (int i = 0; i < 8; ++i) eg(1, i, "R2_EG" + std::to_string(i));
    eg(1, 8, "R5_EG0");
    for (int i = 1; i < 8; ++i) eg(2, i - 1, "R5_EG" + std::to_string(i));
    for (int i = 0; i < 2; ++i) eg(2, i + 7, "R6_EG" + std::to_string(i));
    for (int i = 2; i < 8; ++i) eg(3, i - 2, "R6_EG" + std::to_string(i));

    for (int i = 0; i < 3; ++i) pf(3, i + 6, "R1_PF" + std::to_string(i));
    for (int i = 0; i < 9; ++i) pf(4, i, "R1_PF" + std::to_string(i));
    for (int i = 0; i < 9; ++i) pf(5, i, "R2_PF" + std::to_string(i));
    for (int i = 9; i < 12; ++i) pf(6, i - 9, "R2_PF" + std::to_string(i));
    for (int i = 0; i < 6; ++i) pf(6, i + 3, "R5_PF" + std::to_string(i));
    for (int i = 6; i < 12; ++i) pf(7, i - 6, "R5_PF" + std::to_string(i));
    for (int i = 0; i < 3; ++i) pf(7, i + 6, "R6_PF" + std::to_string(i));
    for (int i = 0; i < 9; ++i) pf(8, i, "R6_PF" + std::to_string(i));

    for (int i = 0; i < 9; ++i) eg(9, i, "R1_EG" + std::to_string(i));
    for (int i = 0; i < 9; ++i) eg(10, i, "R2_EG" + std::to_string(i));
    for (int i = 0; i < 9; ++i) eg(11, i, "R5_EG" + std::to_string(i));
    for (int i = 0; i < 9; ++i) eg(12, i, "R6_EG" + std::to_string(i));
  }

  // Links 13..20: one 12-object SuperTower link per RCT region.
  {
    const std::string path = dir + "/event_" + formatEvent(event) + "_" + label + "_IP1_output_supertowers.csv";
    std::ofstream out(path);
    out << "event,label,link_index,region,slot,raw_hex,energy_raw,em_energy_raw,eta_raw,phi_raw,flags_raw\n";
    for (int link = 13; link <= 20; ++link) {
      const int region = link - 13;
      for (int slot = 0; slot < 12; ++slot) {
        const ap_uint<48> raw = links[link].range(48 * slot + 47, 48 * slot);
        SuperTower st;
        st.fillST(raw);
        out << event.id().event() << ',' << label << ',' << link << ',' << region << ',' << slot << ','
            << formatApUint(raw) << ','
            << static_cast<unsigned>(st.energy) << ','
            << static_cast<unsigned>(st.EMenergy) << ','
            << static_cast<unsigned>(st.eta) << ','
            << static_cast<unsigned>(st.phi) << ','
            << static_cast<unsigned>(st.flags) << '\n';
      }
    }
  }
}

// -----------------------------------------------------------------------------
// IP2 input decoding
// -----------------------------------------------------------------------------

inline bool isIP2EGInputLink(int link) {
  return link == 0 || link == 1 || link == 2 || link == 3 ||
         link == 4 || link == 5 || link == 6 || link == 7;
}

inline bool isIP2STInputLink(int link) {
  return link == 8 || link == 9 || link == 10 || link == 13 || link == 14 ||
         link == 15 || link == 17 || link == 18 || link == 19 || link == 20 ||
         link == 21 || link == 22;
}

inline void dumpPreIP2Decoded(const edm::Event& event,
                              const std::string& label,
                              const std::array<ap_uint<576>, gctip2::kInputLinks>& links,
                              const std::string& baseDir = "gct_IO") {
  const std::string dir = makeBoundaryDir(event, label, "IP2", "input", "decoded", baseDir);
  const std::string path = dir + "/event_" + formatEvent(event) + "_" + label + "_IP2_input_decoded.csv";
  std::ofstream out(path);
  out << "event,label,link_index,slot,object_type,raw_hex,energy_raw,aux_energy_raw,eta_raw,phi_raw,flags_or_spare_raw\n";

  for (int link = 0; link < static_cast<int>(gctip2::kInputLinks); ++link) {
    if (isIP2EGInputLink(link)) {
      // fillInputEG consumes only the first eight 64-bit slots.
      for (int slot = 0; slot < 8; ++slot) {
        const ap_uint<64> raw = links[link].range(64 * slot + 63, 64 * slot);
        gctip2::GctEcalCluster eg;
        eg.fillFrom64(raw);
        out << event.id().event() << ',' << label << ',' << link << ',' << slot << ",EG,"
            << formatApUint(raw) << ',' << static_cast<unsigned>(eg.energy) << ",0,"
            << static_cast<unsigned>(eg.eta) << ',' << static_cast<unsigned>(eg.phi) << ",0\n";
      }
    } else if (isIP2STInputLink(link)) {
      for (int slot = 0; slot < 12; ++slot) {
        const ap_uint<48> raw = links[link].range(48 * slot + 47, 48 * slot);
        gctip2::STower st;
        st.fill(raw);
        out << event.id().event() << ',' << label << ',' << link << ',' << slot << ",ST,"
            << formatApUint(raw) << ',' << static_cast<unsigned>(st.energy) << ','
            << static_cast<unsigned>(st.EMenergy) << ',' << static_cast<unsigned>(st.eta) << ','
            << static_cast<unsigned>(st.phi) << ',' << static_cast<unsigned>(st.flags) << '\n';
      }
    } else {
      out << event.id().event() << ',' << label << ',' << link << ",-1,UNUSED,"
          << formatApUint(links[link]) << ",0,0,-1,-1,0\n";
    }
  }
}

// -----------------------------------------------------------------------------
// IP2 output decoding + software-only global-coordinate diagnostics
// -----------------------------------------------------------------------------

inline double globalSTPhiToPhysicalPhiProvisional(int globalPhiST) {
  // 24 barrel SuperTower bins span 2*pi.  This conversion assumes the
  // provisional GCT1/GCT2/GCT3 topology used by the current CMSSW producer:
  //   GCT1 -> ST phi 0..7, GCT2 -> 8..15, GCT3 -> 16..23.
  // The producer itself documents that the current RCT collection phi grid is
  // not yet verified against the physical Figure-6 card boundaries, so this
  // physical phi must be treated as provisional until that alignment is fixed.
  if (globalPhiST < 0 || globalPhiST >= 24) return -999.0;
  const double width = 2.0 * kPi / 24.0;
  return wrapPhi(-kPi + (static_cast<double>(globalPhiST) + 0.5) * width);
}

inline void dumpPostIP2Decoded(
    const edm::Event& event,
    const std::string& label,
    int gctIndex,
    const std::array<ap_uint<576>, gctip2::kOutputLinks>& links,
    const std::array<gctip2::GlobalCoordDebug, gctip2::kJetsPerRegion>& positiveJetGlobal,
    const std::array<gctip2::GlobalCoordDebug, gctip2::kTausPerRegion>& positiveTauGlobal,
    const std::array<gctip2::GlobalCoordDebug, gctip2::kJetsPerRegion>& negativeJetGlobal,
    const std::array<gctip2::GlobalCoordDebug, gctip2::kTausPerRegion>& negativeTauGlobal,
    const std::string& baseDir = "gct_IO") {
  const std::string dir = makeBoundaryDir(event, label, "IP2", "output", "decoded", baseDir);
  const std::string path =
      dir + "/event_" + formatEvent(event) + "_" + label + "_IP2_output_decoded.csv";
  std::ofstream out(path);

  out << "run,lumi,event,label,gct_index,side,side_sign,link_index,slot,object_type,raw_hex,"
         "energy_raw,pt_GeV,eta_local_raw,phi_local_raw,eta_global_st,phi_global_st,"
         "eta_physical,phi_physical_provisional,ratio_raw,extra0,extra1\n";

  auto dumpEGLink = [&](int link, const char* side, int sideSign) {
    for (int slot = 0; slot < static_cast<int>(gctip2::kEGsPerRegion); ++slot) {
      const ap_uint<48> raw = links[link].range(48 * slot + 47, 48 * slot);
      gctip2::GctEcalCluster obj;
      obj.unpack(raw);

      out << event.id().run() << ','
          << event.id().luminosityBlock() << ','
          << event.id().event() << ','
          << label << ',' << gctIndex << ',' << side << ',' << sideSign << ','
          << link << ',' << slot << ",EG," << formatApUint(raw) << ','
          << static_cast<unsigned>(obj.energy) << ','
          << static_cast<double>(obj.energy) * kEnergyLSB << ','
          << static_cast<unsigned>(obj.eta) << ','
          << static_cast<unsigned>(obj.phi) << ",-1,-1,-999,-999,0,0,0\n";
    }
  };

  auto dumpJetTauLink = [&](
      int link,
      const char* side,
      int sideSign,
      const std::array<gctip2::GlobalCoordDebug, gctip2::kJetsPerRegion>& jetGlobal,
      const std::array<gctip2::GlobalCoordDebug, gctip2::kTausPerRegion>& tauGlobal) {

    for (int i = 0; i < static_cast<int>(gctip2::kJetsPerRegion); ++i) {
      const int slot = i;
      const ap_uint<48> raw = links[link].range(48 * slot + 47, 48 * slot);
      gctip2::Jet obj;
      obj.unpack(raw);

      const int etaGlobalST = jetGlobal[i].etaST;
      const int phiGlobalST = jetGlobal[i].phiST;
      const double phiPhysical = globalSTPhiToPhysicalPhiProvisional(phiGlobalST);

      out << event.id().run() << ','
          << event.id().luminosityBlock() << ','
          << event.id().event() << ','
          << label << ',' << gctIndex << ',' << side << ',' << sideSign << ','
          << link << ',' << slot << ",jet," << formatApUint(raw) << ','
          << static_cast<unsigned>(obj.energy) << ','
          << static_cast<double>(obj.energy) * kEnergyLSB << ','
          << static_cast<int>(obj.eta) << ','
          << static_cast<int>(obj.phi) << ','
          << etaGlobalST << ',' << phiGlobalST << ','
          << -999.0 << ',' << phiPhysical << ','
          << static_cast<unsigned>(obj.ratio) << ",0,0\n";
    }

    for (int i = 0; i < static_cast<int>(gctip2::kTausPerRegion); ++i) {
      const int slot = i + static_cast<int>(gctip2::kJetsPerRegion);
      const ap_uint<48> raw = links[link].range(48 * slot + 47, 48 * slot);
      gctip2::Tau obj;
      obj.unpack(raw);

      const int etaGlobalST = tauGlobal[i].etaST;
      const int phiGlobalST = tauGlobal[i].phiST;
      const double phiPhysical = globalSTPhiToPhysicalPhiProvisional(phiGlobalST);

      out << event.id().run() << ','
          << event.id().luminosityBlock() << ','
          << event.id().event() << ','
          << label << ',' << gctIndex << ',' << side << ',' << sideSign << ','
          << link << ',' << slot << ",tau," << formatApUint(raw) << ','
          << static_cast<unsigned>(obj.energy) << ','
          << static_cast<double>(obj.energy) * kEnergyLSB << ','
          << static_cast<int>(obj.eta) << ','
          << static_cast<int>(obj.phi) << ','
          << etaGlobalST << ',' << phiGlobalST << ','
          << -999.0 << ',' << phiPhysical << ','
          << static_cast<unsigned>(obj.ratio) << ",0,0\n";
    }
  };

  dumpEGLink(0, "positive", +1);
  dumpJetTauLink(1, "positive", +1, positiveJetGlobal, positiveTauGlobal);

  {
    const ap_uint<48> raw = links[2].range(47, 0);
    gctip2::Sums sums;
    sums.unpack(raw);
    out << event.id().run() << ','
        << event.id().luminosityBlock() << ','
        << event.id().event() << ','
        << label << ',' << gctIndex << ",both,0,2,0,sums," << formatApUint(raw)
        << ',' << static_cast<unsigned>(sums.HT)
        << ',' << static_cast<double>(sums.HT) * kEnergyLSB
        << ",-1,-1,-1,-1,-999,-999,0,"
        << static_cast<unsigned>(sums.Ex) << ','
        << static_cast<unsigned>(sums.Ey) << '\n';
  }

  dumpEGLink(3, "negative", -1);
  dumpJetTauLink(4, "negative", -1, negativeJetGlobal, negativeTauGlobal);

  out << event.id().run() << ','
      << event.id().luminosityBlock() << ','
      << event.id().event() << ','
      << label << ',' << gctIndex << ",none,0,5,-1,spare," << formatApUint(links[5])
      << ",0,0,-1,-1,-1,-1,-999,-999,0,0,0\n";
}

// -----------------------------------------------------------------------------
// GEN information
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

inline PropagatedGenParticle propagateToEcalEntrance(const reco::GenParticle& gen,
                                                      double magneticFieldTesla = 4.0) {
  PropagatedGenParticle result;
  if (gen.status() != 1 || gen.charge() == 0) return result;
  result.attempted = true;

  RawParticle particle(gen.p4());
  particle.setVertex(gen.vertex().x(), gen.vertex().y(), gen.vertex().z(), 0.0);
  particle.setMass(gen.mass());
  particle.setCharge(gen.charge());

  BaseParticlePropagator propagator(particle, 0.0, 0.0, magneticFieldTesla);
  const bool propagated = propagator.propagateToEcalEntrance();
  result.successCode = propagator.getSuccess();
  result.success = propagated && result.successCode > 0;
  if (!result.success) return result;

  const RawParticle& p = propagator.particle();
  result.pt = p.Pt();
  result.eta = p.eta();
  result.phi = p.phi();
  result.energy = p.E();
  result.x = p.X();
  result.y = p.Y();
  result.z = p.Z();
  return result;
}

inline void dumpGenJets(const edm::Event& event,
                        const edm::EDGetTokenT<reco::GenJetCollection>& token,
                        const std::string& baseDir = "gct_IO") {
  const auto handle = event.getHandle(token);
  if (!handle.isValid()) return;
  const EventDirs dirs = makeEventDirs(event, baseDir);
  const std::string path = dirs.gen + "/event_" + formatEvent(event) + "_genjets.csv";
  std::ofstream out(path);
  out << "run,lumi,event,genjet_index,pt,eta,phi,energy,mass,px,py,pz,n_daughters,passes_barrel_eta,selected\n";
  for (std::size_t i = 0; i < handle->size(); ++i) {
    const auto& jet = (*handle)[i];
    const bool barrel = std::abs(jet.eta()) < kBarrelEtaMax;
    out << event.id().run() << ',' << event.id().luminosityBlock() << ',' << event.id().event() << ','
        << i << ',' << jet.pt() << ',' << jet.eta() << ',' << wrapPhi(jet.phi()) << ',' << jet.energy() << ','
        << jet.mass() << ',' << jet.px() << ',' << jet.py() << ',' << jet.pz() << ',' << jet.numberOfDaughters() << ','
        << static_cast<int>(barrel) << ',' << static_cast<int>(barrel) << '\n';
  }
}

inline void dumpGenParticles(const edm::Event& event,
                             const edm::EDGetTokenT<reco::GenParticleCollection>& token,
                             const std::string& baseDir = "gct_IO") {
  const auto handle = event.getHandle(token);
  if (!handle.isValid()) return;
  const EventDirs dirs = makeEventDirs(event, baseDir);
  const std::string path = dirs.gen + "/event_" + formatEvent(event) + "_genparticles.csv";
  std::ofstream out(path);
  out << "run,lumi,event,gen_index,pdg_id,status,charge,pt,eta,phi,energy,mass,"
         "is_electron,is_photon,is_pion,is_tau,is_quark,hard_object,stable,passes_barrel_eta,selected,"
         "prop_attempted,prop_success,prop_success_code,ecal_pt,ecal_eta,ecal_phi,ecal_energy,ecal_x,ecal_y,ecal_z\n";

  for (std::size_t i = 0; i < handle->size(); ++i) {
    const auto& gen = (*handle)[i];
    const int absId = std::abs(gen.pdgId());
    const bool isElectron = absId == 11;
    const bool isPhoton = absId == 22;
    const bool isPion = absId == 211;
    const bool isTau = absId == 15;
    const bool isQuark = absId >= 1 && absId <= 6;
    if (!(isElectron || isPhoton || isPion || isTau || isQuark)) continue;

    const bool hardObject = gen.status() == 23 && (isTau || isQuark);
    const bool stable = gen.status() == 1;
    const bool barrel = std::abs(gen.eta()) < kBarrelEtaMax;
    const bool selected = barrel && ((stable && (isElectron || isPhoton || isPion)) || hardObject);

    PropagatedGenParticle prop;
    if (stable && gen.charge() != 0 && (isElectron || isPion)) {
      prop = propagateToEcalEntrance(gen);
    }

    out << event.id().run() << ',' << event.id().luminosityBlock() << ',' << event.id().event() << ','
        << i << ',' << gen.pdgId() << ',' << gen.status() << ',' << gen.charge() << ','
        << gen.pt() << ',' << gen.eta() << ',' << wrapPhi(gen.phi()) << ',' << gen.energy() << ',' << gen.mass() << ','
        << static_cast<int>(isElectron) << ',' << static_cast<int>(isPhoton) << ',' << static_cast<int>(isPion) << ','
        << static_cast<int>(isTau) << ',' << static_cast<int>(isQuark) << ',' << static_cast<int>(hardObject) << ','
        << static_cast<int>(stable) << ',' << static_cast<int>(barrel) << ',' << static_cast<int>(selected) << ','
        << static_cast<int>(prop.attempted) << ',' << static_cast<int>(prop.success) << ',' << prop.successCode << ','
        << prop.pt << ',' << prop.eta << ',' << prop.phi << ',' << prop.energy << ',' << prop.x << ',' << prop.y << ',' << prop.z << '\n';
  }
}

inline void dumpGEN(const edm::Event& event,
                    const edm::EDGetTokenT<reco::GenJetCollection>& genJetToken,
                    const edm::EDGetTokenT<reco::GenParticleCollection>& genParticleToken,
                    const std::string& baseDir = "gct_IO") {
  dumpGenJets(event, genJetToken, baseDir);
  dumpGenParticles(event, genParticleToken, baseDir);
}

}  // namespace gctdump

#endif
