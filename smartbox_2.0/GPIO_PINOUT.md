# SmartBox 2.0 nRF52840 GPIO 핀 배치

nRF52840 QIAA aQFN73 패키지 기준으로 주변 장치 GPIO 할당을 정리합니다.

---

## 핀 배치 요약

| 기능 | GPIO 핀 | 핀 번호 | 비고 |
|------|---------|---------|------|
| **E-Paper SPI** | | | |
| └ SCK | P0.11 | T2 | SPI0_SCK |
| └ MOSI | P0.12 | U1 | SPI0_MOSI |
| └ CS | P0.13 | AD8 | SPI0_CS |
| └ DC | P0.14 | AC9 | Data/Command |
| └ RST | P0.15 | AD10 | Reset |
| └ BUSY | P0.16 | AC11 | Busy 신호 (입력) |
| **RGB LED** | | | |
| └ LED_R | P0.17 | AD12 | PWM 가능 |
| └ LED_G | P0.19 | AC15 | PWM 가능 |
| └ LED_B | P0.20 | AD16 | PWM 가능 |
| **온도 센서** | | | |
| └ TEMP_ADC | P0.28/AIN4 | B11 | ADC 입력 (NTC 서미스터) |
| **배터리 측정** | | | |
| └ BAT_ADC | P0.29/AIN5 | A10 | ADC 입력 (배터리 전압 분압) |
| └ BAT_MOSFET | P0.21 | AC17 | 배터리 측정 MOSFET 제어 (GND 측, 출력) |
| **전원 제어** | | | |
| └ VCC_EN | P0.25 | AC21 | 외부 장치 3.3V 제어 (Load Switch) |
| **버튼** | | | |
| └ BTN_USER1 | P0.22 | AD18 | 유저 버튼 1 (Pull-up, 입력) |
| └ BTN_USER2 | P0.23 | AC19 | 유저 버튼 2 (Pull-up, 입력) |
| └ BTN_POWER | P0.24 | AD20 | 파워 버튼 (Wake-up 가능, Pull-up, 입력) |

**총 사용 핀: 16개** (VCC_EN 추가)

---

## 상세 설명

### 1. E-Paper Display (SPI 통신)

| 핀 이름 | GPIO | 물리 핀 | 기능 | 방향 |
|---------|------|---------|------|------|
| EPD_SCK | P0.11 | T2 | SPI Clock | 출력 |
| EPD_MOSI | P0.12 | U1 | SPI Data | 출력 |
| EPD_CS | P0.13 | AD8 | Chip Select (Active Low) | 출력 |
| EPD_DC | P0.14 | AC9 | Data/Command (0=CMD, 1=Data) | 출력 |
| EPD_RST | P0.15 | AD10 | Reset (Active Low) | 출력 |
| EPD_BUSY | P0.16 | AC11 | Busy 신호 (0=Busy, 1=Ready) | 입력 |

**설정:**
- SPI0 사용 (SCK, MOSI, CS)
- EPD_DC, EPD_RST는 일반 GPIO
- EPD_BUSY는 입력, Pull-up 활성화
- SPI 속도: 4MHz 이하 권장 (E-Paper 특성상)

**초저전력 모드 (EPD 전원 차단 시):**
- ⚠️ **GPIO 누설 전류 주의:** 전원 차단 전 GPIO 설정 변경 필수
- 출력 핀 (SCK, MOSI, CS, DC, RST): LOW로 설정
- 입력 핀 (BUSY): Pull 비활성화
- 누설 전류: 최적화 전 ~355μA → 최적화 후 < 1μA
- 상세 내용: `GPIO_LEAKAGE_CURRENT.md` 참조

---

### 2. RGB LED (Pick-to-Light)

| 핀 이름 | GPIO | 물리 핀 | 기능 | 비고 |
|---------|------|---------|------|------|
| LED_R | P0.17 | AD12 | Red LED | PWM 채널 0 |
| LED_G | P0.19 | AC15 | Green LED | PWM 채널 1 |
| LED_B | P0.20 | AD16 | Blue LED | PWM 채널 2 |

**설정:**
- 공통 캐소드(CC) 또는 공통 애노드(CA)에 따라 저항/드라이버 구성
- PWM으로 밝기 제어 가능 (0~255)
- Pick-to-Light 시 점멸 제어

**회로 제안:**
- 각 LED에 330Ω~1kΩ 저항 직렬 연결
- 필요 시 트랜지스터 드라이버 추가 (높은 전류 필요 시)

---

### 3. 온도 센서 (NTC 서미스터)

