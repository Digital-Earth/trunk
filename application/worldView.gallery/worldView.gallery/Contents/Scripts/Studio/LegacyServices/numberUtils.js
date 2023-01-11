/*
- @name numberUtils
- @desc Helper service for number conversion - I think this 
- should be built into a toolkit for use on both Studio & Gallery
*/
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

