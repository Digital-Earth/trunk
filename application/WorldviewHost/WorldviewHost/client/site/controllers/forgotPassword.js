app.controller('worldviewForgotPasswordController', function ($scope, $http, $pyx, $location, wvAlerts, accountServices) {
    $scope.systemStatus = 'ok';
    //$scope.systemStatusMessage = 'Sun, 24 Aug 2014 : Our servers are down for maintenance and testing';

    $scope.requestForm = {
        UserName: ""
    };
    $scope.requestError = "";
    $scope.requestSubmitted = false;

    $scope.checkUserNameExists = function (value) {
        return accountServices.checkUserNameNotAvailable(value);
    }

    $scope.submitRequest = function () {
        $scope.requestError = "";

        if (!$scope.requestForm.UserName || $scope.requestForm.UserName.length < 5) {
            $scope.requestError = "Valid user name is required";
            return;
        }

        $pyx.gallery.forgotPassword($scope.requestForm.UserName)
            .success(function (data) {
                wvAlerts.success('Email sent.', $scope);
                $scope.requestSubmitted = true;
            })
            .error(function () {
                wvAlerts.error('Sorry, we had a problem sending your request to our servers.', $scope);
            });
    }
});