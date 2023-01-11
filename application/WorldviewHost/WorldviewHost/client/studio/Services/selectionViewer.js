/* 
- @name selectionViewer
- @desc a service that visualize a selection on the globe
- @type {service}
*/
app.service("selectionViewer", function (dispatcher, $pyx) {
    var service = {
        /*
        - @name hideSelection
        - @desc hide selection with a given selection id
        - @param {guid} selectionId - selection id to hide from globe
        - @type {function}
        */
        'hideSelection': function (selectionId) {
            $pyx.globe.hide(selectionId).success(function () {
                dispatcher.actions.hideSelectionCompleted.safeInvoke({ selectionId: selectionId });
            }).error(function (error) {
                dispatcher.actions.hideSelectionFailed.safeInvoke({ selectionId: selectionId, error: error });
            });
        },
        /*
        - @name viewSelection
        - @desc show selection on the globe
        - @param {selection} selection - selection object have 2 properties: Geometry & Style 
        - @type {function}
        */
        'viewSelection': function (selection) {
            //make a copy before it changes...
            selection = angular.copy(selection);

            var style = selection.Style;
            var geometry = selection.Geometry;
            var fc = {
                type: "FeatureCollection",
                features: [
                    {
                        type: "Feature",
                        id: 1,
                        geometry: geometry,
                        properties: {}
                    }
                ]
            };

            $pyx.engine.createFeatureCollection(fc).success(function (geoSource) {
                $pyx.globe.showWithStyle(geoSource, style).success(function (id) {
                    dispatcher.actions.viewSelectionCompleted.safeInvoke({ selectionId: id, selection: selection });
                }).error(function (error) {
                    dispatcher.actions.viewSelectionFailed.safeInvoke({ selection: selection, error: error });
                });
            }).error(function (error) {
                dispatcher.actions.viewSelectionFailed.safeInvoke({ selection: selection, error: error });
            });
        }
    };

    return service;
});