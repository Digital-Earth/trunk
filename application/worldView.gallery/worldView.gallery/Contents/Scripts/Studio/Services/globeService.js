/* 
- @name globeService
- @desc a service that perform updates on the 3d globe
- @type {service}
*/
app.service("globeService", function (dispatcher, $filter, $pyx, networkServiceStateAlert) {
    var i18n = $filter('i18n');

    var service = {
        /*
        - @name showItem
        - @desc show a geoSource on the globe
        - @param {geoSource} geoSource - the GeoSource to show (loading default style from gallery)
        - @param {object} requestId - a token to pass back to resulting action:
                                        "updateGeoSourceVisualizationCompleted" or "updateGeoSourceVisualizationFailed"
        - @type {function}
        */
        'showItem': function (geoSource,requestId) {
            $pyx.globe.show(geoSource).success(function (id) {
                $pyx.globe.getStyle(id).success(function (style) {
                    dispatcher.actions.updateGeoSourceVisualizationCompleted.safeInvoke({
                        'action': "show",
                        'ok': true,
                        'visible': true,
                        'geoSource': geoSource,
                        'visibleId': id,
                        'requestId': requestId,
                        'style': style
                    });
                }).error(function (error) {
                    // alert the request failure
                    networkServiceStateAlert('globeService', false, i18n('Failed to get style'));
                    dispatcher.actions.updateGeoSourceVisualizationFailed.safeInvoke({
                        'action': "show",
                        'ok': false,
                        'visible': true,
                        'geoSource': geoSource,
                        'visibleId': id,
                        'requestId': requestId,
                        'error': error
                    });
                });
            }).error(function (error) {
                // alert the request failure
                networkServiceStateAlert('globeService', false, i18n('Failed to load a GeoSource on the globe'));
                dispatcher.actions.updateGeoSourceVisualizationFailed.safeInvoke({
                    'action': "show",
                    'ok': false,
                    'visible': false,
                    'geoSource': geoSource,
                    'requestId': requestId,
                    'error': error
                });
            });
        },
        /*
        - @name showItemWithStyle
        - @desc show a geoSource on the globe
        - @param {geoSource} geoSource - the GeoSource to show 
        - @param {style} style - a style to use to visualize the geoSource
        - @type {function}
        */
        'showItemWithStyle': function (geoSource, style) {
            //create a copy of style so it would not change while the globe loading the style
            style = angular.copy(style);
            $pyx.globe.showWithStyle(geoSource, style).success(function (id) {
                dispatcher.actions.updateGeoSourceVisualizationCompleted.safeInvoke({
                    'action': "show",
                    'ok': true,
                    'visible': true,
                    'geoSource': geoSource,
                    'visibleId': id,
                    'style': style
                });
            }).error(function (error) {
                dispatcher.actions.updateGeoSourceVisualizationFailed.safeInvoke({
                    'action': "show",
                    'ok': false,
                    'visible': undefined, //we don't know...
                    'geoSource': geoSource,
                    'error': error
                });
                // alert the request failure
                networkServiceStateAlert('globeService', false, i18n('Failed to load a GeoSource on the globe'));
            });
        },
        /*
        - @name setItemStyle
        - @desc update a style for already visible geoSource on the globe
        - @param {string} visibleId - visible id of the globe layer to modify. 
                                      you can get visible by using showItem or showItemWithStyle
        - @param {style} style - the new style to apply
        - @type {function}
        */
        'setItemStyle': function (visibleId, style) {
            //create a copy of style so it would not change while the globe loading the style
            style = angular.copy(style);
            $pyx.globe.setStyle(visibleId, style).success(function () {
                dispatcher.actions.updateGeoSourceVisualizationCompleted.safeInvoke({
                    'action': "update",
                    'ok': true,
                    'visible': true,
                    'visibleId': visibleId,
                    'style': style
                });
            }).error(function (error) {
                dispatcher.actions.updateGeoSourceVisualizationFailed.safeInvoke({
                    'action': "update",
                    'ok': false,
                    'visible': undefined, //we don't know...
                    'visibleId': id,
                    'error': error
                });
            });
        },
        /*
        - @name autoItemStyle
        - @desc update a style for already visible geoSource on the globe
        - @param {object} request - with following parameters:
                                    1) {guid} visibleId - the visible Id of the globe layer to update.
                                    2) {string} field - name of the field to use
                                    3) {palette} palette - the palette to apply to that field (will auto scale)
                                    4) {bool} onScreen - where to use only visible screen to auto scale
        - @param {string} requestId - a token to pass back to resulting action, which allow you to get back the new style
        - @type {function}
        */
        'autoItemStyle': function (request,requestId) {
            var promise = undefined;

            if (request.onScreen) {
                promise = $pyx.globe.setStyleByFieldWithPaletteForScreen(request.visibleId, request.field, request.palette);
            } else {
                promise = $pyx.globe.setStyleByFieldWithPalette(request.visibleId, request.field, request.palette);
            }

            promise.success(function (style) {
                dispatcher.actions.updateGeoSourceVisualizationCompleted.safeInvoke({
                    'action': "update",
                    'ok': true,                    
                    'visible': true,
                    'visibleId': request.visibleId,
                    'requestId': requestId,
                    'style': style
                });
            }).error(function (error) {
                dispatcher.actions.updateGeoSourceVisualizationFailed.safeInvoke({
                    'action': "update",
                    'ok': false,
                    'visible': true,
                    'visibleId': request.visibleId,
                    'requestId': requestId,
                    'error': error
                });
            });
        },
        /*
        - @name hideItem
        - @desc hide already visible geoSource from the globe
        - @param {string} visibleId - visible id of the globe layer to modify. 
                                      you can get visible by using showItem or showItemWithStyle        
        - @type {function}
        */
        'hideItem': function (visibleId) {
            $pyx.globe.hide(visibleId).success(function () {
                dispatcher.actions.updateGeoSourceVisualizationCompleted.safeInvoke({
                    'action': "hide",
                    'ok': true,
                    'visible': false,
                    'visibleId': visibleId
                });
            }).error(function (error) {
                dispatcher.actions.updateGeoSourceVisualizationFailed.safeInvoke({
                    'action': "hide",
                    'ok': false,
                    'visible': undefined, //we don't know...
                    'visibleId': id,
                    'error': error
                });
            });
        }
    };

    return service;
});