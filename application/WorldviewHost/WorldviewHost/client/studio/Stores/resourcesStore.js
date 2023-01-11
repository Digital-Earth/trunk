/* 
- @name resourcesStore
- @desc cache for all the resolved resources 
- @type {store}
*/
app.service("resourcesStore", function (dispatcher) {
    var storeData = {
        resources: {}
    };
    var store = {
        'handle': {
            'resourceResolvedCompleted': function (action) {
                storeData.resources[action.data.resource.Id] = action.data.resource;
            },
            'resourceSpecificationResolvedCompleted': function (action) {
                if (action.data.resource.Id in storeData.resources) {
                    storeData.resources[action.data.resource.Id].Specification = action.data.specification;
                }
            }
        },
        /*
        - @name get
        - @desc get resolved resource based on resource Id
        - @param {guid} id - resource Id
        - @type {function}
        */
        'get': function (id) {
            return storeData.resources[id];
        }
    };

    return store;
});