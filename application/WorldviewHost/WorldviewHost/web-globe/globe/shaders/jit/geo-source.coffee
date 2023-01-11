_ = require 'underscore'
THREE = require 'three'
Jit = require('../../jit-shader')
HexagonSampling = require('./hexagon-sampling')
CotangentFrame = require('./cotangent-frame')
Color = require('color')

class Gradient extends Jit.JitModule
	constructor: (@source,@palette) ->
		super("gradient-#{@source.name}")

	compile: (shader) ->
		float = shader.require("GLSL").float
		source = shader.require(@source)

		gradientFunctionName = shader.getUniqueVariable("gradient");

		colors = _.map @palette.Steps, (step) ->
			c = Color(step.Color)
			return "vec4(#{float(c.red() / 255.0)},#{float(c.green() / 255.0)},#{float(c.blue() / 255.0)},#{float(c.alpha())})"

		values = _.map @palette.Steps, (step) -> step.Value

		min = @palette.Steps[0].Value
		max = @palette.Steps[@palette.Steps.length-1].Value

		gradientFunction = []
		gradientFunction.push "vec4 #{gradientFunctionName}(vec2 uv) {"
		gradientFunction.push "    float value = #{source.sample}(uv);"
		gradientFunction.push "    float valid = #{source.sampleValid}(uv);"
		gradientFunction.push "    vec4 color = vec4(0.0);"

		for i in [0...colors.length-1] by 1
			line = "    color += mix(#{colors[i]},#{colors[i+1]},clamp((value - #{float(values[i])})/(#{float(values[i+1])} - #{float(values[i])}),0.0,1.0))"
			line += " * step(#{float(values[i])},value)" if i > 0
			line += " * (1.0 - step(#{float(values[i+1])},value))" if i+1 < colors.length-1
			gradientFunction.push line + ";"

		gradientFunction.push "    color.a *= valid;"
		gradientFunction.push "    return color;"
		gradientFunction.push "}"

		exports = {}
		_.extend exports, source

		exports.type = "vec4"
		exports.sample = gradientFunctionName
		delete exports.ready
		delete exports.load
		delete exports.dispose

		return {
			definitions: gradientFunction.join "\n"
			exports: exports
		}

