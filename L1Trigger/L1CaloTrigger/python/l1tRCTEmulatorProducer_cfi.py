import FWCore.ParameterSet.Config as cms

l1tRCTEmulatorProducer = cms.EDProducer("Phase2L1CaloL1RCTEmulator",
    ecalTPEB = cms.InputTag("simEcalEBTriggerPrimitiveDigis"),
)

from Configuration.ProcessModifiers.premix_stage2_cff import premix_stage2
premix_stage2.toModify(l1tRCTEmulatorProducer,
    ecalTPEB = cms.InputTag("DMEcalEBTriggerPrimitiveDigis")
)