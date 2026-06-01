---
name: opencode-subagent
description: Use when Codex should delegate work to opencode running as a headless HTTP subagent via `opencode serve`, especially for independent repository analysis, parallel implementation, file search, command execution, second-pass review, or OpenAPI-driven session/message automation.
---

# OpenCode Subagent

## Fast Path

Use this sequence before reading deeper references:

1. Check for a live server at `http://127.0.0.1:4096/doc`.
2. If absent, start one with `opencode serve` from the target repository or explicitly pass the intended working directory in the delegated prompt.
3. Fetch `/doc` from the live server for exact endpoint schemas.
4. Create one session per delegated task.
5. Send a narrow prompt using the template below.
6. Monitor messages or events until completion.
7. Review the subagent output and file changes before using them.

Read [references/server-api.md](references/server-api.md) only when you need server flags, authentication, CORS, or the endpoint capability map.

## When To Delegate

Good delegation targets:

- Independent codebase reconnaissance.
- A second implementation attempt for comparison.
- Focused file search or API tracing.
- Test failure investigation.
- Review of a bounded diff.
- Long-running analysis that can proceed while the main agent works.

Keep in the main agent:

- Final user-facing answer.
- Git commits, pushes, PRs, and destructive operations.
- Secret handling or credential setup.
- Broad product/design judgment.
- Final validation and reconciliation of edits.

## Server Setup

Default base URL: `http://127.0.0.1:4096`

Default document URL:

```text
http://127.0.0.1:4096/doc
```

Start command:

```bash
opencode serve
```

If binding outside localhost, require Basic Auth with `OPENCODE_SERVER_PASSWORD` and prefer a narrow hostname. See the reference file for details.

## Delegation Prompt Template

Use this template verbatim unless the task needs extra constraints:

```text
You are an opencode subagent assisting the main Codex agent.

Task: <one bounded objective>
Repository: <absolute repository path>
Relevant paths: <files/directories or "discover as needed">
Allowed actions: <analysis only | edit files | run tests | run read-only commands>
Constraints:
- Preserve unrelated user changes.
- Avoid broad refactors unless necessary for the task.
- Do not commit, push, delete unrelated files, or handle secrets.
- Prefer existing project patterns.

Return:
- Summary of findings or changes.
- Files changed, if any.
- Commands run and results.
- Remaining risks or tests not run.
```

## API Usage Heuristic

Fetch the live OpenAPI spec from `/doc` whenever endpoint shapes matter. Use these API groups first:

- `session`: create or manage the task context.
- `message`: send the prompt, async prompt, slash command, or shell-command request.
- `event`: stream progress for long tasks.
- `file`: search, read, and inspect file status.
- `config`, `provider`, `agent`: discover available models/providers/agents.

Prefer one small delegation over one vague large delegation. If the subagent edits files, inspect the diff and run the relevant validation yourself.

## Completion Checklist

Before trusting a subagent result:

- Confirm the task objective was actually answered.
- Check changed files with the main agent's normal tools.
- Run or record relevant tests.
- Merge only useful changes into the main line of work.
- Mention subagent uncertainty in the final answer only when it affects the user.

