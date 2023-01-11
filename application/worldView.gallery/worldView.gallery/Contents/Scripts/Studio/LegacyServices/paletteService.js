//paletteService is a utility class to modify palette objects
app.service("paletteService", function ($filter, $pyx, numberUtils) {
    var orderBy = $filter('orderBy');

    function isNumeric(num) {
        return !isNaN(num);
    }

    var service = {
        //making sure the palette object is valid: Value type is of correct type and ordered correctly
        'verify': function (palette, valueType) {
            var result = angular.copy(palette);

            if (valueType === "Number") {
                //convert values into float
                angular.forEach(result.Steps, function (step) {
                    step.Value = parseFloat(step.Value);
                });
                //remove all steps that are not numeric
                result.Steps.filter(function (step) {
                    return isNumeric(step.Value);
                });
            }

            result.Steps = orderBy(result.Steps, 'Value');
            return result;
        },
        //reverse the palette
        'reverse': function (palette) {
            var result = angular.copy(palette);
            var stepsCount = result.Steps.length;
            for (var i = 0; i < stepsCount ; i++) {
                //swap values
                var otherIndex = stepsCount - 1 - i;
                var tmp = result.Steps[i].Value;
                result.Steps[i].Value = result.Steps[otherIndex].Value;
                result.Steps[otherIndex ].Value = tmp;
            }
            //make sure the palette is valid
            return service.verify(result);
        },
        'toCSS' : function (palette, rangeFrom, rangeTo) {
            var colorSteps = [];

            for(var index = 0, steps = palette.length; index < steps; index++) {
                var step = palette[index];
                var stepValue = numberUtils.transform(rangeFrom, rangeTo).map(step.Value);
                
                colorSteps.push(step.Color + ' ' + (stepValue * 100) + '%');
            }

            return colorSteps.join(',');
        },
        //add a step into the palette (will order the steps as well)
        'addStep': function (palette, step, valueType) {
            var result = angular.copy(palette);
            result.Steps.push(step);
            //make sure the palette is valid
            return service.verify(result, valueType);
        },
        //create a random color
        'randomColor': function () {
            var letters = '0123456789ABCDEF'.split('');
            var color = '#';
            for (var i = 0; i < 6; i++) {
                color += letters[Math.floor(Math.random() * 16)];
            }
            return color;
        }
    };

    return service;
});