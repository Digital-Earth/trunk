/* 
- @name searchTagsStore
- @desc keep the search Tags in sync with current map selection
- @type {store}
*/
app.service("searchTagsStore", function (dispatcher, $filter) {
    var i18n = $filter("i18n");

    var storeData = {
        'tags': []
    }

    function createTagFromSimpleGeometry(geometry) {
        var name = i18n("Selection");

        if (geometry.type === "Condition") {

            name = geometry.property;

            var resource = dispatcher.stores.resourcesStore.get(geometry.resource.Id);
            if (!resource) {
                dispatcher.services.resourceResolver.resolve(geometry.resource);
            } else if (!resource.Specification) {
                dispatcher.services.resourceResolver.resolveSpecification(resource);
            } else {
                for (var i = 0; i < resource.Specification.Fields.length; i++) {
                    if (resource.Specification.Fields[i].Name === name) {
                        name = resource.Specification.Fields[i].Metadata.Name;
                    }
                }                
            }                       
        } else if (geometry.createdByTool === "magic-wand" || geometry.type === "FeatureRef") {
            name = i18n("Feature");
        } else if (geometry.createdByTool === "watershed") {
            name = i18n("Watershed");
        } else if (geometry.createdByTool === "freehand") {
            name = i18n("Freehand");
        } else if (geometry.createdByTool === "polygon" || geometry.type === "Polygon") {
            name = i18n("Polygon");
        }

        return {
            geometry: geometry,
            name: name,
            enabled: true,
            operation: "intersection"
        };
    }

    function createTagsFromGeometry(geometry)
    {        
        if (!geometry) {
            return [];
        }

        var tags = [];

        if (geometry.type === "Boolean") {
            angular.forEach(geometry.operations, function (operation) {
                tags.push(createTagFromSimpleGeometry(operation.geometry));
            });
        } else {
            tags.push(createTagFromSimpleGeometry(geometry));
        }

        return tags;
    }
    
    function updateTags() {
        dispatcher.waitForStore("resourcesStore");
        dispatcher.waitForStore("selectionStore");
        storeData.tags = createTagsFromGeometry(dispatcher.stores.selectionStore.getSelectionGeometry());
    }

    var store = {
        'handle': {
            'resourceResolvedCompleted': updateTags,
            'resourceSpecificationResolvedCompleted': updateTags,            
            'changeMap': updateTags,
            'changeSelection': updateTags
        },

        /*
        - @name get
        - @desc get current search tags
        - @type {function}
        */
        'get': function () {
            return storeData.tags;
        }
    }

    return store;
});