app.service('featureFeedback', function ($pyx,$filter) {
    var i18n = $filter('i18n');

    function register($scope) {
        $scope.addTemplate('dialogs', '/template/feature-feedback.html');

        $scope.feedbackModal = {
            enabled: false,
            active: false,

            cancel: function () {
                $scope.feedbackModal.active = false;
            },
            send: function () {
                var model = $scope.feedbackModal;
                $pyx.when($pyx.application.sendFeedback(model.message, model.screenshot, model.sessionLog)).then(
                    function () {
                        $scope.notifyInfo(i18n("feedback.sentCompleted"));
                    }, function () {
                        $scope.notifyError(i18n("feedback.sendFailed"));
                    });
                $scope.feedbackModal.active = false;
            },
            show: function () {
                $pyx.application.captureApplicationState();
                $scope.feedbackModal.message = "";
                $scope.feedbackModal.screenshot = true;
                $scope.feedbackModal.sessionLog = true;

                $scope.feedbackModal.active = true;
            }
        }

        $scope.$on('pyx-application-ready', function () {
            if ('captureApplicationState' in $pyx.application) {
                $scope.feedbackModal.enabled = true;
            }
        });
    };

    return {
        depends: ['notifications'],
        register: register
    };
});