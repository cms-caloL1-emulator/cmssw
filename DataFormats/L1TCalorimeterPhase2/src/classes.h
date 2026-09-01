/// ////////////////////////////////////////
/// Stacked Tracker Simulations          ///
/// ////////////////////////////////////////

#include "DataFormats/Common/interface/Wrapper.h"

/*********************/
/** L1 CALO TRIGGER **/
/*********************/

#include "DataFormats/L1TCalorimeterPhase2/interface/CaloCrystalCluster.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/CaloTower.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/CaloJet.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/DigitizedClusterCorrelator.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/DigitizedTowerCorrelator.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/DigitizedClusterGT.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/CaloPFCluster.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/Phase2L1CaloJet.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/RCT_output.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/GCT_output.h"
#include "DataFormats/L1TCalorimeterPhase2/interface/HF_output.h"
#include <vector>

namespace DataFormats_L1TCalorimeterPhase2 {

  struct dictionary {

    l1tp2::hfOutputLink hfOutputLink_;
    std::vector<l1tp2::hfOutputLink> hfOutputLinkCollection_;
    edm::Wrapper<std::vector<l1tp2::hfOutputLink>> hfOutputLinkCollectionWrapper_;
  };
}