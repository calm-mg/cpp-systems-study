# cpp-systems-study

운영체제와 네트워크의 기본 원리를 C++20 코드로 직접 확인하는 학습
저장소입니다.

목표는 소켓 함수를 외우는 것이 아니라 다음 연결을 설명하고 실험할 수 있게
되는 것입니다.

```text
C++ 사용자 프로그램
        ↓ 라이브러리와 POSIX API
필요한 경우 시스템 호출 경계 통과
        ↓
커널의 프로세스·메모리·파일·네트워크 관리
        ↓
하드웨어와 다른 프로세스
```

## 학습 방식

각 강의는 다음 순서로 진행합니다.

1. 질문과 개념
2. 사용자 공간과 커널 공간의 흐름
3. 최소 C++ 실험
4. 빌드와 실행
5. 결과 해석과 흔한 오해
6. 직접 해볼 문제
7. 다음 주제와의 연결

진도와 다음 작업은 [`ROADMAP.md`](ROADMAP.md)에 기록합니다. 여러 장비에서
작업할 때도 이 파일을 기준으로 이어서 진행합니다.

## 지원 환경

| 환경 | 공통 POSIX 강의 | 플랫폼 전용 I/O |
| --- | --- | --- |
| macOS | 지원 | `kqueue` 강의 |
| Linux | 지원 | `epoll`, `eventfd` 강의 |
| Windows + WSL2 | Linux 환경으로 지원 | Linux 강의와 동일 |
| Windows 네이티브 | 비교 자료만 제공 | WinSock 선택 자료 |

기본 요구사항은 CMake 3.20 이상과 C++20 컴파일러입니다.

### Windows에서 WSL2로 실행하기

관리자 권한 PowerShell에서 Ubuntu 기반 WSL2를 설치합니다.

```powershell
wsl --install -d Ubuntu
```

Windows 재시작과 Ubuntu 초기 설정을 마친 뒤, WSL terminal에서 필요한 도구와
저장소를 준비합니다. 다음 명령은 Ubuntu를 기준으로 한 예시입니다.

```bash
sudo apt update
sudo apt install -y build-essential cmake git
git clone https://github.com/calm-mg/cpp-systems-study.git
cd cpp-systems-study
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

`fork()`, `exec()`, Unix domain socket 같은 POSIX 실습은 WSL2 안에서 실행합니다.
Windows 네이티브 Visual Studio 빌드는 현재 공통 강의의 지원 대상이 아닙니다.

## 커리큘럼

1. 커널, 권한 모드, 시스템 호출 경계
2. 프로세스와 시스템 호출
3. 가상 메모리
4. 파일 디스크립터
5. 소켓의 정체
6. TCP
7. UDP
8. Unix domain socket
9. blocking, non-blocking I/O와 readiness API
10. 메시지 framing과 멀티클라이언트 서버

## 빌드와 실행

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
```

각 실험의 실행 명령은 해당 강의의 `README.md`에 기록합니다.

## 다른 장비에서 이어서 하기

저장소를 최신 상태로 준비한 뒤 저장소 루트에서 작업 도구를 실행하고 다음과
같이 요청하면 됩니다.

> `AGENTS.md와 ROADMAP.md를 읽고 현재 브랜치, 미완료 변경, 최근 진도를 확인한 다음 OS 학습을 이어서 진행해줘. 개념 설명 후 C++ 실험과 검증까지 하고 ROADMAP을 갱신해.`

작업 도구는 대화 기록 대신 저장소의 `ROADMAP.md`, 현재 강의 문서, Git
상태를 기준으로 다음 단계를 결정해야 합니다.
