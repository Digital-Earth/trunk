app.service("featureEditResources", function ($pyx, $filter, $timeout, $q, $pyxIntercom, worldViewStudioConfig, geoSourceCache, dispatcher) {
    var i18n = $filter("i18n");

    function register($scope) {
        $scope.addTemplate('dialogs', '/template/feature-edit-resources.html');

        $scope.cameraCapture = {
            active: false,
            cancel: function () {
                $scope.cameraCapture.active = false;
            },
            save: function () {
                $scope.cameraCapture.active = false;
                if ($scope.cameraCapture.onSave) {
                    $scope.cameraCapture.onSave();
                }
            }
        };

        $scope.editResource = function (item) {
            //hide properties window
            $scope.hidePropertiesWindow();

            var itemCopy = angular.copy(item);

            var model = {
                step: "edit",
                item: itemCopy,
                active: true,
                isFeaturesOutput: false,
                isOwner: false,
                isEmailConfirmed: false,
                needToUpdate: function () { return false; },
                needGetLatest: function () { return false; },
                isLocalResource: !(itemCopy.Metadata.Providers && itemCopy.Metadata.Providers.length),
                commit: function () {
                    if (itemCopy.Type === "GeoSource") {
                        //item is map item. which mean it only have simple Metadata/Specification/ResourceReference
                        item.Metadata = itemCopy.Metadata;
                        item.Specification = itemCopy.Specification;
                        //update version in the resource reference...
                        item.Resource.Version = itemCopy.Version;
                        //update if the coverage should render as a texture
                        item.Style.ShowAsElevation = itemCopy.Style.ShowAsElevation;

                    } else if (itemCopy.Type === "Map") {
                        //map object is a complete Resource object with Id and Version.
                        item.Metadata = itemCopy.Metadata;
                        itemCopy.Camera.cameraChanged = item.Camera.cameraChanged;
                        item.Camera = itemCopy.Camera;
                        //update id & version of the map
                        item.Id = itemCopy.Id;
                        item.Version = itemCopy.Version;
                    }

                    if (model.step === "edit") {
                        model.close();
                    }

                    $scope.currentMap.sanitize();
                    $scope.notifyLibraryChange();
                },
                close: function () {
                    model.active = false;
                },
                cancel: function () {
                    model.active = false;
                },
                starred: function (field) {
                    return $pyx.tags.itemSystemTags(field).exists($pyx.tags.ui.Favorite);
                },
                toggleStarred: function (field) {
                    $pyx.tags.itemSystemTags(field).toggle($pyx.tags.ui.Favorite);
                },
                searchable: function (field) {
                    return $pyx.tags.itemSystemTags(field).exists($pyx.tags.ui.Searchable);
                },
                toggleSearchable: function (field) {
                    $pyx.tags.itemSystemTags(field).toggle($pyx.tags.ui.Searchable);
                },
                geotagable: function (field) {
                    return $pyx.tags.itemSystemTags(field).exists($pyx.tags.ui.Geotagable);
                },
                toggleGeotagable: function (field) {
                    $pyx.tags.itemSystemTags(field).toggle($pyx.tags.ui.Geotagable);
                },
                startImageCapture: function () {
                    $scope.screenCapture.active = true;
                    $scope.screenCapture.onCapture = function (url) {
                        if (!itemCopy.Metadata.ExternalUrls) {
                            itemCopy.Metadata.ExternalUrls = [];
                        } else {
                            //remove all image external urls
                            itemCopy.Metadata.ExternalUrls = $pyx.array.where(
                                itemCopy.Metadata.ExternalUrls,
                                function (item) {
                                    return item.Type !== "Image";
                                });
                        }
                        itemCopy.Metadata.ExternalUrls.push({ Url: url, 'Type': "Image" });
                        delete $scope.screenCapture.onCapture;
                    };
                },
                startCameraCapture: function () {
                    if (itemCopy.Camera && itemCopy.Camera.Range) {
                        $scope.gotoMap(itemCopy, worldViewStudioConfig.globe.captureCameraAnimationDuration);
                    }
                    $scope.cameraCapture.active = true;
                    $scope.cameraCapture.onSave = function () {
                        itemCopy.Camera = $pyx.globe.getCamera();
                        delete $scope.cameraCapture.onSave;
                    };
                    item.Camera.cameraChanged = true;
                },
                // update model errors before proceeding to publish
                updateErrors: function () {
                    model.errors = {};
                    if (itemCopy.Type === "Map") {
                        if (model.localGeoSources.length) {
                            model.errors.localGeoSources = true;
                        }

                        if (!itemCopy.Camera) {
                            model.errors.camera = true;
                        }
                    }

                    if (!itemCopy.Metadata.Name) {
                        model.errors.name = true;
                    }

                    if (!itemCopy.Metadata.Description) {
                        model.errors.description = true;
                    }

                    if (!itemCopy.Metadata.Tags || itemCopy.Metadata.Tags.length < 2 || itemCopy.Metadata.Tags.length > 5) {
                        model.errors.tags = true;
                    }

                    if (!itemCopy.Metadata.ExternalUrls || $pyx.array.firstIndex(itemCopy.Metadata.ExternalUrls, "Type", "Image") === -1) {
                        model.errors.image = true;
                    }
                },
                updateConfirm: function () {
                    this.updateErrors();

                    if (Object.keys(model.errors).length) {
                        model.step = "errorNotice";
                    } else {

                        model.changesSummary = {};

                        var oldTags = $pyx.tags.itemTags(model.publishedVersion);
                        var newTags = $pyx.tags.itemTags(itemCopy);

                        model.changesSummary.tagsRemoved = $pyx.array.where(oldTags.all(), function (tag) { return !newTags.exists(tag); });
                        model.changesSummary.tagsAdded = $pyx.array.where(newTags.all(), function (tag) { return !oldTags.exists(tag); });
                        model.changesSummary.tagsChanged = model.changesSummary.tagsRemoved.length || model.changesSummary.tagsAdded.length;

                        if (model.publishedVersion.Type === "GeoSource") {
                            model.changesSummary.specChanges = [];

                            angular.forEach(itemCopy.Specification.Fields, function (newSpec) {
                                var oldSpec = $pyx.spec(model.publishedVersion.Specification).field(newSpec.Name);

                                if (oldSpec) {
                                    if (newSpec.Metadata.Name !== oldSpec.Metadata.Name ||
                                        newSpec.Metadata.Description !== oldSpec.Metadata.Description ||
                                        !angular.equals(newSpec.Metadata.SystemTags, oldSpec.Metadata.SystemTags)) {
                                        model.changesSummary.specChanges.push({
                                            newSpec: newSpec,
                                            oldSpec: oldSpec
                                        });
                                    }
                                } else {
                                    model.changesSummary.specChanges.push({
                                        newSpec: newSpec,
                                        oldSpec: undefined
                                    });
                                }
                            });
                        }

                        model.changesSummary.imageChanged = $pyx.array.where(itemCopy.Metadata.ExternalUrls, function (url) {
                            return (url.Type === "Image" && url.Url.indexOf("asset://") === 0);
                        }).length !== 0;

                        if (model.publishedVersion.Type === "Map") {
                            model.changesSummary.cameraChanged = angular.copy(item.Camera.cameraChanged);
                        }

                        model.step = "publishSummary";
                    }
                },
                commitMetadataUpdate: function () {
                    this.commit();
                    this.close();

                    var metadataChanges = angular.copy({
                        Type: itemCopy.Type,
                        Id: itemCopy.Id,
                        Version: itemCopy.Version,
                        Metadata: {
                            Name: itemCopy.Metadata.Name,
                            Description: itemCopy.Metadata.Description,
                            Tags: itemCopy.Metadata.Tags,
                            ExternalUrls: itemCopy.Metadata.ExternalUrls
                        }
                    });
                    if (model.publishedVersion.Type === "GeoSource") {
                        metadataChanges.Specification = angular.copy(itemCopy.Specification);
                    }
                    if (model.publishedVersion.Type === "Map") {
                        metadataChanges.Camera = angular.copy(itemCopy.Camera);
                    }

                    var updateImagePromise;
                    var imageIndex = $pyx.array.firstIndex(metadataChanges.Metadata.ExternalUrls, function (external) {
                        return (external.Type === "Image" && external.Url.indexOf("asset://") === 0);
                    });
                    if (imageIndex !== -1) {
                        updateImagePromise = $pyx.when(
                            $pyx.application.resourceToDataUrl(metadataChanges.Metadata.ExternalUrls[imageIndex].Url))
                            .then(function (url) {
                                return $pyx(metadataChanges).updateImage(url);
                            });
                    } else {
                        updateImagePromise = $q.when(metadataChanges);
                    }

                    updateImagePromise.then(function (metadataChanges) {
                        $pyx.gallery.update(metadataChanges).success(function () {
                            $scope.notifyInfo(i18n("Resource changes have been published"));
                            $pyx(metadataChanges).refresh().success(function (resource) {
                                if (model.publishedVersion.Type === "GeoSource") {
                                    geoSourceCache.set(resource);
                                }
                                item.Resource = {
                                    Type: model.publishedVersion.Type,
                                    Id: resource.Id,
                                    Version: resource.Version
                                };
                                delete item.Version;
                                delete item.Id;
                                item.Metadata = resource.Metadata;
                                if (model.publishedVersion.Type === "GeoSource") {
                                    item.Specification = resource.Specification;
                                }
                                if (model.publishedVersion.Type === "Map") {
                                    item.Camera = resource.Camera;
                                }
                            });
                        }).error(function (error) {
                            $scope.notifyError(i18n("Failed to publish changes"));
                        });
                    }, function (error) {
                        $scope.notifyError(i18n("Failed to upload image"));
                    });
                },
                withinStorageQuota: function () {
                    if (itemCopy.Type !== "GeoSource") {
                        return true;
                    }
                    // not within the quota if storage or quota is undefined
                    if (model.storage === undefined || model.storage.required == undefined || model.storage.quota == undefined) {
                        return false;
                    }
                    return (model.storage.used + model.storage.required < model.storage.quota);
                },
                emailConfirmed: function () {
                    return model.isEmailConfirmed;
                },
                startPublish: function () {
                    this.updateErrors();

                    if (Object.keys(model.errors).length) {
                        model.step = "errorNotice";
                    } else {
                        model.selectedGallery = null;
                        $pyx.user.profile().success(function (profile) {
                            $pyx(profile).galleries().getAll().success(function (galleries) {
                                model.userGalleries = galleries;
                            }).error(function (/*error*/) {
                                // no response to the request; alert the failure
                                $scope.alertNetworkServiceState('featureEditResources', false, i18n("Failed to connect to WorldView Gallery. Publishing is unavailable"));
                            });
                        });

                        model.step = "selectGallery";
                    }
                },
                cancelPublish: function () {
                    model.step = "edit";
                },
                selectGallery: function (gallery) {
                    model.selectedGallery = gallery;
                },
                commitSelectGallery: function () {
                    model.step = "publishTerms";
                },
                commitPublishTerms: function () {
                    var self = this;
                    model.step = "publishProgress";
                    model.publishProgress = {
                        state: "publishing"
                    };
                    if (itemCopy.Type === "Map") {
                        $pyxIntercom.track('publish-map');

                        $pyx.when($pyx.engine.publishMap(itemCopy, model.selectedGallery)).then(function (publishedMap) {
                            model.publishProgress.state = "published";
                            itemCopy = publishedMap;
                            model.item = itemCopy;
                            model.isLocalResource = false;
                            self.commit();
                            model.step = "edit";
                        }, function (error) {
                            model.publishProgress.state = error || "failed";
                        });
                    } else if (itemCopy.Type === "GeoSource") {
                        $pyxIntercom.track('publish-geosource');

                        geoSourceCache.get(itemCopy.Id).then(function (geoSource) {
                            var fixedGeoSource = angular.copy(geoSource);

                            fixedGeoSource.Metadata.Name = itemCopy.Metadata.Name;
                            fixedGeoSource.Metadata.Description = itemCopy.Metadata.Description;
                            fixedGeoSource.Metadata.Tags = itemCopy.Metadata.Tags;
                            fixedGeoSource.Metadata.ExternalUrls = itemCopy.Metadata.ExternalUrls;
                            fixedGeoSource.Specification = itemCopy.Specification;

                            if (("startPublishGeoSource" in $pyx.engine) && ("finishPublishGeoSource" in $pyx.engine)) {
                                $pyx.when($pyx.engine.startPublishGeoSource(fixedGeoSource, item.Style, model.selectedGallery)).then(function (status) {
                                    model.publishProgress.geoSource = fixedGeoSource;
                                    model.publishProgress.state = status.Stage;
                                    model.publishProgress.progress = status.Progress;

                                    var donePublishing = false;
                                    function updateStatus() {
                                        if (!donePublishing) {
                                            var status = $pyx.engine.getPublishGeoSourceStatus(fixedGeoSource);

                                            model.publishProgress.geoSource = fixedGeoSource;
                                            model.publishProgress.state = status.Stage;
                                            model.publishProgress.progress = status.Progress;

                                            $timeout(updateStatus, 500);
                                        }
                                    }

                                    $timeout(updateStatus, 500);

                                    $pyx.when($pyx.engine.finishPublishGeoSource(fixedGeoSource)).then(function (publishedGeoSource) {
                                        donePublishing = true;
                                        model.publishProgress.state = "published";

                                        geoSourceCache.set(publishedGeoSource);
                                        dispatcher.actions.resourceResolvedCompleted.safeInvoke({ 'resource': publishedGeoSource });
                                        itemCopy.Metadata = publishedGeoSource.Metadata;
                                        itemCopy.Specification = publishedGeoSource.Specification;
                                        self.commit();
                                        model.isLocalResource = false;
                                        model.step = "edit";
                                    }, function (error) {
                                        donePublishing = true;
                                        model.publishProgress.state = error || "failed";
                                    });

                                }, function (error) {
                                    model.publishProgress.state = error || "failed";
                                });
                            } else {
                                $pyx.when($pyx.engine.publishGeoSource(fixedGeoSource, item.Style, model.selectedGallery)).then(function (publishedGeoSource) {
                                    model.publishProgress.state = "published";

                                    geoSourceCache.set(publishedGeoSource);
                                    itemCopy.Metadata = publishedGeoSource.Metadata;
                                    itemCopy.Specification = publishedGeoSource.Specification;
                                    self.commit();
                                    model.isLocalResource = false;
                                    model.step = "edit";
                                }, function (error) {
                                    model.publishProgress.state = error || "failed";
                                });
                            }
                        });
                    }
                }
            };

            $scope.editResourceModel = model;

            if (!itemCopy.Metadata.Tags) {
                itemCopy.Metadata.Tags = [];
            }

            if (!itemCopy.Type) {
                itemCopy.Type = item.Resource.Type || "GeoSource";
            }
            if (item.Resource) {
                itemCopy.Id = item.Resource.Id;
                itemCopy.Version = item.Resource.Version;
            } else {
                itemCopy.Id = item.Id;
                itemCopy.Version = item.Version;
            }

            function setDownloadUploadFlagHandlers(resource) {
                if (resource.Version === itemCopy.Version) {
                    if (model.isOwner) {
                        model.publishedVersion = resource;

                        model.needToUpdate = function () {
                            var result = !(
                                itemCopy.Metadata.Name === resource.Metadata.Name &&
                                itemCopy.Metadata.Description === resource.Metadata.Description &&
                                angular.equals(itemCopy.Metadata.Tags, resource.Metadata.Tags) &&
                                angular.equals(itemCopy.Metadata.ExternalUrls, resource.Metadata.ExternalUrls));
                            if (!result && itemCopy.Type === "Map") {
                                result = angular.copy(item.Camera.cameraChanged);
                            }
                            if (!result && itemCopy.Type === "GeoSource") {
                                result = !($pyx.array.equals(itemCopy.Specification.Fields, resource.Specification.Fields, function (a, b) {
                                    return a.Name === b.Name &&
                                        a.Metadata.Name === b.Metadata.Name &&
                                        a.Metadata.Description === b.Metadata.Description &&
                                        (
                                            (a.FieldUnit == null && b.FieldUnit == null) ||
                                            (a.FieldUnit != null && b.FieldUnit != null && a.FieldUnit.Name === b.FieldUnit.Name)
                                        );
                                }));
                            }
                            return result;
                        };
                    }
                } else {
                    model.needGetLatest = function () { return true; };
                    model.getLatest = function () {
                        itemCopy.Metadata = resource.Metadata;
                        itemCopy.Version = resource.Version;
                        if (itemCopy.Type === "GeoSource") {
                            if (resource.Specification) {
                                itemCopy.Specification = angular.copy(resource.Specification);
                            }
                        }
                        if (itemCopy.Type === "Map") {
                            itemCopy.Camera = resource.Camera;
                            item.Camera.cameraChanged = undefined;
                            cameraController = false;
                        }
                        model.needGetLatest = function () { return false; };
                    };
                }
            }

            if (itemCopy.Type === "GeoSource") {
                model.type = "GeoSource";

                function getStorage() {
                    model.storage = { used: 0 };
                    geoSourceCache.get(itemCopy.Id).then(function (geoSource) {
                        $pyx.engine.getDataSize(geoSource).success(function (dataSize) {
                            $timeout(function () {
                                model.storage.required = dataSize;
                                model.storage.requiredString = $filter("datasize")(dataSize, function () { return true; });
                            });
                        });
                    });
                    $pyx.user.storage().success(function (data) {
                        model.storage.used = parseInt(data);
                        $pyx.user.quota().success(function (data) {
                            model.storage.quota = parseInt(data);
                            model.storage.usedPercent = 100 * (model.storage.used / model.storage.quota);
                            model.storage.newPercent = 100 * (model.storage.required / model.storage.quota);
                        });
                    })
                    .error(function () {
                        // no response to the request; alert the failure
                        $scope.alertNetworkServiceState('featureEditResources', false, i18n("Failed to connect to WorldView Gallery. Publishing is unavailable"));
                    });
                }

                $pyx.user.profile().success(function (profile) {
                    if (itemCopy.Metadata.User) {
                        model.isOwner = profile.Id === itemCopy.Metadata.User.Id;
                    } else {
                        model.isOwner = true;
                    }

                    if (model.isOwner && model.isLocalResource) {
                        getStorage();
                    }

                    model.isEmailConfirmed = profile.EmailConfirmed;

                    var geoSourcePromise;
                    if (!model.isLocalResource) {
                        geoSourcePromise = $pyx.gallery.geoSources().getById(itemCopy.Id).then(function (result) { return result.data; });
                    } else {
                        geoSourcePromise = geoSourceCache.get(itemCopy.Id);
                    }

                    geoSourcePromise.then(function (geoSource) {
                        if (!model.isLocalResource) {
                            setDownloadUploadFlagHandlers(geoSource);
                        }

                        if (model.isOwner) {
                            var statusRefreshTimes = [1000, 1000, 1000, 1000, 1000, 2000, 3000, 4000, 5000, 5000, 10000];
                            model.statusRefreshTime = 0;

                            function updateStatus() {
                                $pyx(geoSource).status().then(function (result) {
                                    if (!angular.equals(model.geoSourceStatus, result.data)) {
                                        model.geoSourceStatus = result.data;
                                        model.statusRefreshTime = 0;
                                    } else if (model.statusRefreshTime < statusRefreshTimes.length - 1) {
                                        model.statusRefreshTime++;
                                    }
                                }).finally(function () {
                                    if (model.active) {
                                        $timeout(updateStatus, statusRefreshTimes[model.statusRefreshTime]);
                                    }
                                });
                            }

                            updateStatus();
                        }
                    });
                });

                if (!itemCopy.Specification) {
                    $scope.currentMap.getDefinition(item).then(function (definition) {
                        itemCopy.Specification = angular.copy(definition);
                        model.isFeaturesOutput = itemCopy.Specification.OutputType === "Feature";
                    });
                } else {
                    model.isFeaturesOutput = itemCopy.Specification.OutputType === "Feature";
                }
            } else if (itemCopy.Type === "Map") {
                model.type = "Map";

                $pyx.user.profile().success(function (profile) {
                    model.isEmailConfirmed = profile.EmailConfirmed;
                    model.isOwner = itemCopy.Metadata.User ? profile.Id === itemCopy.Metadata.User.Id : true;

                    model.localGeoSources = [];
                    model.publishedGeoSources = [];

                    angular.forEach(itemCopy.Groups, function (group) {
                        angular.forEach(group.Items, function (item) {
                            var fakeResource = {
                                Id: item.Resource.Id,
                                Type: item.Resource.Type,
                                Version: item.Resource.Version,
                                Metadata: item.Metadata
                            };
                            if (fakeResource.Metadata.Providers && fakeResource.Metadata.Providers.length) {
                                model.publishedGeoSources.push(fakeResource);
                            } else {
                                model.localGeoSources.push(fakeResource);
                            }
                        });
                    });

                    if (model.localGeoSources.length === 0 && model.isEmailConfirmed) {
                        model.canPublish = function () { return true; };
                    } else {
                        model.canPublish = function () { return false; };
                    }

                    if (!model.isLocalResource) {
                        $pyx.gallery.maps().getById(itemCopy.Id).then(function (result) {
                            if (result.data && result.data.Id === itemCopy.Id) {
                                setDownloadUploadFlagHandlers(result.data);
                            }
                        }, function (/*error*/) {
                            // no response to the request; alert the failure
                            $scope.alertNetworkServiceState('featureEditResources', false, i18n("Failed to connect to WorldView Gallery. Publishing is unavailable"));
                        });
                    }
                }).error(function (/*error*/) {
                    // no response to the request; alert the failure
                    $scope.alertNetworkServiceState('featureEditResources', false, i18n("Failed to connect to WorldView Gallery. Publishing is unavailable"));
                });
            }

            model.typelower = 'Map' ? 'globe' : model.type.toLowerCase();
        };

        //screen capture

        $scope.screenCapture = {
            active: false
        };

        $scope.$on("pyx-globe-ready", function () {
            if ("capture" in $pyx.globe) {
                $scope.screenCapture.ready = true;
                $scope.screenCapture.capture = function () {
                    if ($scope.screenCapture.onCapture) {
                        var elem = $("#capture");
                        var pos = elem.offset();
                        var width = elem.width();
                        var height = elem.height();
                        var borderWidth = 2;

                        $pyx.globe.capture({
                            Left: pos.left + borderWidth,
                            Top: pos.top + borderWidth,
                            Width: width,
                            Height: height
                        }).success(function (url) {
                            $timeout(function () {
                                $scope.screenCapture.onCapture(url);
                                $scope.screenCapture.active = false;
                            });
                        }).error(function (error) {
                            $timeout(function () {
                                $scope.notifyError(i18n("Screen capture failed with error : ") + error);
                                $scope.screenCapture.active = false;
                            });
                        });
                    }
                };
            }
        });
    };

    return {
        depends: ["notifications","currentMap"],
        register: register
    };
});