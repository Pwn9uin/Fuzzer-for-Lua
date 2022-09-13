# 1. About Token Level Fuzzer

이 퍼저는 USENIX에서 발표된 Token Level Fuzzer에 관한 아이디어를 바탕으로 AFL 퍼저에 접목하여 Lua 프로그램을 대상으로 퍼징을 동작할 수 있도록 자체 제작한 퍼저다.

- [USENIX Token Level Fuzzzer](https://www.usenix.org/conference/usenixsecurity21/presentation/salls#:~:text=Instead%20of%20applying%20mutations%20either,conform%20strictly%20to%20the%20grammar)


- [AFL-FUZZER](https://github.com/google/AFL)

위 token 폴더내에 token level 퍼저의 소스코드가 들어있으며, AFL 코드에서 내가 추가적으로 직접 작성한 부분은 TokenParse.h와 token이며, 해당 해더의 함수를 사용하기 위해 afl-fuzz.c 의 일부를 수정하였다.


# 2. 동작 과정

## 2.1 Token Level Fuzzer에 관한 아이디어

Token Level에 대한 자세한 아이디어는 위에서 언급한 USENIX ‘Token-Level-Fuzzer’를 참고하고, Lua에 맞게 제작하고 수정하는 과정을 거치며 만들어진 동작과정에 대해 간단하게 말하면 다음과 같다.

1. AFL 퍼저에서 input 파일을 읽어오기 직전, lua 인터프리터의 input인 lua script 파일을 encode하여 바이너리 형태로 만들고 해당 바이너리를 AFL 퍼저의 input파일로 전달한다.
2. AFL mutation을 통해 전달받은 바이너리를 mutation한다.
3. mutation된 바이너리를 다시 script파일 형태로 decode한 뒤, Lua 인터프리터를 실행할 때 입력값으로 전달한다.

이때 encode는 script를 token형태로 파싱한 다음, 각각의 token을 사전에 정의하여 매핑된 숫자로 바꾸는 작업이고(이때 token이 바뀐 숫자들이 모여 binary가 된다), decode는 다시 숫자(바이너리)를 token으로 바꾸어 script를 완성하는 작업이다.

- 아이디어에 대한 자세한 내용역시 USENIX 논문을 보면 알 수 있다.

## 2.2 동작 과정 코드

나는 AFL에서 TokenParse.h 헤더파일에 script를 파싱하고 token형태로 만드는데 필요한 함수들을 만든 후, AFL에서 필요한 부분에 호출할 수 있도록 만들었다. 그리고 TokenParse.h에 있는 함수들은 크게 2가지로 나누어 구분할 수 있다.

1. encode, encode_file
    - script를 파싱하여 token형태로 분리한다음 각각의 token을 갖고 binary를 만들어내는 함수이다.
2. decode
    - binary의 각각의 숫자(1byte)를 token과 매핑하여 script형식으로 만드는 함수이다.

encode_file은 afl-fuzz.c에서 perform_dry_run( 2766 line )과 input을 open하기 직전( 6641 line )에서 사용하고 있고, decode는 write_to_testcase( 2525 line ) 와 save_if_interesting( 3219 line, 3351 line)에서 사용하고 있다.

# 3. Token Level 퍼저 실행

build가 완료된 상태로 업로드 하였기 때문에 따로 빌드가 필요하지 않을 것으로 보인다. 

seed는 lua에서 기본적으로 제공하는 lua script testcase중 goto.lua 를 사용하였다. 다른 lua script를 사용할 경우, lua 버전과 다른 특이사항으로 인해 파싱과정중 오류가 발생할 수 있다.

실행 명령어는 다음과 같다.

```bash
./afl-fuzz -i ./seed -o output [lua 프로그램] @@
```

실행시 다음과 같은 화면을 볼 수 있다.
![token_level_fuzzer](https://user-images.githubusercontent.com/77731571/189562975-87f7b424-a25d-4421-b541-4aa050a70f8f.png)

# 4. 결과

Token Level 퍼저를 제작하기 위해 약 한달 반여간의 시간을 소요했다. BOB 프로젝트의 일환으로 진행했기 때문에 시간적 제약이 존재하였기에 일반 AFL mutation기법을 사용하였지만 interpreter를 대상으로 퍼징을 진행하는 만큼, mutation에 대한 연구가 더 진행되면 성능이 향상될 것이라고 생각한다. 또한 decode과정에서 줄바꿈과 같은 것은 고려하지 않고 모든 token을 공백으로 띄어서 처리했는데(실제로 lua는 이렇게 해도 큰 차이 없이 동작이 가능하다), parsing과정을 더 연구하여 더 나은 script 형태로 decode할 수 있기를 바란다.

token level 퍼저를 프로젝트 기간동안 동작하였지만, 위와 같은 이유(mutation과 parsing에서의 한계)로 인해 crash가 발견되지는 않았다. 하지만 앞선 언급한 문제들을 해결할 수 있다면 더 나은 token level 퍼저를 완성시킬 수 있을 것이라 판단한다.