/* 
- @name currentMapStore
- @desc currentMapStore responsible to keep the right state of the current map synced with the globe
- @type {store}
*/
app.service("currentMapStore", function (dispatcher) {
    var geoSourceStates = {
        
        notVisible: "notVisible",
        visible: "visible",

        resolving: "resolving",
        showing: "showing",
        updating: "updating",
        removing: "removing",

        error: "error"
    };

    var storeData = {
        model: undefined,        
        globeModel: {
            geoSources: {}            
        }
    };

    var nextRequestId = 0;

    function getUniqueRequestId() {
        nextRequestId++;
        return nextRequestId;
    }

    function showItem(item) {
        function requestGlobeToShow(item, geoSource) {
            var newState = {
                geoSource: geoSource,
                state: geoSourceStates.showing                
            };            
            if (item.Style) {
                dispatcher.services.globeService.showItemWithStyle(geoSource, item.Style);
            } else {
                newState.requestId = getUniqueRequestId();
                dispatcher.services.globeService.showItem(geoSource, newState.requestId);
            }
            storeData.globeModel.geoSources[item.Resource.Id] = newState;
        }

        if (!(item.Resource.Id in storeData.globeModel.geoSources)) {

            var geoSource = dispatcher.stores.resourcesStore.get(item.Resource.Id);
            if (!geoSource) {
                storeData.globeModel.geoSources[item.Resource.Id] = {
                    state: geoSourceStates.resolving
                };
                dispatcher.services.resourceResolver.resolve(item.Resource);
                return;                
            } else if (!geoSource.Specification || !geoSource.Specification.Fields) {
                storeData.globeModel.geoSources[item.Resource.Id] = {
                    state: geoSourceStates.resolving
                };
                dispatcher.services.resourceResolver.resolveSpecification(geoSource);
                return;
            } else {
                if (!item.Specification || !item.Specification.Fields) {
                    item.Specification = geoSource.Specification;
                }
                requestGlobeToShow(item, geoSource);
            }
        } else {
            var currentState = storeData.globeModel.geoSources[item.Resource.Id];

            switch (currentState.state) {

            case geoSourceStates.visible:
                if (!angular.equals(currentState.style, item.Style)) {
                    currentState.state = geoSourceStates.updating;
                    dispatcher.services.globeService.setItemStyle(currentState.visibleId, item.Style);
                }
                break;

            case geoSourceStates.notVisible:
                requestGlobeToShow(item, currentState.geoSource);
                break;
            }
        }
    }

    var hideItem = function (item) {
        if (!(item.Resource.Id in storeData.globeModel.geoSources)) {
            //we all good            
        } else {
            var currentState = storeData.globeModel.geoSources[item.Resource.Id];

            if (currentState.state === geoSourceStates.visible) {
                currentState.state = geoSourceStates.removing;
                dispatcher.services.globeService.hideItem(currentState.visibleId);
            } else if (currentState.state === geoSourceStates.error) {
                //this geoSource has been giving problems - remove it.
                delete storeData.globeModel.geoSources[item.Resource.Id];
            }
        }
    }

    function findGeoSourceFromVisibleId(visibleId) {
        var result = undefined;
        angular.forEach(storeData.globeModel.geoSources, function (state) {
            if (state.visibleId === visibleId) {
                result = state.geoSource;
            }
        });
        return result;
    }

    function findItem(id) {
        var result = undefined;
        if (storeData.model) {
            angular.forEach(storeData.model.Groups, function (group) {
                angular.forEach(group.Items, function (item) {
                    if (item.Resource.Id === id) {
                        result = item;
                    }
                });
            });
        }
        return result;
    }

    function findItemByVisibleId(id) {
        var result = undefined;
        if (storeData.globeModel) {
            angular.forEach(storeData.globeModel.geoSources, function (geoSourceState) {
                if (geoSourceState.visibleId === id) {
                    if (geoSourceState.geoSource.Id) {
                        result = findItem(geoSourceState.geoSource.Id);
                    }
                }
            });
        }
        return result;
    }

    function updateItem(item) {
        if (item) {
            if (item.Active) {
                showItem(item);
            } else {
                hideItem(item);
            }
        }        
    }

    function resolveGeoSource(geoSource) {
        if (geoSource.Id in storeData.globeModel.geoSources) {
            storeData.globeModel.geoSources[geoSource.Id].geoSource = geoSource;

            if (!geoSource.Specification ||
                !geoSource.Specification.Fields ||
                !geoSource.Specification.Fields.length) {
                //no specification yet - resolve specification now
                dispatcher.services.resourceResolver.resolveSpecification(geoSource);
            } else {
                if (storeData.globeModel.geoSources[geoSource.Id].state === geoSourceStates.resolving) {
                    storeData.globeModel.geoSources[geoSource.Id].state = geoSourceStates.notVisible;
                }
                
                //we are ready to update the item...
                var item = findItem(geoSource.Id);

                if (item && (!item.Specification || !item.Specification.Fields)) {
                    item.Specification = geoSource.Specification;
                }

                updateItem(item);
            }
        } 
    }

    function updateGeoSourceState(geoSourceGlobeState) {
        var geoSource = geoSourceGlobeState.geoSource;
        if (!geoSource) {
            geoSource = findGeoSourceFromVisibleId(geoSourceGlobeState.visibleId);
        }

        if (geoSourceGlobeState.ok) {
            if (geoSourceGlobeState.visible) {
                var oldState = storeData.globeModel.geoSources[geoSource.Id] || {};
                var newState = {
                    'state': geoSourceStates.visible,
                    'geoSource': geoSource,
                    'style': geoSourceGlobeState.style,
                    'visibleId': geoSourceGlobeState.visibleId
                };
                storeData.globeModel.geoSources[geoSource.Id] = newState;

                //update item style from geoSourceGlobeState style if needed
                var item = findItem(geoSource.Id);
                if (item && geoSourceGlobeState.style &&                    
                    geoSourceGlobeState.requestId && oldState.requestId === geoSourceGlobeState.requestId) {
                    item.Style = angular.copy(geoSourceGlobeState.style);
                }

                //make sure the item is synced
                updateItem(item);
            } else {
                //geoSource has been removed - sweet.                
                if (geoSource) {
                    delete storeData.globeModel.geoSources[geoSource.Id];

                    //check if this geoSource that been removed is required to be shown
                    var hiddenItem = findItem(geoSource.Id);
                    updateItem(hiddenItem);
                }
            }
        } else {
            if (geoSource) {
                if (geoSource.id in storeData.globeModel.geoSources) {
                    storeData.globeModel.geoSources[id].state = geoSourceStates.error;
                }
            }
        }
    }


    var store = {
        'handle': {
            'resourceResolvedCompleted': function (action) {
                dispatcher.waitForStore("resourcesStore");
                resolveGeoSource(action.data.resource);
            },
            'resourceSpecificationResolvedCompleted': function (action) {
                dispatcher.waitForStore("resourcesStore");
                resolveGeoSource(action.data.resource);
            },
            'changeMap': function (action) {
                storeData.model = action.data.map;
                
                var items = store.activeItems();

                //update all active items
                angular.forEach(items, updateItem);

                function shouldHide(geoSourceId) {
                    return !items.some(function (item) { return item.Resource.Id === geoSourceId; });
                }

                //start hiding all leftovers geoSources
                angular.forEach(storeData.globeModel.geoSources, function (currentState) {
                    if (currentState.state === geoSourceStates.visible && shouldHide(currentState.geoSource.Id)) {
                        currentState.state = geoSourceStates.removing;
                        dispatcher.services.globeService.hideItem(currentState.visibleId);
                    }
                });
            },
            'changeSelection': function (action) {
                if (storeData.model &&
                    storeData.model.Dashboards &&
                    storeData.model.Dashboards[0]) {
                    if (storeData.model.Dashboards[0].Selection !== action.data.selection) {
                        storeData.model.Dashboards[0].Selection = action.data.selection;
                        if (!storeData.model.Dashboards[0].Selection) {
                            delete storeData.model.Dashboards[0].Selection;
                        }
                    }
                }
            },
            'showItem': function (action) {
                var item = findItem(action.data.item.Resource.Id);
                if (item && !item.Active) {
                    item.Active = true;
                    updateItem(item);
                }                
            },
            'hideItem': function (action) {
                var item = findItem(action.data.item.Resource.Id);
                if (item && item.Active) {
                    item.Active = false;
                    updateItem(item);
                }                
            },
            'updateItemStyle': function (action) {
                var item = findItem(action.data.item.Resource.Id);
                if (item && item.Active) {
                    //make a copy of the item style.
                    item.Style = angular.copy(action.data.item.Style);
                    updateItem(item);
                }
            },
            'calculateItemStyle': function (action) {
                var item = findItem(action.data.item.Resource.Id);

                if (item && store.getItemState(item.Resource.Id) === geoSourceStates.visible) {
                    var palette = action.data.palette;
                    var field = action.data.field;

                    var currentState = storeData.globeModel.geoSources[item.Resource.Id];
                    currentState.state = geoSourceStates.updating;
                    currentState.requestId = getUniqueRequestId();

                    var request = {
                        visibleId: currentState.visibleId,                        
                        field: field,
                        palette: palette,
                        onScreen: action.data.onScreen
                    }

                    dispatcher.services.globeService.autoItemStyle(request, currentState.requestId);
                }
            },
            'updateGeoSourceVisualizationCompleted': function (action) {
                updateGeoSourceState(action.data);
            },
            'updateGeoSourceVisualizationFailed': function (action) {
                updateGeoSourceState(action.data);
            }
        },

        /*
        - @name get
        - @desc get the current map model        
        - @type {function}
        */
        'get': function () {
            return storeData.model;
        },
        /*
        - @name getItem
        - @desc get a map item based on a given Id 
        - @param {guid} id - resource Id or visible Id
        - @type {function}
        */
        'getItem': function (id) {            
            var item = findItem(id) || findItemByVisibleId(id);
            return item;
        },
        /*
        - @name getItemState
        - @desc get a map item status based on a given Id, status is a string value: "loading", "notVisible", "visible", "error", ...
        - @param {guid} id - resource Id 
        - @type {function}
        */
        'getItemState': function (id) {            
            if (id in storeData.globeModel.geoSources) {
                return storeData.globeModel.geoSources[id].state;
            }
            return geoSourceStates.notVisible;
        },
        /*
        - @name items
        - @desc get a flat array of items
        - @param optional {function} filter - filter function to check if item should be included in the results. filter(item)->bool
        - @type {function}
        */
        'items': function (filter) {
            var items = [];
            filter = filter || function () { return true };
            if (storeData.model) {
                angular.forEach(storeData.model.Groups, function (group) {
                    angular.forEach(group.Items, function (item) {
                        if (filter(item)) {
                            items.push(item);
                        }
                    });
                });
            }
            return items;
        },
        /*
        - @name items
        - @desc get a flat array of all active items (visible items)
        - @type {function}
        */
        'activeItems': function () {
            return this.items(function (item) { return item.Active; });
        }        
    };

    return store;
});
