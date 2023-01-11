###
	Define a text display module
	SDF fonts can be created from .ttf files with:  
	http://kvazars.com/littera/
###

_ = require 'underscore'
loadFont = require 'load-bmfont'
THREE = require 'three'
createText = require './text-mesh'
# wgs84 = require '../../core/wgs84'

textShader = require './text-fs.glsl'


referenceSphere = new THREE.Sphere(new THREE.Vector3(0,0,0), wgs84.earthRadius)


fonts = {
	"Relative Medium": {
		font: '/assets/fonts/Relative-Medium-32.fnt'
		image: '/assets/fonts/Relative-Medium-32.png'
		fontSmoothness: 1.0
	}
	"Relative Bold": {
		font: '/assets/fonts/Relative-Bold-32.fnt'
		image: '/assets/fonts/Relative-Bold-32.png'
		fontSmoothness: 0.5  # lower number needed for thicker font, otherwise looks blurry
	}
	"Relative Book": {
		font: '/assets/fonts/Relative-Book-32.fnt'
		image: '/assets/fonts/Relative-Book-32.png'
		fontSmoothness: 1.0
	}
}


window.fontCache = fonts

loadFontTexture = (loader, options, callback) ->
	loadFont options.font, (err, font) ->
		console.log("FONT !!!", font)
		if (err)
			throw err
		loader.load(options.image, (tex) -> callback(font, tex))


###
	Load the fonts we need on initialization.

###
loadSystemFonts = (canvas) ->
	_.each fonts, (fontObject) ->
		loadFontTexture canvas.threeTextureLoader, fontObject, (font, texture) ->
			maxAni = canvas.renderer.getMaxAnisotropy()

			# otherwise we get errors if this isn't defined
			font.common or= {}

			#setup our texture with some nice mipmapping etc
			texture.needsUpdate = true
			texture.minFilter = THREE.LinearFilter #THREE.LinearMipMapLinearFilter
			texture.magFilter = THREE.LinearFilter
			texture.generateMipmaps = true
			texture.anisotropy = maxAni

			fontObject.font = font
			fontObject.texture = texture
			fontObject.loaded = true


# alternative shader
vertexShader = '''
varying vec2 vUv;
void main() {
	vUv = uv;
	gl_Position = projectionMatrix * modelViewMatrix * vec4( position.xyz, 1.0 );
}
'''

fragment2 = '''
precision mediump float;

uniform sampler2D u_texture;
uniform vec4 u_color;
uniform float u_buffer;
uniform float u_gamma;

varying vec2 v_texcoord;

void main() {
    float dist = texture2D(u_texture, v_texcoord).r;
    float alpha = smoothstep(u_buffer - u_gamma, u_buffer + u_gamma, dist);
    gl_FragColor = vec4(u_color.rgb, alpha * u_color.a);
}
'''




###
	Handle loading and caching fonts.

	note: I've disabled this approach to go for a synchronous solution
	that has fonts loaded at initalization
###
fontProvider = (fontName, options, callback) ->
	if _.isFunction(options)
		callback = options
		options = {}

	options.size or= '32'

	if fonts[fontName] and fonts[fontName][options.size]
		fontObject = fonts[fontName][options.size]

		if fontObject.texture
			callback(fontObject.font, texture)
		else if fontObject.promise
			fontObject.promise.done(callback)
		else
			loadFont options.font, (err, font) ->
				if (err)
					throw err

				fontObject.font = font
				THREE.ImageUtils.loadTexture options.image, undefined, (tex) -> 
					fontObject.texture = tex
					callback(font, tex, options)




###
	Create a billboard facing the scene camera and return

	Needs a reference to the scene to reference the camera and add
	to scene
