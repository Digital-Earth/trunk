/*
worldViewStudioController aimed to be the skin for web demos (not fully working studio)
*/
app.controller("worldViewStudioController", function (
    $scope, $pyx, $pyxconfig, worldViewStudioBootstrap, $window, $timeout, $location,

    featureShare,
    featureWalkThrough,
    featureInspectorTools

    ) {

    $scope.excludedTemplates = ["/client/templates/studio/template/feature-dashboard.html"];

    worldViewStudioBootstrap($scope, { mode: 'viewer' });

    featureShare.register($scope);

    // temporary debug
    $window.$scope = $scope;

    featureInspectorTools.register($scope);

    $scope.uiActive = true;
    $scope.uiEditable = true;

    $scope.$on("pyx-engine-ready", function () {
        console.log("PYX ENGINE READY");

        // redefine the "addGeoSource" method to delete
        // the currentMap so that opens the geoSource rather than importing on top
        // also go to the geosource at the end
        var oldAddFunction = $scope.addGeoSource;

        $scope.addGeoSource = function(geoSource, style){

            // pass in geoSource name to new map
            $scope.createNewMap(geoSource.Metadata.Name);

            console.log("custom add geoSource ", geoSource);
            oldAddFunction(geoSource, style);
            $pyx.globe.gotoGeoSource(geoSource, 1000);
        }

        $('.loading-bar > span').css('width', '10%');

        $scope.quickAdd = {
            active: false,
            editMapName: false,
            mode: undefined,
            show: function(mode) {
                $scope.quickAdd.active = true;
                $scope.quickAdd.mode = mode;
                $scope.quickAdd.items = [];

                if ($scope.quickAdd.GalleryId) {
                    var getGallery = $pyx.gallery.galleries().getById($scope.quickAdd.GalleryId);
                } else {
                    var getGallery = $pyx.gallery.galleries().getByName('GeoliteracyAustralia');
                }

                getGallery.success(function(gallery) {
                    $scope.quickAdd.GalleryName = gallery.Metadata.Name;
                    $pyx(gallery).resources().success(function(resources) {
                        $scope.quickAdd.items = []

                        resources.push( {
                            Type: 'GeoSource',
                            Id: 'ac8436e7-e492-4bed-ba17-e2007f6d1099',
                            Metadata: {
                                Name: 'Global Base Imagery - Terra Pixel',
                                Description: '(C) 2013 Microsoft Digital Globe - for non commercial demonstration only',
                                Tags: [ 'Imagery', 'WebService']
                            }
                        });

                        resources.push( {
                            Type: 'GeoSource',
                            Id: '21a3dfe0-6c19-4618-88c9-ab94bf842955',
                            Metadata: {
                                Name: 'World Elevation 2 Minute',
                                Description: 'Gridded (2 minute) elevation and bathymetry for the world.',
                                Tags: [ 'Global', 'Elevation']
                            }
                        });

                        resources.push( {
                            Type: 'Map',
                            Id: 'e6a7665e-cf20-4451-9708-77161fc2711e',
                            Metadata: {
                                Name: 'The Globe',
                                Description: 'GTOPO30 styled to perfection\n source: https://lta.cr.usgs.gov/GTOPO30',
                                Tags: [ 'Global', 'Elevation']
                            }
                        });


                        angular.forEach(resources, function(resource) {
                            if (resource.Type == mode) {
                                $scope.quickAdd.items.push(resource);
                            }
                        });

                    });
                });
            },
            hide: function() {
                $scope.quickAdd.active = false;
                if (!$scope.currentMap) {
                    $scope.quickAdd.newMap();
                }
            },
            add: function(item) {
                $scope.quickAdd.hide();
                $pyx(item).refresh().success(function(updated) {
                    if (updated.Type == "GeoSource") {
                        var previousCamera = $pyx.globe.getCamera();

                        //show a "goto back to previous location" button after 1 sec
                        $timeout(function() {
                            $scope.quickAdd.previousCamera = previousCamera;
                        },1000);

                        //hide that button after 10 sec
                        $timeout(function() {
                            $scope.quickAdd.previousCamera = undefined;
                        },10000);

                        oldAddFunction(updated);
                        $pyx.globe.gotoGeoSource(updated, 1000);
                    }
                    if (updated.Type == "Map") {
                        $scope.addMap(updated);
                        if (updated.Metadata.Visibility == 'NonDiscoverable') {
                            $location.path("/view/" + updated.Id);
                        } else {
                            $location.path("/view/" + $scope.quickAdd.GalleryName + "/" + updated.Metadata.Name);
                        }

                    }
                });
            },
            goBackToLocation: function() {
                $pyx.globe.setCamera($scope.quickAdd.previousCamera, 1000);
                $scope.quickAdd.previousCamera = undefined;
            },
            newMap: function() {
                $scope.createNewMap("New Globe");
                $scope.quickAdd.editMapName = true;
            },
            search: function() {
                $scope.quickSearch.active = true;
            }
        };

        var waitingForSearchToStart = true;

        function gotoFirstResult() {
            var location = $pyx.obj.get($scope.geocodeService.results,0,'features',0);
            if (location) {
                $scope.handleSearchAction("goto",location);
                $scope.geocodeService.results = [];
                $scope.quickSearch.active = false;
            } else if ( $scope.isSearchInProgress() ) {
                waitingForSearchToStart = false;
                $timeout(gotoFirstResult, 100);
            } else if (waitingForSearchToStart) {
                $timeout(gotoFirstResult, 100);
            } else {
                $scope.quickSearch.active = false;
            }
        }

        $scope.quickSearch = {
            active: false,
            text: "",
            search: function() {
                $scope.search = $scope.quickSearch.text;
                $scope.searchChanged(0);
                waitingForSearchToStart = true;
                gotoFirstResult();
            }
        }

        $window.resourceObject = $window.resourceObject || {}

        switch($window.resourceObject.Type) {
            case 'GeoSource':
                $scope.quickAdd.GalleryId = $window.resourceObject.Metadata.Providers[0].Id;
                $scope.addGeoSource($window.resourceObject);
                break;

            case 'Map':
                $scope.quickAdd.GalleryId = $window.resourceObject.Metadata.Providers[0].Id;
                // pass in map object that is embeded within the html from the server
                $scope.setCurrentMap($window.resourceObject);

                if( $window.resourceObject.Camera){
                    // TODO :: support an on load event for map/globe interface
                    setTimeout(function(){
                        $pyx.globe.setCamera($window.resourceObject.Camera, 2000);
                    }, 3000);
                }
                break;

            case 'Gallery':
                $scope.quickAdd.GalleryId = $window.resourceObject.Id;
                $scope.quickAdd.show('Map');
                break;
        }
    });
});