class ValueToNormal extends Jit.JitModule
	constructor: (@source,@factor) ->
		super("value-to-normal-#{@source.name}")

	compile: (shader) ->
		float = shader.require('GLSL').float
		cotangentFrame = shader.require('CotangentFrame').cotangentFrame
		source = shader.require(@source)

		compiledModule = new Jit.JitCompiledModule(this,shader)
		sampleValid = compiledModule.exportFunction("valueToNormalValid",[
			"float valueToNormalValid(vec2 uv) {"
			"    "
			"    float valid;"
			"    float sample = #{source.sampleValueNormalized}(uv, valid);"
			"    return valid;"
			"}"
			].join("\n"),"fragment")
		sample = compiledModule.exportFunction("valueToNormal",[
			"vec3 valueToNormal(vec2 uv) {"
			"    vec3 normal = normalize( vNormal );"
			"    vec2 fuvdx = vec2(1.0,0.0) * length(dFdx( uv ));"
			"    vec2 fuvdy = vec2(0.0,1.0) * length(dFdy( uv ));"
			"    "
			"    float elevationScale = #{float(@factor)};"
			"    "
			"    float s0Valid,s1Valid,s2Valid,s3Valid,s4Valid;"
			"    float s0 = #{source.sampleValueNormalized}(uv, s0Valid);"
			"    float s1 = #{source.sampleValueNormalized}(uv + fuvdy, s1Valid);"
			"    float s2 = #{source.sampleValueNormalized}(uv + fuvdx, s2Valid);"
			"    float s3 = #{source.sampleValueNormalized}(uv - fuvdy, s3Valid);"
			"    float s4 = #{source.sampleValueNormalized}(uv - fuvdx, s4Valid);"
			"    "
			"    //TEMP solution for removing hq noise on big elevation differences"
			"    float maxHeightDiff = 0.04;"
			"    "
			"    vec3 va = vec3( 0.0, 1.0,  clamp((s1 - s0), 0.0, maxHeightDiff)*elevationScale * s1Valid );"
	    	"    vec3 vb = vec3( 1.0, 0.0,  clamp((s2 - s0), 0.0, maxHeightDiff)*elevationScale * s2Valid );"
	    	"    vec3 vc = vec3( 0.0, -1.0, clamp((s3 - s0), 0.0, maxHeightDiff)*elevationScale * s3Valid );"
	    	"    vec3 vd = vec3( -1.0, 0.0, clamp((s4 - s0), 0.0, maxHeightDiff)*elevationScale * s4Valid );"
			"    "
	    	"    vec3 tNormal = normalize(( cross(va, vb) + cross(vb, vc) + cross(vc, vd) + cross(vd, va) ) / -4.0);"
    		"    mat3 TBN = #{cotangentFrame}(normal, -vViewPosition, uv);"
    		"    normal = normalize( TBN * tNormal );"
    		"	 normal *= s0Valid;"
			"    return normal;"
			"}"
			].join("\n"),"fragment")

		compiledModule.exports["sampleValue"] = source.sampleValue
		compiledModule.exports["sample"] = sample
		compiledModule.exports["sampleValid"] = sampleValid

		if source.value
			compiledModule.exports["value"] = source.value

			#Loads the value from vertex attribute and scale it into [source.min..source.max] range
			#Attrbibute is filled by function - compiledModule.getTextureValues
			compiledModule.exportFunction("sampleValueVertex",[
				"float sampleValueVertex(vec2 uv) {",
				"   float value = #{source.value};",
				"   value = mix(#{float(@source.min)},#{float(@source.max)},max(value,0.0));",
				"   return value;",
				"}"
				].join("\n"), "vertex")

			compiledModule.exportFunction("sampleValueVertexValid",[
				"bool sampleValueVertexValid(vec2 uv) {",
				"   return #{source.value} >= 0.0;",
				"}"
				].join("\n"), "vertex")
			compiledModule.exportFunction("sampleValueVertexSkirt",[
				"bool sampleValueVertexSkirt(vec2 uv) {",
				"   return #{source.value} < -1.0;",
				"}"
				].join("\n"), "vertex")
		else
			compiledModule.exports["sampleValueVertex"] = source.sampleValue
			compiledModule.exportFunction("sampleValueVertexValid",[
				"bool sampleValueVertexValid(vec2 uv) {",
				"   return true;",
				"}"
				].join("\n"), "vertex")
			compiledModule.exportFunction("sampleValueVertexSkirt",[
				"bool sampleValueVertexSkirt(vec2 uv) {",
				"   return false;",
				"}"
				].join("\n"), "vertex")


		if source.uvTrans
			compiledModule.exportFunction("sampleValue",[
				"float sampleValue(vec2 uv) {",
				"   vec2 uv2 = uv * #{source.uvTrans}.xy + #{source.uvTrans}.zw;",
				"   return #{source.sampleValue}(uv2);",
				"}"
				].join("\n"), "fragment")

			sample = compiledModule.exportFunction("valueToNormalTransUV",[
				"vec3 valueToNormalTransUV(vec2 uv) {",
				"   vec2 uv2 = uv * #{source.uvTrans}.xy + #{source.uvTrans}.zw;",
				"   return #{sample}(uv2);",
				"}"
				].join("\n"),"fragment")

			compiledModule.exports["sample"] = sample

			sampleValid = compiledModule.exportFunction("valueToNormalValidTransUV",[
				"float valueToNormalValidTransUV(vec2 uv) {"
				"    vec2 uv2 = uv * #{source.uvTrans}.xy + #{source.uvTrans}.zw;",
				"    return #{sampleValid}(uv2);"
				"}"
				].join("\n"),"fragment")
			compiledModule.exports["sampleValid"] = sampleValid

		return compiledModule

