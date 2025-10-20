#include "G4RunManager.hh"
#include "G4UImanager.hh"
#include "G4VisExecutive.hh"
#include "G4UIExecutive.hh"
#include "DetectorConstruction.hh"
#include "PhysicsList.hh"
#include "ActionInitialization.hh"

int main(int argc, char** argv) {
    auto* runManager = new G4RunManager();

    G4cout << "Initializing detector construction..." << G4endl;
    runManager->SetUserInitialization(new DetectorConstruction());

    G4cout << "Initializing physics list..." << G4endl;
    runManager->SetUserInitialization(new PhysicsList());

    G4cout << "Initializing actions..." << G4endl;
    runManager->SetUserInitialization(new ActionInitialization());

    G4VisManager* visManager = new G4VisExecutive();
    visManager->Initialize();

    G4UImanager* uiManager = G4UImanager::GetUIpointer();


    if (argc == 1) {
    // Interactive 모드
    G4UIExecutive* uiExecutive = new G4UIExecutive(argc, argv);
    uiManager->ApplyCommand("/control/execute ../macros/run.mac");
    uiExecutive->SessionStart();
    delete uiExecutive;
} else {
    // Batch 모드
    G4String command = "/control/execute ";
    G4String fileName = argv[1];
    uiManager->ApplyCommand(command + fileName);
}


    delete visManager;
    delete runManager;

    G4cout << "Simulation complete. Exiting." << G4endl;

    return 0;
}

