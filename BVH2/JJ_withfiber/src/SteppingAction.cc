#include "SteppingAction.hh"
#include "EventAction.hh"

#include "G4Step.hh"
#include "G4Track.hh"
#include "G4ParticleDefinition.hh"
#include "G4SystemOfUnits.hh"
#include "G4ios.hh"
#include "G4VProcess.hh"

SteppingAction::SteppingAction(EventAction* eventAction)
    : G4UserSteppingAction(),
      fEventAction(eventAction) {}

SteppingAction::~SteppingAction() {}

void SteppingAction::UserSteppingAction(const G4Step* step) {
    G4Track* track = step->GetTrack();
    auto particleDef = track->GetDefinition();
    auto particleName = particleDef->GetParticleName();

    // 1. 모든 하전 입자의 에너지 적산
    if (particleDef->GetPDGCharge() != 0.0) {
        G4double edep = step->GetTotalEnergyDeposit();
        if (fEventAction && edep > 0.) {
            fEventAction->AddEnergyDeposit(edep);
        }
    }
/*
    // 2. 모든 step에 대해 어떤 process가 불렸는지 출력 (디버깅용)
    auto postPoint = step->GetPostStepPoint();
    auto proc = postPoint->GetProcessDefinedStep();
    if (proc) {
        G4cout << "[DEBUG] proc=" << proc->GetProcessName()
               << " particle=" << particleName
               << " edep=" << step->GetTotalEnergyDeposit()/keV << " keV"
               << G4endl;
    }

 // 3. Scintillation 과정이면 세컨더리 광자 수 출력
    if (proc && proc->GetProcessName() == "Scintillation") {
        auto nSec = step->GetSecondaryInCurrentStep()->size();
        G4cout << "[DEBUG] Scintillation produced " << nSec
               << " photons at position "
               << postPoint->GetPosition() << G4endl;
    }
*/
    // 4. Optical photon 궤적을 보고 싶으면 주석 해제
    if (particleName == "opticalphoton") {
        // G4cout << "Optical photon step at: "
        //        << track->GetPosition() << G4endl;
    }
}
