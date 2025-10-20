#include "EventAction.hh"
#include "RunAction.hh"
#include "G4Event.hh"
#include "G4SystemOfUnits.hh"

EventAction::EventAction(RunAction* runAction)
    : G4UserEventAction(),
      fPhotonCount(0),
      fEnergyDeposit(0),
      fRunAction(runAction)
{}

EventAction::~EventAction() {}

void EventAction::BeginOfEventAction(const G4Event*) {
    // 이벤트 시작 시 초기화
    fPhotonCount = 0;
    fEnergyDeposit = 0;
    fWavelengths.clear();
}

void EventAction::EndOfEventAction(const G4Event*) {
    // RunAction에 이벤트 결과 전달
    fRunAction->AddPhotonCount(fPhotonCount);
    fRunAction->AddEnergyDeposit(fEnergyDeposit);

    // 파장 정보 전달 (RunAction에서 히스토그램에 채움)
    if (fRunAction) {
        fRunAction->FillWavelengths(fWavelengths);
        fRunAction->FillNpe(fPhotonCount);
    }
}

void EventAction::AddPhoton() {
    fPhotonCount++;
}

void EventAction::AddEnergyDeposit(G4double energy) {
    fEnergyDeposit += energy;
}

void EventAction::AddWavelength(G4double wavelength) {
    fWavelengths.push_back(wavelength);
}

G4int EventAction::GetPhotonCount() const {
    return fPhotonCount;
}

G4double EventAction::GetTotalEnergyDeposit() const {
    return fEnergyDeposit;
}

