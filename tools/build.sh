#!/usr/bin/env bash
# Builds PNGTuber Remix from source: native extensions, then the exported app.
#
#   tools/build.sh                 # Windows x86_64 build -> dist/windows/
#   tools/build.sh linux           # Linux x86_64 build   -> dist/linux/
#   tools/build.sh windows --debug # debug build (reports script errors) -> dist/windows-debug/
#   tools/build.sh windows --editor-libs
#                                  # also build the Windows *debug* libraries the
#                                  # Godot editor needs to open the project on Windows
#
# Runs on a Linux x86_64 host (Ubuntu 24.04, WSL2 on Windows, or CI). Needs:
# git, curl, unzip, python3, scons, g++, and mingw-w64 for Windows builds.
# Godot and its export templates are downloaded once into .tools/ and checked
# against pinned SHA-512 sums, so every build uses byte-identical engine files.
set -euo pipefail

GODOT_VERSION="4.7.2"
GODOT_EDITOR_SHA512="9aa00f7a605200940bce3027a567b782f49bd8e940dd06ae9e987bd65aee1b1467edd56ed84fcdcbdd44354bf613bdbb4e5d2913e925850368e150c59ed54c65"
GODOT_TEMPLATES_SHA512="ca4d71c4d7b81dfc15d1a98baa07534aa95b03fdda78a0075b06672e1648d2e5f40980c9adc28d23e1b92e732ee7bf3461997aa804af74ec2fcd7a93ccb84079"

PLATFORM="windows"
MODE="release"
EDITOR_LIBS=0
for arg in "$@"; do
  case "$arg" in
    windows|linux) PLATFORM="$arg" ;;
    --debug) MODE="debug" ;;
    --editor-libs) EDITOR_LIBS=1 ;;
    -h|--help) sed -n '2,14p' "$0"; exit 0 ;;
    *) echo "Unknown argument: $arg" >&2; exit 2 ;;
  esac
done

ROOT="$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
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
    echo "On Ubuntu/WSL: sudo apt-get install -y git curl unzip python3 g++ mingw-w64 scons" >&2
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

require git curl unzip sha512sum python3 scons g++
[[ "$PLATFORM" == windows ]] && require x86_64-w64-mingw32-g++

step "Fetching pinned sources (godot-cpp, godot-gif)"
git -C "$ROOT" submodule update --init native/godot-cpp native/godot-gif

step "Installing Godot $GODOT_VERSION into .tools/ (verified)"
mkdir -p "$TOOLS"
fetch_verified "$RELEASE_URL/Godot_v${GODOT_VERSION}-stable_linux.x86_64.zip" \
  "$TOOLS/editor.zip" "$GODOT_EDITOR_SHA512"
fetch_verified "$RELEASE_URL/Godot_v${GODOT_VERSION}-stable_export_templates.tpz" \
  "$TOOLS/templates.tpz" "$GODOT_TEMPLATES_SHA512"

