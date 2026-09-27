# Third-party notices

PNGTuber Remix itself is © MudkipWorld and distributed under the custom
license in [`LICENSE`](LICENSE). It bundles or builds the components below.

## Built from source by `tools/build.sh`

| Component | Where | Source | License |
|---|---|---|---|
| Godot Engine 4.7.2 (export templates) | shipped `.exe` | https://github.com/godotengine/godot | MIT |
| godot-cpp 10.0.0 | `native/godot-cpp` (submodule) | https://github.com/godotengine/godot-cpp | MIT |
| godot-gif 1.1.1 | `native/godot-gif` (submodule) | https://github.com/BOTLANNER/godot-gif | MIT (bundles giflib, MIT) |
| Godot-Global-Input | `native/global_input` | https://github.com/MudkipWorld/Godot-Global-Input @ `b4d0de1` | Unlicense (public domain), see `native/global_input/LICENSE` |
| MiniAudioMic | `native/miniaudio_mic` | https://github.com/MudkipWorld/GDExtensionsPlayGround @ `aae55de` | © MudkipWorld, no separate license published; used under the Remix license with the author's permission |
| CustomMeshDeform | `native/custom_mesh` | https://github.com/MudkipWorld/GDExtensionsPlayGround @ `aae55de` | © MudkipWorld, no separate license published; used under the Remix license with the author's permission |
| miniaudio | `native/miniaudio_mic/miniaudio.*` | https://github.com/mackron/miniaudio | Public domain or MIT-0 |
| kiss_fft | `native/miniaudio_mic/kiss_fft*` | https://github.com/mborgerding/kissfft | BSD-3-Clause |

## Bundled in the project

| Component | Where | License |
|---|---|---|
| aimg_io (APNG import/export) | `addons/aimg_io` | Unlicense, see `COPYING.txt` |
| WigglyAppendage2D (Tameno, Aaron Franke) | `addons/wiggly_appendage_2d` | MIT |
| Godot lip sync (Malcolm Nixon) | `UI/Lipsync stuff/godot-lip-sync` | MIT |
| PSD import, adapted from Pixelorama | `Scripts/Misc/PSD_parser.gd` | MIT |
| Open Sans | `Scripts/Fonts/OpenSans-Medium.ttf` | SIL Open Font License 1.1 |
