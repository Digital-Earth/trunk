app.service('featureFirstUse', function ($pyx, $filter, worldViewStudioConfig) {
    var i18n = $filter('i18n');

    function register($scope) {
        $scope.addTemplate('dialogs', '/template/feature-first-use.html');

        //show the welcome 'Getting Started' screen again
        $scope.optInGettingStarted = function () {
            $scope.gettingStartedMode = 'show';
            localStorage['gettingStarted'] = 'show';
            $scope.notifyInfo(i18n("Next time you start WorldView you'll see the Welcome screen."));
        }

        //hide the welcome 'Getting Started' screen
        $scope.optOutGettingStarted = function () {
            $scope.gettingStartedMode = 'hide';
            localStorage['gettingStarted'] = 'hide';
            $scope.notifyInfo(i18n("Next time you start WorldView you won't see the Welcome screen."));
        }

        //grab the current state of the welcome 'Getting Started' screen
        $scope.gettingStartedMode = localStorage.getItem('gettingStarted');

        //display a 'Get Started' dialog with a collection of maps
        function initFirstTimeUse() {
            //creating the dialog model
            $scope.getYouStarted = {
                active: false, //we don't show the dialog until the maps are loaded
                optOut: function () {
                    $scope.optOutGettingStarted();
                    $scope.getYouStarted.active = false;
                },
                close: function () {
                    $scope.getYouStarted.active = false;
                },
                'import': function (map) {
                    var inLibrary = $scope.mapInLibrary(map);

                    if (!inLibrary) {
                        $scope.addResource(map);
                        $scope.getYouStarted.close();

                    } else {
                        $scope.notifyError(i18n("It looks like this globe is already in your library!"));
                    }
                },
                maps: []
            };

            //In case we can't load maps from the "GettingStarted" gallery...
            function showRandomNewMaps() {
                var query = $pyx.gallery.maps().orderByDesc("Metadata.Updated");
                query.top = 10;
                query.get().success(function (maps) {
                    $scope.getYouStarted.maps = maps;
                    $scope.getYouStarted.active = true;
                });
            }

            if (worldViewStudioConfig.firstUse.galleryId) {
                //load a gallery
                var galleryFakeJson = { Type: 'Gallery', 'Id': worldViewStudioConfig.firstUse.galleryId };
                $pyx(galleryFakeJson)
                    .resources()
                    .success(function (resources) {
                        //get all maps from the gallery.
                        $scope.getYouStarted.maps = resources.filter(function (resource) {
                            return resource.Type === 'Map';
                        });
                        //show the dialog
                        $scope.getYouStarted.active = true;
                    })
                    .error(function () {
                        //we failed to load resources... show random maps
                        showRandomNewMaps();
                    });
            } else {
                //no "GettingStarted" gallery found in settings.
                showRandomNewMaps();
            }
        }
      
        $scope.$on('studio-setup-completed', function () {
            var gettingStarted = localStorage.getItem('gettingStarted');

            if (gettingStarted === 'show' || !gettingStarted) {
                initFirstTimeUse();
            }
        });
    };

    return {
        depends: ['notifications'],
        register: register
    };
});