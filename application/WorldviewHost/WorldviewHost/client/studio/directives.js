/* global wgs84 */
/* global _ */
/* global $ */
/* global angular */
/* global app */
/* global TimelineMax */
app.directive('subWindow', function () {
    return {
        restrict: 'AC',
        link: function (scope, element, attrs) {
            var dragger = $(element).find('.sub-window-drag');
            var resizer = $(element).find('.sub-window-resize');

            var dragInfo = {}

            var mousemove = function (event) {
                var deltaX = event.clientX - dragInfo.x;
                var deltaY = event.clientY - dragInfo.y;

                if (dragInfo.mode === 'move') {
                    $(element).css({
                        left: (dragInfo.pos.left + deltaX) + "px",
                        top: (dragInfo.pos.top + deltaY) + "px"
                    });
                } else if (dragInfo.mode === 'resize') {
                    scope.$apply(function () {
                        $(element).width(dragInfo.size.width + deltaX);
                        $(element).height(dragInfo.size.height + deltaY);
                    });
                }
            }

            var mousup = function () {
                $(window).off('mousemove', mousemove);
                $(window).off('mouseup', mousup);
            }

            var startDrag = function (event) {
                event.preventDefault();
                dragInfo.mode = 'move';
                dragInfo.pos = $(element).position();
                dragInfo.x = event.clientX;
                dragInfo.y = event.clientY;

                $(window).bind('mousemove', mousemove);
                $(window).bind('mouseup', mousup);
            }

            var startResize = function (event) {
                event.preventDefault();
                dragInfo.mode = 'resize';
                dragInfo.size = { 'height': $(element).height(), 'width': $(element).width() };
                dragInfo.x = event.clientX;
                dragInfo.y = event.clientY;

                $(window).bind('mousemove', mousemove);
                $(window).bind('mouseup', mousup);
            }

            if (resizer.length) {
                resizer.bind('mousedown', startResize);
            }

            if (dragger.length) {
                dragger.bind('mousedown', startDrag);
            }

            scope.$on('$destory', function () {
                if (resizer.length) {
                    resizer.off('mousedown', startResize);
                }

                if (dragger.length) {
                    dragger.off('mousedown', startDrag);
                }
            });

            angular.forEach(['top', 'left', 'bottom', 'right'], function (location) {
                if (attrs[location]) {
                    element.css(location, attrs[location]);
                }
            });
        }
    };
});

app.directive('srsDetails', function ($pyxIntercom) {
    return {
        restrict: 'E',
        replace: true,
        scope: {
            'srs': '=ngModel'
        },
        template: '\
<div>\
    <div style="padding:10px;">\
        <span i18n="SRS System"></span>:\
        <div class="radio-buttons">\
            <div class="radio-button" ng-class="{selected:srs.CoordinateSystem==\'Geographical\'}" ng-click="setSystem(\'Geographical\')" i18n="SRS Geographical"></div>\
            <div class="radio-button" ng-class="{selected:srs.Projection==\'UTM\' && srs.UtmNorth}" ng-click="setSystem(\'UTMNorth\')" i18n="SRS UTM North"></div>\
            <div class="radio-button" ng-class="{selected:srs.Projection==\'UTM\' && !srs.UtmNorth}" ng-click="setSystem(\'UTMSouth\')" i18n="SRS UTM South"></div>\
            <div class="radio-button" ng-class="{selected:srs.Projection==\'Custom\'}" ng-click="setSystem(\'Custom\')" i18n="SRS Custom"></div>\
        </div>\
    </div>\
    <div style="padding:10px;" ng-show="srs.Projection!=\'Custom\'">\
        <span i18n="SRS Datum"></span>:\
        <div class="radio-buttons">\
            <span class="radio-button" ng-class="{selected:srs.Datum==\'WGS84\'}" ng-click="srs.Datum=\'WGS84\'">WGS84</span>\
            <span class="radio-button" ng-class="{selected:srs.Datum==\'WGS72\'}" ng-click="srs.Datum=\'WGS72\'">WGS72</span>\
            <span class="radio-button" ng-class="{selected:srs.Datum==\'NAD27\'}" ng-click="srs.Datum=\'NAD27\'">NAD27</span>\
            <span class="radio-button" ng-class="{selected:srs.Datum==\'NAD83\'}" ng-click="srs.Datum=\'NAD83\'">NAD83</span>\
        </div>\
    </div>\
    <div style="padding:10px;" ng-show="srs.Projection==\'UTM\' || srs.system==\'UTM\'">\
        <span i18n="SRS UTM Zone"></span>:\
        <input type="number" class="form-control" placeholder="1 to 60" ng-model="srs.UtmZone" />\
    </div>\
    <div style="padding:10px;" ng-show="srs.Projection==\'Custom\'">\
        <span i18n="Projection String"></span>:\
        <input type="text" class="form-control" i18n-placeholder="custom projection" ng-model="srs.CustomProjection"/>\
    </div>\
</div>',
        link: function (scope) {
            scope.setSystem = function (system) {
                if (system === 'Geographical') {
                    scope.srs.CoordinateSystem = 'Geographical';
                    delete scope.srs.Projection;
                } else if (system === 'UTMNorth') {
                    scope.srs.CoordinateSystem = 'Projected';
                    scope.srs.Projection = 'UTM';
                    scope.srs.UtmNorth = true;
                } else if (system === 'UTMSouth') {
                    scope.srs.CoordinateSystem = 'Projected';
                    scope.srs.Projection = 'UTM';
                    scope.srs.UtmNorth = false;
                } else if (system === 'Custom') {
                    scope.srs.CoordinateSystem = 'Projected';
                    scope.srs.Projection = 'Custom';
                }
            }

            $pyxIntercom.track('SRS-dialog', scope.srs);
        }
    };
});


//globe controls directive.
//----------------------
//goal: show navigation controls on the globe
//require: css classes: nav-tools,compass-button,compass-north-arrow,
//                      tilt-button,location-button,zoom-in-button,
//                      zoom-out-button
//
//attributes:
//  'gotoUserLocation': Callback - implement user goto location action.
//                      If no function provided the goto location button will be hidden.
//
//example:
// <globe-controls></globe-controls>
app.directive('globeControls', function ($pyx, $timeout, $http, $window, $pyxIntercom, worldViewStudioConfig) {
    return {
        restrict: 'E',
        replace: true,
        scope: {
            'gotoUserLocation': '&'
        },
        template: '\
<div class="nav-tools" onmousedown="return false">\
    <div class="compass-button" ng-mousedown="startRotation($event)">\
        <div class="compass-north-arrow" ng-style="{\'-webkit-transform\': \'rotate(\'+(-northRotation)+\'deg)\'}"></div>\
        <div class="tilt-button" ng-click="tiltGlobe()" stop-propagation="mousedown dblclick" tooltip="{{\'tooltip.navigation.tilt\'|i18n}}"></div>\
    </div>\
    <div ng-if="hasGotoLocationCallback" class="location-button" ng-click="gotoUserLocation()" tooltip="{{\'tooltip.navigation.location\'|i18n}}"></div>\
    <div class="zoom-in-button" ng-click="zoomInGlobe()" tooltip="{{\'tooltip.navigation.zoom_in\'|i18n}}"></div>\
    <div class="zoom-out-button" ng-click="zoomOutGlobe()" tooltip="{{\'tooltip.navigation.zoom_out\'|i18n}}"></div>\
    <div class="scale" ng-style="{width: scaleWidth + \'px\'}" ng-class="{visible:scale}">\
        {{scale}}\
        <div class="scale-border-outline-2"></div>\
        <div class="scale-border-outline"></div>\
        <div class="scale-border"></div>\
    </div>\
</div>',
        link: function (scope, element, attr) {
            var win = angular.element($window);
            var scaleElement = element.find('.scale');

            scope.zoomGlobe = function (factor) {
                scope.camera = $pyx.globe.getCamera();
                scope.camera.Range = factor * scope.camera.Range;
                if (scope.camera.Range < worldViewStudioConfig.globe.cameraMinRange) {
                    scope.camera.Range = worldViewStudioConfig.globe.cameraMinRange;
                }
                $pyx.globe.setCamera(scope.camera, worldViewStudioConfig.globe.navigationAnimationDuration);
            }

            scope.zoomInGlobe = function () {
                scope.zoomGlobe(1.0 / worldViewStudioConfig.globe.zoomLevelFactor);
            }

            scope.zoomOutGlobe = function () {
                scope.zoomGlobe(worldViewStudioConfig.globe.zoomLevelFactor);
            }

            var tilt = 0;
            var tiltLevels = Math.floor(90 / worldViewStudioConfig.globe.tiltRotationAmount);
            var alive = true;

            scope.tiltGlobe = function () {
                tilt++;
                if (tilt === tiltLevels) {
                    tilt = 0;
                }
                scope.camera = $pyx.globe.getCamera();
                scope.camera.Tilt = tilt * worldViewStudioConfig.globe.tiltRotationAmount;
                $pyx.globe.setCamera(scope.camera, worldViewStudioConfig.globe.navigationAnimationDuration);
                
                $pyxIntercom.track('tilt-globe');
            }
            scope.northUp = function () {
                $pyxIntercom.track("compass-double-click");
                scope.camera = $pyx.globe.getCamera();
                scope.camera.Heading = 0;
                $pyx.globe.setCamera(scope.camera, worldViewStudioConfig.globe.northUpAnimationDuration);
            }

            scope.hasGotoLocationCallback = attr['gotoUserLocation'] != null;
            scope.userRotating = false;

            scope.northRotation = 0;
            scope.targetHeading = 0;
            scope.northRotationSpeed = 0;

            scope.lastClickTime = new Date();

            scope.startRotation = function ($event) {
                var now = new Date();
                var clickDelta = now.getTime() - scope.lastClickTime.getTime();
                scope.lastClickTime = now;

                //we need to detect dbl-click manually as we disable mousedown events
                if (clickDelta < 300) {
                    scope.northUp();
                    return;
                }

                var eventElement = $($event.target);
                var offset = eventElement.offset();
                var centerX = offset.left + eventElement.width() / 2;
                var centerY = offset.top + eventElement.height() / 2;

                function getAngleFromPosition(ev) {
                    var x = ev.pageX - centerX;
                    var y = ev.pageY - centerY;
                    var angle = Math.atan2(y, -x) * 180 / Math.PI;
                    return angle;
                }
                scope.camera = $pyx.globe.getCamera();
                var startHeading = scope.camera.Heading;
                var startAngle = getAngleFromPosition($event);
                var hasRotated = false;
                function mouseMove($event) {
                    var angle = getAngleFromPosition($event);
                    var delta = angle - startAngle;
                    if (!hasRotated && Math.abs(delta) > 1) {
                        hasRotated = true;
                        $pyxIntercom.track("compass-rotate-globe");
                    }
                    $timeout(function () {
                        scope.camera.Heading = startHeading + delta;
                        scope.northRotation = scope.camera.Heading;
                        $pyx.globe.setCamera(scope.camera);
                    });
                }

                function mouseUp() {
                    scope.userRotating = false;
                    win.off('mousemove', mouseMove);
                    win.off('mouseup', mouseUp);
                }

                scope.userRotating = true;
                win.bind('mousemove', mouseMove);
                win.bind('mouseup', mouseUp);
            };

            var animatePromise = null;

            function normalizeDegree(angle) {
                angle = angle % 360;
                if (angle < 0) {
                    angle += 360;
                }
                return angle;
            }

            function animateNorthArrow() {
                animatePromise = null;
                scope.targetHeading = normalizeDegree(Math.round(scope.camera.Heading));

                //scope.targetHeading is always between 0...360, we need to deal with that.
                //if scope.northRotation is 320 and target 30 - the animation we want to have is
                //320->360 = 0->30 - aka - 70 degree animation and not 290 degree animation from 320->30
                var delta = Math.round(scope.targetHeading - scope.northRotation);
                if (delta > 180) {
                    delta = delta - 360;
                } else if (delta < -180) {
                    delta = delta + 360;
                }
                //clamp delta/10 to range between -5 and 5
                delta = Math.max(-5, Math.min(5, delta / 10));

                //modify rotation speed. please note that speed change is %10 per frame
                //this create a nice swing animation.
                scope.northRotationSpeed = (scope.northRotationSpeed * 9 + delta) / 10;
                scope.northRotation += scope.northRotationSpeed;

                //make sure northRotation is always between 0...360
                scope.northRotation = normalizeDegree(scope.northRotation);

                //check if we need to stop the animation
                if (Math.abs(scope.northRotation - scope.targetHeading) < 0.5 && Math.abs(scope.northRotationSpeed) < 0.1) {
                    scope.northRotationSpeed = 0;
                    scope.northRotation = scope.targetHeading;
                } else {
                    animatePromise = $timeout(animateNorthArrow, 10);
                }
            }

            function startNorthArrowAnimation() {
                if (animatePromise) {
                    $timeout.cancel(animatePromise);
                    animatePromise = null;
                }
                animateNorthArrow();
            }

            var km = 1000;
            var factors = [1,2,5,10,20,50,100,200,500,1000,2000,5000,10000,20000,50000,100000,200000,500000,1000000,2000000];
            var scales = []

            angular.forEach(factors,function(factor) {
                var scale = {
                    factor: factor,
                    text: factor + " [m]"
                }
                if (factor>=1000) {
                    scale.text = (factor/km) + " [km]"
                }
                scales.push(scale);
            });

            var scalePreferedSize = 100;

            scope.scale = undefined;
            scope.scaleWidth = scalePreferedSize;

            function updateScale() {
                if (typeof wgs84 === 'undefined' || !wgs84) {
                    return;
                }
                if (scope.camera.Range < 3000 * km) {
                    var offset = scaleElement.offset();
                    var width  = scaleElement.width();
                    var height  = scaleElement.height();
                    var points = [];
                    points.push([offset.left+width-scalePreferedSize,offset.top+height/2])
                    points.push([offset.left+width,offset.top+height/2])

                    $pyx.globe.screenToGeographicPosition(points).success(function(locations) {
                        //if conversion failed
                        if (!locations[0] || !locations[1]) {
                            scope.scale = undefined;
                            return;
                        }

                        var p1 = wgs84.latLonToXyz({lat:locations[0][1],lon:locations[0][0]});
                        var p2 = wgs84.latLonToXyz({lat:locations[1][1],lon:locations[1][0]});
                        var distance = p1.angleTo(p2) * wgs84.earthRadius;
                        for (var i = 0; i < scales.length; i++ ){
                            var scaleFactor = scales[i].factor / distance;
                            if (scaleFactor >= 0.8 && scaleFactor < 2) {
                                scope.scale = scales[i].text;
                                scope.scaleWidth = scalePreferedSize * scaleFactor;
                                return;
                            }
                        }
                    });
                } else {
                    scope.scale = undefined;
                    scope.scaleWidth = scalePreferedSize;
                }
            }

            function updateCamera() {
                if ($pyx.globe.exists() && !scope.userRotating) {
                    if ('getCameraAsync' in $pyx.globe) {
                        $pyx.globe.getCameraAsync().success(function (camera) {
                            $timeout(function () {
                                scope.camera = camera;
                                startNorthArrowAnimation();
                                updateScale();
                            });
                        });
                    } else {
                        scope.camera = $pyx.globe.getCamera();
                        startNorthArrowAnimation();
                        updateScale();
                    }
                }
                if (alive) {
                    $timeout(updateCamera, worldViewStudioConfig.globe.cameraHeadingRefreshRate);
                }
            }

            updateCamera();

            scope.$on('$destroy', function () {
                alive = false;
            });
        }
    };
});

