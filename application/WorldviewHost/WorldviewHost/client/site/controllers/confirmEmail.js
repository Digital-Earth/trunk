app.controller('worldviewConfirmEmailController', function ($scope, $http, $pyx, $location, $routeParams, wvAlerts, user, $timeout) {
    $scope.systemStatus = 'ok';
    $scope.signedIn = $pyx.user.auth();
    $scope.sending = false;
    $scope.confirmed = false;
    $scope.submissionError = false;

    $scope.requestForm = {
        email: $routeParams['Email'],
        confirmationToken: $routeParams['Token']
    };

    $scope.submitRequest = function () {
        $pyx.gallery.confirmEmail($scope.requestForm.email, $scope.requestForm.confirmationToken)
            .success(function () {
                wvAlerts.success('Email Confirmed', $scope);
                $scope.confirmed = true;
                $pyx.user.completeLocalRegistration(user);
                $pyx.user.profile(true);
              
            })
            .error(function () {
                wvAlerts.error('Sorry, we had a problem sending your request to our servers.', $scope);
                // Wait 100ms -
                // If there's a problem, display the 'sendConfirmationEmail' button 
                $timeout(function () {
                    return $scope.submissionError = true;
                }, 100);
               

            });
    }

    $scope.sendConfirmationEmail = function () {
        $scope.sending = true;
        $pyx.gallery.sendConfirmationEmail()
            .success(function () {
                wvAlerts.success('Email Sent', $scope);
                $scope.sending = false;
            })
            .error(function () {
                wvAlerts.error('Sorry, we had a problem sending your request to our servers.', $scope);
                $scope.sending = false;
            });
    }

    if ($scope.requestForm.email && $scope.requestForm.confirmationToken) {
        if (!$pyx.user.auth()) {
            wvAlerts.error('Please sign in to confirm your email.');
            $scope.$parent.enterSignInMode();
        } else {
            $scope.submitRequest();
        }
    }
});