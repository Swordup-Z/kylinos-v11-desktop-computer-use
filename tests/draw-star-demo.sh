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
  LINE_TOOL_X=612
  LINE_TOOL_Y=524
  BLACK_X=686
  BLACK_Y=844
  FILL_TOOL_X=612
  FILL_TOOL_Y=484
  YELLOW_X=748
  YELLOW_Y=863
  FILL_X=960
  FILL_Y=620
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
require_command setsid
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
    setsid "$PAINT_APP" >/tmp/kylinos-computer-use-paint.log 2>&1 < /dev/null &
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
line_tool_x="${LINE_TOOL_X:-$((X + 22))}"
line_tool_y="${LINE_TOOL_Y:-$((Y + 294))}"
black_x="${BLACK_X:-$((X + 96))}"
black_y="${BLACK_Y:-$((Y + 614))}"
fill_tool_x="${FILL_TOOL_X:-$((X + 22))}"
fill_tool_y="${FILL_TOOL_Y:-$((Y + 254))}"
yellow_x="${YELLOW_X:-$((X + 158))}"
yellow_y="${YELLOW_Y:-$((Y + 633))}"
fill_x="${FILL_X:-$center_x}"
fill_y="${FILL_Y:-$center_y}"

mapfile -t star_points < <(
    python3 - "$center_x" "$center_y" "$STAR_RADIUS" <<'PY'
import math
import sys

cx = int(sys.argv[1])
cy = int(sys.argv[2])
outer_radius = int(sys.argv[3])
inner_radius = round(outer_radius * 0.45)

points = []
for i in range(10):
    radius = outer_radius if i % 2 == 0 else inner_radius
    angle = math.radians(-90 + i * 36)
    points.append((round(cx + radius * math.cos(angle)),
                   round(cy + radius * math.sin(angle))))

path = points + [points[0]]
for x, y in path:
    print(f"{x} {y}")
PY
)

click_at() {
    local x="$1"
    local y="$2"
    xdotool mousemove "$x" "$y"
    sleep 0.2
    "$COMPUTER_USE_BIN" click 1
    sleep 0.35
}

move_pointer_smooth() {
    local from_x="$1"
    local from_y="$2"
    local to_x="$3"
    local to_y="$4"
    local duration_ms="$5"

    mapfile -t pointer_steps < <(
        python3 - "$from_x" "$from_y" "$to_x" "$to_y" "$duration_ms" <<'PY'
import sys

x1, y1, x2, y2, duration_ms = map(int, sys.argv[1:])
steps = max(8, min(24, duration_ms // 35))
delay = max(0.010, duration_ms / steps / 1000)
for i in range(1, steps + 1):
    x = round(x1 + (x2 - x1) * i / steps)
    y = round(y1 + (y2 - y1) * i / steps)
    print(f"{x} {y} {delay:.3f}")
PY
    )

    for step in "${pointer_steps[@]}"; do
        read -r x y delay <<<"$step"
        xdotool mousemove "$x" "$y"
        sleep "$delay"
    done
}

draw_line() {
    local from_x="$1"
    local from_y="$2"
    local to_x="$3"
    local to_y="$4"

    xdotool mousemove "$from_x" "$from_y"
    sleep 0.15
    xdotool mousedown 1
    sleep 0.12
    move_pointer_smooth "$from_x" "$from_y" "$to_x" "$to_y" "$STAR_SEGMENT_MS"
    sleep 0.08
    xdotool mouseup 1
    sleep 0.2
}

click_at "$line_tool_x" "$line_tool_y"
click_at "$black_x" "$black_y"

for ((i = 0; i < ${#star_points[@]} - 1; i++)); do
    read -r from_x from_y <<<"${star_points[$i]}"
    read -r to_x to_y <<<"${star_points[$((i + 1))]}"
    draw_line "$from_x" "$from_y" "$to_x" "$to_y"
done

click_at "$fill_tool_x" "$fill_tool_y"
click_at "$yellow_x" "$yellow_y"
click_at "$fill_x" "$fill_y"

if ! xdotool search --onlyvisible --class "$PAINT_APP" >/dev/null 2>&1; then
    echo "$PAINT_APP window disappeared during the demo." >&2
    exit 1
fi

echo "Drew and filled a yellow star in $PAINT_APP using $COMPUTER_USE_BIN"