app.directive('searchResultMenu', function ($parse, connectionIndex, worldViewStudioConfig) {
    return {
        restrict: "E",
        templateUrl: "/client/templates/studio/template/search-result-menu.html",
        scope: {
            data: "=",
            itemSelector: "=",
            itemImporter: "=",
            searchServiceFilter: "="
        },
        link: function (scope, element, attrs) {

            scope.selectedService = null;

            scope.selectedItem = null;

            scope.serviceClicked = function (service) {
                // deselect service if selected
                if (service.selected) {
                    scope.selectItem(null, null);
                } else {
                    // select the service otherwise
                    scope.selectItem(null, service);
                }
            }

            scope.selectService = function (service) {
                if (scope.selectedService) {
                    scope.selectedService.selected = undefined;
                }
                if (service) {
                    service.selected = true;
                    scope.selectedService = service;
                }
            }

            scope.selectItem = function(item, service) {
                scope.itemSelector(item);
                scope.selectedItem = item;
                scope.selectService(service);
            }

            scope.importItem = function (item, service) {
                scope.selectItem(item, service);
                scope.itemImporter(item);
            }

        }
    }
});

//search result resource
//----------------------
//goal: show the detailed information about a search result
//require: /client/templates/studio/template/search-result-resource.html
//
//attributes:
//  'resource' : the resource to display
//  'currentMap': Pointer to currentMap object.
//                To allow the directive use current map state
//  'parentSelection': an array of selected items in the parent catalog
//                     (applicable to data sets and sub-catalogs)
//  'parentCatalog': a reference to the parent catalog of this resource
//                   (applicable to data sets and sub-catalogs)
//  'action': callback that get $action and $resource
//            $action: string that specify the action user
//                     perform on the the resource:
//                     'import','goto','toggle'
//            $resource: the resource to use. it can be mapItem
//                     or any worldview.gallery resource
//
//example:
// <search-result-resource resource="geoSource"
//                         current-map="currentMap"
//                         action="doAction($action,$resource)">
// </search-result-resource>
app.directive('searchResultResource', function ($pyx, $parse, $timeout, $filter, $pyxIntercom, connectionIndex, networkServiceStateAlert, worldViewStudioConfig) {
    return {
        restrict: "E",
        templateUrl: "/client/templates/studio/template/search-result-resource.html",
        scope: {
            resource: "=",
            currentMap: "=",
            parentSelection: "=",
            parentCatalog: "="
        },
        link: function (scope, element, attrs) {
            if (attrs.action) {
                var fn = $parse(attrs.action);
                scope.invoke = function (action, resource) {
                    return fn(scope.$parent, { '$action': action, '$resource': resource });
                }
            }

            if (!scope.resource) {
                return;
            }

            var i18n = $filter('i18n');

            //check if search result have license attached to it.
            if (scope.resource.Licenses) {

                //We want to support cases when the studio allows anonymous access
                //If we have a user... track their license
                if ($pyx.user.auth()) {
                    //check with the worldview.gallery if user have access to that resource
                    $pyx.gallery.licensedAccess(scope.resource.Id).then(function (result) {
                        // user has licensed access to the resource already
                        if (result.data.HasAccess) {
                            scope.hasAccess = true;
                        } else {
                            scope.hasAccess = false;
                        }
                    });
                } else {
                    //assume no access was granted before
                    scope.hasAccess = false;
                }
            } else {
                //not a resource,
                scope.hasAccess = undefined;
            }

            scope.beginImport = function () {
                if ($pyx.user.auth()) {
                    scope.showAgreement = true;
                } else {
                    scope.commitImport();
                }
            }

            scope.commitImport = function () {
                scope.showAgreement = false;
                scope.invoke('import', scope.resource);
            }

            scope.importSelected = function () {
                scope.showAgreement = false;
                // copy the selected data sets
                var selectedResource = angular.copy(scope.resource);
                selectedResource.DataSets = scope.selectedChildren;
                // set the importing flags for the catalog and the selected data sets
                scope.resource.importing = true;
                angular.forEach(selectedResource.DataSets, function(dataSet) {
                    dataSet.importing = true;
                });
                // request the import
                scope.invoke('import', selectedResource);
            }

            scope.cancelImport = function () {
                scope.showAgreement = false;
            }

            scope.addToIndex = function () {
                connectionIndex.removeItem(scope.resource);
                scope.invoke('addItemToIndex', scope.resource);
                scope.indexed = true;
            }

            scope.removeFromIndex = function () {
                scope.invoke('removeItemFromIndex', scope.resource);
                scope.indexed = false;
            }

            scope.addDashboardWidget = function (type) {
                scope.invoke("addWidget", {
                    item: scope.resource.MapItem,
                    property: scope.resource.Field,
                    type: type
                });
            }

            function answerQuestion() {
                scope.status = "calculating";
                $pyx.when($pyx.engine.answerQuestion(scope.resource.Question, $pyx.obj.get(scope.currentMap, 'model'))).then(function (answer) {
                    if ("NumberValue" in answer) {
                        scope.answer = answer.NumberValue;
                        scope.status = "value calculated";
                        $pyxIntercom.track('ask-question');
                    } else if ("Geometry" in answer) {
                        scope.status = "geometry calculated";
                        scope.answer = answer.Geometry;
                        scope.selectGeometry = function () { scope.invoke("changeSelection", scope.answer); }
                        $pyxIntercom.track('ask-question');
                    } else {
                        scope.status = "failed";
                        $pyxIntercom.track('ask-question-failed');
                    }

                }, function (error) {
                    scope.status = "failed";
                    $pyxIntercom.track('ask-question-failed');
                });
            }

            function calculatePropertyStats() {
                if (scope.currentMap) {
                    scope.currentMap.getGeoSource(scope.resource.MapItem).then(function (geoSource) {
                        scope.item = geoSource;

                        scope.isCoverage = scope.item.Specification.OutputType === 'Coverage';

                        scope.status = "calculating";

                        var geometry = $pyx.obj.get(scope.currentMap, 'model', 'Dashboards', 0, 'Selection', 'Geometry');

                        if (scope.isCoverage && !geometry) {
                            scope.status = "no-selection";
                            delete scope.range;
                            delete scope.stats;
                            return;
                        }

                        var getStats;

                        if (geometry) {
                            getStats = $pyx.engine.getFieldStatisticsAt(
                                geoSource,
                                scope.resource.Field.Name,
                                geometry,
                                worldViewStudioConfig.whatIsHere.histogramBinCount);
                        } else {
                            getStats = $pyx.engine.getFieldStatistics(
                                geoSource,
                                scope.resource.Field.Name,
                                worldViewStudioConfig.whatIsHere.histogramBinCount);
                        }

                        getStats.success(function (stats) {
                            $timeout(function () {
                                scope.stats = stats;
                                scope.status = "calculated";

                                var bins = stats.Distribution.Histogram;
                                var valueFormat = $filter('valueFormat');
                                scope.graph = {
                                    data: bins.map(function (bin) {
                                        return (bin.MinCount + bin.MaxCount) / 2;
                                    }),
                                    labels: bins.map(function (bin) {
                                        return bin.Min === bin.Max ? valueFormat(bin.Min) : (valueFormat(bin.Min) + " ... " + valueFormat(bin.Max));
                                    })
                                };

                                scope.getRangeValue = function (pos) {
                                    var index = Math.floor(pos);

                                    if (index === bins.length) {
                                        return bins[index - 1].Max;
                                    }
                                    if (index >= 0 && index < bins.length) {
                                        var binMin = bins[index].Min;
                                        if (angular.isNumber(binMin)) {
                                            var binMax = bins[index].Max;
                                            var delta = pos - index;
                                            return binMin + (binMax - binMin) * delta;
                                        } else {
                                            return binMin;
                                        }
                                    }
                                };

                                scope.setGraphTooltip = function (index, event) {
                                    var bar = $(event.target);
                                    bar.attr('class', 'hover');
                                    var offset = bar.offset();

                                    scope.tooltip = {
                                        show: true,
                                        top: offset.top + 5,
                                        left: offset.left + 2,
                                        value: scope.graph.data[index],
                                        name: scope.graph.labels[index]
                                    };

                                    if (scope.isCoverage) {
                                        scope.tooltip.value = $filter('area')(scope.tooltip.value);
                                    }
                                }

                                scope.hideGraphTooltip = function (index, event) {
                                    var bar = $(event.target);
                                    bar.attr('class', '');
                                    scope.tooltip = {
                                        show: false
                                    };
                                }

                                if (geometry && stats.Distribution.Histogram.length > 0) {
                                    scope.range = {
                                        min: 0,
                                        max: stats.Distribution.Histogram.length,
                                        minRange: 0,
                                        maxRange: stats.Distribution.Histogram.length
                                    };

                                    scope.fullRange = function () {
                                        return scope.range.min === scope.range.minRange &&
                                            scope.range.max === scope.range.maxRange;
                                    }

                                    scope.canDoWhereIsIt = ('whereIntersection' in $pyx.engine);

                                    scope.doWhereIsIt = function () {
                                        scope.invoke("performWhere", {
                                            item: scope.resource.MapItem,
                                            property: scope.resource.Field,
                                            range: {
                                                Min: scope.getRangeValue(scope.range.min),
                                                Max: scope.getRangeValue(scope.range.max)
                                            }
                                        });
                                    }

                                } else {
                                    delete scope.range;
                                }
                            });
                        }).error(function () {
                            scope.status = "failed";
                        });
                    });
                }
            }

            scope.$watch('currentMap.model.Dashboards[0].Selection.Geometry', function () {
                if ($pyx.obj.get(scope.resource, 'Type') === 'Property') {
                    calculatePropertyStats();
                }
            });

            // data sets selected inside this resource (if it's a catalog)
            scope.selectedChildren = null;

            // initialize the selected children
            if (scope.resource.Type === "Catalog") {
                scope.selectedChildren = [];
            }

            // handles a change in the list of selected children
            scope.checkSelectedChildren = function () {
                if (scope.selectedChildren != null && scope.selectedChildren.length === 0) {
                    scope.resource.importing = false;
                }
            }

            scope.$watch("selectedChildren.length", function () {
                scope.checkSelectedChildren();
            });

            scope.$watch("resource.selected", function () {
                if (scope.resource.selected) {
                    scope.parentSelection.push(scope.resource);
                } else {
                    $pyx.array.removeFirst(scope.parentSelection, scope.resource);
                }
            });

            // check whether there is a mismatch between the scope resource (that is presumably a data set)
            // and a given data set;
            // returns true if no mismatch appears
            function dataSetMatchLayerAndFields(dataSet) {
                return (!scope.resource.Layer || (dataSet.Layer === scope.resource.Layer))
                    && (!scope.resource.Fields || ($pyx.array.equals(dataSet.Fields, scope.resource.Fields)));
            }

            // Determines whether the scope resource corresponds to an item in the map
            // (either imported or being imported or sent with an event)
            function isMapItemMatch(item) {
                // If the item is an import in progress, just compare the URIs and Layers
                if (item.dataSet) {
                    // if the layers or fields don't match, return false
                    if (!dataSetMatchLayerAndFields(item.dataSet)) {
                        return false;
                    }
                    return $pyx.obj.equalsCaseInsensitive(item.dataSet.Uri, scope.resource.Uri);
                }
                // Otherwise, we assume the item is an imported GeoSource
                switch (scope.resource.Type) {
                case "Property":
                    // compare Properties by the Id
                    return item.Resource.Id === scope.resource.MapItem.Resource.Id;
                case "GeoSource":
                    // compare GeoSources by the Id
                    return scope.resource.Id === item.Id || (item.Resource && item.Resource.Id === scope.resource.Id);
                case "DataSet":
                    // if the GeoSource has a stored information about the data set it was imported from, use it
                    if (item.DataSet) {
                        // if the layers or fields don't match, return false
                        if (!dataSetMatchLayerAndFields(item.DataSet)) {
                            return false;
                        }
                        // compare the URIs
                        return $pyx.obj.equalsCaseInsensitive(item.DataSet.Uri, scope.resource.Uri);
                    } else {
                        // try compare external data sets by the URI (if provided)
                        if (item.Metadata && item.Metadata.ExternalUrls && item.Metadata.ExternalUrls.length) {
                            return $pyx.array.firstIndex(item.Metadata.ExternalUrls, function (external) {
                                return external && external.Type === 'Reference'
                                    && $pyx.obj.equalsCaseInsensitive(external.Url, scope.resource.Uri);
                            }) !== -1;
                        } else {
                            // last resort: compare data sets by name and description
                            // (as this is the last resot option, make it case sensitive)
                            return item.Metadata.Name === scope.resource.Metadata.Name
                                && item.Metadata.Description === scope.resource.Metadata.Description;
                        }
                    }
                default:
                    return false;
                }
            }

            // Searches an item (GeoSource) in the map that corresponds to the scope resource,
            // and updates the scope model if found
            function findResourceInMap() {
                if (scope.currentMap) {
                    var matchItems = scope.currentMap.items(isMapItemMatch);
                    if (matchItems) {
                        scope.item = matchItems[0];
                    }
                }
            }

            // Searches a GeoSource image of the scope resource
            function setImage() {
                if (scope.resource) {
                    var imageIndex = $pyx.array.firstIndex(scope.resource.Metadata.ExternalUrls, function (external) {
                        return external.Type === 'Image';
                    });
                    if (imageIndex === -1) {
                        scope.image = "";
                    } else {
                        scope.image = scope.resource.Metadata.ExternalUrls[imageIndex].Url;
                    }
                    // Set the custom OGC banner, if required
                    if (!scope.image && scope.resource.DataType === 'OGC') {
                        scope.image = "/assets/images/studio/ogc_banner.png";
                    }
                    // Set the custom ArcGIS banner, if required
                    if (!scope.image && scope.resource.DataType === 'ArcGIS') {
                        scope.image = "/assets/images/studio/arcgis_banner.png";
                    }
                    // Set the banner for local data
                    if (!scope.image && scope.resource.DataType === 'Local') {
                        scope.image = "/assets/images/studio/local_banner.png";
                    }
                    // We want to be able to catch when an image fails to load, and inform the network
                    // verification service (some images are stored on webfaction).
                    // One way is listening on 'onerror' event of an '<image>' HTML element,
                    // however the images are not always rendered with the element.
                    // So we use a workaround: check for the image availability by trying to
                    // explicitly load the image, and listen on the errors.
                    if (scope.image) {
                        var img = new Image();
                        img.onerror = function () {
                            networkServiceStateAlert('searchResultResource', false, i18n('Failed to load the image for \' %s \'', scope.resource.Metadata.Name));
                            img.onerror = null;
                        }
                        img.src = scope.image;
                    }
                }
            }

            // determines whether the resource (only if it's a catalog) belongs to the connection index
            scope.indexed = false;
            scope.isSubCatalog = false;
            scope.dataStatus = null;
            scope.lastError = null;

            // Refreshes the scope model data, that binds the directive with the view
            function refreshResource(value) {
                scope.item = null;
                scope.lastError = null;
                scope.subResources = [];
                delete scope.stats;
                delete scope.graph;

                if (scope.selectedChildren) {
                    // remove the selections but don't update the array object
                    scope.selectedChildren.splice(0, scope.selectedChildren.length);
                }

                //search the resource in the current map by its ID
                findResourceInMap();

                //look for an image accompanying the resource, to display in the view
                setImage();

                //custom actions
                if (scope.resource) {
                    switch (scope.resource.Type) {
                    case "Property":
                        calculatePropertyStats();
                        break;
                    case "Gallery":
                        $pyx(scope.resource).resources().success(function (resources) {
                            if (scope.resource === value) {
                                scope.subResources = resources;
                            }
                        });
                        break;
                    case "Question":
                        answerQuestion();
                        break;
                    case "Catalog":
                        //copy data sets and catalogs as subresources...
                        scope.subResources = [].concat(scope.resource.DataSets, scope.resource.SubCatalogs);
                        // check if the catalog belongs to the Catalog Index
                        scope.indexed = (connectionIndex.findItem(scope.resource) || connectionIndex.findItem(scope.resource.Uri)) ? true : false;
                        scope.dataStatus = scope.resource.Status;
                        if (scope.dataStatus === "Failed") {
                            scope.subResources = [];
                            scope.lastError = i18n("Failed to load data");
                        }
                        if (scope.parentSelection) {
                            scope.isSubCatalog = true;
                        }
                        break;
                    }
                }
            }

            scope.reload = function () {
                // hide the sub-catalogs and data sets (if any)
                scope.subResources = null;
                scope.dataStatus = "Loading";
                $pyx.when(scope.invoke('reload', scope.resource)).then(function (resource) {
                    scope.dataStatus = scope.resource.Status || "Ready";
                    refreshResource(scope.resource);
                },
                function(error) {
                    scope.resource.Status = "Failed";
                    scope.dataStatus = scope.resource.Status;
                    // note: display the actual error message to the user
                    scope.lastError = error || i18n("Failed to load data");
                });
            }
            scope.$watch("resource.Status", function (status) {
                // Don't call unless there is a valid 'status'
                if (!status) {
                    return;
                }
                refreshResource();
            });

            // when a sub-catalog inside the resource (if it's a catalog) gets expanded
            // (the current resource gets hidden)
            scope.expandSubCatalog = function() {
                scope.parentCatalog.expandedSubCatalog = scope.resource;
                scope.resource.expanded = true;
                }

            // when a sub-catalog inside the resource (if it's a catalog) gets unexpanded
            // (the current resource gets visible in front again)
            scope.rollUpSubCatalog = function () {
                scope.resource.expandedSubCatalog.expanded = false;
                delete scope.resource.expandedSubCatalog;
            }

            // Determines whether import of the resource is currently in progress
            scope.resource.importing = false;

            // used to display the importing message for 1 sec
            scope.showImportingMessageTimeout = undefined;

            //start showing the importing message
            scope.startImportingMessage = function () {
                scope.resource.importing = true;
                scope.showImportingMessageTimeout = $timeout(function () {
                    //this happen after 1 sec
                    scope.showImportingMessageTimeout = undefined;
                    scope.hideImportingMessageIfNeeded();
                }, 1000);
            }

            //hide importing message if we can.
            scope.hideImportingMessageIfNeeded = function () {
                //we set importing = false when:
                //1) item has been found
                //2) 1 sec has passed
                if (scope.item && !scope.showImportingMessageTimeout) {
                    scope.resource.importing = false;
                }
            }

            // Listeners of changes in the current map,
            // required for instant updates of the view according to the state of the GeoSource/data set
            scope.mapListeners = [];

            function unregisterMapListeners() {
                angular.forEach(scope.mapListeners, function (listener) {
                    // Executing an output of a $watch() unregisters it
                    listener();
                });
                scope.mapListeners = [];
            }

            function registerMapListeners() {
                unregisterMapListeners();

                //make sure we have a resource attached - this can happen in case the parent scope remove our item.
                if (!scope.resource) {
                    return;
                }
                //listener for an import start
                scope.mapListeners.push(scope.$on('import-started', function (e, args) {
                    if (isMapItemMatch(args.resource)) {
                        scope.startImportingMessage();
                    }
                }));
                //listener for successful imports
                scope.mapListeners.push(scope.$on('import-succeeded', function (e, args) {
                    if (isMapItemMatch(args.resource)) {
                        if (scope.resource.Type === "DataSet") {
                            // triggers when importing starts and finishes
                            scope.mapListeners.push(scope.$watch("currentMap.state.Imports.length", function () {
                                findResourceInMap();
                            }));
                        }
                        else {
                            findResourceInMap();
                        }
                        // Data sets only
                        $pyx.array.removeFirst(scope.parentSelection, scope.resource);
                        scope.resource.selected = false;
                    }
                }));
                //listener for failed imports
                scope.mapListeners.push(scope.$on('import-failed', function (e, args) {
                    if (isMapItemMatch(args.resource)) {
                        findResourceInMap();
                        scope.resource.importing = false;
                        // Data sets only
                        $pyx.array.removeFirst(scope.parentSelection, scope.resource);
                        scope.resource.selected = false;
                    }
                }));
            }

            //listener for a GeoSource removal
            scope.$on('map-item-removed', function (e, args) {
                if (!args || isMapItemMatch(args.item)) {
                    findResourceInMap();
                }
            });

            //listener for an import cancellation
            scope.$on('import-cancelled', function (e, args) {
                if (isMapItemMatch(args.item)) {
                    scope.resource.importing = false;
                    // Data sets only
                    $pyx.array.removeFirst(scope.parentSelection, scope.resource);
                    scope.resource.selected = false;
                }
            });

            //listener for a new item selected in the search result menu
            scope.$on('search-result-selected', function (e, args) {
                // clean up properties that are not supposed to be saved when switching
                // between items in the search results menu
                if (scope.resource) {
                    // clean up properties that are responsible for multi-level rendering;
                    // note: this implements the requirement to forget where the user drilled down
                    // inside a top level catalog, as soon as he switched to another item
                    // in the search results window or updated the search query
                    delete scope.resource.expanded;
                    delete scope.resource.expandedSubCatalog;
                    // reset the sub-resource rendering limit inside the resource
                    scope.resource.resultLimit = 50;
                }
            });

            scope.$watch("item", function () {
                // Unregister listeners that are no longer needed
                unregisterMapListeners();
                scope.hideImportingMessageIfNeeded();
            });

            scope.$watch("resource", function (value) {
                // Registers new listeners, that depend on the type of the resource
                registerMapListeners();
                // Refresh the scope model
                refreshResource(value);
            });

            scope.$watch("currentMap", function (map) {
                // Refresh the scope model
                if (!map) {
                    return;
                }
                refreshResource();
            });
        }
    }
});

