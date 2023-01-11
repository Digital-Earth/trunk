app.service('featureShare', function ($pyx) {

    function register($scope) {
        $scope.addTemplate('dialogs', '/client/templates/studio/template/feature-share.html');

        $scope.shareModal = {
            active: false,
            step: "setup"
        }

        function makeMapAnonymous(map) {
            //remove id to force server to generate a new unique one
            delete map.Id;
            delete map.Version;

            //remove information about the user (should we?)
            delete map.Metadata.User;

            //remove information about providers
            delete map.Metadata.Providers;
        }
        
        function addUserToMap(map) {
            if ($pyx.user.id) {
                map.Metadata.User = {
                    Id: $pyx.user.id,
                    Name: $pyx.user.username()
                }
            }
        }

        function addScreenshotToMap(map,image) {
            map.Metadata.ExternalUrls = [
                {
                    Type: "Image",
                    Url: image
                }
            ];
        }

        function addCameraPositionToMap(map,camera) {
            map.Camera = camera;
        }

        function popuplateDefaultNameAndDescription(map,activeItems)
        {
            if (activeItems.length == 1) {
                map.Metadata.Name = activeItems[0].Metadata.Name;
                map.Metadata.Description = activeItems[0].Metadata.Description;
                map.Metadata.Tags = activeItems[0].Metadata.Tags;
            } else {
                if (!map.Metadata.Description) {
                    map.Metadata.Description = "Map made with love by integrating:\n";
                    angular.forEach(activeItems,function(item) {
                        map.Metadata.Description += item.Metadata.Name + "\n";
                    });
                }
                var tags = $pyx.tags.itemTags(map);
                if (!tags.any()) {
                    angular.forEach(activeItems,function(item) {
                        if (item.Metadata.Tags) {
                            tags.add(item.Metadata.Tags[0]);
                        }
                    });
                }
            }
        }

        $scope.shareModal.open = function () {
            if (!$scope.currentMap) {
                $scope.notifyInfo("Please create a map first");
                return;
            }

            var activeItems = $scope.currentMap.activeItems();

            if (activeItems.length == 0) {
                $scope.notifyInfo("Map as no visible GeoSources, Please add some data first.");
                return;
            }

            $pyx.globe.capture().success(function(captureUrl) {
                $scope.shareModal.active = true;
                $scope.shareModal.step = "setup";

                
                $scope.shareModal.screenCapture = captureUrl;
                var newMap = angular.copy($scope.currentMap.model);

                makeMapAnonymous(newMap);
                if ($pyx.user.auth()) {
                    addUserToMap(newMap);
                }
                addScreenshotToMap(newMap,$scope.shareModal.screenCapture);
                addCameraPositionToMap(newMap,$pyx.globe.getCamera());
                popuplateDefaultNameAndDescription(newMap,activeItems);

                //TODO: enable that when LS is ready to accept dataUrl for non anonymous maps
                //newMap.State = "NonDiscoverable";

                $scope.shareModal.map = newMap;
            });
        }

        $scope.shareModal.commit = function () {

            $scope.shareModal.step = "done";
            $scope.shareModal.url = "Generating...";
            $scope.shareModal.urlValid = false;

            var anonymous = !$pyx.user.auth();
            $pyx.gallery.create($scope.shareModal.map, { anonymous: anonymous })
            .success(function(map) {
                $scope.shareModal.url = window.location.protocol + "//" + window.location.host + "/view/" + map.Id
                $scope.shareModal.urlValid = true;
            }).error(function(error) {
                $scope.shareModal.url = "Failed: " + error
            });
        }

        $scope.shareModal.close = function () {
            $scope.shareModal.active = false;
        }
        
        $scope.shareModal.copyUrl = function () {
            var input = $("input[ng-model='shareModal.url']").select();
            document.execCommand('copy');
            $scope.notifyInfo("Url copied to your clipboard");
        }
    };

    return {
        register: register
    };
});