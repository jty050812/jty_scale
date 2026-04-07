# S5PV210 电子秤裸机项目报告（可复现）

本项目在 S5PV210 板卡上实现电子秤功能：从 HX711 读取重量信号，经过滤波和标定后输出克重，并在 LCD 和串口显示状态。文档目标是让第一次接触该工程的读者，也能按步骤完成编译、下载、运行、观察结果和排障。

## 0. 第一次上手快速复现

### 0.1 你将完成什么
- 编译出 `scale.elf`、`scale.bin`、`scale.map`。
- 把 `scale.bin` 下载到开发板（地址 `0x30000000`）。
- 在串口看到启动日志，按键执行去皮和标定。

### 0.2 前置条件检查清单
- 已安装 `arm-none-eabi` 交叉编译工具链（交叉编译是“在 PC 上给 ARM 目标机编译程序”）。
- 已有 S5PV210 开发板、电源、串口线、下载工具（如 DNW 或等效方案）。
- 当前目录位于工程根目录 `template-serial-shell`。

### 0.3 步骤与检查点

1. 编译工程

```bash
make.exe -f Makefile
```

检查点：终端出现 `[LD] Linking output/scale.elf` 与 `[OC] Objcopying output/scale.bin`。

2. 检查产物

```powershell
Get-ChildItem output | Select-Object Name,Length,LastWriteTime
```

检查点：至少看到 `scale.elf`、`scale.bin`、`scale.map` 三个文件。

3. 下载到板卡并启动
- 下载地址：`0x30000000`
- 文件：`output/scale.bin`

检查点：串口输出启动横幅（见第 9 章日志证据）。

4. 功能验证
- 按 `POWER`（去皮）后，串口出现 `[TARE] Zero point set.`。
- 连按两次 `MENU` 进入标定，输入已知砝码克数后出现 `[CAL] Reference: ... factor: ...`。

### 0.4 失败时立刻怎么做
- 编译失败：先执行 `make clean` 再重新编译，确认工具链命令可用。
- 无串口日志：检查串口波特率 115200、下载地址和供电。
- 显示不更新：优先检查 HX711 接线（DOUT/SCK）和第 10 章排障 10.1。

## 0.5 首次术语解释（先看这个再读正文）
- 裸机（bare-metal）：程序不依赖操作系统，直接操作硬件寄存器；这决定了本项目所有初始化都在 `main` 前后手动完成。
- 链接脚本（linker script）：告诉链接器把代码/数据放到什么内存地址；本项目用 `link.ld` 设定运行基址 `0x30000000`。
- `jiffies`：软件时钟计数器（tick 计数），用来做“任务是否到期”的判断。
- 轮询（polling）：程序反复检查硬件状态而不是等中断触发；本项目采样和按键都走轮询。
- 去皮（tare）：把当前重量当作零点，后续重量都相对这个零点计算。
- 标定（calibration）：用已知砝码反推比例因子 `counts/gram`，把 ADC 原始值转换为克重。
- 环形缓冲（ring buffer）：固定大小数组循环覆盖旧数据，适合实时采样窗口。
- 指数平滑（exponential smoothing）：对新旧值按权重混合，降低抖动。

## 1. 项目目标与范围

### 1.1 目标
- 在无操作系统条件下完成可交互电子秤。
- 支持去皮、串口输入参考砝码完成标定。
- 提供故障可观测能力（串口日志 + 屏幕状态提示）。

### 1.2 约束
- 平台：S5PV210（ARM Cortex-A8）
- 传感器：HX711（24-bit ADC）
- 运行方式：裸机轮询调度，基于 `jiffies`
- 工具链：`arm-none-eabi-`

### 1.3 非目标
- 不实现持久化存储（重启后标定因子恢复默认值）。
- 不依赖 RTOS（实时操作系统），不做动态任务调度。

## 2. 工程与平台事实

### 2.1 关键目录

```text
template-serial-shell/
├── Makefile
├── link.ld
├── include/
│   ├── scale.h
│   └── hardware/
├── source/
│   ├── main.c
│   ├── tester-scale.c
│   ├── scale.c
│   └── hardware/
│       ├── hx711.c
│       └── s5pv210-fb.c
└── output/
```

