
app.controller('worldViewStudioController', function ($rootScope, $scope, $pyx, $timeout, $http, $pyxconfig) {

    var lastPalette = 0;

    $scope.palettes = [
   {
       Steps: [
         { Value: 0.0, Color: "#ff1c8c" },
         { Value: 0.5, Color: "#fffde4" },
         { Value: 1.0, Color: "#1a82f6" }
       ]
   },
   {
       Steps: [
         { Value: 0.0, Color: "#000" },
         { Value: 0.5, Color: "#f00" },
         { Value: 1.0, Color: "#ff0" }
       ]
   },
   {
       Steps: [
         { Value: 0.0, Color: "#330083" },
         { Value: 0.2, Color: "#9b0093" },
         { Value: 0.5, Color: "#ec0929" },
         { Value: 0.75, Color: "#f35633" },
         { Value: 1.0, Color: "#fffd4d" }
       ]
   },
   {
       Steps: [
         { Value: 0.0, Color: "#004072" },
         { Value: 0.25, Color: "#0ca6ff" },
         { Value: 0.5, Color: "#a6e1ff" },
         { Value: 0.75, Color: "#ffec6d" },
         { Value: 1.0, Color: "#fff000" }
       ]
   },
   {
       Steps: [
         { Value: 0.0, Color: "#3c0060" },
         { Value: 0.25, Color: "#3c0060" },
         { Value: 0.25, Color: "#9d01dc" },
         { Value: 0.75, Color: "#9d01dc" },
         { Value: 0.75, Color: "#00feb6" },
         { Value: 1.0, Color: "#00feb6" }
       ]
   },
   {
       Steps: [
       { Value: 0.0, Color: "#000" },
       { Value: 1.0, Color: "#fff" }
       ]
   }
    ];




    $pyx.globe.ready(function () {
        $pyx.gallery.geoSources().getById('6143a8b2-2c9a-4f2c-9da9-78414fc0a5b1').success(function (geoSource) {
            $scope.communityGeoSource = geoSource;

            $pyx.globe.getVisibleId($scope.communityGeoSource).success(function (id) {
                if (!id) {
                    $scope.show($scope.communityGeoSource);
                    $pyx.globe.gotoGeoSource($scope.communityGeoSource, 2000);
                } else {
                    $scope.communityGeoSource.visibleId = id;
                    $scope.communityGeoSource.ui = { 'state': 'ready' }
                    $scope.active.push($scope.communityGeoSource);
                    $scope.getInfo($scope.communityGeoSource);
                    $pyx.globe.gotoGeoSource($scope.communityGeoSource, 2000);
                }
            });

            $pyx.engine.ready(function () {
                $pyx.engine.getAllFeatures(geoSource).success(function (communities) {
                    $scope.communities = communities;
                    $scope.searchIndex = {};

                    angular.forEach(communities.features, function (community) {
                        angular.forEach(["name"], function (field) {
                            var value = community.properties[field];
                            if (value) {
                                value = value.toLowerCase();
                                if (!(value in $scope.searchIndex)) {
                                    $scope.searchIndex[value] = [community];
                                } else {
                                    $scope.searchIndex[value].push(community);
                                }
                            }
                        });
                    });
                });
            });
        });

        $pyx.gallery.geoSources().getById('5b901b73-fd2b-41f2-be8f-6dbcd594f323').success(function (geoSource) {
            $scope.bing = geoSource;

            $pyx.globe.getVisibleId($scope.bing).success(function (id) {
                if (!id) {
                    $pyx.globe.show($scope.bing);
                }
            });
        });
    });

    $scope.active = [];

    $scope.searchChanged = function () {
        $scope.suggestions = [];
        var words = $scope.search.toLowerCase().split(" ");
        if (words.length) {
            angular.forEach($scope.searchIndex, function (value, key) {
                if ($scope.suggestions.length > 10) return;
                var allFound = true;
                angular.forEach(words, function (word) { if (key.search(word) == -1) { allFound = false; } });
                if (allFound) {
                    $scope.suggestions.push({ text: key, values: value });
                }
            });
        }
    }

    $scope.selectOption = function (option) {
        $scope.selected = option.values;
        $scope.gotoCommunity(option.values[0])
        $scope.selectedIndex = 0;
        $scope.search = "";
        $scope.suggestions = [];

    }

    $scope.gotoCommunity = function (community) {
        $pyx.globe.gotoGeometry($pyx.area.featureRef($scope.communityGeoSource, community.id), 2000);
    }


    $scope.getHistogram = function (geoSource, geometry) {
        var fieldName = geoSource.ui.field;
        $pyx.engine.getFieldStatisticsAt(geoSource, fieldName, geometry).success(function (stats) {
            $timeout(function () {
                if (fieldName != geoSource.ui.field) return;
                geoSource.ui.statistics = stats;
            });
        });
    }

    $scope.getInfo = function (geoSource) {
        $pyx.engine.getSpecification(geoSource).success(function (specification) {
            $timeout(function () {
                geoSource.Metadata.Definition = specification;
            });
        });
        $pyx.globe.getStyle(geoSource.visibleId).success(function (style) {
            $timeout(function () {
                if (!geoSource.ui) {
                    geoSource.ui = {};
                }
                geoSource.Metadata.Style = style;
                if (style.Fill && style.Fill.Palette) {
                    geoSource.ui.palette = style.Fill.Palette;
                    geoSource.ui.field = style.Fill.PaletteExpression;
                } else if (style.Icon && style.Icon.Palette) {
                    geoSource.ui.palette = style.Icon.Palette;
                    geoSource.ui.field = style.Icon.PaletteExpression;
                }
            });
        });
    }

    $scope.show = function (item) {
        item.css = {
            top: (100 + 40 * $scope.active.length) + 'px',
            left: (40 + 40 * $scope.active.length) + 'px'
        };
        item.ui = { state: 'loading' };
        $scope.active.push(item);
        $pyx.globe.show(item).success(function (id) {
            $timeout(function () {
                item.visibleId = id;
                item.ui.state = 'ready';
                $scope.getInfo(item);
            });
        });
    }

    $scope.hide = function (item, index) {
        if (item.visibleId) {
            // $scope.active.splice(index, 1);
            $pyx.globe.hide(item.visibleId);
        }
    }

    $scope.setStyleByField = function (geoSource, fieldIndex) {
        var fieldName = geoSource.Metadata.Definition.Fields[fieldIndex].Name;
        var palette = $scope.palettes[lastPalette];
        $pyx.globe.setStyleByFieldWithPalette(geoSource.visibleId, fieldName, palette).success(function (style) {
            $timeout(function () {
                geoSource.ui.palette = palette;
                geoSource.ui.field = fieldName;
                geoSource.Metadata.Style = style;
                geoSource.ui.statistics = undefined;
                if ($scope.selectedArea && !geoSource.ui.selected) {
                    $scope.getHistogram(geoSource, $scope.selectedArea);
                }
            });
        });
        $scope.corField2 = $scope.corField1;
        $scope.corField1 = fieldName;
        if (!$scope.close) {
            $scope.getCorelation();
        }
    }
    $scope.close = true;

    $scope.corField1 = "married Percent";
    $scope.corField2 = "divorced Percent";

    $scope.getCorelation = function () {

        $pyx.engine.ready(function () {
            $pyx.engine.getAllFeatures($scope.communityGeoSource).success(function (communities) {
                $timeout(function () {
                    $scope.communities = communities;

                    var dataTable = new google.visualization.DataTable();
                    dataTable.addColumn('number', $scope.corField1);
                    dataTable.addColumn('number', $scope.corField2);
                    // A column for custom tooltip content
                    dataTable.addColumn({ type: 'string', role: 'tooltip' });
                    
                    data = [[$scope.corField1, $scope.corField2]];

                    angular.forEach(communities.features, function (community) {
                        var value1 = community.properties[$scope.corField1];
                        var value2 = community.properties[$scope.corField2];
                        if (value1 && value2) {
                            dataTable.addRow([value1, value2, community.properties["name"]]);
                        }
                    });

                    var options = {
                        title: $scope.corField1 + ' vs. ' + $scope.corField2,
                        hAxis: { title: $scope.corField1, },
                        vAxis: { title: $scope.corField2, },
                        tooltip: { trigger: 'selection' },
                        legend: 'none',
                        width:500,
                        height:500
                    };

                    var chart = new google.visualization.ScatterChart(document.getElementById('chart_div'));
                   
                    chart.draw(dataTable, options);
                    $scope.close = false;
                });
            });
        });
    }
});