# Electronic Scale (Scale) - S5PV210 项目

基于 S5PV210 裸机环境的电子秤嵌入式项目，集成 HX711 传感器驱动、LCD 显示、按键交互、串口标定和完整的故障处理机制。

## 1. 项目概览

### 硬件平台
- **SoC**: S5PV210 (ARM Cortex-A8)
- **传感器**: HX711 24-bit ADC（称重）
- **显示**: LCD 屏幕 1024×600 @ 60Hz
- **输入**: 板载按键 + 串口
- **构建工具**: ARM GNU 工具链 (`arm-none-eabi-gcc`)

### 核心特性
- **非阻塞调度**: 基于 Tick 的分频轮询（采样/按键/渲染独立）
- **去皮 (TARE)**: 单键设置零点
- **实时标定**: 串口输入参考重量，自动计算 counts/gram
- **滤波算法**: 去极值平均 + 指数平滑（容错性强）
- **传感器保护**: 超时/超限/不稳定等多层故障保护
- **双确认**: MENU 需二次确认才进入标定，防止误触

## 2. 目录结构

```
.
├── README_CURRENT.md                 # 本文件
├── link.ld                           # ARM 链接脚本
├── Makefile                          # 构建配置
├── include/
│   ├── main.h
│   ├── scale.h                       # 电子秤配置和接口
│   ├── hardware/
│   │   └── ...（S5PV210 底层驱动头文件）
│   └── library/                      # C 标准库替代品
├── source/
│   ├── main.c                        # 系统初始化
│   ├── tester-scale.c                # 主循环和调度
│   ├── scale.c                       # 秤逻辑：采样、滤波、标定、渲染
│   ├── hardware/
│   │   ├── hx711.c                   # HX711 驱动
│   │   ├── s5pv210-fb.c              # LCD 驱动（已调优）
│   │   ├── hw-key.c                  # 按键驱动
│   │   └── ...（其他底层驱动）
│   └── ...（其他源文件）
└── output/
    ├── scale.elf                     # 最终 ELF 可执行文件
    └── scale.bin                     # 烧录用二进制
```

## 3. 硬件连接

### HX711 接线
```
HX711 DOUT (数据输出) → S5PV210 GPB_2
HX711 SCK  (时钟输入) → S5PV210 GPB_0
HX711 VCC                → 3.3V
HX711 GND                → GND
```

### 按键映射
- **POWER** (GPH0_1): 去皮 (TARE) - 设置当前重量为零点
- **MENU** (GPH2_3): 标定 - 需按两次确认（防误触）

## 4. 关键配置参数

### 在 `include/scale.h` 中

```c
#define SCALE_COUNTS_PER_GRAM           (430.0f)      // 默认克度（counts/gram）
#define SCALE_HX711_READ_TIMEOUT_MS     (100)         // HX711 读取超时时间 (ms)
#define SCALE_SENSOR_FAIL_LIMIT         (100)         // 触发故障前的失败次数
#define SCALE_RAW_FILTER_SIZE           (8)           // 滤波窗口大小
#define SCALE_STABLE_RAW_SPAN_MAX       (1200)        // 稳定性判定阈值（对应 ±1.4g 在 430counts/g）
#define SCALE_SAMPLE_PERIOD_MS          (50)          // 采样周期 (ms) - 已优化
#define SCALE_MAX_DISPLAY_GRAMS         (5000.0f)     // 显示范围上限
#define SCALE_MIN_DISPLAY_GRAMS         (-500.0f)     // 显示范围下限
```

### 采样周期的重要性 ⚠️
- **HX711 转换时间**: ~12.5ms @80SPS（每秒80样本）
- **采样周期**: 必须 **≥ 25ms**（给足转换时间）
- **当前设置**: 50ms（充分宽松，不会因超时导致故障）
- **历史坑**: 改为 20ms 会导致频繁超时 → sensor_fault → 数字卡死

## 5. 构建与烧录

### 5.1 编译

在项目根目录执行：
```bash
make.exe -f Makefile
```

或者在 VS Code 中运行任务：
- 快捷键: `Ctrl+Shift+B`
- 选择 `build x210` 任务

**输出文件**:
- `output/scale.elf` - 调试用 ELF 文件
- `output/scale.bin` - 烧录用二进制（约 160KB）

