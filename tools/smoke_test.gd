# Regression check run by tools/smoke_test.sh, which injects this as an autoload.
# Boots the app normally, loads every bundled demo model, and checks each one
# builds the same sprite tree as upstream PNGTuber Remix 1.4.7 (sprite count and
# a hash of the sorted sprite names). Exits with the number of mismatches.
extends Node

const EXPECTED := {
	"PickleModel": [39, "32a7b6ce95079b22a70915b2e0c98a26"],
	"PickleModelAssets": [46, "e1e694037f4816bbb685b63efd2849f9"],
	"PickleModelFollowMouse": [20, "c15ee865e7305666c6b7a2d004865989"],
	"PickleModelWithNormalMap": [20, "94748dbd69167ed32d708217891f906c"],
	"PicklesModelJoypad": [34, "3bc6902ea7f40cb74f39f576a71c89fe"],
}
const FRAMES_PER_MODEL := 120


func _ready() -> void:
	# Give the app's own startup (Settings, main scene) time to finish.
	await get_tree().create_timer(1.0).timeout
	var failures := 0
	for model in EXPECTED:
		SaveAndLoad.load_file("res://DemoModels/%s.pngRemix" % model)
		for i in FRAMES_PER_MODEL:
			await get_tree().process_frame
		var names := PackedStringArray()
		for sprite in get_tree().get_nodes_in_group("Sprites"):
			names.append(str(sprite.get("sprite_name")))
		names.sort()
		var got := [names.size(), ",".join(names).md5_text()]
		var ok: bool = got == EXPECTED[model]
		print("SMOKE %s %s sprites=%d names_md5=%s" % ["ok  " if ok else "FAIL", model, got[0], got[1]])
		if not ok:
			failures += 1
	print("SMOKE done: %d of %d models mismatched" % [failures, EXPECTED.size()])
	get_tree().quit(failures)
