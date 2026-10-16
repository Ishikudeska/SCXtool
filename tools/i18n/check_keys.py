import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]
KEY_RE = re.compile(r'\b(?:qsTr|tr|text)\(\s*"([a-z0-9_]+(?:\.[a-z0-9_]+)+)"')


def flatten(node, prefix=""):
    out = set()
    for k, v in node.items():
        if isinstance(v, dict) and not set(v) <= {"ht", "at"}:
            out |= flatten(v, prefix + k + ".")
        else:
            out.add(prefix + k)
    return out


def main() -> int:
    ignore = {"dot.key"}
    catalog = flatten(json.loads((ROOT / "resources/languages/english/ui.json").read_text(encoding="utf-8")))
    missing = {}
    for path in sorted((ROOT / "src").rglob("*")):
        if path.suffix not in {".cpp", ".h", ".qml"}:
            continue
        for key in KEY_RE.findall(path.read_text(encoding="utf-8")):
            if key not in catalog and key not in ignore:
                missing.setdefault(key, path.relative_to(ROOT).as_posix())
    for key, where in sorted(missing.items()):
        print(f"{key}  ({where})")
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
