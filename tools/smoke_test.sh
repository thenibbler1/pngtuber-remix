#!/usr/bin/env bash
# Boots the app headless with the Godot that tools/build.sh pinned, loads every
# bundled demo model (tools/smoke_test.gd), and fails if any model differs from
# upstream or if the app logs a script or extension error. Run tools/build.sh
# first; it builds the Linux debug libraries this uses.
set -euo pipefail

ROOT="$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
GODOT_VERSION="$(sed -n 's/^GODOT_VERSION="\(.*\)"$/\1/p' "$ROOT/tools/build.sh")"
TOOLS="$ROOT/.tools/godot-$GODOT_VERSION"
GODOT="$TOOLS/Godot_v${GODOT_VERSION}-stable_linux.x86_64"
OVERRIDE="$ROOT/override.cfg"
LOG="$(mktemp)"
# The app keeps its settings, autosaves and backups next to the executable,
# which here is the pinned Godot binary; start from a clean slate every run.
APP_STATE=(Preferences.pRDat DefaultTraining.tres WebsocketDocumentation.txt autosaves Backups ExportedAssets)
clean_app_state() { for f in "${APP_STATE[@]}"; do rm -rf "${TOOLS:?}/$f"; done; }

[[ -x "$GODOT" ]] || { echo "Godot not found at $GODOT: run tools/build.sh first." >&2; exit 1; }
[[ ! -e "$OVERRIDE" ]] || { echo "$OVERRIDE already exists; move it aside first." >&2; exit 1; }

trap 'rm -f "$OVERRIDE" "$LOG"; clean_app_state' EXIT
clean_app_state
printf '[autoload]\n\nSmokeTest="*res://tools/smoke_test.gd"\n' > "$OVERRIDE"

set +e
# A throwaway XDG_DATA_HOME keeps Godot's user:// (logs) out of the host's home.
XDG_DATA_HOME="$(mktemp -d)" timeout 600 "$GODOT" --headless --path "$ROOT" 2>&1 | tee "$LOG"
code=${PIPESTATUS[0]}
set -e

if grep -E 'SCRIPT ERROR|Parse Error|Node not found|dynamic library|Error loading extension|Cannot get class' "$LOG"; then
  echo "Smoke test: errors in the app log (above)." >&2
  exit 1
fi
if ((code != 0)); then
  echo "Smoke test: app exited with code $code." >&2
  exit "$code"
fi
echo "Smoke test passed."
