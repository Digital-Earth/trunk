app.service("featureImport", function ($pyx, $pyxconfig, $filter, $timeout, $q, $pyxIntercom, importService) {
    var i18n = $filter("i18n");

    function register($scope) {
        $scope.addTemplate('widgets', '/client/templates/studio/template/feature-drag-drop-files.html');

        $scope.cancelItemImport = function (item) {
            $scope.currentMap.cancelImport(item, i18n('Cancelled by the user'));
            $scope.$broadcast('import-cancelled', { item: item });
        };

        $scope.$on("pyx-engine-ready", function () {

            function trackImportItemRequests(importAction) {
                importAction.onRequest = function (request, args) {
                    if (request === "SRS") {
                        if ($scope.srs.applyToAll) {
                            importAction.provide("SRS", $scope.srs);
                        } else {
                            $scope.notifyInfo(i18n("The data set you imported does not specify a Spatial Reference System, which is necessary in order to know where to display it on the Globe."));
                        }
                    }
                    if (request === "GeoTag") {
                        $scope.notifyInfo(i18n("The data set you imported does not specify a GeoTag method, which is necessary in order to know where to display it on the Globe."));
                    }
                }
            }

            $scope.importDataSet = function (dataSet) {
                if (!$scope.currentMap) {
                    $scope.createNewMap();
                }

                //reset the apply to all on first import
                if ($scope.currentMap.getImportQueueLength() === 0) {
                    $scope.srs.applyToAll = false;
                }

                var importAction = $scope.currentMap.import(dataSet);
                importAction.promise.then(function () {
                    //if import completed - notify library change
                    $scope.notifyLibraryChange();
                }).catch(function (error) {
                    // failure to import, display error message
                    $scope.notifyError(error);
                });
                trackImportItemRequests(importAction);
                return importAction;
            }

            $scope.importSupported = importService.supported() && $scope.activeFeatures.dragAndDrop;
            if ($scope.importSupported) {
                $scope.imports = [];

                var setupGeotagReferences = function () {
                    var references = [];
                    var loadingActions = [];

                    if ($scope.currentMap) {
                        angular.forEach($scope.currentMap.items(), function (item) {
                            if (!item.Specification) {
                                return;
                            }
                            angular.forEach(item.Specification.Fields, function (field) {
                                if ($pyx.tags.itemSystemTags(field).exists($pyx.tags.ui.Geotagable)) {
                                    var promise = $scope.currentMap.getGeoSource(item).then(function (geoSource) {
                                        references.push({
                                            ReferenceGeoSource: geoSource,
                                            ReferenceFieldName: field.Name
                                        });
                                    });
                                    loadingActions.push(promise);
                                }
                            });
                        });
                    }

                    return $q.all(loadingActions).then(function () {
                        $pyx.engine.setGeoTagReferenceGeoSources(references);
                        return true;
                    });
                };

                $scope.reportImportStatus = function (importCount) {
                    if (importCount === 0) {
                        $scope.notifyError(i18n("No supported file formats found to import"));
                    } else if (importCount === 1) {
                        $scope.notifyInfo(i18n("Importing one supported data set into WorldView.Studio"));
                    } else {
                        $scope.notifyInfo(i18n("Importing %d supported data sets into WorldView.Studio", importCount));
                    }
                };

                $scope.startImportDataSets = function (dataSets) {
                    var metadata = {
                        dataSetCount: dataSets.length
                    };
                    
                    $pyxIntercom.track('import-local', metadata);

                    setupGeotagReferences().then(function() {
                        var importCount = 0;
                        var dataSourceChecks = [];
                        angular.forEach(dataSets, function(dataSet) {
                            var promise = $timeout(function() {
                                importCount++;
                                $scope.importDataSet(dataSet);
                            });
                            dataSourceChecks.push(promise);
                        });
                        $q.all(dataSourceChecks).then(function() {
                            $scope.reportImportStatus(importCount);
                        });
                    });
                };

                // a helper method to support the method startImportFiles
                var pathToDataSet = function(path) {
                    return {
                        "Uri": path,
                        "Type": "DataSet"
                    }
                }

                // TODO:
                // DEPRECATED - get rid of this method when integrating drag&drop with the search results window
                $scope.startImportFiles = function (files) {
                    setupGeotagReferences().then(function () {
                        var importCount = 0;

                        if ("isDataSourceSupported" in $pyx.engine) {
                            var dataSourceChecks = [];
                            angular.forEach(files, function (file) {
                                var promise = $pyx.when($pyx.engine.isDataSourceSupported(file)).then(function (supported) {
                                    if (supported) {
                                        importCount++;
                                        $timeout(function () {
                                            $scope.importDataSet(pathToDataSet(file));
                                        });
                                    }
                                });

                                dataSourceChecks.push(promise);
                            });

                            $q.all(dataSourceChecks).then(function () {
                                $scope.reportImportStatus(importCount);
                            });
                        } else {
                            // deprecated - for backwards compatibility
                            // do simple file extension check
                            angular.forEach(files, function (file) {
                                var ext = file.split(".").pop().toLowerCase();

                                if (ext in $scope.supportedImportFileFormats) {
                                    importCount++;
                                    $scope.importDataSet(file);
                                }
                            });

                            $scope.reportImportStatus(importCount);
                        }
                    });
                };

                $scope.srs = {
                    'CoordinateSystem': "Geographical",
                    'Datum': "WGS84"
                };

                $scope.geotag = {
                    'option': null
                };

                $scope.provideSrs = function (item, srsSettings, popup) {
                    if (!srsSettings) {
                        item.provide("SRS", null);
                        popup.hide();
                    } else {
                        item.provide("SRS", srsSettings).then(function () {
                            popup.hide();
                        });
                    }
                };

                $scope.selectDefaultGeoTag = function (options) {
                    options = options || [];
                    if (options.length > 0 &&
                        options.indexOf($scope.geotag.option) === -1) {
                        $scope.geotag.option = options[0];
                    }
                };

                $scope.provideGeoTag = function (item, geoTagOption, popup) {
                    if (!geoTagOption) {
                        item.provide("GeoTag", null);
                        popup.hide();
                    } else {
                        item.provide("GeoTag", geoTagOption).then(function () {
                            popup.hide();
                        });
                        $pyxIntercom.track('select-geocode-method');
                    }
                };

                // deprecated - for backwards compatibility
                $pyx.engine.supportedImportFileFormats().success(function (result) {
                    $timeout(function () {
                        $scope.supportedImportFileFormats = {};
                        angular.forEach(result, function (ext) {
                            $scope.supportedImportFileFormats[ext] = true;
                        });
                    });
                });
            }

            if ("onCommandLine" in $pyx.application && $scope.activeFeatures.commandLineHandling) {
                $scope.handleCommandLine = function () {
                    var args = $pyx.application.getNextCommandLine();
                    if (args.length) {
                        //handle pyxis:// commands
                        if (args[0].indexOf("pyxis://") === 0) {

                            //bring window to front
                            $pyx.application.bringToFront();

                            var argument = args[0].replace("pyxis://", "");
                            var failedMessage = i18n("Failed to handle command: ") + args.join(" ");

                            var backendHost = $pyxconfig.backendUrl.replace("https://", "").replace("http://", "");
                            if (argument.indexOf(backendHost) === 0) {
                                var regEx = new RegExp(backendHost.replace(".", "\\.").replace("/", "\\/") + "\\/api\\/v\\d\\/(GeoSource|Map)\\/([^\\/\\?]*)", "i");
                                var parts = regEx.exec(argument);
                                //parts[0] - match
                                //parts[1] - resource type
                                //parts[2] - id
                                //version is not supported yet - we assume latest at the moment
                                if (parts) {
                                    var query = undefined;
                                    if (parts[1] === "GeoSource") {
                                        query = $pyx.gallery.geoSources();
                                    } else if (parts[1] === "Map") {
                                        query = $pyx.gallery.maps();
                                    }
                                    if (query) {
                                        query.getById(parts[2]).success(function (resource) {
                                            $scope.addResource(resource);

                                        }).error(function (error) {
                                            $scope.notifyError(failedMessage);
                                        });
                                    } else {
                                        $scope.notifyError(failedMessage);
                                    }
                                } else {
                                    $scope.notifyError(failedMessage);
                                }
                            } else {
                                //extract procref
                                var procRef = argument;

                                $pyx.gallery.resources().getByProcRef(procRef).success(function (resource) {
                                    $pyxIntercom.track('import-gallery');

                                    //when resource.Type is 'GeoSource'
                                    var handleGeoSource = function (resource) {
                                        var inMap;

                                        //if a map is active - check if the
                                        //resource has already been added
                                        if ($scope.currentMap) {
                                            inMap = $scope.resourceInMap(resource, true);

                                            if (!inMap) {
                                                $scope.addResource(resource);
                                            } else {
                                                $scope.notifyWarning("That GeoSource is already in your Map");
                                            }
                                        } else {
                                            $scope.addResource(resource);
                                        }

                                    }

                                    //when resource.Type is 'Map'
                                    var handleMap = function (resource) {
                                        var inLibrary = $scope.mapInLibrary(resource);

                                        //check if the resource matches any 
                                        //maps in the user's library
                                        if (!inLibrary) {
                                            $scope.addResource(resource);
                                        } else {
                                            $scope.notifyWarning("That Map is already in your Library");
                                            $scope.triggerCurrentMap(resource.Id);
                                        }
                                    }

                                    if (resource.Type) {
                                        switch (resource.Type) {
                                            case "GeoSource":
                                                handleGeoSource(resource);
                                                break;
                                            case "Map":
                                                handleMap(resource);
                                                break;
                                        }

                                    } else {
                                        $scope.notifyError(failedMessage);
                                    }
                                }).error(function (error) {
                                    $scope.notifyError(failedMessage);
                                });
                            }
                        }

                        //schedule next command line handling (with a small delay)
                        $timeout(function () {
                            $scope.handleCommandLine();
                        }, 500);
                    }
                };

                $pyx.application.onCommandLine(function () {
                    $timeout(function () {
                        //schedule next command line handling
                        $scope.handleCommandLine();
                    });
                });

                //try to handle first command if any
                $scope.handleCommandLine();

            }
        });
    };

    return {
        register: register
    };
});