/*
- @name editFilePath
- @desc Extracts the file name or file and parent 
- folder name to build a cleaner, abridged path 
- to local data files.
*/
app.filter('editFilePath', function () {
    /*
    - @param {string} pathName
    - @param {number} pathIndex - Append another piece to the output path
    - @example 2 takes an input '/Users/username/Documents/Pyxis/Data/somefile.shp'
    - and returns 'Data/somefile.shp'
    - @example 3 takes an input '/Users/username/Documents/Pyxis/Data/somefile.shp'
    - skips the closest parent container and returns 'Pyxis/somefile.shp'
    */
    return function (pathName, pathIndex) {
        var pathIndex = +pathIndex || false;
        var pathPieces = pathName.split('\\');
        var numbPieces;
 
        if (!angular.isString(pathName)) {
            return;
        }

        // Find and remove empty strings in the pathPieces array
        for (var index = 0; index < pathPieces.length; index++) {
            if (pathPieces[index] === "") {
                pathPieces.splice(index, 1);
            }
        }

        numbPieces = pathPieces.length;
   
        if (pathIndex && numbPieces > pathIndex) {
            return (pathPieces[numbPieces - pathIndex] + '\\' + pathPieces[numbPieces - 1]);
        } else if (!pathIndex && numbPieces) {
            return pathName.replace(/^.*[\\\/]/, '');
        } else {
            return pathName;
        }
    }
});

