---
name: tjc-serial-hmi
description: 为淘晶驰串口屏生成、审查和修复 MCU 串口通讯代码，重点覆盖 STM32/HAL/LL/裸机场景下的页面切换、控件赋值、字符串收发、触摸事件解析、返回帧解析和接线排障。用户提到淘晶驰、TJC、串口屏、Nextion 类似指令、page/get/click/vis/tsw/ref、0x70/0x71/0x65 返回帧、STM32 串口屏驱动、屏幕协议解析、中文编码、串口缓冲区或用单片机控制串口屏时使用。
---

# TJC Serial HMI

先把任务归类成下面三类，再执行：

1. 生成 MCU 到串口屏的发送代码。
2. 生成或修复串口屏到 MCU 的接收解析代码。
3. 审查现有工程里的淘晶驰驱动、接线假设和协议实现。

## 协议速查卡

- 所有发给串口屏的 ASCII 指令都要追加 `0xFF 0xFF 0xFF` 结束符。
- 所有从串口屏返回的标准帧也以 `0xFF 0xFF 0xFF` 结束。
- 不要用超时推断帧尾；优先基于环形缓冲区搜索三个连续 `0xFF`。
- 数值返回帧类型是 `0x71`，后接 4 字节小端整数。
- 字符串返回帧类型是 `0x70`，后接原始字节串。
- 当前页返回帧类型是 `0x66`。
- 控件触发返回帧类型是 `0x65`。
- `page` 指令后面的 HMI 语句不会继续执行；生成 HMI 侧逻辑时不要把关键语句放在 `page` 后。
- 中文字符串必须先确认工程编码是 `GB2312` 还是 `UTF-8`，再决定 MCU 发送的字节内容。

需要完整协议表时，读取：

- [references/protocol.md](references/protocol.md)
- [references/wiring.md](references/wiring.md)
- [references/pitfalls.md](references/pitfalls.md)
- [references/series.md](references/series.md)
- [references/system-variables.md](references/system-variables.md)
- [references/troubleshooting.md](references/troubleshooting.md)
- [references/widgets.md](references/widgets.md) — 控件速查：属性读写规则、txt/val 区别、各控件交互模式与易犯错误

## 工作流

### 场景一：生成发送代码

先确认这些信息：

- MCU 平台和串口库：`HAL`、`LL`、裸机或 RTOS 驱动。
- 串口屏使用的指令：如 `page`、`click`、`get`、`vis`、`tsw`、`ref`、`n0.val=...`、`t0.txt="..."`。
- 是否需要中文文本发送。

生成代码时强制遵守：

- 发送函数要么接受“纯命令文本”并在底层统一补 `0xFF 0xFF 0xFF`，要么调用点明确补尾；不要两边都补。
- 避免把字符串拼接散落在业务层，优先封装成 `tjc_send_cmd()`、`tjc_set_val()`、`tjc_set_txt()`、`tjc_get_val()` 这类 API。
- 对 `get` 的返回不要假设是字符串；根据目标属性决定期待 `0x70` 或 `0x71`。
- 长文本发送前先查 `references/series.md` 核对屏端缓冲区型号差异，并根据目标系列保守规划帧长，必要时拆帧发送。

常见发送模式：

```c
tjc_send_cmd("page main");
tjc_send_cmd("get n0.val");
tjc_send_cmd("click b0,1");
tjc_send_cmd("t0.txt=\"hello\"");
```

如果需要把命令文本转十六进制或带尾帧字节，运行：

```powershell
python scripts/gen_tjc_frame.py 'page main'
python scripts/gen_tjc_frame.py 't0.txt="温度:25"' --encoding utf-8 --hex
```

如果用户要直接产出 STM32 模板代码，优先复用：

- [assets/tjc_hal_driver.h](assets/tjc_hal_driver.h)
- [assets/tjc_hal_driver.c](assets/tjc_hal_driver.c)
- [assets/tjc_ringbuf.h](assets/tjc_ringbuf.h)
- [assets/tjc_ringbuf.c](assets/tjc_ringbuf.c)