### 2.2 链接与装载
- `link.ld` 指定入口 `_start`。
- 运行内存基址：`0x30000000`，可用长度 `0x20000000`（512MB）。
- 段布局：`.text/.rodata`、`.data`、`.bss`、堆与多模式栈区。

### 2.3 硬件映射
- HX711：`DOUT -> GPB_2`，`SCK -> GPB_0`。
- 按键：`POWER` 用于去皮，`MENU` 用于标定流程。

## 3. 构建与部署（含验证证据）

### 3.1 标准构建命令

```bash
make.exe -f Makefile
```

或在 VS Code 运行任务 `build x210`。

### 3.2 本次构建验证记录（2026-04-07）

执行摘要：

```text
[CC] source/scale.c
[CC] source/tester-scale.c
[CC] source/hardware/hw-key.c
[LD] Linking output/scale.elf
[OC] Objcopying output/scale.bin
```

产物清单（实测）：

```text
output/scale.elf  884904 bytes
output/scale.bin  155920 bytes
output/scale.map  291679 bytes
```

### 3.3 部署检查点表

| 步骤 | 命令/动作 | 期望输出 | 通过条件 |
|---|---|---|---|
| 编译 | `make.exe -f Makefile` | 出现 `Linking` 与 `Objcopying` | `output/scale.bin` 已生成 |
| 下载 | 写入 `output/scale.bin` 到 `0x30000000` | 下载工具提示完成 | 设备成功启动 |
| 观察 | 打开串口 115200 | 启动横幅和按键提示 | 可看到 `Electronic Scale v1.0` |

## 4. 系统架构

### 4.1 分层结构
1. 启动与平台层
- `source/startup/*` 与 `link.ld` 完成入口和内存布局。

2. 硬件抽象层
- 时钟、中断、tick、串口、帧缓冲、按键、蜂鸣器。
- HX711 驱动通过 GPIO bit-banging（软件翻转 GPIO 模拟时序）读取数据。

3. 应用逻辑层
- `source/scale.c`：状态机、滤波、标定、克重计算、UI 渲染。
- `source/tester-scale.c`：主循环分频调度。

4. 表现层
- LCD 显示重量和告警。
- 串口打印启动、标定、故障日志。

### 4.2 启动路径

```text
main()
    -> do_system_initial()
    -> tester_scale()
            -> hx711_init()
            -> scale_state_bootstrap()
            -> while(1) 分频循环
```

## 5. 调度设计

系统使用基于 `jiffies` 的非阻塞分频轮询。这里“非阻塞”指一个任务不应长时间占用 CPU，避免拖慢其他任务。

### 5.1 周期配置
- 采样：50ms
- 按键扫描：10ms
- 渲染：80ms

周期通过 `ms_to_ticks()` 换算到 tick，保证在不同系统时钟下逻辑时间一致。

### 5.2 调度行为
1. 采样到期：`scale_update_raw()`，若无故障再 `scale_update_grams()`。
2. 按键到期：`get_key_event()` 后调用 `scale_handle_keydown()`。
3. 渲染到期：双缓冲交换后 `scale_render()` + `s5pv210_screen_flush()`。

## 6. 滤波与重量估算

### 6.1 HX711 读取机制
- 等待 `DOUT` 拉低，超时则本次读取失败。
- 连续 24 个时钟采样原始位流。
- 第 25 个脉冲选择 A 通道增益 128。
- 24 位值做符号扩展后写入 `s32_t raw`。

### 6.2 多级滤波策略
1. 环形窗口缓存：窗口 `SCALE_RAW_FILTER_SIZE = 8`。
2. 去极值均值：样本 >= 4 时去掉最小和最大值。
3. 指数平滑：

$$
raw_{new} = \frac{3 \cdot raw_{prev} + raw_{filtered}}{4}
$$

4. 稳定判定：样本跨度超过 `SCALE_STABLE_RAW_SPAN_MAX(1200)` 则 `unstable=1`。

### 6.3 克重换算与钳制

$$
grams = \frac{raw - tare\_raw}{counts\_per\_gram}
$$

- `-0.3g ~ 0.3g` 归零，抑制小抖动。
- 显示范围钳制为 `[-500g, 5000g]`，越界置 `overload`。

### 6.5 关键函数输入/输出与运作逻辑

下面这些函数覆盖了从启动、调度、采样、换算、交互、渲染到故障处理的核心功能。阅读时先看“输入/输出”，再看“运作逻辑”。

