import FWCore.ParameterSet.Config as cms

# This cff file is based off of the L1T Phase-2 Menu group using
# the process name "REPR"

# RCT - l1tPhase2RCTEmulatorProducer
from L1Trigger.L1CaloTrigger.l1tPhase2RCTEmulatorProducer_cfi import *

# GCTB - l1tPhase2GCTEmulatorProducer
from L1Trigger.L1CaloTrigger.l1tPhase2GCTEmulatorProducer_cfi import *

# HF - l1tPhase2HFEmulatorProducer
from L1Trigger.L1CaloTrigger.l1tPhase2HFEmulatorProducer_cfi import *

# DMX - l1tPhase2L1DMXEmulator (doesn't actually do demultiplexing in emulator, just gets HGCAL hits)
from L1Trigger.L1CaloTrigger.l1tPhase2L1DMXEmulator_cfi import *
l1tPhase2L1DMXEmulatorPositive = l1tPhase2L1DMXEmulator.clone(zSide=1)
l1tPhase2L1DMXEmulatorNegative = l1tPhase2L1DMXEmulator.clone(zSide=-1)

# MHH - l1tPhase2L1MHHEmulator
from L1Trigger.L1CaloTrigger.l1tPhase2L1MHHEmulator_cfi import *
def _get_hf_input_links(side):
        '''
        side is 1 for positive, -1 for negative eta. Fill 5 extra links that just
        duplicate the first 3, since those 5 are unused.
        '''
        if side > 0:
                side_str = "Pos"
        else:
                side_str = "Neg"
        return [
                cms.InputTag("l1tPhase2HFEmulatorProducer", f"LinkOutIP1{side_str}EtaCh{6+(link%3)}")
                for link in range(8)
        ]
def _get_dmx_input_links(module_label):
        '''
        First 8 links are unset (reserved for HF), and last 5 are unset (unused).
        '''
        return [
                cms.InputTag(module_label, f"LinkIn{8+link}")
                for link in range(8)
        ]
mhh_pos_input_links = _get_hf_input_links(1)+_get_dmx_input_links("l1tPhase2L1DMXEmulatorPositive")
mhh_neg_input_links = _get_hf_input_links(-1)+_get_dmx_input_links("l1tPhase2L1DMXEmulatorNegative")
l1tPhase2L1MHHEmulatorPositive = l1tPhase2L1MHHEmulator.clone(
        inputLinks=cms.VInputTag(*mhh_pos_input_links)
)
l1tPhase2L1MHHEmulatorNegative = l1tPhase2L1MHHEmulator.clone(
        inputLinks=cms.VInputTag(*mhh_neg_input_links)
)


# GCTSum (only one card for now) - l1tPhase2L1GCTSumEmulator
from L1Trigger.L1CaloTrigger.l1tPhase2L1GCTSumEmulator_cfi import *
l1tPhase2L1GCTSumEmulator.inputLinks = cms.VInputTag(
        cms.InputTag("l1tPhase2L1MHHEmulatorPositive", "LinkOut0"), # MHH+ EGammas
        cms.InputTag("l1tPhase2L1MHHEmulatorPositive", "LinkOut1"), # MHH+ Jets+Taus
        cms.InputTag("l1tPhase2L1MHHEmulatorPositive", "LinkOut2"), # MHH+ Sums
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT1PostIP2LinkOut0"), # GCTB1 +eta EGammas
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT1PostIP2LinkOut1"), # GCTB1 +eta Jets+Taus
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT1PostIP2LinkOut2"), # GCTB1 +eta Sums
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT2PostIP2LinkOut0"), # GCTB2 +eta EGammas
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT2PostIP2LinkOut1"), # GCTB2 +eta Jets+Taus
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT2PostIP2LinkOut2"), # GCTB2 +eta Sums
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT3PostIP2LinkOut0"), # GCTB3 +eta EGammas
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT3PostIP2LinkOut1"), # GCTB3 +eta Jets+Taus
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT3PostIP2LinkOut2"), # GCTB3 +eta Sums
        cms.InputTag("l1tPhase2L1MHHEmulatorNegative", "LinkOut0"), # MHH- EGammas
        cms.InputTag("l1tPhase2L1MHHEmulatorNegative", "LinkOut1"), # MHH- Jets+Taus
        cms.InputTag("l1tPhase2L1MHHEmulatorNegative", "LinkOut2"), # MHH- Sums
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT1PostIP2LinkOut3"), # GCTB1 -eta EGammas
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT1PostIP2LinkOut4"), # GCTB1 -eta Jets+Taus
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT1PostIP2LinkOut5"), # GCTB1 -eta Sums
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT2PostIP2LinkOut3"), # GCTB2 -eta EGammas
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT2PostIP2LinkOut4"), # GCTB2 -eta Jets+Taus
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT2PostIP2LinkOut5"), # GCTB2 -eta Sums
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT3PostIP2LinkOut3"), # GCTB3 -eta EGammas
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT3PostIP2LinkOut4"), # GCTB3 -eta Jets+Taus
        cms.InputTag("l1tPhase2GCTEmulatorProducer", "GCT3PostIP2LinkOut5"), # GCTB3 -eta Sums
)

l1tPhase2CaloL1 = cms.Sequence(
        l1tPhase2RCTEmulatorProducer *
        l1tPhase2GCTEmulatorProducer *
        l1tPhase2HFEmulatorProducer *
        l1tPhase2L1DMXEmulatorPositive *
        l1tPhase2L1DMXEmulatorNegative *
        l1tPhase2L1MHHEmulatorPositive *
        l1tPhase2L1MHHEmulatorNegative *
        l1tPhase2L1GCTSumEmulator
)
