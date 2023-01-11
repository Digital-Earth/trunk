/*
worldViewStudioBootstrap to load an initialize WorldView studio
*/
app.service("worldViewStudioBootstrap", function (
    $location, $timeout, $window,
    $pyx, $pyxconfig, $pyxIntercom, worldViewStudioConfig, 
    styleOptions, analytics, dispatcher,

    featureNotifications,
    featureAboutDialog,
    featureFeedback,
    featureAuthentication,
    featureNetworkCheck,

    featureCurrentMap,
    featureGoto,
    featureEditResources,
    featurePropertiesWindow,
    featureMyConnections,
    featureSearch,
    featureImport,
    featureLibrary,
    featureDashboard,
    featureStyle,
    featureSelectionTools,
    featureFirstUse,
    featureHelpCenter,
    featureGlobeHotKeys,

    //experimental features
    featureLocalPublish,
    featureFps,
    featureMosaic,
    featureCopyAndPaste

    ) {

    return function ($scope, bootOptions) {
        bootOptions = bootOptions || {};
        bootOptions.mode = bootOptions.mode || 'default';

        // we need a way to check whether we are in the studio or gallery for analytics 
        analytics.setContext("studio");

        //use suggested backend from the front-end website if provided
        if (window.pyxisBackendUrl) {
            var backendUrl = window.pyxisBackendUrl.replace("/api/v1", "");
            $pyxconfig.backendUrl = backendUrl;
            $pyxconfig.baseUrl = $pyxconfig.backendUrl + "/api/v1";
        }
        
        // Project specific Id for Intercom
        var intercomId = "cnp6v6cw";

        // When a user is running WorldView in development mode - switch Intercom Id
        if ($pyxconfig.backendUrl !== "https://api.pyxis.worldview.gallery") {
            intercomId = "v68e9p8e";
        }

        //origin: resourceResolver
        dispatcher.registerAction("resourceResolvedCompleted");
        dispatcher.registerAction("resourceResolvedFailed");

        dispatcher.registerAction("resourceSpecificationResolvedCompleted");
        dispatcher.registerAction("resourceSpecificationResolvedFailed");

        //origin: selectionViewer
        dispatcher.registerAction("hideSelectionCompleted");
        dispatcher.registerAction("hideSelectionFailed");

        dispatcher.registerAction("viewSelectionCompleted");
        dispatcher.registerAction("viewSelectionFailed");

        //origin: globeService
        dispatcher.registerAction("updateGeoSourceVisualizationCompleted");
        dispatcher.registerAction("updateGeoSourceVisualizationFailed");

        //origin: ui
        dispatcher.registerAction("changeMap");
        dispatcher.registerAction("changeSelection");

        dispatcher.registerAction("showItem");
        dispatcher.registerAction("hideItem");
        dispatcher.registerAction("updateItemStyle");
        dispatcher.registerAction("calculateItemStyle");

        //stores
        dispatcher.registerStore("currentMapStore");
        dispatcher.registerStore("resourcesStore");
        dispatcher.registerStore("selectionStore");
        dispatcher.registerStore("searchTagsStore");

        //services
        dispatcher.registerService("selectionViewer");
        dispatcher.registerService("resourceResolver");
        dispatcher.registerService("globeService");

        // Intercom
        $pyxIntercom.registerAppId(intercomId);

        /* 
         - @name demoUserModal
         - @desc allows a developer to easily test modal dialogs using the console
         - @param {string} modal - $scope property for modal" 'login.local' or 'showTerms'
         - @param {boolean/string} - value to show or hide modal: true, false, 'login', etc
         - @type {function}
         -
        */
        $window.demoUserModal = function (modal, state) {
            var modalString = modal.split(".");

            if (modalString[0] === "login") {
                $scope.login.local = state;
            } else if (modalString[0] === "userInitiation") {
                $scope.userInitiation[modalString[1]] = state;
            } else {
                $scope[modal] = state;
            }

            $scope.$apply();
        }

        /* 
        - LocalStorage Items 
        - 'userInitiationMode' - @type {string} register | login | verify
        - 'postRedirect' - @type {boolean}
        - 'startUpModal' - @type {string} register | login
        - 'gettingStarted' - @type {string} show | hide
        */

        $scope.styleOptions = styleOptions;

        $scope.activeFeatures = {
            dragAndDrop: true,
            commandLineHandling: true,
            publishResources: true,
            updateResources: true,
            publishMaps: true,
            allowLocalPublish: false,
            refinePaletteOnImport: false,
            copyAndPaste: true
        };

        var devParameter = $location.search()["dev"];
        $scope.devParameters = devParameter ? devParameter.split(",") : [];
      
        //Use this to setup a background theme
        $scope.theme = {
            url: "" //for example: '/Contents/Images/Studio/Themes/calgary2.jpg'
        }

        $scope.uiActive = false;

        $scope.library = { Maps: [], Subscriptions: [] };

        $scope.applicationVersion = "";

        //extending $pyx to handle spec
        //thinking about moving this to $pyx?
        $pyx.spec = function (spec) {
            spec = spec || { Fields: [] };
            return {
                field: function (name) {
                    for (var i = 0; i < spec.Fields.length; i++) {
                        if (spec.Fields[i].Name === name) {
                            return spec.Fields[i];
                        }
                    }
                    return undefined;
                },
                unitNameOfField: function (name) {
                    var fieldSpec = this.field(name);
                    if (fieldSpec && fieldSpec.FieldUnit) {
                        return fieldSpec.FieldUnit.Name;
                    }
                    return "";
                },
                starredFields: function () {
                    var result = [];
                    for (var i = 0; i < spec.Fields.length; i++) {
                        if ($pyx.tags.itemSystemTags(spec.Fields[i]).exists($pyx.tags.ui.Favorite)) {
                            result.push(spec.Fields[i].Name);
                        }
                    }
                    return result;
                }
            };
        };

        $scope.templates = {}

        $scope.addTemplate = function (section, url) {
            if (!(section in $scope.templates)) {
                $scope.templates[section] = [];
            }
            if ($scope.templates[section].indexOf(url) === -1) {
                $scope.templates[section].push(url);
            }
        }

        featureNotifications.register($scope);
        featureAboutDialog.register($scope);
        featureFeedback.register($scope);
        featureNetworkCheck.register($scope);

        featureCurrentMap.register($scope);
        featureGoto.register($scope);
        featureEditResources.register($scope);
        featurePropertiesWindow.register($scope);
        featureMyConnections.register($scope);
        featureSearch.register($scope);
        featureImport.register($scope);
        featureLibrary.register($scope);
        featureDashboard.register($scope);
        featureStyle.register($scope);
        featureSelectionTools.register($scope);
        
        featureHelpCenter.register($scope);

        featureGlobeHotKeys.register($scope);
        featureCopyAndPaste.register($scope);

        switch (bootOptions.mode) {
            case 'demo':
                featureAuthentication.register($scope, { forceLogin: false });
                break;

            case 'default':
            default:
                featureFirstUse.register($scope);
                featureAuthentication.register($scope);
                $scope.$on('pyx-engine-ready', function () {
                    $scope.uiActive = true;
                });
                break;
        }
        

        $scope.$on("studio-setup-started", function () {
            $pyx.globe.ready(function () {

                //place holders for click and rightClick
                $scope.handleGlobeClick = function () { };
                $scope.handleGlobeRightClick = function () { };

                $pyx.globe.click(function (index, e) {
                    $timeout(function () { $scope.handleGlobeClick(index, e) });
                });

                $pyx.globe.rightClick(function (index, e) {
                    $timeout(function () { $scope.handleGlobeRightClick(index, e) });
                });

                $scope.globeKeyUp = function ($event) {
                    $scope.$emit("pyx-globe-keyup", { $event: $event });
                }

                //clear globe on refresh
                $pyx.globe.getLayers().success(function (ids) {
                    angular.forEach(ids, function (id) {
                        $pyx.globe.hide(id);
                    });
                });

                //allow features to register when $pyx.globe is ready
                $scope.$emit("pyx-globe-ready");

                $pyx.engine.ready(function () {
                    $timeout(function () {

                        //allow features to register when $pyx.engine is ready
                        $scope.$emit("pyx-engine-ready");                       

                        if (!("getSpecification" in $pyx.engine)) {
                            $pyx.engine.getSpecification = $pyx.engine.getDefinition;
                        }

                        if (!("publishLocally" in $pyx.engine)) {
                            $scope.activeFeatures.allowLocalPublish = false;
                        }

                        if ($scope.activeFeatures.allowLocalPublish) {
                            featureLocalPublish.register($scope);
                        }

                        if (!("publishGeoSource" in $pyx.engine)) {
                            $scope.activeFeatures.publishResources = false;
                            $scope.activeFeatures.updateResources = false;
                        }

                        if (!("publishMap" in $pyx.engine)) {
                            $scope.activeFeatures.publishMaps = false;
                        }

                        $scope.experimentalFeatures = {};
                        if ($scope.devParameters.indexOf("experimental") > -1) {
                            // include experimentalFeatures here
                            if ("mosaic" in $pyx.engine) {
                                featureMosaic.register($scope);
                                $scope.experimentalFeatures.mosaic = true;
                            }
                        }
                        angular.extend($scope.activeFeatures, $scope.experimentalFeatures);
                    });
                });

                if ($scope.devParameters.indexOf("fps") > -1 && "renderReport" in $pyx.globe) {
                    featureFps.register($scope);
                }

                //enable map items loading progress if enabled
                if ("isVisibleIdLoading" in $pyx.globe) {
                    function updateGlobeLoadingStatus() {
                        if ($scope.currentMap) {
                            $scope.currentMap.updateLoadingState();
                        }
                        $timeout(updateGlobeLoadingStatus, worldViewStudioConfig.globe.loadingStatusRefreshRate);
                    }

                    updateGlobeLoadingStatus();
                }
            });
        });
    }
});
