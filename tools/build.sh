#!/usr/bin/env bash
# Builds PNGTuber Remix from source: native extensions, then the exported app.
#
#   tools/build.sh                 # Windows x86_64 build -> dist/
#   tools/build.sh linux           # Linux x86_64 build   -> dist/
#   tools/build.sh windows --editor-libs
#                                  # also build the Windows *debug* libraries the
#                                  # Godot editor needs to open the project on Windows
#
# Runs on a Linux x86_64 host (Ubuntu 24.04, WSL2 on Windows, or CI). Needs:
# git, curl, unzip, python3 + scons, g++, and mingw-w64 for Windows builds.
# Godot and its export templates are downloaded once into .tools/ and checked
# against pinned SHA-512 sums, so every build uses byte-identical engine files.
set -euo pipefail

GODOT_VERSION="4.7.2"
GODOT_EDITOR_SHA512="9aa00f7a605200940bce3027a567b782f49bd8e940dd06ae9e987bd65aee1b1467edd56ed84fcdcbdd44354bf613bdbb4e5d2913e925850368e150c59ed54c65"
GODOT_TEMPLATES_SHA512="ca4d71c4d7b81dfc15d1a98baa07534aa95b03fdda78a0075b06672e1648d2e5f40980c9adc28d23e1b92e732ee7bf3461997aa804af74ec2fcd7a93ccb84079"

PLATFORM="windows"
EDITOR_LIBS=0
for arg in "$@"; do
  case "$arg" in
    windows|linux) PLATFORM="$arg" ;;
    --editor-libs) EDITOR_LIBS=1 ;;
    -h|--help) sed -n '2,13p' "$0"; exit 0 ;;
    *) echo "Unknown argument: $arg" >&2; exit 2 ;;
  esac
done

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TOOLS="$ROOT/.tools/godot-$GODOT_VERSION"
GODOT="$TOOLS/Godot_v${GODOT_VERSION}-stable_linux.x86_64"
RELEASE_URL="https://github.com/godotengine/godot/releases/download/${GODOT_VERSION}-stable"
JOBS="$(nproc 2>/dev/null || echo 4)"

step() { printf '\n==> %s\n' "$*"; }

require() {
  local missing=()
  for tool in "$@"; do command -v "$tool" >/dev/null 2>&1 || missing+=("$tool"); done
  if ((${#missing[@]})); then
    echo "Missing required tools: ${missing[*]}" >&2
    echo "On Ubuntu/WSL: sudo apt-get install -y git curl unzip g++ mingw-w64 python3-pip && pip install scons" >&2
    exit 1
  fi
}

fetch_verified() { # url dest sha512
  local url="$1" dest="$2" sha="$3"
  if [[ ! -f "$dest" ]] || ! echo "$sha  $dest" | sha512sum --check --status; then
    echo "Downloading $(basename "$dest")"
    curl -fL --retry 4 -o "$dest.part" "$url"
    mv "$dest.part" "$dest"
  fi
  echo "$sha  $dest" | sha512sum --check
}

require git curl unzip sha512sum scons g++
[[ "$PLATFORM" == windows ]] && require x86_64-w64-mingw32-g++

step "Fetching pinned sources (godot-cpp, godot-gif)"
git -C "$ROOT" submodule update --init native/godot-cpp native/godot-gif

step "Installing Godot $GODOT_VERSION into .tools/ (verified)"
mkdir -p "$TOOLS"
fetch_verified "$RELEASE_URL/Godot_v${GODOT_VERSION}-stable_linux.x86_64.zip" \
  "$TOOLS/editor.zip" "$GODOT_EDITOR_SHA512"
fetch_verified "$RELEASE_URL/Godot_v${GODOT_VERSION}-stable_export_templates.tpz" \
  "$TOOLS/templates.tpz" "$GODOT_TEMPLATES_SHA512"
[[ -x "$GODOT" ]] || unzip -o -q "$TOOLS/editor.zip" -d "$TOOLS"
# Self-contained mode: Godot keeps its settings and templates next to the binary
# instead of in ~/.local, so the build never depends on the host's Godot setup.
touch "$TOOLS/._sc_"
TEMPLATES="$TOOLS/editor_data/export_templates/${GODOT_VERSION}.stable"
if [[ ! -f "$TEMPLATES/version.txt" ]]; then
  mkdir -p "$TEMPLATES"
  unzip -o -q -j "$TOOLS/templates.tpz" -d "$TEMPLATES" \
    templates/version.txt \
    "templates/${PLATFORM}_release*x86_64*" "templates/${PLATFORM}_debug*x86_64*"
fi

step "Building native extensions from source"
build_native() { scons -C "$ROOT/native" -j"$JOBS" platform="$1" target="$2"; }
# The headless editor below loads the host (Linux) debug libraries while it
# imports and exports scenes that use the extension classes.
build_native linux template_debug
build_native "$PLATFORM" template_release
if ((EDITOR_LIBS)) && [[ "$PLATFORM" != linux ]]; then
  build_native "$PLATFORM" template_debug
fi

step "Importing project"
# --import exits once the import finishes; a non-zero exit here is a real failure.
"$GODOT" --headless --path "$ROOT" --import

step "Exporting $PLATFORM build"
OUT="$ROOT/dist/$PLATFORM"
rm -rf "$OUT" && mkdir -p "$OUT"
touch "$ROOT/dist/.gdignore"  # keep Godot from importing build output
if [[ "$PLATFORM" == windows ]]; then
  PRESET="Windows Desktop"; BINARY="PNGTube-Remix.exe"
else
  PRESET="Linux/X11"; BINARY="PNGTube-Remix.x86_64"
fi
"$GODOT" --headless --path "$ROOT" --export-release "$PRESET" "$OUT/$BINARY"
git -C "$ROOT" describe --tags --exclude nightly --always --dirty > "$OUT/version.txt"
cp "$ROOT/LICENSE" "$OUT/LICENSE.txt"
cp "$ROOT/THIRD-PARTY-NOTICES.md" "$OUT/THIRD-PARTY-NOTICES.md"

ARCHIVE="$ROOT/dist/PNGTube-Remix-${PLATFORM}-x86_64.zip"
rm -f "$ARCHIVE"
(cd "$OUT" && python3 -m zipfile -c "$ARCHIVE" .)

step "Done"
ls -l "$OUT"
echo "Archive: $ARCHIVE"