| 函数 | 输入（参数） | 输出（返回） | 副作用（会改什么） | 源码位置 |
|---|---|---|---|---|
| `main(int argc, char *argv[])` | 启动参数（本项目中未使用） | `int`，正常返回 0 | 调用系统初始化并进入业务循环 | `source/main.c:21` |
| `do_system_initial(void)` | 无 | `void` | 初始化时钟、中断、tick、串口、帧缓冲、按键、蜂鸣器 | `source/main.c:5` |
| `tester_scale(int argc, char *argv[])` | 启动参数（内部忽略） | `int`，理论返回 0（常驻循环） | 初始化 HX711、UI、状态；按周期执行采样/按键/渲染 | `source/tester-scale.c:37` |
| `ms_to_ticks(u32_t ms)` | 毫秒数 | `u32_t` tick 数 | 无硬件副作用，用于调度周期换算 | `source/tester-scale.c:8` |
| `scale_state_init(struct scale_state_t *state)` | 状态结构体指针 | `void` | 清零并初始化状态字段和滤波缓存 | `source/scale.c:178` |
| `scale_state_bootstrap(struct scale_state_t *state)` | 状态结构体指针 | `void` | 启动时读一次 HX711，设置初始零点和故障态 | `source/scale.c:206` |
| `hx711_init(void)` | 无 | `void` | 配置 HX711 GPIO 方向和上拉 | `source/hardware/hx711.c:48` |
| `hx711_read_raw(s32_t *raw, u32_t timeout_ms)` | 原始值输出指针；超时毫秒 | `bool_t`，1 成功/0 失败 | 读 GPIO、发时钟、写入 `*raw` | `source/hardware/hx711.c:63` |
| `scale_update_raw(struct scale_state_t *state)` | 状态结构体指针 | `bool_t`，1 成功/0 失败 | 更新样本窗口、平滑值、稳定标志、故障计数 | `source/scale.c:231` |
| `scale_update_grams(struct scale_state_t *state)` | 状态结构体指针 | `void` | 根据比例因子更新克重，处理归零和超量程 | `source/scale.c:434` |
| `scale_handle_keydown(struct scale_state_t *state, u32_t keydown)` | 状态；按键位图 | `bool_t`，1 已处理/0 未处理 | 去皮、标定确认、串口提示、蜂鸣器反馈 | `source/scale.c:360` |
| `scale_prompt_reference_grams(float *reference_grams)` | 参考重量输出指针 | `bool_t`，1 成功/0 失败 | 从串口读取并校验输入，写入参考重量 | `source/scale.c:323` |
| `scale_commit_calibration(struct scale_state_t *state, float reference_grams)` | 状态；参考砝码克数 | `bool_t`，1 成功/0 失败 | 更新 `counts_per_gram`、`cal_ready`，并输出日志 | `source/scale.c:274` |
| `scale_print_banner(void)` | 无 | `void` | 输出启动信息和参数范围到串口 | `source/scale.c:509` |
| `scale_render(struct surface_t *screen, struct scale_state_t *state)` | 屏幕缓冲；状态 | `void` | 把当前重量/故障状态绘制到 LCD | `source/scale.c:468` |

1. `main`
- 作用：程序入口，负责把“平台初始化”与“业务逻辑”连接起来。
- 运行逻辑：
    1. 调用 `do_system_initial()` 初始化时钟、串口、显示、按键等硬件。
    2. 调用 `tester_scale()` 进入电子秤主流程。

2. `tester_scale`
- 作用：主调度器，相当于系统的“心跳循环”。
- 运行逻辑：
    1. 初始化传感器比例因子、屏幕、状态、启动日志。
    2. 把 50/10/80ms 转换为 tick 周期。
    3. `while(1)` 循环中按到期时间分别执行采样、按键处理、渲染。

2.1 `do_system_initial`
- 作用：完成所有基础硬件初始化，是系统可运行的前提。
- 运行逻辑：依次初始化内存、时钟、中断、tick、串口、显示和输入输出外设。

2.2 `ms_to_ticks`
- 作用：把“人类可读的毫秒周期”转换成“系统时钟 tick”。
- 运行逻辑：读取系统频率 `hz`，按公式换算并保证最小返回值为 1。

