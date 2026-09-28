/*
 * Description:
 *   Source-faithful CMSSW adaptation of the attached algo_topIP2gct.cpp.
 *
 *   The firmware routing is preserved exactly:
 *     positive EG inputs : 0, 1, 4, 5
 *     negative EG inputs : 6, 7, 2, 3
 *     positive ST inputs : 8, 9, 10, 17, 18, 19
 *     negative ST inputs : 20, 21, 22, 13, 14, 15
 *     unused inputs      : 11, 12, 16, 23
 *
 *   Outputs are also preserved exactly:
 *     0 = positive EG, 1 = positive jets+taus, 2 = combined sums,
 *     3 = negative EG, 4 = negative jets+taus, 5 = zero/spare.
 *
 *   The only non-firmware concern handled outside this file is the adapter
 *   between the existing IP1 ST slot order and the slot order expected by
 *   fillInputST().  That transposition occurs in the CMSSW producer before
 *   the PreIP2 product is saved.
 */

#ifndef L1Trigger_L1CaloTrigger_algo_topIP2gct_cpp_h
#define L1Trigger_L1CaloTrigger_algo_topIP2gct_cpp_h

#include "L1Trigger/L1CaloTrigger/interface/algo_topIP2gct_h.h"
#include "L1Trigger/L1CaloTrigger/interface/gctforgtIP2_cpp.h"

