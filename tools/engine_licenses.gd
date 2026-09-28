# Writes Godot's license, and the copyright and license text of every
# third-party component compiled into the engine, to the path given after "--".
# tools/build.sh runs it in a throwaway project so the app itself isn't loaded:
#   godot --headless --path <empty project> -s res://engine_licenses.gd -- <out>
extends SceneTree


func _init() -> void:
	var sections := PackedStringArray()
	sections.append("Godot Engine %s\n\n%s" % [Engine.get_version_info().string, Engine.get_license_text()])

	var credits := PackedStringArray(["Components bundled in the Godot Engine binary:"])
	for component in Engine.get_copyright_info():
		for part in component["parts"]:
			credits.append("- %s\n  Copyright: %s\n  License: %s" % [
				component["name"], "\n             ".join(part["copyright"]), part["license"]])
	sections.append("\n".join(credits))

	var texts: Dictionary = Engine.get_license_info()
	var ids := texts.keys()
	ids.sort()
	for id in ids:
		sections.append("License: %s\n\n%s" % [id, texts[id]])

	var out := FileAccess.open(OS.get_cmdline_user_args()[0], FileAccess.WRITE)
	out.store_string(("\n\n" + "=".repeat(79) + "\n\n").join(sections) + "\n")
	out.close()
	quit()
