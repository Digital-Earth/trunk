/*
 - @name requestQueue
 - @ngdoc service
 *
 - @param {Object} $rootScope
 *
 - @description
   A global queue for all load events and GET requests. Useful for enabling UI elements like 
   loading screens and progress bars among other things.
*
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
   Intercepts all 'http' requests to transform or act on specifc native
   or custom request config properties, for example 'action'
*
- @example
*/

// This service is a WIP 
app.factory('httpInterceptor', function ($cacheFactory, $timeout, requestQueue) {
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
        return rejection;
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

// ** Carried over from the current Studio & Gallery ** //
app.service('numberUtils', function() {
    var methods = {};

    methods.isNumber = function (value) {
        return !isNaN(parseFloat(value)) && isFinite(value);
    }

    methods.range = function (min, max) {
        var bounds = [min,max];

        //zero range
        if (bounds[0] === bounds[1] || bounds[1] === undefined) {
            return {
                contains: function(value) { return value === bounds[0]; },
                toPortion: function(value) { return 0; },
                fromPortion: function(portion) { return bounds[0]; }
            }
        }

        //positive range
        if (bounds[0] < bounds[1]) {
            return {
                contains: function(value) { return value >= bounds[0] && value <= bounds[1]; },
                toPortion: function(value) { 
                    if (value <= bounds[0]) return 0;
                    if (value >= bounds[1]) return 1;
                    return (value - bounds[0]) / (bounds[1] - bounds[0] + 0.0);
                },
                fromPortion: function(portion) {
                    if (portion <= 0) return bounds[0];
                    if (portion >= 1) return bounds[1]; 
                    return (bounds[1] - bounds[0])*portion + bounds[0];
                }    
            }
        }

        //negative range
        var revRange = this.range(max, min);

        return {
            contains: function(value) { return revRange.contains(value); },
            toPortion: function(value) { return 1-revRange.toPortion(value); },
            fromPortion: function(portion) { return revRange.fromPortion(1-portion); }
        }
    }

    methods.transform = function (fromRange, toRange) {
        return {
            map: function(value) {
                return toRange.fromPortion(fromRange.toPortion(value));
            },
            reverseMap: function(value) {
                return fromRange.fromPortion(toRange.toPortion(value));
            }
        }
    }
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
   A helper service for dealing with an array of aggregated content.
*
- @example
 // All posts from year
 newsStore.getPosts(arrayItems, 2015);
 // Single post from year
 newsStore.getPosts(arrayItems, 2015, 'some-path');
*/

app.service('newsStore', function ($window, $timeout, $sce, $q) {
    var methods = {};

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
                        });
                    }
                });
            } else {
                $timeout(function() {
                    deferred.resolve(posts[yearIndex][name]);
                });
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
   Detect what platform the User in browsing on.
*
- @example
  detectPlatform.isMobile();
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

app.service('parallaxUtils', function ($window, $timeout, numberUtils) {
    var methods = {};

    methods.scrolling = function () {
        var lastSpot = 0;

        var direction = function (nextSpot) {
            var isMoving = 'static';

            if (nextSpot > lastSpot) {
                isMoving = 'down';
            } else {
                isMoving = 'up';
            }

            lastSpot = nextSpot;
            return isMoving;
        };

        return {
            direction : direction

        }
    };

    methods.converter = function (from, to) {
        var rangeFrom, rangeTo;
        var self = this;
        
        rangeFrom = from;
        rangeTo = to;
        
        var getRange = function (context) {
            if (context === 'from') {
                return rangeFrom;
            } else {
                return rangeTo;
            }
        };

        var getRemap = function (value) {
            return numberUtils.transform(self.getRange('from'), self.getRange('to')).map(value);
        };

        return {
            getRange : getRange,
            getRemap : getRemap          
        }
    }
});



