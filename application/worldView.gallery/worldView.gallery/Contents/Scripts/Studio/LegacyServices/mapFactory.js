app.service('geoSourceCache', function ($q, $pyx, $cacheFactory) {
    var cache = $cacheFactory('pyx-cache');

    var geoSourceCache = {
        set: function (geoSource) {
            var oldEntry = cache.get(geoSource.Id);

            if (oldEntry && oldEntry.Version !== geoSource.Version) {
                var oldEntryUpdateDate = new Date(oldEntry.Metadata.Updated);
                var newEntryUpdateDate = new Date(geoSource.Metadata.Updated);
                if (oldEntryUpdateDate > newEntryUpdateDate) {
                    return oldEntry;
                }
            }
            return this.overwrite(geoSource);
        },
        overwrite: function (geoSource) {
            cache.put(geoSource.Id, geoSource);
            return geoSource;
        },
        get: function (id) {
            var self = this;

            var geoSource = cache.get(id);

            if (geoSource) {
                return $q.when(geoSource);
            }

            return $pyx.gallery.geoSources().getById(id).then(function (d) {
                var geoSource = d.data;
                return self.set(geoSource);
            });
        },
        getWithSpecification: function (id) {
            return this.get(id).then(function (geoSource) {
                if (geoSource.Specification && geoSource.Specification.Fields) {
                    return geoSource;
                } else {
                    var deferred = $q.defer();

                    $pyx.engine.getSpecification(geoSource).success(function (specification) {
                        deferred.resolve(specification);
                    }).error(function (error) {
                        deferred.reject(error);
                    });

                    return deferred.promise.then(function (specification) {
                        geoSource.Specification = specification;
                        return geoSource;
                    });
                }
            });
        },
        remove: function (id) {
            cache.remove(id);
        }
    };

    return geoSourceCache;
});

