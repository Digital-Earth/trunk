app.service("featureHelpCenter", function ($pyx, $timeout, $window, $pyxIntercom) {

    function register($scope) {
        $scope.$on("studio-setup-completed", function () {
            $scope.getHelpCenter = function () {
                // Allows the User to access 'Pyxis Support'
                $pyx.user.jwt("zendesk").success(function (response) {
                    $timeout(function () {
                        if ($scope.$$phase) {
                            $window.open(response.url + "?jwt=" + response.jwt, "_blank");
                        } else {
                            $window.open(response.url + "?jwt=" + response.jwt, "_blank");
                            $scope.$apply();
                        }
                    });
                });
                $pyxIntercom.track('access-help-center');
            }

            $scope.getVideos = function () {
                window.open('https://www.youtube.com/playlist?list=PLhi3gHiey51Caj0Ct8wD37AG0lWV79Y-J','_blank');
                $pyxIntercom.track('access-videos');
            }

            $scope.getUserInfo = function () {
                window.open('https://www.pyxisinnovation.com/Downloads/WorldViewStudio-UserGuide.pdf','_blank');
                $pyxIntercom.track('access-user-guides');
            }
        });
    };

    return {
        register: register
    };
});