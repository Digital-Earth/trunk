app.controller('worldviewResetPasswordController', function ($scope, $http, $pyx, $location, $routeParams, wvAlerts) {
    $scope.systemStatus = 'ok';
    //$scope.systemStatusMessage = 'Sun, 24 Aug 2014 : Our servers are down for maintenance and testing';

    $scope.requestForm = {
        UserName: $routeParams['UserName'],
        ResetToken: $routeParams['Token'],
        NewPassword: "",
        ConfirmPassword: ""
    };
    $scope.requestError = "";

    $scope.submitRequest = function () {
        $scope.requestError = "";

        if (!$scope.requestForm.UserName) {
            $scope.requestError = "User name is required";
            return;
        }
        if (!$scope.requestForm.ResetToken) {
            $scope.requestError = "Unable to reset password without a reset token. Click on the link in your email after completing the forgot your password form.";
            return;
        }
        if (!$scope.requestForm.NewPassword) {
            $scope.requestError = "New password is required";
            return;
        }
        if ($scope.requestForm.NewPassword !== $scope.requestForm.ConfirmPassword) {
            $scope.requestError = "Passwords do not match";
            return;
        }

        $pyx.gallery.resetPassword($scope.requestForm.UserName, $scope.requestForm.ResetToken, $scope.requestForm.NewPassword, $scope.requestForm.ConfirmPassword)
            .success(function (data) {
                wvAlerts.success('New password set.  You can now sign in.', $scope);
                // remove url parameters
                $location.url('/updatedPassword');
            })
            .error(function () {
                wvAlerts.error('Sorry, we had a problem sending your request to our servers.', $scope);
            });
    }
});