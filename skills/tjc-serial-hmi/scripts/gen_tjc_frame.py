#!/usr/bin/env python3
"""将淘晶驰 ASCII 指令转换为带帧尾的字节序列。"""

from __future__ import annotations

import argparse
import sys

FRAME_END = b"\xff\xff\xff"


def build_frame(command: str, encoding: str) -> bytes:
    return command.encode(encoding) + FRAME_END


def format_hex(data: bytes) -> str:
    return " ".join(f"{byte:02X}" for byte in data)


def main() -> int:
    parser = argparse.ArgumentParser(description="生成淘晶驰串口屏命令帧")
    parser.add_argument("command", help="不带帧尾的 ASCII 或文本命令，例如: page main")
    parser.add_argument(
        "--encoding",
        default="utf-8",
        help="命令字符串编码，默认 utf-8；发送中文时可改为 gb2312",
    )
    parser.add_argument(
        "--hex",
        action="store_true",
        help="输出十六进制字节序列而不是 Python 风格字节串",
    )
    args = parser.parse_args()

    try:
        frame = build_frame(args.command, args.encoding)
    except LookupError as exc:
        print(f"未知编码: {args.encoding}", file=sys.stderr)
        return 2
    except UnicodeEncodeError as exc:
        print(f"命令无法按 {args.encoding} 编码: {exc}", file=sys.stderr)
        return 2

    if args.hex:
        print(format_hex(frame))
    else:
        print(repr(frame))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