updateUV = (key,uvTrans) ->
	childIndex = parseInt(key[key.length-1])

	childUvTrans = uvTrans.clone()

	childUvTrans.x /= 3
	childUvTrans.y /= 3
	childUvTrans.z += Math.floor(childIndex % 3) * childUvTrans.x
	childUvTrans.w += (2-Math.floor(childIndex / 3)) * childUvTrans.y

	return childUvTrans

class TextureReader
	@canvas: undefined
	@context: undefined
	@lastImage: undefined
	@lastData: undefined

	@getImageData: (htmlImage) ->
		if htmlImage == TextureReader.lastImage
			return @lastData

		if (htmlImage.data)
			return htmlImage;

		imageWidth = htmlImage.width
		imageHeight = htmlImage.height

		if TextureReader.canvas == undefined
			TextureReader.canvas = document.createElement("canvas")
			TextureReader.context = TextureReader.canvas.getContext("2d")
			TextureReader.context.imageSmoothingEnabled = false

		if TextureReader.canvas.width != imageWidth || TextureReader.canvas.height != imageWidth
			TextureReader.canvas.width = imageWidth
			TextureReader.canvas.height = imageHeight

		TextureReader.context.clearRect(0, 0, imageWidth, imageHeight);
		TextureReader.context.drawImage(htmlImage, 0, 0, imageWidth, imageHeight, 0, 0, imageWidth, imageHeight)

		TextureReader.lastData = TextureReader.context.getImageData(0, 0, imageWidth, imageHeight)
		TextureReader.lastImage = htmlImage

		return TextureReader.lastData

###*
 * a GeoSource loding JitModule
 *
 * options.field - field name to use
 * options.range - max and minimum of the field [min,max]
 * options.boundingSphere - will request only rhombues that intersects the bounding sphere
 * options.style - style to use (when requesting vectors)
 * options.hexSampling - if to do hex sampling
 * options.borrowParentTexture - borrow parent textures while loading current texture
 * options.resolution - bias of texture level, for ex: for key:010 use texture from 01
