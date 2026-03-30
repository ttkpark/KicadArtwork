# CPLD-STM32 UI Controller & FND Multiplexer

## 📌 프로젝트 개요
본 프로젝트는 Altera MAX 3000A(EPM3064) CPLD를 활용하여 메인 MCU(STM32F030)의 부하를 줄이기 위한 하드웨어 UI 컨트롤러를 구현한 설계입니다.  
버튼 입력의 디바운싱(Debouncing), 0~31 양방향 카운터, 그리고 4-Digit 7-Segment(FND)의 다이나믹 구동(Multiplexing)을 CPLD 하드웨어 로직으로 분리 처리합니다.

## 🛠️ 개발 환경 및 하드웨어 사양
- **Target Device:** Altera MAX 3000A 계열 (`EPM3064ALC44-10`)
- **Main MCU:** STM32F030
- **IDE:** Quartus II 13.0 SP1 Web Edition
- **Clock Source:** STM32에서 공급하는 8kHz 외부 클럭 (`IO33`)

## 🎯 주요 기능 (Core Features)
1. **하드웨어 디바운싱 (Debouncing)**  
   - 8kHz 클럭 기반으로 약 18.7ms 딜레이 필터 구현  
   - 버튼 채터링을 소프트웨어 개입 없이 하드웨어에서 차단

2. **5-bit 양방향 카운터 (0~31)**  
   - `BTN1` 입력 시 -1 감소 (0에서 31 순환)  
   - `BTN2`, `BTN4` 입력 시 +1 증가 (31에서 0 순환)

3. **경량화된 BCD 변환 로직**  
   - EPM3064 매크로셀(64) 한계를 고려해 나눗셈/Double Dabble 대신  
   - `if-else` 기반 최적화된 뺄셈 로직으로 10의 자리/1의 자리 분리

4. **FND 다이나믹 멀티플렉싱**  
   - 8kHz를 32분주하여 약 250Hz로 4자리 Common Anode 순차 구동

5. **실시간 MCU 상태 보고**  
   - 카운터 5비트 상태(0~31)를 전용 I/O 핀 5개로 STM32에 비동기 전달

## 🔌 핀맵 할당 (Pin Mapping)
| 기능 그룹 | 신호명 | CPLD 핀 번호 | 비고 |
| :--- | :--- | :--- | :--- |
| **Clock** | `clk_8khz` | 33 | STM32에서 공급받는 8kHz 클럭 |
| **Button Input** | `btn1` | 1 | -1 감소 (Active Low) |
|  | `btn2` | 2 | +1 증가 (Active Low) |
|  | `btn4` | 4 | +1 증가 (Active Low) |
| **MCU Output** | `mcu_28` ~ `mcu_37` | 28, 29, 31, 34, 37 | 카운터 5비트 상태 실시간 출력 |
| **FND Segment** | `seg_a` ~ `dp` | 5, 6, 8, 9, 11, 12, 14, 16 | 7-Segment 제어 (Active Low) |
| **FND Common** | `com1` ~ `com4` | 18, 19, 20, 21 | 자릿수 제어 (Active High) |

> ⚠️ **주의 (하드웨어 핀맵 제약사항)**  
> 3번 핀은 VCC와 물리적으로 결선되어 있어 버튼 입력으로 사용하지 않습니다.

## 🚨 트러블슈팅 및 개발 가이드 (Lessons Learned)
프로젝트 수정/업로드 시 아래 항목을 반드시 확인하세요.

### 1) 98% 프로그래밍 실패(Verification Failed)
- **원인:** Verify 단계에서 JTAG 핀(TDI, TMS 등)에 연결된 외부 소자(FND 등)가 노이즈를 유발
- **해결책:**
  - 프로그래밍 시 **FND 모듈을 일시적으로 탈거**한 상태에서 업로드
  - 임시 조치로 Programmer에서 `Verify` 체크 해제 후 다운로드

### 2) 미사용 핀 설정 (Unused Pins Configuration)
- **원인:** Quartus 기본값이 미사용 핀을 `As output driving ground`로 설정해 전원 충돌 가능
- **해결책:**
  - `Assignments -> Device -> Device and Pin Options -> Unused Pins`
  - 값을 **`As input tri-stated`** 로 변경 후 전체 컴파일

### 3) 디바이스 라이브러리 지원
- MAX 3000A는 최신 Quartus Prime에서 미지원
- **Quartus II 13.0 SP1 Web Edition** 사용
- Device Installer에서 `MAX 3000A` 디바이스 팩(`.qdz`) 추가 설치 필요

## 📁 저장소 구조(예시)
```text
SPLD_Test/
├─ CPLD/          # Quartus 프로젝트/소스
├─ firmware/      # STM32F030 펌웨어 (OLED 모니터)
└─ README.md      # 본 문서
```

## 🔗 관련 문서
- 펌웨어 배선/핀 설명: `firmware/docs/CONNECTIONS.md`
- 펌웨어 개발 문서: `firmware/docs/DEVELOPMENT.md`

## 🙌 회고
하드웨어 핀맵 충돌, JTAG 간섭, 매크로셀 용량 최적화를 모두 통과해 완성한 프로젝트입니다.  
임베디드 하드웨어/펌웨어 협업 개발의 실제 제약과 해법이 담긴 결과물로, 유지보수 시 본 README와 관련 문서를 함께 참고하는 것을 권장합니다.