//library item legend
//----------------------
//goal: show the legend for a library item
//require: /client/templates/studio/template/library-item-legend.html
//
//attributes:
//  'item' : the library item legend to display
//  'styleModifier': Pointer to styleModifier object
//
//example:
//<library-item-legend item="item" style-modifier="styleModifier"></library-item-legend>
app.directive('libraryItemLegend', function ($pyx, $filter, $pyxIntercom, paletteService, geoSourceCache) {
    return {
        restrict: "E",
        templateUrl: "/client/templates/studio/template/library-item-legend.html",
        scope: {
            item: "=",
            styleModifier: "=",
            copyAndPaste: "="
        },
        link: function (scope) {
            var orderBy = $filter('orderBy');

            //http://stackoverflow.com/questions/175739/is-there-a-built-in-way-in-javascript-to-check-if-a-string-is-a-valid-number
            function isNumeric(num) {
                return !isNaN(num);
            }

            scope.editModel = {
                editable: false,
                editing: false,
                valueType: undefined, //"Number" | "String" | "Color"
                palette: undefined, // palette object,
                oldPalette: undefined,
                newStepValue: "",
                displayElevationControls: true,
                startEditing: function () {
                    scope.editModel.editing = true;
                    scope.editModel.palette = paletteService.verify(scope.editModel.palette, scope.editModel.valueType);
                    scope.editModel.oldPalette = angular.copy(scope.editModel.palette);
                    if (scope.editModel.valueType === "String") {
                        $pyxIntercom.track("edit-string-gradient");
                    } else {
                        $pyxIntercom.track("edit-gradient");
                    }
                },
                autoStyle: function (currentPalette) {
                    scope.editModel.autoStyling = true;

                    //normalize the palette steps to make sure we can rerun auto-palette again and again
                    var palette = paletteService.verify(currentPalette, scope.editModel.valueType);
                    var steps = palette.Steps.length;
                    for (var i = 0; i < steps; i ++) {
                        palette.Steps[i].Value = (i + 0.0) / steps;
                    }

                    scope.styleModifier.updatePalette(scope.item, palette, { useScreenBasedStyling: true });
                },
                commitEditing: function (newPalette) {
                    var palette = paletteService.verify(newPalette, scope.editModel.valueType);

                    scope.editModel.editing = false;
                    scope.styleModifier.setItemPalette(scope.item, palette);
                    scope.editModel.palette = palette;
                },
                toggleShowAsElevation: function () {
                    scope.styleModifier.toggleShowAsElevation(scope.item);
                },
                cancelEditing: function () {
                    var palette = paletteService.verify(scope.editModel.oldPalette, scope.editModel.valueType);
                    scope.editModel.editing = false;
                    scope.styleModifier.setItemPalette(scope.item, palette);
                    scope.editModel.palette = scope.editModel.oldPalette;
                    scope.editModel.oldPalette = undefined;
                },
                verify: function () {
                    scope.editModel.palette = paletteService.verify(scope.editModel.palette, scope.editModel.valueType);
                },
                removeStep: function (step, index) {
                    if (scope.editModel.palette.Steps.length > 2) {
                        scope.editModel.palette.Steps.splice(index, 1);
                        scope.editModel.verify();
                    }
                },
                addStep: function () {
                    if (scope.editModel.newStepValue !== "") {
                        scope.editModel.palette = paletteService.addStep(scope.editModel.palette, {
                            Value: scope.editModel.newStepValue,
                            Color: paletteService.randomColor()
                        }, scope.editModel.valueType);
                        scope.editModel.newStepValue = "";
                    }
                }
            };

            // Duck typing for pipelines published before 'Specification' object added.
            // We check suitability by determining if the Style object contains
            // a Palette object
             function determineSuitability (item) {
                var palette;
                var stepValue;

                if (scope.styleModifier.hasPaletteStyle(item)) {
                    // Valid types that can contain Palette objects
                    palette = item.Style.Fill.Palette || item.Style.Icon.Palette;

                    if (palette && palette.Steps.length) {
                        // We'll assume value type based on the first step
                        stepValue = palette.Steps[0].Value;

                        scope.editModel.editable = true;
                        scope.editModel.oldPalette = palette;
                        scope.editModel.palette = palette;
                        scope.editModel.valueType = isNumeric(stepValue) ? "Number" : "String";
                    }
                }
            }

            // Check if the item has a Specification property
            // When it doesn't have Specification - try to get it
            function ensureItemHasSpecification (onComplete) {
                if (scope.item.Specification && scope.item.Specification.OutputType) {
                    onComplete();
                } else {
                    geoSourceCache.getWithSpecification(scope.item.Resource.Id).then(function (result) {
                        scope.item.Specification = result.Specification;
                        onComplete();
                    }).catch(function() {
                        onComplete();
                    });
                }
            };

            function classifyFieldType(style) {
                // scope.editModel.editable = false;
                scope.numericAsText = false;
                scope.prefixLength = 0;

                var item = scope.item;

                if (style && item.Specification) {
                    var fieldSpecIndex = $pyx.array.firstIndex(item.Specification.Fields,
                        function (field) {
                            return field.Name === style.PaletteExpression;
                        });

                    if (fieldSpecIndex !== -1) {
                        scope.editModel.valueType = item.Specification.Fields[fieldSpecIndex].FieldType;

                        //this can be used to have different editors for different valueTypes
                        //scope.editModel.editable = scope.editModel.valueType === "Number";

                        scope.editModel.editable = true;
                        scope.editModel.oldPalette = style.Palette;
                        scope.editModel.palette = style.Palette;

                        // Turn off Elevation controls for vector Geosources
                        scope.editModel.displayElevationControls = !(item.Specification.OutputType === "Feature");

                        // if all labels are numbers, and they have been sorted as text, truncate
                        // all labels to the length of the shortest one
                        // this helps avoid confusion when the user expects numeric ordering
                        if (item.Specification.Fields[fieldSpecIndex].FieldType === "String") {
                            var minNumberLength = Number.MAX_VALUE;

                            // trim whitespace to ensure collections are sorted properly
                            for (var i = 0; i < style.Palette.Steps.length; i++) {
                                var value = style.Palette.Steps[i].Value;
                                if (angular.isString(value)) {
                                    style.Palette.Steps[i].Value = value.trim();
                                }
                            }

                            style.Palette.Steps = orderBy(style.Palette.Steps, 'Value');

                            for (var i = 0; i < style.Palette.Steps.length; i++) {
                                if (!style.Palette.Steps[i].Value ||
                                    !isNumeric(style.Palette.Steps[i].Value)) {
                                    break;
                                }
                                minNumberLength = Math.min(style.Palette.Steps[i].Value.length, minNumberLength);
                            }

                            if (i >= style.Palette.Steps.length) {
                                scope.numericAsText = true;
                                scope.prefixLength = minNumberLength;
                            }
                        }
                    }
                } else {
                    // Fail safe
                    determineSuitability(item);
                }
            }

            //when the style changes to a solid color - remove the Editor
            scope.$on('item-style-changed', function (event, item) {
                if (scope.styleModifier.isSameGeoSource(item.item, scope.item)) {
                    scope.editModel.editable = scope.styleModifier.hasPaletteStyle(item.item);
                    scope.editModel.editing = false;
                }
                // This needs to be set to false
                // stops the loading gif from endlessly cycling
                scope.editModel.autoStyling = false;
            });

            scope.$watch('item.Style', function () {
                if (!scope.item.Style) {
                    return;
                }

                ensureItemHasSpecification(function () {
                    classifyFieldType(scope.item.Style.Fill || scope.item.Style.Icon);
                });
            }, true);
        }
    }
});

/* Begin Prototype Section
 * ==================================================================
 */

app.directive("itemLegend", function ($pyx, $timeout, geoSourceCache) {
    function postLink (scope, element, attrs) {
        var index = +attrs.itemIndex;

        var safeKeys = {
            style: 'Style',
            expression: 'PaletteExpression',
            fill: 'Fill',
            line: 'Line',
            icon: 'Icon',
            name: 'Name'
            };

        scope.setRange = function (groups, key) {
            var map = function (group) {
                return group[key];
            };

            return {
                min: Math.min.apply(0, groups.map(map)),
                max: Math.max.apply(0, groups.map(map))
            }
        };

        function listener (model) {
            function extractMetaData (model) {
                if ($pyx.obj.get(model, safeKeys.style)) {
                    scope.hasExpression =_.chain(model[safeKeys.style])
                     .pick(safeKeys.fill, safeKeys.icon, safeKeys.line)
                     .findKey(safeKeys.expression)
                     .value();

                    scope.theResource = model[safeKeys.style][scope.hasExpression];
                    scope.outputType = model.Specification.OutputType;

                    if (scope.hasExpression) {
                        scope.getExpression = scope.theResource[safeKeys.expression];

                        _.each(model.Specification.Fields, function (field, index) {
                            if (field.Name === scope.getExpression && _.has(field.Metadata, safeKeys.name)) {
                                scope.getExpression = field.Metadata.Name;
                                if (_.has(field, 'FieldUnit') && field.FieldUnit) {
                                    scope.fieldUnitName = field.FieldUnit.Name;
                                }
                            }
                        });
                    }
                }
            }

            if (_.has(model, 'Specification') && _.has(model.Specification, 'OutputType')) {
                extractMetaData(model);
                return;
            }

            geoSourceCache.getWithSpecification(model.Resource.Id).then(function (result) {
                model.Specification = result.Specification;
                extractMetaData(model);
            }).catch(extractMetaData);

        }

        $timeout(function () {
            element.removeClass('state-initial');
        }, (index * 200));

        scope.$watch(attrs.itemLegend, listener, true);

    }
    return {
        restrict: "A",
        scope: true,
        link: postLink
    }
});

/* End Prototype Section
 * ==================================================================
 */

