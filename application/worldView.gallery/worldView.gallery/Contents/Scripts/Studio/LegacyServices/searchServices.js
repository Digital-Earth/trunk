app.factory('searchServices', function ($q, $timeout, $pyx) {
    var nextId = 0;
    var services = {};

    var currentQuery = undefined;
    var nextQueryId = 0;

    // true when at least one of the registered services is performing a search operation
    var searchInProgress = false;
    var onSingleSearchCompleted = function () {
        if (!$pyx.obj.some(services, function (service) {
            return service.state === 'searching';
        })) {
            searchInProgress = false;
        }
    }

    var invokeSingleSearch = function (service) {
        if (service.state === 'searching') {
            //service is busy
            // unless the service has a search cancellation option, return
            if (!service.cancel) {
                return;
            }
            service.cancel();
            service.state = 'failed';
            onSingleSearchCompleted();
        }
        if (service.query && service.query.id == currentQuery.id) {
            //service already completed current query
            return;
        }
        service.state = 'searching';
        service.query = currentQuery;
        try {
            $q.when(service.search(service.query))
                .then(
                //success
                function (result) {
                    service.state = 'idle';
                    if (service.id in services) {
                        if (service.query.id == currentQuery.id) {
                            service.onResult(result);
                            onSingleSearchCompleted();
                        } else {
                            invokeSingleSearch(service);
                        }
                    }
                },
                //error
                function (error) {
                    service.state = 'failed';
                    if (service.id in services) {
                        if (service.query.id == currentQuery.id) {
                            service.onError(error);
                            onSingleSearchCompleted();
                        } else {
                            invokeSingleSearch(service);
                        }
                    }
                });
        }
        catch (error) {
            service.state = 'failed';
            service.onError(error);
        }
    }

    var invokeSearch = function () {
        angular.forEach(services, invokeSingleSearch);
        searchInProgress = true;
    }
    
    return {
        createQuery: function (text, geometry) {
            return {
                text: text,
                geometry: geometry
            };
        },
        search: function (query) {
            currentQuery = query;
            currentQuery.id = nextQueryId;
            nextQueryId++;
            invokeSearch();
        },
        isSearchInProgress: function() {
            return searchInProgress;
        },
        register: function (name, searchFunc, onResultFunc, onErrorFunc) {
            var newService = {
                id: nextId,
                name: name,
                state: 'idle',
                resultLimit: 5,
                query: undefined,
                search: searchFunc,
                onResult: onResultFunc,
                onError: onErrorFunc,
            };

            services[nextId] = newService;
            nextId++;
            return newService;
        },
        unregister: function (id) {
            delete services[id];
        },
        getServices: function() {
            var registeredServices = [];
            angular.forEach(services, function(service) {
                if (service.name === "WorldView.Gallery") {
                    registeredServices.unshift(service); // prioritize the gallery results
                } else {
                    registeredServices.push(service);
                }
            });
            return registeredServices;
        },
        registerGeoSourceSearch: function (geoSource, fields, onSearchFunc, onResultFunc, onErrorFunc) {
            return this.register(geoSource.Metadata.Name,
                function (query) { //search
                    var deferred = $q.defer();

                    onSearchFunc(query);

                    $pyx.engine.searchQuery({
                        geoSource: geoSource,
                        fields: fields,
                        search: query.text,
                        geometry: query.geometry,
                        skip: 0,
                        take: 50
                    }).success(function (result) {
                        $timeout(function () {
                            deferred.resolve(result);
                        });
                    }).error(function (error) {
                        $timeout(function () {
                            deferred.reject(error);
                        });
                    });

                    return deferred.promise;
                },
                function (result) {
                    onResultFunc(result.features);
                },
                function (error) {
                    onErrorFunc(error);
                });
        },
        registerGallerySearch: function (onSearchFunc, onResultFunc, onErrorFunc) {
            return this.register("WorldView.Gallery",
                function (query) { //search
                    onSearchFunc(query);

                    var resourceType = ["GeoSource", "Map", "Gallery"];

                    //type:[resource-type] support
                    //1) find all type:[resource-type] strings in the query string
                    //2) collect all types
                    //3) remove from queryString
                    var typeFilters = [];
                    var queryString = ' ' + query.text + ' ';

                    angular.forEach(resourceType, function (type) {
                        var match = ' type:' + type.toLowerCase() + ' ';
                        var index = queryString.toLowerCase().indexOf(match);
                        if (index != -1) {
                            queryString = queryString.replace(match,' ');
                            typeFilters.push(type);
                        }
                    });

                    if (typeFilters.length) {
                        resourceType = typeFilters;
                    }

                    function createSearchSuggestions(data) {
                        return data.map(function (suggestion) {
                            return {
                                Type: "Search",
                                Text: suggestion,
                                Metadata: {
                                    Name: suggestion
                                }
                            };
                        });
                    }

                    return $pyx.gallery.resources(resourceType).search(queryString).orderByDesc('Metadata.Updated')
                        .get()
                        .then(function (response) {
                            //filter empty galleries and removed resources
                            response.data = $pyx.array.where(response.data, function (resource) {
                                if (resource.Type == 'Gallery') {
                                    return resource.Resources.length > 0;
                                }
                                if (resource.Type == 'GeoSource' || resource.Type == 'Map') {
                                    return resource.State == 'Active';
                                }
                                return true;
                            });

                            if (response.data.length > 0) {
                                return response;
                            }

                            //no data: try to give completions...
                            return $pyx.gallery.suggestCompletions(queryString.trim()).then(function (suggestResponse) {
                                suggestResponse.data = createSearchSuggestions(suggestResponse.data);

                                if (suggestResponse.data.length > 0) {
                                    return suggestResponse;
                                }

                                //no data: try to give other suggestion
                                return $pyx.gallery.suggestTerms(queryString.trim()).then(function (suggestResponse2) {
                                    suggestResponse2.data = createSearchSuggestions(suggestResponse2.data);
                                    return suggestResponse2;
                                });
                            });
                        });
                },
                function (response) {
                    onResultFunc(response.data);
                },
                function (response) {
                    onErrorFunc(response.error);
                });
        }
    };
});


