"""Build resources/icon/app.ico from the icon SVGs.

    python tools/icon/build_ico.py [--preview <dir>]

Sizes up to 32 px are drawn from app-small.svg, larger ones from app.svg.
Each size is stored as a PNG inside the .ico (Windows Vista and later).
--preview also writes every size as a PNG to <dir> for checking by eye.
Dev-only; needs PyQt6.
"""
import argparse
import struct
import sys
from pathlib import Path

from PyQt6.QtCore import QBuffer, QByteArray, QIODevice, QRectF, Qt
from PyQt6.QtGui import QGuiApplication, QImage, QPainter
from PyQt6.QtSvg import QSvgRenderer

ROOT = Path(__file__).resolve().parents[2]
ICON_DIR = ROOT / "resources/icon"
SIZES = (16, 20, 24, 32, 40, 48, 64, 96, 128, 256)
SMALL_MAX = 32


def render(svg: Path, size: int) -> bytes:
    renderer = QSvgRenderer(str(svg))
    if not renderer.isValid():
        sys.exit(f"cannot read {svg}")
    image = QImage(size, size, QImage.Format.Format_ARGB32)
    image.fill(Qt.GlobalColor.transparent)
    painter = QPainter(image)
    painter.setRenderHint(QPainter.RenderHint.Antialiasing)
    painter.setRenderHint(QPainter.RenderHint.SmoothPixmapTransform)
    renderer.render(painter, QRectF(0, 0, size, size))
    painter.end()
    data = QByteArray()
    buffer = QBuffer(data)
    buffer.open(QIODevice.OpenModeFlag.WriteOnly)
    image.save(buffer, "PNG")
    return bytes(data)


def write_ico(path: Path, images: list[tuple[int, bytes]]) -> None:
    header = struct.pack("<HHH", 0, 1, len(images))
    offset = len(header) + 16 * len(images)
    entries, blobs = b"", b""
    for size, png in images:
        dim = 0 if size >= 256 else size  # 0 means 256
        entries += struct.pack("<BBBBHHII", dim, dim, 0, 0, 1, 32, len(png), offset)
        blobs += png
        offset += len(png)
    path.write_bytes(header + entries + blobs)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--preview", type=Path)
    args = ap.parse_args()
    app = QGuiApplication(sys.argv[:1])  # QPainter needs an application object
    images = []
    for size in SIZES:
        svg = ICON_DIR / ("app-small.svg" if size <= SMALL_MAX else "app.svg")
        png = render(svg, size)
        images.append((size, png))
        if args.preview:
            args.preview.mkdir(parents=True, exist_ok=True)
            (args.preview / f"app-{size}.png").write_bytes(png)
    write_ico(ICON_DIR / "app.ico", images)
    print(f"wrote {ICON_DIR / 'app.ico'} ({', '.join(str(s) for s in SIZES)} px)")
    del app


if __name__ == "__main__":
    main()
