angular.module("pyxis").factory("$pyxarea",function($http){

    function asBooleanGeometry(geometry) {
        if (geometry.type === "Boolean") {
            return geometry;
        } else {
            return {
                'type': "Boolean",
                'operations': [
                    {
                        'operation': "Disjunction",
                        'geometry': geometry
                    }
                ]
            };
        }
    }

    function addBooleanOperation(geometry, operation) {
        var booleanGeometry = asBooleanGeometry(geometry);

        var result = {
            'type': "Boolean",
            'operations': []
        }
        angular.forEach(booleanGeometry.operations, function (op) {
            result.operations.push(op);
        });
        result.operations.push(operation);

        return result;
    }


    var area = {
        geoJson: function (object) {
            if (object.type === "Polygon" || object.type === "MultiPolygon")
                return object;
            return null;
        },
        filter: function (dataset, filter) {
            return {
                'type': "Filter",
                'dataset': dataset,
                'filter': filter
            };
        },
        circle: function (center, radius) {
            return {
                'type': "Circle",
                'coordinates': center,
                'radius': radius
            };
        },
        polygon: function (points) {
            if (arguments.length > 1) {
                return {
                    'type': "Polygon",
                    'coordinates': Array.prototype.slice.call(arguments, 0)
                };
            } else {
                return {
                    'type': "Polygon",
                    'coordinates': points
                };
            }
        },
        multipolygon: function (multipoints) {
            return {
                'type': "MultiPolygon",
                'coordinates': multipoints
            };
        },
        cell: function (index) {
            return {
                'type': "PYXCell",
                'index': index
            };
        },
        tile: function (index, resolution) {
            return {
                'type': "PYXTile",
                'index': index,
                'resolution': resolution
            };
        },
        featureRef: function (resource, featureId) {
            return {
                'type': "FeatureRef",
                'resource': {
                    'Id': resource.Id,
                    'Type': resource.Type,
                    'Version': resource.Version
                },
                'id': featureId
            };
        },
        condition: function (resource, property, range) {
            return {
                'type': "Condition",
                'resource': {
                    'Id': resource.Id,
                    'Type': resource.Type,
                    'Version': resource.Version
                },
                'property': property,
                'range': {
                    'Min': range.min || range.Min,
                    'Max': range.max || range.Max
                }
            };
        },
        intersection: function (one, two) {
            return addBooleanOperation(one, {
                'operation': "Intersection",
                'geometry': two
            });
        },
        subtraction: function (one, two) {
            return addBooleanOperation(one, {
                'operation': "Subtraction",
                'geometry': two
            });
        },
        disjunction: function (one, two) {
            return addBooleanOperation(one, {
                'operation': "Disjunction",
                'geometry': two
            });
        },
        bbox: function (srs, minLat, minLong, maxLat, maxLong) {
            return {
                'type': "BBox",
                'coordinates': [[minLong, minLat], [maxLong, maxLat]],
                'srs': srs
            };
        },
        wgs84bbox: function (minLat, minLong, maxLat, maxLong) {
            return wgs84bbox({
                'CoordinateSystem': "Geographical",
                'Datum': "WGS84"
            }, minLat, minLong, maxLat, maxLong);
        }
    };

    return area;
});