| 핀 이름 | GPIO | 물리 핀 | 기능 | 비고 |
|---------|------|---------|------|------|
| TEMP_ADC | P0.28/AIN4 | B11 | ADC 입력 | NTC 분압 회로 |

**회로 구성:**
```
VDD_nRF ──┬── 10kΩ (상단 저항)
          │
          ├── TEMP_ADC (P0.28/AIN4)
          │
          └── NTC 10kΩ @ 25℃ (하단, GND)
```

**설정:**
- ADC 채널 4 (AIN4)
- 해상도: 10-bit 또는 12-bit
- 참조 전압: VDD_nRF (내부)
- 샘플링 시간: 충분히 길게 설정 (NTC 안정화)

**대안:** 디지털 온도 센서(예: DS18B20, TMP102) 사용 시 I2C/1-Wire로 변경 가능

---

### 4. 배터리 전압 측정

| 핀 이름 | GPIO | 물리 핀 | 기능 | 비고 |
|---------|------|---------|------|------|
| BAT_ADC | P0.29/AIN5 | A10 | ADC 입력 | 배터리 전압 분압 |
| BAT_MOSFET | P0.21 | AC17 | MOSFET 제어 | 배터리 측정 스위치 |

**회로 구성 (초저전력 설계 - Low-side MOSFET):**
```
배터리+ ──┬── R1 (1MΩ~2MΩ)  [초저전력: 100kΩ → 1MΩ~2MΩ]
          │
          ├── BAT_ADC (P0.29/AIN5)
          │
          └── R2 (1MΩ~2MΩ) ──┬── BAT_MOSFET (P0.21) ──► N-MOSFET Gate
                             │                              │
                             └── N-MOSFET Drain ──► GND
```

**동작:**
1. 배터리 측정 시: BAT_MOSFET = HIGH → N-MOSFET ON → GND 경로 활성화 → 전류 흐름
2. 측정 후: BAT_MOSFET = LOW → N-MOSFET OFF → 전류 차단 (< 0.1μA)
3. Sleep 모드: MOSFET OFF 상태 유지, ADC 비활성화

**장점:**
- GND 측 스위칭으로 더 낮은 누설 전류 (< 0.1μA)
- MOSFET OFF 시 분압 저항에 전류가 흐르지 않음
- N-MOSFET 사용 (P-MOSFET보다 저렴)

**설정:**
- ADC 채널 5 (AIN5)
- 분압 비율: 1:1 (배터리 전압의 절반을 측정)
- 배터리 전압 = ADC 값 × 2 × VREF / ADC_MAX
- 측정 주기: 5분~10분마다 한 번씩 (일반 모드), Sleep 모드에서는 Wake-up 시에만
- **ADC 입력 Pull 비활성화** (누설 전류 방지)
- **MOSFET 선택**: 낮은 누설 전류 (< 1nA), 예) BSS138, DMN2004

**초저전력 최적화:**
- 분압 저항: 1MΩ ~ 2MΩ (전류 소비: ~2μA → < 1μA)
- MOSFET OFF 시 전류: < 1μA
- 측정 시간: 최소화 (~5ms)
- Sleep 모드 진입 전: MOSFET OFF, ADC 비활성화 필수

**상세 설계 가이드:** 
- `LOW_POWER_BATTERY_MEASUREMENT.md` - 기본 설계
- `ULTRA_LOW_POWER_DESIGN.md` - Low-side MOSFET 설계 (5μA 이하 목표)

---

### 5. 외부 장치 전원 제어 (Load Switch)

| 핀 이름 | GPIO | 물리 핀 | 기능 | 비고 |
|---------|------|---------|------|------|
| VCC_EN | P0.25 | AC21 | 외부 장치 3.3V 제어 | Load Switch 제어 |

**회로 구성:**
```
VDD_3V3 ──┬── P-MOSFET Source
          │       │
          │   P-MOSFET Gate ──► N-MOSFET Drain
          │       │                  │
          │       │              N-MOSFET Gate ──► VCC_EN (P0.25)
          │       │                  │
          │       │              N-MOSFET Source ──► GND
          │       │
          │   P-MOSFET Drain ──► VDD_EXT (외부 장치 전원)
          │
          └── Pull-up (100kΩ~1MΩ) ──► P-MOSFET Gate
```

**동작:**
- VCC_EN = HIGH → N-MOS ON → P-MOS Gate = GND → P-MOS ON → VDD_EXT = 3.3V
- VCC_EN = LOW → N-MOS OFF → P-MOS Gate = 3.3V → P-MOS OFF → VDD_EXT = 0V

