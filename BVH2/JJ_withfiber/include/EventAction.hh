#ifndef EventAction_h
#define EventAction_h 1

#include "G4UserEventAction.hh"
#include "globals.hh"
#include <vector>

class RunAction;

class EventAction : public G4UserEventAction
{
  public:
    EventAction(RunAction* runAction);
    virtual ~EventAction();

    virtual void BeginOfEventAction(const G4Event*);
    virtual void EndOfEventAction(const G4Event*);

    void AddPhoton();                       // 포톤 1개 추가
    void AddEnergyDeposit(G4double energy); // 에너지 누적
    void AddWavelength(G4double wavelength);// 파장 기록

    G4int GetPhotonCount() const;
    G4double GetTotalEnergyDeposit() const;
    const std::vector<G4double>& GetWavelengths() const { return fWavelengths; }

  private:
    G4int fPhotonCount;               // 이 이벤트에서 검출된 photoelectron 개수
    G4double fEnergyDeposit;          // 이벤트 동안 에너지 적산
    RunAction* fRunAction;            // RunAction 포인터
    std::vector<G4double> fWavelengths; // 검출된 광자의 파장 기록
};

#endif