使用这些模板时必须遵守：

- 只声称已支持模板里明确实现的返回帧类型。
- 对 `0x65`、`0x67`、`0x68` 这类帧结构，按照 `references/protocol.md` 中已确认的字段布局解析；对可能存在的未确认扩展字段，不要擅自补充。
- 系列相关行为先查 `references/series.md`，系统变量先查 `references/system-variables.md`。

### 场景二：生成接收解析代码

默认采用以下结构：

1. UART 中断或 DMA 回调只做“收字节入环形缓冲区”。
2. 主循环或独立任务中做“按帧尾取包”。
3. 解析器先看首字节类型码，再分发到不同回调。

最低要求：

- MCU 侧预留约 `1KB` 环形缓冲区。
- 使用中断接收优于轮询。
- 解析 `0x71` 时按小端拼成 `int32_t`。
- 解析 `0x70` 时保留原始字节，不要先入为主当作 ASCII。
- 对 `0x24` 串口缓冲区溢出做显式错误上报。

建议输出接口：

```c
void tjc_poll(void);
void tjc_on_numeric(void (*cb)(int32_t value));
void tjc_on_string(void (*cb)(const uint8_t *data, uint16_t len));
void tjc_on_page(void (*cb)(uint8_t page_id));
void tjc_on_click(void (*cb)(uint8_t page_id, uint8_t ctrl_id, uint8_t event));
```

### 场景三：审查已有代码

逐项检查：

1. 是否所有串口命令都追加了三个 `0xFF`。
2. 是否把板载 USB 转串口占用的 USART 误拿来连屏。
3. 是否在 `3.3V MCU <- 5V TX` 的组合里遗漏电平兼容处理。
4. 是否用固定长度或超时而不是帧尾解析返回帧。
5. 是否把 `0x70`、`0x71`、`0x65` 这些返回帧类型写错。
6. 是否在未知编码下直接发送中文字符串。
7. 是否错误地把 `prints` 当作 MCU 查询命令。

## 代码生成约束

- 默认优先生成可复用驱动层，不要只给零散 `printf`。
- 没有用户明确要求时，不要凭空发明协议字段、校验算法或转义规则。
- `0x65` 等字段宽度不确定时，要在代码注释中标出“依据当前文档假设为单字节字段，建议联机验证”。
- 文档没明确说明数据区中出现 `0xFF` 时的转义机制，不能自信声称存在某种转义规则。
- 涉及具体控件时，先查 `references/widgets.md`，确认该控件主要使用的是 `txt` 还是 `val`，以及哪些属性支持运行时修改。
- 生成控件赋值代码时，优先使用控件文档明确支持的绿色属性；黑色属性不要在运行时代码里硬改。
- 涉及 `bkcmd`、`bauds`、`dim`、`dims`、`dp`、`sleep`、`wup` 这类系统变量时，先查 `references/system-variables.md`。
- 涉及 X2/X3/X5/T0/T1/K0 差异时，先查 `references/series.md`，不要把 X 系列专属能力默认推广到全部系列。
- 如果用户只说“帮我写 STM32 控制淘晶驰串口屏代码”，优先提供：
  - 发送命令封装
  - 接收环形缓冲区
  - 基础返回帧解析
  - 若干常用 API

## 诊断清单

交付前自检：

- 能否解释接线：`TX->RX`、`RX->TX`、`GND 共地`。
- 能否指出目标串口是否独占。
- 每条命令是否只追加一次 `0xFF 0xFF 0xFF`。
- 数值返回是否按小端解析。
- 中文发送是否说明了编码前提。
- 目标控件的属性名是否用对，例如文本类优先看 `txt`，数值类优先看 `val`。
- 所引用的系列特性、系统变量和控件能力是否都能在参考文档中找到来源。
- 对未完全确认的协议细节是否留下注释或验证提示。
