app.service("featureLibrary", function ($pyx, $q, $filter, $timeout, $pyxIntercom, worldViewStudioConfig, mapFactory, dispatcher) {

    var i18n = $filter("i18n");

    function register($scope, options) {

        var defaultOptions = {
            useLocalStorage: true
        }
        options = angular.extend({},defaultOptions,options);

        $scope.addTemplate('sections', '/client/templates/studio/template/feature-library.html');
        $scope.addTemplate('widgets', '/client/templates/studio/template/feature-saving.html');

        $scope.libraryState = {
            minimized: false
        };

        /*
        - @name mapInLibrary
        - @desc Confirm if map is already in the library
        - @type {function}
        - @param {object} - map
        - @return {boolean}
        */
        $scope.mapInLibrary = function (map) {
            var inLibrary = false;

            angular.forEach($scope.library.Maps, function (mapObj, index) {
                if (mapObj.Type === 'Map') {
                    if (mapObj.Id === map.Id) {
                        inLibrary = true;
                    }
                }
            });

            return inLibrary;
        }

        /*
        - @name resourceInMap
        - @desc Confirm if resource is already in a map(s)
        - @type {function}
        - @param {object} - resource
        - @param {boolean} - mapState (check in 'active' vs. 'all' maps)
        - @return {boolean}
        */
        $scope.resourceInMap = function (resource, mapState) {
            var inMap = false;
            var groupItems;

            var checkResourceExists = function (resourceObj, index) {
                if (resourceObj.Type) {
                    if (resourceObj.Id === resource.Id) {
                        inMap = true;
                    }
                } else {
                    if (resourceObj.Resource.Id === resource.Id) {
                        inMap = true;
                    }
                }
            };

            if (mapState) {
                //check if the map is empty
                if (!$scope.currentMap.model.Groups || !$scope.currentMap.model.Groups[0]) {
                    return inMap;
                }
                groupItems = $scope.currentMap.model.Groups[0].Items;
                angular.forEach(groupItems, checkResourceExists);

            } else {
                angular.forEach($scope.library.Maps, function (mapObj, index) {
                    if (mapObj.BasedOn) {
                        angular.forEach(mapObj.BasedOn, checkResourceExists);
                    }
                });
            }

            return inMap;
        }

        $scope.createNewMap = function (mapName) {
            mapName = mapName || i18n("New Globe Name");

            var map = {
                Type: "Map",
                Metadata: {
                    Name: mapName
                },
                Expanded: true,
                Groups: []
            };

            if ("verifyMapSchema" in $pyx.engine) {
                map = $pyx.engine.verifyMapSchema(map);
                map.Expanded = true;
            }

            $scope.library.Maps.unshift(map);
            $scope.setCurrentMap($scope.library.Maps[0]);
            $pyxIntercom.track('create-new-map');
        };

        $scope.removeMap = function (map, index) {
            $scope.mapPopupMenu = false;
            $scope.library.Maps.splice(index, 1);
            if ($scope.currentMap && map === $scope.currentMap.model) {
                $scope.currentMap.cancelAllImports(i18n("Cancelled by the user"));
                $scope.currentMap = undefined;
                dispatcher.actions.changeMap.invoke({ map: undefined });
            }
            $scope.notifyLibraryChange();
        };

        $scope.gotoMap = function (map, time) {
            if (map.Camera && map.Camera.Range) {
                $pyx.globe.setCamera(map.Camera, time || worldViewStudioConfig.globe.gotoCameraAnimationDuration);
            } else {
                var activeItems = $scope.currentMap.activeItems();
                // If there's no default map camera - revert to Geosource default camera
                if (activeItems) {
                    $scope.gotoItem(activeItems[0]);
                }
            }
        };

        $scope.setMapCamera = function (map) {
            map.Camera = $pyx.globe.getCamera();
            map.Camera.cameraChanged = true;
            $scope.notifyLibraryChange();
        };

        $scope.duplicateMap = function (map, index) {
            var clone = angular.copy(map);
            var originalName = clone.Metadata.Name;
            clone.Metadata.Name = originalName + " Copy";

            $scope.library.Maps.splice(0, 0, clone);

            $scope.currentMap = mapFactory.load(clone);
            $scope.currentMap.show();
            $scope.notifyLibraryChange();
        };


        //units
        $scope.getFieldUnit = function (item, fieldName) {
            return $pyx.spec(item.Specification).unitNameOfField(fieldName);
        };


        $scope.notifyLibraryChange = function () {
            $scope.$emit("studio-library-changed");
            $scope.saving = true;
        };

        function connectToLibrary() {
            $pyx.application.load("library", $scope.library).success(function (value) {
                $timeout(function () {
                    $scope.library = value;
                });
            });

            function saveLibrary() {
                $scope.saving = false;
                console.log("auto saving");
                var cleanLibrary = JSON.parse(angular.toJson($scope.library));
                $pyx.application.save("library", cleanLibrary);
            }

            var autoSavePromise = undefined;

            function autoSave() {
                saveLibrary();
                autoSavePromise = $timeout(autoSave, worldViewStudioConfig.library.autoSaveTime);
            }

            autoSavePromise = $timeout(autoSave, worldViewStudioConfig.library.autoSaveTime);

            $scope.$on("studio-library-changed", function () {
                if (autoSavePromise) {
                    $timeout.cancel(autoSavePromise);
                }
                autoSavePromise = $timeout(autoSave, worldViewStudioConfig.library.delayAfterModification);
            });
        }

        $scope.$on("studio-setup-started", function () {
            if (options.useLocalStorage) {
               connectToLibrary();
            }
        });
    };

    return {
        register: register
    };
});