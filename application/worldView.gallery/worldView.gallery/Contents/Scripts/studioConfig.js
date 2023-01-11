app.factory("worldViewStudioConfig", function () {
    return {
        firstUse: {
            //which gallery to feature on first use
            galleryId: "d659b36c-02e0-4b98-a2ad-c5a835c0dcd8"            
        },
        search: {
            //the amount of delay from the search term change until search been performed
            searchDelay: 500, //milliseconds,

            //the amount of delay to clear the search terms
            searchClearDelay: 10, //milliseconds,

            //number of results to show from each service by default
            defaultResultCount: 5, //results

            //number of search results that will cause long-running rendering
            longRenderThreshold: 100, //results

            //the amount of delay to ensure a warning message is displayed before a long render
            longRenderDelay: 150 //milliseconds
        },
        "import": {
            //the number of active items in a map before the import operation will stop auto-show on import
            maxActiveItemsOnImport: 10
        },
        whatIsHere: {
            //how many histogram bins to display on property results
            histogramBinCount: 50 //bins
        },
        library: {
            //internal to perform auto-save even if there was no change
            autoSaveTime: 60 * 1000, //milliseconds

            //a small delay before saving the library after modification has been made
            delayAfterModification: 3 * 1000 //milliseconds
        },
        dashboard: {
            //a small pop up time for the dashboard when a new data set is activated.
            popupTime: 3 * 1000 //milliseconds
        },
        alerts: {
            //how long an alert should be displayed
            alertPopupTime: 5 * 1000, //milliseconds
            //how often to check network connectivity
            networkCheckPeriod: 10 * 1000 //milliseconds
        },
        userPolling: {
            //how frequently to check if a user's email has been confirmed
            checkFrequency: 3 * 1000 // milliseconds
        },
        fpsTracing: {
            //threshold to catch a slow frame
            slowFrame: 100, //milliseconds

            //speed to refresh the fps report
            fpsReportUpdate: 250 //milliseconds
        },
        globe: {
            //navigation: factor for zoom in/out.
            zoomLevelFactor: 2, // camera.range = factor * camera.range

            //navigation: tilt rotation amount
            tiltRotationAmount: 15, //degrees

            //navigation: heading rotation amount
            headingRotationAmount: 15, //degrees

            //animation duration when navigation with keyboard or nav controls
            navigationAnimationDuration: 200, //milliseconds

            //animation duration when navigation when moving the north arrow up
            northUpAnimationDuration: 1000, //milliseconds

            //refresh rate to update the loading status for geosource on the globe
            loadingStatusRefreshRate: 1000, //milliseconds

            //refresh rate to update the north arrow in the nav controls
            cameraHeadingRefreshRate: 500, //milliseconds

            //camera animation duration when importing a new map
            gotoCameraAnimationDuration: 3 * 1000, //milliseconds

            //camera animation duration when importing a new map
            captureCameraAnimationDuration: 50, //milliseconds

            //depend on the quality of the geo-ip service, what is the camera zoom level we should use.
            gotoUserLocationCameraRange: 80000 //meters
        }
    };
});