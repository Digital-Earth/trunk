/* 
feature: network check
goal: serves for a single purpose - automatic network state verification and notifying the user
*/
app.service('featureNetworkCheck', function ($filter, $interval, $timeout, $http, $pyxconfig, worldViewStudioConfig) {

    var i18n = $filter('i18n');
    
    function register($scope) {

        // list of network services inside the application that have reported a problem
        var failedNetworkServices = {};
        // indicates the network connection state
        $scope.isOffline = false;

        /*
        * a global method for use by network dependent services
        */
        $scope.alertNetworkServiceState = function (serviceName, isOk, message) {
            var alreadyReported = (serviceName in failedNetworkServices);
            if (!isOk) {
                // add the service to the list and inform the user only if it's not in the list already
                if (!alreadyReported) {
                    failedNetworkServices[serviceName] = "failed";
                    if (message) {
                        // to avoid a flood of notifications when the network goes down, delay the message for a period of network check
                        // and then only show it to the user if the network isn't down
                        $timeout(function () {
                            if (!$scope.isOffline) {
                                $scope.notifyError(message);
                            }
                        },
                        worldViewStudioConfig.alerts.networkCheckPeriod);
                    }
                }
            } else {
                // only pay attention if the service has reported an unrecovered problem
                if (alreadyReported) {
                    delete failedNetworkServices[serviceName];
                    if (message) {
                        // to synchronize service failure and recovery messages, hold this one for a period of network check as well
                        $timeout(function () {
                            $scope.notifyInfo(message);
                        },
                        worldViewStudioConfig.alerts.networkCheckPeriod);
                    }
                }
            }
        }

        /*
        an alternative way for a network dependent service to alert a failure or recovery is sending a global event
        */
        $scope.$on('alert-network-service-state', function (e, args) {
            $scope.alertNetworkServiceState(args.serviceName, args.state, args.message);
        });

        var runNetworkCheck = function () {
            try {
                // simply verify connection with the Pyxis backend: send a light request and check the result
                $http.head($pyxconfig.backendUrl)
                .success(
                    function (/*response*/) {
                        if ($scope.isOffline) {
                            $scope.notifyInfo(i18n('Network connection up'));
                        }
                        // once we know we are online, clean up the buffer of services in offline state - they'll update the states soon, if needed
                        failedNetworkServices = {};
                        // as long as the server has responded, the network is fine
                        $scope.isOffline = false;
                    })
                .error(
                    function (/*error*/) {
                        if (!$scope.isOffline) {
                            $scope.notifyError(i18n('Network connection down'));
                        }
                        // apparently, the network is down
                        $scope.isOffline = true;
                    }
                );
            }
            catch (error) {
                // unexpected error;
                // ignore it in this context ($http isn't supposed to throw anything)
            }
        }

        $interval(function () {
            // send a verification request only when the network has been offline or there are services that have reported offline state;
            // this reduces load on our backend by eliminating unnecessary checking requests
            if (Object.keys(failedNetworkServices).length || $scope.isOffline) {
                runNetworkCheck();
            }
        },
        worldViewStudioConfig.alerts.networkCheckPeriod);
    }

    return {
        register: register
    };
});