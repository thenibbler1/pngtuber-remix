#!/usr/bin/env bash
# Regenerates licenses/mesa.txt: the notices of the Mesa code (NIR, SPIR-V and
# DXIL compilers) that Godot's official Windows export templates contain for
# the D3D12 renderer. Godot's COPYRIGHT.txt doesn't cover it.
#
# Needs git, python3 with mako, and access to gitlab.freedesktop.org, so it's
# normally run by the "Regenerate Mesa notice" workflow. Rerun it after bumping
# GODOT_VERSION: set NIR_TAG to the godot-nir-static version (mesa_version) in
# that Godot release's misc/scripts/install_d3d12_sdk_windows.py.
set -euo pipefail

NIR_TAG="25.3.1-3"  # Godot 4.7.2

ROOT="$(CDPATH='' cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

git clone --quiet --depth 1 --branch "$NIR_TAG" --recurse-submodules --shallow-submodules \
  https://github.com/godotengine/godot-nir-static.git "$WORK/nir"
# update_mesa.sh copies exactly the Mesa files godot-nir-static compiles, then
# generates the rest, into godot-mesa/.
(cd "$WORK/nir" && bash ./update_mesa.sh >/dev/null)
python3 "$ROOT/tools/mesa_notice.py" "$WORK/nir" "$NIR_TAG" > "$ROOT/licenses/mesa.txt"
echo "Wrote licenses/mesa.txt ($(wc -l < "$ROOT/licenses/mesa.txt") lines)"
