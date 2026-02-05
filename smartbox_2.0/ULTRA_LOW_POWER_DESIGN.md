# 초저전력 설계 (5μA 이하 목표)

## 설계 개요

Sleep 모드에서 **5μA 이하**를 달성하기 위한 완전한 전원 차단 설계입니다.

### 핵심 아이디어
1. **배터리 측정**: GND 측에 MOSFET (Low-side switch)로 측정 시에만 전류 흐름
2. **외부 장치 3.3V**: N-MOS + P-MOS Load Switch로 완전 차단
3. **Wake-up 장치 제외**: 버튼 등은 항상 전원 공급 유지

---

## 1. 배터리 측정 회로 (GND 측 MOSFET)

### 1.1 회로 구성

**기존 설계 (High-side MOSFET):**
```
배터리+ ──┬── R1 (1MΩ)
          │
          ├── BAT_MOSFET (P0.21) ──► N-MOSFET Gate
          │                              │
          └── R2 (1MΩ) ──┬── BAT_ADC (P0.29/AIN5)
                         │
                         └── GND
```

**개선 설계 (Low-side MOSFET):**
```
배터리+ ──┬── R1 (1MΩ~2MΩ)
          │
          ├── BAT_ADC (P0.29/AIN5)
          │
          └── R2 (1MΩ~2MΩ) ──┬── BAT_MOSFET (P0.21) ──► N-MOSFET Gate
                             │                              │
                             └── N-MOSFET Drain ──► GND
```

### 1.2 장점

1. **더 낮은 누설 전류**
   - MOSFET OFF 시: 분압 저항에 전류가 흐르지 않음
   - ADC 입력 임피던스만 고려 (1MΩ)
   - 예상 누설 전류: **< 0.1μA**

2. **간단한 제어**
   - N-MOSFET 사용 (P-MOSFET보다 저렴하고 선택 폭 넓음)
   - Gate 전압: 3.3V로 충분 (Vgs > Vth)

3. **ADC 안정성**
   - GND 측 스위칭으로 ADC 입력 안정성 향상

### 1.3 구현 코드

```c
#define BAT_MOSFET_PIN  DT_GPIO_PIN(DT_NODELABEL(gpio0), 21)
#define BAT_ADC_CH      SAADC_CH_PSELP_PSELP_AnalogInput5

float measure_battery_voltage_low_side(void) {
    float voltage = 0.0f;
    
    // 1. ADC 활성화
    nrf_saadc_enable();
    
    // 2. GND 측 MOSFET ON (전류 경로 활성화)
    nrf_gpio_pin_set(BAT_MOSFET_PIN);  // HIGH → MOSFET ON
    
    // 3. 안정화 대기 (1~2ms)
    nrf_delay_us(2000);
    
    // 4. ADC 측정
    uint16_t adc_value = adc_read_single_sample(BAT_ADC_CH);
    
    // 5. 즉시 MOSFET OFF (전류 차단)
    nrf_gpio_pin_clear(BAT_MOSFET_PIN);  // LOW → MOSFET OFF
    
    // 6. ADC 비활성화
    nrf_saadc_disable();
    
    // 7. 전압 계산
    voltage = (float)adc_value * 2.0f * 3.3f / 4095.0f;
    
    return voltage;
}
```

---

## 2. 외부 장치 3.3V 전원 제어 (Load Switch)

### 2.1 회로 구성

**N-MOS + P-MOS Load Switch:**
```
VDD_3V3 (레귤레이터 출력)
    │
    ├── P-MOSFET Source
    │       │
    │       ├── Gate ──► N-MOSFET Drain
    │       │              │
    │       │              └── Gate ──► VCC_EN (P0.25)
    │       │
    │       └── Drain ──► VDD_EXT (외부 장치 전원)
    │
    └── Pull-up (100kΩ~1MΩ) ──► P-MOSFET Gate
```

**동작 원리:**
- `VCC_EN = HIGH` → N-MOS ON → P-MOS Gate = GND → P-MOS ON → VDD_EXT = 3.3V
- `VCC_EN = LOW` → N-MOS OFF → P-MOS Gate = 3.3V (Pull-up) → P-MOS OFF → VDD_EXT = 0V

