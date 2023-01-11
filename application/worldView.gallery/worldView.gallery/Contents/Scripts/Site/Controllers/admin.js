app.controller('worldviewAdminController', function ($scope, $http, $pyx, $pyxconfig, $interval) {
    $scope.servers = [];

    var refreshTime = 30 * 1000; //milliseconds

    function updateServers() {
        return $http.get($pyxconfig.baseUrl + "/Gwss")
            .success(function (data) {
                $scope.servers = data.Items;

                angular.forEach($scope.servers, function (server) {
                    server.PipelinesCount = server.PipelineStatuses.length;
                    server.DownloadingCount = 0;
                    server.PublishedCount = 0;
                    server.Operations = [];
                    angular.forEach(server.PipelineStatuses, function (pipeline) {
                        if (pipeline.Status == 'Published') server.PublishedCount++;
                        if (pipeline.Status == 'Downloading') server.DownloadingCount++;
                        if (pipeline.OperationStatus) {
                            server.Operations.push(pipeline.OperationStatus);
                        }
                    });
                });
            })
            .error(function (error) {
                $scope.errorMessage = error;
            });
    };

    updateServers().then(function () {
        var refresh = $interval(updateServers, refreshTime);

        $scope.$on('$destroy', function () {
            $interval.cancel(refresh);
        });
    });    
});