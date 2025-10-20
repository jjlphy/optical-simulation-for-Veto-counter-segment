#ifndef SteppingAction_hh
#define SteppingAction_hh

#include "G4UserSteppingAction.hh"
#include "EventAction.hh"
#include "G4SystemOfUnits.hh"

class SteppingAction : public G4UserSteppingAction {
public:
    SteppingAction(EventAction* eventAction); // 수정: EventAction 포인터를 받는 생성자 추가
    virtual ~SteppingAction();

    virtual void UserSteppingAction(const G4Step* step);

private:
    EventAction* fEventAction; // 수정: EventAction 포인터를 저장
};

#endif

