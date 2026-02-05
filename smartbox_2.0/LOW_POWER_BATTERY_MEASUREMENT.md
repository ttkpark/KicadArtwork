# 초저전력 배터리 측정 설계 가이드

## 개요

Sleep 모드에서 nRF52840 칩 자체는 전기를 거의 소비하지 않지만, 외부 칩(EPD 등)이 전력을 많이 소비할 수 있습니다. 배터리 측정 회로의 전력 소비를 최소화하기 위한 설계 방안을 제시합니다.

---

## 1. 하드웨어 설계 개선

### 1.1 분압 저항 값 증가

**현재 설계:**
- R1 = 100kΩ
- R2 = 100kΩ
- 전류 소비: ~20μA (4.2V / 200kΩ)

**개선 방안:**
- R1 = 1MΩ ~ 2MΩ
- R2 = 1MΩ ~ 2MΩ
- 전류 소비: ~2μA ~ 1μA (4.2V / 2MΩ ~ 4MΩ)

**주의사항:**
- 저항 값이 너무 크면 ADC 입력 임피던스와의 매칭 문제 발생 가능
- nRF52840 ADC 입력 임피던스: ~1MΩ (내부)
- 권장: **1MΩ ~ 2MΩ** (전류 소비와 정확도 균형)

### 1.2 MOSFET 선택 및 회로 개선

**MOSFET 요구사항:**
- **낮은 누설 전류 (Leakage Current)**: OFF 상태에서 < 1nA
- **낮은 ON 저항 (Rds_on)**: < 100Ω
- **낮은 게이트 전하 (Qg)**: 빠른 스위칭
- **소신호 MOSFET 권장**: 예) 2N7002, BSS138, DMN2004

**회로 구성 개선:**
```
배터리+ ──┬── R1 (1MΩ~2MΩ)
          │
          ├── BAT_MOSFET (P0.21) ──► N-MOSFET Gate
          │                              │
          │                              ▼
          └── R2 (1MΩ~2MΩ) ──┬── BAT_ADC (P0.29/AIN5)
                             │
                             └── GND
```

**추가 개선 (선택사항):**
- ADC 입력에 **보호용 저항 (10kΩ~100kΩ)** 직렬 연결
- ADC 입력에 **ESD 보호 다이오드** 추가 (선택)

### 1.3 ADC 입력 Pull 설정

**중요:** ADC 입력 핀에 **Pull-up/Pull-down을 비활성화**해야 합니다.
- Pull이 활성화되면 누설 전류 발생 가능
- nRF52840 GPIO 설정에서 `GPIO_PIN_CNF_PULL_Disabled` 사용

---

## 2. 소프트웨어 설계

### 2.1 측정 시퀀스 (최적화)

```c
// 배터리 측정 함수 (초저전력)
float measure_battery_voltage(void) {
    float voltage = 0.0f;
    
    // 1. ADC 활성화 (필요한 경우에만)
    nrf_saadc_enable();
    
    // 2. MOSFET ON (분압 회로 활성화)
    nrf_gpio_pin_set(BAT_MOSFET_PIN);
    
    // 3. 안정화 대기 (최소화: 1~2ms)
    nrf_delay_us(2000);  // 2ms 대기
    
    // 4. ADC 측정 (빠른 샘플링)
    // - 샘플링 시간 최소화
    // - Oversampling 비활성화
    // - 단일 샘플 측정
    uint16_t adc_value = adc_read_single_sample(BAT_ADC_CH);
    
    // 5. 즉시 MOSFET OFF (전류 차단)
    nrf_gpio_pin_clear(BAT_MOSFET_PIN);
    
    // 6. ADC 비활성화 (전력 절약)
    nrf_saadc_disable();
    
    // 7. 전압 계산
    // 배터리 전압 = ADC 값 × 2 × VREF / ADC_MAX
    voltage = (float)adc_value * 2.0f * 3.3f / 4095.0f;  // 12-bit ADC 가정
    
    return voltage;
}
```

### 2.2 측정 주기 최적화

**권장 측정 주기:**
- **일반 모드**: 5분 ~ 10분마다 1회
- **Sleep 모드**: Wake-up 시에만 측정 (또는 30분~1시간마다)
- **배터리 부족 감지 시**: 더 자주 측정 (예: 1분마다)

