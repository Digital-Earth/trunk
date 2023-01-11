var app = angular.module("worldViewStudio", ["ngRoute", "flux", "pyxis", "pyxis-ui", "localization", "analytics"]);

app.config([
    "$locationProvider", function ($locationProvider) {
        $locationProvider.html5Mode(true);
    }
]);