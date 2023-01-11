_ = require 'underscore'
THREE = require 'three'
theme = require './theme'
SharedUniforms = require './shaders/jit/shared-uniforms'

moduleId = 0

toGLSLType =
	t: "sampler2D"
	f: "float"
	c: "vec3"
	v2: "vec2"
	v3: "vec3"
	v4: "vec4"
	m3: "mat3"
	m4: "mat4"

mergeMeshAttrbutes = (attributes) ->
	attribs = {}
	for i in [0...attributes.length]
		moduleAttribs = attributes[i]
		for key, value of moduleAttribs
			attribs[key] = value
	return attribs

class JitCompiledModule
	constructor: (@_module,@_shader) ->
		@id = @_module.id;
		@name = @_module.name;

		@definitions = ""
		@vertexDefinitions = ""
		@fragmentDefinitions = ""
		@body = ""
		@post = ""
		@uniforms = {}
		@attributes = {}
		@exports = {}

	###*
	 * define a uniform with a unique name and it to exports sections
	 * @param  {string} name  name of the unifrom
	 * @param  {string} type  THREE.JS uniform type code: "t"/"f"/"v2"/...
	 * @param  {Object} value default value of the unifrom in THREE.JS object
	 * @param  {bool}   shared (optional) if true the uniform is a globe uniform that is shared between all contexts
	 * @return {string}       the unqiue name for the created uniform
	###
	uniform: (name,type,value,shared) ->
		uniqueName = @_shader.getUniqueVariable(name)
		@uniforms[uniqueName] = { type: type, value: value}
		@uniforms[uniqueName].shared = true if shared
		@definitions += "uniform #{toGLSLType[type]} #{uniqueName};\n"
		@exports[name] = uniqueName
		return uniqueName

	###*
	 * define a attribute with a unique name and it to exports sections
	 * @param  {string} name  name of the unifrom
	 * @param  {string} type  THREE.JS uniform type code: "t"/"f"/"v2"/...
	 * @param  {Object} value default value of the unifrom in THREE.JS object
	 * @return {string}       the unqiue name for the created uniform
	###
	attribute: (name,type,value) ->
		uniqueName = @_shader.getUniqueVariable(name)

		attrib = new THREE.BufferAttribute()
		attrib.itemSize = switch
			when type == 'f' then 1
			when type == 'v2' then 2
			when type == 'v3' then 3
			when type == 'v4' then 4

		@attributes[uniqueName] = attrib # { type: type, value: value}
		@vertexDefinitions += "attribute #{toGLSLType[type]} #{uniqueName};\n"
		@exports[name] = uniqueName
		return uniqueName

	###*
	 * define a varying with a unique name and it to exports sections
	 * @param  {string} name  name of the unifrom
	 * @param  {string} type  THREE.JS uniform type code: "t"/"f"/"v2"/...
	 * @param  {Object} value default value of the unifrom in THREE.JS object
	 * @return {string}       the unqiue name for the created uniform
	###
	varying: (name,type,value) ->
		uniqueName = @_shader.getUniqueVariable(name)
		@definitions += "varying #{toGLSLType[type]} #{uniqueName};\n"
		@exports[name] = uniqueName
		return uniqueName

	###*
	 * define a function using GLSL code and rename it to have a unique name on shader
	 * @param  {string} name    name of the function
	 * @param  {string} glsl    glsl code
	 * @param @optional {"vertex"|"fragment"|} section	if defined - this fucation will be only defined on vertex / fragment shader
	 * @return {string}         unique name of the created function
	###
	function: (name,glsl,section) ->
		uniqueName = @_shader.getUniqueVariable(name)
		nameFinder = new RegExp("#{name}(?![a-zA-Z0-9_])","g")
		fixedCode = "\n"+glsl.replace(nameFinder,uniqueName)+"\n"
		if section == "vertex"
			@vertexDefinitions += fixedCode
		else if section == "fragment"
			@fragmentDefinitions += fixedCode
		else
			@definitions += fixedCode
		return uniqueName

	###*
	 * define a function using GLSL code and rename it to have a unique name on shader, and add it to exports section
	 * @param  {string} name    name of the function
	 * @param  {string} glsl    glsl code
	 * @param @optional {"vertex"|"fragment"|} section	if defined - this fucation will be only defined on vertex / fragment shader
	 * @return {string}         unique name of the created function
	###
	exportFunction: (name,glsl,section) ->
		return @exports[name] = @function(name,glsl,section)


