_ = require 'underscore'
hash = require 'string-hash'

###
 * Hosts is a function that help to distribute requests to hosts based on a given key
 *
 * usage:
 *
 * var hosts = new Hosts(url); //create a single url for both get / post request
 * var hosts = new Hosts(getCdnUrl,postUrl); //create hosts with a cdn url for get requests and server url for post requests
 * var hosts = new Hosts([url1,url2,url3],postUrl); //create hosts with several cdn urls and a single post url
 * 
 * requestUrl = hosts.get(key) + "/api/v1/doSomething" -> for all get requests
 * requestUrl = hosts.post(key) + "/api/v1/doSomething" -> for all post requests
###
class Hosts
	constructor: (@getUrls,@postUrls,@hash) ->
		@hash or= (key) -> hash(key)
		
		@postUrls or= @getUrls 

		#convert into array
		@getUrls = [ @getUrls ] if _.isString(@getUrls)
		@postUrls = [ @postUrls ] if _.isString(@postUrls)

		#optimize get / post functions if we only have 1 url as input
		if @postUrls.length == 1
			@post = () -> @postUrls[0]

		if @getUrls.length == 1
			@get = () -> @getUrls[0]

	post: (key) ->
		return @postUrls[@hash(key || "") % @postUrls.length]

	get: (key) ->
		return @getUrls[@hash(key || "") % @getUrls.length]

	###
	 * return true if geoWebCore is on local computer
	###
	isLocal: () ->
		return false if @postUrls.length != 1
		return @postUrls[0].indexOf('http://localhost:') == 0 || @postUrls[0].indexOf('http://127.0.0.1:') == 0

module.exports = Hosts