//grid layout directive.
//----------------------
//goal: allow order items in grid structure.
//require: grid-layout css class, grid-item css class
//example:
//
// <div class="grid-layout" rows=5 cols=6>
//   <div class="grid-item" colSpan=4>Hello</div>
//   <div class="grid-item" rowSpan=2 colSpan=5>World</div>
//   <div class="grid-item" col=4 row=4>Specific grid cell</div>
// </div>
//
// grid-layout attributes:
//   item-width : width in pixels (default 100)
//   item-height: height in pixels (default item-width)
//   h-space: space between grid items in pixel (default 10)
//   v-space: space between grid items in pixel (default h-space);
//   cols: number of columns in grid (default 3)
//   rows: number of rows in grid (default 4)
//   expand-rows: to allow grid to add rows (default true)
app.directive('gridLayout', function () {
    return {
        restrict: 'C',

        controller: function ($scope, $element, $attrs) {
            this.itemWidth = Math.max(parseFloat($attrs['itemWidth'] || 100), 1);
            this.itemHeight = Math.max(parseFloat($attrs['itemHeight'] || this.itemWidth), 1);
            this.hSpace = Math.max(parseFloat($attrs['hSpace'] || 10), 0);
            this.vSpace = Math.max(parseFloat($attrs['vSpace'] || this.hSpace), 0);
            this.cols = Math.max(parseFloat($attrs['cols'] || 3), 1);
            this.rows = Math.max(parseFloat($attrs['rows'] || 4), 1);
            this.expandRows = $attrs['expandRows'] !== 'false';

            this.items = [];
            var self = this;

            var allocations = function (rows, cols) {
                var alloc = {
                    'rows': rows,
                    'cols': cols,
                    'allocations': []
                };

                for (var y = 0; y < alloc.rows; y++) {
                    alloc.allocations.push([]);
                    for (var x = 0; x < alloc.cols; x++) {
                        alloc.allocations[y].push(false);
                    }
                }

                alloc.markUsed = function (row, col, rowSpan, colSpan) {
                    for (var y = row; y < row + rowSpan; y++) {
                        for (var x = col; x < col + colSpan; x++) {
                            alloc.allocations[y][x] = true;
                        }
                    }
                }

                alloc.isFree = function (row, col, rowSpan, colSpan) {
                    for (var y = row; y < row + rowSpan; y++) {
                        for (var x = col; x < col + colSpan; x++) {
                            if (alloc.allocations[y][x]) {
                                return false;
                            }
                        }
                    }
                    return true;
                }

                alloc.findFree = function (rowSpan, colSpan, expandRows) {
                    var x, y;
                    for (y = 0; y < alloc.rows - rowSpan + 1; y++) {
                        for (x = 0; x < alloc.cols - colSpan + 1; x++) {
                            if (alloc.isFree(y, x, rowSpan, colSpan)) {
                                return { 'row': y, 'col': x }
                            }
                        }
                    }
                    if (expandRows) {
                        alloc.rows += 1;
                        alloc.allocations.push([]);
                        for (x = 0; x < alloc.cols; x++) {
                            alloc.allocations[y].push(false);
                        }
                        return this.findFree(rowSpan, colSpan, expandRows);
                    }
                    return undefined;
                }

                alloc.size = function () {
                    var width = 0;
                    var height = 0;
                    for (var y = 0; y < alloc.rows ; y++) {
                        for (var x = 0; x < alloc.cols; x++) {
                            if (alloc.allocations[y][x]) {
                                if (width < x + 1) {
                                    width = x + 1;
                                }
                                if (height < y + 1) {
                                    height = y + 1;
                                }
                            }
                        }
                    }
                    return { 'width': width, 'height': height }
                }

                return alloc;
            };

            this.doLayout = function () {
                var alloc = allocations(self.rows, self.cols);
                angular.forEach(self.items, function (item) {
                    var element = item.element;
                    var col = item.col;
                    var row = item.row;
                    var colSpan = Math.min(parseInt(item.colSpan || "1"), self.cols);
                    var rowSpan = Math.min(parseInt(item.rowSpan || "1"), self.rows);
                    if (!angular.isNumber(row) || !angular.isNumber(col)) {
                        var allocation = alloc.findFree(rowSpan, colSpan, self.expandRows);
                        if (allocation) {
                            row = allocation.row;
                            col = allocation.col;
                        }
                    }

                    if (!angular.isNumber(row) || !angular.isNumber(col)) {
                        element.css({ 'display': 'none' });
                    } else {
                        alloc.markUsed(row, col, rowSpan, colSpan);
                        element.css({
                            'position': 'absolute',
                            'left': col * (self.itemWidth + self.hSpace) + 'px',
                            'top': row * (self.itemHeight + self.vSpace) + 'px',
                            'width': (colSpan * (self.itemWidth + self.hSpace) - self.hSpace) + 'px',
                            'height': (rowSpan * (self.itemHeight + self.vSpace) - self.vSpace) + 'px'
                        });
                    }
                });

                var size = alloc.size();

                $element.css({
                    'width': (size.width * (self.itemWidth + self.hSpace) - self.hSpace) + 'px',
                    'height': (size.height * (self.itemHeight + self.vSpace) - self.vSpace) + 'px'
                });
            }

            this.addChild = function (item) {
                self.items.push(item);
                self.doLayout();
            }
            this.removeChild = function (item) {
                var index = self.items.indexOf(item);
                self.items.splice(index, 1);
                self.doLayout();
            }
        }
    }
});

//grid item directive.
//----------------------
//goal: allow order items in grid structure.
//require: grid-layout as parent
//example: see grid layout example
//
//attributes:
//  col,row: index of columns and row (default - auto layout. find first top,left free area on grid)
//  col-span: width in grid cells (default 1)
//  row-span: height in grid cells (default 1)
app.directive('gridItem', function () {
    return {
        restrict: 'C',
        require: '^gridLayout',
        link: function (scope, element, attr, ctrl) {
            var item = {
                element: element,
                colSpan: scope.$eval(attr.colSpan),
                rowSpan: scope.$eval(attr.rowSpan),
                row: scope.$eval(attr.row),
                col: scope.$eval(attr.col)
            };

            ctrl.addChild(item);

            scope.$on('$destroy', function () {
                ctrl.removeChild(item);
            });
        }
    }
});

//right click directive.
//----------------------
//goal: allow bind to right click event
//example:
// <div right-click="handleEvent($event)"></div>
app.directive('rightClick', function ($parse) {
    return function (scope, element, attrs) {
        var fn = $parse(attrs.rightClick);
        element.bind('contextmenu', function (event) {
            scope.$apply(function () {
                event.preventDefault();
                event.stopPropagation();
                fn(scope, { $event: event });
            });
        });
    };
});

//delayed hover directive.
//----------------------
//goal: allow to set a class while mouse is over the DOM element to enable usable pop-up windows
//example:
//
// <div class="button" delayed-hover="my-class" delay=1000 delayed-leave="callback($event)">
//   <div class="popup"></div>
// </div>
//
// style sheet:
// .button .popup { display:none }
// .button.my-class .popup { display:block }
//
// the problem with .button:hover .popup { display:block } is that as soon as the mouse
// leaves the element the pop-up menu disappear.
// This directive delay the mouse leave event by requested delay.
// Therefore, the class doesn't disappear if the mouse leave and enter the
// DOM element before the delayed timer expires
//
// attributes:
//   delay: delay before remove the css class in milliseconds (default 500)
//   delayed-hover: class name to add to element (default 'hover')
//   delayed-leave: callback to invoke when class is been removed
app.directive('delayedHover', function ($parse, $timeout) {
    return {
        restrict: 'CA',
        link: function (scope, element, attrs) {
            var delay = parseInt(attrs.delay || "500");
            var cls = attrs.delayedHover || 'hover';

            var fn;
            if (attrs.delayedLeave) {
                fn = $parse(attrs.delayedLeave);
            }

            var leavePromise;
            var mouseEnter = function () {
                if (leavePromise) {
                    $timeout.cancel(leavePromise);
                }
                element.addClass(cls);
            }
            var mouseLeave = function (event) {
                leavePromise = $timeout(function () {
                    element.removeClass(cls);
                    if (fn) {
                        fn(scope, { $event: event });
                    }
                }, delay);
            }

            element.bind('mouseenter', mouseEnter);
            element.bind('mouseleave', mouseLeave);

            scope.$on('$destroy', function () {
                element.off('mouseenter', mouseEnter);
                element.off('mouseleave', mouseLeave);
            });
        }
    };
});

//bar chart directive.
//----------------------
//goal: draw bar chart using svg
//
//example: <bar-chart dataset="[0,1,2,3,4,5]" graph-width="200" graph-height="200"></bar-chart>
//
//attributes:
//   dataset : array of numbers
//   graph-width: width of the svg object in pixels
//   graph-height: height of the svg object in pixels
//   on-bar-enter: callback when mouse enter a bar element. $data,$index,$event are passed
//   on-bar-leave: callback when mouse leave a bar element. $data,$index,$event are passed
//   labels: array of strings (only for horizontal mode)
//   mode: 'horizontal' / 'vertical'
app.directive('barChart', function () {
    return {
        restrict: 'E',
        scope: {
            dataset: '=',
            labels: '=',
            width: '=graphWidth',
            height: '=graphHeight',
            onBarEnter: '&',
            onBarLeave: '&'
        },
        replace: true,
        template: '\
<svg ng-attr-width="{{svgWidth}}" ng-attr-height="{{svgHeight}}">\
    <g ng-if="horizontal">\
        <text ng-repeat="label in labels track by $index"\
            ng-attr-y="{{itemY(0,$index) + itemSize/2 + 5}}"\
            x="0"\
            ng-mouseenter="onEnter($index,$event)"\
            ng-mouseleave="onLeave($index,$event)"\
            ng-attr-font-size={{labelFont}}>\
            {{label|valueFormat|limitTo:5}} \
        </text>\
    </g>\
    <rect ng-repeat="data in dataset track by $index"\
          ng-attr-height="{{itemBarHeight(data)}}"\
        ng-attr-width="{{itemBarWidth(data)}}"\
        ng-attr-y="{{itemY(data,$index)}}"\
        ng-attr-x="{{itemX(data,$index)}}"\
        ng-mouseenter="onEnter($index,$event)"\
        ng-mouseleave="onLeave($index,$event)">\
    </rect>\
</svg>',
        link: function (scope, element, attr) {
            var updateGraphData = function () {
                var dataPoints = scope.dataset.length;
                scope.horizontal = attr.mode !== 'vertical';

                if (dataPoints === 0) {
                    scope.itemSize = 1;
                    scope.max = 1;
                } else {
                    scope.itemSize = 1.0 * (scope.horizontal ? scope.height : scope.width) / Math.max(dataPoints, 1);
                    scope.max = Math.max.apply(Math, scope.dataset);
                }

                if (scope.horizontal) {
                    scope.itemSize = Math.max(scope.itemSize, 20);
                    scope.itemSize = Math.min(scope.itemSize, 40);
                    scope.barSize = Math.max(1, scope.itemSize - 2);
                    scope.labelFont = Math.min(12, scope.barSize);

                    scope.svgWidth = scope.width;
                    scope.svgHeight = scope.itemSize * dataPoints;

                    scope.itemBarHeight = function () {
                        return scope.barSize;
                    }

                    scope.itemBarWidth = function (data) {
                        return data / scope.max * (scope.width - scope.itemX());
                    };

                    scope.itemX = function () {
                        return 40;
                    }

                    scope.itemY = function (data, index) {
                        return index * scope.itemSize;
                    }
                } else {
                    scope.svgWidth = scope.width;
                    scope.svgHeight = scope.height;

                    scope.barSize = Math.max(1, scope.itemSize - 2);

                    scope.itemBarHeight = function (data) {
                        return data / scope.max * scope.height;
                    };

                    scope.itemBarWidth = function () {
                        return scope.barSize;
                    };

                    scope.itemX = function (data, index) {
                        return index * scope.itemSize;
                    }

                    scope.itemY = function (data) {
                        return scope.height - scope.itemBarHeight(data);
                    }
                }
            }

            scope.onEnter = function (index, event) {
                if (scope.onBarEnter) {
                    scope.onBarEnter({ $data: scope.dataset[index], $index: index, $event: event });
                }
            }

            scope.onLeave = function (index, event) {
                if (scope.onBarLeave) {
                    scope.onBarLeave({ $data: scope.dataset[index], $index: index, $event: event });
                }
            }

            scope.$watchCollection('dataset', updateGraphData);
            scope.$watch('width', updateGraphData);
            scope.$watch('height', updateGraphData);
        }
    }
});

