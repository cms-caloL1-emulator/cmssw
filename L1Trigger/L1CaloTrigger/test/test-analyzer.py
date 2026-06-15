import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras

process = cms.Process("HFEMU", eras.Phase2C17I13M9)

process.load("Configuration.StandardSequences.Services_cff")
process.load("FWCore.MessageService.MessageLogger_cfi")
process.load("Configuration.EventContent.EventContent_cff")
process.load("Configuration.StandardSequences.FrontierConditions_GlobalTag_cff")

process.maxEvents = cms.untracked.PSet(input=cms.untracked.int32(1))

process.source = cms.Source(
    "PoolSource",
    fileNames=cms.untracked.vstring(
        "file:/path/to/input.root",
    ),
)

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, "131X_mcRun4_realistic_v6", "")

process.load("SimCalorimetry.HcalTrigPrimProducers.hcaltpdigi_cff")
process.load("CalibCalorimetry.CaloTPG.CaloTPGTranscoder_cfi")
process.load("L1Trigger.L1CaloTrigger.l1tPhase2HFEmulatorProducer_cfi")

process.hfTbAnalyzer = cms.EDAnalyzer("Phase2L1CaloL1HFTestBenchAnalyzer")

process.out = cms.OutputModule(
    "PoolOutputModule",
    fileName=cms.untracked.string("hfemu_out.root"),
    outputCommands=cms.untracked.vstring(
        "drop *",
        "keep *_l1tPhase2HFEmulatorProducer_LinkOutIP1_*",
        "keep *_l1tPhase2HFEmulatorProducer_LinkOutIP2_*",
    ),
)

process.p = cms.Path(process.l1tPhase2HFEmulatorProducer * process.hfTbAnalyzer)
process.e = cms.EndPath(process.out)

process.options.numberOfThreads = cms.untracked.uint32(8)
process.options.numberOfStreams = cms.untracked.uint32(0)