#ifndef RunAction_h
#define RunAction_h 1

#include "G4UserRunAction.hh"
#include "G4Accumulable.hh"
#include "globals.hh"
#include <vector>

// ROOT 클래스 전방 선언
class TFile;
class TH1F;

class RunAction : public G4UserRunAction
{
  public:
    RunAction();
    virtual ~RunAction();

    virtual void BeginOfRunAction(const G4Run*);
    virtual void EndOfRunAction(const G4Run*);

    // 기존 누적용
    void AddPhotonCount(G4int count);
    void AddEnergyDeposit(G4double energy);

    // 새로운 ROOT 기록용
    void FillWavelengths(const std::vector<G4double>& wavelengths);
    void FillNpe(G4int npe);

  private:
    // Accumulable (기존)
    G4Accumulable<G4int>    fTotalPhotonCount;
    G4Accumulable<G4double> fTotalEnergyDeposit;

    // ROOT 출력용
    TFile* rootFile = nullptr;
    TH1F*  hNpe = nullptr;         // 이벤트당 photoelectron 수
    TH1F*  hWavelength = nullptr;  // 파장 분포
};

#endif


