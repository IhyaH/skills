# Personal Skills

这是我个人维护的 agent skills 仓库，用来沉淀可复用的工作流、工具脚本和任务委派说明。每个 skill 都放在 `skills/<skill-name>/` 下，并以 `SKILL.md` 作为入口文档。

## 目录结构

```text
.
|-- skills/
|   `-- tjc-serial-hmi/
|       |-- SKILL.md
|       |-- agents/
|       |   `-- openai.yaml
|       |-- assets/
|       |   |-- tjc_hal_driver.c
|       |   |-- tjc_hal_driver.h
|       |   |-- tjc_ringbuf.c
|       |   `-- tjc_ringbuf.h
|       |-- references/
|       |   |-- pitfalls.md
|       |   |-- protocol.md
|       |   |-- series.md
|       |   |-- system-variables.md
|       |   |-- troubleshooting.md
|       |   |-- widgets.md
|       |   `-- wiring.md
|       `-- scripts/
|           `-- gen_tjc_frame.py
|-- LICENSE
`-- README.md
```

## Skills

| Skill | 用途 | 入口 |
| --- | --- | --- |
| `tjc-serial-hmi` | 为淘晶驰串口屏生成、审查和修复 MCU 串口通讯代码，重点覆盖 STM32/HAL/LL/裸机场景下的页面切换、控件赋值、字符串收发、触摸事件解析、返回帧解析、接线排障，以及控件属性与系统变量的文档化约束。 | `skills/tjc-serial-hmi/SKILL.md` |

## 使用方式

### 作为个人 skills 集合使用

将仓库中的 `skills/` 目录同步或链接到你所使用的 agent skills 目录中。例如：

```powershell
git clone <repo-url> C:\Users\<you>\Documents\GitHub\skills
```

如果目标工具支持按目录加载 skills，可以直接指向本仓库的 `skills/` 目录；如果需要复制安装，则复制单个 skill 目录，例如 `skills/tjc-serial-hmi`。

### 使用 `tjc-serial-hmi`

该 skill 适合下面几类任务：

- 生成 STM32 / HAL / LL / 裸机环境下的淘晶驰串口屏发送代码
- 生成或审查 `0x65` / `0x66` / `0x67` / `0x68` / `0x70` / `0x71` 等返回帧解析代码
- 核对控件属性、系统变量、跨页面限制、编码约束和接线排障逻辑
- 基于官方离线文档约束 agent，避免编出超出文档支持范围的协议细节

skill 内已包含：

- 协议、接线、控件、系列差异、系统变量、排障等参考文档
- 一个命令转帧脚本：`skills/tjc-serial-hmi/scripts/gen_tjc_frame.py`
- 一套可直接复用的 C 模板驱动：`skills/tjc-serial-hmi/assets/`

示例：

```powershell
python skills/tjc-serial-hmi/scripts/gen_tjc_frame.py 'page main'
python skills/tjc-serial-hmi/scripts/gen_tjc_frame.py 't0.txt="温度:25"' --encoding utf-8 --hex
```

如果要让 agent 严格按该 skill 工作，优先把需求描述成：

- “按淘晶驰官方文档为 STM32 HAL 生成串口屏驱动代码”
- “检查这段 TJC 返回帧解析是否符合文档”
- “根据控件属性文档修正这段 `txt` / `val` / `vis` / `tsw` 代码”

## 维护约定

- 每个 skill 使用独立目录，目录名与 `SKILL.md` 中的 `name` 保持一致。
- 脚本、模板、示例等辅助文件放在对应 skill 目录下。
- 新增 skill 后更新本 README 的 Skills 表格和相关使用说明。
- 不在 skill 文档或脚本中提交密钥、令牌、私钥或 `.env` 内容。

## License

本仓库使用 MIT License，详见 `LICENSE`。