3. `hx711_read_raw`
- 作用：从 HX711 读取 24 位原始 ADC 数据。
- 运行逻辑：
    1. 在 `timeout_ms` 时间内等待 DOUT 变低（表示数据就绪）。
    2. 连续采 24 位，再发 1 个脉冲选择 A 通道增益 128。
    3. 做符号扩展后写入 `*raw` 并返回成功；超时返回失败。

3.1 `hx711_init`
- 作用：配置 HX711 相关 GPIO，保证后续读取时序可用。
- 运行逻辑：设置 DOUT 为输入、SCK 为输出，并对 DOUT 打开上拉。

3.2 `scale_state_init`
- 作用：初始化秤状态结构，防止使用脏数据。
- 运行逻辑：重置 raw、tare、样本窗口、标志位和默认标定参数。

3.3 `scale_state_bootstrap`
- 作用：系统启动时建立“首个有效状态”。
- 运行逻辑：尝试读取一次 HX711，成功则设置初始零点，失败则进入故障态。

4. `scale_update_raw`
- 作用：把一次原始采样变成“更稳定”的 `state->raw`。
- 运行逻辑：
    1. 调用 `hx711_read_raw` 获取新值。
    2. 成功时更新环形缓冲，计算去极值均值并做指数平滑。
    3. 根据样本跨度更新 `unstable`。
    4. 失败时累加 `sensor_fail_count`，超阈值后标记 `sensor_fault`。

5. `scale_update_grams`
- 作用：把过滤后的原始值转换为克重显示值。
- 运行逻辑：
    1. 检查比例因子是否在合法范围。
    2. 计算 `grams = (raw - tare_raw) / counts_per_gram`。
    3. 小范围抖动归零，超出显示范围则钳制并标记 `overload`。

6. `scale_handle_keydown`
- 作用：处理按键事件（去皮/标定）。
- 运行逻辑：
    1. `POWER`：若非故障态，写入新零点 `tare_raw`，蜂鸣提示并清零显示。
    2. `MENU`：先做 1 秒双确认，再进入串口输入参考重量。
    3. 通过 `scale_commit_calibration` 完成参数更新。

6.1 `scale_prompt_reference_grams`
- 作用：从串口获取参考砝码重量并完成输入合法性检查。
- 运行逻辑：读取一行字符串，转换成数字，检查范围与格式，成功后写回输出参数。

7. `scale_commit_calibration`
- 作用：真正提交标定结果。
- 运行逻辑：
    1. 检查输入克数范围、样本数量、稳定性、正增量。
    2. 计算 `counts_per_gram = delta / reference_grams` 并检查范围。
    3. 成功则写回状态并调用 `hx711_set_scale`，失败则打印原因。

8. `scale_render`
- 作用：把内部状态翻译成“用户可见界面”。
- 运行逻辑：
    1. 先清屏并绘制标题。
    2. 故障态显示 `HX711 ERROR` 及检查提示。
    3. 正常态显示克重、标定状态、`UNSTABLE/OVERLOAD` 等状态文本。

8.1 `scale_print_banner`
- 作用：开机时输出系统版本、默认参数和接线提示，便于确认运行环境。
- 运行逻辑：通过多次 `serial_printf` 输出关键引导信息。

这些核心函数连起来的完整链路为：`main` -> `do_system_initial` -> `tester_scale` -> `ms_to_ticks` -> `hx711_init/hx711_read_raw` -> `scale_state_init/scale_state_bootstrap/scale_update_raw/scale_update_grams` -> `scale_handle_keydown/scale_prompt_reference_grams/scale_commit_calibration` -> `scale_print_banner/scale_render`。

## 7. 标定流程

### 7.1 交互设计
- `MENU` 双确认：第一次只提示，1 秒内再次按下才真正进入标定。
- 目的：防止误触把比例因子改坏。

### 7.2 标定输入与校验
1. 输入范围：`[1.0, 50000.0] g`
2. 样本量：`sample_count >= 4`
3. 稳定性：`unstable == 0`
4. 正增量：`delta = raw - tare_raw > 0`
5. 因子范围：`counts_per_gram` 在 `[10.0, 50000.0]`

通过后更新：`counts_per_gram`、`last_reference_grams`、`cal_ready`。

## 8. 故障处理策略

### 8.1 故障检测
- `hx711_read_raw()` 连续超时会累加 `sensor_fail_count`。
- 达到 `SCALE_SENSOR_FAIL_LIMIT(100)` 后置 `sensor_fault`。

