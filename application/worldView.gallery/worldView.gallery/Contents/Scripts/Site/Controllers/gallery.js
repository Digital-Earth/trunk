app.controller('worldviewGalleryController', function ($scope, $pyx, $routeParams, fileUploader, imageServer, wvAlerts, $q, $location, gallery) {
    
    var findImages = function () {
        $pyx.array.first($scope.gallery.Metadata.ExternalUrls, "Type", "Image")
            .found(function (externalUrl) { $scope.banner = imageServer.fixImageUrl(externalUrl.Url, $scope.gallery.Version); });
        
        $pyx.array.first($scope.gallery.Metadata.ExternalUrls, "Type", "Icon")
            .found(function (externalUrl) { $scope.logo = imageServer.fixImageUrl(externalUrl.Url, $scope.gallery.Version); });
    }
    $scope.gallery = gallery.gallery;
    $scope.owner = $pyx.user.id == $scope.gallery.Metadata.User.Id;
    $scope.cards = gallery.resources;

    findImages();

    $scope.editmode = false;
    $scope.newValues = {};

    $scope.enterEditMode = function () {
        $scope.editmode = true;
        $scope.newValues = {
            Name: $scope.gallery.Metadata.Name,
            Description: $scope.gallery.Metadata.Description,
            Tags : angular.copy($scope.gallery.Metadata.Tags),
            Visibility: $scope.gallery.Metadata.Visibility,
            banner: $scope.banner,
            bannerUpdated: false,
            bannerFile: undefined,
            updateBanner: function ($files) {
                $scope.newValues.bannerFile = $files[0];
                fileUploader.readAsDataURL($scope.newValues.bannerFile).then(function (dataUrl) {
                    $scope.newValues.bannerUpdated = true;
                    $scope.newValues.banner = dataUrl;
                });
            },
            logo: $scope.logo,
            logoUpdated: false,
            logoFile: undefined,
            updateLogo: function ($files) {
                $scope.newValues.logoFile = $files[0];
                fileUploader.readAsDataURL($scope.newValues.logoFile).then(function (dataUrl) {
                    $scope.newValues.logoUpdated = true;
                    $scope.newValues.logo = dataUrl;
                });
            }
        };
    }

    $scope.cancelEditMode = function () {
        $scope.editmode = false;
    }

    $scope.commitEditMode = function () {
        var newResource = {
            Type: $scope.gallery.Type,
            Id: $scope.gallery.Id,
            Version: $scope.gallery.Version,
            Metadata: {
                //Name: $scope.newValues.Name,
                Description: $scope.newValues.Description,
                Tags: $scope.newValues.Tags,
                Visibility: $scope.newValues.Visibility
            }
        }

        if ($scope.newValues.bannerUpdated || $scope.newValues.logoUpdated) {
            newResource.Metadata.ExternalUrls = angular.copy($scope.gallery.Metadata.ExternalUrls);
        }

        var uploadBanner = function() {
            if (!$scope.newValues.bannerUpdated) {
                return $q.when('no new banner');
            }

            var ext = $scope.newValues.bannerFile.name.split('.').pop();
            var newName = $scope.gallery.Id + "." + ext;

            return fileUploader.upload(imageServer.setBannerUrl(), $scope.newValues.bannerFile, newName)
                .then(function () {
                    $pyx.array.first(newResource.Metadata.ExternalUrls, "Type", "Image")
                        .found(function (externalUrl) {
                            externalUrl.Url = imageServer.getBannerUrl(newName);
                        })
                        .notFound(function () {
                            newResource.Metadata.ExternalUrls.push({ Type: 'Image', url: imageServer.getBannerUrl(newName) });
                        });
                })
                .catch(function() {
                    return wvAlerts.error('Failed to uploaded gallery banner');
                });
        }

        var uploadLogo = function () {
            if (!$scope.newValues.logoUpdated) {
                return $q.when('no new logo');
            }

            var ext = $scope.newValues.logoFile.name.split('.').pop();
            var newName = $scope.gallery.Id + "." + ext;

            return fileUploader.upload(imageServer.setAvatarUrl(), $scope.newValues.logoFile, newName)
                .then(function () {
                    $pyx.array.first(newResource.Metadata.ExternalUrls, "Type", "Icon")
                        .found(function (externalUrl) {
                            externalUrl.Url = imageServer.getAvatarUrl(newName);
                        })
                        .notFound(function () {
                            newResource.Metadata.ExternalUrls.push({ Type: 'Icon', url: imageServer.getAvatarUrl(newName) });
                        });                    
                })
                .catch(function () {
                    return wvAlerts.error('Failed to uploaded gallery logo');
                });
        }

        $q.all([uploadBanner(), uploadLogo()])
            .then(function () {
                return $pyx.gallery.update(newResource)
                    .catch(function() {
                        return wvAlerts.error('Failed to fetch updated gallery details');
                    });
            }).then(function() {
                wvAlerts.success('Gallery updated successfully');
                return $pyx($scope.gallery).refresh();
            }).then(function (result) {
                $scope.editmode = false;
                $scope.gallery = result.data;

                if ($scope.newValues.bannerUpdated) {
                    $scope.banner = $scope.newValues.banner;
                }
                if ($scope.newValues.logoUpdated) {
                    $scope.logo = $scope.newValues.logo;
                }
            });
    }

    $scope.deletemode = false;

    $scope.enterDeleteMode = function () {
        $scope.deletemode = true;
    }

    $scope.commitDeleteMode = function () {
        var newResource = {
            Type: $scope.gallery.Type,
            Id: $scope.gallery.Id,
            Version: $scope.gallery.Version,
        };

        $pyx.gallery.delete(newResource)
            .success(function () {
                wvAlerts.success('Gallery \'' + $scope.gallery.Metadata.Name + '\' was deleted successfully', $scope);
                $location.path( 'User/' + $pyx.user.id);
            })
            .error(function (error) {
                wvAlerts.error('Failed to delete gallery', $scope);
            });
    }

    $scope.cancelDeleteMode = function () {
        $scope.deletemode = false;
    }
});