app.factory('mapSearchService', function ($pyx, $q, $timeout) {
    function SearchService(map,options) {
        var self = this;

        self.options = {
            findProperties: true,
            findExpressions: true
        };
        angular.extend(self.options, options);

        self.queryMiniumLength = 2;

        self.update = function (map) {
            self.map = map;
            self.properties = [];

            if (self.map && self.map.Groups) {
                angular.forEach(map.Groups, function (group) {
                    angular.forEach(group.Items, function (item) {
                        //no specification - skip this item
                        if (!item.Specification) {
                            return;
                        }

                        angular.forEach(item.Specification.Fields, function (field, index) {
                            self.properties.push({
                                item: item,
                                field: field,
                                fieldIndex: index
                            });
                        });
                    });
                });
            }
        }

        self.update(map);

        /*
        find properties in current map.

        This function will search all properties in map by name.

        There are some cases when 2 GeoSources have the fields with the same name.
        In order to select the right GeoSource, fields can be specified as "field @ GeoSource name"
        */
        self.findProperties = function (queryString) {
            if (queryString.indexOf('@') !== -1) {
                var parts = queryString.split('@');

                var fieldName = parts[0].trim();
                var mapItemName = parts[1].trim();

                var allFields = self.findProperties(fieldName);

                return $pyx.array.where(allFields, function (field) {
                    return field.MapItem.Metadata.Name.toLowerCase().indexOf(mapItemName) !== -1;
                });
            }

            var results = [];

            if (queryString.length >= self.queryMiniumLength) {
                angular.forEach(self.properties, function (prop) {
                    if (prop.field.Metadata.Name.toLowerCase().indexOf(queryString) !== -1) {
                        results.push({
                            Type: 'Property',
                            Metadata: prop.field.Metadata,
                            Field: prop.field,
                            FieldIndex: prop.fieldIndex,
                            MapItem: prop.item
                        });
                    }
                });
            }
            

            return results;
        }

        /*
        search for coverage calculator expressions.

        the expression we are looking for have the following format:
        1) start with "=" char.
        2) field names are in [name].
        
        example of valid arguments:
        =[Value]/2
        =min([2050 Temperature],[2040 Temperature])
        */
        //TODO: merge this with Question Engine
        self.findExpression = function (queryString) {
            var expression = queryString;

            //make sure the string start with "="
            if (expression.indexOf("=") !== 0) {
                return [];
            }
            //remove the first "=" from expression
            expression = expression.substring(1);
            
            //this array will contain all referenced map items
            var items = [];

            //contains all variables references...
            var references = {};

            /*
            calculator process expect variables names to be "a0", "b1" , "c0".
            where a,b,c - are pointing the input coverages (a - input[0], b - input[1], ...)
            a0,a1,a2 - are pointing to a specific field. a0 - input[0].field[0]. c1 - input[2].field[1]

            this function take a field object (returned from self.findProperties(text) and perform 2 actions
            1) create the matching calculator process variable name
            2) add the relevant map item into items array.
            */
            function getExpressionVariableName(field) {
                //try to find this map item already been used
                var variableIndex = items.indexOf(field.MapItem);

                //if this is a new map item, add it to the list of supported items
                if (variableIndex === -1) {
                    items.push(field.MapItem);
                    variableIndex = items.length - 1;
                }
                
                //create the right variable name
                var expressionVariableName = String.fromCharCode("a".charCodeAt(0) + variableIndex) + field.FieldIndex;
                return expressionVariableName;
            }

            var regex = /\[(.*?)\]/;

            var regexItem = regex.exec(expression);

            //while we found [name] in expression...
            while (regexItem) {                
                //lookup field by name
                var fields = self.findProperties(regexItem[1]);
                if (fields.length === 0) {
                    return [];
                }

                //create valid variable name from field name
                var field = fields[0];
                var newName = getExpressionVariableName(field);
                
                //update expression
                expression = expression.replace(regexItem[0], newName);

                //add reference
                references[regexItem[0]] = field;

                //search for the next [name]
                regexItem = regex.exec(expression);
            }

            //expression has no supported map-item
            if (items.length === 0) {
                return [];
            }

            return [
                {
                    Type: "Expression",
                    Metadata: {
                        Name: queryString
                    },
                    Expression: expression,
                    References: references,
                    MapItems: items
                }
            ];
        }

        self.search = function (queryString) {
            var safeQuery = queryString.toLowerCase().trim();

            var results = [];

            if (self.options.findExpressions && ('calculate' in $pyx.engine)) {
                results = results.concat(self.findExpression(safeQuery));
            }

            if (self.options.findProperties) {
                results = results.concat(self.findProperties(safeQuery));
            }

            return results;
        }
    }

    return {
        createFromMap: function (map,options) {
            return new SearchService(map, options);
        }
    }
});