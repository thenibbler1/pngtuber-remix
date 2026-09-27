# Building PNGTube-Remix

Everything the app needs is either in this repo or pinned by it:

| Input | Pinned at |
|---|---|
| Godot editor + export templates 4.7.2 | SHA-512 sums in `tools/build.sh` |
| godot-cpp 10.0.0-stable (API 4.7) | `native/godot-cpp` submodule |
| godot-gif 1.1.1 | `native/godot-gif` submodule |
| Global input, mic input, mesh deform plugins | Source vendored in `native/` (origins in [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md)) |
| App (GDScript, scenes, assets) | This repo |

No compiled binaries are committed. `tools/build.sh` produces all of them.

## Option 1: let GitHub build it

Every push to `main` runs [`.github/workflows/build.yml`](.github/workflows/build.yml):

1. **build-windows** (Ubuntu): runs `tools/build.sh windows`.
2. **smoke-test-windows** (real Windows): boots the exported exe headless for
   600 frames. It fails on a crash, a DLL that won't load, or any script error.
3. **release** (only for `v*` tags): attaches the zip to a GitHub Release.

Download the result from the run's **Artifacts**.

## Option 2: build locally

Use a Linux x86_64 host: Ubuntu 24.04, or **WSL2 on Windows** (`wsl --install -d Ubuntu-24.04`).

```bash
sudo apt-get update
sudo apt-get install -y git curl unzip g++ mingw-w64 python3-pip
pip install scons

git clone <this repo> pngtuber-remix && cd pngtuber-remix
tools/build.sh
```

Output:

- `dist/windows/`: `PNGTube-Remix.exe`, `.pck`, and the four plugin DLLs
- `dist/PNGTube-Remix-windows-x86_64.zip`: the same, zipped

The first run downloads Godot (~1.4 GB, into `.tools/`, verified against the
pinned hashes) and compiles godot-cpp, which takes about 5–10 minutes. Later runs
reuse both and take about a minute.

`tools/build.sh linux` builds the Linux version instead.

## Opening the project in the Godot editor on Windows

The editor loads the *debug* plugin DLLs, which a normal build skips. Build them
once from WSL, inside a folder Windows can reach:

```bash
cd /mnt/c/Users/<you>/pngtuber-remix
tools/build.sh windows --editor-libs
```

Then open that folder with the [Godot 4.7.2 Windows editor](https://github.com/godotengine/godot/releases/tag/4.7.2-stable).

## Building just the native plugins

```bash
cd native
scons platform=windows target=template_release   # for exported builds
scons platform=windows target=template_debug     # for the editor
```

`native/SConstruct` writes each library into the `addons/` folder its
`.gdextension` file points at.

## Pulling in upstream updates

This repo keeps upstream's full history, so updates merge normally:

```bash
git remote add upstream https://github.com/MudkipWorld/PNGTuber-Remix.git
git fetch upstream
git merge upstream/1.4.x
```

After merging:

- **Upstream commits new prebuilt plugin binaries:** delete them again
  (`git rm --cached` under `addons/*/windows`, `addons/*/linux`, `addons/godotgif/bin`).
- **Upstream changes a plugin's C++:** copy the new source from
  [Godot-Global-Input](https://github.com/MudkipWorld/Godot-Global-Input) or
  [GDExtensionsPlayGround](https://github.com/MudkipWorld/GDExtensionsPlayGround)
  into `native/`, and note the commit in THIRD-PARTY-NOTICES.md.
- **Upstream touches theme code:** keep this fork's version, which has no themes.
