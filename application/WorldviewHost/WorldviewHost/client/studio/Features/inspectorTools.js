/*
featureInspectorTools allow developer to get the map/camera document using the inspector window in the browser.

window.getCurrentMapModel() -> return the current map model object 
window.getCurrentMap() -> return a json string represent the current map
window.getCurrentCamera() -> return a json string represent the current camera location
*/
app.service('featureInspectorTools', function ($pyx) {

    function register($scope) {
        window.getCurrentMapModel = function () {
            if ($scope.currentMap) {
                return $scope.currentMap.model;
            }
            return null;
        };

        window.getCurrentMap = function () {
            return angular.toJson(window.getCurrentMapModel());
        }

        window.getCurrentCamera = function () {
            return angular.toJson($pyx.globe.getCamera());
        }
    };

    return {
        register: register
    };
});