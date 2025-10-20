#include "SiPMSensitiveDetector.hh"
#include "EventAction.hh"

#include "G4SystemOfUnits.hh"
#include "G4Step.hh"
#include "G4Track.hh"
#include "G4EventManager.hh"
#include "CLHEP/Units/PhysicalConstants.h"
#include "G4OpticalPhoton.hh"
#include "Randomize.hh"

// ----------------------------------------------------------------------
// PDE 데이터 (Hamamatsu)
// ----------------------------------------------------------------------
namespace {
    const int Npde = 22;
    const double pde_wl[Npde] = {
        280,300,320,340,360,380,400,420,440,450,
        460,480,500,550,600,650,700,750,800,850,
        890,900
    };
    const double pde_val[Npde] = {
        0,2,3,10,17,26,35,38,39.5,40,
        39.8,39.2,39,32.5,27,20,15.5,11.5,9,5.5,
        4.5,4
    };

    double GetPDE(double wavelength_nm) {
        if (wavelength_nm <= pde_wl[0]) return pde_val[0] / 100.0;
        if (wavelength_nm >= pde_wl[Npde-1]) return pde_val[Npde-1] / 100.0;
        for (int i=0; i<Npde-1; i++) {
            if (wavelength_nm >= pde_wl[i] && wavelength_nm < pde_wl[i+1]) {
                double t = (wavelength_nm - pde_wl[i]) / (pde_wl[i+1] - pde_wl[i]);
                return ((1-t)*pde_val[i] + t*pde_val[i+1]) / 100.0;
            }
        }
        return 0.0;
    }
}

// ======================================================================
// 클래스 구현
// ======================================================================
SiPMSensitiveDetector::SiPMSensitiveDetector(const G4String& name)
    : G4VSensitiveDetector(name) {}

SiPMSensitiveDetector::~SiPMSensitiveDetector() {}

void SiPMSensitiveDetector::Initialize(G4HCofThisEvent* /*hce*/) {}

G4bool SiPMSensitiveDetector::ProcessHits(G4Step* step, G4TouchableHistory* /*history*/) {
    auto track = step->GetTrack();

    // Optical photon만 처리
    if (track->GetDefinition() != G4OpticalPhoton::OpticalPhotonDefinition())
        return false;

    // photon은 무조건 종료
    track->SetTrackStatus(fStopAndKill);

    // EventAction 가져오기
    auto eventAction = static_cast<EventAction*>(
        G4EventManager::GetEventManager()->GetUserEventAction());
    if (!eventAction) return false;

    // 파장 계산 (nm)
    double energy = step->GetTrack()->GetTotalEnergy();
    double wavelength = (CLHEP::h_Planck * CLHEP::c_light / energy) / nm;

    // PDE 확률
    double pde = GetPDE(wavelength);
    if (G4UniformRand() >= pde) return false; // 검출 실패

    // 검출 성공 시 카운트
    eventAction->AddPhoton();
   eventAction->AddWavelength(wavelength);

    // 디버그 출력 (100개마다)
    if (eventAction->GetPhotonCount() % 100 == 0) {
        G4cout << "[SiPM] Photon detected! Count = "
               << eventAction->GetPhotonCount() << G4endl;
    }

    return true;
}

void SiPMSensitiveDetector::EndOfEvent(G4HCofThisEvent* /*hce*/) {
    G4cout << "End of event for SiPM sensitive detector." << G4endl;
}

