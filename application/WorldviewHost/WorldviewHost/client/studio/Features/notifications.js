app.service('featureNotifications', function ($timeout, worldViewStudioConfig) {

    function register($scope) {
        $scope.addTemplate('widgets', '/client/templates/studio/template/feature-notifications.html');

        $scope.alertArchive = [];

        var alertBufferTime = 100;
        var clearAlertArchive = (60 * 2000);
        var maxMessagesVisible = 3;
        var notifyId = 0;

        var inActiveAlert = function () {
            $scope.alertArchive = [];
        };

        var alertTimeout = $timeout(inActiveAlert, clearAlertArchive);

        var retrieveAlert = function (alert) {
            var index = -1;

            for (var i = 0, length = $scope.alertArchive.length; i < length; i++) {
                var id = $scope.alertArchive[i].id;

                if (alert.id === id) {
                    index = i;
                    break;
                }
            }

            return index;
        };

        var hideAlert = function (alert) {
            var index = retrieveAlert(alert);

            alert.show = false;

            $timeout(function () {
                alert.visible = false;
                if (index > -1) {
                    $scope.alertArchive.splice(index, 1);
                }
            }, alertBufferTime);
        }

        var showAlert = function (alert) {
            alert.visible = true;

            $timeout(function () {
                alert.show = true;
            }, alertBufferTime);
        }

        $scope.hideMessage = function (alert) {
            hideAlert(alert);
        };

        $scope.notifyMessage = function (type, message, options) {
            var settings = options || {};
            notifyId += 1;

            $scope.alert = {
                message: message,
                type: type,
                show: false,
                id: notifyId,
                hideImmediately: false,
                visible: true
            };

            angular.extend($scope.alert, settings);

            // - when there's no new notifications for 
            // 2 minutes clear the queue
            $timeout.cancel(alertTimeout);
            alertTimeout = $timeout(inActiveAlert, clearAlertArchive);

            $scope.alertArchive.unshift($scope.alert);

            showAlert($scope.alert);

            // - never have more than 2 messages visible
            // - there's no need to keep the Welcome message visible
            if ($scope.alertArchive.length >= maxMessagesVisible) {
                $timeout(function () {
                    var total = $scope.alertArchive.length - 1;
                    var alert = $scope.alertArchive[total];

                    hideAlert(alert);

                }, worldViewStudioConfig.alerts.alertPopupTime);
            }

            if ($scope.alert.hideImmediately) {
                $timeout(function () {
                    hideAlert($scope.alert);
                }, worldViewStudioConfig.alerts.alertPopupTime);
            }

        };

        $scope.notifyInfo = function (message) {
            var options = {
                hideImmediately: true
            };

            $scope.notifyMessage('info', message, options);
        };

        $scope.notifyError = function (message) {
            $scope.notifyMessage('error', message);
        };

        $scope.notifyWarning = function (message) {
            $scope.notifyMessage('warning', message);
        };
    };

    return {
        register: register
    };
});