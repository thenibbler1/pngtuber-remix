# PNGTube-Remix (reproducible build)

A fork of [MudkipWorld/PNGTuber-Remix](https://github.com/MudkipWorld/PNGTuber-Remix)
(version 1.4.7, upstream commit `12c662b`), used with the developer's permission.
It's the same PNGTubing app with every feature, but anyone can rebuild it from
source with one command, from pinned inputs.

## What's different from upstream

| | Upstream | This fork |
|---|---|---|
| Native plugins (global hotkeys, mic input, mesh deform, GIF import) | Prebuilt `.dll`/`.so` files committed to the repo; sources live in other repos and don't build against current godot-cpp | Built from source in [`native/`](native/) against a pinned godot-cpp, by one command |
| Build | Hand-built binaries plus a CI export | `tools/build.sh` pins Godot 4.7.2 by SHA-512 and does everything; CI runs the same script and boots the result on real Windows |
| MinGW runtime DLLs | Shipped loose next to the exe | Linked statically, so none needed |
| UI themes | 8 color skins plus a picker in Settings | Removed; the app uses the plain default Godot look (upstream's "None" option) |
| Platforms | Windows, Linux | Windows (Linux still builds and is used for testing) |

Everything else (models, rigging, appendages, meshes, PSD/GIF/APNG import,
WebSocket and tracking, lip sync, hotkeys, stream mode) is upstream code,
unchanged.

## Download

- **Latest build:** Actions tab → latest green **Build** run → artifact
  `PNGTube-Remix-windows-x86_64`.
- **Releases:** pushing a tag like `v1.4.7-r1` publishes a zip on the Releases page.

Unzip it anywhere and run `PNGTube-Remix.exe`. Keep the `.pck` and `.dll` files
next to the exe.

## Build it yourself

See [BUILDING.md](BUILDING.md). Short version, on Ubuntu 24.04 or WSL2:

```bash
sudo apt-get install -y git curl unzip g++ mingw-w64 python3-pip && pip install scons
git clone <this repo> pngtuber-remix && cd pngtuber-remix
tools/build.sh            # -> dist/PNGTube-Remix-windows-x86_64.zip
```

## License

PNGTuber Remix is © MudkipWorld under a custom license: see [LICENSE](LICENSE).
Commercial use or redistribution needs the copyright holder's prior written
permission. Third-party components are listed in
[THIRD-PARTY-NOTICES.md](THIRD-PARTY-NOTICES.md).

## Credits

Made by [MudkipWorld](https://github.com/MudkipWorld) ([docs](https://mudkipworld.github.io/PNGRemix-Doc/#/),
[Discord](https://discord.gg/un3JBEvYNR)). Originally a fork of PNGTuber+ by Kaiakairos.
Upstream thanks General Tekno, Guuvita, LeoRson, the Godot project, Pixelorama
(PSD import), vj4 (WebSocket implementation) and Mushie (documentation).