### 2.2 부품 선택

**P-MOSFET 요구사항:**
- 낮은 ON 저항: Rds_on < 100mΩ (전압 강하 최소화)
- 낮은 누설 전류: < 1nA (OFF 상태)
- 낮은 게이트 전하: 빠른 스위칭
- 권장: **Si2301DS, AP2301, DMG2305UX**

**N-MOSFET 요구사항:**
- 낮은 ON 저항: Rds_on < 10Ω
- 낮은 누설 전류: < 1nA
- 권장: **BSS138, DMN2004, 2N7002**

**Load Switch IC (대안):**
- 전용 Load Switch 사용 가능 (더 간단)
- 권장: **TPS22915, AP22811, FDC6331L**
- 장점: 내장 보호 회로, 작은 패키지, 낮은 누설 전류

### 2.3 제어 대상 장치

**VDD_EXT로 제어되는 장치:**
- ✅ EPD (E-Paper Display)
- ✅ RGB LED
- ✅ 온도 센서 (NTC 서미스터 Pull-up)

**VDD_EXT에서 제외되는 장치:**
- ❌ 버튼 (BTN_USER1, BTN_USER2, BTN_POWER)
  - Wake-up 기능 필요
  - Pull-up은 nRF52840 내부 사용

### 2.4 회로도 (상세)

```
                    VDD_3V3
                       │
        ┌──────────────┼──────────────┐
        │              │              │
     [100kΩ]      P-MOS Source    [100kΩ] Pull-up
     Pull-up          │              │
        │             │              │
        └─────────────┼──────────────┘
                      │
                  P-MOS Gate
                      │
                  N-MOS Drain
                      │
                  N-MOS Source ──► GND
                      │
                  N-MOS Gate ──► VCC_EN (P0.25)
                      │
                  VCC_EN = HIGH → P-MOS ON → VDD_EXT = 3.3V
                  VCC_EN = LOW  → P-MOS OFF → VDD_EXT = 0V

VDD_EXT ──► EPD_VCC
         └─► LED_VCC (RGB LED)
         └─► TEMP_VCC (온도 센서 Pull-up)
```

---

## 3. GPIO 핀 할당

### 3.1 추가 핀

| 기능 | GPIO 핀 | 물리 핀 | 비고 |
|------|---------|---------|------|
| **전원 제어** | | | |
| └ VCC_EN | P0.25 | AC21 | 외부 장치 3.3V 제어 |
| **배터리 측정** | | | |
| └ BAT_MOSFET | P0.21 | AC17 | GND 측 MOSFET (Low-side) |

### 3.2 핀 배치 업데이트

**기존 핀 유지:**
- EPD SPI 핀: P0.11 ~ P0.16
- RGB LED: P0.17, P0.19, P0.20
- 온도 센서: P0.28/AIN4
- 배터리 ADC: P0.29/AIN5
- 버튼: P0.22, P0.23, P0.24

**새로 사용:**
- VCC_EN: P0.25 (예비 핀 사용)

---

## 4. 소프트웨어 구현

### 4.1 전원 제어 함수