###*
 * @class JitModule
 #
 * JitModule is the basic unit for compiling code into a JIT shader
 *
 * JitModule have a compile function that get the shader as context and return a compiled module object.
 *
 * compiled module object has 4 fields:
 * {
 *    uniforms:  THREE.JS uniforms object
 *    attributes: THREE.JS attributs objects
 *    vertexSettings: GLSL settings code. #pragma, #entensions and precision
 *    fragmentSettings: GLSL settings code. #pragma, #entensions and precision
 *    vertexDefinitions: GLSL definition code. this code need to include all uniforms and global variables and functions (vertex only)
 *    fragmentDefinitions: GLSL definition code. this code need to include all uniforms and global variables and functions (fragment only)
 *    definitions:  GLSL definition code. this code need to include all uniforms and global variables and functions (both vertex and fragment)
 *    vertexBody: GLSL code that will be inserted in the main GLSL function
 *    vertexPost: GLSL code that will be inserted at the ned of the main GLSL function
 *    fragmentBody: GLSL code that will be inserted in the main GLSL function
 *    fragmentPost: GLSL code that will be inserted at the ned of the main GLSL function
 *    exports: all exports to be used by other modules
 * }
 *
###
class JitModule
	constructor: (@name,compile) ->
		@id = moduleId++

		if _.isFunction(compile)
			@compile = compile
		else
			@compiledResults = compile

	compile: (shader) ->
		return @compiledResults || {}

###*
 * @class JitShader
 *
 * a class that can create a GLSL shader by compiling several JitModule together
 *
 * this class is responsible for keeping track of all compiled modules,
 * and the generation of the shader code and THREE uniform code structure
 *
###
class JitShader
	@repository: {}

	###*
	 * register a global module. this good for commonly used shader elements like phong.
	 * @param  {string} name    			name of the module
	 * @param  {string|function} compile 	compiled resuls or a compile function
	 * @return {JitModule}         			created module
	###
	@register: (name,compile) ->
		JitShader.repository[name] = new JitModule(name,compile)
		return JitShader.repository[name]

	###*
	 * Jit constructor now takes a theme object
	 * @param  {string} theme    			theme object
	###
	constructor: (themeObject) ->
		@compiledModules = {}
		@compiledModulesByOrder = []
		@variablePrefixCount = {}
		@theme = themeObject or theme.default

	###*
	 * require a specific JitModule
	 * @param  {JitModule} module 	module to require
	 * @return {object}           	exports generated from the module
	###
	require: (module) ->
		if !module
			throw new Error("module is not defined")

		#recover well known module if we able
		if module of JitShader.repository
			module = JitShader.repository[module]

		#recover compiled module in this context
		if module.id of @compiledModules
			return @compiledModules[module.id].exports

		return @_compileModule(module).exports

	###*
	 * compiles the specific module and register it into compiled module repository
	 * @param  {JitModule} module module to compile
	 * @return {object}           the compiled module object
	###
	_compileModule: (module) ->
		compiledModule =
			id: module.id,
			name: module.name,
			uniforms: {}
			attributes: {}
			vertexSettings: ""
			fragmentSettings: ""
			definitions: ""
			vertexDefinitions: ""
			fragmentDefinitions: ""
			fragmentBody: ""
			fragmentPost: ""
			vertexBody: ""
			vertexPost: ""
			exports: {}
		compiledModule = _.extend compiledModule, module.compile(this)
		@compiledModules[module.id] = compiledModule
		@compiledModulesByOrder.push compiledModule

		return compiledModule

	###*
	 * compiles the shader and return a compiled object
	 * @return {object} compiled GLSL shader code and uniforms
	 *
	 * shader = {
	 *    uniforms: THREE.JS uniforms
	 *    code: GLSL code
	 * }
	###
	compile: () ->
		uniforms = []
		attributes = []
		vertexBody = ""
		vertexPost = ""
		fragmentBody = ""
		fragmentPost = ""
		vertexSettings = ""
		fragmentSettings = ""
		vertexDefinitions = ""
		fragmentDefinitions = ""

		readies = []
		inits = []
		loads = []
		disposes = []

		decorate = (title,value) ->
			return "" if !value
			return "\n/// #{title} ///\n#{value}\n"

		_.each @compiledModulesByOrder, (module) ->
			uniforms.push module.uniforms
			attributes.push module.attributes

			vertexSettings += decorate "#{module.name} settings", module.vertexSettings
			fragmentSettings += decorate "#{module.name} settings", module.fragmentSettings

			vertexDefinitions += decorate "#{module.name} vertex only definitions", module.vertexDefinitions
			fragmentDefinitions += decorate "#{module.name} fragment only definitions", module.fragmentDefinitions

			vertexDefinitions += decorate "#{module.name} definitions", module.definitions
			fragmentDefinitions += decorate "#{module.name} definitions", module.definitions

			vertexBody += decorate "#{module.name} code", module.vertexBody
			fragmentBody += decorate "#{module.name} code", module.fragmentBody

			#merge post in referse order
			vertexPost = decorate("#{module.name} code", module.vertexPost) + vertexPost
			fragmentPost = decorate("#{module.name} code", module.fragmentPost) + fragmentPost

			readies.push module.ready if module.ready
			inits.push module.init if module.init
			loads.push module.load if module.load
			disposes.push module.dispose if module.dispose

		vertexBody = "void main(void) {\n#{vertexBody}\n#{vertexPost}\n}"
		fragmentBody = "void main(void) {\n#{fragmentBody}\n#{fragmentPost}\n}"

		return {
			uniforms: SharedUniforms.merge(uniforms)
			attributes: mergeMeshAttrbutes(attributes)

			vertexCode: [vertexSettings,vertexDefinitions,vertexBody].join("\n")
			fragmentCode: [fragmentSettings,fragmentDefinitions,fragmentBody].join("\n")

			compiledModules: @compiledModules
			tryGetCompiledModule: (module) ->
				if !module
					throw new Error("module is not defined")

				#recover well known module if we able
				if module of JitShader.repository
					module = JitShader.repository[module]

				#recover compiled module in this context
				if module.id of @compiledModules
					return @compiledModules[module.id].exports

				for id, compiledModule of @compiledModules
					return compiledModule.exports if compiledModule.name == module or compiledModule.name == module.name

				return undefined

			getCompiledModule: (module) ->
				result = tryGetCompiledModule(module)
				return result if result

				throw new Error("can't find request module")

			ready: (attributes, uniforms,context) ->
				for ready in readies
					return false unless ready(attributes,uniforms,context)

				return true
				
			init: (geometry, attributes,uniforms,context) ->
				_.each inits, (init) -> init(attributes,uniforms,context)
				for key, value of attributes
					geometry.addAttribute( key, value )

			load: (geometry, attributes,uniforms,context) ->
				_.each loads, (load) -> load(attributes,uniforms,context)

			dispose: (attributes,uniforms,context) ->
				_.each disposes, (dispose) -> dispose(attributes,uniforms,context)
		}

	###*
	 * get a unique variable name, good to create temporary variables in GLSL code
	 #  - DEAN: modified so that the first iteration does not append anything
	 *
	 * example:
	 *    getUniqueVariable("color") -> "color"
	 *    getUniqueVariable("color") -> "color_1"
	 *
	 * @param  {string} prefix 		prefix to use for variable name (to make sure it readable)
	 * @return {string}        		a unique variable in shader context
	###
	getUniqueVariableX: (prefix) ->
		if @variablePrefixCount[prefix]?   # question mark checks if undefined rather than falsy
			@variablePrefixCount[prefix]++
			return "#{prefix}_#{@variablePrefixCount[prefix]}"
		else
			@variablePrefixCount[prefix] = 0
			return prefix

	# using this until we can solve the errors
	getUniqueVariable: (prefix) ->
		@variablePrefixCount[prefix] or= 0
		@variablePrefixCount[prefix]++
		return "#{prefix}_#{@variablePrefixCount[prefix]}"





