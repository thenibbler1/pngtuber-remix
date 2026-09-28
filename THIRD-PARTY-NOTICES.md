# Third-party notices

PNGTuber Remix itself is © MudkipWorld and distributed under the custom
license in [`LICENSE`](LICENSE). It ships with or builds the components below.
Every build (`tools/build.sh`) includes `THIRD-PARTY-LICENSES.txt` with the
full license text of each one, plus the notices for the libraries compiled
into Godot itself.

## Downloaded by `tools/build.sh` (official prebuilt, SHA-512 pinned)

| Component | Where | Source | License |
|---|---|---|---|
| Godot Engine 4.7.2 editor and export templates | the shipped `.exe` is Godot's official Windows template | https://github.com/godotengine/godot/releases/tag/4.7.2-stable | MIT, plus the licenses of the libraries bundled in the engine |

## Built from source by `tools/build.sh`

| Component | Where | Source | License |
|---|---|---|---|
| godot-cpp 10.0.0 | `native/godot-cpp` (submodule), linked into every plugin DLL | https://github.com/godotengine/godot-cpp | MIT |
| godot-gif 1.1.1 | `native/godot-gif` (submodule) | https://github.com/BOTLANNER/godot-gif | MIT; bundles giflib (MIT) and code from Goost (MIT) |
| Godot-Global-Input | `native/global_input` | https://github.com/MudkipWorld/Godot-Global-Input @ `b4d0de1`, plus include fixes (see below) | Unlicense (public domain), see `native/global_input/LICENSE` |
| MiniAudioMic | `native/miniaudio_mic` | https://github.com/MudkipWorld/GDExtensionsPlayGround @ `aae55de`, plus include fixes (see below) | © MudkipWorld, no separate license published; used under the Remix license with the author's permission |
| CustomMeshDeform | `native/custom_mesh` | https://github.com/MudkipWorld/GDExtensionsPlayGround @ `aae55de` | © MudkipWorld, no separate license published; used under the Remix license with the author's permission |
| miniaudio | `native/miniaudio_mic/miniaudio.*` | https://github.com/mackron/miniaudio | Public domain (Unlicense) or MIT-0 |
| kiss_fft | `native/miniaudio_mic/kiss_fft*` | https://github.com/mborgerding/kissfft | BSD-3-Clause |

The vendored MudkipWorld sources are unmodified apart from missing standard
`#include`s (`<vector>` in `mini_audio.h`, `<cstdio>` and `<string>` in
`trackers/linux/x11_global_input.h`) that current godot-cpp no longer pulls in
for them. `git diff 5b80834 3c87a9c -- native/global_input native/miniaudio_mic`
shows the exact patch.

The plugins built from these sources expose the same classes, methods,
properties and signals as upstream's prebuilt Windows DLLs. Upstream's prebuilt
*Linux* GlobalInput library also has three methods from a newer, unpublished
revision (`get_hook_mouse_position`, `get_virtual_mouse_position`,
`update_virtual_mouse`). The app never calls them.

## Bundled in the project

| Component | Where | License |
|---|---|---|
| aimg_io (APNG import/export) | `addons/aimg_io` | Unlicense, see `COPYING.txt` |
| WigglyAppendage2D (Tameno) | `addons/wiggly_appendage_2d` | Unlicense |
| Godot lip sync (Malcolm Nixon) | `UI/Lipsync stuff/godot-lip-sync` | MIT |
| PSD import, adapted from Pixelorama | `Scripts/Misc/PSD_parser.gd` | MIT |
| Open Sans | `Scripts/Fonts/OpenSans-Medium.ttf` | SIL Open Font License 1.1 |

License texts for components that don't carry their own file in this repo are
in [`licenses/`](licenses/), with where each was taken from.
