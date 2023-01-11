app.service("featureFps", function ($pyx, $timeout, worldViewStudioConfig) {

    function register($scope) {
        $scope.addTemplate('dialogs', '/template/feature-fps.html');

        $scope.renderReportRefresh = "auto";
        var updateFrameRate = function () {
            if ($scope.renderReportRefresh !== "stop") {
                var newReport = $pyx.globe.renderReport();

                if ($scope.renderReportRefresh === "auto") {
                    $scope.renderReport = newReport;
                }
                if ($scope.renderReportRefresh === "step") {
                    if (newReport.RenderTimeInMilliseconds > worldViewStudioConfig.fpsTracing.slowFrame) {
                        $scope.renderReport = newReport;
                        $scope.renderReportRefresh = "stop";
                    }
                }
            }
            $timeout(updateFrameRate, worldViewStudioConfig.fpsTracing.fpsReportUpdate);
        };
        updateFrameRate();
    };

    return {
        register: register
    };
});