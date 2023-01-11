app.controller('worldviewUserController', function ($scope, $location, $pyx, $routeParams, imageServer, wvAlerts, user) {
    if ($pyx.user.auth()) {
        $scope.emailConfirmed = $scope.profile.EmailConfirmed;
    }
    $scope.profile = user.profile;

    $scope.owner = $pyx.user.id == $scope.profile.Id;
    
    $scope.cards = user.galleries;
    var index = $pyx.array.firstIndex($scope.profile.Metadata.ExternalUrls, "Type", "Image");
    if (index != -1) {
        $scope.banner = imageServer.fixImageUrl($scope.profile.Metadata.ExternalUrls[index].Url, $scope.profile.Version);
    }

    $scope.newGalleryMode = false;
    $scope.newValues = {};

    $scope.enterNewGalleryMode = function () {
        $scope.newGalleryMode = true;
        $scope.newValues = {
            Name: "",
            Description: "",
            Tags: [],
            Visibility: 'Public'
        };
    }

    $scope.cancelNewGalleryMode = function () {
        $scope.newGalleryMode = false;
    }

    $scope.commitNewGalleryMode = function () {
        if (!/\S/.test($scope.newValues.Name)) {
            wvAlerts.error('Gallery must have a name', $scope);
            return;
        }

        var newResource = {
            Type: 'Gallery',
            Metadata: {
                Name: $scope.newValues.Name,
                Description: $scope.newValues.Description,
                Tags: $scope.newValues.Tags,
                Visibility: $scope.newValues.Visibility
            }
        }

        $pyx.gallery.create(newResource)
            .success(function (resource) {
                wvAlerts.success($scope.newValues.Name + ' created successfully', $scope);
                $location.path('Gallery/' + resource.Id);
            }).
            error(function () {
                wvAlerts.error('Failed to create Gallery', $scope);
            });
    }

    $scope.checkNewGalleryName = function ($value) {
        if (!$value || $value.length == 0) return false;

        return $pyx.gallery.galleries().nameAvailable($value).then(function () { return true; });
    }
});