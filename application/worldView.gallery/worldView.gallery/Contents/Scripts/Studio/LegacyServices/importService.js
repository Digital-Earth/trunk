app.service('importService', function ($q, $timeout, $pyx) {
    var statusCodes = {
        'queue': 'queue',
        'processing': 'processing',
        'waiting': 'waiting',
        'completed': 'completed',
        'failed': 'failed'
    };

    // GeoSources are imported one by one, as this is a memory consuming process
    var queue = $q.when(true);

    // a simplified serializer of a data set (required for use as an associative array key)
    var dataSetToString = function (dataSet) {
        return (dataSet.Metadata ? dataSet.Metadata.Name + dataSet.Metadata.Description : "") + (dataSet.Layer || "");
    }

    var activeImports = {};
    var attachedToEngine = function () {
        $pyx.engine.importSettingRequired(function (dataSet, request, args) {
            $timeout(function () {
                var item = activeImports[dataSetToString(JSON.parse(dataSet))];

                if (item) {
                    item.status = statusCodes.waiting;
                    item.requests[request] = args ? JSON.parse(args) : args;

                    if (item.onRequest) {
                        item.onRequest(request, item.requests[request]);
                    }
                }
            });
        });
    };

    var importService = {
        'supported': function () {
            return 'import' in $pyx.engine;
        },
        'import': function (dataSet) {
            if (attachedToEngine) {
                attachedToEngine();
                attachedToEngine = null;
            }

            var item = {
                'dataSet': dataSet,
                'name': dataSet.Metadata ? dataSet.Metadata.Name : dataSet.Uri,
                'status': statusCodes.queue,
                'requests': {},
                'hasRequest': function (request) {
                    return request in this.requests;
                },
                'onRequest': function (request, args) {
                    //to be overwritten
                },
                'provide': function (request, value) {
                    var self = this;
                    var provideDeferred = $q.defer();
                    $pyx.engine.provideImportSetting(this.dataSet, request, value).success(function () {
                        provideDeferred.resolve(true);
                    }).error(function (error) {
                        provideDeferred.reject(error);
                    });

                    provideDeferred.promise.then(function () {
                        delete self.requests[request];
                        if (self.status === statusCodes.waiting) {
                            self.status = statusCodes.processing;
                        }
                    });

                    return provideDeferred.promise;
                }
            };

            activeImports[dataSetToString(dataSet)] = item;

            var deferred = $q.defer();

            //put this import on the queue
            //currently, there's no need to allow import of more than one GeoSource at once
            queue = queue.then(function () {
                item.status = statusCodes.processing;

                $pyx.engine.import(dataSet).success(function (geoSource) {
                    deferred.resolve(geoSource);
                }).error(function (error) {
                    deferred.reject(error);
                });

                return deferred.promise.then(function () { }, function () { });
            });

            //add the promise and a cancellation option to the result
            item.cancel = function (reason) {

                //cancel all pending requests
                angular.forEach(item.requests, function (value, request) {                    
                    item.provide(request, null);
                });

                deferred.reject(reason);
            }
            item.promise = deferred.promise;

            deferred.promise.then(function () {
                item.status = statusCodes.completed;
                delete activeImports[dataSetToString(dataSet)];
            }, function () {
                item.status = statusCodes.failed;
                delete activeImports[dataSetToString(dataSet)];
            });

            return item;
        },
        'srs': {
            datums: {
                'NAD27': 'NAD27',
                'NAD83': 'NAD83',
                'WGS72': 'WGS72',
                'WGS84': 'WGS84'
            },
            custom: function (str) {
                return {
                    'CoordinateSystem': 'Projected',
                    'Datum':'WGS84',
                    'Projection': 'Custom',
                    'CustomProjection': str
                }
            },
            geographical: function (datum) {
                return {
                    'CoordinateSystem': 'Geographical',
                    'Datum': datum,                    
                }
            },
            utmNorth: function (datum,zone) {
                return {
                    'CoordinateSystem': 'Projected',
                    'Datum': datum,
                    'Projection': 'UTM',
                    'UtmNorth': true,
                    'UtmZone': zone
                }
            },
            utmSouth: function (datum, zone) {
                return {
                    'CoordinateSystem': 'Projected',
                    'Datum': datum,
                    'Projection': 'UTM',
                    'UtmNorth': false,
                    'UtmZone': zone
                }
            }
        }
    };

    return importService;
});