//background palette directive
//----------------------
//goal: set background style based on a palette object.
//require: palette object that have array of Steps. Step is {Color:,Value:}
//example:
// <span background-palette="{Steps:[{Color:'#f00',Value:0},{Color:'#fff',Value:1}]}"></span>
//
// optional attributes:
//   direction: left,right,bottom,top
//              (default: left)
//   value-transform: function($value,$index,$count) - get called when the palette need to convert a step.Value into number between 0...1
//              (default: return $index/($count-1) - equally spread the steps
//
app.directive('backgroundPalette', function ($parse) {
    return {
        restrict: 'A',
        link: function (scope, element, attr) {
            var direction = attr.direction || "left";

            var valuePositionFunction = function (value, index, count) {
                return index / (count - 1);
            }

            if (attr.valueTransform) {
                var fn = $parse(attr.valueTransform);

                valuePositionFunction = function (value, index, count) {
                    return fn(scope, { $value: value, $index: index, $count: count });
                }
            }

            var updateBackgound = function (palette) {
                if (!palette) {
                    return;
                }
                var steps = [];
                var count = palette.Steps.length;

                var background = undefined;
                if (count > 1) {
                    angular.forEach(palette.Steps, function (step, index) {
                        steps.push(step.Color + " " + (valuePositionFunction(step.Value, index, count) * 100) + "%");
                    });
                    background = "-webkit-linear-gradient(" + direction + ", " + steps.join(",") + ")";
                } else if (count === 1) {
                    background = palette.Steps[0].Color;
                }

                element.css("background", background);
            }

            scope.$watch(attr.backgroundPalette, function (newValue) {
                updateBackgound(newValue);
            });

            var value = scope.$eval(attr.backgroundPalette);
            updateBackgound(value);
        }
    };
});

//background image blend directive
//----------------------
//goal: awesomium doesn't support css background-blend-mode: 'multiply'
//      this directive implement the same using in memory canvas.

//example:
// <span background-image-blend="http://some.good.image/1.jpg" background-image-color="#f00"></span>
app.directive('backgroundImageBlend', function () {
    return {
        restrict: 'A',
        link: function (scope, element, attr) {

            function updateImage() {
                var imageUrl = scope.$eval(attr.backgroundImageBlend);
                var color = scope.$eval(attr.backgroundImageColor);
                if (color.Steps) {
                    color = color.Steps[Math.floor(color.Steps.length/2)].Color;
                }

                var canvas = document.createElement('canvas');

                var imageObj = new Image();
                imageObj.onload = function () {
                    canvas.width = imageObj.width;
                    canvas.height = imageObj.height;

                    var context = canvas.getContext('2d');

                    //convert color to rgba array
                    context.fillStyle = color;
                    context.fillRect(0, 0, 1, 1);
                    var colorData = context.getImageData(0, 0, 1, 1);


                    //draw image
                    context.drawImage(this, 0, 0);
                    var imageData = context.getImageData(0, 0, canvas.width, canvas.height);

                    //perform multiply on pixel data
                    for (var i = 0; i < imageData.data.length; i += 4) {
                        imageData.data[i] = imageData.data[i] * colorData.data[0] / 255;
                        imageData.data[i + 1] = imageData.data[i + 1] * colorData.data[1] / 255;
                        imageData.data[i + 2] = imageData.data[i + 2] * colorData.data[2] / 255;
                    }

                    //draw result pixel back to canvas
                    context.putImageData(imageData, 0, 0);

                    //convert into data url
                    var dataUrl = canvas.toDataURL();

                    //update the background-image for the element
                    element.css('background-image', 'url(' + dataUrl + ')');
                };

                imageObj.src = imageUrl;
            }

            updateImage();

            scope.$watch(attr.backgroundImageBlend, updateImage);
            scope.$watch(attr.backgroundImageColor, updateImage);
        }
    };
});

app.directive('focusOn', function () {
    return function (scope, elem, attr) {
        scope.$on(attr.focusOn, function () {
            elem[0].focus();
        });
    };
});

app.directive('autofocus', function () {
    return function (scope, elem, attr) {
        elem[0].focus();
    };
});

//stop propagation directive
//--------------------------
//this directive will prevent specific events from propagating (bubble up).
//
//usage:
//<div stop-propagation="keypress keyup keydown"></div>
app.directive('stopPropagation', function () {
    return function (scope, elem, attr) {
        var events = attr['stopPropagation'];
        var handler = function (event) {
            event.stopPropagation();
        }

        elem.bind(events, handler);
        scope.$on('$destroy', function () {
            elem.off(events, handler);
        });
    };
});

//loading progress icon directive
//-------------------------------
//this directive shows an animated png.
//this directive assumes the css file has loading-progress-icon defined.
//
//usage:
//<i class="loading-progress-icon"></i>
app.directive('loadingProgressIcon', function () {
    return {
        restrict: 'C',
        link: function (scope, elem) {
            var frame = 18;
            var alive = true;
            var spriteSheet = {
                'width': 6,
                'height': 6,
                'imgWidth': 14,
                'imgHeight': 14
            }

            spriteSheet.frameCount = spriteSheet.width * spriteSheet.height;

            var fps = 1000 / 30;

            var animate = function () {
                frame += 1;
                if (frame >= spriteSheet.frameCount) {
                    frame = 0;
                }

                var xpos = frame % spriteSheet.width;
                var ypos = (frame - xpos) / spriteSheet.width;

                elem.css({
                    'background-position': (-xpos * spriteSheet.imgWidth) + 'px ' + (-ypos * spriteSheet.imgHeight) + 'px'
                });

                if (alive) {
                    window.setTimeout(animate, fps);
                }
            }

            animate();

            scope.$on('$destroy', function () {
                alive = false;
            });
        }
    };
});

//map item status icon directive
//------------------------------
//this directive shows the map item status icon.
//note: this is a very studio custom made directive that can be replaced with css :hover and built in support of animated png.
//
//the icon behavior:
// 1) when item is not ready - loading progress icon is shown
// 2) when item is ready - the eye on/off show if the item is active or not
// 3) when the item is ready & loading - a loading progress icon is shown when mouse is not over the icon.
//    when mouse is over the icon - the eye on/off is shown.
//
//usage:
//<map-item-status-icon ready="isItemReady()" active="isItemActive()" loading="isItemLoading()"></map-item-status-icon>
app.directive('mapItemStatusIcon', function () {
    return {
        'restrict': 'E',
        'replace': true,
        'scope': {
            'active': '&',
            'ready': '&',
            'loading': '&'
        },
        'template': '\
<span ng-mouseenter="mouseEnter()" ng-mouseleave="mouseLeave()">\
    <i ng-if="showEye" class="eye-button {{activeState}}"></i>\
    <i ng-if="!showEye" class="loading-progress-icon"></i>\
</span>',
        'link': function (scope) {
            scope.mouseOver = false;

            function updateState() {
                scope.activeState = scope.active() ? 'on' : 'off';
                if (scope.ready()) {
                    //the GeoSource is ready - the eye is visible on mouse over and when it not loading.
                    scope.showEye = scope.mouseOver || !scope.loading();
                } else {
                    //the GeoSource is not ready - show progress icon
                    scope.showEye = false;
                }
            }

            scope.$watch('active()', updateState);
            scope.$watch('loading()', updateState);
            scope.$watch('ready()', updateState);

            scope.mouseEnter = function () {
                scope.mouseOver = true;
                updateState();
            }
            scope.mouseLeave = function () {
                scope.mouseOver = false;
                updateState();
            }

            updateState();
        }
    };
});


//scale font to match width directive
//----------------------
// goal: make sure the text of the element is scaled to include the text without "..."
//
// example:
//   <div style="overflow:hidden;width:100px;text-overflow:ellipsis" scale-font-to-match text="{{number|valueFormat}} of people"></div>
//
// attributes:
//   text: expression to set as text of the element
//   min-font-size: in pixels (default 10) (optional)
//   max-font-size: in pixels (default 24) (optional)
//
// the element text will be set to the text attribute.
// Moreover, every time the text value is changed the directive will start an animation to find the best font-size to match the entire width of the element.
// please note, this directive will require a style that support hidden overflow over width.
app.directive('scaleFontToMatch', function ($timeout) {
    return {
        restrict: 'CA',
        scope: {
            text: '@'
        },
        link: function (scope, elem, attrs) {
            var minFontSize = parseInt(attrs['minFontSize'] || '10');
            var maxFontSize = parseInt(attrs['maxFontSize'] || '24');
            var fontSize = parseInt(elem.css('font-size') || maxFontSize);

            scope.$watch('text', function (newValue) {
                elem.text(newValue);

                function updateWidth() {
                    var realWidth = elem.outerWidth();
                    var textWidth = elem[0].scrollWidth;

                    if (realWidth < textWidth && fontSize > minFontSize) {
                        fontSize--;
                        elem.css({ 'font-size': fontSize + 'px' });
                        $timeout(updateWidth, 10);
                    }
                    if (textWidth < realWidth * 0.9 && fontSize < maxFontSize) {
                        fontSize++;
                        elem.css({ 'font-size': fontSize + 'px' });
                        $timeout(updateWidth, 10);
                    }
                }

                $timeout(updateWidth, 10);
            });
        }
    };
});


//dashboard directive
//----------------------
// goal: a visualization of a dashboard widgets.
//
// example:
//   <dashboard widgets="array-of-widgets" selection="selection-object">
//
// attributes:
//   widgets: array of widgets model: { Type:,Inputs:,Settings }
//   selection: selection model: { 'Geometry':GeoJsonGeometry }
//   readonly: [optional] true,false.
//
// this directive will create a grid view of widgets.
// if dashboard is not readonly, it allow the user to remove the widget.
app.directive('dashboard', function ($pyx) {
    return {
        restrict: 'E',
        scope: {
            widgets: '=',
            selection: '=',
            readonly: '='
        },
        replace: true,
        template: '<div>\
    <div class="grid-layout" item-width="90" h-space="4" expand-rows="true">\
        <widget ng-repeat="widget in widgets" model="widget"></widget>\
    </div>\
</div>',
        controller: function ($scope) {
            var ctrl = this;
            ctrl.widgets = $scope.widgets;
            ctrl.selection = $scope.selection;
            ctrl.readonly = $scope.readonly;

            function getSelectionHash(selection) {
                return $pyx.obj.hash($pyx.obj.get(selection, 'Geometry'));
            }

            var hash = getSelectionHash(ctrl.selection);

            $scope.$watch('selection', function (value) {
                ctrl.selection = value;
                var newHash = getSelectionHash(ctrl.selection);

                if (hash !== newHash) {
                    hash = newHash;
                    $scope.$broadcast('pyx-dashboard-selection-changed');
                }
            });
        }
    }
});



//dashboardLegend directive
//----------------------
// goal: A prototype of the legend.
//
// example:
//   <dashboard widgets="array-of-widgets" selection="selection-object">
//
// attributes:
//   widgets: array of widgets model: { Type:,Inputs:,Settings }
//   selection: selection model: { 'Geometry':GeoJsonGeometry }
//   readonly: [optional] true,false.
//
// this directive will create a grid view of widgets.
// if dashboard is not readonly, it allow the user to remove the widget.
app.directive('dashboardLegend', function ($pyx) {
    return {
        restrict: 'E',
        scope: {
            widgets: '=',
            selection: '=',
            readonly: '='
        },
        replace: true,
        template: '<div>\
    <div class="grid-layout" item-width="90" h-space="4" expand-rows="true">\
        <widget ng-repeat="widget in widgets" model="widget"></widget>\
    </div>\
</div>',
        controller: function ($scope) {
            var ctrl = this;
            ctrl.widgets = $scope.widgets;
            ctrl.selection = $scope.selection;
            ctrl.readonly = $scope.readonly;

            function getSelectionHash(selection) {
                return $pyx.obj.hash($pyx.obj.get(selection, 'Geometry'));
            }

            var hash = getSelectionHash(ctrl.selection);

            $scope.$watch('selection', function (value) {
                ctrl.selection = value;
                var newHash = getSelectionHash(ctrl.selection);

                if (hash !== newHash) {
                    hash = newHash;
                    $scope.$broadcast('pyx-dashboard-selection-changed');
                }
            });
        }
    }
});



