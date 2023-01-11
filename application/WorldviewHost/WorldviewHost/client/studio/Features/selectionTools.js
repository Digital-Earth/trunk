app.service("featureSelectionTools", function ($pyx, $filter, $q, $timeout, $pyxIntercom, styleOptions, dispatcher) {
    var i18n = $filter("i18n");

    function register($scope) {
        $scope.addTemplate('sections', '/client/templates/studio/template/feature-selection-tools-sections.html');
        $scope.addTemplate('dialogs', '/client/templates/studio/template/feature-selection-tools-dialogs.html');

        //selection tools store stuff for now
        var selectionToolsStore = {
            handle: {}
        };
        dispatcher.registerStore("selectionToolsStore", selectionToolsStore);

        $scope.tools = {
            arrow: {
                name: "arrow",
                enabled: function () { return true; },
                setup: function () {},
                handleClick: function (index) {}
            },
            'magic-wand': {
                name: "magic-wand",
                enabled: function () { return $scope.currentMapHasFeatures(); },
                setup: function () {},
                handleClick: function (index, event) {
                    $pyxIntercom.track('select-feature-magic-wand');

                    var items = $scope.currentMap.activeItems();

                    if (items.length === 0) {
                        return;
                    }

                    var completed = 0;
                    var deferred = $q.defer();
                    var results = [];

                    var addCompleted = function () {
                        completed++;
                        if (items.length === completed) {
                            deferred.resolve(results);
                        }
                    };

                    angular.forEach(items, function (item) {
                        $scope.currentMap.getDefinition(item).then(function (definition) {
                            if (definition.OutputType === "Feature") {
                                $scope.currentMap.getGeoSource(item).then(function (geoSource) {
                                    $pyx.engine.getFeatures(geoSource, index).success(function (fc) {
                                        if (fc && fc.features.length) {
                                            var f = fc.features[0];
                                            results.push({ item: item, geoSource: geoSource, feature: f });
                                        }
                                        addCompleted();
                                    }).error(function (error) {
                                        addCompleted();
                                    });
                                });
                            } else {
                                addCompleted();
                            }
                        });
                    });

                    deferred.promise.then(function (results) {
                        if (results.length === 1) {
                            var area = {
                                "createdByTool": "magic-wand",
                                "type": "FeatureRef",
                                "resource": { "Type": "GeoSource", "Id": results[0].geoSource.Id, "Version": results[0].geoSource.Version },
                                "id": results[0].feature.id
                            };
                            $scope.currentMap.dashboard(0).setSelection(area);
                        } else if (results.length > 1) {
                            $scope.magicwandPopupMenu = {
                                active: true,
                                results: results,
                                left: event.clientX,
                                top: event.clientY - 10
                            };
                            $scope.selectFeature = function (result) {
                                var area = {
                                    "createdByTool": "magic-wand",
                                    "type": "FeatureRef",
                                    "resource": { "Type": "GeoSource", "Id": result.geoSource.Id, "Version": result.geoSource.Version },
                                    "id": result.feature.id
                                };
                                $scope.currentMap.dashboard(0).setSelection(area);
                            };
                        } else {
                            $scope.currentMap.dashboard(0).setSelection(undefined);
                        }
                    }).then(function () {
                        $scope.selectTool("arrow");
                    });
                }
            },
            freehand: {
                name: "freehand",
                enabled: function () { return true; },
                setup: function () {
                    $scope.clearPoints();
                },
                handleClick: function (index) {
                }
            },
            polygon: {
                name: "polygon",
                enabled: function () { return true; },
                setup: function () {
                    $scope.clearPoints();
                    $scope.points.push([0, 0]);
                },
                handleClick: function (index) {
                }
            },
            watershed: {
                name: "watershed",
                enabled: function () { return $scope.currentMapHasElevation(); },
                setup: function () {},
                handleClick: function (index) {
                    $scope.toolBusy = true;
                    $pyx.engine.createWatershedGeometry(index).success(function (geometry) {
                        $timeout(function () {
                            geometry.createdByTool = "watershed";
                            $scope.toolBusy = false;
                            $scope.currentMap.dashboard(0).setSelection(geometry);
                        });
                    }).error(function (error) {
                        $timeout(function () {
                            $scope.toolBusy = false;
                            $scope.notifyError(i18n("Failed to create watershed"), error);
                        });
                    });
                    $scope.selectTool("arrow");
                    $pyxIntercom.track('select-watershed');
                }
            }
        };

        $scope.currentTool = $scope.tools.arrow;
        $scope.lastTool = "arrow";

        $scope.selectTool = function (name) {
            //check if new tool is enabled...
            if (!$scope.tools[name].enabled()) {
                return;
            }

            if ($scope.currentTool.name !== "arrow") {
                $scope.lastTool = $scope.currentTool.name;
            }
            $scope.currentTool = $scope.tools[name];
            $scope.currentTool.setup();
        };

        $scope.swapCurrentTool = function () {
            if ($scope.currentTool.name === "arrow") {
                $scope.selectTool($scope.lastTool);
            } else {
                $scope.selectTool("arrow");
            }
        };

        ///drawing

        $scope.points = [];

        $scope.clearPoints = function () {
            $scope.points = [];
            $scope.snapLocation = undefined;
        };

        //this is very ugly
        var divElement = undefined;

        $scope.addPoint = function ($event) {
            if ($event.which === 1) {
                divElement = $event.currentTarget;
                $event.preventDefault();
                if ($scope.points.length) {
                    var lastPoint = $scope.points[$scope.points.length - 1];
                    if (lastPoint[0] === $event.offsetX && lastPoint[0] === $event.offsetY) {
                        return;
                    }
                }
                $scope.points.push([$event.offsetX, $event.offsetY]);
            }
        };

        $scope.modifyLastPoint = function ($event) {
            if ($scope.points.length) {
                divElement = $event.currentTarget;
                var lastPoint = $scope.points[$scope.points.length - 1];
                lastPoint[0] = $event.offsetX;
                lastPoint[1] = $event.offsetY;

                if ($scope.snapLocation) {
                    lastPoint[0] = $scope.snapLocation[0];
                    lastPoint[1] = $scope.snapLocation[1];
                }
            }
        };

        $scope.snapLastPoint = function (should) {
            if ($scope.points.length > 1 && should) {
                $scope.snapLocation = $scope.points[0];
            } else {
                $scope.snapLocation = undefined;
            }
        };

        $scope.getPoints = function (from, to) {
            var str = "";
            from = from || 0;
            to = to || $scope.points.length;
            if (to < 0) {
                to = 0;
            }
            if (from < 0) {
                from = 0;
            };
            for (var i = from; i < to; i++) {
                var point = $scope.points[i];
                str += point[0] + "," + point[1] + " ";
            }
            return str;
        };

        $scope.commitPaint = function () {
            var screenPoints = [];
            var rect = divElement.getClientRects()[0];

            angular.forEach($scope.points, function (point) {
                screenPoints.push([point[0] + rect.left, point[1] + rect.top]);
            });

            $scope.clearPoints();
            var toolName = $scope.currentTool.name;

            $scope.selectTool('arrow');

            $pyx.globe.screenToGeographicPosition(screenPoints).success(function (points) {
                $timeout(function () {
                    if ($pyx.array.some(points, function (p) { return p == null; }) ||
                        points.length < 3) {
                        $scope.notifyError(i18n('Failed to create selection region'));
                    } else {
                        var geometry = $pyx.area.polygon([points]);
                        geometry.createdByTool = toolName;
                        $scope.currentMap.dashboard(0).setSelection(geometry);
                        $scope.notifyLibraryChange();
                        $pyxIntercom.track('create-geometry-with', { toolName: toolName });
                    }
                });
            }).error(function (error) {
                $timeout(function () {
                    $scope.notifyError(i18n('Failed to create selection region'));
                });
            });
        };

        $scope.closePolygon = function () {
            if ($scope.points.length > 3) {

                //remove the last point if we are "snapping"
                if ($scope.snapLocation) {
                    $scope.points.pop();
                }
                $scope.commitPaint();
            } else {
                $scope.notifyInfo(i18n('Polygon can not be closed'));
            }
        };

        $scope.cancelPaint = function () {
            $scope.clearPoints();
            $scope.selectTool('arrow');
        };

        $scope.hasSelection = function () {
            return $pyx.obj.get($scope.currentMap, 'model', 'Dashboards', 0, 'Selection', 'Geometry');
        }

        $scope.removeSearchTag = function (tag) {
            var index = $scope.searchTags.indexOf(tag);

            if (index !== -1) {
                if (index === 0) {
                    $scope.currentMap.dashboard(0).setSelection(undefined);
                } else {
                    var currentGeometry = $pyx.obj.get($scope.currentMap, 'model', 'Dashboards', 0, 'Selection', 'Geometry');

                    //remove operation from currentGeometry
                    if (currentGeometry && currentGeometry.type === "Boolean") {
                        currentGeometry.operations.splice(index, 1);
                        $scope.currentMap.dashboard(0).setSelection(currentGeometry);
                    }
                }
            }
        }

        $scope.isSelectionLoading = function () {
            return dispatcher.stores.selectionStore.get().state === "loading";
        }

        $scope.selection = {};

        function updateSelectionDetails() {
            dispatcher.waitForStore("searchTagsStore");
            $scope.searchTags = dispatcher.stores.searchTagsStore.get();

            dispatcher.waitForStore("selectionStore");
            if ($scope.searchTags.length) {
                $scope.selection.color = dispatcher.stores.selectionStore.getSelectionColor();
            } else {
                $scope.selection.color = undefined;
            }
        }

        selectionToolsStore.handle.changeMap = updateSelectionDetails;
        selectionToolsStore.handle.resourceResolvedCompleted = updateSelectionDetails;
        selectionToolsStore.handle.resourceSpecificationResolvedCompleted = updateSelectionDetails;

        selectionToolsStore.handle.changeSelection = function () {
            updateSelectionDetails();
            $scope.notifyLibraryChange();
        }

        $scope.changeSelectionColor = function (color) {
            var selection = angular.copy(dispatcher.stores.selectionStore.get().currentSelection);
            if (selection) {
                selection.Style.Line.Color = styleOptions.setAlpha(color, 1);
                selection.Style.Fill.Color = styleOptions.setAlpha(color, 0.3);
                dispatcher.actions.changeSelection.invoke({ 'selection': selection });
            }
        }


        $scope.$on("pyx-globe-ready", function () {
            $scope.handleGlobeClick = function (cursor, e, clickMetadata) {
                if (document) {
                    document.activeElement.blur();
                }
                $scope.cursor = cursor;
                $scope.hideSearchWindow();
                $scope.currentTool.handleClick($scope.cursor, JSON.parse(e));
                
                if ($scope.currentTool.name == 'arrow') {
                    if (clickMetadata && clickMetadata.icon && clickMetadata.isGroupIcon) {
                        var camera = $pyx.globe.getCamera();
                        camera.Latitude = clickMetadata.latlon.lat;
                        camera.Longitude = clickMetadata.latlon.lon;
                        camera.Range *= 0.4; // zoom in by about half
                        console.log("zoom to camera ", camera);
                        $pyx.globe.setCamera(camera, {duration: 350});
                        $scope.showPropWindow = false;
                    } else {
                        $scope.handleShowProperties(cursor,e);
                    }
                } else {
                    $scope.showPropWindow = false;
                }
            };
        });

        $scope.$on("pyx-globe-keyup", function ($event,$args) {
            if ($args.$event.keyCode === 32) { //Space 
                $scope.swapCurrentTool();
            }
            
            if ($args.$event.keyCode === 27) { //Escape
                $scope.showPropWindow = false;
            }
        });
    }

    return {
        depends: ["currentMap"],
        register: register
    };
});