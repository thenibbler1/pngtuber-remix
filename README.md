# PNGTube-Remix (reproducible build)

A fork of [MudkipWorld/PNGTuber-Remix](https://github.com/MudkipWorld/PNGTuber-Remix)
(version 1.4.7, upstream commit `12c662b`), used with the developer's permission.
It's the same PNGTubing app with every feature, but anyone can rebuild it from
source with one command, from pinned inputs.

## What's different from upstream

| | Upstream | This fork |
|---|---|---|
| Native plugins (global hotkeys, mic input, mesh deform, GIF import) | Prebuilt `.dll`/`.so` files committed to the repo; sources live in other repos and don't build against current godot-cpp | Built from source in [`native/`](native/) against a pinned godot-cpp, by one command |
| Build | Hand-built binaries plus a CI export | `tools/build.sh` pins Godot 4.7.2 by SHA-512 and does everything; the plugin DLLs come out byte-identical on every rebuild. CI runs the same script, loads every demo model, and boots release and debug builds on real Windows |
| MinGW runtime DLLs | Shipped loose next to the exe | Linked statically, so none needed |
| License texts | Not included in the download | `THIRD-PARTY-LICENSES.txt` in every build, covering Godot and every bundled component |
| UI themes | 8 color skins plus a picker in Settings | Removed. Every window uses Godot's plain default look. (Upstream's "None" option came close but still left the White skin on the Switch Session and Grid Snap Size dialogs.) |
| Platforms | Windows, Linux | Windows (Linux still builds and is used for testing) |

Everything else (models, rigging, appendages, meshes, PSD/GIF/APNG import,
WebSocket and tracking, lip sync, hotkeys, stream mode) is upstream code,
unchanged.

## Download

- **Releases:** publish a release on the Releases page (Draft a new release, new tag like
  `v1.4.7-r1`, Publish). CI builds and tests it, then attaches the zip in about 10 minutes.
  This is the copy to keep.
- **Latest build:** Actions tab → latest green **Build** run → artifact
  `PNGTube-Remix-windows-x86_64` (kept for 3 days).

Unzip it anywhere and run `PNGTube-Remix.exe`. Keep the `.pck` and `.dll` files
next to the exe.

## Build it yourself

See [BUILDING.md](BUILDING.md). Short version, on Ubuntu 24.04 or WSL2:

```bash
sudo apt-get update && sudo apt-get install -y git curl unzip python3 g++ mingw-w64 scons
git clone <this repo> pngtuber-remix && cd pngtuber-remix
tools/build.sh            # -> dist/PNGTube-Remix-windows-x86_64.zip
```

## License

PNGTuber Remix is © MudkipWorld under a custom license; [LICENSE](LICENSE) is
the authority. In short: commercial use (selling, sublicensing, or distributing
it as part of a commercial product or service) needs the copyright holder's
prior written permission, and using it in websites as part of a service needs
their prior permission. Selling commissions made with the app is explicitly
allowed.
Third-party components are listed in [THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).

## Credits

Made by [MudkipWorld](https://github.com/MudkipWorld) ([docs](https://mudkipworld.github.io/PNGRemix-Doc/#/),
[Discord](https://discord.gg/un3JBEvYNR)). Originally a fork of PNGTuber+ by Kaiakairos.
Upstream thanks General Tekno, Guuvita, LeoRson, the Godot project, Pixelorama
(PSD import), vj4 (WebSocket implementation) and Mushie (documentation).
