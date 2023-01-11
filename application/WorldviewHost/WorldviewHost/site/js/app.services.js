/*
 - @name requestQueue
 - @ngdoc service
 *
 - @param {Object} $rootScope
 *
 - @description
   A global queue for all load events and GET requests. Useful for enabling UI elements like 
   loading screens and progress bars among other things.
* *
- @example
  requestQueue.increment();
  requestQueue.survey();
*/

// This service is a WIP 
app.service('requestQueue', function ($rootScope) {
    var requestNumber = 0;
    var responsesDone = 0;
    var callbacksDone = [];

    return {
        // In prorgess
        registerDone: function (callback, params) {
            var options = {
                callback: callback,
                params: params || []
            }
            callbacksDone.push(callback);
        },
        percentDone: function() {
            $rootScope.$broadcast('loading-percent', responsesDone / requestNumber);
        },
        survey: function () {
            responsesDone += 1;
            if (responsesDone >= requestNumber && requestNumber !== 0) { 
                responsesDone = 0;
                requestNumber = 0; 
                $rootScope.$broadcast('loading-complete');
            } else {
                this.percentDone();
            }
        },
        increment: function () {
            if (requestNumber === 0) {
                $rootScope.$broadcast('loading-started');
            }

            requestNumber += 1;
            this.percentDone();
        }
    }
});

/*
 - @name httpInterceptor
 - @ngdoc service
 *
 - @param {Object} $cacheFactory
 - @param {Function} $timeout
 - @param {Object} requestQueue
 *
 - @description
   Intercepts all 'http' requests to transform or act on specific native
   or custom request config properties, for example 'action'
*
- @example
*/

// This service is a WIP 
app.factory('httpInterceptor', function ($cacheFactory, $timeout, $q, requestQueue) {
    var methods = {};
 
    methods.request = function (config) {
        if (config.action && config.action === 'queue') {
            requestQueue.increment();
        }
        return config;
    };

    methods.response = function(response) {
        var config = response.config;
        if (config.action && config.action === 'queue') {
            requestQueue.survey();
        }
        return response;
    };

    methods.responseError = function (rejection) {
        return $q.reject(rejection);
    }

    return methods;
});

/*
 - @name httpLoader
 - @ngdoc service
 *
 - @param {Object} $http
 - @param {Object} $rootScope
 *
 - @description
   A native Angular $http wrapper service with the ability to pass custom
   'actions' that can be used by the 'httpInterceptor' service.
*
- @example
  httpLoader.get(pathToJson, 'queue').then(handleLoad);
*/
app.service('httpLoader', function ($http, $rootScope) {
    var methods = {};
    // @param queue - custom config property used with 'httpInterceptor'
    // adds request to progress backlog  
    methods.get = function (path, action) {
        var options = {action: action || 'default'};
        var promise = $http.get(path, options);
        return promise;
    };

    return methods;
});

/*
 - @name newsStore
 - @ngdoc service
 *
 - @param {Object} $window
 - @param {Function} $timeout
 - @param {Function} $sce
 - @param {Function} $q
 *
 - @description
   A helper service for dealing with an array of aggregated conten
*
*
- @example
 // All posts from year
 newsStore.getPosts(arrayItems, 2015);
 // Single post from year
 newsStore.getPosts(arrayItems, 2015, 'some-pat
* );
*/

app.service('newsStore', function ($window, $timeout, $sce, $q) {
    var methods = {};
    var archive = null;

    methods.getAllPosts = function () {
        if (archive === null) {
            archive = $window.newsRepository;
        }
        return archive;
    };

    methods.currentYear = function () {
        return new Date().getFullYear();
    };   

    methods.getYearIndex = function (year) {
        return this.currentYear() - year;
    };

    methods.postNameToUrl = function (name) {
        var pathUrl = name.toLowerCase().replace(/^[-=\s]*/mg, "").replace(/[^\w ]+/g,'').replace(/ +/g,'-');
        return pathUrl;
    };

    methods.getPosts = function (posts, year, name) {
        var self = this;
        var deferred = $q.defer();
        var yearIndex = this.getYearIndex(year);

        if (name === undefined) {
            $timeout(function() {
                deferred.resolve(posts[yearIndex]);
            });
        } else {
            if (angular.isString(name)) {
                angular.forEach(posts[yearIndex], function (post, index) {
                    if (self.postNameToUrl(post.title) === name) {
                        $timeout(function() {
                            deferred.resolve(post);
                        }, 100);
                    }
                });
            } else {
                $timeout(function() {
                    deferred.resolve(posts[yearIndex][name]);
                }, 100);
            }
        }

        return deferred.promise;
    };

    return methods;

});

/*
 - @name detectPlatform
 - @ngdoc service
 *
 - @description
   Detect what platform the User in browsi
*  on.
*
- @example
  detectPlatform.isM
* ile();
*/
app.service('detectPlatform', function () {
    var methods = {};
    // Determine if the user is on a mobile device
    // @returns {Boolean}
    methods.isMobile = function () {
        return (/android|webos|iphone|ipad|ipod|blackberry|windows phone/).test(navigator.userAgent.toLowerCase());
    };
    return methods;
});

/*
 - @name externalLogin
 *
 - @description
   Encapsulates functionality for logging in using a social sign-in provider
*/
app.service('externalLogin', function($location, $pyx, $pyxconfig, siteParams) {
    var methods = {};

    var externalProviders = [];

    var externalLoginUrl = function (provider) {
        var index = $pyx.array.firstIndex(externalProviders, function(p) {
            return p.Name === provider;
        });
        return $pyxconfig.backendUrl + externalProviders[index].Url;
            }

    // if there is a return_to URL parameter, pass it along (single sign-on target)
    var redirect = $location.search()["return_to"] ? "?return_to=" + $location.search()["return_to"] : "";
    $pyx.user.externalLoginProviders(siteParams.galleryUrl(), "/signUp" + redirect)
        .success(function(data) {
            externalProviders = data;
        })
        .error(function() {
            $scope.$emit("wv-alert", { 'message': "Failed to get social sign in providers.", type: "error" });
        });
        
    methods.socialSignIn = function(provider) {
        window.location = externalLoginUrl(provider);
            }

    return methods;
});
