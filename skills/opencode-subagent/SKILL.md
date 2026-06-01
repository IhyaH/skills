---
name: opencode-subagent
description: Use when Codex needs to run, connect to, or orchestrate opencode as an HTTP server/subagent for delegation, parallel coding assistance, repository analysis, file search, shell execution, session/message management, or OpenAPI-driven automation via `opencode serve`.
---

# OpenCode Subagent

## Purpose

Use `opencode serve` to expose opencode as a headless HTTP server that another agent can control. Treat it as a subordinate coding agent: start or discover the server, create a session, send a bounded task prompt, collect messages/events/results, and reconcile any file changes with the main agent's workspace rules.

Read [references/server-api.md](references/server-api.md) when you need endpoint details, authentication, CORS, or the API capability map.

## Workflow

1. Confirm whether an opencode server is already running.
   - Try the expected health endpoint or OpenAPI document URL.
   - Default base URL: `http://127.0.0.1:4096`.
   - OpenAPI document: `GET /doc`.
2. Start the server only when needed.
   - Basic local server: `opencode serve`.
   - Custom port/host: `opencode serve --port <port> --hostname <host>`.
   - Browser callers may need repeated `--cors <origin>` flags.
3. Add authentication when exposing beyond trusted localhost.
   - Set `OPENCODE_SERVER_PASSWORD`.
   - Optional username override: `OPENCODE_SERVER_USERNAME`.
   - Default username is `opencode`.
4. Create or select an opencode session for the delegated task.
5. Send a narrow, self-contained prompt that states:
   - the exact objective,
   - the working directory or relevant paths,
   - constraints from the main task,
   - expected output format,
   - whether file edits, shell commands, or only analysis are allowed.
6. Monitor the task through message responses or server-sent events.
7. Inspect outputs and changed files before trusting them.
   - The main Codex agent remains responsible for validation, tests, git hygiene, and final user communication.

## Delegation Pattern

Use opencode as a subagent for work that benefits from an independent pass:

```text
You are a subagent assisting the main coding agent.
Task: <specific task>
Repository: <absolute path>
Constraints: <tests, style, no unrelated refactors, etc.>
Allowed actions: <analysis only | edit files | run tests>
Return: <summary, changed files, commands run, risks>
```

Keep prompts small and explicit. Do not ask the subagent to own broad project direction, user communication, credentials, git commits, pushes, or destructive operations unless the user explicitly requested that delegation.

## Practical API Use

Prefer generated clients or direct HTTP calls from the OpenAPI spec at `/doc` when building integrations. For ad hoc agent orchestration, use these groups first:

- Sessions: create, update, fork, share, revert, summarize, abort, and delete task contexts.
- Messages: send prompts, run async prompts, execute slash commands, or request shell command execution.
- Files: search text, find files, read file contents, and inspect file status.
- Config/providers/agents: discover models, providers, authentication state, and available agents.
- Events: subscribe to server or session event streams for long-running work.

If endpoint shapes are uncertain, fetch `/doc` from the live server instead of relying on memory; opencode's API may change.

## Safety Rules

- Prefer localhost binding unless there is a clear need for remote access.
- Require Basic Auth before binding to non-local interfaces.
- Never pass secrets to opencode unless the user has authorized that use.
- Treat subagent edits as untrusted until reviewed.
- Preserve user changes and follow the main agent's repository instructions when merging results.
- Avoid delegating irreversible filesystem, git, network, or production actions.

