#include "DetectorConstruction.hh"
#include "G4Box.hh"
#include "G4LogicalVolume.hh"
#include "G4PVPlacement.hh"
#include "G4LogicalSkinSurface.hh"
#include "G4LogicalBorderSurface.hh"
#include "G4OpticalSurface.hh"
#include "G4NistManager.hh"
#include "G4SystemOfUnits.hh"
#include "G4MaterialPropertiesTable.hh"
#include "G4VisAttributes.hh"
#include "CLHEP/Units/PhysicalConstants.h"
#include "SiPMSensitiveDetector.hh"
#include "G4SDManager.hh"
#include "Randomize.hh"
#include "G4SubtractionSolid.hh"
#include "G4Tubs.hh"
#include "G4Colour.hh"
#include <cmath>
#include <string>

DetectorConstruction::DetectorConstruction() {}
DetectorConstruction::~DetectorConstruction() {}

G4VPhysicalVolume* DetectorConstruction::Construct() {

  auto nist = G4NistManager::Instance();
  auto worldMat = nist->FindOrBuildMaterial("G4_AIR");
  auto teflonMat = nist->FindOrBuildMaterial("G4_POLYETHYLENE"); // PTFE 근사
  auto sipmMat   = nist->FindOrBuildMaterial("G4_Si");

  // ------------------ Elements ------------------
  G4Element* elC = nist->FindOrBuildElement("C");
  G4Element* elH = nist->FindOrBuildElement("H");
  G4Element* elO = nist->FindOrBuildElement("O");

  // ------------------ EJ-212 (Scintillator) ------------------
  G4Material* scintMat = new G4Material("EJ212",1.023*g/cm3,2);
  scintMat->AddElement(elC,4.69e22/(4.69e22+5.17e22));
  scintMat->AddElement(elH,5.17e22/(4.69e22+5.17e22));

  const G4int NUMENTRIES = 22;
  G4double Wavelength[NUMENTRIES] = {
    520*nm,510*nm,500*nm,490*nm,480*nm,475*nm,470*nm,465*nm,460*nm,455*nm,
    450*nm,445*nm,440*nm,435*nm,430*nm,425*nm,423*nm,420*nm,415*nm,410*nm,
    405*nm,400*nm
  };
  G4double photonEnergy[NUMENTRIES];
  for (int i=0;i<NUMENTRIES;i++)
    photonEnergy[i] = (CLHEP::h_Planck*CLHEP::c_light)/Wavelength[i];

  // EJ-212 scint emission (relative)
  G4double EmissionSpectrum[NUMENTRIES] = {
    0.03,0.05,0.07,0.10,0.16,0.20,0.25,0.30,0.38,0.45,
    0.55,0.65,0.70,0.70,0.75,0.90,1.00,0.95,0.75,0.42,
    0.18,0.05
  };
  G4double rindexScint[NUMENTRIES], absLengthScint[NUMENTRIES];
  for (int i=0;i<NUMENTRIES;i++) { rindexScint[i]=1.58; absLengthScint[i]=2.5*m; }

  auto scintMPT = new G4MaterialPropertiesTable();
  scintMPT->AddProperty("ABSLENGTH",photonEnergy,absLengthScint,NUMENTRIES);
  scintMPT->AddProperty("RINDEX",photonEnergy,rindexScint,NUMENTRIES);
  scintMPT->AddProperty("SCINTILLATIONCOMPONENT1",photonEnergy,EmissionSpectrum,NUMENTRIES);
  scintMPT->AddConstProperty("SCINTILLATIONYIELD",10000./MeV);
  scintMPT->AddConstProperty("RESOLUTIONSCALE",1.0);
  scintMPT->AddConstProperty("SCINTILLATIONTIMECONSTANT1",2.4*ns);
  scintMPT->AddConstProperty("SCINTILLATIONRISETIME1",0.9*ns);
  scintMat->SetMaterialPropertiesTable(scintMPT);

  // ------------------ World (Air) RINDEX ------------------
  auto airMPT = new G4MaterialPropertiesTable();
  G4double airRIndex[NUMENTRIES]; for (int i=0;i<NUMENTRIES;i++) airRIndex[i]=1.0003;
  airMPT->AddProperty("RINDEX", photonEnergy, airRIndex, NUMENTRIES);
  worldMat->SetMaterialPropertiesTable(airMPT);

  // ------------------ SiPM RINDEX ------------------
  G4double siRIndex[NUMENTRIES]; for (int i=0;i<NUMENTRIES;i++) siRIndex[i]=1.55;
  auto siMPT = new G4MaterialPropertiesTable();
  siMPT->AddProperty("RINDEX",photonEnergy,siRIndex,NUMENTRIES);
  sipmMat->SetMaterialPropertiesTable(siMPT);

  // ------------------ World ------------------
  auto solidWorld = new G4Box("World",0.5*m,0.5*m,0.5*m);
  auto logicWorld = new G4LogicalVolume(solidWorld,worldMat,"World");
  auto physWorld  = new G4PVPlacement(nullptr,{},logicWorld,"World",nullptr,false,0,true);

  // ------------------ SD ------------------
  auto siPMSD = new SiPMSensitiveDetector("SiPMSD");
  G4SDManager::GetSDMpointer()->AddNewDetector(siPMSD);

  // =========================================================
  //  Fiber materials (PS core / PMMA cladding) + Optical glue + WLS(간단)
  // =========================================================
  // PS Core: ρ=1.05, n=1.59
  G4Material* PS_Core = new G4Material("PS_Core", 1.05*g/cm3, 2);
  PS_Core->AddElement(elC, 8); PS_Core->AddElement(elH, 8);

  // PMMA Cladding: ρ=1.18, n=1.49
  G4Material* PMMA_Clad = new G4Material("PMMA_Clad", 1.18*g/cm3, 3);
  PMMA_Clad->AddElement(elC, 5); PMMA_Clad->AddElement(elH, 8); PMMA_Clad->AddElement(elO, 2);

  // Optical glue (coupling & groove filler): n=1.465
  G4Material* OpticalGlue = new G4Material("OpticalGlue", 1.18*g/cm3, 2);
  OpticalGlue->AddElement(elC, 5); OpticalGlue->AddElement(elH,10);

  // --- indices & bulk absorption (간단 상수 근사) ---
  G4double nPS[NUMENTRIES], nPMMA[NUMENTRIES], nGlue[NUMENTRIES];
  G4double absBulkPS[NUMENTRIES], absBulkPMMA[NUMENTRIES], absGlue[NUMENTRIES];
  for (int i=0;i<NUMENTRIES;i++) {
    nPS[i]   = 1.59;  nPMMA[i] = 1.49;  nGlue[i] = 1.465;
    absBulkPS[i]   = 12.*m;
    absBulkPMMA[i] = 20.*m;
    absGlue[i]     = 10.*m;
  }

  // --- WLS(간단 근사): 청색 흡수 → 녹색 방출 ---
  G4double wlsAbs[NUMENTRIES];   // m
  G4double wlsEmit[NUMENTRIES];  // relative
  for (int i=0;i<NUMENTRIES;i++) {
    G4double lam = Wavelength[i]/nm;
    if (lam >= 500.0)      wlsAbs[i] = 20.*m;
    else if (lam >= 480.0) wlsAbs[i] = 6.*m;
    else if (lam >= 460.0) wlsAbs[i] = 3.*m;
    else if (lam >= 440.0) wlsAbs[i] = 1.*m;
    else if (lam >= 420.0) wlsAbs[i] = 0.4*m;
    else                   wlsAbs[i] = 0.3*m;

    double mu = 500.0, sigma = 15.0;
    double g = std::exp(-0.5*std::pow((lam-mu)/sigma,2.0));
    wlsEmit[i] = (lam>=460.0 && lam<=580.0) ? g : 0.02*g;
  }

  auto mptPS = new G4MaterialPropertiesTable();
  mptPS->AddProperty("RINDEX",        photonEnergy, nPS,        NUMENTRIES);
  mptPS->AddProperty("ABSLENGTH",     photonEnergy, absBulkPS,  NUMENTRIES);
  mptPS->AddProperty("WLSABSLENGTH",  photonEnergy, wlsAbs,     NUMENTRIES);
  mptPS->AddProperty("WLSCOMPONENT",  photonEnergy, wlsEmit,    NUMENTRIES);
  mptPS->AddConstProperty("WLSTIMECONSTANT", 7.0*ns);
  PS_Core->SetMaterialPropertiesTable(mptPS);

  auto mptPMMA = new G4MaterialPropertiesTable();
  mptPMMA->AddProperty("RINDEX",    photonEnergy, nPMMA,       NUMENTRIES);
  mptPMMA->AddProperty("ABSLENGTH", photonEnergy, absBulkPMMA, NUMENTRIES);
  PMMA_Clad->SetMaterialPropertiesTable(mptPMMA);

  auto mptGlue = new G4MaterialPropertiesTable();
  mptGlue->AddProperty("RINDEX",    photonEnergy, nGlue,  NUMENTRIES);
  mptGlue->AddProperty("ABSLENGTH", photonEnergy, absGlue, NUMENTRIES);
  OpticalGlue->SetMaterialPropertiesTable(mptGlue);

  // =========================================================
  //  공통 빌더
  // =========================================================
  auto BuildScintSet = [&](G4LogicalVolume* mother, G4ThreeVector pos,
                           G4RotationMatrix* rot, G4String suffix)
  {
    auto ApplyRot = [&](const G4ThreeVector& v, G4RotationMatrix* r) {
      return (r == nullptr) ? v : (*r) * v;
    };

    // 신틸 기본 사이즈
    G4double scint_x=2*mm, scint_y=10*mm, scint_z=140*mm;
    G4double halfX = scint_x/2, halfY = scint_y/2, halfZ = scint_z/2;

    // --- (A) Groove 있는 신틸: +X 끝, 크기 1.2×1.2×140 ---
    auto solidScintBox = new G4Box("ScintBox"+suffix, halfX, halfY, halfZ);

    G4double groove_x = 1.2*mm;
    G4double groove_y = 1.2*mm;
    G4double groove_z = scint_z; // 140 mm

    auto solidGroove = new G4Box("GrooveBox"+suffix, groove_x/2, groove_y/2, groove_z/2);
    G4ThreeVector grooveShift(+halfX - groove_x/2, 0.0, 0.0); // +X에 밀착

    auto solidScint = new G4SubtractionSolid("ScintWithGroove"+suffix,
                                             solidScintBox, solidGroove,
                                             nullptr, grooveShift);

    auto logicScint = new G4LogicalVolume(solidScint, scintMat, "ScintLV"+suffix);
    auto scintPV    = new G4PVPlacement(rot, pos, logicScint,
                                        "Scintillator"+suffix, mother,false,0,true);

    // --- (B) Optical glue: Glue = Box - Cylinder(=파이버 자리) ---
    G4double tol = 0.01*mm;
    auto glueBox = new G4Box("GlueBoxRaw"+suffix,
                             groove_x/2 - tol, groove_y/2 - tol, groove_z/2 - tol);

    // 파이버 자리(원통) — r_hole = r_clad
    G4double r_clad = 0.500*mm;   // 싱글클래딩 1.0 mm
    G4double r_core = 0.480*mm;
    auto holeCyl = new G4Tubs("GlueHole"+suffix, 0., r_clad, groove_z/2, 0.*deg, 360.*deg);

    // 글루 좌표계에서 +X 끝 밀착: x = 0.6 - 0.5 = +0.1
    G4double x_in_glue = + (groove_x/2 - r_clad);
    G4ThreeVector holeShift(x_in_glue, 0., 0.);

    auto glueSolid = new G4SubtractionSolid("GlueMinusHole"+suffix,
                                            glueBox, holeCyl, nullptr, holeShift);
    auto glueLV    = new G4LogicalVolume(glueSolid, OpticalGlue, "GlueLV"+suffix);
    auto gluePV    = new G4PVPlacement(rot,
                        pos + ApplyRot(grooveShift, rot),
                        glueLV, "GluePV"+suffix, mother, false, 0, true);

    // (보기)
    auto vGlue = new G4VisAttributes(G4Colour(0.6,0.6,1.0,0.15)); vGlue->SetForceSolid(true);
    glueLV->SetVisAttributes(vGlue);

    // --- (C) Fiber: L=180 mm, zC=+20 mm (mother에 배치; glue는 구멍만 제공) ---
    G4double L_fiber  = 180.0*mm;
    G4double zC_fiber = +20.0*mm; // +Z로 40 mm 돌출 → 끝 z=+110

    G4ThreeVector fiberInGlue(x_in_glue, 0.0, zC_fiber);
    G4ThreeVector fiberWorldPos = pos + ApplyRot(grooveShift + fiberInGlue, rot);

    auto cladSolid = new G4Tubs("FiberClad"+suffix, 0., r_clad, L_fiber/2.0, 0.*deg, 360.*deg);
    auto cladLV    = new G4LogicalVolume(cladSolid, PMMA_Clad, "FiberCladLV"+suffix);
    auto cladPV    = new G4PVPlacement(nullptr, fiberWorldPos,
                                       cladLV, "FiberCladPV"+suffix, mother, false, 0, true);

    auto coreSolid = new G4Tubs("FiberCore"+suffix, 0., r_core, L_fiber/2.0, 0.*deg, 360.*deg);
    auto coreLV    = new G4LogicalVolume(coreSolid, PS_Core, "FiberCoreLV"+suffix);
    new G4PVPlacement(nullptr, G4ThreeVector(),
                      coreLV, "FiberCorePV"+suffix, cladLV, false, 0, true);

    // (보기)
    auto vClad = new G4VisAttributes(G4Colour(0.2,0.8,0.2,0.15)); vClad->SetForceSolid(true);
    auto vCore = new G4VisAttributes(G4Colour(0.2,0.8,0.2,0.35)); vCore->SetForceSolid(true);
    cladLV->SetVisAttributes(vClad);
    coreLV->SetVisAttributes(vCore);

    // --- (D) Coupling disk (OpticalGlue, 0.1 mm) + SiPM ---
    G4double zEnd  = zC_fiber + L_fiber/2.0;           // 파이버 +Z 끝 (+110)
    G4double coupT = 0.10*mm;

    auto coupDisk  = new G4Tubs("CouplingDisk"+suffix, 0., r_clad, coupT/2.0, 0.*deg, 360.*deg);
    auto coupLV    = new G4LogicalVolume(coupDisk, OpticalGlue, "CouplingLV"+suffix);
    auto coupPV    = new G4PVPlacement(nullptr,
                         pos + ApplyRot(grooveShift + G4ThreeVector(x_in_glue,0., zEnd + coupT/2.0), rot),
                         coupLV, "CouplingPV"+suffix, mother, false, 0, true);

    G4double siPMSizeXY = 1.3*mm, siPMThick = 0.3*mm;
    auto siPMBox   = new G4Box("SiPM"+suffix, siPMSizeXY/2, siPMSizeXY/2, siPMThick/2);
    auto siPMLogic = new G4LogicalVolume(siPMBox, sipmMat, "SiPMLogic"+suffix);
    siPMLogic->SetSensitiveDetector(siPMSD);

    G4double zSiPM = zEnd + coupT + siPMThick/2.0;
    auto sipmPV = new G4PVPlacement(rot,
                    pos + ApplyRot(grooveShift + G4ThreeVector(x_in_glue,0., zSiPM), rot),
                    siPMLogic, "SiPM"+suffix, mother, false, 0, true);

    // --- (E) 경계 마감 (polished: Fresnel만) ---
    auto polishedInt = new G4OpticalSurface("IntPolished"+suffix);
    polishedInt->SetType(dielectric_dielectric);
    polishedInt->SetModel(unified);
    polishedInt->SetFinish(polished);

    // Scint ↔ Glue (groove 표면)
    auto surfScintGlue = new G4OpticalSurface("SurfScintGlue"+suffix);
    surfScintGlue->SetType(dielectric_dielectric);
    surfScintGlue->SetModel(unified);
    surfScintGlue->SetFinish(polished);
    new G4LogicalBorderSurface("BScintGlue"+suffix, scintPV, gluePV, surfScintGlue);

    // Clad ↔ Coupling
    new G4LogicalBorderSurface("Border_Clad_Coupling"+suffix, cladPV, coupPV, polishedInt);
    // Coupling ↔ SiPM
    new G4LogicalBorderSurface("Border_Coupling_SiPM"+suffix, coupPV, sipmPV, polishedInt);

    // --- (F) Teflon wrapping (확산 반사) ---
    G4double t=0.01*mm, margin=0.001*mm;
    auto teflonOptSurface = new G4OpticalSurface("TeflonSurface"+suffix);
    teflonOptSurface->SetType(dielectric_dielectric);
    teflonOptSurface->SetModel(unified);
    teflonOptSurface->SetFinish(groundfrontpainted);

    auto teflonMPT = new G4MaterialPropertiesTable();
    G4double reflectivity[NUMENTRIES]; for (int i=0;i<NUMENTRIES;i++) reflectivity[i]=0.98;
    teflonMPT->AddProperty("REFLECTIVITY",photonEnergy,reflectivity,NUMENTRIES);
    teflonOptSurface->SetMaterialPropertiesTable(teflonMPT);

    auto boxBottom = new G4Box("TeflonBottom"+suffix, halfX+t, halfY+t, t/2);
    auto logicTeflonBottom = new G4LogicalVolume(boxBottom, teflonMat, "TeflonBottom"+suffix);
    new G4PVPlacement(rot, pos + ApplyRot(G4ThreeVector(0,0,-(halfZ+t/2)), rot),
                      logicTeflonBottom,"TeflonBottom"+suffix,mother,false,0,true);
    new G4LogicalSkinSurface("SurfBottom"+suffix,logicTeflonBottom,teflonOptSurface);

    auto boxSideX = new G4Box("TeflonSideX"+suffix, t/2, halfY+t - margin, halfZ - siPMThick - margin);
    auto logicTeflonLeft = new G4LogicalVolume(boxSideX, teflonMat, "TeflonLeft"+suffix);
    new G4PVPlacement(rot, pos + ApplyRot(G4ThreeVector(-(halfX + t/2),0,0), rot),
                      logicTeflonLeft,"TeflonLeft"+suffix,mother,false,0,true);
    new G4LogicalSkinSurface("SurfLeft"+suffix,logicTeflonLeft,teflonOptSurface);

    auto boxSideX2 = new G4Box("TeflonSideX2"+suffix, t/2, halfY+t - margin, halfZ - siPMThick - margin);
    auto logicTeflonRight = new G4LogicalVolume(boxSideX2, teflonMat, "TeflonRight"+suffix);
    new G4PVPlacement(rot, pos + ApplyRot(G4ThreeVector(+(halfX + t/2),0,0), rot),
                      logicTeflonRight,"TeflonRight"+suffix,mother,false,0,true);
    new G4LogicalSkinSurface("SurfRight"+suffix,logicTeflonRight,teflonOptSurface);

    auto boxSideY = new G4Box("TeflonSideY"+suffix, (halfX) - margin, t/2, halfZ - siPMThick - margin);
    auto logicTeflonFront = new G4LogicalVolume(boxSideY, teflonMat, "TeflonFront"+suffix);
    new G4PVPlacement(rot, pos + ApplyRot(G4ThreeVector(0,-(halfY + t/2),0), rot),
                      logicTeflonFront,"TeflonFront"+suffix,mother,false,0,true);
    new G4LogicalSkinSurface("SurfFront"+suffix,logicTeflonFront,teflonOptSurface);

    auto boxSideY2 = new G4Box("TeflonSideY2"+suffix, (halfX) - margin, t/2, halfZ - siPMThick - margin);
    auto logicTeflonBack = new G4LogicalVolume(boxSideY2, teflonMat, "TeflonBack"+suffix);
    new G4PVPlacement(rot, pos + ApplyRot(G4ThreeVector(0,+(halfY + t/2),0), rot),
                      logicTeflonBack,"TeflonBack"+suffix,mother,false,0,true);
    new G4LogicalSkinSurface("SurfBack"+suffix,logicTeflonBack,teflonOptSurface);
  };

  // ---- 단일 세트 배치 ----
  BuildScintSet(logicWorld, G4ThreeVector(0,0,0), nullptr, "_BVH1");

  logicWorld->SetVisAttributes(G4VisAttributes::GetInvisible());
  return physWorld;
}
