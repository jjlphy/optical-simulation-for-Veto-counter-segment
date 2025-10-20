#include "PhysicsList.hh"

#include "G4DecayPhysics.hh"
#include "G4EmStandardPhysics.hh"
#include "G4HadronPhysicsQGSP_BERT.hh"
#include "G4OpticalPhysics.hh"
#include "G4SystemOfUnits.hh"

PhysicsList::PhysicsList()
    : G4VModularPhysicsList()
{
    // 전역 컷값: 단위 mm
    SetDefaultCutValue(0.01 * mm);

    // 1. 붕괴 물리
    RegisterPhysics(new G4DecayPhysics());

    // 2. 전자기 물리
    RegisterPhysics(new G4EmStandardPhysics());

    // 3. 하드론 물리
    RegisterPhysics(new G4HadronPhysicsQGSP_BERT());

    // 4. 광학 물리
    auto opticalPhysics = new G4OpticalPhysics();
    // Geant4 11.2 이후는 모든 프로세스 자동 활성화
    // (Scintillation / Cerenkov 포함)
    // Yield scaling은 MaterialPropertiesTable로 제어
    RegisterPhysics(opticalPhysics);
}

PhysicsList::~PhysicsList() {}
