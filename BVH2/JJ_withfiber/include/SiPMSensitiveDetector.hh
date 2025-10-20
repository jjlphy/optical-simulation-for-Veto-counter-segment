#ifndef SIPM_SENSITIVE_DETECTOR_HH
#define SIPM_SENSITIVE_DETECTOR_HH

#include "G4VSensitiveDetector.hh"
#include "G4Step.hh"
#include "G4HCofThisEvent.hh"
#include "G4Track.hh"
#include "G4VTouchable.hh"
#include "G4TouchableHistory.hh"
#include "G4THitsCollection.hh"
#include "G4SystemOfUnits.hh"
#include <vector>
#include <string>

class SiPMSensitiveDetector : public G4VSensitiveDetector {
public:
    // Constructor and Destructor
    SiPMSensitiveDetector(const G4String& name);
    virtual ~SiPMSensitiveDetector();

    // Required methods for sensitive detectors
    virtual void Initialize(G4HCofThisEvent* hce) override;
    virtual G4bool ProcessHits(G4Step* step, G4TouchableHistory* history) override;
    virtual void EndOfEvent(G4HCofThisEvent* hce) override;

private:
    G4int fPhotonCount; // Counter for detected photons
};

#endif // SIPM_SENSITIVE_DETECTOR_HH

