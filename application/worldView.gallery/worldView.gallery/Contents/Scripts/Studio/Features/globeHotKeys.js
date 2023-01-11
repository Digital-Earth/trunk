app.service("featureGlobeHotKeys", function ($pyx, worldViewStudioConfig) {

    function register($scope) {
        var globeKeys = {
            189: function (camera) { //-
                camera.Range = worldViewStudioConfig.globe.zoomLevelFactor * camera.Range;
                return camera;
            },
            187: function (camera) { //+
                camera.Range = (1.0 / worldViewStudioConfig.globe.zoomLevelFactor) * camera.Range;
                return camera;
            },
            33: function (camera) { //PageUp
                if (camera.Tilt > worldViewStudioConfig.globe.tiltRotationAmount) {
                    camera.Tilt -= worldViewStudioConfig.globe.tiltRotationAmount;
                    return camera;
                }
                return null;
            },
            34: function (camera) { //PageDown
                if (camera.Tilt < 90 - worldViewStudioConfig.globe.tiltRotationAmount) {
                    camera.Tilt += worldViewStudioConfig.globe.tiltRotationAmount;
                    return camera;
                }
                return null;
            },
            35: function (camera) { //End
                camera.Heading += worldViewStudioConfig.globe.headingRotationAmount;
                return camera;
            },
            36: function (camera) { //Home
                camera.Heading -= worldViewStudioConfig.globe.tiltRotationAmount;
                return camera;
            }
        };

        $scope.$on("pyx-globe-keyup", function ($event,args) {

            if (args.$event.keyCode in globeKeys) {
                var animation = globeKeys[args.$event.keyCode];

                var camera = $pyx.globe.getCamera();

                var newCamera = animation(camera);
                if (newCamera) {
                    $pyx.globe.setCamera(newCamera, worldViewStudioConfig.globe.navigationAnimationDuration);
                }
            }
        });
    };

    return {
        register: register
    };
});