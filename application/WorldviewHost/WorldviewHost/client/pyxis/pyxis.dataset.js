angular.module('pyxis').factory('$pyxdataset', function ($q, $http, $pyxconfig) {

    var baseUrl = function () {
        return $pyxconfig.geoWebCoreUrl;
    }

    var dataset = {
        open: function (geoSource) {
            var q = $q.defer();

            setTimeout(function () {
                q.resolve({ 'ProcRef': geoSource.ProcRef });
            }, 100);

            return q.promise;
        },
        definition: function (geoSource) {
            return $http({
                'method': "POST",
                'url': baseUrl() + "/api/dataset/definition",
                'data': { 'source': geoSource }
            });
        },
        report: function (geoSource, area, options) {
            return $http({
                'method': "POST",
                'url': baseUrl() + "/api/dataset/report",
                'data': {
                    'source': geoSource,
                    'area': area,
                    'field': options.field
                }
            });
        }
    };

    return dataset;
});