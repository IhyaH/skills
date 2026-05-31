---
name: opencode-delegate
description: Delegate large codebase analysis and multi-file code modification tasks to a running opencode serve HTTP server. Use when the user asks for broad refactoring, large bug fixes, repository-wide edits, complex codebase investigation, or edit-test-review loops that should be handled by opencode as a subordinate coding agent.
---

# opencode Delegate

Use this skill to delegate large coding tasks to a local `opencode serve` HTTP server.

Claude Code remains the controller. It prepares the task brief, sends it to opencode, reviews the result, checks the working tree, and reports the final outcome. opencode is used only as a subordinate coding agent.

## Requirements

An opencode server must already be running from the repository that should be inspected or modified.

Default endpoint:

```bash
http://127.0.0.1:4096
```

Recommended startup command, run from the target repository root:

```bash
OPENCODE_SERVER_PASSWORD="your-password" \
opencode serve --hostname 127.0.0.1 --port 4096
```

Use localhost unless the user explicitly configured a secure remote endpoint.

## Environment variables

Use these variables when available:

```bash
OPENCODE_BASE_URL="${OPENCODE_BASE_URL:-http://127.0.0.1:4096}"
OPENCODE_SERVER_USERNAME="${OPENCODE_SERVER_USERNAME:-opencode}"
OPENCODE_SERVER_PASSWORD="${OPENCODE_SERVER_PASSWORD:-}"
OPENCODE_PROVIDER_ID="${OPENCODE_PROVIDER_ID:-}"
OPENCODE_MODEL_ID="${OPENCODE_MODEL_ID:-}"
OPENCODE_AGENT="${OPENCODE_AGENT:-}"
OPENCODE_DIRECTORY="${OPENCODE_DIRECTORY:-$(pwd)}"
OPENCODE_NO_REPLY="${OPENCODE_NO_REPLY:-}"
```

If `OPENCODE_SERVER_PASSWORD` is set, use HTTP Basic Auth.

## When to use

Use this skill when the task involves large repository context, multi-file edits, repository-wide refactoring, complex bug investigation, repeated edit and validation cycles, or a coding task where opencode should inspect the project, modify files, run tests, and return a diff.

Do not use this skill for small direct edits that Claude Code can safely perform itself.

## Workflow

1. Inspect the user request and current repository state.
2. Confirm the repository is clean enough for delegated edits with `git status --short`.
3. Prepare a compact task brief for opencode.
4. Run the helper script at `scripts/opencode_delegate.py`.
5. Review the returned summary, changed files, diff, and validation output.
6. Inspect the actual working tree with `git status` and `git diff`.
7. Report the final result to the user.

Do not blindly accept opencode output.

## Locating the helper script

For a project-level skill, the helper script is usually here:

```bash
.claude/skills/opencode-delegate/scripts/opencode_delegate.py
```

For a user-level skill, the helper script is usually here:

```bash
~/.claude/skills/opencode-delegate/scripts/opencode_delegate.py
```

If unsure, locate it with:

```bash
SCRIPT_PATH="$(find .claude/skills ~/.claude/skills -path '*/opencode-delegate/scripts/opencode_delegate.py' -type f 2>/dev/null | head -n 1)"
```

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

Run this command from the repository root. The helper sends the current working directory to opencode as the target `directory`; set `OPENCODE_DIRECTORY` if the target repository is somewhere else.

For an HTTP smoke test that does not wait for a model response, set `OPENCODE_NO_REPLY=true`.

```bash
SCRIPT_PATH="$(find .claude/skills ~/.claude/skills -path '*/opencode-delegate/scripts/opencode_delegate.py' -type f 2>/dev/null | head -n 1)"
python3 "$SCRIPT_PATH" "$(cat <<'TASK'
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

After opencode completes, always run:

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
