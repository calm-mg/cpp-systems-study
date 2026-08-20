# 2강: 프로세스의 생성, 교체, 종료

## 이번 강의의 질문

> shell에서 실행한 프로그램은 어떻게 별도의 프로세스가 되고, 그 프로세스가
> 끝났다는 사실을 shell은 어떻게 알 수 있을까?

이번 강의에서는 이 흐름을 세 개의 POSIX 인터페이스로 관찰한다.

```text
fork() -> exec() -> waitpid()
복제       교체       회수
```

## 프로그램과 프로세스

**프로그램(program)**은 디스크에 저장된 실행 파일과 코드다. 아직 실행되지
않은 프로그램은 CPU register, process ID, 열린 파일 같은 실행 상태를 갖지
않는다.

**프로세스(process)**는 프로그램을 실행하기 위해 커널이 관리하는 실행
객체다. 커널은 프로세스마다 다음 상태를 추적한다.

- process ID와 parent process ID
- CPU register와 다음에 실행할 instruction 위치
- 가상 주소 공간과 memory mapping
- 열린 file descriptor 표
- signal, 권한, scheduling 상태

같은 실행 파일을 두 번 실행하면 코드의 출처는 같지만 서로 다른 프로세스가
생긴다. 두 프로세스는 서로 다른 PID와 가상 주소 공간을 가진다.

## 가상 주소와 프로세스 격리

포인터가 담는 숫자는 일반적으로 물리 주소가 아니라 현재 프로세스에서 의미를
갖는 **가상 주소**다. CPU의 memory management unit과 커널이 설정한 page
table이 가상 주소를 물리 page로 변환한다.

```text
프로세스 A의 가상 주소 0x1000 -> 물리 page X
프로세스 B의 가상 주소 0x1000 -> 물리 page Y
```

따라서 두 프로세스가 같은 숫자의 주소를 사용해도 일반적으로 서로의 값을
변경하지 않는다. 명시적으로 shared memory를 mapping한 경우에는 서로 다른
프로세스의 가상 주소가 같은 물리 page를 가리킬 수 있다.

## `fork()`: 현재 프로세스 복제

`fork()`는 호출한 프로세스를 바탕으로 새 자식 프로세스를 만든다. 호출 직후
부모와 자식은 `fork()` 다음 instruction부터 각각 실행을 계속한다.

```cpp
const pid_t child_pid = ::fork();

if (child_pid == 0) {
  // 자식에서 실행
} else if (child_pid > 0) {
  // 부모에서 실행, child_pid는 새 자식의 PID
} else {
  // 생성 실패
}
```

코드에는 호출이 한 번 있지만 결과는 두 프로세스에서 반환된 것처럼 보인다.

- 부모에게는 생성된 자식의 PID를 반환한다.
- 자식에게는 `0`을 반환한다.
- 실패하면 새 자식이 생기지 않고 호출자에게 `-1`을 반환한다.

부모와 자식의 메모리는 논리적으로 독립적이다. 실제로 모든 물리 page를 즉시
복사하면 비싸기 때문에 현대 POSIX OS는 보통 **Copy-on-Write(COW)**를 사용한다.
처음에는 page를 공유하되 쓰기 시도를 감지할 수 있도록 설정하고, 한쪽이
쓰려고 할 때 커널이 그 page를 복사해 분리한다.

```text
fork 직후: 부모 page ─┐
                      ├─ 같은 물리 page를 읽기 전용으로 공유
           자식 page ─┘

자식이 쓰기: 부모 page -> 기존 물리 page
             자식 page -> 복사된 물리 page
```

이는 관찰 가능한 동작 계약이 아니라 OS가 독립된 주소 공간을 효율적으로
구현하는 방법이다. 프로그램은 부모와 자식의 일반 메모리가 독립적이라는
동작에 의존해야 한다.

## `exec()`: 프로세스가 실행할 프로그램 교체

`exec` 계열 함수는 새 프로세스를 만들지 않는다. 성공하면 현재 프로세스의
program image를 새 실행 파일의 코드, 데이터, stack 등으로 교체한다.

```cpp
::execlp(file, file, "--after-exec", static_cast<char*>(nullptr));
```

성공한 `exec()`는 기존 코드로 반환하지 않는다. 새 프로그램의 시작 지점부터
실행하기 때문이다. 실패했을 때만 `-1`을 반환하고 `errno`를 설정한다.

