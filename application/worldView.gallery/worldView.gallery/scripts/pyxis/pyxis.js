angular.module('pyxis', ['ngCookies'], function ($provide) {
    $provide.service('$pyx', function ($http, $timeout, $q, $rootScope, $pyxuser, $pyxgallery, $pyxarea, $pyxconfig) {
        var array = {
            /*
                usage:
                $pyx.array.firstIndex(array,condition)
                $pyx.array.firstIndex(array,selector,value)

                condition - function(item,index) that get an item and the array index and return boolean.
                            the function return the index of the first value in that return true.

                selector -  access string to select value from.
                            for example, "Metadata.Tags.0" would be translated to: item["Metadata"]["Tags"]["0"]
                            which select the first tag in the metadata object.

                value    -  value to compare the selector against.
                            if selector return an array. the compare would check if the value exists.
                            else, the selector would test the value using === operator.

                examples:
                index = $pyx.array.firstIndex(geoSources,"Metadata.Category","Demo");
                index = $pyx.array.firstIndex(geoSources,"Id","Guid");
        
                index = $pyx.array.firstIndex(geoSources,function(item) { Array.indexOf(item.Tags,"Oil")!=-1; );

                the last can be replaced with 
                index = $pyx.array.firstIndex(geoSources,"Tags","Oil");

                */
            firstIndex: function (array, condition, value) {
                if (!array || !condition) {
                    return -1;
                }
                if (typeof (condition) == "function") {
                    for (var i = 0; i < array.length; i++) {
                        if (condition(array[i], i)) {
                            return i;
                        }
                    }
                    return -1;
                }
                var parts = condition.split(".");
                return this.firstIndex(array, function (item) {
                    var obj = item;
                    for (var i = 0; i < parts.length; i++) {
                        if (!obj) {
                            //can't access object property if it is null
                            return false;
                        } else {
                            //access object property
                            obj = obj[parts[i]];
                        }
                    }
                    if (obj instanceof Array) {
                        return Array.prototype.indexOf.call(obj, value) !== -1;
                    } else {
                        return obj === value;
                    }
                });
            },
            first: function (array, condition, value) {
                var index = this.firstIndex(array, condition, value);
                if (index === -1) {
                    return {
                        found: function () { return this; },
                        notFound: function (func) {
                            func();
                            return this;
                        }
                    };
                }
                return {
                    found: function (func) {
                        func(array[index]);
                        return this;
                    },
                    notFound: function () { return this; }
                };
            },
            some: function (array, condition) {
                for (var i = 0; i < array.length; i++) {
                    if (condition(array[i], i)) {
                        return true;
                    }
                }
                return false;
            },
            where: function (array, condition) {
                var result = [];
                for (var i = 0; i < array.length; i++) {
                    if (condition(array[i], i)) {
                        result.push(array[i]);
                    }
                }
                return result;
            },
            equals: function (array1, array2, condition) {
                condition = condition || angular.equals;

                //return true if both array are empty or null
                if (!array1 && !array2) {
                    return true;
                }

                //return false if only one of them.
                if (!array1 || !array2) {
                    return false;
                }

                if (array1.length !== array2.length) {
                    return false;
                }

                for (var i = 0; i < array1.length; i++) {
                    if (!condition(array1[i], array2[i])) {
                        return false;
                    }
                }

                return true;
            },
            removeFirst: function(array, item) {
                if (array) {
                    var index = array.indexOf(item);
                    if (index !== -1) {
                        array.splice(index, 1);
                        return true;
                    }
                }
                return false;
            }
        }

        var tags = function (array) {
            //make sure we have an array if its empty
            if (array === null || array === undefined) {
                array = [];
            }            
            return {
                exists: function (tag) {
                    return array.indexOf(tag) !== -1;
                },
                toggle: function (tag) {
                    var index = array.indexOf(tag);
                    if (index === -1) {
                        array.push(tag);
                        return tag;
                    } else {
                        array.splice(index, 1);
                        return undefined;
                    }
                },
                add: function (tag) {
                    if (!this.exists(tag)) {
                        array.push(tag);
                        return tag;
                    }
                    return undefined;
                },
                remove: function (tag) {
                    var index = array.indexOf(tag);
                    if (index !== -1) {
                        array.splice(index, 1);
                        return tag;
                    }
                    return undefined;
                },
                all: function () {
                    return array;
                }
            }
        };

        tags.ui = {
            'Favorite': 'Favorite',
            'Searchable': 'Searchable',
            'Geotagable': 'Geotagable'
        };

        tags.itemSystemTags = function (item) {
            if (!item || !item.Metadata) {
                return tags([]);
            }
            if (!item.Metadata.SystemTags) {
                item.Metadata.SystemTags = [];
            }
            return tags(item.Metadata.SystemTags);
        }

        tags.itemTags = function (item) {
            if (!item || !item.Metadata) {
                return tags([]);
            }
            if (!item.Metadata.Tags) {
                item.Metadata.Tags = [];
            }
            return tags(item.Metadata.Tags);
        }

        var loadPyxServiceConfigs = {
            retryCount: 100,    // how many time to try to connect to the embedded service
            retryInterval: 100, // [in milliseconds] interval to retry to check if embedded service is online
        };

        /*
        Connect to an embedded pyxis service.
        @param {string} name of the embedded service to connect to: "application"/"globe"/"engine"
        @returns {proxy} a proxy object to use as embedded service 

        While the embedded service has not initialized, the proxy object will have 2 methods:
            proxy.exists() -> return true or false. Will become true when embedded service has been connected
            proxy.ready(success,error) -> invoke success or error callback when embedded service has been successfully connected or not

        Once the embedded service has been connected, the proxy will have all the methods available on that service.
        For example, 
            globe = loadPyxService("globe");
            globe.ready(function() {

               //all globe embedded functions are available
               globe.show(geoSource);
               globe.setCamera(camera);

            });
        
        */
        var loadPyxService = function (serviceName) {
            var service = {
                exists: function () { return false; }
            };

            var tryToLoadCount = loadPyxServiceConfigs.retryCount;
            var deferred = undefined;

            var requesting = false;
            var loaded = false;

            var loadService = function () {      
                try
                {
                    //if the proxy was loaded or we are in the process of request a proxy already - just wait
                    if (loaded || requesting) {
                        return;
                    }

                    //check if we can call a PYX.get
                    if ('PYX' in window && 'get' in window.PYX) {
                        var doGet = true;

                        if ('PYX' in window && 'has' in window.PYX) {
                            doGet = window.PYX.has(serviceName);
                        }

                        if (doGet) {
                            requesting = true;
                            window.PYX.get(serviceName, function (proxy) {
                                requesting = false;

                                if (proxy) {
                                    service.exists = function () { return true; }
                                    service.__proto__ = proxy;

                                    console.log("PYX.get(" + serviceName + ") completed.");

                                    deferred.resolve(service);

                                    loaded = true;
                                }
                            }, function (error) {
                                requesting = false;
                                console.log("PYX.get(" + serviceName + ") has failed : " + error);
                            });
                        }
                    }
                }
                catch (e) {
                    console.log("error occurred while trying to load embedded service, will retry (error :" + e + ")");
                }
                finally {
                    if (loaded) {
                        //we done.
                        return;
                    }
                    else if (tryToLoadCount) {
                        tryToLoadCount--;
                        $timeout(loadService, loadPyxServiceConfigs.retryInterval);
                    } else {
                        message = "can't load embedded service " + serviceName;
                        deferred.reject(message);
                        console.error(message);
                    }
                }
            }

            service.ready = function (success, error) {
                //start loading on the first call to ready()
                if (!deferred) {
                    deferred = $q.defer();
                    loadService();                
                }

                //register callbacks
                deferred.promise.then(success, error);                
            }

            return service;
        }

        function makeAgent(resource) {
            if (resource.Type in $pyxconfig.resourceAgents) {
                return $pyxconfig.resourceAgents[resource.Type](resource);
            } else {
                var nullAgent = {
                    'self': resource
                };
                return nullAgent;
            }
        }
     
        //wrapper around asnyc like $http or pyxis api functions to look like angular $q promise
        function when(value) {
            if ('success' in value && 'error' in value) {
                var deferred = $q.defer();
                value.success(function (data) {
                    deferred.resolve(data);
                });
                value.error(function (error) {
                    deferred.reject(error);
                });
                return deferred.promise;
            }
            return $q.when(value);
        };

        //http://stackoverflow.com/questions/7616461/generate-a-hash-from-string-in-javascript-jquery
        function hashString(str) { return str.split("").reduce(function (a, b) { a = ((a << 5) - a) + b.charCodeAt(0); return a & a }, 0); };

        // allow to generate a hash number from any object
        //
        // this function will covert the object into json string for hashing complex objects.
        function hash(obj) {
            if (angular.isUndefined(obj)) {
                return hashString("");
            } else if (angular.isString(obj)) {
                return hashString(obj);
            } else {
                return hashString(angular.toJson(obj));
            }
        }
        
        // allow to access an object properties in an safe way without check for null every time.
        //
        // replace: 
        //  if (currentMap.model.Dashboards &&
        //      currentMap.model.Dashboards.length > 0 &&
        //      currentMap.model.Dashboards[index].Selection &&
        //      currentMap.model.Dashboards[index].Selection.Geometry) {
        //     var geometry = currentMap.model.Dashboards[index].Selection.Geometry;
        //  }
        //
        // with:
        //  var geometry = $pyx.obj.get(currentMap,'model','Dashboards',index,'Selection','Geometry');
        function get() {
            if (arguments.length === 0) {
                return undefined;
            }
            var obj = arguments[0];
            for (var i = 1; i < arguments.length; i++) {
                if (obj === undefined || obj === null) {
                    break;
                }
                obj = obj[arguments[i]];
            }
            return obj;
        }

        // allow to check if an object has specific properties in an safe way without check for null every time.
        //
        // replace: 
        //  if (currentMap.model.Dashboards &&
        //      currentMap.model.Dashboards.length > 0 &&
        //      currentMap.model.Dashboards[index].Selection &&
        //      currentMap.model.Dashboards[index].Selection.Geometry) {
        //     var geometry = currentMap.model.Dashboards[index].Selection.Geometry;
        //  }
        //
        // with:
        //  if ($pyx.obj.has(currentMap,'model','Dashboards',index,'Selection','Geometry')) {
        ///    var geometry = currentMap.model.Dashboards[index].Selection.Geometry;
        //  }
        function has() {
            if (arguments.length === 0) {
                return undefined;
            }
            var obj = arguments[0];
            for (var i = 1; i < arguments.length; i++) {
                if (obj === undefined || obj === null || !obj.hasOwnProperty(arguments[i])) {
                    return false;
                }
                obj = obj[arguments[i]];
            }
            return true;
        }

        // verify whether a given object has at least one property that satisifies the given condition
        function some(obj, condition) {
            var result = false;
            angular.forEach(obj, function(property) {
                if (condition(property)) {
                    result = true;
                }
            });
            return result;
        }

        // returns true if the given objects are both strings and satisfy case insensitive comparison
        function equalsCaseInsensitive(str1, str2) {
            return str1 && str2 && str1.toLocaleLowerCase && str2.toLocaleLowerCase
                && (str1 === str2 || str1.toLocaleLowerCase() === str2.toLocaleLowerCase());
        }

        var pyx = function (resource) {
            return makeAgent(resource);
        }

        pyx.obj = {
            get: get,
            has: has,
            hash: hash,
            some: some,
            equalsCaseInsensitive: equalsCaseInsensitive
        };
        pyx.when = when;
        pyx.array = array;
        pyx.tags = tags;
        pyx.user = $pyxuser;
        pyx.gallery = $pyxgallery;
        pyx.area = $pyxarea;
        pyx.globe = loadPyxService('globe');
        pyx.engine = loadPyxService('engine');
        pyx.application = loadPyxService('application');

        var embeddedInternal = {
            EmitKeyValue: function (key, value) {
                if (window.external) {
                    try {
                        window.external.EmitKeyValue(key, value);
                    } catch (e) {
                        console.log(e);
                    }
                }
            },
            AgreementDeclined: function () {
                if (window.external) {
                    try {
                        window.external.AgreementDeclined();
                    } catch (e) {
                        console.log(e);
                    }
                }
            },
            AgreementAccepted: function () {
                if (window.external) {
                    try {
                        window.external.AgreementAccepted();
                    } catch (e) {
                        console.log(e);
                    }
                }
            }
        };

        var embeddedApi = {
            notifyPublishCompleted: function () {
                embeddedInternal.EmitKeyValue('User', pyx.user.username());
                embeddedInternal.EmitKeyValue('access_token', pyx.user.authHeaders()['Authorization'].replace('Bearer ', ''));
                embeddedInternal.AgreementAccepted();
            },
            notifyPublishCanceled: function () {
                embeddedInternal.AgreementDeclined();
            }
        }

        //we need to replace this name to embedded
        pyx.embedded = embeddedApi;

        return pyx;
    });
})
.config(function ($provide) {
    var backendUrl = 'https://api.pyxis.worldview.gallery';
    $provide.value('$pyxconfig',
    {
        backendUrl: backendUrl,
        baseUrl: backendUrl + '/api/v1',
        geoWebCoreUrl: 'http://localhost:44055',
        resourceAgents: {}
    });
});