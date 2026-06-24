#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
COMPUTER_USE_BIN="${COMPUTER_USE_BIN:-${ROOT_DIR}/build/kylinos-v11-desktop-computer-use}"
PAINT_APP="${PAINT_APP:-gpaint}"
STAR_RADIUS="${STAR_RADIUS:-190}"
STAR_SEGMENT_MS="${STAR_SEGMENT_MS:-900}"

usage() {
    cat <<'EOF'
Usage:
  tests/draw-star-demo.sh

Environment overrides:
  COMPUTER_USE_BIN=/path/to/kylinos-v11-desktop-computer-use
  PAINT_APP=gpaint
  STAR_RADIUS=210
  STAR_SEGMENT_MS=1500
  STAR_CENTER_X=960
  STAR_CENTER_Y=620
EOF
}

if [[ "${1:-}" == "-h" || "${1:-}" == "--help" ]]; then
    usage
    exit 0
fi

if [[ "$#" -ne 0 ]]; then
    usage >&2
    exit 2
fi

require_command() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Missing required command: $1" >&2
        exit 127
    fi
}

require_command xdotool
require_command python3
require_command "$PAINT_APP"

if [[ ! -x "$COMPUTER_USE_BIN" ]]; then
    echo "Missing executable: $COMPUTER_USE_BIN" >&2
    echo "Build it first: cmake --build build" >&2
    exit 127
fi

find_paint_window() {
    xdotool search --onlyvisible --class "$PAINT_APP" 2>/dev/null | tail -n 1 || true
}

window_id="$(find_paint_window)"
if [[ -z "$window_id" ]]; then
    "$PAINT_APP" >/tmp/kylinos-computer-use-paint.log 2>&1 &
    sleep 2
    window_id="$(find_paint_window)"
fi

if [[ -z "$window_id" ]]; then
    echo "Could not find a visible $PAINT_APP window." >&2
    exit 1
fi

xdotool windowraise "$window_id" >/dev/null 2>&1 || true
xdotool windowactivate --sync "$window_id"
sleep 0.5

eval "$(xdotool getwindowgeometry --shell "$window_id")"

center_x="${STAR_CENTER_X:-$((X + WIDTH / 2 + 45))}"
center_y="${STAR_CENTER_Y:-$((Y + HEIGHT / 2 + 35))}"

mapfile -t star_steps < <(
    python3 - "$center_x" "$center_y" "$STAR_RADIUS" <<'PY'
import math
import sys

cx = int(sys.argv[1])
cy = int(sys.argv[2])
radius = int(sys.argv[3])

points = []
for i in range(5):
    angle = math.radians(-90 + i * 72)
    points.append((round(cx + radius * math.cos(angle)),
                   round(cy + radius * math.sin(angle))))

order = [0, 2, 4, 1, 3, 0]
path = [points[i] for i in order]
print(f"start {path[0][0]} {path[0][1]}")
for (x1, y1), (x2, y2) in zip(path, path[1:]):
    print(f"drag {x2 - x1} {y2 - y1}")
PY
)

read -r _ start_x start_y <<<"${star_steps[0]}"
xdotool mousemove --sync "$start_x" "$start_y"
sleep 0.5

for step in "${star_steps[@]:1}"; do
    read -r _ dx dy <<<"$step"
    "$COMPUTER_USE_BIN" drag "$dx" "$dy" 1 "$STAR_SEGMENT_MS"
    sleep 0.15
done

echo "Drew a star in $PAINT_APP using $COMPUTER_USE_BIN"
