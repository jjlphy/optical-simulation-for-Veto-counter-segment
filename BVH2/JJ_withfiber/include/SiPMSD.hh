#ifndef SIPMSD_HH
#define SIPMSD_HH

#include "G4VSensitiveDetector.hh"
#include "G4Step.hh"

class SiPMSD : public G4VSensitiveDetector {
public:
    SiPMSD(const G4String& name);
    virtual ~SiPMSD();

    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history);
};

#endif