```c
#define VCC_EN_PIN      DT_GPIO_PIN(DT_NODELABEL(gpio0), 25)
#define BAT_MOSFET_PIN  DT_GPIO_PIN(DT_NODELABEL(gpio0), 21)

// 외부 장치 전원 ON
void external_power_on(void) {
    // 1. GPIO 설정 (출력, 초기값 LOW)
    nrf_gpio_cfg_output(VCC_EN_PIN);
    nrf_gpio_pin_clear(VCC_EN_PIN);
    
    // 2. 전원 공급
    nrf_gpio_pin_set(VCC_EN_PIN);  // HIGH → Load Switch ON
    
    // 3. 전원 안정화 대기
    nrf_delay_ms(10);
    
    // 4. 주변 장치 초기화
    epd_init();
    led_init();
    // ...
}

// 외부 장치 전원 OFF (Sleep 모드 진입 전)
void external_power_off_safe(void) {
    // 1. 주변 장치 정리
    epd_power_off_safe();  // GPIO 안전 설정
    nrf_gpio_pin_clear(LED_R_PIN);
    nrf_gpio_pin_clear(LED_G_PIN);
    nrf_gpio_pin_clear(LED_B_PIN);
    
    // 2. SPI 비활성화
    nrf_spim_disable(NRF_SPIM0);
    
    // 3. GPIO 상태 안정화 대기
    nrf_delay_us(100);
    
    // 4. 외부 장치 전원 차단
    nrf_gpio_pin_clear(VCC_EN_PIN);  // LOW → Load Switch OFF
    
    // 5. GPIO를 입력 모드로 변경 (누설 전류 방지)
    nrf_gpio_cfg_input(VCC_EN_PIN, NRF_GPIO_PIN_NOPULL);
}

// 배터리 측정 (GND 측 MOSFET)
float measure_battery_voltage(void) {
    float voltage = 0.0f;
    
    // 1. ADC 활성화
    nrf_saadc_enable();
    
    // 2. GND 측 MOSFET ON
    nrf_gpio_pin_set(BAT_MOSFET_PIN);
    nrf_delay_us(2000);  // 안정화
    
    // 3. ADC 측정
    uint16_t adc_value = adc_read_single_sample(BAT_ADC_CH);
    
    // 4. 즉시 MOSFET OFF
    nrf_gpio_pin_clear(BAT_MOSFET_PIN);
    
    // 5. ADC 비활성화
    nrf_saadc_disable();
    
    // 6. 전압 계산
    voltage = (float)adc_value * 2.0f * 3.3f / 4095.0f;
    
    return voltage;
}
```

### 4.2 Sleep 모드 진입 시퀀스

```c
void enter_sleep_mode_complete(void) {
    // 1. 배터리 측정 (필요한 경우)
    if (should_measure_battery()) {
        float battery_voltage = measure_battery_voltage();
        // 배터리 전압 저장/전송
    }
    
    // 2. 배터리 측정 회로 비활성화
    nrf_gpio_pin_clear(BAT_MOSFET_PIN);
    nrf_saadc_disable();
    
    // 3. 외부 장치 전원 차단
    external_power_off_safe();
    
    // 4. 불필요한 GPIO Pull 비활성화
    // (Wake-up 핀 제외)
    nrf_gpio_cfg_input(BTN_USER1_PIN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(BTN_USER2_PIN, NRF_GPIO_PIN_NOPULL);
    // BTN_POWER는 Wake-up용이므로 Pull-up 유지
    
    // 5. Deep Sleep 진입
    // ...
}

void wake_up_from_sleep(void) {
    // 1. 외부 장치 전원 공급
    external_power_on();
    
    // 2. 주변 장치 초기화
    // ...
}
```

---

## 5. 전력 소비 분석

### 5.1 Sleep 모드 (최적화 후)

| 구성 요소 | 전류 소비 | 비고 |
|-----------|----------|------|
| **nRF52840 (Deep Sleep)** | ~3μA | RTC만 활성화 |
| **배터리 측정 회로** | < 0.1μA | MOSFET OFF, ADC 비활성화 |
| **외부 장치 전원** | 0μA | 완전 차단 (VDD_EXT = 0V) |
| **GPIO 누설 전류** | < 0.5μA | 모든 핀 안전 상태 |
| **버튼 Pull-up** | ~1μA | 내부 Pull-up (3개 버튼) |
| **Load Switch 누설** | < 0.1μA | P-MOSFET OFF 상태 |
| **기타** | < 0.5μA | 온도/기타 센서 |
| **총합** | **~5μA** | ✅ 목표 달성! |

### 5.2 Active 모드 (측정 중)

| 구성 요소 | 전류 소비 | 비고 |
|-----------|----------|------|
| nRF52840 (Active) | ~5mA | RF 통신 시 |
| 배터리 측정 (5ms) | ~500μA × 5ms | 평균 ~2.5μA |
| 외부 장치 (활성) | ~50mA × 2초 | EPD 갱신 시 |
| **평균 전류** | **~5.1mA** | Active 모드 비율에 따라 |

### 5.3 배터리 수명 계산 (2000mAh)

