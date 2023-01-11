app.controller('downloadController', function ($scope, $pyx, userPlatform, analytics) {

    // Are users on a supported OS?
    $scope.worldViewSupported = userPlatform.isSupported('WorldView');
    $scope.userPlatformUnknown = false;

    $scope.requirements = {
        visibility: 'hidden'
    }

    //check for false negatives - new browser versions 
    //may change the way they handle OS and OS version
    var checkFalseNegative = function () {
        if (!$scope.worldViewSupported) {
            if ($scope.userOS === 'Unknown' || $scope.userOSVersion === 'Unknown') {
                $scope.userPlatformUnknown = true;
            }
        }
    }

    checkFalseNegative();

    if ($scope.worldViewSupported) {
        analytics.event('download page', 'supported');
    } else {
        analytics.event('download page', 'not supported');
    }

});