###
class GeoSource extends Jit.JitModule
	constructor: (@geosource, @options) ->
		super("geosource-#{@geosource.name}")
		
		if not @options.resolution
			@options.resolution = 0
			
		if @options.range
			@field = @options.field
			@min = @options.range[0]
			@max = @options.range[1]
			@type = "float"
			@imageFormat = "PYX0"

			@generateVertexAtrribute = false
		else
			@type = "vec4"
		@baseUVTransform = new THREE.Vector4(1,1,0,0);
		#TODO maybe move in options?
		#comment a next line if you connect to the old GeoWebCore version
		@bits = 24

	asNormal: (factor) ->
		@generateVertexAtrribute = true
		return new ValueToNormal(this,factor)

	gradient: (palette) ->
		return new Gradient(this,palette)
	
	asLowResolution: (resolution) ->
		options = _.extend {}, @options 
		
		options.resolution = @options.resolution + resolution
		options.borrowParentTexture = true
		return new GeoSource(@geosource, options)

	compile: (shader) ->
		self = this

		float = shader.require('GLSL').float
		emptyTexture = shader.require('EmptyTexture').texture
		floatEmptyTexture = shader.require('EmptyTexture').floatTexture
		compiledModule = new Jit.JitCompiledModule(this,shader)
		sampler = compiledModule.uniform("texture","t")
		if @options.resolution
			originalSampler = compiledModule.uniform("originalTexture","t")

		sample = compiledModule.exportFunction("sampleColor",[
			"vec4 sampleColor(vec2 uv) {",
			"    return texture2D(#{sampler},uv);",
			"}"
			].join("\n"))
		sampleValid = compiledModule.exportFunction("sampleColorValid",[
			"float sampleColorValid(vec2 uv) {",
			"    return 1.0;",
			"}"
			].join("\n"))

		if @type == "float"
			#decode 16 bits by default
			if window.glSupport.floatTextures
				encodeValid = "float(color.x != -1.0)"
				encodeString = "color.x"
			else
				encodeValid = "color.a"
				encodeString = "(256.0 * color.x + 16.0*color.y + color.z) / 256.0"
				if @bits == 24
					encodeString = "(256.0 * 256.0 * color.x + 256.0 * color.y + color.z) / (256.0*256.0)"

			sampleNormalized = compiledModule.exportFunction("sampleValueNormalized",[
				"float sampleValueNormalized(vec2 uv, out float valid) {"
				"    vec4 color = #{sample}(uv);"
				"    valid = " + encodeValid + ";"
				"    float value = " + encodeString + ";"
				"    return value;"
				"}"
				].join("\n"))

			sampleValid = compiledModule.exportFunction("sampleValueValid",[
				"float sampleValueValid(vec2 uv) {",
				"    vec4 color = #{sample}(uv);"
				"    return "+ encodeValid + ";",
				"}"
				].join("\n"))

			sample = compiledModule.exportFunction("sampleValue",[
				"float sampleValue(vec2 uv) {"
				"    float valid;"
				"    float value = #{sampleNormalized}(uv, valid);",
				"    value = mix(#{float(@min)},#{float(@max)},value);"
				"    return value;"
				"}"
				].join("\n"))

		if @options.hexSampling
			hexTransform = shader.require('HexagonSampling').nearest

			sample = compiledModule.exportFunction("sampleHex",[
				"#{@type} sampleHex(vec2 uv) {",
				"   return #{sample}(#{hexTransform}(uv));",
				"}"].join("\n"))

			sampleValid = compiledModule.exportFunction("sampleHexValid",[
				"float sampleHexValid(vec2 uv) {",
				"   return #{sampleValid}(#{hexTransform}(uv));",
				"}"].join("\n"))

		if @options.borrowParentTexture
			uvTrans = compiledModule.uniform('uvTrans','v4', new THREE.Vector4(0,0,0,0) );
			sample = compiledModule.exportFunction("sampleUvTrans", [
				"#{@type} sampleUvTrans(vec2 uv) {",
				"   vec2 uv2 = uv * #{uvTrans}.xy + #{uvTrans}.zw;",
				"   return #{sample}(uv2);",
				"}"].join("\n"))

			sampleValid = compiledModule.exportFunction("sampleUvTransValid", [
				"float sampleUvTransValid(vec2 uv) {",
				"   vec2 uv2 = uv * #{uvTrans}.xy + #{uvTrans}.zw;",
				"   return #{sampleValid}(uv2);",
				"}"].join("\n"))

		compiledModule.exports["sample"] = sample
		compiledModule.exports["sampleValid"] = sampleValid

		# check if we assign any texture to shader, include the default empty texture
		hasTexture = (uniforms, uniform) ->
			return uniforms[uniform] and uniforms[uniform].value

		# check if we assign a texture that is not the default empty texture
		hasValidTexture = (uniforms, uniform) ->
			return hasTexture(uniforms, uniform) and uniforms[uniform].value != emptyTexture and uniforms[uniform].value != floatEmptyTexture

		# check if we assign a texture that is not the default empty texture and it not been borrowed from the parent
		hasOwnTexture = (uniforms, uniform) ->
			return hasValidTexture(uniforms, uniform) and uniforms[uvTrans] and uniforms[uvTrans].value.x == 1
			
		# find the distance between own texture holder and current context
		getBorrowDistance = (context) ->
			distance = 0
			c = context
			while c
				if distance >= self.options.resolution and hasTexture(c.uniforms, originalSampler)
					break
				distance+=1
				c = c.parentContext
			return distance
			
		# calculate borrow texture and borrow transform for specefied resolution
		getBorrowData = (context) ->
			distance = 0
			c = context
			texture = undefined
			uv = new THREE.Vector4(1,1,0,0)
			keyStack = []
			
			while c
				if (distance >= self.options.resolution or c.key.length == 1) and hasTexture(c.uniforms, originalSampler)
					texture = c.uniforms[originalSampler].value
					break
				distance+=1
				keyStack.push c.key
				c = c.parentContext
			for i in [0...keyStack.length]
				key = keyStack[keyStack.length - i - 1]
				uv = updateUV(key, uv)
			return [texture, uv]

		compiledModule.ready = (attributes, uniforms, context) ->
			if uniforms[sampler].status == "failed"
				console.log "reloading - #{context.key} - #{self.name}"
				compiledModule.load(attributes, uniforms, context)

			return uniforms[sampler].value

		compiledModule.borrowParentTexture = (attributes, uniforms, context) ->
			if self.options.borrowParentTexture && !hasOwnTexture(uniforms, sampler)
				if context.parentContext and hasValidTexture(context.parentContext.uniforms, sampler)
					updated = false
					if !self.options.resolution
						parentUniforms = context.parentContext.uniforms
						uniforms[sampler].value = parentUniforms[sampler].value
						uniforms[uvTrans].value = updateUV(context.key, parentUniforms[uvTrans].value)
						updated = true

					else if self.options.resolution and (getBorrowDistance(context) >= self.options.resolution or context.key.length < self.options.resolution)
						borrowData = getBorrowData(context) 
						uniforms[sampler].value = borrowData[0]
						uniforms[uvTrans].value = borrowData[1]
						updated = true
						
					if updated
						if self.type == "float" and self.generateVertexAtrribute
							compiledModule.updateValueAttribute(attributes,uniforms,context)
	
						context.emit 'updated', self

					context.foreachChildren( compiledModule.borrowParentTexture )

		compiledModule.init = (attributes, uniforms, context) ->
			context.disposed = false
			compiledModule.borrowParentTexture(attributes, uniforms, context)

			#use empty texture if we have nothing to use at the moment
			if !hasValidTexture(uniforms, sampler)
				if self.type == "float" and window.glSupport.floatTextures
					uniforms[sampler].value = floatEmptyTexture
				else
					uniforms[sampler].value = emptyTexture

				if self.options.borrowParentTexture
					uniforms[uvTrans].value = new THREE.Vector4(1,1,0,0)

				if self.type == "float" and self.generateVertexAtrribute
					compiledModule.updateValueAttribute(attributes,uniforms,context)
					
		compiledModule.load = (attributes, uniforms, context) ->
			uniforms[sampler].status = "loading"

			if context.boundingSphere and self.options.boundingSphere
				if not context.boundingSphere.intersectsSphere(self.options.boundingSphere)
					uniforms[sampler].status = "loaded"
					context.emit 'updated', self
					return

			context.loadTexture self.geosource,
				(texture) ->
					if context.disposed
						context.disposeTexture(self.geosource, texture)
						return
						
					texture.generateMipmaps = false
					texture.magFilter = THREE.LinearFilter
					texture.minFilter = THREE.LinearFilter
					texture.anisotropy = 1
					texture.flipY = true
					
					if !self.options.resolution or context.key.length == 1
						if self.options.resolution
							uniforms[originalSampler].value = texture
							
						uniforms[sampler].value = texture
						uniforms[sampler].status = "loaded"
	
						if self.options.borrowParentTexture
							uniforms[uvTrans].value = new THREE.Vector4(1,1,0,0)
	
						if self.type == "float" and self.generateVertexAtrribute
							compiledModule.updateValueAttribute(attributes,uniforms,context)
	
						context.emit 'updated', self
					else #if we have resolution bias just store original texture
						uniforms[sampler].status = "loaded"
						uniforms[originalSampler].value = texture

					if self.options.borrowParentTexture
						context.foreachChildren( compiledModule.borrowParentTexture )
				(error) ->
					uniforms[sampler].status = "failed"
				self.imageFormat,
				self.options.range

		compiledModule.dispose = (attributes, uniforms, context) ->
			context.disposed = true
			if hasValidTexture(uniforms, sampler) and !hasValidTexture(uniforms, originalSampler)
				if !(self.options.borrowParentTexture and uniforms[uvTrans].value.x != 1)
					context.disposeTexture(self.geosource, uniforms[sampler].value)
					uniforms[sampler].value = undefined
					
			if hasValidTexture(uniforms, originalSampler)
				context.disposeTexture(self.geosource, uniforms[originalSampler].value)
				uniforms[originalSampler].value = undefined

		if @type == "float" and self.generateVertexAtrribute
			valueAttribute = compiledModule.attribute("value","f",[])

			compiledModule.updateValueAttribute = (attributes, uniforms, context) ->
				attributes[valueAttribute].array = compiledModule.getTextureValues(attributes, uniforms, context.meshSize, context.skirtVerticesCount)
				attributes[valueAttribute].needsUpdate = true

			#fill attribute array with normalized texture values [0..1] and -1 for invalid values
			compiledModule.getTextureValues = (attributes, uniforms, size, skirtVerticesCount) ->

				values = new Float32Array( size * size + skirtVerticesCount )

				if hasTexture(uniforms, sampler)
					uvTranform = self.baseUVTransform
					if(self.options.borrowParentTexture)
						uvTranform = uniforms[uvTrans].value

					textureImage = uniforms[sampler].value.image
					imageWidth = textureImage.width
					imageHeight = textureImage.height
					imageData = TextureReader.getImageData(textureImage).data

					if imageData instanceof Float32Array
						sampleImageData = (u,v) ->
							offset = v * imageWidth + u
							return imageData[offset]
					else
						sampleImageData = (u,v) ->
							offset = v * imageWidth + u

							r = imageData[offset * 4 + 0] / 255.0
							g = imageData[offset * 4 + 1] / 255.0
							b = imageData[offset * 4 + 2] / 255.0
							a = imageData[offset * 4 + 3] / 255.0

							if self.bits == 24
								value = (r * 256 * 256 + g * 256 + b) / (256 * 256)
							else
								value = (r * 256 + g * 16 + b) / 256

							if a < 0.5
								value = -1.0

							#value = ( self.max - self.min ) * value + self.min

							return value

					for y in [0...size] by 1
						for x in [0...size] by 1
							u = (x+0.0) / (size-1) * uvTranform.x + uvTranform.z
							v = (1-(y+0.0) / (size-1)) * uvTranform.y + uvTranform.w

							u = u * (imageWidth-1)
							v = (1-v) * (imageHeight-1)

							#uvs.push "#{u},#{v}"

							minU = Math.floor(u)
							minV = Math.floor(v)
							maxU = Math.ceil(u)
							maxV = Math.ceil(v)

							minU = Math.max(Math.min(minU, imageWidth - 1), 0);
							maxU = Math.max(Math.min(maxU, imageWidth - 1), 0);
							minV = Math.max(Math.min(minV, imageHeight - 1), 0);
							maxV = Math.max(Math.min(maxV, imageHeight - 1), 0);

							q0 = sampleImageData(minU, minV)
							q1 = sampleImageData(maxU, minV)
							q2 = sampleImageData(minU, maxV)
							q3 = sampleImageData(maxU, maxV)

							blendCoefU = u - minU
							blendCoefV = v - minV
							q0 = q0 + (q1-q0) * blendCoefU
							q1 = q2 + (q3-q2) * blendCoefU
							value = q0 + (q1 - q0) * blendCoefV

							values[y * size + x] = value

				skirtIdx = size * size

				for i in [0...skirtVerticesCount]
					values[skirtIdx + i] = -2.0 # detect this values on the shader side

				return values

		return compiledModule

module.exports =
	ValueToNormal: ValueToNormal,
	Gradient: Gradient,
	GeoSource: GeoSource,