교체 전후에도 같은 프로세스이므로 PID는 유지된다. 반면 기존 C++ 지역 변수와
heap 객체는 새 program image에서 그대로 사용할 수 없다. 열린 file
descriptor는 close-on-exec로 표시된 것을 제외하면 기본적으로 유지되며, 이
특성이 이후 pipe와 socket을 자식 프로그램에 연결할 때 사용된다.

## `waitpid()`: 자식의 종료 상태 회수

자식이 종료하면 커널은 부모가 종료 원인을 확인할 수 있도록 최소한의 정보를
남긴다. 부모는 `wait()` 또는 `waitpid()`로 그 정보를 회수한다.

```cpp
int status = 0;
const pid_t result = ::waitpid(child_pid, &status, 0);
```

`status`는 exit code 그 자체가 아니라 여러 상태가 encoding된 값이다. 먼저
macro로 종료 종류를 확인한 뒤에 세부 값을 읽어야 한다.

```cpp
if (WIFEXITED(status)) {
  const int exit_code = WEXITSTATUS(status);
} else if (WIFSIGNALED(status)) {
  const int signal_number = WTERMSIG(status);
}
```

자식은 이미 종료했지만 부모가 아직 상태를 회수하지 않은 동안 **zombie
process**라고 부른다. zombie는 코드를 계속 실행하는 프로세스가 아니며, PID와
종료 상태 같은 최소 정보가 process table에 남아 있는 상태다. 부모가 wait를
호출하면 커널이 이 정보를 제거할 수 있다.

## 사용자 공간과 커널 공간의 동작 흐름

shell이 **foreground 외부 명령 하나**를 실행하는 전형적인 흐름은 다음과
같다.

```text
1. shell 프로세스가 fork() 요청
2. 커널이 새 PID와 자식 실행 상태를 준비
3. 자식이 exec()로 자신을 명령 프로그램으로 교체
4. 커널이 실행 파일을 확인하고 새 주소 공간을 구성
5. 부모 shell은 waitpid()에서 자식 종료를 기다림
6. 자식이 종료하면 커널이 종료 상태를 보관하고 부모를 깨움
7. shell이 종료 상태를 회수하고 다음 prompt를 표시
```

background job에서는 shell이 즉시 prompt로 돌아올 수 있고, pipeline에서는
여러 자식과 file descriptor 연결을 함께 관리해야 한다. job control의 세부
동작은 이후 프로세스 간 통신과 terminal을 다룰 때 확장한다.

POSIX 함수 이름과 커널 내부 system call의 이름이나 개수는 항상 1:1이라고
가정하지 않는다. 우리가 사용하는 계약은 `fork()`, `exec()` 계열,
`waitpid()`가 제공하는 POSIX 동작이다.

## C++ 실험의 목적과 코드 읽기

[`process_lifecycle.cpp`](examples/process_lifecycle.cpp)는 하나의 실행 파일로 세
단계를 관찰한다.

1. 부모가 지역 변수 `value = 10`을 만든다.
2. `fork()` 후 자식만 `value = 20`으로 변경한다.
3. 자식이 자기 실행 파일을 `--after-exec` 인수로 다시 `exec()`한다.
4. 부모가 `waitpid()`로 자식의 exit code `42`를 회수한다.

`exec()` 전후 자식 PID가 같은지 보면 program image 교체가 새 프로세스 생성과
다르다는 것을 확인할 수 있다. 부모의 값이 끝까지 `10`이면 두 프로세스의
주소 공간이 독립적이라는 것도 확인된다.

`fork()` 전에 `std::cout`을 flush하는 이유도 중요하다. user-space 출력
buffer가 남은 상태로 `fork()`하면 그 buffer 상태까지 부모와 자식에 복제되어
같은 내용이 두 번 출력될 수 있다.

예제는 `execlp()`를 사용하므로 실행할 이름에 `/`가 없으면 `PATH`에서 파일을
찾고, `/`가 있으면 그 경로를 그대로 사용한다. 자식에서 `exec()`가 실패하면
예제는 `std::exit()` 대신 `_exit(127)`을
호출한다. `_exit()`은 상속받은 C++ stream buffer나 process 종료 handler를
다시 처리하지 않고 커널에 종료를 요청한다. 실제 멀티스레드 프로그램에서는
`fork()` 후 `exec()` 전 자식에서 호출할 수 있는 함수도 더 엄격히 제한되므로,
이번 단일 스레드 예제를 그대로 일반화하면 안 된다.

