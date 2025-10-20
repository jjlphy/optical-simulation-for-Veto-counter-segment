#include "RunAction.hh"
#include "G4Run.hh"
#include "G4UnitsTable.hh"
#include "G4AccumulableManager.hh"
#include "G4AnalysisManager.hh"

#include "TFile.h"
#include "TH1F.h"

RunAction::RunAction()
    : G4UserRunAction(),
      fTotalPhotonCount(0),
      fTotalEnergyDeposit(0.0),
      rootFile(nullptr),
      hNpe(nullptr),
      hWavelength(nullptr)
{
   auto accumulableManager = G4AccumulableManager::Instance();
accumulableManager->Register(fTotalPhotonCount);
accumulableManager->Register(fTotalEnergyDeposit);

}

RunAction::~RunAction() {}

void RunAction::BeginOfRunAction(const G4Run*) {
    auto accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Reset();

    // ROOT 파일과 히스토그램 생성
    rootFile = new TFile("../Histogram/sipm_output.root", "RECREATE");
    hNpe = new TH1F("hNpe", "Number of photoelectrons per event", 80, 0, 80);
    hWavelength = new TH1F("hWavelength", "Detected photon wavelength;Wavelength (nm);Counts", 120, 300, 900);

    G4cout << "Run started, accumulables reset." << G4endl;
}

void RunAction::EndOfRunAction(const G4Run* run) {
    auto accumulableManager = G4AccumulableManager::Instance();
    accumulableManager->Merge();

    G4int numEvents = run->GetNumberOfEvent();
    if (numEvents == 0) return;

    G4cout << "======================= Run Summary =======================" << G4endl;
    G4cout << "Number of events processed: " << numEvents << G4endl;
    G4cout << "Average number of photons per event: "
           << fTotalPhotonCount.GetValue() / static_cast<G4double>(numEvents) << G4endl;
    G4cout << "Average energy deposition per event: "
           << G4BestUnit(fTotalEnergyDeposit.GetValue() / numEvents, "Energy") << G4endl;

    // ROOT 파일 저장
    if (rootFile) {
        rootFile->cd();
        if (hNpe) hNpe->Write();
        if (hWavelength) hWavelength->Write();
        rootFile->Close();
        delete rootFile;
        rootFile = nullptr;
    }
}

void RunAction::AddPhotonCount(G4int count) {
    fTotalPhotonCount += count;
}

void RunAction::AddEnergyDeposit(G4double energy) {
    fTotalEnergyDeposit += energy;
}

// ---- 새로 추가된 함수 ----
void RunAction::FillWavelengths(const std::vector<G4double>& wavelengths) {
    if (!hWavelength) return;
    for (auto wl : wavelengths) {
        hWavelength->Fill(wl);
    }
}

void RunAction::FillNpe(G4int npe) {
    if (hNpe) hNpe->Fill(npe);
}
