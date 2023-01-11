app.controller('worldviewSingleSignOnController', function ($scope, $pyx, $routeParams, wvAlerts) {
    if (!$pyx.user.auth()) {
        wvAlerts.info('Please sign in to access WorldView Help Center.');
        $scope.$parent.enterSignInMode();
    } else {
        $pyx.user.jwt('zendesk')
            .success(function(data) {
                window.location = data.url + '?jwt=' + data.jwt + '&return_to=' + $routeParams['return_to'];
            })
            .error(function(error) {
                wvAlerts.error('Sorry, we had a problem sending your request to our servers.  Refresh the page to try again.', $scope);
            });
    }
});