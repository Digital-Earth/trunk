app.service("featureCurrentMap", function ($pyx, $filter, $q, dispatcher, mapFactory) {
    var i18n = $filter("i18n");

    function register($scope) {

        $scope.addTemplate('dialogs', '/client/templates/studio/template/feature-current-map.html');

        //studio stuff for now
        var studioStore = {
            handle: {}
        };
        dispatcher.registerStore("studioStore", studioStore);

        $scope.isCurrentMap = function (map) {
            if ($scope.currentMap) {
                return $scope.currentMap.model === map;
            }
            return false;
        };

        // This dialog will appear when the user is switching between maps while import
        // is in progress.
        //
        // This will break our import logic, and therefore we allow the user to:
        //  1) cancel switching maps and continue to import
        //  2) cancel all current imports and then switch to a new map.
        $scope.switchMapWhileImportDialog = {
            active: false,
            targetMap: undefined,
            commit: function () {
                $scope.switchMapWhileImportDialog.active = false;

                if ($scope.currentMap) {
                    $scope.currentMap.cancelAllImports(i18n("Cancelled by the user"));
                }

                $scope.setCurrentMap($scope.switchMapWhileImportDialog.targetMap);
            },
            cancel: function () {
                $scope.switchMapWhileImportDialog.active = false;
            }
        }

        $scope.setCurrentMap = function (map) {

            if ($scope.currentMap && $scope.currentMap.state.Imports.length > 0) {
                $scope.switchMapWhileImportDialog.targetMap = map;
                $scope.switchMapWhileImportDialog.active = true;
                return;
            }

            if (map) {
                try {
                    $scope.currentMap = mapFactory.load(map);
                    $scope.currentMap.verify();
                    $scope.currentMap.show();
                    if ($scope.dashboard) {
                        $scope.dashboard.minimized = $scope.currentMap.dashboard(0).getModel().Widgets.length === 0;
                        if ($scope.dashboard.minimized && $scope.currentMap.activeItems().length) {
                            $scope.popupDashboard();
                        }
                    }

                    $scope.notifyLibraryChange();
                } catch (error) {
                    console.log(error);
                    $scope.notifyError(i18n("Failed to load globe"));
                    $scope.setCurrentMap(undefined);
                }
            } else {
                $scope.currentMap = undefined;
                dispatcher.actions.changeMap.invoke({ map: undefined });
            }

            $scope.$emit("studio-map-changed");

            //hide properties window - as we switch to a new map
            $scope.hidePropertiesWindow();
        };

        /*
         - @name triggerCurrentMap
         - @desc triggers the click event on a map to setCurrentMap()
         - @param {string} mapId - the Map.Id
         */
        $scope.triggerCurrentMap = function (mapId) {
            var elMap = angular.element("#" + mapId);
            var elHandler = elMap.find(".map > div");

            $timeout(function () {
                elHandler.trigger("click");
            });
        }

        $scope.currentMapHasElevation = function () {
            if (!$scope.currentMap) {
                return false;
            }
            return $scope.currentMap.someItems(function (item) {
                //find an active item that ShowAsElevation set to true
                return (item.Active && item.Style && item.Style.ShowAsElevation);
            });
        };

        $scope.currentMapHasFeatures = function () {
            if (!$scope.currentMap) {
                return false;
            }
            return $scope.currentMap.someItems(function (item) {
                //find an active item that output features
                return (item.Active && item.Specification.OutputType === "Feature");
            });
        };

        $scope.allGroupItemsVisible = function (group) {
            var result = true;
            angular.forEach(group.Items, function (item) {
                if (!item.Active) {
                    result = false;
                }
            });
            return result;
        };

        $scope.hideGroup = function (group) {
            $scope.currentMap.hideGroup(group);
        };

        $scope.showGroup = function (group) {
            $scope.currentMap.showGroup(group);
        };


        studioStore.handle.updateGeoSourceVisualizationCompleted = function (action) {
            //popupDashboard when we just showed a new geoSource
            if (action.data.action === "show") {
                if ($scope.popupDashboard) {
                    $scope.popupDashboard();
                }
            }

            if (action.data.action === "update") {
                var item = dispatcher.stores.currentMapStore.getItem(action.data.visibleId);
                var palette = $pyx.obj.get(action.data.style, 'Icon', 'Palette') || $pyx.obj.get(action.data.style, 'Fill', 'Palette');
                $scope.notifyStyleChange(item, palette);
                $scope.notifyLibraryChange();
            }
        }

        studioStore.handle.updateGeoSourceVisualizationFailed = function (action) {
            var id = action.data.visibleId || (action.data.geoSource && action.data.geoSource.Id);
            var item = dispatcher.stores.currentMapStore.getItem(id);
            var name = item ? item.Metadata.Name : "";

            switch (action.data.action) {
                case "show":
                    $scope.notifyError(i18n('Failed to show globe item %s', name));
                case "hide":
                    $scope.notifyError(i18n('Failed to hide globe item %s', name));
                case "update":
                    $scope.notifyError(i18n('Failed to update globe item %s', name));
            }
        }

        $scope.toggleMapItem = function (item) {
            //if item is not ready we don't allow it to be toggle.
            if (!$scope.currentMap.isItemReady(item)) {
                return;
            }
            if (item.Active) {
                dispatcher.actions.hideItem.invoke({ 'item': item });
            } else {
                dispatcher.actions.showItem.invoke({ 'item': item });
            }
            //notify library update
            $scope.notifyLibraryChange();
        };

        $scope.removeMapItem = function (group, item, index) {
            $scope.currentMap.removeItem(group, item);
            $scope.$broadcast('map-item-removed', { item: item });
            $scope.notifyLibraryChange();
        };

        $scope.removeMapGroup = function (map, group, index) {
            $scope.groupPopupMenu = false;
            angular.forEach(group.Items, function (item) {
                $scope.currentMap.removeItem(group, item);
            });
            map.Groups.splice(index, 1);
            $scope.notifyLibraryChange();
        };
    }

    return {
        depends: [],
        register: register
    }
});