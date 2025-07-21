//------------------------------------
// IP1 Logic for Phase2L1CaloL1RCTEmulator.cc
//------------------------------------
#ifndef L1Trigger_L1CaloTrigger_RCT_IO
#define L1Trigger_L1CaloTrigger_RCT_IO

#include <iostream>
#include "ap_int.h"
#include <algorithm>
#include <utility>
#include <stdint.h>

#include <bitset>

namespace p2rctIO {

static constexpr int N_BITS_CRYSTAL = 16;
static constexpr int N_BITS_ENERGY = 10;
static constexpr int N_BITS_TIMING = 5;
static constexpr int N_BITS_SPIKE = 1;
static constexpr float LSB_ENERGY = 0.5;

static constexpr int N_TOWERS_PHI = 6;
static constexpr int N_TOWERS_ETA = 17;

static constexpr int N_CARDS = 24;

static constexpr int CRYSTALS_IN_TOWER_ETA = 5;
static constexpr int CRYSTALS_IN_TOWER_PHI = 5;

static constexpr float ECAL_ETA_RANGE = 1.4841;

//////////////////////////////////////////////////////////////////////////
  // RCT: indexing helper functions
  //////////////////////////////////////////////////////////////////////////

  // Assert that the card index is within bounds. (Valid cc: 0 to N_CARDS, since there are N_CARDS RCT cards)
  inline bool isValidCard(int cc) { return ((cc > -1) && (cc < N_CARDS)); }

  // RCT Cards: need to know their min/max crystal boundaries.

  // For a card (ranging from 0 to N_CARDS, since there are N_CARDS cards), return the iEta of the crystal with max iEta.
  // This represents the card boundaries in eta (identical to getEtaMax_card in the original emulator)
  inline int getCard_iEtaMax(int cc) {
    assert(isValidCard(cc));

    int etamax = 0;
    if (cc % 2 == 0)                                            // Even card: negative eta
      etamax = (N_TOWERS_ETA * CRYSTALS_IN_TOWER_ETA - 1);  // First eta half. 5 crystals in eta in 1 tower.
    else                                                        // Odd card: positive eta
      etamax = (2 * N_TOWERS_ETA * CRYSTALS_IN_TOWER_ETA - 1);
    return etamax;
  }

  // Same as above but for minimum iEta.
  inline int getCard_iEtaMin(int cc) {
    int etamin = 0;
    if (cc % 2 == 0)  // Even card: negative eta
      etamin = (0);
    else  // Odd card: positive eta
      etamin = (N_TOWERS_ETA * CRYSTALS_IN_TOWER_ETA);
    return etamin;
  }

  // Same as above but for maximum iPhi.
  inline int getCard_iPhiMax(int cc) {
    int phimax = ((cc / 2) + 1) * N_TOWERS_PHI * CRYSTALS_IN_TOWER_PHI - 1;
    return phimax;
  }

  // Same as above but for minimum iPhi.
  inline int getCard_iPhiMin(int cc) {
    int phimin = (cc / 2) * N_TOWERS_PHI * CRYSTALS_IN_TOWER_PHI;
    return phimin;
  }

  // Given the RCT card number (0-N_CARDS), get the crystal iEta of the "bottom left" corner
  inline int getCard_refCrystal_iEta(int cc) {
    if ((cc % 2) == 1) {  // if cc is odd (positive eta)
      return (N_TOWERS_ETA * CRYSTALS_IN_TOWER_ETA);
    } else {  // if cc is even (negative eta) the bottom left corner is further in eta
      return (N_TOWERS_ETA * CRYSTALS_IN_TOWER_ETA - 1);
    }
  }

  // Given the RCT card number (0-N_CARDS), get the global crystal iPhi of the "bottom left" corner (0- 71*5)
  inline int getCard_refCrystal_iPhi(int cc) {
    if ((cc % 2) == 1) {
      // if cc is odd: positive eta
      return int(cc / 2) * N_TOWERS_PHI * CRYSTALS_IN_TOWER_PHI;
    } else {
      // if cc is even, the bottom left corner is further in phi, hence the +1 and -1
      return (((int(cc / 2)+1) * N_TOWERS_PHI) * CRYSTALS_IN_TOWER_PHI) - 1;
    }
  }

class linkECAL {
    private:
    ap_uint<576> data;

    public:
    // constructor
    linkECAL() {
        data = (ap_uint<576>)0;
    }

    inline ap_uint<576> Data(void) {return data;}

    inline void setCrystal(float energy, float timing, int spike, int iEta, int iPhi) {
        // Clear the 16 bits for this crystal
        clearCrystal(iEta, iPhi);

        // Build the 16 bits for this crystal
        ap_uint<16> crystalData;
        crystalData = 
        ((ap_uint<16>)(energy/LSB_ENERGY) & 0x3FF) |
        (((ap_uint<16>)(timing)<<N_BITS_ENERGY) & 0x1F) |
        (((ap_uint<16>)(spike)<<(N_BITS_ENERGY+N_BITS_TIMING)) & 0x1) ;

        // Put this crystal's 16 bits into the 576 bit data
        int startId = iEta*CRYSTALS_IN_TOWER_PHI+iPhi;
        int start = startId * N_BITS_CRYSTAL;
        ap_uint<576> shiftedCrystalData = (ap_uint<576>)crystalData<<start;

        this->data = this->data | shiftedCrystalData;
    }

