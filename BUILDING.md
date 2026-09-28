# Building PNGTube-Remix

Everything the app needs is either in this repo or pinned by it:

| Input | Pinned at |
|---|---|
| Godot editor + export templates 4.7.2 (official prebuilt) | SHA-512 sums in `tools/build.sh` |
| godot-cpp 10.0.0-stable (API 4.7) | `native/godot-cpp` submodule |
| godot-gif 1.1.1 | `native/godot-gif` submodule |
| Global input, mic input, mesh deform plugins | Source vendored in `native/` (origins in [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md)) |
| App (GDScript, scenes, assets) | This repo |

No compiled binaries are committed. `tools/build.sh` produces all of them, and
the plugin DLLs come out byte-identical wherever and whenever they're built.

## Option 1: let GitHub build it

Every push to `main` runs [`.github/workflows/build.yml`](.github/workflows/build.yml):

1. **build-windows** (Ubuntu):
   - checks that no prebuilt binaries are committed
   - builds the release and debug Windows exports with `tools/build.sh`
   - runs `tools/smoke_test.sh`, which loads every demo model
2. **smoke-test-windows** (real Windows): boots both exports headless for 600
   frames. It fails on a crash, a plugin DLL that won't load, a missing node, or
   (in the debug build) any GDScript error.
3. **release** (only for a published release or a pushed `v*` tag): attaches the
   zip to the GitHub Release.

Download the result from the run's **Artifacts**. Artifacts are kept for 3 days
to stay within GitHub's storage quota. For a build you want to keep, publish a
release: Releases, Draft a new release, choose a new tag such as `v1.4.7-r1`,
Publish. (Pushing a `v*` tag does the same.) The zip appears under its Assets.

## Option 2: build locally

Use a Linux x86_64 host: Ubuntu 24.04, or **WSL2 on Windows** (`wsl --install -d Ubuntu-24.04`).

```bash
sudo apt-get update
sudo apt-get install -y git curl unzip python3 g++ mingw-w64 scons

git clone <this repo> pngtuber-remix && cd pngtuber-remix
tools/build.sh
```

Output:

- `dist/windows/`: `PNGTube-Remix.exe`, `.pck`, the four plugin DLLs, and the
  license files (`LICENSE.txt`, `THIRD-PARTY-NOTICES.md`, `THIRD-PARTY-LICENSES.txt`)
- `dist/PNGTube-Remix-windows-x86_64.zip`: the same, zipped

The first run downloads Godot (~1.4 GB, into `.tools/`, verified against the
pinned hashes) and compiles godot-cpp, which takes 10–15 minutes. Later runs
reuse both and take about a minute.

Other options:

- `tools/build.sh linux` builds the Linux version into `dist/linux/`.
- `tools/build.sh windows --debug` builds a debug export into
  `dist/windows-debug/`. It prints GDScript errors that a release build hides.
- `tools/smoke_test.sh` (after a build) boots the app headless, loads every
  bundled demo model, and checks each one against upstream's result.

## Opening the project in the Godot editor on Windows

The editor loads the *debug* plugin DLLs, which a normal build skips. Clone into
a folder Windows can reach and build them once from WSL:

```bash
git clone <this repo> /mnt/c/Users/<you>/pngtuber-remix
cd /mnt/c/Users/<you>/pngtuber-remix
tools/build.sh windows --editor-libs
```

Then open `C:\Users\<you>\pngtuber-remix` with the
[Godot 4.7.2 Windows editor](https://github.com/godotengine/godot/releases/tag/4.7.2-stable).
It re-imports the assets the first time, which is normal. Builds under `/mnt/c`
are slower than in the WSL home folder.

## Building just the native plugins

```bash
git submodule update --init native/godot-cpp native/godot-gif   # once
cd native
scons platform=windows target=template_release   # for exported builds
scons platform=windows target=template_debug     # for the editor
```

`native/SConstruct` writes each library into the `addons/` folder its
`.gdextension` file points at, and always relinks them.

## Pulling in upstream updates

This repo keeps upstream's full history, so updates merge normally:

```bash
git remote add upstream https://github.com/MudkipWorld/PNGTuber-Remix.git
git fetch upstream
git merge upstream/1.4.x
```

After merging:

- **Upstream committed new prebuilt plugin binaries:** remove them with
  `git rm` (CI refuses to build while any are committed). The next build
  overwrites any that are still on disk, because the plugins are always relinked
  from source.
- **Upstream changed a plugin's C++:** copy the new source from
  [Godot-Global-Input](https://github.com/MudkipWorld/Godot-Global-Input) or
  [GDExtensionsPlayGround](https://github.com/MudkipWorld/GDExtensionsPlayGround)
  into `native/`, then re-apply this fork's include fixes and record the new
  commit in THIRD-PARTY-NOTICES.md. A three-way apply skips fixes the new source
  already has; resolve any conflict markers by keeping every `#include`:
  ```bash
  git add native/global_input native/miniaudio_mic
  git diff 5b80834 3c87a9c -- native/global_input native/miniaudio_mic | git apply --3way
  ```
- **Upstream touched theme code:** keep this fork's version, which has no themes.
- **Upstream changed the demo models:** `tools/smoke_test.sh` will fail. Once
  you're happy the new models load correctly, update the expected values in
  `tools/smoke_test.gd`.

## Bumping Godot

1. In `tools/build.sh`, update `GODOT_VERSION` and the two SHA-512 sums (from
   the release's `SHA512-SUMS.txt`).
2. Replace `licenses/godot-<version>/` with `LICENSE.txt` and `COPYRIGHT.txt`
   from the new `<version>-stable` tag. The build stops if they're missing.
3. In `tools/mesa_notice.sh`, set `NIR_TAG` to the `mesa_version` in that tag's
   `misc/scripts/install_d3d12_sdk_windows.py`, and push. The "Regenerate Mesa
   notice" workflow runs and pushes the new `licenses/mesa.txt` to the
   `mesa-notice` branch; merge it.
4. If godot-cpp supports the new version's API (see `supported_api_versions` in
   `native/godot-cpp/tools/godotcpp.py`), raise the `api_version` default in
   `native/SConstruct` and `compatibility_minimum` in the four `.gdextension`
   files to match.