###
createBillboard = (scene, text, location, options) ->
	options or= {}
	defaults =
		font: 'Relative Bold'
		scale: 1.0
		radiusScale: 1.0
		color: 'rgb(255, 255, 255)'
		borderColor: 'rgb(20, 20, 20)'
		border: true

	options = _.extend {}, defaults, options

	fontObject = fonts[options.font]
	console.log "FONT OBJECT ", options.font, fontObject
	return false unless fontObject
	return false unless fontObject.loaded

	options.fontSmoothness or= fontObject.fontSmoothness

	geom = createText({
		text: text or "",
		font: fontObject.font
		# lineHeight: 10,
		# width: 1000   # word-wrap width
	})

	useBorder = undefined
	useBorder = "true" if options.border

	material = new THREE.ShaderMaterial({
		side: THREE.DoubleSide,
		transparent: true,
		depthTest: false,
		uniforms: {
			opacity: { type: 'f', value: 1.0 },
			smooth: { type: 'f', value: 1/8 * (1/options.scale) * options.fontSmoothness },
			map: { type: 't', value: fontObject.texture },
			color: { type: 'c', value: new THREE.Color(options.color) },
			borderColor: { type: 'c', value: new THREE.Color(options.borderColor) },
			borderAlpha: { type: 'f', value: 1.0 }
		},
		vertexShader: vertexShader,
		fragmentShader: textShader,
		defines: {
			"USE_MAP": "",
			"ALPHATEST": "0.06",
			"USE_BORDER": useBorder
		}
	})

	layout = geom.layout
	text = new THREE.Mesh(geom, material)

	text.position.x = -layout.width/2
	text.position.y = layout.height * 1.75  
	text.rotation.x = Math.PI

	#scale it down so it fits in our 3D units
	textAnchor = new THREE.Object3D()
	window.textAnchor = textAnchor # debug
	textAnchor.scale.multiplyScalar(5000.0 * options.scale)
	textAnchor.position.copy(location)

	camera = scene.cameraController.camera
	# cameraPos = cameraPos.sub(textAnchor.position).normalize()
	textAnchor.quaternion.copy(camera.quaternion)
	textAnchor.add(text)

	scene.threeScene.add(textAnchor)
	window.textParent = textAnchor
	return textAnchor





setupTest = (font, texture, options) ->
	options or= {}

	options.scale or= 1.0
	options.radiusScale or= 1.0
	options.color or= 'rgb(20, 20, 20)'
	options.borderColor or= 'rgb(255, 255, 255)'
	# options.location or= new THREE.Vector3(0.0, 0.0, 0.0) 

	# each font has a different smoothing value that works for it
	# if bold we multiply smoothness by 0.6
	options.fontSmoothness or= 0.6

	# copy = "Hello World"

	maxAni = window.GC.renderer.getMaxAnisotropy()

	#setup our texture with some nice mipmapping etc
	texture.needsUpdate = true
	texture.minFilter = THREE.LinearFilter #THREE.LinearMipMapLinearFilter
	texture.magFilter = THREE.LinearFilter
	texture.generateMipmaps = true
	texture.anisotropy = maxAni

	# otherwise we get errors if this isn't defined
	font.common or= {}

	geom = createText({
		text: options.text or "",
		font: font,
		# lineHeight: 10,
		# width: 1000   # word-wrap width
	})

	material = new THREE.ShaderMaterial({
		side: THREE.DoubleSide,
		transparent: true,
		depthTest: false,
		uniforms: {
			opacity: { type: 'f', value: 1.0 },
			smooth: { type: 'f', value: 1/8 * (1/options.scale) * options.fontSmoothness },
			map: { type: 't', value: texture },
			color: { type: 'c', value: new THREE.Color(options.color) },
			borderColor: { type: 'c', value: new THREE.Color(options.borderColor) },
			borderAlpha: { type: 'f', value: 1.0 }
		},
		vertexShader: vertexShader,
		fragmentShader: textShader,
		defines: {
			"USE_MAP": "",
			"ALPHATEST": "0.06",
			"USE_BORDER": ""
		}
	})

	layout = geom.layout
	text = new THREE.Mesh(geom, material)

	text.position.x = -layout.width/2
	text.position.y = layout.height * 1.45  

	text.rotation.x = Math.PI
	# text.rotation.z = Math.PI/2

	#scale it down so it fits in our 3D units
	textAnchor = new THREE.Object3D()
	window.textAnchor = textAnchor # debug
	textAnchor.scale.multiplyScalar(5000.0 * options.scale)
	textAnchor.position.y = 6371007 * options.radiusScale
	if options.location
		textAnchor.position.copy(options.location)

	camera = window.GC.scene.cameraController.camera
	# cameraPos = cameraPos.sub(textAnchor.position).normalize()
	textAnchor.quaternion.copy(camera.quaternion)
	# textAnchor.rotation.x = Math.PI/2 
	# textAnchor.rotation.y = 2.0
	# textAnchor.rotation.z = 1.37

	if options.upright
		textAnchor.rotation.x = Math.PI
	textAnchor.add(text)

	window.GC.scene.threeScene.add(textAnchor)
	window.textParent = textAnchor
	return textAnchor







# for debug
# window.initFontTexture = init


module.exports =
	loadFontTexture: loadFontTexture
	setupTest: setupTest
	createBillboard: createBillboard
	loadSystemFonts: loadSystemFonts
