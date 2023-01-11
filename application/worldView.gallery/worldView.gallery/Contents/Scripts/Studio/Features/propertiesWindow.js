app.service("featurePropertiesWindow", function ($pyx, $timeout, $filter, $pyxIntercom, dispatcher, dispatcherPromise) {

    function register($scope) {
        $scope.addTemplate('dialogs', '/template/feature-properties-window.html');

        $scope.hidePropertiesWindow = function () {
            $scope.showPropWindow = false;
        };

        $scope.updateStyle = function (pickedItem, field, palette) {
            var orderBy = $filter('orderBy');
            var promise = dispatcherPromise.promise(function (action, deferred) {
                if (action.type === "updateGeoSourceVisualizationCompleted") {
                    dispatcher.waitForStore("currentMapStore");
                    if (action.data.visibleId) {
                        var item = dispatcher.stores.currentMapStore.getItem(action.data.visibleId);
                        if (item.Resource.Id === pickedItem.Item.Resource.Id) {
                            deferred.resolve(item.Style);
                        }
                    }
                }
                if (action.type === "updateGeoSourceVisualizationFailed") {
                    dispatcher.waitForStore("currentMapStore");
                    if (action.data.visibleId) {
                        var item = dispatcher.stores.currentMapStore.getItem(action.data.visibleId);
                        if (item.Resource.Id === pickedItem.Item.Resource.Id) {
                            deferred.reject(action.data.error);
                        }
                    }
                }
            });

            dispatcher.actions.calculateItemStyle.invoke({
                'item': pickedItem.Item,
                'field': field.Name,
                'palette': palette,
                'onScreen': false
            });

            promise.then(function (style) {
                pickedItem.Style = style;
                pickedItem.StyleInfo.Field = field.Name;

                if (pickedItem.Style.Icon && pickedItem.Style.Icon.Palette) {
                    pickedItem.StyleInfo.Palette = pickedItem.Style.Icon.Palette;
                } else if (pickedItem.Style.Fill && pickedItem.Style.Fill.Palette) {
                    pickedItem.StyleInfo.Palette = pickedItem.Style.Fill.Palette;
                }
                $scope.notifyLibraryChange();
            }, function (error) {
                $scope.notifyError('failed to update style');
            });
        };

        $scope.toggleFieldStar = function (pickedItem, field) {
            var fieldSpec = $pyx.spec(pickedItem.Item.Specification).field(field.Name);

            if (!fieldSpec) {
                return;
            }

            var result = $pyx.tags.itemSystemTags(fieldSpec).toggle($pyx.tags.ui.Favorite);
            field.Starred = result != undefined;
            $scope.notifyLibraryChange();
        };

        $scope.$on("pyx-globe-ready", function () {
            $scope.handleGlobeRightClick = function (cursor, e) {
                if (document) {
                    document.activeElement.blur();
                }

                $pyxIntercom.track('view-geosource-properties-on-globe');

                //if no map is selected - there is no properties to show
                if (!$scope.currentMap) {
                    return;
                }

                //if there is a properties window open already - close it 
                if ($scope.showPropWindow) {
                    $scope.showPropWindow = false;
                    $timeout(function () {
                        $scope.handleGlobeRightClick(cursor, e);
                    });
                    return;
                }

                $scope.cursor = cursor;
                $scope.propWindowPos = JSON.parse(e);
                $scope.propWindowPos.clientX += 10;
                $scope.showPropWindow = true;

                var items = $scope.currentMap.activeItems();

                $scope.pickedItems = [];

                angular.forEach(items, function (item) {
                    var pickedItem = {
                        'Item': item,
                        'Fields': [],
                        'ShownFields': [],
                        'HiddenFields': []
                    };

                    $scope.pickedItems.push(pickedItem);

                    var index = $scope.cursor;

                    $scope.currentMap.getDefinition(item).then(function (definition) {
                        pickedItem.Fields = definition.Fields;

                        if (item.ShowDetails === undefined) {
                            item.ShowDetails = true;
                        }

                        if ($pyx.spec(definition).starredFields().length === 0) {
                            for (var i = 0; i < 5 && i < definition.Fields.length; i++) {
                                $pyx.tags.itemSystemTags(definition.Fields[i]).toggle($pyx.tags.ui.Favorite);
                            }
                        }

                        return item.Style;
                    }).then(function (style) {
                        pickedItem.Style = style;
                        pickedItem.StyleInfo = {};

                        if (pickedItem.Style) {
                            if (pickedItem.Style.Icon && pickedItem.Style.Icon.Palette) {
                                pickedItem.StyleInfo.Palette = pickedItem.Style.Icon.Palette;
                                pickedItem.StyleInfo.Field = pickedItem.Style.Icon.PaletteExpression;
                            } else if (pickedItem.Style.Fill && pickedItem.Style.Fill.Palette) {
                                pickedItem.StyleInfo.Palette = pickedItem.Style.Fill.Palette;
                                pickedItem.StyleInfo.Field = pickedItem.Style.Fill.PaletteExpression;
                            }
                        }

                        var starred = {};

                        angular.forEach($pyx.spec(pickedItem.Item.Specification).starredFields(), function (fieldName) {
                            starred[fieldName] = true;
                        });

                        var dashboard = {};

                        if ($scope.currentMap) {
                            angular.forEach($scope.currentMap.dashboard(0).getModel().Widgets, function (widget) {
                                if (item.Resource.Id === widget.Inputs.Resource.Id) {
                                    dashboard[widget.Inputs.FieldName] = true;
                                }
                            });
                        }

                        angular.forEach(pickedItem.Fields, function (field) {
                            field.Starred = (field.Name in starred);
                            field.OnDashboard = (field.Name in dashboard);

                            if (pickedItem.StyleInfo.Field === field.Name) {
                                pickedItem.ShownFields.unshift(field);
                            } else if (field.Starred || field.OnDashboard) {
                                pickedItem.ShownFields.push(field);
                            } else {
                                pickedItem.HiddenFields.push(field);
                            }
                        });

                        return $scope.currentMap.pick(item, index);
                    }).then(function (fc) {
                        if (index !== $scope.cursor) return;

                        if (!fc || !fc.features.length) {
                            angular.forEach(pickedItem.Fields, function (field) {
                                delete field.Value;
                            });

                            //remove pipeline from properties window
                            var arrayIndex = $scope.pickedItems.indexOf(pickedItem);
                            $scope.pickedItems.splice(arrayIndex, 1);

                        } else {
                            var f = fc.features[0];
                            angular.forEach(pickedItem.Fields, function (field) {
                                field.Value = f.properties[field.Name];
                            });
                        }
                    });
                });
            };
        });
    };

    return {
        register: register
    };
});