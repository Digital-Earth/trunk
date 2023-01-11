app.service('featureMyConnections', function (connectionIndex) {

    function register($scope) {
       
        $scope.connectionIndexModel = connectionIndex.getIndex();

        $scope.addItemToIndex = function (item, category) {
            connectionIndex.addItem(item, category);
            $scope.connectionIndexModel = connectionIndex.getIndex();
        }

        $scope.removeItemFromIndex = function (item) {
            connectionIndex.removeItem(item);
            $scope.connectionIndexModel = connectionIndex.getIndex();
        }

        // load My Connections from disk
        $scope.$on("studio-setup-started", function () {
            connectionIndex.load();
        });
    };

    return {
        register: register
    };
});