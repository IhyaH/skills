# opencode Server Reference

Source document: https://opencode.ai/docs/zh-cn/server/
Last observed update on source page: 2026-05-31.

Use this file as a lookup table after the main skill's Fast Path. Prefer the live OpenAPI document at `/doc` for exact endpoint schemas.

## Server flags

```bash
opencode serve [--port <number>] [--hostname <string>] [--cors <origin>]
```

Defaults:

- Port: `4096`
- Hostname: `127.0.0.1`
- OpenAPI document: `http://<hostname>:<port>/doc`
- Local OpenAPI document: `http://localhost:4096/doc`

Use repeated `--cors <origin>` flags to allow multiple browser origins. Avoid CORS unless a browser-based caller needs it.

## Authentication

Enable HTTP Basic Auth with environment variables:

```bash
OPENCODE_SERVER_PASSWORD=your-password opencode serve
```

- Default username: `opencode`
- Override username: `OPENCODE_SERVER_USERNAME`

Use authentication before exposing the server on non-localhost interfaces.

## Runtime model

Running `opencode` normally starts both a TUI client and a server. The TUI talks to the server. Running `opencode serve` starts the server without the TUI, which is the preferred mode for subagent orchestration.

The server exposes an OpenAPI 3.1 specification at `/doc`. For client generation or exact request/response schemas, fetch `/doc` from the running server.

## Endpoint groups

Use this map to decide which API area to inspect in the live OpenAPI document.

### Global and project

- Health check.
- Global event stream using server-sent events.
- List projects.
- Get the current project.

### Path, VCS, and instance

- Get current path information.
- Get version-control information.
- Destroy the current instance.

### Configuration

- Read configuration.
- Update configuration.
- List providers and default model information.

### Providers and authentication

- List providers.
- Inspect provider authentication options.
- Start OAuth authorization.
- Handle OAuth callback.
- Set provider credentials.

### Sessions

- Create sessions.
- Delete sessions.
- Update session metadata.
- Fork sessions.
- Share sessions.
- Revert sessions.
- Summarize sessions.
- Abort active session work.

### Messages and commands

- Send a message/prompt to a session.
- Send an asynchronous prompt.
- Execute slash commands.
- Request shell command execution.
- List available commands.

### Files

- Search text.
- Find files.
- Read file contents.
- Get file status.

### Tools, LSP, formatters, MCP, agents

- List experimental tools and schemas.
- Query LSP status.
- Query formatter status.
- Query MCP status.
- Add MCP dynamically.
- List available agents.

### Logs and TUI

- Write logs.
- Append, submit, and clear TUI prompt text.
- Open help, session, theme, or model UI.
- Show toast notifications.
- Control requests.

### Events and docs

- Subscribe to server events.
- Read OpenAPI docs from `GET /doc`.

## Efficient orchestration

- Prefer one opencode session per delegated task.
- Keep delegated prompts under one screen when possible.
- Include absolute repository paths in prompts.
- Ask the subagent to report changed files and commands run.
- Use file status/search/read endpoints to inspect results before accepting them.
- Use event streams for long-running prompts; otherwise poll message/session state according to the live API schema.
- Keep the main agent responsible for final validation and user-facing conclusions.
