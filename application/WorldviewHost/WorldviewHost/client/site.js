var app = angular.module('worldViewSite', ['ngRoute', 'pyxis', 'pyxis-ui', 'analytics', 'autocomplete', 'ngResource', 'ngCookies', 'ngAnimate', 'ngRetina', 'ngSVGAttributes', 'validation', 'validation.rule', 'validation.schema', '720kb.socialshare', 'flux', 'localization']);

app.config(function ($httpProvider, ngRetinaProvider, validationSchemaProvider) {

    $httpProvider.interceptors.push('httpInterceptor');

    // Assist with Retina Images
    ngRetinaProvider.setInfix('_2x');

    // Custom validation schemas for our forms 
    var registerSchema = {};

    registerSchema['globals'] = {
        'validations': 'required',
        'validate-on': 'submit',
        'messages': {
            'required': {
                'error': 'We need it',
                'success': 'All good'
            }
        }
    }
    registerSchema['firstName'] = {};
    registerSchema['lastName'] = {};
    registerSchema['email'] = {
        'validations': 'required, email',
        'messages': {
            'email': {
                'error': 'Must be an email address'
            }
        }
    };
    registerSchema['password'] = {
        'validations': 'required, minlength=6',
        'messages': {
            'minlength': {
                'error': 'Not long enough'
            }
        }
    };
    registerSchema['confirmPassword'] = {
        'validations': 'required, minlength=6',
        'messages': {
            'minlength': {
                'error': 'Not long enough'
            }
        }
    };
    registerSchema['galleryName'] = {
        'validations': 'required, minlength=5',
        'messages': {
            'required': {
                'error': 'Required letters and numbers only'
            },
            'minlength': {
                'error': 'Not long enough'
            }
        }
    };

    var signInSchema = {};

    signInSchema['globals'] = {
        'validations': 'required',
        'validate-on': 'submit',
        'messages': {
            'required': {
                'error': 'We need it',
                'success': 'All good'
            }
        }
    }
    signInSchema['email'] = {
        'validations': 'required',
        'messages': {
            'email': {
                'error': 'Must be an email address'
            }
        }
    };

    var newsletterSchema = {};

    newsletterSchema['email'] = {
        'validations': 'required, email',
        'messages': {
            'email': {
                'error': 'Must be an email address'
            }
        }
    };

    validationSchemaProvider.set("Register", registerSchema);
    validationSchemaProvider.set("SignIn", signInSchema);
    validationSchemaProvider.set("Newsletter", newsletterSchema);

});

app.run(function($rootScope, $window) {
    // Angular bug - ng-view autoscroll="true" doesn't
    // work on page refresh
    $rootScope.$on('$viewContentLoaded', function () {
        $window.scrollTo(0, 0);
    });
});