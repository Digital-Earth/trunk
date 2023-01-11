###
	
	A place to specify hierarchy model.

	Rhombus: 
	''  
		'0'
			'00'
				'001' 
					'0011'  etc.

	Icon:   "1..12, A..T"
	'' 
		'1'
			'1-'
				'1-0'
					..
					'1-03002'
		'N'
			'N-'
				'N-0'
					..
					'N-03002'


###


THREE = require 'three'

rootKeys = ("#{i}" for i in [0...10])

###*
 * Represent an pyxis hierarchy
 * @class RhombusHierarchy
###
class PyxisHierarchy

	###*
	 * maximum depth of resolution to use
	 * @type {Number}
	###
	@MaxResolution = 16

	###*
	 * what kind of hierarchy?
	 * @return {string} tree type
	###
	type: -> ''

	###*
	 * get the root key
	 * @return {string} root key
	###
	root: -> ''

	###*
	 * get all children of a given key
	 * @param  {string} key
	 * @return {string[]} all children key
	###
	children: (key) ->
		return rootKeys if !key
		return [] if @isLeaf(key)
		return ( "#{key}#{i}" for i in [0...9]) 

	###*
	 * get the child #index of a given key
	 * @param  {string} key
	 * @param  {Number} child index - child index to get
	 * @return {string} the key of child #index
	###
	child: (key, childNum) ->
		return rootKeys[childNum] if !key
		return undefined if @isLeaf(key)
		return "#{key}#{childNum}"

	###*
	 * return if a key has no children
	 * @param  {string} key
	 * @return {bool} true if key has no children
 	###
	isLeaf: (key) ->
		return key.length >= PyxisHierarchy.MaxResolution

	###*
	 * get the parent key of a given key
	 * @param  {string} key
	 * @return {stirng} the parent key
	###
	parent:  (key) ->
		l = key.length
		if l > 1
			return key.substring(0, l - 1)
		else
			return ''


window.PyxisHierarchy = PyxisHierarchy




###*
 *  Let's use the same hierarchy structure for both rhombus and icon trees.
 *
 * @class RhombusHierarchy
###
class RhombusHierarchy extends PyxisHierarchy
	###*
	 * what kind of hierarchy?
	 * @return {string} tree type
	###
	type: -> 'Rhombus'

window.RhombusHierarchy = RhombusHierarchy



###*
 *  Icon Trees might be difficult to model since the first level child nodes
 *  are all optional.  Keep a reference to the Tree in order to get around this.
 *
 * @class IconHierarchy
###
class IconHierarchy extends PyxisHierarchy
	constructor: (@tree) ->
		console.log "create icon hierarchy with tree ", @tree

	###*
	 * what kind of hierarchy?
	 * @return {string} tree type
	###
	type: -> 'Icon'

	###
	 * get all children of a given key - we'll just use the data here
	 * @param  {string} key
	 * @return {string[]} all children key
	###
	children: (key) ->
		key or= ''
		node = @tree.nodes[key]
		return [] unless node
		return _.map node.children, (n) -> n.key

	###*
	 * get the child #index of a given key
	 * @param  {string} key
	 * @param  {Number} child index - child index to get
	 * @return {string} the key of child #index
	###
	child: (key, childNum) ->
		return childNum if !key

		if key.indexOf('-') is -1
			return "#{key}-#{childNum}"
		return "#{key}#{childNum}"
		

	###*
	 * get the parent key of a given key
	 * @param  {string} key
	 * @return {stirng} the parent key
	###
	parent:  (key) ->
		return null unless @tree.nodes[key]
		return null unless @tree.nodes[key].parent
		return @tree.nodes[key].parent.key


window.IconHierarchy = IconHierarchy



module.exports =
	PyxisHierarchy: PyxisHierarchy
	RhombusHierarchy: RhombusHierarchy
	IconHierarchy: IconHierarchy