# Unpacks into a scratch dir and moves files into place only once unzip has
# succeeded, so an interrupted run never leaves a truncated binary behind.
unpack() { # archive dest member...
  local archive="$1" dest="$2" tmp
  shift 2
  tmp="$(mktemp -d "$TOOLS/unpack.XXXXXX")"
  unzip -q -j "$archive" "$@" -d "$tmp"
  mkdir -p "$dest"
  mv -f "$tmp"/* "$dest"/
  rmdir "$tmp"
}
rm -rf "$TOOLS"/unpack.*  # leftovers from an interrupted run

[[ -x "$GODOT" ]] || unpack "$TOOLS/editor.zip" "$TOOLS" "$(basename "$GODOT")"
# Self-contained mode: Godot keeps its settings and templates next to the binary
# instead of in ~/.local, so the build never depends on the host's Godot setup.
touch "$TOOLS/._sc_"
TEMPLATES="$TOOLS/editor_data/export_templates/${GODOT_VERSION}.stable"
if [[ "$PLATFORM" == windows ]]; then
  NEEDED=(windows_release_x86_64.exe windows_release_x86_64_console.exe
          windows_debug_x86_64.exe windows_debug_x86_64_console.exe)
else
  NEEDED=(linux_release.x86_64 linux_debug.x86_64)
fi
MISSING=()
for f in version.txt "${NEEDED[@]}"; do
  [[ -f "$TEMPLATES/$f" ]] || MISSING+=("templates/$f")
done
((${#MISSING[@]} == 0)) || unpack "$TOOLS/templates.tpz" "$TEMPLATES" "${MISSING[@]}"

# Full license texts for everything shipped besides the app itself: Godot and
# the libraries compiled into it (dumped from the pinned engine), then each
# plugin and bundled asset.
LICENSE_FILES=(
  "godot-cpp (in the plugin DLLs)|native/godot-cpp/LICENSE.md"
  "godot-gif (GIF import plugin)|native/godot-gif/LICENSE.txt"
  "giflib (in godot-gif)|native/godot-gif/src/thirdparty/giflib/COPYING"
  "Goost (in godot-gif)|native/godot-gif/src/thirdparty/GOOST LICENSE.txt"
  "Godot-Global-Input (global hotkeys plugin)|native/global_input/LICENSE"
  "miniaudio (microphone plugin)|licenses/miniaudio.txt"
  "kiss_fft (microphone plugin)|licenses/kiss_fft.txt"
  "aimg_io (APNG import/export)|addons/aimg_io/COPYING.txt"
  "WigglyAppendage2D|licenses/wiggly-appendage-2d.txt"
  "Godot lip sync|licenses/godot-lip-sync.txt"
  "Pixelorama (PSD import)|licenses/pixelorama.txt"
  "Open Sans font|licenses/open-sans.txt"
)
write_licenses() { # out
  local out="$1" tmp rule entry
  tmp="$(mktemp -d)"
  printf 'config_version=5\n' > "$tmp/project.godot"
  cp "$ROOT/tools/engine_licenses.gd" "$tmp/"
  "$GODOT" --headless --path "$tmp" -s res://engine_licenses.gd -- "$tmp/engine.txt" >/dev/null
  rule="$(printf '=%.0s' {1..79})"
  {
    echo "Third-party licenses for PNGTube-Remix. The app itself is under LICENSE.txt."
    printf '\n%s\n\n' "$rule"
    cat "$tmp/engine.txt"
    for entry in "${LICENSE_FILES[@]}"; do
      printf '\n%s\n\n%s\n\n' "$rule" "${entry%%|*}"
      cat "$ROOT/${entry#*|}"
    done
  } > "$out"
  rm -rf "$tmp"
}

step "Building native extensions from source"
build_native() { scons -C "$ROOT/native" -j"$JOBS" platform="$1" target="$2"; }
# The headless editor below loads the host (Linux) debug libraries while it
# imports and exports scenes that use the extension classes.
build_native linux template_debug
if [[ "$MODE" == release ]]; then
  build_native "$PLATFORM" template_release
fi
if [[ "$PLATFORM" != linux ]] && { [[ "$MODE" == debug ]] || ((EDITOR_LIBS)); }; then
  build_native "$PLATFORM" template_debug
fi

step "Importing project"
# --import exits once the import finishes; a non-zero exit here is a real failure.
"$GODOT" --headless --path "$ROOT" --import

step "Exporting $PLATFORM $MODE build"
SUFFIX=""
[[ "$MODE" == debug ]] && SUFFIX="-debug"
OUT="$ROOT/dist/$PLATFORM$SUFFIX"
ARCHIVE="$ROOT/dist/PNGTube-Remix-${PLATFORM}-x86_64$SUFFIX.zip"
rm -rf "$OUT" "$ARCHIVE" && mkdir -p "$OUT"
touch "$ROOT/dist/.gdignore"  # keep Godot from importing build output
if [[ "$PLATFORM" == windows ]]; then
  PRESET="Windows Desktop"; BINARY="PNGTube-Remix.exe"
else
  PRESET="Linux/X11"; BINARY="PNGTube-Remix.x86_64"
fi
"$GODOT" --headless --path "$ROOT" "--export-$MODE" "$PRESET" "$OUT/$BINARY"
git -C "$ROOT" describe --tags --exclude nightly --always --dirty > "$OUT/version.txt"
cp "$ROOT/LICENSE" "$OUT/LICENSE.txt"
cp "$ROOT/THIRD-PARTY-NOTICES.md" "$OUT/THIRD-PARTY-NOTICES.md"
write_licenses "$OUT/THIRD-PARTY-LICENSES.txt"

(cd "$OUT" && python3 -m zipfile -c "$ARCHIVE" .)

step "Done"
ls -l "$OUT"
echo "Archive: $ARCHIVE"