[`wait_status.cpp`](examples/wait_status.cpp)는 raw wait status를 다음 구조로
변환한다.

```cpp
struct ChildStatus {
  ChildState state;
  int detail;
};
```

정상 종료라면 `detail`은 exit code이고, signal 종료라면 signal 번호다.

## 빌드 및 실행

저장소 루트에서 실행한다.

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/lessons/02-process-and-syscall/process_lifecycle
```

## 예상 결과와 플랫폼 차이

출력 형태는 다음과 같다. PID 숫자는 실행할 때마다 달라질 수 있다.

```text
parent_before_fork pid=<parent PID> value=10
child_before_exec pid=<child PID> parent_pid=<parent PID> value=20
after_exec pid=<same child PID> program_image=replaced
parent_after_wait pid=<same parent PID> child_pid=<same child PID> value=10 child_state=exited child_detail=42
```

관찰해야 할 조건은 다음과 같다.

- 부모 PID와 자식 PID는 다르다.
- `child_before_exec`와 `after_exec`의 PID는 같다.
- 자식이 `20`을 출력해도 마지막 부모의 값은 `10`이다.
- 부모가 회수한 자식의 정상 exit code는 `42`다.

예제는 POSIX 환경인 macOS와 Linux를 대상으로 한다. Windows에서는 WSL2에서
실행한다. PID의 실제 범위와 scheduling에 따른 출력 시점은 환경마다 다를 수
있지만 위 조건은 유지되어야 한다.

## 흔한 오해

### “`fork()`는 같은 실행 파일을 처음부터 다시 실행한다”

아니다. 호출 시점의 프로세스 상태를 바탕으로 자식을 만들고, 부모와 자식 모두
`fork()` 다음 지점부터 실행을 이어간다.

### “COW를 사용하므로 부모와 자식은 같은 메모리를 공유한다”

물리 page를 일시적으로 공유할 수 있지만 일반 메모리의 관찰 가능한 값은
프로세스별로 독립적이다. 한쪽의 쓰기가 다른 쪽의 일반 변수 변경으로 보이지
않는다.

### “`exec()`는 새 자식 프로세스를 만든다”

아니다. 현재 프로세스의 program image를 교체한다. 새 프로세스가 필요하면
보통 먼저 `fork()`한 자식에서 `exec()`한다.

### “자식이 종료되면 모든 정보가 즉시 사라진다”

부모가 종료 상태를 읽을 수 있도록 일부 정보가 남는다. 부모가 이를 회수하지
않으면 자식은 zombie 상태로 남을 수 있다.

### “`waitpid()`가 반환한 `status`가 exit code다”

아니다. `WIFEXITED`, `WEXITSTATUS`, `WIFSIGNALED`, `WTERMSIG` 같은 macro로
해석해야 한다.

## 직접 확인할 문제

1. `fork()` 후 자식이 `value = 20`으로 바꿔도 부모가 `10`을 출력하는 이유를
   가상 주소와 물리 page 관점에서 설명해본다.
2. 출력에서 `child_before_exec`와 `after_exec`의 PID가 같은 이유를 설명한다.
3. `kChildExitCode`를 다른 값으로 바꾸고 부모가 동일한 값을 회수하는지
   확인한다.
4. `execlp()`의 실행 경로를 존재하지 않는 경로로 바꾸면 어떤 오류와 exit
   code가 관찰되는지 확인한다.
5. 자식이 끝난 뒤 부모가 바로 `waitpid()`하지 않도록 잠시 수정하고 `ps`에서
   zombie 상태가 어떻게 표시되는지 관찰한다. 실험 후에는 반드시 원래대로
   되돌린다.

## 다음 강의와의 연결

`fork()` 후 부모와 자식의 일반 메모리는 독립적이지만 열린 file descriptor는
상속된다. 다음 강의에서는 가상 메모리를 더 자세히 살펴본 뒤, file
descriptor가 파일, pipe, socket 같은 커널 객체를 어떻게 가리키는지
연결한다.
