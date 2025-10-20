cmake_minimum_required(VERSION 3.16 FATAL_ERROR)
project(SiPM_Scintillator_Simulation)

# Geant4 패키지 찾기
find_package(Geant4 REQUIRED)

# include 디렉토리 추가
include_directories(${CMAKE_SOURCE_DIR}/include)

# 소스 파일 디렉토리 설정
set(SRC_DIR ${CMAKE_SOURCE_DIR}/src)

# 실행 파일 생성
add_executable(SiPM_Scintillator
    ${SRC_DIR}/DetectorConstruction.cc
    ${SRC_DIR}/PhysicsList.cc
    ${SRC_DIR}/PrimaryGeneratorAction.cc
    ${SRC_DIR}/RunAction.cc
    ${SRC_DIR}/EventAction.cc
    ${SRC_DIR}/ActionInitialization.cc
    ${SRC_DIR}/Main.cc
    ${SRC_DIR}/SteppingAction.cc
    ${SRC_DIR}/SiPMSensitiveDetector.cc
    )

# Geant4 라이브러리 연결
target_link_libraries(SiPM_Scintillator PRIVATE ${Geant4_LIBRARIES})

# 실행 파일 설치 위치 설정
install(TARGETS SiPM_Scintillator DESTINATION bin)

set(CMAKE_BUILD_TYPE Debug) # 디버깅 빌드 설정

file(COPY ${CMAKE_SOURCE_DIR}/macros DESTINATION ${CMAKE_BINARY_DIR})

# ROOT 사용 추가
find_package(ROOT REQUIRED COMPONENTS Hist Core Tree RIO)

# include 디렉토리 추가
include_directories(${ROOT_INCLUDE_DIRS})

# ROOT 라이브러리 경고 방지
add_definitions(${ROOT_CXX_FLAGS})

# ROOT 라이브러리 연결
target_link_libraries(SiPM_Scintillator PRIVATE ${ROOT_LIBRARIES})