**구현 예시:**
```c
#define BATTERY_MEASURE_INTERVAL_NORMAL_MS  (5 * 60 * 1000)  // 5분
#define BATTERY_MEASURE_INTERVAL_SLEEP_MS   (30 * 60 * 1000) // 30분

static uint32_t last_battery_measure_time = 0;

bool should_measure_battery(void) {
    uint32_t now = k_uptime_get_32();
    uint32_t interval = is_sleep_mode() ? 
                        BATTERY_MEASURE_INTERVAL_SLEEP_MS : 
                        BATTERY_MEASURE_INTERVAL_NORMAL_MS;
    
    if ((now - last_battery_measure_time) >= interval) {
        last_battery_measure_time = now;
        return true;
    }
    return false;
}
```

### 2.3 Sleep 모드 진입 전 정리

**중요:** Sleep 모드 진입 전에 모든 주변 장치를 비활성화해야 합니다.

```c
void enter_sleep_mode(void) {
    // 1. 배터리 측정 회로 비활성화
    nrf_gpio_pin_clear(BAT_MOSFET_PIN);  // MOSFET OFF
    
    // 2. ADC 완전 비활성화
    nrf_saadc_disable();
    nrf_saadc_task_trigger(NRF_SAADC_TASK_STOP);
    
    // 3. EPD 전원 차단 (가능한 경우)
    // EPD_VCC_EN = LOW;  // EPD 전원 제어 핀이 있다면
    
    // 4. LED 모두 OFF
    nrf_gpio_pin_clear(LED_R_PIN);
    nrf_gpio_pin_clear(LED_G_PIN);
    nrf_gpio_pin_clear(LED_B_PIN);
    
    // 5. SPI 비활성화 (EPD와의 통신)
    nrf_spim_disable(NRF_SPIM0);
    
    // 6. 불필요한 GPIO Pull 비활성화
    // (Wake-up 핀 제외)
    
    // 7. Deep Sleep 진입
    // ...
}
```

---

## 3. 전력 소비 분석

### 3.1 배터리 측정 회로 전류 소비

| 상태 | 전류 소비 | 비고 |
|------|----------|------|
| **MOSFET OFF** | < 1μA | 분압 저항 누설 전류 (1MΩ 기준) |
| **MOSFET ON (측정 중)** | ~2μA | 분압 저항 전류 (2MΩ 기준) |
| **ADC 활성화** | ~200μA ~ 500μA | nRF52840 SAADC 활성화 시 |

**최적화 후 예상 전류:**
- Sleep 모드 (MOSFET OFF): **< 1μA**
- 측정 중 (MOSFET ON + ADC): **~500μA** (짧은 시간만)
- 측정 시간: **~5ms** (MOSFET ON 시간)

### 3.2 전체 시스템 전류 소비 (Sleep 모드)

| 구성 요소 | 전류 소비 | 비고 |
|-----------|----------|------|
| nRF52840 (Deep Sleep) | ~2μA ~ 5μA | RTC만 활성화 |
| 배터리 측정 회로 (OFF) | < 1μA | MOSFET OFF |
| **EPD (전원 ON)** | **~100μA ~ 500μA** | ⚠️ 주요 전력 소비원 |
| LED (OFF) | < 1μA | GPIO Low |
| 온도 센서 (NTC) | < 1μA | Pull-up만 활성화 |

**⚠️ 주의:** EPD가 Sleep 모드에서도 전원이 공급되면 전류를 많이 소비할 수 있습니다.

---

## 4. EPD 전원 관리 (추가 권장사항)

### 4.1 EPD 전원 제어 회로 추가

**권장 설계:**
- EPD 전원을 MOSFET 또는 Load Switch로 제어
- Sleep 모드 진입 시 EPD 전원 완전 차단
- Wake-up 시에만 EPD 전원 공급

**회로 예시:**
```
VDD_3V3 ──┬── EPD_VCC_EN (GPIO) ──► P-MOSFET Gate
          │                              │
          └── P-MOSFET Source ──► EPD_VCC (EPD 전원)
```

**GPIO 핀 할당:**
- 예비 핀 사용: P0.25 또는 P0.26
- 또는 기존 핀 재활용 (EPD_RST 등)

### 4.2 EPD 전원 제어 코드 (GPIO 누설 전류 방지 포함)

**⚠️ 중요:** EPD 전원을 차단해도 GPIO 핀에서 누설 전류가 발생할 수 있습니다!
- 출력 핀 HIGH 상태: 1~10μA (핀당)
- 입력 핀 Pull-up 활성화: ~330μA (내부 Pull-up 기준)
- **총 누설 전류: ~355μA** (최적화 전)

