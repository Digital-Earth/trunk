app.factory('networkServiceStateAlert', function ($rootScope) {
    return function networkServiceStateAlert(serviceName, isOk, message) {
        $rootScope.$broadcast('alert-network-service-state',
            { serviceName: 'searchResultResource', state: false, message: message });
    }
});