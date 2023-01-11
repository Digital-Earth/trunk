/* 
feature: search
goal: implement the search bar features.

this feature uses searchServices to register several search options to the user.

TODO: this feature should  be broken in to different search providers.
*/
app.service('featureSearch', function ($pyx, $filter, $q, $timeout, $rootScope, $pyxIntercom,
    worldViewStudioConfig, mapFactory, searchServices, connectionIndex, onlineGeospatialService, localSearch, mapSearchService, geocodeService) {
    
    var i18n = $filter('i18n');
    
    function register($scope) {
        $scope.gallerySearch = searchServices.registerGallerySearch(
            function (query) {
                $scope.gallerySearch.resultLimit = worldViewStudioConfig.search.defaultResultCount;
                $scope.gallerySearch.results = [];
                $rootScope.gaEvent('search-gallery-query', query.text);
            },
            function (results) {
                $scope.gallerySearch.results = results;
                if ($scope.gallerySearch.results.length > 0) {
                    if ($scope.gallerySearch.results[0].Type === "Search") {
                        $scope.gallerySearch.name = i18n("Looking for ...");
                        $rootScope.gaEvent('search-gallery-suggestion', $scope.gallerySearch.results[0].Text);
                    } else {
                        $scope.gallerySearch.name = i18n("WorldView Gallery");
                        $rootScope.gaEvent('search-gallery-results', 'found');
                    }
                } else {
                    $rootScope.gaEvent('search-gallery-results', 'not found');
                }
            },
            function (error) {
                // inform the network verification service about the request failure
                $rootScope.$broadcast('alert-network-service-state', { serviceName: 'gallerySearch', state: false, message: i18n("Failed to connect to WorldView Gallery") });
                $scope.gallerySearch.results = [];
            });
        
        
        function updateMapSearchService() {
            if ($scope.currentMap) {
                $scope.mapSearchService = mapSearchService.createFromMap($scope.currentMap.model);
            } else {
                $scope.mapSearchService = undefined;
            }
        }
        
        updateMapSearchService();
        
        $scope.$on('studio-library-changed', updateMapSearchService);
        $scope.$on('studio-map-changed', updateMapSearchService);
        
        $scope.mapSearch = searchServices.register(i18n("Properties"),
            function (query) {
            $scope.mapSearch.resultLimit = worldViewStudioConfig.search.defaultResultCount;
            $scope.mapSearch.results = [];
            if ($scope.mapSearchService) {
                return $scope.mapSearchService.search(query.text);
            }
            return [];
        },
            function (results) {
            $scope.mapSearch.results = results;
        },
            function () {
            $scope.mapSearch.results = [];
        });
        
        $scope.questionEngine = searchServices.register(i18n("Quick Answer"),
            // if the query represents a question, answer it
            function (query) {
            $scope.questionEngine.resultLimit = worldViewStudioConfig.search.defaultResultCount;
            $scope.questionEngine.results = [];
            // verify that question is supported
            if (!('isValidQuestion' in $pyx.engine)) {
                return [];
            }
            if ($pyx.engine.isValidQuestion(query.text)) {
                return [{ Type: "Question", Metadata: { Name: "Ask!" }, Question: query.text }];
            } else {
                var suggestions = $pyx.engine.getQuestionSuggestions(query.text);
                var results = [];
                // NH: Temporary disable the question engine suggestions in the Studio 12/08/2015
                //angular.forEach(suggestions, function(suggestion) {
                //    results.push(
                //        {
                //            Type: "QuestionSuggestion",
                //            Metadata: { Name: suggestion.length < 30 ? suggestion : "..." + $filter('limitTo')(suggestion, -30) },
                //            Suggestion: suggestion
                //        });
                //});
                
                return results;
            }
        },
            //on-result
            function (results) {
            $scope.questionEngine.results = results;
        },
            //on-error
            function (query) {
            $scope.questionEngine.results = [];
        });
        
        $scope.geocodeService = geocodeService;
        
        $scope.services = searchServices.getServices();
        
        $scope.search = "";
        $scope.searchResultWindowActive = false;
        $scope.currentSearchResult = undefined;
        
        $scope.selectSearchResult = function (result) {
            if (result 
                && result.DataType === "Local" 
                && (!$scope.currentSearchResult 
                    || !$scope.currentSearchResult.Metadata 
                    || result.Metadata.Name !== $scope.currentSearchResult.Metadata.Name) 
                && result.DataSets.length > worldViewStudioConfig.search.longRenderThreshold) {
                $scope.currentSearchResult = {
                    "DataSets": [],
                    "Metadata": {
                        "Name": result.Metadata.Name,
                        "Description": "Loading data sets..."
                    },
                    "DataType": "Local",
                    "Type": "Catalog"
                };
                $timeout(function () {
                    $scope.currentSearchResult = result;
                }, worldViewStudioConfig.search.longRenderDelay);
            } else {
                $scope.currentSearchResult = result;
            }
            
            if (result && result.Type === "Search") {
                $scope.search = result.Text;
                $scope.searchChanged(worldViewStudioConfig.search.searchClearDelay);
            }
            if (result && result.Type === "QuestionSuggestion") {
                $scope.search = result.Suggestion;
                $scope.searchChanged(worldViewStudioConfig.search.searchClearDelay);
            }
            // inform the listeners that a search result has been selected
            $scope.$broadcast('search-result-selected', { resource: result });
        };
        
        $scope.handleSearchAction = function (action, resource) {
            switch (action) {
                case "import":
                    if (resource.Id && resource.Metadata.Name) {
                        // log event for import of worldview.gallery Resources
                        $rootScope.gaEvent('search-gallery-import', resource.Metadata.Name);
                    }
                    $scope.addResource(resource);
                    break;
                case "goto":
                    if (resource.Camera) {
                        $scope.gotoCamera(resource.Camera);
                    } else if (resource.Viewport) {
                        $scope.gotoGeometry(resource.Viewport);
                    } else {
                        $scope.gotoItem(resource);
                    }
                    $scope.hideSearchWindow();
                    break;
                case "toggle":
                    $scope.toggleMapItem(resource);
                    // Note: we don't hide the search window in this case, this should improve the user experience
                    break;
                case "addItemToIndex":
                    $scope.addItemToIndex(resource, resource.DataType);
                    break;
                case "removeItemFromIndex":
                    if (resource && resource.Uri) {
                        $scope.removeItemFromIndex(resource.Uri);
                    } else if (resource && resource.Id) {
                        $scope.removeItemFromIndex(resource.Id);
                    } else {
                        $scope.removeItemFromIndex(resource);
                    }
                    break;
                case "reload":
                    return $scope.reloadResource(resource);
                case "addWidget":
                    $scope.currentMap.dashboard(0).addWidget(resource.type, resource.item, resource.property.Name, resource.property.Metadata.Name);
                    if ($scope.dashboard) {
                        $scope.dashboard.minimized = false;
                    }
                    $scope.notifyLibraryChange();
                    break;
                case "performWhere":
                    $scope.hideSearchWindow();
                    $scope.search = "";
                    
                    var geometry = $scope.currentMap.dashboard(0).getModel().Selection.Geometry;
                    var condition = $pyx.area.condition(resource.item.Resource, resource.property.Name, resource.range);
                    $scope.currentMap.dashboard(0).setSelection($pyx.area.intersection(geometry, condition));
                    break;
                case "changeSelection":
                    $scope.hideSearchWindow();
                    $scope.currentMap.dashboard(0).setSelection(resource);
                    $pyx.globe.gotoGeometry(resource, worldViewStudioConfig.globe.gotoCameraAnimationDuration);
                    $scope.notifyLibraryChange();
                    break;
            }
        };
        
        $scope.showSearchWindow = function () {
            $scope.searchResultWindowActive = true;
            $scope.$broadcast('search-result-active');
            
            //hide properties window as it appear on top of the search result...
            $scope.hidePropertiesWindow();

        };
        
        $scope.showSearchWindowIfNeeded = function () {
            if ($scope.search.length && !$scope.searchResultWindowActive) {
                $scope.showSearchWindow();
            }
            $scope.$broadcast('search-result-active');
        };
        
        $scope.hideSearchWindow = function (ev) {
            if (!ev || !ev.which) {
                //Hide search window only if mouse has no pressed buttons.
                $scope.searchResultWindowActive = false;
            }
        };

        $scope.searchIconClick = function () {
            if ($scope.search.length === 0) {
                $scope.searchChanged(worldViewStudioConfig.search.searchClearDelay);
                $scope.showSearchWindow();
                $pyxIntercom.track('search-click-magnify');
            } else {
                $scope.search = "";
                $scope.searchChanged(worldViewStudioConfig.search.searchClearDelay);
                $scope.hideSearchWindow();
                $pyxIntercom.track('search-click-x');
            }
        };
        
        var searchChangePromise = undefined;
        
        // indicates whether the last search query is associated with any particular search services
        $scope.matchedSearchServices = [];
        // checks whether the specified search service's results are to be shown
        $scope.searchServiceFilter = function (serviceName) {
            return !$scope.matchedSearchServices.length || ($scope.matchedSearchServices.indexOf(serviceName) !== -1);
        }
        // finds particular search services that are associated with a provided query
        $scope.detectSearchServicesFromQuery = function (query) {
            // reset the previous matches
            $scope.matchedSearchServices = [];
            if (!query.text.length) {
                return;
            }
            // check if it's an OGC service URL
            if (('isOgcUrl' in $pyx.engine) && $pyx.engine.isOgcUrl(query.text)) {
                $scope.matchedSearchServices.push(onlineGeospatialService.name);
            }
            // check if it's a GeoServices URL
            if (('isGeoServiceUrl' in $pyx.engine) && $pyx.engine.isGeoServiceUrl(query.text)) {
                $scope.matchedSearchServices.push(onlineGeospatialService.name);
            }
            // check if it's a local path
            if (("toLocalResource" in $pyx.engine) && $pyx.engine.toLocalResource(query.text)) {
                $scope.matchedSearchServices.push(localSearch.name);
            }
        }

        var doSearch = function () {
            if (searchChangePromise) {
                $timeout.cancel(searchChangePromise);
            }
            var metadata = {
                search: $scope.search
            };
            $pyxIntercom.track('enter-search-string', metadata);
            $scope.selectSearchResult(undefined);
            var query = searchServices.createQuery($scope.search, $scope.selectedArea);
            searchServices.search(query);
            // verify if the query belongs particularly to one of the search services
            $scope.detectSearchServicesFromQuery(query);
        };
        $scope.isSearchInProgress = function() {
            return searchServices.isSearchInProgress();
        }


        var lastSearchValue = "";
        
        $scope.handleSearchKey = function (event) {
            if (event.keyCode === 27) { //escape
                $scope.search = "";
                $scope.hideSearchWindow();
                angular.forEach($scope.services, function (service) { service.results = [] });
            }
            if (event.keyCode === 8) { //backspace
                if (lastSearchValue === "" && $scope.searchTags) {
                    $scope.removeSearchTag($scope.searchTags[$scope.searchTags.length - 1]);
                }
            }
            lastSearchValue = $scope.search;
        };
        
        $scope.searchChanged = function (delay) {
            delay = delay || worldViewStudioConfig.search.searchDelay;
            if (searchChangePromise) {
                $timeout.cancel(searchChangePromise);
            }
            searchChangePromise = $timeout(function () { doSearch(); }, delay);
            $scope.showSearchWindow();
        };
        
        $scope.addResource = function (resource, style) {
            if (resource.Type === 'GeoSource') {
                $scope.addGeoSource(resource, style);
            } else if (resource.Type === 'Feature') {
                $scope.addFeature(resource);
            } else if (resource.Type === 'Map') {
                $scope.addMap(resource);
            } else if (resource.Type === 'Catalog') {
                angular.forEach(resource.DataSets, function (dataSet) { $scope.addDataSet(dataSet); });
            } else if (resource.Type === 'DataSet') {
                $scope.addDataSet(resource);
            } else if (resource.Type === 'Expression') {
                $scope.addExpression(resource);
            } else {
                $scope.notifyInfo(i18n('Importing %s is not supported.', i18n(resource.Type)));
            }
            if (resource.Type && resource.Type != 'GeoSource') {
                var metadata = {
                    resourceType: resource.Type
                };
                
                $pyxIntercom.track('add-resource', metadata);
            }
        };
        
        $scope.addGeoSource = function (geoSource, style) {
            //hide search results window if this is the current search results...
            if ($scope.currentSearchResult === geoSource) {
                $scope.hideSearchWindow();
            }
            
            if (geoSource.Definition && geoSource.ProcRef) {
                if (!$scope.currentMap) {
                    $scope.createNewMap();
                }
                
                var currentMap = $scope.currentMap;
                var group = currentMap.getDefaultGroup();
                
                $scope.$broadcast('import-started', { resource: geoSource });
                currentMap.addItem(group, geoSource, style)
                    .then(
                    function (item) {
                        // Create sensible default values for Coverages
                        // TODO: [idan] - this broke my local WV import on several files. feature disabled
                        if ($scope.activeFeatures.refinePaletteOnImport && false) {
                            var geoSourceStyle = $pyx.obj.has(item.Style, 'Fill') ? item.Style.Fill : item.Style.Icon;
                            
                            // Ensure the correct output type
                            if (geoSourceStyle && item.Specification.OutputType === 'Coverage') {
                                var defaultPalette = geoSourceStyle.Palette;
                                var fieldSpecIndex;
                                
                                fieldSpecIndex = $pyx.array.firstIndex(item.Specification.Fields,
                                        function (field) {
                                    return field.Name === geoSourceStyle.PaletteExpression;
                                });
                                
                                if (fieldSpecIndex !== -1) {
                                    var steps = defaultPalette.Steps.length;
                                    
                                    for (var i = 0; i < steps; i++) {
                                        defaultPalette.Steps[i].Value = (i + 0.0) / steps;
                                    }
                                    
                                    // Convert any uppercase tags to lowercase
                                    var processTags = function (tags) {
                                        var tempArray = [];
                                        var tagNumber = tags.length;
                                        
                                        for (var i = 0; i < tagNumber; i++) {
                                            tempArray.push(tags[i].toLowerCase());
                                        }
                                        
                                        return tempArray;
                                    };
                                    
                                    var metadataTags = processTags($pyx.tags(item.Metadata.Tags).all());
                                    
                                    item.Style.ShowAsElevation = $pyx.tags(metadataTags).exists("elevation");
                                    $scope.styleModifier.updatePalette(item, defaultPalette, {useScreenBasedStyling: true});
                                }
                            }
                        }

                        var metadata;
                        if (item.Resource && item.Metadata) {
                            metadata = {
                                geosourceGuid: item.Resource.Id,
                                geosourceType: item.Resource.Type,
                                geosourceName: item.Metadata.Name,
                            };
                        }
                        
                        $pyxIntercom.track('add-geosource', metadata || {});
                        
                        //everything is great the item was imported ok
                        $scope.notifyLibraryChange();
                        $scope.$broadcast('import-succeeded', { resource: geoSource });
                        $scope.notifyInfo(i18n('GeoSource %s imported successfully', geoSource.Metadata.Name));
                    },
                        function (itemAndError) {
                        //we failed to import the item
                        currentMap.removeItem(group, itemAndError.item);
                        $scope.$broadcast('import-failed', { resource: geoSource });
                        $scope.notifyError(i18n('Failed to import %s. It has been removed from globe', itemAndError.item.Metadata.Name));
                    });
            } else {
                $pyx(geoSource).refresh().success(function (updated) {
                    if (updated.Definition && updated.ProcRef) {
                        $scope.addGeoSource(updated);
                    }
                });
            }
        };
        
        $scope.addFeature = function (feature) {
            var featureCollection = {
                type: "FeatureCollection",
                features: [feature.Feature]
            };
            
            $pyx.engine.createFeatureCollection(featureCollection).success(function (geoSource) {
                angular.extend(geoSource.Metadata, feature.Metadata);
                
                if (!$scope.currentMap) {
                    $scope.createNewMap();
                }
                
                $scope.currentMap.importGeoSource(geoSource).success(function (style) {
                    $scope.addResource(geoSource, style);
                }).error(function (error) {
                    $scope.notifyError(i18n('Failed to import %s. It has been removed from globe', feature.Metadata.Name));
                    // it could be a network issue - inform the network verification service (without providing a message to the user)
                    $scope.alertNetworkServiceState('import', false, null);
                });
            }).error(function (error) {
                $scope.notifyError(i18n('Failed to import %s. It has been removed from globe', feature.Metadata.Name));
            });
        }
        
        $scope.addExpression = function (expression) {
            $scope.hideSearchWindow();
            
            var geoSources = [];
            var promises = [];
            var defaultStyle = undefined;
            
            //resolve all GeoSources
            angular.forEach(expression.MapItems, function (item,index) {
                if (!defaultStyle && item.Style) {
                    defaultStyle = item.Style;
                }
                promises.push($scope.currentMap.getGeoSource(item).then(function (geoSource) {
                    geoSources[index] = geoSource;
                }));
            });
            
            $q.all(promises).then(function () {
                $pyx.when($pyx.engine.calculate(geoSources, expression.Expression)).then(function (newGeoSource) {
                    $scope.currentMap.importGeoSource(newGeoSource, undefined, defaultStyle).success(function (style) {
                        $scope.addResource(newGeoSource, style);
                    });
                }, function (error) {
                    $scope.notifyError(i18n('Failed to calculate expression'));
                    // it could be a network issue - inform the network verification service (without providing a message to the user)
                    $scope.alertNetworkServiceState('import', false, null);
                });
            }, function (error) {
                $scope.notifyError(i18n('Failed to calculate expression'));
            });
        }
        
        $scope.addDataSet = function (dataSetResource) {
            // Note: do not close the search window on import to improve the user experience
            
            if (!$scope.currentMap) {
                $scope.createNewMap();
            }
            
            $scope.$broadcast('import-started', { resource: dataSetResource });
            
            var importAction = $scope.importDataSet(dataSetResource);
            
            importAction.promise
                .then(
                function () {
                    $scope.$broadcast('import-succeeded', { resource: dataSetResource });
                    $scope.notifyInfo(i18n('GeoSource %s imported successfully', dataSetResource.Metadata.Name));
                },
                    function (error) {
                    $scope.$broadcast('import-failed', { resource: dataSetResource });
                    var message = error === i18n('Cancelled by the user')
                            ? 'Cancelled import of %s. It has been removed from globe'
                            : 'Failed to import %s. It has been removed from globe';
                    $scope.notifyError(i18n(message, dataSetResource.Metadata.Name));
                }
            );
        };
        
        $scope.importingMaps = [];
        
        $scope.addMap = function (mapResource) {
            $scope.hideSearchWindow();
            
            if (!('upgradeMap' in $pyx.engine)) {
                $scope.notifyError(i18n('Importing globe is not supported in this version'));
                return;
            }
            
            if (mapResource.Definition && mapResource.ProcRef ||
                mapResource.Groups && mapResource.Groups.length > 0) {
                
                $scope.importingMaps.unshift(mapResource);
                
                $pyx.engine.upgradeMap(mapResource).success(function (map) {
                    $timeout(function () {
                        var index = $scope.importingMaps.indexOf(mapResource);
                        $scope.importingMaps.splice(index, 1);
                        map.Expanded = true;
                        $scope.library.Maps.unshift(map);
                        
                        $scope.currentMap = mapFactory.load($scope.library.Maps[0]);
                        $scope.currentMap.show();
                        $scope.notifyLibraryChange();
                        
                        if (map.Camera) {
                            $pyx.globe.setCamera(map.Camera, worldViewStudioConfig.globe.gotoCameraAnimationDuration);
                        }
                    });
                }).error(function (error) {
                    $timeout(function () {
                        var index = $scope.importingMaps.indexOf(map);
                        $scope.importingMaps.splice(index, 1);
                        $scope.notifyError(i18n('Importing globe %s has failed', map.Metadata.Name));
                        // inform the network verification service (without providing a message to the user)
                        $scope.alertNetworkServiceState('import', false, null);
                    });
                });
            } else {
                $pyx(mapResource).refresh().success(function (updated) {
                    if (updated.Definition && updated.ProcRef) {
                        $scope.addMap(updated);
                    }
                }).error(function (error) {
                    $scope.notifyError(i18n('Importing globe %s has failed', map.Metadata.Name));
                });
            }
        };
        
        $scope.reloadResource = function (resource) {
            if (!resource) {
                return resource;
            }
            switch (resource.Type) {
                case "Catalog":
                    var resourceDataTypes = {};
                    resourceDataTypes.OGC = function () {
                        return onlineGeospatialService.reloadServer(resource);
                    }
                    resourceDataTypes.ArcGIS = function () {
                        return onlineGeospatialService.reloadServer(resource);
                    }
                    resourceDataTypes.Local = function () {
                        return localSearch.reloadLocalResource(resource);
                    }
                    resourceDataTypes.NoType = function () {
                        // Note: add a short delay to make an impression for the user that something is really going on
                        return $pyx.when($timeout(function () { return resource; }, 1000));;
                    }
                    return resourceDataTypes[resource.DataType || 'NoType']();

                default:
                    return resource;
            }
        }
    };
    
    return {
        register: register
    };
});