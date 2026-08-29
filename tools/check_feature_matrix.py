#!/usr/bin/env python3
import json
from pathlib import Path

root = Path(__file__).resolve().parents[1]
matrix = json.loads((root / "docs/feature-matrix.json").read_text())
dimensions = ["tokenize", "parse", "compile", "execute", "error", "embedding", "conformance"]
allowed = {"planned", "partial-planned", "not-applicable"}
assert matrix["schema_version"] == 1
assert matrix["dimensions"] == dimensions
ids = set()
for feature in matrix["features"]:
    assert feature["id"] not in ids
    ids.add(feature["id"])
    assert set(feature["status"]) == set(dimensions)
    assert set(feature["status"].values()) <= allowed
    assert feature["checkpoint"].startswith("JS")
print(f"JS++ feature matrix valid: {len(ids)} planned families")
