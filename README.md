# Personal Skills

这是我个人维护的 agent skills 仓库，用来沉淀可复用的工作流、工具脚本和任务委派说明。每个 skill 都放在 `skills/<skill-name>/` 下，并以 `SKILL.md` 作为入口文档。

## 目录结构

```text
.
├── skills/
│   └── opencode-delegate/
│       ├── SKILL.md
│       └── scripts/
│           └── opencode_delegate.py
├── LICENSE
└── README.md
```

## Skills

| Skill | 用途 | 入口 |
| --- | --- | --- |
| `opencode-delegate` | 将大型代码库分析、多文件修改、复杂排障和改测审循环委派给本地运行的 `opencode serve`。当前会话仍负责理解需求、整理任务说明、审查结果并向用户汇报。 | `skills/opencode-delegate/SKILL.md` |

## 使用方式

### 作为个人 skills 集合使用

将仓库中的 `skills/` 目录同步或链接到你所使用的 agent skills 目录中。例如：

```powershell
git clone <repo-url> C:\Users\<you>\Documents\GitHub\skills
```

如果目标工具支持按目录加载 skills，可以直接指向本仓库的 `skills/` 目录；如果需要复制安装，则复制单个 skill 目录，例如 `skills/opencode-delegate`。

### 使用 `opencode-delegate`

该 skill 需要本地先启动 `opencode serve`：

```bash
OPENCODE_SERVER_PASSWORD="your-password" opencode serve --hostname 127.0.0.1 --port 4096
```

默认端点为：

```text
http://127.0.0.1:4096
```

可通过环境变量覆盖连接和模型配置：

```bash
OPENCODE_BASE_URL=http://127.0.0.1:4096
OPENCODE_SERVER_USERNAME=opencode
OPENCODE_SERVER_PASSWORD=your-password
OPENCODE_PROVIDER_ID=...
OPENCODE_MODEL_ID=...
```

更多任务说明模板、审查要求和安全规则见 `skills/opencode-delegate/SKILL.md`。

## 维护约定

- 每个 skill 使用独立目录，目录名与 `SKILL.md` 中的 `name` 保持一致。
- 脚本、模板、示例等辅助文件放在对应 skill 目录下。
- 新增 skill 后更新本 README 的 Skills 表格。
- 不在 skill 文档或脚本中提交密钥、令牌、私钥或 `.env` 内容。

## License

本仓库使用 MIT License，详见 `LICENSE`。