app.factory('mapFactory', function ($q, $timeout, $pyx, geoSourceCache, importService, styleOptions, dispatcher, dispatcherPromise, worldViewStudioConfig) {

    return {
        load: function (mapDoc) {
            if (!mapDoc.Groups) {
                mapDoc.Groups = [];
            }
            if (!mapDoc.Dashboards) {
                mapDoc.Dashboards = [];
            }
            if (mapDoc.Dashboards.length === 0) {
                mapDoc.Dashboards.push(
                {
                    Widgets: []
                });
            }
            if (!mapDoc.EmbeddedResources) {
                mapDoc.EmbeddedResources = [];
            }

            var map = {
                model: mapDoc,
                state: {
                    Items: {},
                    Dashboards: [],
                    Imports: [],
                    VisibleIds: {},
                    VisibleIdsLoading: {}
                },
                show: function () {
                    dispatcher.actions.changeMap.invoke({ map: mapDoc });
                    return;
                },
                showGroup: function (group) {
                    var self = this;
                    angular.forEach(group.Items, function (item) {
                        if (!item.Active) {
                            self.showItem(item);
                        }
                    });
                },
                hideGroup: function (group) {
                    var self = this;
                    angular.forEach(group.Items, function (item) {
                        if (item.Active) {
                            self.hideItem(item);
                        }
                    });
                },
                hideItem: function (item) {
                    dispatcher.actions.hideItem.invoke({ 'item': item });
                },
                showItem: function (item) {
                    dispatcher.actions.showItem.invoke({ 'item': item });
                },
                addItem: function (group, geoSource, style, autoShow) {
                    geoSourceCache.set(geoSource);
                    var item = {
                        Metadata: geoSource.Metadata,
                        Specification: geoSource.Specification,
                        Resource: { 'Id': geoSource.Id, 'Type': geoSource.Type, 'Version': geoSource.Version },
                        DataSet: geoSource.DataSet,
                        Style: style,
                        Active: false,
                        ShowDetails: true
                    };
                    group.Items.push(item);

                    if (autoShow !== false) {
                        var promise = dispatcherPromise.promise(function (action, deferred) {
                            dispatcher.waitForStore("currentMapStore");
                            var foundItem = dispatcher.stores.currentMapStore.getItem(geoSource.Id);
                            if (foundItem) {
                                var state = dispatcher.stores.currentMapStore.getItemState(geoSource.Id);
                                if (state === "visible") {
                                    deferred.resolve(foundItem);
                                } else if (state === "error") {
                                    deferred.reject({
                                        item: item,
                                        error: "failed to add item"
                                    });
                                }
                            }
                        });

                        dispatcher.actions.showItem.invoke({ item: item });

                        return promise;
                    } else {
                        return $q.when(item);
                    }
                },
                removeItem: function (group, item) {
                    var index = $pyx.array.firstIndex(group.Items, function (i) { return i.Resource.Id === item.Resource.Id; });

                    if (index !== -1) {
                        var currentItem = group.Items[index];

                        if (currentItem.Active) {
                            this.hideItem(currentItem);
                        }
                        group.Items.splice(index, 1);
                        this.sanitize();
                    }
                },
                someItems: function (filter) {
                    filter = filter || function () { return true; };
                    for (var g = 0; g < this.model.Groups.length; g++) {
                        var group = this.model.Groups[g];
                        for (var i = 0; i < group.Items.length; i++) {
                            var item = group.Items[i];
                            if (filter(item)) {
                                return true;
                            }
                        }
                    }
                    return false;
                },
                items: function (filter) {
                    var self = this;
                    var result = [];
                    filter = filter || function () { return true; };
                    angular.forEach(self.model.Groups, function (group) {
                        angular.forEach(group.Items, function (item) {
                            if (filter(item)) {
                                result.push(item);
                            }
                        });
                    });
                    return result;
                },
                activeItems: function () {
                    return this.items(function (item) { return item.Active; });
                },
                getGeoSource: function (item) {
                    return geoSourceCache.get(item.Resource.Id);
                },
                getVisibleId: function (item) {
                    var self = this;
                    var resourceId = item.Resource.Id;
                    if (resourceId in self.state.VisibleIds) {
                        return $q.when(self.state.VisibleIds[resourceId]);
                    }
                    return geoSourceCache.get(item.Resource.Id)
                        .then(function (geoSource) {
                            var deferred = $q.defer();
                            $pyx.globe.getVisibleId(geoSource).success(function (id) {
                                self.state.VisibleIds[resourceId] = id;
                                self.state.VisibleIdsLoading[resourceId] = false;
                                deferred.resolve(id);
                            }).error(function (error) {
                                deferred.reject(error);
                            });
                            return deferred.promise;
                        });
                },
                //return if item is ready. which mean not in the middle of initializing or hiding
                isItemReady: function (item) {
                    var state = dispatcher.stores.currentMapStore.getItemState(item.Resource.Id);
                    return state === "notVisible" || state === "visible" || state === "error";
                },
                //return the last loading status for a map item
                isItemLoading: function (item) {
                    return this.state.VisibleIdsLoading[item.Resource.Id];
                },
                //update loading status of all visible items in the map
                updateLoadingState: function () {
                    var self = this;
                    if ('getAllLoadingVisibleIds' in $pyx.globe) {
                        $pyx.globe.getAllLoadingVisibleIds().success(function (loadingIds) {
                            $timeout(function () {
                                self.state.VisibleIdsLoading = {};
                                angular.forEach(loadingIds, function (visibleId) {
                                    var item = dispatcher.stores.currentMapStore.getItem(visibleId);
                                    if (item) {
                                        self.state.VisibleIdsLoading[item.Resource.Id] = true;
                                    }
                                });
                            });
                        });
                    }
                },
                getDefinition: function (item) {
                    if (item.Specification && item.Specification.Fields) {
                        return $q.when(item.Specification);
                    } else {
                        return geoSourceCache.getWithSpecification(item.Resource.Id).then(function (geoSource) {
                            item.Specification = angular.copy(geoSource.Specification);
                            return item.Specification;
                        });
                    }
                },
                pick: function (item, index) {
                    var deferred = $q.defer();
                    geoSourceCache.get(item.Resource.Id).then(function (geoSource) {
                        $pyx.engine.getFeatures(geoSource, index).success(function (fc) {
                            deferred.resolve(fc);
                        }).error(function (error) {
                            deferred.reject(error);
                        });
                    });
                    return deferred.promise;
                },
                addDashboard: function () {
                    this.model.Dashboards.push({
                        Widgets: []
                    });
                    this.state.Dashboards.push(undefined);
                    return this.dashboard(this.model.Dashboards.length - 1);
                },
                dashboard: function (id) {
                    if (!(id in this.model.Dashboards)) {
                        throw "dashboard not found";
                    }

                    if (this.state.Dashboards[id]) {
                        return this.state.Dashboards[id];
                    }

                    var map = this;

                    var dashboard = {
                        id: id,
                        map: map,
                        getModel: function () { return map.model.Dashboards[id]; },
                        addWidget: function (type, item, field, name, value) {
                            var widget = {
                                Metadata: {
                                    Name: name,
                                    Description: ""
                                },
                                Type: type,
                                Inputs: {
                                    Resource: item.Resource,
                                    FieldName: field
                                },
                                Settings: {}
                            };
                            if (value !== undefined) {
                                widget.Inputs.FieldValue = value;
                            }
                            if (type === 'Hist') {
                                widget.Settings.RowSpan = 2;
                                widget.Settings.ColSpan = 2;
                            }
                            this.getModel().Widgets.push(widget);
                            var index = this.getModel().Widgets.length - 1;
                            return index;
                        },
                        widget: function (widgetId) {
                            return this.getModel().Widgets[widgetId];
                        },
                        removeWidget: function (widgetId) {
                            this.getModel().Widgets.splice(widgetId, 1);
                        },
                        setSelection: function (geometry, color, borderColor) {
                            var currentSelectionColor = dispatcher.stores.selectionStore.getSelectionColor();

                            color = color || styleOptions.setAlpha(currentSelectionColor, 0.3);
                            borderColor = borderColor || currentSelectionColor;

                            if (geometry) {
                                this.getModel().Selection = {
                                    Geometry: geometry,
                                    Style: {
                                        Fill: {
                                            Style: 'SolidColor',
                                            Color: color,
                                            Effects: [
                                                {
                                                    Type: 'SinePattern',
                                                    Properties: {
                                                        //direction of the lines
                                                        Angle: 45,
                                                        //sine movement speed in length per sec
                                                        Speed: 1,
                                                        //the length of sine frequency in pixels
                                                        Length: 2
                                                    }
                                                }]},
                                        Line: { Style: 'SolidColor', Color: borderColor }
                                    }
                                };
                            } else {
                                delete this.getModel().Selection;
                            }

                            dispatcher.actions.changeSelection.invoke({ selection: this.getModel().Selection });
                        }
                    };

                    function upgradeDashboard() {
                        //upgrade widgets

                        //from: { 'Name':'name','Type':'Hist','Resource':{...},'FieldName':'X','ColSpan':2 }
                        //to:   { 'Metadata': {'Name':'name'}, 'Type':'Hist','Inputs':{'Resource':{...},'FieldName':'X'},'Settings':{'ColSpan':2} }

                        var map = {
                            'Name': 'Metadata',
                            'Description': 'Metadata',
                            'Resource': 'Inputs',
                            'FieldName': 'Inputs',
                            'FieldValue': 'Inputs',
                            'RowSpan': 'Settings',
                            'ColSpan': 'Settings'
                        }

                        angular.forEach(dashboard.getModel().Widgets, function (widget) {
                            angular.forEach(map, function (section, parameter) {
                                if (parameter in widget) {
                                    if (!(section in widget)) {
                                        widget[section] = {};
                                    }
                                    widget[section][parameter] = widget[parameter];
                                    delete widget[parameter];
                                }
                            });
                        });
                    };

                    upgradeDashboard();

                    this.state.Dashboards[id] = dashboard;
                    return dashboard;
                },
                'getDefaultGroup': function () {
                    if (this.model.Groups.length === 0) {
                        this.model.Groups.push({
                            Metadata: { Name: '' },
                            Items: []
                        });
                    }
                    return this.model.Groups[0];
                },
                'import': function (dataSet, group, defaultStyle) {
                    if (!group) {
                        group = this.getDefaultGroup();
                    }

                    var alreadyImportedItem = undefined;

                    $pyx.array.first(this.state.Imports, 'dataSet', dataSet).found(function (item) {
                        alreadyImportedItem = item;
                    });

                    if (alreadyImportedItem) {
                        return alreadyImportedItem;
                    }

                    var importItem = importService.import(dataSet);
                    this.state.Imports.push(importItem);

                    var self = this;

                    importItem.promise.then(
                        function (geoSource) {
                            var importItemIndex = $pyx.array.firstIndex(self.state.Imports, function (item) { return item.dataSet === dataSet; });
                            if (importItemIndex !== -1) {
                                // a hack: memorize the DataSet that the GeoSource was created from
                                // to be able to recognize it in the search results
                                geoSource.DataSet = dataSet;
                                // add the GeoSource to the map
                                self.importGeoSource(geoSource, group, defaultStyle).success(function (style) {
                                    $timeout(function () {
                                        // re-find index of item as array may have changed
                                        importItemIndex = $pyx.array.firstIndex(self.state.Imports, function (item) { return item.dataSet === dataSet; });
                                        if (importItemIndex !== -1) {
                                            self.state.Imports.splice(importItemIndex, 1);
                                        }
                                        // visualize the imported GeoSource if there are less than 10 active items in the map
                                        if (self.activeItems().length < worldViewStudioConfig.import.maxActiveItemsOnImport) {
                                            self.addItem(group, geoSource, style);
                                        } else {
                                            // just add the map item without visualizing it
                                            self.addItem(group, geoSource, style, false);
                                        }
                                    });
                                }).error(function () {
                                    // re-find index of item as array may have changed
                                    importItemIndex = $pyx.array.firstIndex(self.state.Imports, function (item) { return item.dataSet === dataSet; });
                                    if (importItemIndex !== -1) {
                                        self.state.Imports.splice(importItemIndex, 1);
                                    }
                                });
                            }
                        },
                        function () {
                            $timeout(function () {
                                self.cancelImport(importItem, "Import failed");
                            }, 2000);
                        }
                    );

                    return importItem;
                },
                'importGeoSource': function (geoSource, group, defaultStyle) {
                    if (!group) {
                        group = this.getDefaultGroup();
                    }
                    var self = this;

                    //add geosource as an embedded resource to map and cache
                    self.model.EmbeddedResources.push(geoSource);
                    geoSourceCache.set(geoSource);
                    dispatcher.actions.resourceResolvedCompleted.invoke({ 'resource': geoSource });

                    if (!defaultStyle) {
                        defaultStyle = {
                            'Icon': {
                                Style: 'SolidColor',
                                Color: styleOptions.colors[group.Items.length % (styleOptions.colors.length - 1)],
                                IconDataUrl: styleOptions.icons[group.Items.length % (styleOptions.icons.length - 1)].oldDataUrl,
                                Scale: 0.2
                            },
                            'Fill': {
                                Style: 'SolidColor',
                                Color: styleOptions.colors[group.Items.length % (styleOptions.colors.length - 1)]
                            }
                        }
                    }

                    return $pyx.globe.createDefaultStyle(geoSource, defaultStyle);
                },
                'cancelImport': function (item, reason) {
                    //check if the item is among the current imports
                    var index = this.state.Imports.indexOf(item);
                    if (index !== -1) {
                        //remove the item from the collection
                        this.state.Imports.splice(index, 1);
                        //explicitly cancel the importing
                        item.cancel(reason);
                    }
                },
                'cancelAllImports': function (reason) {
                    //cancel importing items from the end to the front...
                    while (this.state.Imports.length > 0) {
                        var importItem = this.state.Imports[this.state.Imports.length - 1];
                        this.cancelImport(importItem, reason);
                    }
                },
                'getImportQueueLength': function () {
                    return this.state.Imports.length;
                },
                //sanitize up the map. use this function before saving the map.
                'sanitize': function () {

                    //step 1: clean unused embedded resources..
                    var usedIds = {};
                    var self = this;

                    angular.forEach(self.model.Groups, function (group) {
                        angular.forEach(group.Items, function (item) {
                            usedIds[item.Resource.Id] = item;
                        });
                    });

                    self.model.EmbeddedResources = $pyx.array.where(self.model.EmbeddedResources,
                        function (resource) {
                            var found = resource.Id in usedIds;
                            if (found) {
                                //update the metadata and spec of the embedded resource
                                var item = usedIds[resource.Id];
                                resource.Metadata = angular.copy(item.Metadata);
                                resource.Specification = angular.copy(item.Specification);
                            }
                            return found;
                        });

                    self.model.Type = 'Map';
                    if (self.model.Camera && !('Range' in self.model.Camera)) {
                        delete self.model.Camera;
                    }
                },
                //verify that the schema of the map is valid and correct.
                'verify': function () {
                    if ('verifyMapSchema' in $pyx.engine) {
                        var expanded = this.model.Expanded;

                        this.sanitize();

                        //verify schema
                        var verified = $pyx.engine.verifyMapSchema(this.model);

                        //copy back verified into the map model
                        //this will replace the old self.model with the new properties
                        angular.copy(verified, this.model);

                        this.model.Expanded = expanded;
                    }
                }
            };

            //populate cache with embedded resources 
            angular.forEach(map.model.EmbeddedResources, function (resource) {
                geoSourceCache.set(resource);
                //populate all embedded resources
                dispatcher.actions.resourceResolvedCompleted.invoke({ 'resource': resource });
            });

            //initialize the state of the dashboards. 
            angular.forEach(map.model.Dashboards, function (dashboard, index) {
                map.state.Dashboards.push(undefined);
                map.dashboard(index);
            });

            return map;
        }
    }
});