app.service('featureAboutDialog', function () {    

    function register($scope) {
        $scope.addTemplate('dialogs', '/template/feature-about.html');

        $scope.showAbout = false;

        $scope.openAbout = function () {
            $scope.showAbout = true;
        }

        $scope.closeAbout = function () {
            $scope.showAbout = false;
        }
    };

    return {
        register: register
    };
});