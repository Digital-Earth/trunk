app.controller('worldviewCreateController', function ($scope, $http, $pyx, $pyxuser, wvAlerts, fileUploader, $location, user, resourceType) {
    $scope.profile = user.profile;
    $scope.galleries = user.galleries;
    $scope.gallery = $scope.galleries[0];
    $scope.resourceType = resourceType;
    $scope.storage = { used: 0, quota: 0, usedPercent: 100, newPercent: 100 }
    $scope.newValues = window.wvCreateJson ||
        {
            Metadata: {
                Name: "",
                Description: "",
                Tags: [],
                SystemTags: [],
                ExternalUrls: [],
                Visibility: 'Public',
            },
            Type: resourceType,
            State: "Active",
            ProcRef: "",
            Definition: "",
            DataSize: 0
        };

    if (!$scope.newValues.Metadata.Visibility) {
        $scope.newValues.Metadata.Visibility = 'Public';
    }

    if (!$scope.newValues.Metadata.ExternalUrls) {
        $scope.newValues.Metadata.ExternalUrls = [];
    }

    $scope.showTerms = false;
    $scope.acceptTerms = true;

    $scope.isGeoSource = function () {
        return $scope.resourceType == 'GeoSource';
    }

    $scope.withinStorageQuota = function () {
        if (!$scope.isGeoSource()) {
            return true;
        }
        if($scope.newValues.DataSize == undefined) {
            return false;
        }
        return ($scope.storage.used + $scope.newValues.DataSize < $scope.storage.quota);
    }

    $scope.showTermsOpen = function () {
        $scope.showTerms = true;
    }

    $scope.showTermsCommit = function () {
        $scope.showTerms = false;
        $scope.acceptTerms = true;
    }

    $scope.showTermsCancel = function () {
        $scope.showTerms = false;
        $scope.acceptTerms = false;
    }

    $scope.uploadImage = function ($files) {
        $scope.newImageFile = $files[0];

        var ext = $scope.newImageFile.name.split('.').pop();
        var newName = $scope.newValues.Id + "." + ext;

        fileUploader.readAsDataURL($scope.newImageFile)
            .then(function (dataUrl) {
                $scope.newImageUploaded = true;
                $scope.img = dataUrl;
                return fileUploader.upload(imageServer.setImageUrl(), $scope.newImageFile, newName)
                    .catch(function () { return wvAlerts.error('Failed to upload ' + resourceType + ' image', $scope); });
            }, /*catch*/ function (error) {
                return wvAlerts.error('Failed to read image', $scope);
            })
            .then(function (fileUpload) {
                var index = $pyx.array.firstIndex($scope.newValues.Metadata.ExternalUrls, "Type", "Image");
                if (index != -1) {
                    $scope.newValues.Metadata.ExternalUrls.splice(index, 1);
                }
                $scope.newValues.Metadata.ExternalUrls.push({ Type: 'Image', url: imageServer.getImageUrl(newName) });
            });
    }

    $scope.commitCreateMode = function () {
        $scope.newValues.Metadata.Providers = [{'Type':$scope.gallery.Type,'Id':$scope.gallery.Id,'Name':$scope.gallery.Metadata.Name}];

        $pyx.gallery.create($scope.newValues)
            .success(function () {
                try {
                    $pyx.embedded.notifyPublishCompleted();
                } catch (e) {
                    wvAlerts.error('Internal Error: ' + e);
                }
                $location.path('/' + resourceType + '/' + $scope.newValues.Id);
            })
            .error(function (error) {
                wvAlerts.error('Failed to publish a new ' + resourceType + ': ' + error.error_description);
            });
    }

    $scope.cancelCreateMode = function () {
        $pyx.embedded.notifyPublishCanceled();
        wvAlerts.info('Publishing a new ' + resourceType +' was canceled');
        $location.path('/');
    }

    if ($scope.isGeoSource()) {
        $pyxuser.storage().success(function (data) {
            $scope.storage.used = parseInt(data); 
            $pyxuser.quota().success(function (data) {
                $scope.storage.quota = parseInt(data);
                $scope.storage.usedPercent = 100 * ($scope.storage.used / $scope.storage.quota);
                $scope.storage.newPercent = 100 * ($scope.newValues.DataSize / $scope.storage.quota);
            })
            .error(function (error) {
                wvAlerts.error('Failed to obtain storage usage from the server');
            });
        })
        .error(function (error) {
            wvAlerts.error('Failed to obtain storage quota from the server');
        });
    }
});