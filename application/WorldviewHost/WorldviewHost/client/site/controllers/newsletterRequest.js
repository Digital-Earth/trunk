app.controller('worldviewNewsRequestController', function ($scope, $http, $pyx, $location, wvAlerts) {
    $scope.systemStatus = 'ok';
    //$scope.systemStatusMessage = 'Sun, 24 Aug 2014 : Our servers are down for maintenance and testing';
   
    $scope.waiting = false;
    $scope.showSignBox = false;

    $scope.newsForm = {
        FirstName: "",
        LastName: "",
        Email: "",
        Email2: "",
        LinkedIn: "",
        Twitter: "",
        Country: "",
        State: "",
        City: "",
        Community: "",
        Interest: "",
        Geography: "",
        Where: "",
        Discover: "",
        Map: "",
        Yourself: "",
        HearAbout: "",
        HearAboutOther: "",
        Tools: "",
        Time: ""
    };
    $scope.newsError = "";
    $scope.newsSubmitted = false;
    
    $scope.submitNewsRequest = function () {
        $scope.newsError = "";

        if (!$scope.newsForm.FirstName ||
            !$scope.newsForm.LastName) {
            $scope.newsError = "Name is required";
            return;
        }
        if ($scope.newsForm.Email != $scope.newsForm.Email2) {
            $scope.newsError = "Verify email";
            return;
        }

        $scope.waiting = true;
        $http.post('/api/requestNews', $scope.newsForm)
            .success(function (data) {
                $scope.waiting = false;
                wvAlerts.success('Thank you for subscribing to the e-newsletter.', $scope);
                $scope.newsSubmittedId = data;
                $scope.newsSubmitted = true;
            })
            .error(function () {
                $scope.waiting = false;
                wvAlerts.error('Sorry, we had a problem sending your request to our servers.', $scope);
            });
    }
});