###
	Number.isInteger is ES6 so not supported in Safari yet, so here is a workaround
###
isInteger = (value) ->
	return !isNaN(value) && parseInt(Number(value)) == value && !isNaN(parseInt(value, 10))

###
 glslFloat return a safe float string for glsl code.

 glslFloat(1) -> "1.0"
 glslFloat(1.2) -> "1.2"
###
glslFloat = (number) ->
	if isInteger(number)
		return "" + number + ".0"
	return "" + number


###*
 * GLSL Provides utility functions to create a nice GLSL code
###
JitShader.register('GLSL',
	{
		exports:
			float: glslFloat
			value: (object) ->
				if isFinite(object)
					return glslFloat(object)

				if 'x' of object and 'y' of object
					return "vec2(#{glslFloat(object.x)},#{glslFloat(object.y)})" unless 'z' of object
					return "vec3(#{glslFloat(object.x)},#{glslFloat(object.y)},#{glslFloat(object.z)})" unless 'w' of object
					return "vec4(#{glslFloat(object.x)},#{glslFloat(object.y)},#{glslFloat(object.z)},#{glslFloat(object.w)})"

				if 'red' of object and 'green' of object and 'blue' of object and 'alpha' of object
					return "vec4(#{glslFloat(object.red()/255.0)},#{glslFloat(object.green()/255.0)},#{glslFloat(object.blue()/255.0)},#{glslFloat(object.alpha())})"
	})



data = new Uint8Array(4)
texture = new THREE.DataTexture( data, 1, 1, THREE.RGBAFormat, undefined, THREE.UVMapping )
texture.generateMipmaps = false
texture.minFilter = THREE.LinearFilter
texture.needsUpdate = true

floatData = new Float32Array(1)
floatData[0] = -1
floatEmptyTexture = new THREE.DataTexture( floatData, 1, 1, THREE.LuminanceFormat, THREE.FloatType )
floatEmptyTexture.generateMipmaps = false
floatEmptyTexture.minFilter = THREE.LinearFilter
floatEmptyTexture.needsUpdate = true

###*
 * EmptyTexture provide shader to use an empty texture when needed
###
JitShader.register('EmptyTexture',
	{
		exports:
			texture: texture,
			floatTexture: floatEmptyTexture
	})



module.exports =
	JitShader: JitShader
	JitModule: JitModule
	JitCompiledModule: JitCompiledModule