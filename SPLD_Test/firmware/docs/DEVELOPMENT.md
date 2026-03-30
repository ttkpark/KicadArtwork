# SPLD_Test 개발 문서

## 1) 프로젝트 목적

`SPLD_Test/firmware`는 STM32F030K6 기반 테스트 펌웨어입니다.

- CPLD에 8kHz 클럭(TIM1 CH4, PA11)을 공급
- CPLD 상태 입력 5개(PA2, PA3, PA10, PA12, PA15)를 주기적으로 샘플링
- 1.3" SH1106 OLED(SPI) 화면에 입력 상태를 아이콘/숫자로 실시간 표시

즉, **CPLD 동작을 눈으로 바로 검증하기 위한 모니터/패턴 출력 펌웨어**입니다.

---

## 2) 하드웨어 구성

### MCU
- STM32F030K6
- 시스템 클럭: HSI 8MHz

### OLED
- 모듈: M130-12864-7-V2.0 (SH1106 계열)
- 인터페이스: 4-wire SPI + RST

핀 매핑:
- `PA4`  : OLED CS(NSS, GPIO 제어)
- `PA5`  : SPI1 SCK
- `PA7`  : SPI1 MOSI
- `PA8`  : OLED RST
- `PA9`  : OLED D/C

### CPLD
- 입력 모니터: `PA2`, `PA3`, `PA10`, `PA12`, `PA15`
- 클럭 출력: `PA11` (TIM1_CH4 PWM, 8kHz, 50%)

---

## 3) 소프트웨어 구조

### 핵심 파일
- `Core/Src/main.c`
  - CubeMX 초기화 호출
  - TIM1 PWM 시작
  - 상태 읽기 + 화면 갱신 메인 루프
- `Core/Src/oled_sh1106.c`
  - SH1106 초기화 시퀀스
  - 프레임버퍼(1024B) 기반 드로잉/문자 출력
  - SPI 전송(CS/DC 제어 포함)
- `Core/Inc/oled_sh1106.h`
  - OLED API 선언
- `Core/Src/stm32f0xx_hal_msp.c`
  - SPI1/TIM1 GPIO MSP 설정 (CubeMX 생성)

### 동작 흐름
1. `HAL_Init()` / `SystemClock_Config()`
2. `MX_GPIO_Init()`, `MX_SPI1_Init()`, `MX_TIM1_Init()`
3. `HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4)`로 CPLD 클럭 출력 시작
4. `OLED_Init()` 후 정적 UI(테두리/제목/라벨) 출력
5. 루프:
   - GPIO 5개 읽기
   - 아이콘(원 채움/빈 원) + `0/1` 갱신
   - `OLED_Refresh()`
   - `HAL_Delay(50)`

---

## 4) 표시(UI) 규칙

- 상단 제목: `CPLD MON`
- 라벨: `28`, `29`, `31`, `34`, `37`
- 각 채널은 원형 아이콘 + 숫자 `0/1`로 표시
- 화면은 프레임버퍼 기반으로 부분 클리어 후 재렌더

참고:
- 현재 좌표계 보정 때문에 `OLED_SetPixel()`에서 X축 반전 처리(`x = OLED_WIDTH - 1 - x`)를 사용합니다.
- SH1106 방향은 초기화 커맨드(`0xA0/0xA1`, `0xC0/0xC8`)와 함께 패널 특성에 맞춰 조정합니다.

---

## 5) 빌드

프로젝트 루트(`SPLD_Test/firmware`)에서:

```bash
make
```

생성물:
- `build/firmware.elf`
- `build/firmware.bin`
- `build/firmware.hex`

주의:
- 현재 `Makefile`의 `clean` 타깃은 `rm -rf`를 사용하므로 Windows PowerShell 환경에서 실패할 수 있습니다.
- 이 경우 `make` 자체는 정상 동작하며 증분 빌드는 가능합니다.

---

## 6) STM32CubeMX 운용 정책

이 프로젝트는 **가능한 CubeMX 생성 자원(HAL init)을 우선 사용**합니다.

- 사용:
  - `MX_GPIO_Init()`
  - `MX_SPI1_Init()`
  - `MX_TIM1_Init()`
  - `HAL_SPI_Transmit()`
- 사용자 코드 위치:
  - `main.c`의 `/* USER CODE BEGIN ... */` 블록

권장:
- 핀/클럭/주변장치 변경은 CubeMX에서 먼저 수정 후 코드 재생성
- 재생성 후 사용자 로직(메인 루프, OLED 드라이버 사용자 구간)만 유지

---

## 7) 트러블슈팅 메모

### OLED가 한 줄만 보이는 경우
- SH1106 방향/매핑 문제 가능성:
  - `0xA0` vs `0xA1`
  - `0xC0` vs `0xC8`
  - display offset(`0xD3`) 확인
- 페이지 전송은 한 페이지를 버스트 전송으로 유지

### 표시 좌우/상하 반전
- 초기화 명령(SEG/COM remap)과 소프트웨어 좌표 변환(`OLED_SetPixel`)을 함께 확인

### CubeMX 재생성 후 동작 깨짐
- `MX_GPIO_Init()`에서 입력 핀/출력 핀 모드가 의도대로 유지되는지 확인
- `HAL_TIM_PWM_Start()` 호출 유무 확인

---

## 8) 향후 개선 아이디어

- OLED 드라이버에 런타임 회전/미러 옵션 추가
- 폰트 확장(소문자/기호) 및 정렬 유틸리티 함수 추가
- CPLD 채널 수/라벨을 테이블화하여 확장 용이성 개선
- Windows 친화적 `make clean` 개선(`rmdir /s /q` 또는 조건 분기)