### 5.2 烧录到开发板

**方式1: 串口下载（DNW）**
```
加载地址: 0x30000000
二进制文件: output/scale.bin
波特率: 115200
```

**方式2: SD 卡启动**
使用 `tools/windows/SDcardBurner.exe` 或 `tools/windows/mkv210.exe`

## 6. 工作原理

### 6.1 主循环调度 (tester-scale.c)

```c
while(1) {
    now = jiffies;
    
    // 采样任务（50ms）
    if(time_after_eq(now, next_sample)) {
        scale_update_raw(&state);
        if(!state.sensor_fault)
            scale_update_grams(&state);
        next_sample += SAMPLE_PERIOD;
    }
    
    // 按键扫描（10ms）
    if(time_after_eq(now, next_key)) {
        scale_handle_keydown(&state, keydown);
        next_key += KEY_PERIOD;
    }
    
    // 屏幕刷新（80ms）
    if(time_after_eq(now, next_render)) {
        scale_render(screen, &state);
        s5pv210_screen_flush();
        next_render += RENDER_PERIOD;
    }
}
```

### 6.2 采样与滤波 (scale.c)

1. **原始读取**: 调用 `hx711_read_raw()` 获取 24-bit ADC 值
2. **窗口缓存**: 最新样本放入大小为 8 的环形缓冲
3. **去极值平均**: 剔除最大/最小值后取平均（鲁棒性强）
4. **指数平滑**: raw = (prev_raw × 3 + filtered) / 4（平滑尖峰）
5. **稳定检测**: 样本跨度 < 1200 则标记为"稳定"

### 6.3 故障处理

| 故障类型 | 触发条件 | 表现 | 恢复 |
|---------|---------|------|------|
| **HX711 超时** | 连续失败 > 100 次 | 屏幕显示"HX711 ERROR" | 恢复读取后自动清除 |
| **不稳定** | 样本跨度 > 1200 | 屏幕显示"UNSTABLE" | 数据稳定后消除 |
| **超量程** | grams > 5000 或 < -500 | 屏幕显示"OVERLOAD" | 重量回到范围内 |
| **标定错误** | 参考重量超出范围或克度异常 | 串口提示错误 | 重新标定 |

## 7. 串口交互

### 启动输出
```
## Starting application at 0x30000000 ...
========================================
  Electronic Scale v1.0
========================================
Scale factor: 430.00 counts/gram
POWER:TARE
MENU:CALIBRATE (INPUT REFERENCE GRAMS IN SERIAL)
...
========================================
```

### 标定流程

按 `MENU` 键两次（第一次会提示"Press MENU again within 1s to confirm"），然后在串口输入参考重量（单位克）：

```
[CAL] Put known weight on scale, then type grams in serial.
CAL REF (g)> 500
[CAL] Reference: 500.00 g, factor: 280.50 counts/gram
```

成功标定后屏幕会显示"CAL OK"。

### 故障日志

故障或异常时会输出：
```
[SCALE] HX711 read timeout or sensor disconnected.
[SCALE] HX711 recovered.
[TARE] Zero point set.
```

## 8. 故障排查

### 问题: 屏幕左半边显示模糊条纹

**原因**: LCD 面板参数错误（分辨率不匹配）  
**解决**: 检查 `s5pv210-fb.c` 中的 LCD 配置是否与实际硬件一致

### 问题: 重量显示卡在某个值不动

**原因**: 通常是采样周期太短导致 HX711 超时  
**检查清单**:
1. `SCALE_HX711_READ_TIMEOUT_MS` 是否 ≥ 50ms
2. `SCALE_SAMPLE_PERIOD_MS` 是否 ≥ 50ms（对应 HX711 ~12.5ms 转换时间）
3. HX711 的 DOUT/SCK 连线是否松动

### 问题: 按键失效

**原因**: GPIO 初始化错误  
**检查**: `hw-key.c` 中的位偏移是否正确

### 问题: 标定后克度数值不合理

**原因**: 参考重量输入错误或传感器受压不均  
**解决**: 
- 确认参考重量确实是输入的克数
- 称重时物体要放在秤盘中心
- 等待 UNSTABLE 标志消除后再标定

