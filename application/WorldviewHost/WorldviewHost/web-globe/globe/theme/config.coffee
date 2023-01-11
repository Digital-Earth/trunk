###
	Define a theme config that can be serialized to JSON via
	the dat-gui control interface.
###

_ = require 'underscore'


###
	this config is serializable and editable, so no  
	functions here
###
darkThemeConfig = 
	name: 'Dark Theme'
	specular: 0.4
	shininess: 16.0
	diffuseStrength: 2.0
	lightingEnable: 0.0
	temporalSuperSamping:true
	
	# map defaults
	dataMin: 0
	dataMax: 9000  # elevation
	backgroundColor: '#282828'
	backgroundColor2: '#666666'
	phongStrength: 0.25
	environmentMultiplier: 1.0
	heightMultiplier: 1.0
	normalmapStrength: 1.0
	clipAnimation: true
	clipAnimationPower: 3.0

	# we can use the base alpha map to mask out the water and still bring on
	# full coverage Bing maps.  This acts as a boolean as 1 or 0
	landOnly: 0

	#post process options
	gammaValue: 1.0
	colorBoost: '#000000'
	colorTint: '#777777'
	tiltshiftValue: 0.0 #.0022
	tiltshiftRes: .45

	# camera limits
	rangeMax: 14000000.0
	rangeMin: 500.0
	latMax: -1.0 # no limit
	latMin: -1.0
	lonMax: -1.0
	lonMin: -1.0

	# lighting
	lightIntensity: 0.01 #0.15

	# assets
	roughMatcap: 'assets/images/textures/matcaps/globe_matcap_silver.png'
	roughMatcapFrameCount: 1,
	roughMatcapFrame: 0,
	#matcap blend example
	#roughMatcap: 'assets/images/textures/matcaps/globe_matcap_silver_composite.png' 
	#roughMatcapFrameCount: 2,

	glossMatcap: 'assets/images/textures/matcaps/globe_matcap_black_glossy_blur.png'
	glossMatcapFrameCount: 1,
	glossMatcapFrame: 0,
	noiseMap: 'assets/images/textures/noisemap512.jpg'

	# base layer object
	globeBackground: '#202020'
	noiseMultiplier: 0.2
	basemapRoughness: 1.0
	basemapLightingMultiplier: 1.0
	surfaceRoughness: 0.2
	surfaceLightingMultiplier: 1.0

	# rhombus layers 
	# note instead of using an array or an object we use numbered variables for
	# ease of access
	layer0Alpha: 1.0
	layer0Tint: '#777777'

	layer1Alpha: 1.0
	layer1Tint: '#777777'

	layer2Alpha: 1.0
	layer2Tint: '#777777'

	layer3Alpha: 1.0
	layer3Tint: '#777777'

	layer4Alpha: 1.0
	layer4Tint: '#777777'

	layer5Alpha: 1.0
	layer5Tint: '#777777'

	layer6Alpha: 1.0
	layer6Tint: '#777777'

	layer7Alpha: 1.0
	layer7Tint: '#777777'

	layer8Alpha: 1.0
	layer8Tint: '#777777'

	layer9Alpha: 1.0
	layer9Tint: '#777777'


	# icon layers
	iconAlpha: 1.0
	iconBorderColor: '#000000'

	# each theme should have a base that gets created automatically 
	# when a new map is generated

	# 
	defaultRenderLayers: [
		{
			geoSource:
				Id:'8be6a2ec-5110-49cf-a295-1008f8e9a21b',
				Specification: {OutputType: 'Coverage'}
			style:
				ShowAsElevation: true,
				Fill:
					Style:'Palette',
					PaletteExpression: 'RGB',
					Palette:
						Steps:[  
							{Value: -1000, Color:"rgba(0,0,0,0.0)" }
							{Value:0, Color:"#444444" }
							{Value:3341, Color:"#aaaaaa" }
							{Value:5052, Color:"#ffffff" }
						]
		}
	]


	defaultMapGroupObject: {
		Metadata: {Name: "Theme Basemap"},
		"Items":[
			{
			    "Metadata": {
			        "Comments": [],
			        "Ratings": {
			            "Likes": 0,
			            "Dislikes": 0
			        },
			        "User": {
			            "Id": "c509bb71-192b-4fa9-aa58-58ab58f43473",
			            "Name": "Pyxis"
			        },
			        "Providers": [
			            {
			                "Type": "Gallery",
			                "Id": "32f9cd5d-6ef6-4601-b367-109d8d288557",
			                "Name": "Microsoft Bing Maps"
			            }
			        ],
			        "Category": "Demo",
			        "Created": "2014-05-05T21:21:34Z",
			        "Updated": "2015-01-11T00:09:40.058Z",
			        "BasedOnVersion": "1d6a3b01-50ef-4d80-906c-9e1524d77c32",
			        "Visibility": "Public",
			        "Name": "Global Elevation (GTOPO30)",
			        "Description": "GTOPO30..",
			        "Tags": [
			            "Global",
			            "Elevation"
			        ],
			        "SystemTags": [],
			        "ExternalUrls": [
			            {
			                "Type": "Image",
			                "Url": "https://www.pyxisinnovation.com/images/pipelines/8be6a2ec-5110-49cf-a295-1008f8e9a21b.jpg"
			            }
			        ]
			    },
			    "Specification": {
			        "OutputType": "Coverage",
			        "Fields": [
			            {
			                "Name": "RGB",
			                "FieldType": "Number",
			                "FieldUnit": {
			                    "Name": "m"
			                },
			                "Metadata": {
			                    "Name": "Elevation",
			                    "Description": "",
			                    "SystemTags": [
			                        "Favorite"
			                    ]
			                },
			                "Starred": true,
			                "OnDashboard": false,
			                "Value": 1848
			            }
			        ]
			    },
			    "Resource": {
			        "Id": "8be6a2ec-5110-49cf-a295-1008f8e9a21b",
			        "Type": "GeoSource",
			        "Version": "0329caed-c92e-49c7-b7ce-d0b81af25d97"
			    },
			    "Style": {
			        "Fill": {
			            "PaletteExpression": "RGB",
			            "Style": "Palette",
			            "Palette": {
			                "Steps": [
			                    {
			                        "Value": -1000,
			                        "Color": "rgba(0,0,0,0.0)"
			                    },
			                    {
			                        "Value": 0,
			                        "Color": "#000000"
			                    },
			                    {
			                        "Value": 3341.39,
			                        "Color": "#111111"
			                    },
			                    {
			                        "Value": 5052.27,
			                        "Color": "#111111"
			                    }
			                ]
			            },
			            "Color": null
			        },
			        "ShowAsElevation": true
			    },
			    "Active": true,
			    "ShowDetails": true,
			    "Legend": {
			        "Expanded": true
			    }
			}
		]
	}
	#Blending theme parameters based on the altitude
	#Blocks must be sorted from higher altitude to lower
	#altitudeBasedParameters: [
	#	{
	#		#at alitude 10000.0 parameters in 'parameters' section will be applied with 0 power, and altitude 9000.0 - with 1.0. 
	#		#And parameters will be lerped between 'startAltitude' and 'startAltitude'-'blendAmount'
	#		#        ------|0---------------------------1|--------------
	#		#        ------|<------BlendAmount--------->|---------------
	#		#        ------^----------------------------^---------------
	#		#        --startAltitude---------------blend end------------
	#		startAltitude: 5000000.0,
	#		blendAmount:   4000000.0,
	#		parameters:	
	#			{
	#				lightIntensity: 1.0,
	#				backgroundColor: '#ff0000',
	#				roughMatcapFrame: 1,
	#			}
	#	},
	#	{
	#		startAltitude: 10000.0,
	#		blendAmount: 1000.0,
	#		parameters:	
	#			{
	#				lightIntensity: 0.0
	#			}
	#	}
	#]