**제어 대상:**
- ✅ EPD (E-Paper Display)
- ✅ RGB LED
- ✅ 온도 센서 (NTC Pull-up)

**제외 대상:**
- ❌ 버튼 (Wake-up 기능 필요)

**설정:**
- Sleep 모드 진입 전: VCC_EN = LOW (외부 전원 차단)
- Wake-up 시: VCC_EN = HIGH (외부 전원 공급)

**상세 설계 가이드:** `ULTRA_LOW_POWER_DESIGN.md` 참조

---

### 5. 버튼 입력

| 핀 이름 | GPIO | 물리 핀 | 기능 | 비고 |
|---------|------|---------|------|------|
| BTN_USER1 | P0.22 | AD18 | 유저 버튼 1 | Pull-up, 입력 |
| BTN_USER2 | P0.23 | AC19 | 유저 버튼 2 | Pull-up, 입력 |
| BTN_POWER | P0.24 | AD20 | 파워 버튼 | Wake-up 가능, Pull-up, 입력 |

**회로 구성:**
```
VDD_nRF ──┬── 10kΩ Pull-up
          │
          ├── GPIO (P0.22/23/24)
          │
          └── 버튼 ──► GND
```

**설정:**
- Pull-up 활성화 (내부 또는 외부)
- 버튼 누름 = LOW, 떼면 = HIGH
- BTN_POWER는 **GPIO Wake-up** 기능 사용 (Deep Sleep에서 깨우기)
- 디바운싱: 소프트웨어 또는 하드웨어(RC) 처리

**Wake-up 설정 (BTN_POWER):**
- `GPIO_INT_TYPE_EDGE` (상승/하강 엣지)
- `GPIO_INT_POLARITY_HI_TO_LO` 또는 `LO_TO_HI`
- Deep Sleep 모드에서도 Wake-up 가능하도록 설정

---

## 사용하지 않는 핀 (예비/확장용)

| GPIO | 물리 핀 | 비고 |
|------|---------|------|
| P0.02/AIN0 | A12 | 예비 ADC |
| P0.03/AIN1 | B13 | 예비 ADC |
| P0.04/AIN2 | J1 | 예비 ADC |
| P0.05/AIN3 | K2 | 예비 ADC |
| P0.06 | L1 | 예비 GPIO |
| P0.07 | M2 | 예비 GPIO |
| P0.08 | N1 | 예비 GPIO |
| P0.09/NFC1 | L24 | NFC (선택) |
| P0.10/NFC2 | J24 | NFC (선택) |
| ~~P0.25~~ | ~~AC21~~ | ~~VCC_EN 사용~~ |
| P0.26 | G1 | 예비 GPIO |
| P0.27 | H2 | 예비 GPIO |
| P0.30/AIN6 | B9 | 예비 ADC |
| P0.31/AIN7 | A8 | 예비 ADC (RF 전원 L2 연결용) |
| P1.00~P1.15 | - | 예비 GPIO (16개) |

---

## 전원 및 접지

| 신호 | 연결 | 비고 |
|------|------|------|
| VDD_nRF | 3.3V (칩 내부 레귤 출력) | 모든 주변 장치 전원 |
| VSS/GND | GND | 공통 접지 |

**주의:**
- E-Paper, LED, 버튼 등은 모두 **VDD_nRF(3.3V)** 기준으로 동작
- 배터리 측정은 **배터리 전압(3.7V~4.2V)**을 분압하여 측정
- ADC 참조 전압은 VDD_nRF 사용

---

## 핀 배치 다이어그램 (요약)

```
nRF52840-QIAA (aQFN73)

[E-Paper SPI]
P0.11 (T2)   ──► EPD_SCK
P0.12 (U1)   ──► EPD_MOSI
P0.13 (AD8)  ──► EPD_CS
P0.14 (AC9)  ──► EPD_DC
P0.15 (AD10) ──► EPD_RST
P0.16 (AC11) ◄── EPD_BUSY (입력)

[RGB LED]
P0.17 (AD12) ──► LED_R
P0.19 (AC15) ──► LED_G
P0.20 (AD16) ──► LED_B

[온도 센서]
P0.28/AIN4 (B11) ◄── TEMP_ADC

[배터리 측정]
P0.29/AIN5 (A10) ◄── BAT_ADC
P0.21 (AC17) ──► BAT_MOSFET (GND 측)

[전원 제어]
P0.25 (AC21) ──► VCC_EN (외부 장치 3.3V 제어)

[버튼]
P0.22 (AD18) ◄── BTN_USER1
P0.23 (AC19) ◄── BTN_USER2
P0.24 (AD20) ◄── BTN_POWER (Wake-up)
```

