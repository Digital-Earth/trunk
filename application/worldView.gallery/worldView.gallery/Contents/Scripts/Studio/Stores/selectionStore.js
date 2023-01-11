/* 
- @name selectionStore
- @desc keep the selection on the globe in sync with current map selection
- @type {store}
*/
app.service("selectionStore", function (dispatcher,styleOptions) {

    var states = {
        none: "none",
        loading: "loading",
        visible: "visible",
        error: "error"
    };

    var storeData = {
        currentSelection: undefined,
        currentSelectionId: undefined,
        state: states.none
    };

    function updateSelection(selection) {
        if (storeData.state !== states.loading && !angular.equals(selection, storeData.currentSelection)) {
            if (storeData.currentSelection) {
                if (storeData.currentSelectionId) {
                    dispatcher.services.selectionViewer.hideSelection(storeData.currentSelectionId);
                }
                storeData.currentSelection = undefined;
                storeData.currentSelectionId = undefined;
                storeData.state = states.none;
            }
            if (selection && selection.Geometry) {
                storeData.currentSelection = angular.copy(selection);
                storeData.state = states.loading;
                dispatcher.services.selectionViewer.viewSelection(storeData.currentSelection);
            }
        } else {
            //we are loading at the moment... update currentSelection only
            storeData.currentSelection = angular.copy(selection);
        }
    }

    var store = {
        handle: {
            'changeMap': function (action) {
                if (action.data.map &&
                    action.data.map.Dashboards &&
                    action.data.map.Dashboards.length &&
                    action.data.map.Dashboards[0].Selection &&
                    action.data.map.Dashboards[0].Selection.Geometry) {
                    updateSelection(action.data.map.Dashboards[0].Selection);
                } else {
                    updateSelection();
                }
            },
            'changeSelection': function (action) {
                updateSelection(action.data.selection);
            },
            'viewSelectionCompleted': function (action) {
                if (angular.equals(action.data.selection, storeData.currentSelection)) {
                    //this is our selection - great
                    storeData.state = states.visible;
                    storeData.currentSelectionId = action.data.selectionId;
                } else {
                    //not our selection - hide it
                    dispatcher.services.selectionViewer.hideSelection(action.data.selectionId);
                    storeData.currentSelectionId = undefined;

                    if (storeData.currentSelection) {
                        storeData.state = states.loading;
                        dispatcher.services.selectionViewer.viewSelection(storeData.currentSelection);
                    }
                }
            },
            'viewSelectionFailed': function (action) {
                if (angular.equals(action.data.selection, storeData.currentSelection)) {
                    storeData.state = states.error;
                } else {
                    //this is an error from previous selection - try to view the new and update currentSelection
                    storeData.state = states.loading;
                    dispatcher.services.selectionViewer.viewSelection(storeData.currentSelection);
                }
            }
        },
        /*
        - @name get
        - @desc get store data
        - @type {function}
        */
        get: function () {
            return storeData;
        },
        /*
        - @name getSelectionGeometry
        - @desc get current selection geometry 
        - @type {function}
        */
        getSelectionGeometry: function () {
            if (storeData.currentSelection) {
                return storeData.currentSelection.Geometry;
            }
            return undefined;
        },
        /*
        - @name getSelectionColor
        - @desc get current selection color (or default color if no selection color found
        - @type {function}
        */
        getSelectionColor: function () {
            if (storeData.currentSelection) {
                return storeData.currentSelection.Style.Line.Color;
            }
            //return the first color (red like color)
            return styleOptions.colors[0];
        }
    }

    return store;
});