lightThemeConfig = 
	name: 'Light Theme'
	backgroundColor: '#dcdcdc'
	backgroundColor2: '#4a4a4a'
	globeBackground: '#a2a2a2'
	lightIntensity: 0.1
	noiseMultiplier: 0.2
	surfaceRoughness: 0.0
	phongStrength: 0.25
	environmentMultiplier: 1.0
	gammaValue: 1.0
	roughMatcap: 'assets/images/textures/matcaps/globe_matcap_black_diffuse.png'
	glossMatcap: 'assets/images/textures/matcaps/globe_matcap_black_glossy_blur.png'

lightThemeConfig = _.extend _.clone(darkThemeConfig), lightThemeConfig


landingGlobeConfig = 
	name: 'Landing Globe'
	backgroundColor: '#656565' #'#252525'
	backgroundColor2: '#252525'
	globeBackground: '#000000'
	iconBorderColor: '#f0f0f0'
	lightIntensity: 0.01 
	gammaValue: 1.2
	noiseMultiplier: 0.09
	phongStrength: 0.5
	environmentMultiplier: 2.7
	surfaceRoughness: 0.0
	surfaceLightingMultiplier: 0.7
	roughMatcap: 'assets/images/textures/matcaps/globe_matcap_black_diffuse.png'
	glossMatcap: 'assets/images/textures/matcaps/globe_matcap_black_glossy2.png'
	defaultRenderLayers: [
		{
			geoSource:
				Id:'8be6a2ec-5110-49cf-a295-1008f8e9a21b',
				Specification: {OutputType: 'Coverage'}
			style:
				ShowAsElevation: true,
				Fill:
					Style:'Palette',
					PaletteExpression: 'RGB',
					Palette:
						Steps:[  
							{Value: -1000, Color:"rgba(0,0,0,0.0)" }
							{Value:0, Color:"#000000" }
							{Value:3341, Color:"#000000" }
							{Value:5052, Color:"#000000" }
						]
		}
	]

landingGlobeConfig = _.extend _.clone(darkThemeConfig), landingGlobeConfig


basemapTestGlobeConfig = 
	name: 'Basemap Test Globe'
	gammaValue: 1.0
	noiseMultiplier: 0.09
	backgroundColor: '#dcdcdc'
	environmentMultiplier: 1.1
	surfaceLightingMultiplier: 0.7
	surfaceRoughness: 0.4
	phongStrength: 0.37
	roughMatcap: 'assets/images/textures/matcaps/globe_matcap_black_diffuse.png'

basemapTestGlobeConfig = _.extend _.clone(darkThemeConfig), basemapTestGlobeConfig

module.exports = {
	darkThemeConfig: darkThemeConfig
	lightThemeConfig: lightThemeConfig
	landingGlobeConfig: landingGlobeConfig
	basemapTestGlobeConfig: basemapTestGlobeConfig
}