### 8.2 故障表现
- LCD：`HX711 ERROR` / `NO VALID DATA`
- 串口：`[SCALE] HX711 read timeout or sensor disconnected.`

### 8.3 故障恢复
- 后续读取成功即清除故障并打印 `[SCALE] HX711 recovered.`。

### 8.4 保护策略
- 故障态禁止标定。
- 故障态忽略去皮命令。
- 无效比例因子置 `invalid_scale` 并归零显示。

## 9. 串口日志证据（关键路径）

```text
========================================
    Electronic Scale v1.0
========================================
Scale factor: <value> counts/gram
POWER:TARE
MENU:CALIBRATE (INPUT REFERENCE GRAMS IN SERIAL)
```

```text
[CAL] Press MENU again within 1s to confirm.
[CAL] Put known weight on scale, then type grams in serial.
CAL REF (g)> 500
[CAL] Reference: 500.00 g, factor: 280.50 counts/gram
```

```text
[SCALE] HX711 read timeout or sensor disconnected.
[SCALE] HX711 recovered.
[TARE] Zero point set.
```

## 9.5 代码证据索引（Claim -> Source）

| 结论 | 证据来源 | 验证方式 |
|---|---|---|
| 主循环采用分频轮询调度 | `source/tester-scale.c:37` | 代码阅读 |
| 采样/按键/渲染周期为 50/10/80ms | `source/tester-scale.c:4`, `source/tester-scale.c:5`, `source/tester-scale.c:6` | 常量核对 |
| 使用去极值均值 + 指数平滑 | `source/scale.c:71`, `source/scale.c:231` | 算法路径核对 |
| 标定需要双确认和串口输入 | `source/scale.c:360`, `source/scale.c:323`, `source/scale.c:274` | 交互流程核对 |
| HX711 连续失败触发故障标志 | `source/scale.c:231`, `include/scale.h:15` | 阈值核对 |
| HX711 通过 GPIO 时序读 24bit 数据 | `source/hardware/hx711.c:63` | 驱动实现核对 |
| 运行地址为 `0x30000000` | `link.ld:12` | 链接脚本核对 |
| 产物为 `.elf/.bin/.map` | `Makefile:121`, `output/` | 构建验证 |

## 10. 排障章节

### 10.1 现象：重量值不更新或乱跳
- 原因：HX711 超时、接线不稳、采样周期过短。
- 快速检查：
    1. `SCALE_SAMPLE_PERIOD_MS` 是否 50ms 附近。
    2. `SCALE_HX711_READ_TIMEOUT_MS` 是否合理。
    3. DOUT/SCK 线和供电是否牢靠。
- 立即修复：恢复默认周期后观察是否出现 recovered 日志。

### 10.2 现象：标定失败
- 原因：输入不合法、秤体不稳、砝码摆放偏心。
- 快速检查：输入范围、`UNSTABLE` 标记、`raw - tare_raw` 是否为正。
- 立即修复：等待稳定后重试，砝码放中心。

### 10.3 现象：按 MENU 误触进入标定
- 说明：设计上需要 1 秒内二次确认，首次按下只会提示。
- 检查：是否出现 `[CAL] Press MENU again within 1s to confirm.`。

### 10.4 现象：屏幕显示异常
- 原因：LCD 参数和面板不匹配。
- 检查：`source/hardware/s5pv210-fb.c` 的分辨率和时序参数。

### 10.5 现象：去皮无效
- 原因：故障态下去皮被保护逻辑忽略。
- 检查：串口是否提示 `[TARE] Ignored while HX711 is in fault state.`。

## 11. 设计取舍与改进建议

### 11.1 当前取舍
- 轮询而非中断：实现简单直观，但极限实时性较弱。
- 轻量滤波：资源占用低，但对复杂噪声适应有限。

### 11.2 可扩展方向
1. 标定参数持久化到 Flash（掉电保留）。
2. 多点标定与非线性补偿。
3. 日志分级（INFO/WARN/ERR）。
4. 增加自检页面（漂移、稳定时间、失败计数）。

## 12. 结论

该项目在 S5PV210 裸机场景下实现了从采样、滤波、标定到故障恢复的完整闭环。配合本文的快速上手、检查点和排障路径，读者可以独立完成工程复现并理解关键实现逻辑。

