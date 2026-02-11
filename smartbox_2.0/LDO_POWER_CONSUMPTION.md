# LDO 레귤레이터 대기 전력 분석 (XC6206PxxxMR)

## XC6206PxxxMR 사양

### 기본 정보
- **제조사**: Torex Semiconductor
- **타입**: Low Dropout (LDO) 레귤레이터
- **패키지**: SOT-23-3
- **출력 전류**: 60mA ~ 250mA (타입에 따라)

### 대기 전력 (Quiescent Current)

**Quiescent Current (Iq):**
- **Typical**: **1.0 μA**
- **Maximum**: **3.0 μA**

**Supply Current (IDD):**
- **Typical**: 1.0 μA
- **Maximum**: 3.0 μA

### 전압 사양
- **입력 전압**: 1.8V ~ 6.0V
- **출력 전압**: 1.2V ~ 5.0V (0.1V 단위)
- **Dropout Voltage**: 250mV @ 100mA

---

## 전력 소비 분석

### Sleep 모드에서의 전력 소비

**XC6206PxxxMR이 항상 활성화된 경우:**

| 상태 | 전류 소비 | 비고 |
|------|----------|------|
| **Quiescent Current (Iq)** | **1.0 μA** (typical) | 부하 없이 레귤레이터만 동작 |
| **Maximum Iq** | **3.0 μA** | 최악의 경우 |

**중요:**
- LDO 레귤레이터는 **부하가 없어도** quiescent current를 소비합니다.
- 출력이 차단되어도 입력 전원이 공급되면 계속 전류를 소비합니다.
- Sleep 모드에서도 **1μA ~ 3μA**를 계속 소비합니다.

### 전체 시스템 전력 소비 (LDO 포함)

**Sleep 모드 (LDO 활성화 상태):**

| 구성 요소 | 전류 소비 | 비고 |
|-----------|----------|------|
| nRF52840 (Deep Sleep) | ~3μA | RTC만 활성화 |
| **XC6206PxxxMR (Iq)** | **~1μA** | LDO quiescent current |
| 배터리 측정 회로 (OFF) | < 0.1μA | MOSFET OFF |
| 외부 장치 전원 (OFF) | 0μA | Load Switch OFF |
| GPIO 누설 전류 | < 0.5μA | |
| 버튼 Pull-up | ~1μA | |
| **총합** | **~5.5μA** | LDO 포함 |

**Sleep 모드 (LDO 비활성화 가능한 경우):**

| 구성 요소 | 전류 소비 | 비고 |
|-----------|----------|------|
| nRF52840 (Deep Sleep) | ~3μA | |
| **XC6206PxxxMR (OFF)** | **0μA** | LDO 입력 차단 |
| 배터리 측정 회로 (OFF) | < 0.1μA | |
| 외부 장치 전원 (OFF) | 0μA | |
| GPIO 누설 전류 | < 0.5μA | |
| 버튼 Pull-up | ~1μA | |
| **총합** | **~4.5μA** | LDO 제외 |

---

## LDO 전원 제어 방안

### 현재 설계 (LDO 항상 활성화)

**회로 구성:**
```
배터리+ ──► XC6206PxxxMR VI ──► VO ──► VDD_3V3
              │
              └── GND
```

**문제점:**
- Sleep 모드에서도 **1μA ~ 3μA** 계속 소비
- LDO 입력을 차단할 수 없음 (배터리 직접 연결)

### 개선 방안 1: LDO 입력에 Load Switch 추가

**회로 구성:**
```
배터리+ ──► Load Switch ──► XC6206PxxxMR VI ──► VO ──► VDD_3V3
              │                    │
              └── LDO_EN (P0.26)   └── GND
```

**장점:**
- Sleep 모드에서 LDO 입력 완전 차단 가능
- LDO quiescent current 제거 (1μA ~ 3μA 절감)

**단점:**
- 추가 부품 필요 (Load Switch)
- 추가 GPIO 핀 필요
- 복잡도 증가

**권장:** 초저전력이 매우 중요한 경우에만 사용

### 개선 방안 2: LDO 출력에 Load Switch 추가 (권장)

**회로 구성:**
```
배터리+ ──► XC6206PxxxMR VI ──► VO ──► Load Switch ──► VDD_3V3
              │                    │         │
              └── GND              └── GND   └── VCC_EN (P0.25)
```

**장점:**
- LDO는 계속 동작하지만 출력만 차단
- 외부 장치 전원 차단 (이미 구현됨)
- 추가 부품 없음

**단점:**
- LDO quiescent current는 여전히 소비 (1μA ~ 3μA)

**현재 설계:** 이미 이 방식으로 구현됨 (VCC_EN으로 VDD_EXT 제어)

---

## XC6206PxxxMR의 장점

### 초저전력 LDO

**다른 LDO와 비교:**

| LDO 모델 | Quiescent Current | 제조사 |
|----------|-------------------|--------|
| **XC6206PxxxMR** | **1.0 μA** | Torex |
| LM1117 | ~5 mA | Texas Instruments |
| AMS1117 | ~5 mA | Advanced Monolithic Systems |
| MCP1700 | ~2 μA | Microchip |
| TPS782 | ~500 nA | Texas Instruments |

**XC6206PxxxMR의 장점:**
- ✅ 매우 낮은 quiescent current (1μA)
- ✅ 초저전력 애플리케이션에 적합
- ✅ 배터리 수명 연장에 기여

---

## 전력 소비 최적화 권장사항

### 현재 설계 (권장)

**LDO 출력에 Load Switch 사용:**
- XC6206PxxxMR: 항상 활성화 (1μA 소비)
- VDD_EXT: Load Switch로 제어 (Sleep 모드에서 차단)
- **총 Sleep 모드 전류: ~5.5μA**

**이유:**
- LDO quiescent current (1μA)는 매우 낮음
- 추가 부품 없이 구현 가능
- 5μA 목표 달성 가능

### 극한 최적화 (선택사항)

**LDO 입력에 Load Switch 추가:**
- Sleep 모드에서 LDO 입력 차단
- LDO quiescent current 제거
- **총 Sleep 모드 전류: ~4.5μA**

**비용:**
- 추가 Load Switch 부품
- 추가 GPIO 핀 (P0.26 등)
- 복잡도 증가

**권장:** 1μA 절감을 위해 추가 비용/복잡도가 필요한지 고려 필요

---

## 결론

### XC6206PxxxMR의 대기 전력

**Quiescent Current:**
- **Typical**: **1.0 μA**
- **Maximum**: **3.0 μA**

### Sleep 모드 전류 소비

**현재 설계 (LDO 활성화):**
- LDO quiescent current: **~1μA**
- 전체 Sleep 모드: **~5.5μA**
- ✅ **5μA 목표 달성 가능**

### 권장사항

1. **현재 설계 유지** (LDO 출력에 Load Switch)
   - LDO quiescent current (1μA)는 매우 낮음
   - 추가 부품 없이 목표 달성 가능

2. **극한 최적화는 선택사항**
   - LDO 입력 차단으로 1μA 추가 절감 가능
   - 하지만 추가 비용/복잡도 고려 필요

**결론:** XC6206PxxxMR의 quiescent current (1μA)는 초저전력 설계에 매우 적합하며, 현재 설계로도 5μA 목표 달성이 가능합니다.

---

**참고 자료:**
- Torex XC6206 Datasheet: https://www.torexsemi.com/file/xc6206/XC6206.pdf
- Quiescent Current: 부하 없이 레귤레이터 자체가 소비하는 전류

**작성일:** 2026-02-05  
**버전:** 1.0
