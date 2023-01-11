app = angular.module('appRoot', ['ngResource', 'ngRoute', 'ngCookies', 'ngAnimate', 'ngRetina', 'ngSVGAttributes', 'validation', 'validation.rule', 'validation.schema', '720kb.socialshare']);

app.config(function($httpProvider, ngRetinaProvider, validationSchemaProvider) {

	$httpProvider.interceptors.push('httpInterceptor');

	// Assist with Retina Images
	ngRetinaProvider.setInfix('_2x');

	// Custom validation schemas for our forms 
	var registerSchema = {};

	registerSchema['firstName'] = {
        'validations': 'required',
        'validate-on': 'submit',
        'messages':{
            'required': {
                'error':'We need it',
                'success': 'All good'
            }
        }
    }

	validationSchemaProvider.set("Register", registerSchema);

});