//widget directive
//----------------------
// goal: a dashboard widget that can calculate values based on the dashboard selection
//
// require: dashboard directive as parent
//
// example:
//   <widget model="widgte-model">
//
// attributes:
//   model: widgets model: { Type:,Inputs:,Settings }
//
// this directive create a grid-item that visualize the widget.
// this directive listen for selection change event to recalculate the value displayed
// by the widget.
app.directive('widget', function ($timeout, $pyx, $filter, mapFactory, geoSourceCache, worldViewStudioConfig) {
    return {
        restrict: 'E',
        scope: {
            model: '='
        },
        require: '^dashboard',
        replace: true,
        template: '\
<div class="grid-item" row-span="model.Settings.RowSpan" col-span="model.Settings.ColSpan">\
    <div ng-if="!readonly" class="gird-item-remove"><i class="fa fa-times-circle" ng-click="removeWidget()"></i></div>\
    <div class="title">{{model.Metadata.Name}}</div>\
    <div ng-if="widgetState.State==\'idle\'">\
        <div ng-if="model.Type != \'Hist\'" class="value" scale-font-to-match text="{{widgetState.Value|valueFormat}}"></div>\
        <div ng-if="model.Type == \'Hist\'" class="graph">\
            <scrollbar style="position:relative;width:170px;height:150px;overflow:hidden">\
                <bar-chart class="blue-graph" graph-width="170" graph-height="150" dataset="widgetState.Value" labels="widgetState.BinsName" on-bar-enter="setGraphTooltip($index,$event)" on-bar-leave="tooltip.show=false"></bar-chart>\
            </scrollbar>\
            <div ng-if="tooltip.show" ng-style="{top:tooltip.top+\'px\',left:tooltip.left+\'px\'}" style="position:fixed;z-index:100000">\
                <popup-window arrow="top center">\
                    <div class="graph-tooltip">\
                        <div class="graph-tooltip-name">{{tooltip.name|valueFormat}}</div>\
                        <div class="graph-tooltip-value" scale-font-to-match text="{{tooltip.value|valueFormat}}"></div>\
                    </div>\
                </popup-window>\
            </div>\
        </div>\
    </div>\
    <div ng-if="widgetState.State==\'calculating\'">\
        <div class="value"><i class="fa fa-refresh"></i></div>\
    </div>\
    <div ng-if="widgetState.State==\'error\'">\
        <div class="value"><i class="fa fa-frown-o"></i></div>\
    </div>\
</div>',
        link: function (scope, element, attr, dashboardCtrl) {
            scope.removeWidget = function () {
                var index = dashboardCtrl.widgets.indexOf(scope.model);
                if (index !== -1) {
                    dashboardCtrl.widgets.splice(index, 1);
                }
            }

            scope.readonly = dashboardCtrl.readonly;

            scope.widgetState = {
                State: 'idle',
                Value: ''
            }
            scope.tooltip = {
                show: false
            };

            scope.setGraphTooltip = function (index, event) {
                scope.tooltip = {
                    show: true,
                    top: event.clientY - 10,
                    left: event.clientX,
                    value: scope.widgetState.ValueName[index],
                    name: scope.widgetState.BinsName[index]
                };
            }

            var areaFormat = $filter('area');

            function updateWidget() {
                var widget = scope.model;
                var state = scope.widgetState;
                var geometry = $pyx.obj.get(dashboardCtrl, 'selection', 'Geometry');
                if (geometry) {
                    state.State = "calculating";
                    geoSourceCache.getWithSpecification(widget.Inputs.Resource.Id).then(function (geoSource) {
                        if (widget.Type === 'Area') {
                            $pyx.engine.getArea(geometry).success(function (area) {
                                $timeout(function () {
                                    state.State = "idle";
                                    state.Value = areaFormat(area);
                                });
                            }).error(function () {
                                $timeout(function () {
                                    state.State = "error";
                                });
                            });
                        } else if (widget.Type === 'Count' && 'FieldValue' in widget.Inputs) {
                            $pyx.engine.getFieldValueCountAt(geoSource, widget.Inputs.FieldName, widget.Inputs.FieldValue, geometry).success(function (statistics) {
                                $timeout(function () {
                                    state.State = "idle";
                                    state.Value = statistics.MinCount;
                                });
                            }).error(function () {
                                $timeout(function () {
                                    state.State = "error";
                                });
                            });
                        } else {
                            $pyx.engine.getFieldStatisticsAt(geoSource, widget.Inputs.FieldName, geometry, worldViewStudioConfig.whatIsHere.histogramBinCount).success(function (statistics) {
                                $timeout(function () {
                                    state.State = "idle";
                                    if (widget.Type === 'Count') {
                                        state.Value = statistics.MinCount;
                                    } else if (widget.Type === 'Average') {
                                        state.Value = statistics.Average;
                                    } else if (widget.Type === 'Min') {
                                        state.Value = statistics.Min;
                                    } else if (widget.Type === 'Max') {
                                        state.Value = statistics.Max;
                                    } else if (widget.Type === 'Sum') {
                                        state.Value = statistics.Sum;
                                    } else if (widget.Type === 'Hist') {
                                        var areaHistogram = geoSource.Specification && (geoSource.Specification.OutputType === 'Coverage');

                                        var values = [];
                                        angular.forEach(statistics.Distribution.Histogram, function (bin) {
                                            var tooltipValue = {
                                                Value: (bin.MinCount + bin.MaxCount) / 2,
                                            }
                                            if (bin.Min === bin.Max) {
                                                tooltipValue.Name = bin.Min;
                                            } else {
                                                tooltipValue.Name = bin.Min + " ... " + bin.Max;
                                            }
                                            values.push(tooltipValue);
                                        });

                                        if (values.length > 6 && !areaHistogram) {
                                            values.sort(function (a, b) { return b.Value - a.Value; });
                                        }

                                        state.Value = [];
                                        state.BinsName = [];
                                        state.ValueName = [];

                                        angular.forEach(values, function (bin) {
                                            state.Value.push(bin.Value);
                                            state.BinsName.push(bin.Name);
                                            if (areaHistogram) {
                                                state.ValueName.push(areaFormat(bin.Value));
                                            } else {
                                                state.ValueName.push(bin.Value);
                                            }
                                        });
                                    }
                                });
                            }).error(function () {
                                $timeout(function () {
                                    state.State = "error";
                                });
                            });
                        }
                    });
                } else {
                    state.State = "idle";
                    state.Value = "";
                }
            }

            scope.$on('pyx-dashboard-selection-changed', function () {
                updateWidget();
            });

            updateWidget();
        }
    }
});

app.directive("colorPicker", function (styleOptions) {
    return {
        restrict: "E",
        scope: {
            selectedColor: "=color",
            onColorPicked: "&"
        },
        template: '<div class="color-picker" ng-style="{\'background-color\':selectedColor}" ng-class="{\'is-transparent\': selectedColor == \'rgba(0,0,0,0.0)\' || selectedColor == \'rgba(0,0,0,0.00)\'}" popup-template="/client/templates/studio/popup-menu/color-picker.html" placement="left center" popup-group="popup-menu" show-trigger="click"></div>',
        link: function (scope, element, attrs) {
            scope.colors = styleOptions.colors;

            scope.selectColor = function (color) {
                if (scope.selectedColor !== color) {
                    var oldColor = scope.selectedColor;
                    scope.selectedColor = color;
                    scope.onColorPicked({ $color: color, $oldColor: oldColor });
                }
            }
        }
    };
});

/*
- @name gradientDesigner
- @desc Master directive for the all things Gradient
- Desginer is the umbrella category for anything related
- to the Editor
*/
app.directive("gradientDesigner", function ($timeout, $filter, $pyxIntercom, numberUtils, paletteService, delayEvent) {
  /*
  - @name gradientDesignerCtrl
  - @desc Gradient Designer Controller
  - @type {function}
  */
    function gradientDesignerCtrl($scope) {
        $scope.getNumStep = function () {
            return $scope.colorPalette.Steps.length;
        }

        $scope.getRange = function () {
            var minMax = {};
            var steps = paletteService.verify($scope.colorPalette).Steps;
            var amount = $scope.getNumStep() - 1;

            minMax.Min = steps[0].Value;
            minMax.Max = steps[amount].Value;

            return minMax;
        }

        $scope.rangeValue = $scope.getRange();
        $scope.editorMode = 'collapsed';
    }

  /*
   - @name gradientDesignerLink
   - @desc Gradient Designer Link
   - @type {function}
   */
    function postGradientDesignerLink(scope, element, attrs, ctrl) {
        var elDisplayBar = element.find('.gradient');
        var elGradientEditor = element.find('.gradient-editor');
        var elGradientStops = element.find('.gradient-stops');
        var colorGlobeTime = 1000;


        var gradientRange = {
                Min: scope.rangeValue.Min,
                Max: scope.rangeValue.Max
        }

        scope.buildRange = function (value) {
            if (value < gradientRange.Min) {
                gradientRange.Min = value;
            } else if (value > gradientRange.Max) {
                gradientRange.Max = value;
            }

            return gradientRange;
        }

        scope.normalized = numberUtils.range(0, 1);

        scope.colorRange = function () {
            return numberUtils.range(gradientRange.Min, gradientRange.Max);
        }

        scope.pixelRange = function (range) {
            var minPixelValue = 0;
            var maxPixelValue = elGradientStops.height();

            return numberUtils.range(minPixelValue, maxPixelValue);
        }

        scope.applyToGlobe = function () {
            scope.styleModifier.setItemPalette(scope.item, scope.colorPalette);
        }

        scope.eventContext = function (e, elementClass) {
            var userEvent = false;
            if (e.target) {
                if (e.target.className === elementClass) {
                    userEvent = true;
                }
            }

            return userEvent;
        }

        scope.getStepIndex = function (outerStep) {
            var stepIndex = undefined;

            for (var index = 0, steps = scope.colorPalette.Steps.length; index < steps; index++) {
                var step = scope.colorPalette.Steps[index];
                if (angular.equals(step, outerStep)) {
                    stepIndex = index;
                    break;
                }

            }

            return stepIndex;
        }

        scope.deleteStep = function (step) {
            var delayApply;

            if (scope.colorPalette.Steps.length <= 2) {
                return;
            }

            var removeIndex = scope.getStepIndex(step);
            scope.colorPalette.Steps.splice(removeIndex, 1);
            delayApply = delayEvent(scope.applyToGlobe, colorGlobeTime / 2, false);
            delayApply();

        }

        scope.appendStep = function () {
            var newValue, stepIndex, newStep = {}, delayApply;

            if (scope.eventContext(event, 'gradient')) {
                newValue = numberUtils.transform(scope.pixelRange(), scope.colorRange()).map(event.offsetY);
            }

            newStep.Value = +newValue.toFixed(2);
            newStep.Color = '#000000';

            scope.colorPalette.Steps.push(newStep);

            scope.sequenceIt();

            stepIndex = scope.getStepIndex(newStep);

            scope.colorPalette.Steps[stepIndex].Color = scope.colorPalette.Steps[stepIndex + 1].Color;
            delayApply = delayEvent(scope.applyToGlobe, colorGlobeTime / 2, false);
            delayApply();
        }

        var delayApplyUpdate = delayEvent(scope.applyToGlobe, colorGlobeTime, false);

        scope.updateStep = function (oldValue, step, transform) {
            $timeout(function () {
                var newValue = numberUtils.transform(scope.pixelRange(), scope.colorRange()).map(oldValue);
                var stepIndex = scope.getStepIndex(step);
                var stepToUpdate = scope.colorPalette.Steps[stepIndex];

                if (transform) {
                    newValue = parseFloat(oldValue);
                }

                if (stepToUpdate) {
                    stepToUpdate.Value = +newValue.toFixed(2);
                }

                scope.sequenceIt();
                delayApplyUpdate();
            },0);
        }

        scope.sequenceIt = function () {
            var orderBy = $filter('orderBy');
            return scope.colorPalette.Steps = orderBy(scope.colorPalette.Steps, 'Value');
        }

        scope.drawColors = function (palette) {
            if (!palette) {
                return;
            }

            var gradientColors = paletteService.toCSS(palette, scope.colorRange(), scope.normalized);
            var background = "-webkit-linear-gradient(top, " + gradientColors + ")";

            elDisplayBar.css('background', background);
        }

        scope.$watch('colorPalette.Steps', function (newPalette) {
            scope.drawColors(newPalette);
        }, true);

        //when the style changes to a solid color - remove the Gradient Editor
        scope.$on('item-style-changed', function (event, item) {
            if (scope.styleModifier.isSameGeoSource(item.item, scope.item)) {
                var canEdit = scope.styleModifier.hasPaletteStyle(item.item);
                scope.editInterface.editable = scope.editInterface.editing = canEdit;
            }
        });

        var setupEditor = function () {
            var onEditorComplete = function () {
                scope.editorMode = 'expanded';
                element.removeClass('mode-legend').addClass('mode-editor');

                scope.$apply();
            }

            var animation = new TimelineMax({ onComplete: onEditorComplete });
            animation.to(elGradientEditor, 0.2, { height: 464, ease: Power1.easeIn });

        }

        $timeout(setupEditor, 100);

    }
    return {
        restrict: 'E',
        scope: {
            colorPalette: '=',
            item: '=designerItem',
            styleModifier: '=designerModifier',
            editInterface: '=?'
        },
        templateUrl: '/client/templates/studio/template/gradient-designer.html',
        controller: gradientDesignerCtrl,
        link: {
            post: postGradientDesignerLink
        }
    }
});