**해결책:** 전원 차단 전 모든 GPIO를 안전한 상태로 설정해야 합니다.

```c
#define EPD_VCC_EN_PIN  DT_GPIO_PIN(DT_NODELABEL(gpio0), 25)

void epd_power_on(void) {
    // 1. EPD 전원 공급
    nrf_gpio_pin_set(EPD_VCC_EN_PIN);
    
    // 2. 전원 안정화 대기
    nrf_delay_ms(10);
    
    // 3. GPIO 설정 복원
    nrf_gpio_cfg_input(EPD_BUSY_PIN, NRF_GPIO_PIN_PULLUP);
    // 출력 핀은 기본값 유지 (필요 시 설정)
}

void epd_power_off_safe(void) {
    // 1. 모든 출력 핀을 LOW로 설정 (누설 전류 방지)
    nrf_gpio_pin_clear(EPD_SCK_PIN);
    nrf_gpio_pin_clear(EPD_MOSI_PIN);
    nrf_gpio_pin_clear(EPD_CS_PIN);
    nrf_gpio_pin_clear(EPD_DC_PIN);
    nrf_gpio_pin_clear(EPD_RST_PIN);
    
    // 2. 입력 핀 Pull 비활성화 (누설 전류 방지)
    nrf_gpio_cfg_input(EPD_BUSY_PIN, NRF_GPIO_PIN_NOPULL);
    
    // 3. SPI 비활성화
    nrf_spim_disable(NRF_SPIM0);
    
    // 4. GPIO 상태 안정화 대기
    nrf_delay_us(100);
    
    // 5. EPD 전원 차단
    nrf_gpio_pin_clear(EPD_VCC_EN_PIN);
}

// 이전 함수 (호환성 유지)
void epd_power_off(void) {
    epd_power_off_safe();  // 안전한 버전 사용
}
```

**상세 내용:** `GPIO_LEAKAGE_CURRENT.md` 참조

---

## 5. 구현 체크리스트

### 하드웨어
- [ ] 분압 저항 값을 1MΩ ~ 2MΩ으로 변경
- [ ] 낮은 누설 전류 MOSFET 선택 (예: BSS138, DMN2004)
- [ ] ADC 입력 Pull 비활성화 확인
- [ ] EPD 전원 제어 회로 추가 (권장)

### 소프트웨어
- [ ] 배터리 측정 시 MOSFET ON/OFF 제어 구현
- [ ] 측정 후 즉시 ADC 비활성화
- [ ] 측정 주기 최적화 (5분~10분)
- [ ] Sleep 모드 진입 전 모든 주변 장치 비활성화
- [ ] EPD 전원 제어 구현 (권장)

### 테스트
- [ ] Sleep 모드 전류 측정 (< 10μA 목표)
- [ ] 배터리 측정 정확도 확인 (±50mV 이내)
- [ ] 배터리 수명 시뮬레이션 (목표: 1년 이상)

---

## 6. 참고 자료

### nRF52840 ADC 사양
- ADC 해상도: 8/10/12-bit 선택 가능
- 입력 임피던스: ~1MΩ
- 샘플링 시간: 3μs ~ 40μs (설정 가능)
- 활성화 시 전류: ~200μA ~ 500μA

### 권장 부품
- **MOSFET**: BSS138, DMN2004, 2N7002
- **저항**: 1MΩ ~ 2MΩ, 1% 정밀도 권장
- **Load Switch** (EPD 전원용): TPS22915, AP22811

---

## 7. 예상 전력 소비 (최적화 후)

### Sleep 모드 (99% 시간)
- nRF52840: ~3μA
- 배터리 측정 회로: < 1μA
- EPD (전원 OFF): 0μA
- **총합: ~4μA**

### Active 모드 (1% 시간)
- nRF52840: ~5mA (RF 통신 시)
- 배터리 측정: ~500μA × 5ms = 2.5μA 평균
- EPD (갱신 시): ~50mA × 2초 = 100μA 평균
- **총합: ~5.1mA 평균**

### 배터리 수명 계산 (2000mAh 배터리 기준)
- Sleep 모드: 2000mAh / 4μA = **500,000시간** (~57년)
- Active 모드 고려: 실제 수명은 **1년 ~ 2년** (Active 모드 비율에 따라)

---

**작성일:** 2026-02-05  
**버전:** 1.0
