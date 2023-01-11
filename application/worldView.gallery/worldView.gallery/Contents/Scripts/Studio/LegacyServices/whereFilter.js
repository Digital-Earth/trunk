app.factory('whereFilter', function ($q, $pyx) {
    var active = [];

    var generateGeneralSuggestions = function (searchString) {
        query = /where\s+(\S+)/i;

        var result = query.exec(searchString);

        suggestedFields = [];

        if (result) {
            var prefix = result[1];

            angular.forEach(active, function (item) {
                angular.forEach(item.Metadata.Definition.Fields, function (field) {
                    if (field.Name.toLowerCase().search(prefix.toLowerCase()) == 0) {
                        suggestedFields.push({ 'geoSource': item, 'field': field });
                    }
                });
            });
        }

        return $q.when(suggestedFields);
    }

    var fixFieldName = function (name) {
        if (name[0] == '"') {
            return name.substr(1, name.length - 2);
        }
        return name;
    }

    var generateSuggestionsRelatedTo = function (basedOnSuggestion, searchString) {
        suggestedFields = [];
        query = /where\s+([^"\s]+|\".*\")/i;
        var result = query.exec(searchString);
        if (!result ||
            fixFieldName(result[1].toLowerCase()) != basedOnSuggestion.field.Name.toLowerCase()) {
            return $q.when(generateGeneralSuggestions(searchString));
        }
        query = /where\s+([^"\s]+|\".*\")\s+(\S+)/;
        var result = query.exec(searchString);
        if (!result) {
            if (basedOnSuggestion.field.FieldType == 'Number') {
                options = ['>', '<'];
                angular.forEach(options, function (option) {
                    suggestedFields.push({ 'geoSource': basedOnSuggestion.geoSource, 'field': basedOnSuggestion.field, 'operation': option });
                });
            } else {
                var values = [];
                if (basedOnSuggestion.field.Values) {
                    angular.forEach(basedOnSuggestion.field.Values, function (value) {
                        suggestedFields.push({
                            'geoSource': basedOnSuggestion.geoSource, 'field': basedOnSuggestion.field, 'operation': 'is', 'value': value, 'palette': {
                                Steps: [
                                    { Value: "", Color: "rgba(0,0,0,0)" },
                                    { Value: value.substr(0, value.length - 1), Color: "rgba(0,0,0,0)" },
                                    { Value: value, Color: "#f00" },
                                    { Value: value + ".", Color: "rgba(0,0,0,0)" }
                                ]
                            }
                        });
                    });
                } else {
                    var deferred = $q.defer();
                    $pyx.engine.getFieldStatistics(basedOnSuggestion.geoSource, basedOnSuggestion.field.Name).success(function (stats) {
                        basedOnSuggestion.field.Values = [];

                        angular.forEach(stats.Distribution.Histogram, function (bin) {
                            basedOnSuggestion.field.Values.push(bin.Min);
                        });

                        angular.forEach(basedOnSuggestion.field.Values, function (value) {
                            suggestedFields.push({ 'geoSource': basedOnSuggestion.geoSource, 'field': basedOnSuggestion.field, 'operation': 'is', 'value': value });
                        });

                        deferred.resolve(suggestedFields);
                    });
                    return deferred.promise;
                }
            }
        }

        query = /where\s+([^"\s]+|\".*\")\s+(<|>) (\d+)/;
        var result = query.exec(searchString);
        if (result) {
            var value = parseInt(result[3]);
            if (result[2] == '>') {
                suggestedFields.push({
                    'geoSource': basedOnSuggestion.geoSource, 'field': basedOnSuggestion.field, 'operation': result[2], 'value': value, 'palette': {
                        Steps: [
                            { Value: value, Color: "rgba(0,0,0,0)" },
                            { Value: value, Color: "#f00" }
                        ]
                    }
                });
            } else {
                suggestedFields.push({
                    'geoSource': basedOnSuggestion.geoSource, 'field': basedOnSuggestion.field, 'operation': result[2], 'value': value, 'palette': {
                        Steps: [
                            { Value: value, Color: "#f00" },
                            { Value: value, Color: "rgba(0,0,0,0)" }
                        ]
                    }
                });
            }
        }

        return $q.when(suggestedFields);
    }

    return {
        setGeoSources: function (geoSources) {
            active = [];
            angular.forEach(geoSources, function (item) {
                if (item.Metadata.Definition) {
                    active.push(item);
                }
            });
        },

        isWhereQuery: function (searchString) {
            return searchString && searchString.toLowerCase().search('where') == 0;
        },

        getSuggestions: function (searchString, basedOnSuggestion) {
            if (basedOnSuggestion) {
                return generatesuggestionsRelatedTo(basedOnSuggestion, searchString);
            } else {
                return generateGeneralSuggestions(searchString);
            }


        }
    };
});