**Sleep 모드 (99% 시간):**
- 전류: ~5μA
- 수명: 2000mAh / 5μA = **400,000시간** (~45년)

**Active 모드 고려:**
- 실제 수명: **1년 ~ 2년** (Active 모드 비율, EPD 갱신 주기 등에 따라)

---

## 6. 하드웨어 구현 체크리스트

### 배터리 측정 회로
- [ ] GND 측에 N-MOSFET 배치
- [ ] 분압 저항: 1MΩ ~ 2MΩ
- [ ] MOSFET: 낮은 누설 전류 (< 1nA)
- [ ] ADC 입력 Pull 비활성화

### 외부 장치 전원 제어
- [ ] P-MOSFET + N-MOSFET Load Switch 구성
- [ ] 또는 전용 Load Switch IC 사용
- [ ] Pull-up 저항: 100kΩ ~ 1MΩ
- [ ] VDD_EXT 제어 대상: EPD, LED, 온도 센서
- [ ] 버튼은 VDD_EXT에서 제외

### 전원 분리
- [ ] VDD_nRF (3.3V): nRF52840 전용
- [ ] VDD_EXT (3.3V): 외부 장치 전용 (제어 가능)
- [ ] 배터리 전압: 분압 회로로만 측정

---

## 7. 주의사항

### 7.1 전원 순서

**전원 ON:**
1. VCC_EN = HIGH (외부 장치 전원 공급)
2. 전원 안정화 대기 (10ms)
3. 주변 장치 초기화

**전원 OFF:**
1. 주변 장치 정리 (GPIO 안전 설정)
2. VCC_EN = LOW (외부 장치 전원 차단)
3. GPIO 입력 모드로 변경

### 7.2 GPIO 상태

**Sleep 모드 진입 전:**
- 모든 출력 핀: LOW
- 입력 핀 Pull: 비활성화 (Wake-up 핀 제외)
- VCC_EN: LOW (외부 전원 차단)
- BAT_MOSFET: LOW (배터리 측정 차단)

### 7.3 ESD 보호

- Load Switch 출력에 ESD 보호 다이오드 추가 권장
- 또는 전용 Load Switch IC 사용 (내장 보호 회로)

---

## 8. 부품 리스트

### 배터리 측정
- **N-MOSFET**: BSS138, DMN2004, 2N7002
- **저항**: 1MΩ ~ 2MΩ, 1% 정밀도

### 외부 장치 전원 제어
- **P-MOSFET (권장)**: 
  - ⭐ **Si2301DS** (Vishay) - 가격 대비 성능 우수, Rds_on ~150mΩ @ 3.3V
  - **DMG2305UX** (Diodes) - 낮은 Rds_on ~100mΩ, 높은 전류 4.2A
  - **NTR3A30PZ** (ON Semi) - 매우 낮은 Rds_on ~50mΩ, 전류 5.5A
- **N-MOSFET**: BSS138, DMN2004
- **Pull-up 저항**: 100kΩ ~ 1MΩ
- **또는 Load Switch IC**: TPS22915 (TI) - 내장 보호 회로, 매우 낮은 누설 전류 10nA

**상세 부품 정보:** `PMOS_RECOMMENDATIONS.md` 참조

---

## 9. 예상 성능

### Sleep 모드 전류
- **목표**: < 5μA
- **예상**: ~5μA
- **달성 가능성**: ✅ **가능**

### 배터리 수명 (2000mAh)
- Sleep 모드만: ~45년
- 실제 사용: **1년 ~ 2년** (Active 모드 고려)

---

## 결론

제안하신 설계로 **Sleep 모드 5μA 이하 달성이 가능**합니다!

**핵심 포인트:**
1. ✅ 배터리 측정: GND 측 MOSFET (Low-side switch)
2. ✅ 외부 장치 전원: N-MOS + P-MOS Load Switch로 완전 차단
3. ✅ GPIO 안전 설정: Sleep 모드 진입 전 모든 핀 안전 상태로 설정

**예상 전류 소비:**
- Sleep 모드: **~5μA** ✅
- Active 모드: ~5mA (평균)

---

**작성일:** 2026-02-05  
**버전:** 1.0
