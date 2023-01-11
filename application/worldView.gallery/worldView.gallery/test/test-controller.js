app.controller('testController', function ($scope, $timeout, $pyx, styleOptions) {
    //init funcunit
    F.attach(jasmine);

    //connect $pyx to FS

    var requests = {
        style: {},
        geoSource: {}
    };

    /**
     * fetchDataByName - fetch data from the globe
     *
     * @param type - 'style' or 'geoSource'
     * @param name - name of the GeoSource to fetch
     */
    var fetchDataByName = function (type, name) {
        if (name in requests[type]) {
            if (requests[type][name].style) {
                var dataFound = requests[type][name];
                if (type === 'style') {
                    dataFound = dataFound.style;
                }

                //delete request object - so next time would create a new request
                delete requests[type][name];

                return dataFound;
            } 
                
            return undefined;            
        }

        //mark that we requesting 
        requests[type][name] = { status: 'requesting' };

        $pyx.globe.getGeoSources().success(function (geoSources) {
            var found = false;
            angular.forEach(geoSources, function (geoSource) {
                if (geoSource.Metadata.Name.toLowerCase().indexOf(name.toLowerCase()) !== -1) {
                    found = true;
                    if (type === 'geoSource') {
                        requests[type][name] = geoSource;
                    } else if (type === 'style') {
                        $pyx.globe.getVisibleId(geoSource).success(function(id) {
                            $pyx.globe.getStyle(id).success(function(style) {
                                //request completed
                                requests[type][name].style = style;
                            }).error(function() {
                                //request failed
                                delete requests[type][name];
                            });
                        }).error(function() {
                            //request failed
                            delete requests[type][name];
                        });
                    }
                }
            });
            if (!found) {
                //request failed
                delete requests[type][name];
            }
        }).error(function () {
            //request failed
            delete requests[type][name];
        });

        return undefined;
    };

    var fetchGeoSourceStyleByName = function (name) {
        return fetchDataByName('style', name);
    }

    var fetchGeoSourceByName = function (name) {
        return fetchDataByName('geoSource', name);
    }

    FS.globe = {
        click: function (index, event) {
            return F.wait(
                10,
                function () {
                    $scope.handleGlobeClick(index,
                        JSON.stringify(
                            event || {
                                clientX: 400,
                                clientY: 400
                            }));
                }
            );

        },
        rightClick: function (index,event) {
            return F.wait(
                10,
                function () {
                    $scope.handleGlobeRightClick(index,
                        JSON.stringify(
                            event || {
                                clientX: 400,
                                clientY: 400
                            }));
                }
            );            
        },
        gotoCamera: function (camera, duration) {
            F.wait(0, function() {
                $pyx.globe.setCamera(camera, duration);
            });            
        },
        sameColor: function (a, b) {
            a = styleOptions.toRgba(a);
            b = styleOptions.toRgba(b);
            return angular.equals(a, b);
        },
        expectGeoSourceStyle: function (name, style) {
            var compareStyle = angular.isFunction(style) ?
                style :
                function (compareWith) {
                    return angular.equals(compareWith, style);
                };

            return F('body').wait(
                function () { //checker function
                    var styleFound = fetchGeoSourceStyleByName(name);
                    return styleFound && compareStyle(styleFound);
                },
                5000, //timeout
                function () {}, //success
                "geoSource " + name + " didn't have the expected style "); //message
        },
        getGeoSource: function (name, callback) {
            var geoSource;
            return F('body').wait(
                function () { //checker function
                    geoSource = fetchGeoSourceByName(name);
                    return geoSource;
                },
                5000, //timeout
                function() {
                    callback(geoSource);
                }, //success
                "geoSource " + name + " was not found "); //message
        }
    }

    FS.complex.importFile = function () {
        var files = [].slice.call(arguments);
        return F.wait(10, function () {
            $scope.startImportFiles(files);
        });
    }


    //init jasmine
    var jasmineEnv = jasmine.getEnv();
    jasmineEnv.updateInterval = 1000;   

    //load all tests suites
    $scope.tests = window.tests;

    var defaultTest = [
        "describe(\"custom test\", function () {",
        "",
        "    it(\"create new map and load gtopo30\", function () {",
        "        FS.complex.createNewMap();",
        "        FS.complex.importGeoSource(\"GTOPO30\");",
        "    });",
        "",
        "    it(\"remove map\", function () {",
        "        FS.complex.removeCurrentMap();",
        "    });",
        "",
        "});"
    ].join("\n");

    $scope.tester = {
        expanded: false,
        customMode: false,
        customTest: ""
    };

    //run a specific test   
    $scope.tester.run = function (test) {

        //clear old reporters
        $(".jasmine_reporter").remove();

        //kind of a hack to clear jasmineEnv status
        jasmineEnv.reporter.subReporters_ = [];
        jasmineEnv.currentRunner_.suites_ = [];

        //create html reporter
        var htmlReporter = new jasmine.HtmlReporter();

        jasmineEnv.addReporter(htmlReporter);

        jasmineEnv.specFilter = function (spec) {
            return htmlReporter.specFilter(spec);
        };

        //register test
        test();
        
        //run tests
        jasmineEnv.execute();

        //place html reporter in it right place
        $timeout(function () {
            $(".test-result").append($(".jasmine_reporter"));
        }, 10);
    }

    $scope.tester.runAllTests = function () {
        $scope.tester.run(function () {
            //register all tests
            angular.forEach($scope.tests, function (test) {
                test();
            });
        });
    }

    $scope.tester.updateCamera = function () {
        $pyx.globe.getCameraAsync().success(function (camera) {
            $scope.camera = camera;
        });
    }

    $scope.tester.toggleCustomMode = function () {
        $scope.tester.customMode = !$scope.tester.customMode;

        if ($scope.tester.customMode && !$scope.tester.customTest) {
            $scope.tester.loadCustomTest();
        }
    }

    $scope.tester.loadCustomTest = function () {
        $pyx.application.load("customTest", "").success(function (test) {
            $timeout(function () {
                $scope.tester.customTest = test || defaultTest;
            });
        });
    }

    $scope.tester.saveCustomTest = function () {
        $pyx.application.save("customTest", $scope.tester.customTest);
    }

    $scope.tester.runCustomTest = function () {
        $scope.tester.run(function () {
            $scope.tester.compileError = undefined;
            try {
                eval($scope.tester.customTest);
            } catch (e) {
                $scope.tester.compileError = e.message;
            }
        });        
    }
});