#!/usr/bin/env python3
"""
Delegate a coding task to a running opencode serve HTTP server.

Usage:
    python3 opencode_delegate.py '<task brief>'

Environment variables:
    OPENCODE_BASE_URL              Default: http://127.0.0.1:4096
    OPENCODE_SERVER_USERNAME       Default: opencode
    OPENCODE_SERVER_PASSWORD       Optional. Enables HTTP Basic Auth when set.
    OPENCODE_PROVIDER_ID           Optional. Used with OPENCODE_MODEL_ID.
    OPENCODE_MODEL_ID              Optional. Used with OPENCODE_PROVIDER_ID.
    OPENCODE_AGENT                 Optional. Agent name to pass to opencode.
    OPENCODE_TIMEOUT_SECONDS       Default: 1800
    OPENCODE_OUTPUT_LIMIT          Default: 30000
"""

from __future__ import annotations

import base64
import json
import os
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from typing import Any, Dict, Optional


BASE_URL = os.environ.get("OPENCODE_BASE_URL", "http://127.0.0.1:4096").rstrip("/")
USERNAME = os.environ.get("OPENCODE_SERVER_USERNAME", "opencode")
PASSWORD = os.environ.get("OPENCODE_SERVER_PASSWORD", "")
PROVIDER_ID = os.environ.get("OPENCODE_PROVIDER_ID", "")
MODEL_ID = os.environ.get("OPENCODE_MODEL_ID", "")
AGENT = os.environ.get("OPENCODE_AGENT", "")
TIMEOUT_SECONDS = int(os.environ.get("OPENCODE_TIMEOUT_SECONDS", "1800"))
OUTPUT_LIMIT = int(os.environ.get("OPENCODE_OUTPUT_LIMIT", "30000"))


def make_headers() -> Dict[str, str]:
    headers = {
        "Content-Type": "application/json",
        "Accept": "application/json, text/plain, */*",
    }

    if PASSWORD:
        token = base64.b64encode(f"{USERNAME}:{PASSWORD}".encode("utf-8")).decode("ascii")
        headers["Authorization"] = f"Basic {token}"

    return headers


def request(method: str, path: str, body: Optional[Dict[str, Any]] = None, timeout: int = TIMEOUT_SECONDS) -> Any:
    data = None

    if body is not None:
        data = json.dumps(body, ensure_ascii=False).encode("utf-8")

    req = urllib.request.Request(
        f"{BASE_URL}{path}",
        data=data,
        headers=make_headers(),
        method=method,
    )

    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            raw = resp.read().decode("utf-8", errors="replace")
            if not raw:
                return None

            content_type = resp.headers.get("Content-Type", "")
            if "json" in content_type or raw.startswith("{") or raw.startswith("["):
                return json.loads(raw)

            return raw

    except urllib.error.HTTPError as exc:
        detail = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"opencode HTTP error: {exc.code} {method} {path}\n{detail}") from exc
    except urllib.error.URLError as exc:
        raise RuntimeError(
            f"Cannot connect to opencode server at {BASE_URL}. "
            "Start it from the target repository with: "
            "opencode serve --hostname 127.0.0.1 --port 4096"
        ) from exc


def compact_json(value: Any, limit: int = OUTPUT_LIMIT) -> str:
    text = json.dumps(value, ensure_ascii=False, indent=2)
    if len(text) <= limit:
        return text
    return text[:limit] + "\n... output truncated ..."


def extract_session_id(session_response: Any) -> str:
    if isinstance(session_response, dict):
        if isinstance(session_response.get("id"), str):
            return session_response["id"]
        if isinstance(session_response.get("session"), dict) and isinstance(session_response["session"].get("id"), str):
            return session_response["session"]["id"]

    raise RuntimeError(f"Unexpected session response:\n{compact_json(session_response)}")


def extract_text_parts(message_response: Any) -> str:
    if not isinstance(message_response, dict):
        return ""

    parts = message_response.get("parts", [])
    if not isinstance(parts, list):
        return ""

    texts = []
    for part in parts:
        if not isinstance(part, dict):
            continue
        if part.get("type") == "text" and isinstance(part.get("text"), str):
            texts.append(part["text"])

    return "\n\n".join(texts).strip()


def build_message_body(task_brief: str) -> Dict[str, Any]:
    body: Dict[str, Any] = {
        "parts": [
            {
                "type": "text",
                "text": task_brief,
            }
        ]
    }

    if PROVIDER_ID and MODEL_ID:
        body["model"] = {
            "providerID": PROVIDER_ID,
            "modelID": MODEL_ID,
        }

    if AGENT:
        body["agent"] = AGENT

    return body


def safe_get(path: str, timeout: int = 60) -> Any:
    try:
        return request("GET", path, timeout=timeout)
    except Exception as exc:
        return {
            "error": str(exc),
            "path": path,
        }


def main() -> int:
    if len(sys.argv) < 2:
        print("Usage: opencode_delegate.py '<task brief>'", file=sys.stderr)
        return 2

    task_brief = sys.argv[1].strip()
    if not task_brief:
        print("Task brief is empty.", file=sys.stderr)
        return 2

    health = safe_get("/global/health", timeout=30)

    session = request("POST", "/session", {"title": "delegated coding task"}, timeout=60)
    session_id = extract_session_id(session)
    encoded_session_id = urllib.parse.quote(session_id, safe="")

    started_at = time.time()
    message = request(
        "POST",
        f"/session/{encoded_session_id}/message",
        build_message_body(task_brief),
        timeout=TIMEOUT_SECONDS,
    )
    elapsed = round(time.time() - started_at, 2)

    diff = safe_get(f"/session/{encoded_session_id}/diff", timeout=60)
    messages = safe_get(f"/session/{encoded_session_id}/message?limit=20", timeout=60)
    todo = safe_get(f"/session/{encoded_session_id}/todo", timeout=60)

    result = {
        "base_url": BASE_URL,
        "health": health,
        "session_id": session_id,
        "elapsed_seconds": elapsed,
        "assistant_text": extract_text_parts(message),
        "message_response": message,
        "todo": todo,
        "diff": diff,
        "recent_messages": messages,
        "next_review_commands": [
            "git status --short",
            "git diff --stat",
            "git diff",
        ],
    }

    print(compact_json(result))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
