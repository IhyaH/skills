#!/usr/bin/env python3
import base64
import json
import os
import sys
import time
import urllib.error
import urllib.parse
import urllib.request


BASE_URL = os.environ.get("OPENCODE_BASE_URL", "http://127.0.0.1:4096").rstrip("/")
USERNAME = os.environ.get("OPENCODE_SERVER_USERNAME", "opencode")
PASSWORD = os.environ.get("OPENCODE_SERVER_PASSWORD", "")
PROVIDER_ID = os.environ.get("OPENCODE_PROVIDER_ID", "")
MODEL_ID = os.environ.get("OPENCODE_MODEL_ID", "")
TIMEOUT_SECONDS = int(os.environ.get("OPENCODE_TIMEOUT_SECONDS", "1800"))


def make_headers():
    headers = {"Content-Type": "application/json"}

    if PASSWORD:
        token = base64.b64encode(f"{USERNAME}:{PASSWORD}".encode("utf-8")).decode("ascii")
        headers["Authorization"] = f"Basic {token}"

    return headers


def request(method, path, body=None, timeout=TIMEOUT_SECONDS):
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
            if "application/json" in content_type or raw.startswith("{") or raw.startswith("["):
                return json.loads(raw)

            return raw

    except urllib.error.HTTPError as exc:
        detail = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"opencode HTTP error: {exc.code} {method} {path}\n{detail}") from exc
    except urllib.error.URLError as exc:
        raise RuntimeError(
            f"Cannot connect to opencode server at {BASE_URL}. "
            "Start it with: opencode serve --hostname 127.0.0.1 --port 4096"
        ) from exc


def compact_json(value, limit=20000):
    text = json.dumps(value, ensure_ascii=False, indent=2)
    if len(text) <= limit:
        return text
    return text[:limit] + "\n... output truncated ..."


def extract_text_parts(message):
    if not message:
        return ""

    parts = message.get("parts", []) if isinstance(message, dict) else []
    texts = []

    for part in parts:
        if not isinstance(part, dict):
            continue
        if part.get("type") == "text" and isinstance(part.get("text"), str):
            texts.append(part["text"])

    return "\n\n".join(texts).strip()


def main():
    if len(sys.argv) < 2:
        print("Usage: opencode_delegate.py '<task brief>'", file=sys.stderr)
        sys.exit(2)

    task_brief = sys.argv[1].strip()
    if not task_brief:
        print("Task brief is empty.", file=sys.stderr)
        sys.exit(2)

    health = request("GET", "/global/health", timeout=30)

    session = request("POST", "/session", {
        "title": "delegated coding task"
    }, timeout=60)

    if not isinstance(session, dict) or "id" not in session:
        raise RuntimeError(f"Unexpected session response:\n{compact_json(session)}")

    session_id = session["id"]

    body = {
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

    started_at = time.time()
    message = request("POST", f"/session/{urllib.parse.quote(session_id)}/message", body)
    elapsed = round(time.time() - started_at, 2)

    diff = request("GET", f"/session/{urllib.parse.quote(session_id)}/diff", timeout=60)
    messages = request("GET", f"/session/{urllib.parse.quote(session_id)}/message?limit=20", timeout=60)
    todo = request("GET", f"/session/{urllib.parse.quote(session_id)}/todo", timeout=60)

    result = {
        "base_url": BASE_URL,
        "health": health,
        "session_id": session_id,
        "elapsed_seconds": elapsed,
        "assistant_text": extract_text_parts(message),
        "todo": todo,
        "diff": diff,
        "recent_messages": messages,
    }

    print(compact_json(result))


if __name__ == "__main__":
    main()
