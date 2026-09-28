extends Node
# Builds the model view's graphics pipelines at startup.
#
# Godot builds a 2D pipeline the first time a shader is drawn, and on Direct3D 12
# that briefly freezes the app: selecting a layer for the first time stalled while
# the outline and handle shaders were built. Drawing everything once here, off
# screen, moves that one-time cost to startup, where it happens during loading.
#
# A 2D pipeline is keyed by shader, render target format, draw type (rectangle or
# polygon) and whether a light touches the item. So each shader is drawn as a
# sprite and as a textured line (appendages), both lit and unlit, into viewports
# with the same format as the model view (Main/main.tscn: SubViewport defaults).

# Pipelines belong to the shader, so these stay loaded for the app's lifetime;
# the object scenes get these same instances from the resource cache.
const SHADERS := [
	preload("res://Scripts/Shaders/SpriteShader.gdshader"),
	preload("res://Scripts/Shaders/SpriteSelection.gdshader"),
	preload("res://Scripts/Shaders/SpriteSelectionWA.gdshader"),
	null, # Godot's default 2D shader (origin and pointer markers)
]


func _ready() -> void:
	var image := Image.create(8, 8, false, Image.FORMAT_RGBA8)
	image.fill(Color.WHITE)
	var texture := CanvasTexture.new()
	texture.diffuse_texture = ImageTexture.create_from_image(image)

	var viewports : Array[SubViewport] = []
	for lit in [false, true]:
		var viewport := SubViewport.new()
		viewport.size = Vector2i(64, 64)
		viewport.transparent_bg = true
		viewport.render_target_update_mode = SubViewport.UPDATE_ONCE
		if lit:
			viewport.add_child(DirectionalLight2D.new())
		for shader in SHADERS:
			var material : ShaderMaterial = null
			if shader:
				material = ShaderMaterial.new()
				material.shader = shader
			viewport.add_child(_sprite(texture, material))
			var line := Line2D.new()
			line.points = PackedVector2Array([Vector2(8, 8), Vector2(56, 56)])
			line.texture = texture
			line.texture_mode = Line2D.LINE_TEXTURE_STRETCH
			line.material = material
			viewport.add_child(line)
		# Layers with Clip Children set draw through a canvas group.
		var clip_material := ShaderMaterial.new()
		clip_material.shader = SHADERS[0]
		var clip_parent := _sprite(texture, clip_material)
		clip_parent.clip_children = CanvasItem.CLIP_CHILDREN_AND_DRAW
		clip_parent.add_child(_sprite(texture, clip_material))
		viewport.add_child(clip_parent)
		# Comment objects draw text.
		var label := Label.new()
		label.text = "Aa"
		viewport.add_child(label)
		add_child(viewport)
		viewports.append(viewport)

	# The viewports render during the next frame's draw.
	await RenderingServer.frame_post_draw
	await RenderingServer.frame_post_draw
	for viewport in viewports:
		viewport.queue_free()


func _sprite(texture: Texture2D, material: Material) -> Sprite2D:
	var sprite := Sprite2D.new()
	sprite.texture = texture
	sprite.material = material
	sprite.position = Vector2(32, 32)
	return sprite
