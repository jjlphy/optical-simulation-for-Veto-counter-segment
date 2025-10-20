#ifndef PrimaryGeneratorAction_h
#define PrimaryGeneratorAction_h 1


#include "G4VUserPrimaryGeneratorAction.hh"
#include "globals.hh"
#include "G4ThreeVector.hh"


class G4ParticleGun;
class G4Event;

class PrimaryGeneratorAction : public G4VUserPrimaryGeneratorAction
{
  public:
    PrimaryGeneratorAction();
    virtual ~PrimaryGeneratorAction();

    virtual void GeneratePrimaries(G4Event*);

  private:
    G4double SampleBetaEnergy(G4double Emax);
    G4ThreeVector SampleConeDirection(G4double maxTheta);
    G4ParticleGun* fParticleGun; // <-- 이름 맞추기
};

#endif
