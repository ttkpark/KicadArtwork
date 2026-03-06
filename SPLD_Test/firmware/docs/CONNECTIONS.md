# SPLD_Test 보드 연결도

## 개요
STM32F030K6와 CPLD, 1.3inch SPI OLED(M130-12864-7-V2.0) 연결을 정리한 문서입니다.

---

## OLED (M130-12864-7-V2.0) 연결

| OLED 핀 | STM32 핀 | 비고 |
|--------|----------|------|
| NSS/CS | **PA4**  | SPI Chip Select (소프트웨어 SPI 시 GPIO로 제어) |
| CLK/SCK | **PA5** | SPI Clock |
| MOSI   | **PA7**  | SPI Data (MOSI) |
| RST    | **PA8**  | OLED 리셋 (Active Low) |
| D/C    | **PA9**  | Data(1) / Command(0) 선택 |
| VCC    | 3.3V     | 전원 |
| GND    | GND      | 접지 |

- OLED는 **4-wire SPI** (CLK, MOSI, D/C, CS) + **RST** 사용.
- 펌웨어에서는 HAL SPI 모듈 없이 **GPIO 소프트웨어 SPI**로 제어합니다.

---

## 디버그 (SWD)

| 기능 | STM32 핀 |
|------|----------|
| SWDIO | PA13 |
| SWCLK | PA14 |

---

## STM32 ↔ CPLD GPIO

### CPLD 모니터 (5핀 입력)
| STM32 핀 | CPLD 핀 | 용도 |
|----------|---------|------|
| PA2  | IO28 | 모니터 입력 |
| PA3  | IO29 | 모니터 입력 |
| PA10 | IO31 | 모니터 입력 |
| PA12 | IO34 | 모니터 입력 |
| PA15 | IO37 | 모니터 입력 |

### CPLD 클럭 (8kHz 출력)
| STM32 핀 | CPLD 핀 | 용도 |
|----------|---------|------|
| **PA11** (TIM1_CH4) | 클럭 입력 | 8kHz PWM, 50% 듀티 |

---

## 핀 요약 (STM32 LQFP32)

| 포트 | OLED/SPI | 디버그 | CPLD |
|------|----------|--------|------|
| PA2  | -        | -      | IO28 |
| PA3  | -        | -      | IO29 |
| PA4  | NSS      | -      | -    |
| PA5  | CLK      | -      | -    |
| PA7  | MOSI     | -      | -    |
| PA8  | OLED RST | -      | -    |
| PA9  | D/C      | -      | -    |
| PA10 | -        | -      | IO31 |
| PA11 | -        | -      | IO33 |
| PA12 | -        | -      | IO34 |
| PA13 | -        | SWDIO  | -    |
| PA14 | -        | SWCLK  | -    |
| PA15 | -        | -      | IO37 |

---

## OLED 테스트

- `make`로 빌드 후 `build/firmware.bin`을 보드에 플래시하면 OLED에 테스트 화면이 표시됩니다.
- 테스트 화면: 테두리, 타이틀 "SPLD_Test", 간단한 패턴/문자.