namespace gctip2 {
  #ifndef __SYNTHESIS__

inline int globalEtaSTFromLocal(
    int localEta
) {

  // IP2 physical barrel region occupies
  // padded eta rows 2..7.
  //
  // 2 -> global eta ST 0
  // ...
  // 7 -> global eta ST 5

  if (
      localEta < 2 ||
      localEta > 7
  ) {
    return -1;
  }

  return localEta - 2;
}


inline int globalPhiSTFromLocal(
    int gctIndex,
    int localPhi
) {

  // IP2 physical region occupies
  // padded phi columns 2..9.

  if (
      localPhi < 2 ||
      localPhi > 9
  ) {
    return -1;
  }

  if (
      gctIndex < 0 ||
      gctIndex >= 3
  ) {
    return -1;
  }

  // GCT1 ->  0..7
  // GCT2 ->  8..15
  // GCT3 -> 16..23

  return
      8 * gctIndex
      + (localPhi - 2);
}

#endif


namespace detail {

inline void fillInputEG(const LinkWord& input0,
                        const LinkWord& input1,
                        const LinkWord& input2,
                        const LinkWord& input3,
                        std::array<GctEcalCluster, kEcalClustersPerRegion>& clusters) {
#pragma HLS INLINE
  for (std::size_t i = 0; i < 8; ++i) {
#pragma HLS UNROLL
    const int start = static_cast<int>(64 * i);
    const int end = start + 63;
    clusters[i].fillFrom64(input0.range(end, start));
    clusters[i + 8].fillFrom64(input1.range(end, start));
    clusters[i + 16].fillFrom64(input2.range(end, start));
    clusters[i + 24].fillFrom64(input3.range(end, start));
  }
}

inline void fillInputST(const LinkWord& input0,
                        const LinkWord& input1,
                        const LinkWord& input2,
                        const LinkWord& input3,
                        const LinkWord& input4,
                        const LinkWord& input5,
                        std::array<std::array<STower, kPaddedPhi>, kPaddedEta>& towers) {
#pragma HLS INLINE
  const std::array<LinkWord, 6> inputs{{input0, input1, input2, input3, input4, input5}};

  // Literal attached-firmware slot contract:
  //   slots 0..5  = eta 0..5 for the first phi ST column
  //   slots 6..11 = eta 0..5 for the second phi ST column.
  for (std::size_t link = 0; link < inputs.size(); ++link) {
#pragma HLS UNROLL
    for (std::size_t eta = 0; eta < kRegionEta; ++eta) {
#pragma HLS UNROLL
      const int firstStart = static_cast<int>(48 * eta);
      const int secondStart = static_cast<int>(48 * (kRegionEta + eta));
      towers[eta + 2][2 * link].fill(inputs[link].range(firstStart + 47, firstStart));
      towers[eta + 2][2 * link + 1].fill(inputs[link].range(secondStart + 47, secondStart));
    }
  }
}

inline void fillOverlap(std::array<std::array<STower, kPaddedPhi>, kPaddedEta>& positive,
                        std::array<std::array<STower, kPaddedPhi>, kPaddedEta>& negative) {
#pragma HLS INLINE
  for (std::size_t phi = 0; phi < kPaddedPhi; ++phi) {
#pragma HLS UNROLL
    const std::size_t mirrored = kPaddedPhi - 1 - phi;
    positive[1][phi] = negative[2][mirrored];
    positive[0][phi] = negative[3][mirrored];
    negative[1][phi] = positive[2][mirrored];
    negative[0][phi] = positive[3][mirrored];
  }
}

inline void assignLocalCoordinates(std::array<std::array<STower, kPaddedPhi>, kPaddedEta>& towers) {
#pragma HLS INLINE
  for (std::size_t eta = 0; eta < kPaddedEta; ++eta) {
#pragma HLS UNROLL
    for (std::size_t phi = 0; phi < kPaddedPhi; ++phi) {
#pragma HLS UNROLL
      towers[eta][phi].eta = static_cast<unsigned int>(eta);
      towers[eta][phi].phi = static_cast<unsigned int>(phi);
    }
  }
}

}  // namespace detail

inline void algoTop(const std::array<LinkWord, kInputLinks>& input,
                    std::array<LinkWord, kOutputLinks>& output) {
#pragma HLS INLINE
  Region positive;
  Region negative;

  detail::fillInputEG(input[0], input[1], input[4], input[5], positive.gctecalclusters);
  detail::fillInputEG(input[6], input[7], input[2], input[3], negative.gctecalclusters);

  detail::fillInputST(input[8], input[9], input[10], input[17], input[18], input[19], positive.stowers);
  detail::fillInputST(input[20], input[21], input[22], input[13], input[14], input[15], negative.stowers);

  detail::fillOverlap(positive.stowers, negative.stowers);
  detail::assignLocalCoordinates(positive.stowers);
  detail::assignLocalCoordinates(negative.stowers);

  createSums(positive.stowers, negative.stowers, positive.sum);

  createJets(positive.stowers, positive.jets);
  createJets(negative.stowers, negative.jets);

  createTaus(positive.stowers, positive.taus);
  createTaus(negative.stowers, negative.taus);

  createEGs(positive.gctecalclusters, positive.egs);
  createEGs(negative.gctecalclusters, negative.egs);

  output.fill(0);
  positive.fillEGLink(output[0]);
  positive.fillJetTauLink(output[1]);
  positive.fillSumsLink(output[2]);
  negative.fillEGLink(output[3]);
  negative.fillJetTauLink(output[4]);
  // output[5] remains zero/spare exactly as the attachment.
}

#ifndef __SYNTHESIS__

inline void algoTopDebug(const std::array<LinkWord, kInputLinks>& input,
    std::array<LinkWord, kOutputLinks>& output, int gctIndex,
    std::array<GlobalCoordDebug,kJetsPerRegion>& positiveJetGlobal,
    std::array<GlobalCoordDebug,kTausPerRegion>& positiveTauGlobal,
    std::array<GlobalCoordDebug,kJetsPerRegion>& negativeJetGlobal,
    std::array<GlobalCoordDebug,kTausPerRegion>& negativeTauGlobal
) {

  // ----------------------------------------------------------
  // Run the original firmware algorithm first.
  //
  // Packed output is therefore produced ONLY by the original
  // algoTop().
  // ----------------------------------------------------------

  algoTop(input,output);


  // ----------------------------------------------------------
  // Reconstruct the two IP2 regions again, only for diagnostic
  // coordinate provenance.
  // ----------------------------------------------------------

  Region positive;
  Region negative;
  detail::fillInputEG(input[0], input[1], input[4], input[5], positive.gctecalclusters);
  detail::fillInputEG(input[6], input[7], input[2], input[3],negative.gctecalclusters);
  detail::fillInputST(input[8],input[9],input[10], input[17], input[18], input[19],positive.stowers);
  detail::fillInputST(input[20], input[21], input[22], input[13], input[14], input[15], negative.stowers);

  detail::fillOverlap(positive.stowers, negative.stowers);
  detail::assignLocalCoordinates(positive.stowers);
  detail::assignLocalCoordinates(negative.stowers);

  // ----------------------------------------------------------
  // Re-run only object reconstruction for diagnostic purposes.
  //
  // This does NOT repack or modify 'output'.
  // ----------------------------------------------------------

  createJets(positive.stowers, positive.jets);
  createJets(negative.stowers, negative.jets);
  createTaus(positive.stowers, positive.taus);
  createTaus(negative.stowers, negative.taus);

  // ----------------------------------------------------------
  // Jets
  // ----------------------------------------------------------

  for (std::size_t i = 0; i < kJetsPerRegion; ++i) {

    // ---------------- positive ----------------

    if (positive.jets[i].energy > 0) {
      positiveJetGlobal[i].etaST =globalEtaSTFromLocal(static_cast<int>(positive.jets[i].eta));
      positiveJetGlobal[i].phiST = globalPhiSTFromLocal(gctIndex, static_cast<int>(positive.jets[i].phi
              )
          );

    } else {

      positiveJetGlobal[i].etaST = -1;
      positiveJetGlobal[i].phiST = -1;
    }


    // ---------------- negative ----------------

    if (
        negative.jets[i].energy > 0
    ) {

      negativeJetGlobal[i].etaST =
          globalEtaSTFromLocal(
              static_cast<int>(
                  negative.jets[i].eta
              )
          );

      negativeJetGlobal[i].phiST =
          globalPhiSTFromLocal(
              gctIndex,
              static_cast<int>(
                  negative.jets[i].phi
              )
          );

    } else {

      negativeJetGlobal[i].etaST = -1;
      negativeJetGlobal[i].phiST = -1;
    }
  }


  // ----------------------------------------------------------
  // Taus
  // ----------------------------------------------------------

  for (
      std::size_t i = 0;
      i < kTausPerRegion;
      ++i
  ) {

    // ---------------- positive ----------------

    if (
        positive.taus[i].energy > 0
    ) {

      positiveTauGlobal[i].etaST =
          globalEtaSTFromLocal(
              static_cast<int>(
                  positive.taus[i].eta
              )
          );

      positiveTauGlobal[i].phiST =
          globalPhiSTFromLocal(
              gctIndex,
              static_cast<int>(
                  positive.taus[i].phi
              )
          );

    } else {

      positiveTauGlobal[i].etaST = -1;
      positiveTauGlobal[i].phiST = -1;
    }


    // ---------------- negative ----------------

    if (
        negative.taus[i].energy > 0
    ) {

      negativeTauGlobal[i].etaST =
          globalEtaSTFromLocal(
              static_cast<int>(
                  negative.taus[i].eta
              )
          );

      negativeTauGlobal[i].phiST =
          globalPhiSTFromLocal(
              gctIndex,
              static_cast<int>(
                  negative.taus[i].phi
              )
          );

    } else {

      negativeTauGlobal[i].etaST = -1;
      negativeTauGlobal[i].phiST = -1;
    }
  }
}

#endif

}  // namespace gctip2

#endif
