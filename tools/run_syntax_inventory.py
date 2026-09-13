#!/usr/bin/env python3
"""Inventory JS++ lossless syntax coverage over minification-benchmarks."""

import argparse
import json
import pathlib
import re
import subprocess
import sys


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--benchmark-root", required=True, type=pathlib.Path)
    parser.add_argument("--scanner", required=True, type=pathlib.Path)
    parser.add_argument("--output", type=pathlib.Path)
    args = parser.parse_args()

    artifacts = args.benchmark_root / "packages/artifacts/artifacts"
    modules = args.benchmark_root / "packages/artifacts/node_modules"
    results = []
    for definition in sorted(artifacts.glob("*/index.ts")):
        text = definition.read_text(encoding="utf-8")
        package = re.search(r"package:\s*'([^']+)'", text)
        file_path = re.search(r"filePath:\s*'([^']+)'", text)
        if not package or not file_path:
            continue
        source = modules / package.group(1) / file_path.group(1).lstrip("/")
        run = subprocess.run(
            [str(args.scanner), str(source)], capture_output=True, text=True
        )
        fields = run.stdout.rstrip("\n").split("\t")
        record = {"artifact": definition.parent.name, "path": str(source)}
        if run.returncode == 0 and len(fields) == 10 and fields[0] == "ok":
            keys = ("bytes", "tokens", "nodes", "opaque_nodes", "delimited_nodes",
                    "identifiers", "regexes", "template_chunks", "expressions")
            record.update({key: int(value) for key, value in zip(keys, fields[1:])})
            record["accepted"] = True
        else:
            record.update({"accepted": False, "diagnostic": run.stdout.strip() or run.stderr.strip()})
        results.append(record)

    summary = {
        "artifacts": len(results),
        "accepted": sum(item["accepted"] for item in results),
        "bytes": sum(item.get("bytes", 0) for item in results),
        "tokens": sum(item.get("tokens", 0) for item in results),
        "opaque_nodes": sum(item.get("opaque_nodes", 0) for item in results),
        "nodes": sum(item.get("nodes", 0) for item in results),
        "expressions": sum(item.get("expressions", 0) for item in results),
    }
    report = {"summary": summary, "fixtures": results}
    rendered = json.dumps(report, indent=2) + "\n"
    if args.output:
        args.output.write_text(rendered, encoding="utf-8")
    sys.stdout.write(rendered)
    return 0 if summary["artifacts"] == 12 and summary["accepted"] == 12 else 1


if __name__ == "__main__":
    raise SystemExit(main())