    inline void clearCrystal(int iEta, int iPhi) {
        // Find first bit corresponding to crystal at iEta, iPhi
        int startId = iEta*CRYSTALS_IN_TOWER_PHI+iPhi;
        int start = startId * N_BITS_CRYSTAL;

        ap_uint<576> spaceHolder = (ap_uint<576>)(0xFFFF)<<start; //16 1s, shifted to start at start

        this->data = this->data & ~spaceHolder; //Turn the 16 bits for this crystal to 0
    }

    inline void clearAllCrystals(void) {
        for(int i=0; i<CRYSTALS_IN_TOWER_ETA; i++) {
            for(int j=0; j<CRYSTALS_IN_TOWER_PHI; j++) {
                clearCrystal(i,j);
            }
        }
    }
};

class RCTcard {
    private:
    linkECAL links[N_TOWERS_ETA][N_TOWERS_PHI];

    public:
    //constructor
    RCTcard() {
        for(int iEta=0; iEta<N_TOWERS_ETA; iEta++) {
            for(int iPhi=0; iPhi<N_TOWERS_PHI; iPhi++) {
                links[iEta][iPhi] = linkECAL();
            }
        }
    }

    inline void addHit(float energy, float timing, int spike, int iEtaCrystalCard, int iPhiCrystalCard) {
        // Find which link(=tower) this hit is in
        int iEtaTowerCard = iEtaCrystalCard/CRYSTALS_IN_TOWER_ETA;
        int iPhiTowerCard = iPhiCrystalCard/CRYSTALS_IN_TOWER_PHI;

        // Find which crystal within that tower it is
        int iEtaCrystalTower = iEtaCrystalCard%CRYSTALS_IN_TOWER_ETA;
        int iPhiCrystalTower = iPhiCrystalCard%CRYSTALS_IN_TOWER_PHI;

        // Update that crystal within that link
        links[iEtaTowerCard][iPhiTowerCard].setCrystal(energy, timing, spike, iEtaCrystalTower, iPhiCrystalTower);
        // if (energy > 0.0) {
          // std::cout << "Link output: " << (bitset<576>)links[iEtaTowerCard][iPhiTowerCard].Data() << std::endl;
        // }
    }

    inline linkECAL getLink(int iEtaCrystalCard, int iPhiCrystalCard) {
      // Return the link at iEtaCrystalCard, iPhiCrystalCard. int accessible via .Data() method
      return links[iEtaCrystalCard][iPhiCrystalCard];
    }
};

/* 
* Represents one input HCAL or ECAL hit.
*/
class SimpleCaloHit {
    private:
    float et_ = 0.;
    GlobalVector position_;  // As opposed to GlobalPoint, so we can add them (for weighted average)

    public:
    // tool functions
    inline void setEt(float et) { et_ = et; };
    inline void setPosition(const GlobalVector& pos) { position_ = pos; };

    inline float et() const { return et_; };
    inline const GlobalVector& position() const { return position_; };

    /* 
       * Get crystal's iEta from real eta. (identical to getCrystal_etaID in L1EGammaCrystalsEmulatorProducer.cc)
       * This "global" iEta ranges from 0 to (33*5) since there are 34 towers in eta in the full detector, 
       * each with five crystals in eta.
       */
    int crystaliEta(void) const {
      float size_cell = 2 * ECAL_ETA_RANGE / (CRYSTALS_IN_TOWER_ETA * 2 * N_TOWERS_ETA);
      int iEta = int((position().eta() + ECAL_ETA_RANGE) / size_cell);
      return iEta;
    }

    /* 
       * Get crystal's iPhi from real phi. (identical to getCrystal_phiID in L1EGammaCrystalsEmulatorProducer.cc)
       * This "global" iPhi ranges from 0 to (71*5) since there are 72 towers in phi in the full detector, each with five crystals in eta.
       */
    int crystaliPhi(void) const {
      float phi = position().phi();
      float size_cell = 2 * M_PI / (CRYSTALS_IN_TOWER_PHI * N_TOWERS_PHI * N_CARDS / 2);
      int iPhi = int((phi + M_PI) / size_cell);
      return iPhi;
    }

    /*
       * Check if it falls within the boundary of a card.
       */
    bool isInCard(int cc) const {
      return (crystaliPhi() <= getCard_iPhiMax(cc) && crystaliPhi() >= getCard_iPhiMin(cc) &&
              crystaliEta() <= getCard_iEtaMax(cc) && crystaliEta() >= getCard_iEtaMin(cc));
    };

    /*
      * For a crystal with real eta, and falling in card cc, get its local iEta 
      * relative to the bottom left corner of the card (possible local iEta ranges from 0 to N_TOWERS_ETA * CRYSTALS_IN_TOWER_ETA,
      * since in one card, there are N_TOWERS_ETA towers in eta, each with CRYSTALS_IN_TOWER_ETA crystals in eta.
      */
    int crystalLocaliEta(int cc) const { return abs(getCard_refCrystal_iEta(cc) - crystaliEta()); }

    /*
      * Same as above, but for iPhi (possible local iPhi ranges from 0 to (N_TOWERS_PHI*CRYSTALS_IN_TOWER_PHI), since in one card,
      * there are N_TOWERS_PHI towers in phi, each with CRYSTALS_IN_TOWER_PHI crystals in phi.
      */
    int crystalLocaliPhi(int cc) const { return abs(getCard_refCrystal_iPhi(cc) - crystaliPhi()); }
};

} // namespace p2rctIO

#endif