app.directive('gradientStop', function ($document, $timeout, numberUtils) {
   /*
    - @name gradientStopLink
    - @desc Gradient Stop Link
    - @type {function}
    */
    function gradientStopLink(scope, element, attrs) {
        // Because ng-repeat creates an isolated scope
        // we need to reference the $parent scope
        // explicitly
        var parent = scope.$parent;
        var elGradientStops = angular.element('.gradient-stops');
        var elGradientStopsHeight = elGradientStops.height();

        scope.stepValue = {};

        var makeDraggable = function () {
            var startY = 0;
            var elStopHandle = element.find('.gradient-stop-handle');
            
            var valueY = element.position().top || 0;
            
            function startDrag(e) {
                if (e.target.nodeName === 'SPAN') {
                    return;
                }

                startY = e.clientY - element[0].offsetTop;
                $document.on('mousemove.draggable', mousemove);
                $document.on('mouseup.draggable', mouseup);
            }

            function mousemove(e) {
                e.stopPropagation();
                valueY = e.clientY - startY;
                checkInBounds();
                parent.updateStep(valueY, scope.step);
                scope.stepValue.copy = scope.step.Value;
            }

            function mouseup(e) {
                $document.off('mousemove.draggable', mousemove);
                $document.off('mouseup.draggable', mouseup);
                parent.updateStep(valueY, scope.step);
                scope.stepValue.copy = scope.step.Value;
            }

            function checkInBounds() {
                var boundsMin = 0;
                var boundsMax = elGradientStopsHeight;

                if (!elGradientStops) {
                    return;
                }

                if (valueY < boundsMin) {
                    valueY = boundsMin;
                } else if (valueY > boundsMax) {
                    valueY = boundsMax;
                }

                scope.updateStepPosition(valueY);
            }

            elStopHandle.on('mousedown.draggable', startDrag);
        }

        scope.stepValue.copy = Number(scope.step.Value).toFixed(2);

        scope.updateStepValue = function (value) {
            //set to current value when the new value
            //is not a number
            if (!numberUtils.isNumber(value)) {
                scope.stepValue.copy = scope.step.Value;
                return;
            }

            //remap the range when the value is
            //outside of current range min/max
            if (!parent.colorRange().contains(value)) {
                parent.buildRange(value);
            }

            scope.step.Value = value;
            scope.updateStepPosition(value, true);
            parent.updateStep(value, scope.step, true);

        }

        scope.updateStepPosition = function (value, mapRange) {
            if (mapRange) {
                value = numberUtils.transform(parent.colorRange(), parent.pixelRange()).map(value);
            }

            element.css('top', value);
        }

        var setupSteps = function () {
            element.addClass('is-visible');
            scope.updateStepPosition(scope.step.Value, true);
            makeDraggable();
        }

        $timeout(setupSteps, 100);

    }

    return {
        restrict: 'A',
        link: gradientStopLink
    }
});

app.directive("searchBoxTag", function ($timeout, $pyxIntercom, dispatcher) {
    return {
        restrict: "E",
        scope: {
            'tag': "="
        },
        replace: true,
        template: ['<div class="search-box-tag" pyx-track="selection-tag-select" ',
            'popup-template="/client/templates/studio/popup-menu/search-tag-menu.html" popup-group="popup-menu" placement="bottom center" show-trigger="click" append-to-body="true" hide-delay="1000">',
            '<span class="remove-tag"><i class="close-button" ng-click="removeSearchTag()"></i></span>',
            '<span class="tag-icon {{icon}}"></span>',
            '<span>{{tag.name}}</span>',
            '</div>'].join(""),
        link: function (scope, element, attrs) {
            var iconsMap = {
                'Condition': 'tag-condition',
                'Polygon': 'tag-polygon',
                'FeatureRef': 'tag-feature',

                'polygon': 'tag-polygon',
                'freehand': 'tag-freehand',
                'watershed': 'tag-watershed',
                'magic-wand': 'tag-feature'
            };

            scope.icon = iconsMap[scope.tag.geometry.createdByTool] || iconsMap[scope.tag.geometry.type] || "tag-selection";

            if (scope.tag.geometry.type === "Condition") {
                scope.range = angular.copy(scope.tag.geometry.range);
                if (scope.range) {
                    scope.numericRange = angular.isNumber(scope.range.Min);
                }
            }

            scope.removeSearchTag = function () {
                var tags = dispatcher.stores.searchTagsStore.get();

                var tagIndex = tags.indexOf(scope.tag);

                if (tagIndex !== -1) {
                    if (tagIndex === 0) {
                        dispatcher.actions.changeSelection.invoke({ selection: undefined });
                    } else {
                        var modifiedSelection = angular.copy(dispatcher.stores.selectionStore.get().currentSelection);

                        if (modifiedSelection.Geometry && modifiedSelection.Geometry.type === "Boolean") {
                            modifiedSelection.Geometry.operations.splice(tagIndex, 1);
                            dispatcher.actions.changeSelection.invoke({ selection: modifiedSelection });
                        }
                    }
                }
            }

            var triggerChangePromise = undefined;

            scope.triggerRangeChange = function () {
                if (triggerChangePromise) {
                    $timeout.cancel(triggerChangePromise);
                }
                triggerChangePromise = $timeout(scope.updateRange, 1500);
            }

            scope.updateRange = function () {
                $pyxIntercom.track('edit-selection-using-tags');

                var tags = dispatcher.stores.searchTagsStore.get();

                var tagIndex = tags.indexOf(scope.tag);

                if (tagIndex !== -1) {
                    var modifiedSelection = angular.copy(dispatcher.stores.selectionStore.get().currentSelection);

                    if (modifiedSelection.Geometry && modifiedSelection.Geometry.type === "Boolean") {

                        if (scope.numericRange) {
                            scope.range.Min = parseFloat(scope.range.Min);
                            scope.range.Max = parseFloat(scope.range.Max);
                        }

                        modifiedSelection.Geometry.operations[tagIndex].geometry.range = scope.range;
                        dispatcher.actions.changeSelection.invoke({ selection: modifiedSelection });
                    }
                }
            }
        }
    }
});



app.directive('timeLine', function ($timeout) {
    return {
        restrict: "E",
        scope: {
            'timeLine': "=",
            'sections': "=",
            'state': "=",
            'onSelectionClick': "&",
            'onTimeLineClick': "&",
            'onCurrentTimeMousedown': "&",
            'onCurrentTimeMousemove': "&",
            'onCurrentTimeMouseup': "&"
        },
        replace: true,
        template: [
            '<div class="timeline">',
            '<div ng-repeat="section in sections" class="section" ng-class="{active: currentSection == section }" ng-style="{left: timeToPercent(section.start), width: timeToPercent(section.duration)}" ng-click="sectionClick(section,$event)"><span class="section-name">{{section.name}}</span><div class="section-bar"></div></div>',
            '<div class="background" ng-click="timelineClick(null,$event)"><div class="current-time-fill" ng-style="{width: timeToPercent(state.currentTime)}"></div></div>',
            '<span ng-repeat="step in timeLine" class="step step-{{$index}}" ng-style="{left: stepPosition($index)}" ng-click="timelineClick(step,$event)"></span>',
            '<span class="current-time" ng-style="{left: timeToPercent(state.currentTime)}" ng-mousedown="currentTimeMousedown($event)" ng-mousemove="currentTimeMousemove($event)" ng-mouseup="currentTimeMouseup($event)"></span>',
            '</div>'
        ].join(""),
        link: function (scope, element, attrs) {
            console.log("bootstrap timeline");

            scope.stepStartTime = [];
            scope.endTime = 0;
            angular.forEach(scope.timeLine, function (step) {
                scope.stepStartTime.push(scope.endTime);
                scope.endTime += step.duration;
            });

            scope.stepPosition = function (index) {
                return scope.timeToPercent(scope.stepStartTime[index]);
            }

            scope.timeToPercent = function (time) {
                return (100.0 * time / scope.endTime) + "%";
            }

            function updateActiveSection() {
                angular.forEach(scope.sections, function (section) {
                    if (scope.state.currentTime >= section.start && scope.state.currentTime < section.start + section.duration) {
                        scope.currentSection = section;
                    }
                });
            }

            // function handleState (state, prevState) {
            //     if (!state) {
            //         TweenMax.pauseAll();
            //     } else {
            //         TweenMax.resumeAll();
            //     }
            // };

            updateActiveSection();

            scope.$watch('state.currentTime', updateActiveSection);
            //scope.$watch('state.play', handleState);

            scope.getTimeFromEvent = function (event) {
                var offset = element.offset();
                var left = event.pageX - offset.left;

                var time = (scope.endTime * 1.0 * left) / element.width();
                if (time < 0) {
                    time = 0;
                }
                if (time > scope.endTime) {
                    time = scope.endTime;
                }

                return time;
            }

            scope.sectionClick = function (section, $event) {
                scope.onTimeLineClick({ section: section, $event: event, $time: scope.getTimeFromEvent(event) });
            }

            scope.timelineClick = function (step, event) {
                var time = scope.getTimeFromEvent(event);
                if (!step) {
                    angular.forEach(scope.timeLine, function (aStep, index) {
                        if (time >= scope.stepStartTime[index]) {
                            step = aStep;
                        }
                    });
                }
                scope.onTimeLineClick({ $step: step, $event: event, $time: time });
            }

            scope.currentTimeMousedown = function (event) {
                scope.onCurrentTimeMousedown({ $event: event, $time: scope.getTimeFromEvent(event) });
            }
            scope.currentTimeMousemove = function (event) {
                scope.onCurrentTimeMousemove({ $event: event, $time: scope.getTimeFromEvent(event) });
            }
            scope.currentTimeMouseup = function (event) {
                scope.onCurrentTimeMouseup({ $event: event, $time: scope.getTimeFromEvent(event) });
            }
        }
    }
});

/*
- @name pyxTrack
- @desc directive to intercept and track events using $pyxIntercom.
- pyx-track: name assigned to the tracked event (for tracking click event) or object of {event: 'tracked-name'} pairs
- pyx-track-metadata: metadata object attached to the event
- pyx-track-suffix: interpolated event suffix added to all assigned names
- Examples: <div ng-click="function()" pyx-track="element-click" pyx-track-metadata='{"type": "{{item.type}}"}'></div>
-           <div ng-click="function()" pyx-track="select-item-{{item.Name}}"></div>
-           <div ng-click="function1()" ng-dblclick="function2()" pyx-track='{"click": "element-click","dblclick": "element-double-click"}' pyx-track-suffix="{{context}}"></div>
*/
app.directive("pyxTrack", function ($pyxIntercom) {
    // Used to function scope the eventName which changes in the event loop
    function createCallback (eventName, metadata) {
        return function() {
            $pyxIntercom.track(eventName, metadata);
        }
    }

    return {
        priority: 1,
        restrict: "A",
        link: function (scope, element, attrs) {
            var trackCode = attrs.pyxTrack;
            if (!trackCode || !trackCode.length) {
                return;
            }
            var trackMetadata = attrs.pyxTrackMetadata;
            var trackSuffix = attrs.pyxTrackSuffix || '';
            var trackObject;
            try {
                trackObject = JSON.parse(trackCode);
            } catch (error) {
                trackObject = {
                    click: trackCode
                };
            }
            if (trackMetadata) {
                try {
                    trackMetadata = JSON.parse(trackMetadata);
                } catch (error) {
                    console.log('Invalid pyx-track-metadata: ' + trackMetadata);
                    trackMetadata = undefined;
                }
            }
            for (event in trackObject) {
                if (trackObject.hasOwnProperty(event)) {
                    var trackName = trackObject[event] + trackSuffix;
                    element.bind(event, createCallback(trackName, trackMetadata));
                }
            }
        }
    };
});