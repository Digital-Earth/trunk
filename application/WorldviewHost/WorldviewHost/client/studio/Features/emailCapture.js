app.service('featureEmailCapture', function ($http, $pyx, $timeout, $location, wvAlerts) {
    function register ($scope) {
        $scope.addTemplate('dialogs', '/site/parts/modals/email-capture.html');

        var path = '/api/requestNews';

        $scope.captureEmail = {};
        $scope.captureEmail.isOpen = false;
        $scope.captureEmail.captured = false;
        $scope.captureEmail.submitting = false;
        $scope.captureEmail.model = { Email: "" }

        $scope.captureEmail.toggle = function ($event) {
            $event.preventDefault();
            $scope.captureEmail.isOpen = !$scope.captureEmail.isOpen;
        }

        $scope.captureEmail.submit = function () {
            $scope.captureEmail.submitting = true;
      
            $http({
                method: 'POST',
                url: path,
                data: $.param($scope.captureEmail.model),
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                }).success(function (data) {
                    $scope.captureEmail.submitting = false;
                    $scope.captureEmail.captured = true;
                    $scope.captureEmail.model.Email = "";
                    wvAlerts.success('Thank you for subscribing to the e-newsletter.', $scope);
                })
                .error(function () {
                    $scope.captureEmail.captured = false;
                    $scope.captureEmail.submitting = false;
                    wvAlerts.error('Sorry, we had a problem sending your request to our servers.', $scope);
                });
        }
    }

    return {
        register: register
    }

});