---

## 펌웨어 설정 예시 (Zephyr/nRF Connect SDK)

```c
// E-Paper SPI
#define EPD_SCK_PIN  DT_GPIO_PIN(DT_NODELABEL(gpio0), 11)
#define EPD_MOSI_PIN DT_GPIO_PIN(DT_NODELABEL(gpio0), 12)
#define EPD_CS_PIN   DT_GPIO_PIN(DT_NODELABEL(gpio0), 13)
#define EPD_DC_PIN   DT_GPIO_PIN(DT_NODELABEL(gpio0), 14)
#define EPD_RST_PIN  DT_GPIO_PIN(DT_NODELABEL(gpio0), 15)
#define EPD_BUSY_PIN DT_GPIO_PIN(DT_NODELABEL(gpio0), 16)

// RGB LED
#define LED_R_PIN    DT_GPIO_PIN(DT_NODELABEL(gpio0), 17)
#define LED_G_PIN    DT_GPIO_PIN(DT_NODELABEL(gpio0), 19)
#define LED_B_PIN    DT_GPIO_PIN(DT_NODELABEL(gpio0), 20)

// 온도 센서 (ADC)
#define TEMP_ADC_CH  SAADC_CH_PSELP_PSELP_AnalogInput4  // P0.28/AIN4

// 배터리 측정
#define BAT_ADC_CH   SAADC_CH_PSELP_PSELP_AnalogInput5  // P0.29/AIN5
#define BAT_MOSFET_PIN DT_GPIO_PIN(DT_NODELABEL(gpio0), 21)  // GND 측 MOSFET

// 외부 장치 전원 제어
#define VCC_EN_PIN   DT_GPIO_PIN(DT_NODELABEL(gpio0), 25)  // Load Switch 제어

// 버튼
#define BTN_USER1_PIN DT_GPIO_PIN(DT_NODELABEL(gpio0), 22)
#define BTN_USER2_PIN DT_GPIO_PIN(DT_NODELABEL(gpio0), 23)
#define BTN_POWER_PIN DT_GPIO_PIN(DT_NODELABEL(gpio0), 24)  // Wake-up
```

---

## 주의사항

1. **전원 순서:** VDD_nRF가 안정화된 후 주변 장치 초기화
2. **Deep Sleep:** BTN_POWER만 Wake-up 가능, 다른 GPIO는 Wake-up 불가
3. **ADC 정확도:** 배터리 측정 시 MOSFET ON 후 충분한 안정화 시간 필요 (최소 1~2ms)
4. **SPI 속도:** E-Paper는 느린 속도(1~4MHz) 권장
5. **전류 소비:** LED, E-Paper 갱신 시 전류 소비 증가 → 배터리 수명 고려
6. **초저전력 배터리 측정:**
   - Sleep 모드 진입 전: BAT_MOSFET = LOW, ADC 비활성화 필수
   - ADC 입력 Pull 비활성화 (누설 전류 방지)
   - 분압 저항은 1MΩ ~ 2MΩ 권장 (100kΩ 대비 전류 소비 1/10)
   - MOSFET은 낮은 누설 전류 제품 선택 (< 1nA)
7. **EPD 전원 관리:** Sleep 모드에서 EPD 전원 차단 권장 (전류 소비 100μA ~ 500μA 절감)
   - **⚠️ 중요:** EPD 전원 차단 시 GPIO 누설 전류 발생 가능!
   - 전원 차단 전: 모든 출력 핀 LOW, 입력 핀 Pull 비활성화 필수
   - 누설 전류: 최적화 전 ~355μA → 최적화 후 < 1μA
   - 상세 내용: `GPIO_LEAKAGE_CURRENT.md` 참조
8. **초저전력 설계 (5μA 이하 목표):**
   - 배터리 측정: GND 측 MOSFET (Low-side switch) 사용
   - 외부 장치 전원: N-MOS + P-MOS Load Switch로 완전 차단
   - Sleep 모드 진입 전: VCC_EN = LOW (외부 전원 차단)
   - 예상 Sleep 모드 전류: ~5μA
   - 상세 내용: `ULTRA_LOW_POWER_DESIGN.md` 참조

---

이 핀 배치는 SmartBox 2.0의 기본 구성입니다. 필요 시 예비 핀으로 확장 가능합니다.
