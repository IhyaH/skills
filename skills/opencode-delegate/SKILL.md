---

name: opencode-delegate
description: Delegate large codebase analysis and multi-file code modification tasks to a running opencode serve HTTP server. Use when the user asks for broad refactoring, large bug fixes, repository-wide edits, complex codebase investigation, or edit-test-review loops that should be handled by opencode as a subordinate coding agent.
compatibility: Designed for Claude Code. Requires shell access, Python 3, network access to a local opencode serve endpoint, and a running opencode server.
allowed-tools: Bash Read Grep Glob
metadata:
version: "1.0"
target: "opencode serve"
------------------------

# opencode Delegate

Use this skill to delegate large coding tasks to a local `opencode serve` HTTP server.

The current Claude Code session remains responsible for understanding the user request, preparing a precise task brief, reviewing opencode's result, and reporting the final outcome. opencode is only used as a subordinate coding agent.

## Requirements

Before using this skill, confirm that an opencode server is running.

Default endpoint:

```bash
http://127.0.0.1:4096
```

Recommended startup command:

```bash
OPENCODE_SERVER_PASSWORD="your-password" \
opencode serve --hostname 127.0.0.1 --port 4096
```

Use localhost unless the user explicitly configured a secure remote endpoint.

## Environment variables

Use these values when present:

```bash
OPENCODE_BASE_URL="${OPENCODE_BASE_URL:-http://127.0.0.1:4096}"
OPENCODE_SERVER_USERNAME="${OPENCODE_SERVER_USERNAME:-opencode}"
OPENCODE_SERVER_PASSWORD="${OPENCODE_SERVER_PASSWORD:-}"
OPENCODE_PROVIDER_ID="${OPENCODE_PROVIDER_ID:-}"
OPENCODE_MODEL_ID="${OPENCODE_MODEL_ID:-}"
```

If `OPENCODE_SERVER_PASSWORD` is set, use HTTP Basic Auth.

## When to use

Use this skill when the task involves one or more of the following:

* Large repository context.
* Multi-file edits.
* Repository-wide refactoring.
* Complex bug investigation.
* Repeated edit and validation cycles.
* Tasks where opencode should inspect the project, modify files, run tests, and return a diff.

Do not use this skill for small direct edits that Claude Code can safely perform itself.

## Workflow

1. Inspect the user request and current repository state.
2. Prepare a compact task brief for opencode.
3. Run the helper script in `scripts/opencode_delegate.py`.
4. Review the returned summary, changed files, diff, and validation output.
5. Inspect the actual working tree with `git diff` and relevant file reads.
6. Report the final result to the user.

Do not blindly accept opencode output.

## Task brief format

Send opencode a task brief using this structure:

```text
You are operating as a subordinate coding agent.

Goal:
<state the requested result>

Repository:
<absolute or relative repository path>

Relevant files or directories:
<list known paths, or say unknown and ask opencode to inspect>

Constraints:
<include user constraints, coding style, forbidden changes, compatibility requirements>

Validation commands:
<commands opencode should run, such as npm test, pytest, go test ./..., cargo test>

Expected output:
1. Summary of changes.
2. Files changed.
3. Validation commands run and results.
4. Diff summary.
5. Remaining risks or follow-up work.
```

## Helper script usage

Run:

```bash
python3 "${CLAUDE_SKILL_DIR}/scripts/opencode_delegate.py" "$(cat <<'TASK'
You are operating as a subordinate coding agent.

Goal:
...

Repository:
...

Relevant files or directories:
...

Constraints:
...

Validation commands:
...

Expected output:
1. Summary of changes.
2. Files changed.
3. Validation commands run and results.
4. Diff summary.
5. Remaining risks or follow-up work.
TASK
)"
```

## Review requirements

After opencode completes, always check:

```bash
git status --short
git diff --stat
git diff
```

If tests were expected, verify whether they actually ran. If opencode did not run them, either run them directly or clearly report that they were not run.

If opencode changed unrelated files, produced an overly broad patch, or failed to follow constraints, do not present the work as complete. Ask opencode for a narrower patch or manually correct the result.

## Security rules

Do not send secrets, tokens, private keys, or `.env` contents to opencode.

Do not use a public opencode endpoint unless the user explicitly configured and approved it.

Do not approve destructive shell commands unless the user explicitly requested them.

Prefer working on a clean git branch or git worktree before delegating broad modifications.
