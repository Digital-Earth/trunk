app.controller('worldviewDetailsController', function ($scope, $pyx, $pyxconfig, $routeParams, $location, $timeout, $q, $filter, fileUploader, imageServer, wvAlerts, resource, resourceStatus, siteParams, detectAndLaunchStudio) {
    var upgradeResourceType = $filter('upgradeResourceType');
    $scope.resource = resource;
    if ($pyx.user.auth()) {
        $pyx.user.profile().success(function (profile) {
            $scope.emailConfirmed = profile.EmailConfirmed;
        });
    }

    var index = $pyx.array.firstIndex(resource.Metadata.ExternalUrls, "Type", "Image");
    if (index != -1) {
        $scope.img = imageServer.fixImageUrl(resource.Metadata.ExternalUrls[index].Url, resource.Version);
    }

    $scope.comment = { body: "" };
    $scope.postError = "";
    $scope.replyNow = function () {
        if ($scope.comment.body) {
            $scope.postError = "";
            var resource = $pyx($scope.resource);
            resource.comment($scope.comment.body).success(function () {
                resource.refresh().success(function () {
                    $scope.resource = resource.json;
                });
                $scope.comment.body = "";
                wvAlerts.success('Comment Posted', $scope);
            }).error(function(error) {
                $scope.postError = error.error_description;
            });
        } else {
            $scope.postError = "empty message";
        }
    }

    $scope.getProviderImage = function (provider) {
        //we can't get provider.Version - so we are using a hack and use the $resource version instead.
        return imageServer.getAvatarThumbnailUrl(provider.Id, $scope.resource.Version);
    }

    $scope.owner = $pyx.user.id == resource.Metadata.User.Id;
    $scope.admin = $pyx.user.admin;
    $scope.canAgree = !resource.Licenses.length || $pyx.user.auth();
    $scope.hasAccess = false;

    //has the user already agreed to a resource's
    //license - this always 'resolves', even if a user
    //doesn't have access to a license - both use cases
    //are valid and handled by the 'success' callback
    var doesUserHaveLicense = function () {
        var deferred = $q.defer();

        if ($pyx.user.auth() && resource.Licenses.length) {
            $pyx.gallery.licensedAccess($scope.resource.Id).then(function(result) {
                // user has licensed access to the resource already
                if (result.data.HasAccess) {
                    $scope.hasAccess = true;
                    deferred.resolve('user-has-access');
                } else {
                    deferred.resolve('user-has-no-access');
                }
            });
        } else {
            deferred.resolve('user-has-no-access');
        }

        return deferred.promise;
    }
   


    $scope.statusRefreshTimes = [1000, 2000, 3000, 5000, 10000, 30000, 60000];
    $scope.statusRefreshTime = $scope.status != 'Published' ? 0 : $scope.statusRefreshTimes.length - 1;
    $scope.alive = true;
    $scope.$on('$destroy', function () {
        $scope.alive = false;
    });

    var updateStatus = function () {
        if (!$scope.alive) return;
        $pyx($scope.resource).status()
        .then(function (result) {
            if (!angular.equals($scope.status, result.data)) {
                $scope.status = result.data;
                $scope.canSubscribe = $scope.canAgree && ($scope.resource.State === "Active" || $scope.admin) && /Published/.test($scope.status.Status);
                $scope.statusRefreshTime = 0;
            } else if ($scope.statusRefreshTime < $scope.statusRefreshTimes.length - 1) {
                $scope.statusRefreshTime++;
            }
        })
        .finally(function () {
            $timeout(updateStatus, $scope.statusRefreshTimes[$scope.statusRefreshTime]);
        });
    }

    if ($scope.resource.Type === 'GeoSource') {
        $scope.status = resourceStatus;
        $scope.canSubscribe = $scope.canAgree && ($scope.resource.State === "Active" || $scope.admin) && /Published/.test($scope.status.Status);
        if ($scope.resource.State === "Broken") {
            wvAlerts.error('This GeoSource is currently undergoing maintenance', $scope);
        }

        if ($scope.owner) {
            $timeout(updateStatus, $scope.statusRefreshTimes[$scope.statusRefreshTime]);
        }
    } else if ($scope.resource.Type === 'Map') {
        $pyx($scope.resource).geoSources().getAll().success(function (layers) {
            $scope.layers = layers;
        });
        $scope.canSubscribe = true;
    }

    $scope.editmode = false;
    $scope.newValues = {};

    $scope.enterEditMode = function () {
        $scope.editmode = true;
        $scope.newValues.Name = $scope.resource.Metadata.Name;
        $scope.newValues.Description = $scope.resource.Metadata.Description;
        $scope.newValues.Tags = angular.copy($scope.resource.Metadata.Tags);
        $scope.newValues.Visibility = $scope.resource.Metadata.Visibility;
        if ($scope.resource.Type === 'GeoSource') {
            $scope.newValues.State = $scope.resource.State;
        }
    }

    $scope.cancelEditMode = function () {
        $scope.editmode = false;
    }

    $scope.commitEditMode = function () {
        var newResource = {
            Type: $scope.resource.Type,
            Id: $scope.resource.Id,
            Version: $scope.resource.Version,
            Metadata: {
                Name: $scope.newValues.Name,
                Description: $scope.newValues.Description,
                Tags: $scope.newValues.Tags,
                Visibility: $scope.newValues.Visibility
            }
        }
        if ($scope.resource.Type === 'GeoSource') {
            newResource.State = $scope.newValues.State;
        }

        $pyx.gallery.update(newResource)
            .success(function () {
                wvAlerts.success(upgradeResourceType($scope.resource.Type) + ' updated successfully', $scope);
                $pyx(resource).refresh().success(function (resource) {
                    $scope.editmode = false;
                    $scope.resource = resource;
                }).error(function (error) {
                    wvAlerts.error('Failed to fetch updated resource', $scope);
                });
            }).
            error(function () {
                wvAlerts.error('Failed to update ' + upgradeResourceType($scope.resource.Type), $scope);
            });
    }

    $scope.deletemode = false;

    $scope.enterDeleteMode = function () {
        $scope.deletemode = true;
    }

    $scope.commitDeleteMode = function () {
        var newResource = {
            Type: $scope.resource.Type,
            Id: $scope.resource.Id,
            Version: $scope.resource.Version,
            State: 'Removed'
        };
        
        $pyx.gallery.update(newResource)
            .success(function () {
                var provider = $scope.resource.Metadata.Providers[0];
                wvAlerts.success(upgradeResourceType($scope.resource.Type) + ' \'' + $scope.resource.Metadata.Name + '\' was deleted successfully', $scope);
                $location.path(provider.Type + '/' + provider.Id);
            })
            .error(function (error) {
                wvAlerts.error('Failed to delete ' + upgradeResourceType($scope.resource.Type), $scope);
            });
    }

    $scope.cancelDeleteMode = function () {
        $scope.deletemode = false;
    }

    $scope.newImageUploaded = false;
    $scope.newImageFile = undefined;

    $scope.uploadImage = function ($files) {
        $scope.newImageFile = $files[0];

        var ext = $scope.newImageFile.name.split('.').pop();
        var newName = $scope.resource.Id + "." + ext;
            
        var newResource = {
            Type: $scope.resource.Type,
            Id: $scope.resource.Id,
            Version: $scope.resource.Version,
            Metadata: {
                ExternalUrls: angular.copy($scope.resource.Metadata.ExternalUrls)
            }
        };

        var index = $pyx.array.firstIndex(newResource.Metadata.ExternalUrls, "Type", "Image");
        if (index !== -1) {
            newResource.Metadata.ExternalUrls.splice(index, 1);
        }
        newResource.Metadata.ExternalUrls.push({ Type: 'Image', url: imageServer.getImageUrl(newName) });


        fileUploader.readAsDataURL($scope.newImageFile)
        .then(function (dataUrl) {
            $scope.newImageUploaded = true;
            $scope.img = dataUrl;
            return fileUploader.upload(imageServer.setImageUrl(), $scope.newImageFile, newName)
                .catch(function () { return wvAlerts.error('Failed to upload ' + upgradeResourceType($scope.resource.Type) + ' image', $scope); });
        })
        .then(function(fileUpload) {
            return $pyx.gallery.update(newResource)
                .catch(function () { return wvAlerts.error('Failed to update ' + upgradeResourceType($scope.resource.Type), $scope); });
        })
        .then(function () {
            wvAlerts.success(upgradeResourceType($scope.resource.Type) + ' image updated successfully', $scope);
            return $pyx(resource).refresh()
                .catch(function () { return wvAlerts.error('Failed to fetch updated resource', $scope); });
        })
        .then(function(updatedResource) {
            $scope.resource = updatedResource.data;
        });
    }

    $scope.subscribe = false;
    $scope.downloadApp = false;
    $scope.downloaded = false;
    
    $scope.beginSubscribe = function () {
        if (!$scope.canSubscribe) {
            return wvAlerts.error('Can\'t subscribe to this ' + upgradeResourceType($scope.resource.Type) + ' at the moment');
        }
        $scope.subscribe = true;
    }

    $scope.commitSubscribe = function () {
        $scope.subscribe = false;
        if ($scope.resource.ProcRef) {
            window.open('pyxis://' + $scope.resource.ProcRef, "_self");

        } else {
            var baseUrl = $pyxconfig.baseUrl.replace('https://', 'pyxis://');
            window.open(baseUrl + '/' + $scope.resource.Type + "/" + $scope.resource.Id + "?Version=" + encodeURIComponent($scope.resource.Version), "_self");
        }
    }

    //rules for handleSubscribe     
    $scope.userRules = {};
    $scope.userRules.isAuthenticated = $pyx.user.auth();

    $scope.notSupportedSignedIn = false;

    $scope.exitNotSupportedMode = function () {
        $scope.notSupportedSignedIn = false;
    }

    $scope.userSignedIn = function () {
        var auth = $scope.userRules.isAuthenticated;
        return auth;
    }

    //address all user flows for 'View in WorldView'
    $scope.handleSubscribe = function () {
        
        //when Studio installed, but user isn't signed in
        var handleStudioExists = function () {
            $scope.enterSignInMode();
        }

        //when the Studio isn't installed - user is signed in or not
        var handleStudioVapour = function () {
            $scope.downloadApp = true;
            $scope.downloaded = false;
        }

        var hasDownloaded = function () {
            //confirm user has the 'downloaded' cookie
            //when they don't, try and launch Studio because  
            //they may have flushed browser history, etc
            if (siteParams.downloadAppNeeded()) {
                detectAndLaunchStudio.launchStudio().then($scope.beginSubscribe, handleStudioVapour);
            } else {
                $scope.beginSubscribe();
            }
        }

        var checkUserRules = function () {
            //update user rules
            $scope.userRules.canSubscribeNotAgreed = $scope.canSubscribe && !$scope.hasAccess;
            $scope.userRules.canSubscribeHasAgreed = $scope.canSubscribe && $scope.hasAccess;

            //user is on an OS that supports Studio
            if ($scope.worldViewSupported) {
                //user isn't signed in, but may have Studio installed - if they do
                //lets show them the sign in modal - if not - 
                //we display the download modal
                if (!$scope.userRules.isAuthenticated) {
                    if (siteParams.downloadAppNeeded()) {
                        detectAndLaunchStudio.launchStudio().then(handleStudioExists, handleStudioVapour);
                    } else {
                        handleStudioExists();
                    }
                //user is signed in and the resource is available to
                //view, but we don't know if they have Studio
                } else if ($scope.userRules.canSubscribeNotAgreed) {
                    hasDownloaded();
                //user already has agreed to license - we can assume they
                //have Studio, but they may have unistalled, so we can't let them Subscribe
                //without checking if the have the 'downloadApp'
                } else if ($scope.userRules.canSubscribeHasAgreed) {
                    if (siteParams.downloadAppNeeded()) {
                        detectAndLaunchStudio.launchStudio().then($scope.beginSubscribe, handleStudioVapour)
                    } else {
                        $scope.commitSubscribe();
                    }
                } else {
                    hasDownloaded();
                }
            //user isn't on a supported OS
            } else {
                //user isn't signed in - we assume they also haven't signed up
                //so we send them to the 'download' page that has a OS
                //unsupported explanation 
                if (!$scope.userRules.isAuthenticated) {
                    $location.path('/download');
                //user is signed in and still clicks on 'View in WorldView'
                //we trigger a modal to notify them that a web version will
                //be available soon.
                } else {
                    $scope.notSupportedSignedIn = true;
                }
            }
        }

        //check for any licenses first - then check the rest of the rules
        doesUserHaveLicense().then(checkUserRules).catch(checkUserRules);
    }

    $scope.cancelSubscribe = function () {
        $scope.subscribe = false;
    }

    $scope.startDownload = function () {
        $location.path('/download');
    }

    $scope.skipDownload = function () {
        $scope.downloadApp = false;
        siteParams.setAppDownloaded();
        $scope.beginSubscribe();
    }

    $scope.confirmDownload = function () {
        $scope.downloadApp = false;
        siteParams.setAppDownloaded();
        $scope.beginSubscribe();
    }
});