#include "PrimaryGeneratorAction.hh"
#include "G4ParticleGun.hh"
#include "G4ParticleTable.hh"
#include "G4SystemOfUnits.hh"
#include "G4Event.hh"
#include "Randomize.hh"
#include "CLHEP/Units/PhysicalConstants.h"
#include <cmath>

// ====== 토글 매크로 ======
// 코스믹 뮤온 모드 (주석 해제 시 코스믹만 발생; 콜리메이터 무시)
 #define USE_COSMIC_RAY

// 입사선이 신틸레이터 10x140 mm 입구면의 정중앙(-1 mm, 0, 0)을 정확히 지나가도록 조준
// (Sr-90/코스믹 모두에 적용)
// #define PASS_THROUGH_CENTER
// =========================

PrimaryGeneratorAction::PrimaryGeneratorAction()
 : fParticleGun(nullptr)
{
    fParticleGun = new G4ParticleGun(1);
}

PrimaryGeneratorAction::~PrimaryGeneratorAction()
{
    delete fParticleGun;
}

// -------------------------
// Sr-90 / Y-90 베타 에너지 샘플러 (단순 모델)
// -------------------------
G4double PrimaryGeneratorAction::SampleBetaEnergy(G4double Emax)
{
    while (true) {
        G4double E = Emax * G4UniformRand();
        G4double P = std::pow(1.0 - E / Emax, 2) * E * E;
        G4double maxP = Emax * Emax;
        if (G4UniformRand() * maxP < P) return E; // [MeV]
    }
}

// -------------------------
// x축 중심 원뿔 각 내 방향 (Sr-90 콜리메이터용)
// -------------------------
G4ThreeVector PrimaryGeneratorAction::SampleConeDirection(G4double maxTheta)
{
    G4double costheta_min = std::cos(maxTheta);
    G4double costheta = costheta_min + (1.0 - costheta_min) * G4UniformRand();
    G4double sintheta = std::sqrt(1.0 - costheta * costheta);
    G4double phi = 2.0 * CLHEP::pi * G4UniformRand();
    // x 성분이 진행방향(+x)
    return G4ThreeVector(costheta, sintheta * std::cos(phi), sintheta * std::sin(phi));
}

// -------------------------
// from→to 조준 단위벡터
// -------------------------
static inline G4ThreeVector AimDir(const G4ThreeVector& from, const G4ThreeVector& to)
{
    G4ThreeVector v = (to - from).unit();
    // 수치오차로 x<0가 되면 +x로 향하도록 보정 (희귀 케이스)
    if (v.x() < 0) v.setX(std::fabs(v.x()));
    return v;
}

void PrimaryGeneratorAction::GeneratePrimaries(G4Event* anEvent)
{
    // ===== DetectorConstruction과 합의된 기하 파라미터 =====
    const G4double collimatorRadius = 3.53 * mm;    // 7.06 mm / 2
    const G4double collimatorLength = 33.01 * mm;   // 길이
    const G4double scintThickness   = 2.0  * mm;    // x-두께
    const G4double halfScintX       = 0.5  * scintThickness; // = 1 mm

    // 신틸레이터 입구면(10x140) 중심 좌표: x = -1 mm, y=z=0
    const G4ThreeVector target(-halfScintX, -2.0, 0.0);

#ifdef USE_COSMIC_RAY
    // ======================= 코스믹 뮤온 모드 =======================
    // 콜리메이터는 사용하지 않음.

    // 입자 정의: μ-
    G4ParticleDefinition* mu = G4ParticleTable::GetParticleTable()->FindParticle("mu-");
    fParticleGun->SetParticleDefinition(mu);

    // 시작 위치: 신틸레이터 위쪽(+z)에서 충분히 떨어진 곳 (예: z=+200 mm)
    // x는 입구면과 같은 -1 mm에 두면 중심 조준 시 직관적임
    // 필요하면 y,z 범위를 넓혀서 샘플링해도 됨.
   G4ThreeVector posCosmic(-200.0 * mm, 0.0 * mm, 0.0 * mm);


    // 방향
#ifdef PASS_THROUGH_CENTER
    // 중앙을 정확히 지나가도록 조준
    G4ThreeVector dirCosmic = AimDir(posCosmic, target);
#else
    // 간단히 중앙을 향하되, 소량의 각 분산(예: 최대 5°)을 부여하려면 아래 주석 해제
    // G4double maxTheta = 5.0 * deg;
    // G4ThreeVector ideal = AimDir(posCosmic, target);
    // // ideal을 중심으로 작은 원뿔 각 분포를 만들고 싶다면 회전 구현 필요(여기선 단순화)
    // G4ThreeVector dirCosmic = ideal;
    G4ThreeVector dirCosmic = AimDir(posCosmic, target);
#endif

    fParticleGun->SetParticlePosition(posCosmic);
    fParticleGun->SetParticleMomentumDirection(dirCosmic);
    fParticleGun->SetParticleEnergy(3.0 * GeV); // 대표 MIP 뮤온 에너지

#else
    // ======================= Sr-90 / Y-90 모드 (콜리메이터 적용) =======================
    // 입자 정의: 전자
    G4ParticleDefinition* electron = G4ParticleTable::GetParticleTable()->FindParticle("e-");
    fParticleGun->SetParticleDefinition(electron);

    // 콜리메이터 출구 원판 내부에서 균일한 (y,z) 위치 샘플
    G4double r   = collimatorRadius * std::sqrt(G4UniformRand());
    G4double phi = 2.0 * CLHEP::pi * G4UniformRand();
    G4double y   = r * std::cos(phi);
    G4double z   = r * std::sin(phi);
    G4double x   = -(collimatorLength/2.0 + halfScintX); // 출구면 x

    G4ThreeVector pos(x, y, z);

    // 방향
#ifdef PASS_THROUGH_CENTER
    // 출구면 임의 위치 → 신틸 입구면 중앙으로 조준 (개구각 물리와 약간 불일치 가능)
    G4ThreeVector dir = AimDir(pos, target);
#else
    // 현실적: 콜리메이터 최대 반각 내에서 샘플
    G4double maxTheta = std::atan(collimatorRadius / collimatorLength);
    G4ThreeVector dir = SampleConeDirection(maxTheta);
#endif

    // Sr-90 (50%) / Y-90 (50%) — 필요 시 Sr-90만 쓰려면 아래 분기 정리
    if (G4UniformRand() < 0.5) {
        // Sr-90: Emax ≈ 0.546 MeV
        G4double E = SampleBetaEnergy(0.546);
        fParticleGun->SetParticlePosition(pos);
        fParticleGun->SetParticleMomentumDirection(dir);
        fParticleGun->SetParticleEnergy(E * MeV);
    } else {
        // Y-90: Emax ≈ 2.28 MeV
        G4double E = SampleBetaEnergy(2.28);
        fParticleGun->SetParticlePosition(pos);
        fParticleGun->SetParticleMomentumDirection(dir);
        fParticleGun->SetParticleEnergy(E * MeV);
    }
#endif

    // 발사
    fParticleGun->GeneratePrimaryVertex(anEvent);
}
