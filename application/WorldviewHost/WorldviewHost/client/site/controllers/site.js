app.controller("worldviewSiteController", function (
    $rootScope, $scope, $window, $location, $route, $timeout, $http, $injector, $pyx, $pyxconfig, 
    $pyxIntercom, globalCookieStore, analytics, siteParams, userAuthResolve, userPlatform, Modernizr) {
    
    //use suggested backend from the front-end website if provided
    if (window.pyxisBackendUrl) {
        var backendUrl = window.pyxisBackendUrl.replace("/api/v1", "");
        $pyxconfig.backendUrl = backendUrl;
        $pyxconfig.baseUrl = $pyxconfig.backendUrl + "/api/v1";
    }

    $scope.app = siteParams.getApp();
    $scope.downloadAppNeeded = siteParams.downloadAppNeeded();

    // Are users on a supported OS
    $scope.worldViewSupported = userPlatform.isSupported("WorldView");

    // Check details for customized messages. 
    $scope.userDevice = userPlatform.get("device");
    $scope.userOS = userPlatform.get("platform");
    $scope.userOSVersion = userPlatform.get("version");
    $scope.userBrowser = userPlatform.get("browser");

    // HTML5 and Mobile Feature support 
    $scope.supports = {};
    $scope.supports['webgl'] = Modernizr.webgl;
    $scope.supports['touch'] = Modernizr.touch;
    
    $scope.onSignInCallback = undefined;

    $scope.setSignIn = function (callback) {
        $scope.onSignInCallback = callback;
        
    }
    $scope.showSignIn = function () {
        return $scope.onSignInCallback;
    }

    $scope.doSignIn = function () {
        if ($scope.onSignInCallback) {
            $scope.onSignInCallback();
        }
    }

    $scope.enterSignInMode = function () {
        $scope.show.modal = 'sign-in';
    }

    $scope.searchDisabled = true;
    $scope.signInDisabled = false;

    $scope.search = $location.search()["search"] || "";
    $scope.$on("$routeChangeSuccess", function (event, currentRoute) {
        $scope.search = $location.search()["search"] || "";
        if (currentRoute.title) {
            $rootScope.title = currentRoute.title + " | WorldView.Gallery";
        } else {
            $rootScope.title = "PYXIS WorldView.Gallery";
        }
        if (currentRoute.description) {
            $rootScope.description = currentRoute.description;
        } else {
            $rootScope.description = "";
        }
        $scope.searchDisabled = !!currentRoute.searchDisabled;
        $scope.signInDisabled = !!currentRoute.signInDisabled;
        $scope.newSite = !!currentRoute.newSite;

        $pyxIntercom.updateFeed();
    });

    var searchChangePromise = undefined;

    var updatePageUrl = function (value) {
        if (value !== undefined && value !== "") {
            if ($route.current.controller !== "worldviewSearchController") {
                $location.path("/browse");
            }
            $location.search({ 'search': value });
        } else if ($route.current.controller === "worldviewSearchController") {
            $location.search({});
        }
    }

    $scope.$watch("search", function (newValue) {
        if (searchChangePromise) {
            $timeout.cancel(searchChangePromise);
        }
        searchChangePromise = $timeout(function () { updatePageUrl(newValue); }, 500);
    });
    
    $scope.$on("search-list-changed", function (e, suggestions) {
        $scope.searchListSuggestions = suggestions;
        e.stopPropagation();
    });

    $scope.$on("external-registration-submit", function (e, form) {
        if (e.hasOwnProperty("stopPropagation")) {
            e.stopPropagation();
            $scope.$broadcast("external-registration-submit", form);
        }
    });

    var processProfile = function (profile) {
        $scope.profile = profile;        
        $scope.admin = $scope.profile.Roles && $scope.profile.Roles.length &&
            ($scope.profile.Roles[0] === "admin" || $scope.profile.Roles[0] === "siteAdmin");

        $pyx.user.admin = $scope.admin;
        $pyxIntercom.boot($scope.profile);
    }

    $scope.user = $pyx.user.username();
    if ($scope.user) {
        $pyx.user.profile()
            .success(processProfile)
            .error(function () {
                $scope.$emit("wv-alert", { 'message': "Your session has expired, Please log in again.", type: "info" });
                $pyx.user.logout();
            });
    }

    $scope.$on("pyx-user-login", function () {
        $scope.user = $pyx.user.username();
        if ($scope.user) {
            $pyx.user.profile().success(processProfile);
            $scope.show.modal = "";
        }
    });

    $scope.$on("pyx-user-registered", function () {
        $scope.user = $pyx.user.username();
        if ($scope.user) {
            $pyx.user.profile().success(processProfile);
            $scope.show.modal = "";
        }
    });

    $scope.$on("pyx-user-logout", function () {
        delete $pyx.user.admin;
        $scope.user = "";
        $scope.profile = undefined;
        $pyxIntercom.boot();
        $route.reload();
    });

    $scope.signOut = function () {
        $pyx.user.logout();
        $location.path("/browse").search({});
    }

    $scope.gotoHomePage = function () {
        $location.path("/").search({});
    }

    $scope.gotoUserPage = function () {
        $location.path("User/" + $pyx.user.id);
    }

    $scope.alert = { 'message': "", 'type': "info", id: 0, show: false };

    $scope.$on("wv-alert", function (e, message) {
        $scope.alert.show = true;
        $scope.alert.message = message.message || "";
        $scope.alert.type = message.type || "info";
        var newId = $scope.alert.id + 1;
        $scope.alert.id = newId;
        $timeout(function () {
            if ($scope.alert.id === newId) {
                $scope.alert.show = false;
            }
        }, 5000);
    });


    $scope.feedback = false;
    $scope.feedbackForm = {
        Name: "",
        UserId: "",
        Email: "",
        Body: ""
    };
    $scope.enterFeedbackMode = function () {
        $scope.feedback = true;
        $scope.feedbackForm = {
            Name: $scope.profile.Metadata.Name,
            UserId: $scope.profile.Id,
            Email: $scope.profile.Email,
            Body: "",
            Url: $location.path()
        };
    }

    $scope.commitFeedbackMode = function () {
        $scope.feedback = false;
        $http.post("/api/feedback", $scope.feedbackForm)
            .success(function () {
                $scope.$emit("wv-alert", { 'message': "Thank you for your feedback.", type: "success" });
            })
            .error(function () {
                $scope.$emit("wv-alert", { 'message': "Sorry, we have problem sending the feedback to our servers.", type: "error" });
            });
    }

    $scope.cancelFeedbackMode = function () {
        $scope.feedback = false;
    }

    var $validationProvider = $injector.get('$validation');
    
    // Mobile 
    $scope.isMobile = function () {
        return $rootScope.mobileDevice;
    }
    // Page
    $scope.page = '';
    $scope.show = {};
    $scope.show.modal = '';

    // Fullscreen menu
    $scope.menu = {};
    $scope.menu.active = false;

    // Forms 
    $scope.form = {};
    $scope.form.names = {};
    $scope.form.names['register'] = 'registerForm';
    $scope.form.names['sign-in'] = 'signInForm';

    $scope.form.reset = function (form) {
        var modal = '';

        if (!form) {
            modal = $scope.show.modal;
   
            if (modal in $scope.form.names) {
                form = $scope.form.names[modal];
   
                }
    }

        if (form) {
            $validationProvider.reset(form);
    }
    };


    // Trigger a route change from ng-click or another custom event.
    // @param {String} path
    $scope.path = function (path) {
        $location.path(path);
    };

    // Open and close the fullscreen menu.
    // @param {String} toggleOn
    $scope.menu.toggle = function (toggleOn) {
        if (toggleOn === 'exit') {
            if (this.active) {
                this.active = false;
            }
        } else {
            if (this.active) {
                this.active = false;
            } else {
                this.active = true;
            }
    }
    };

    // Add a custom class to the 'body' based on route
    // @param {String} path
    var addClass = function (path) {
        var path = path.split('/');
        var root = path[1];

        $scope.page = root;

        if (root && root !== 'site') {
            $scope.page = [root, 'type-page'];
        } else {
            $scope.page = 'home';
    }

    };

    // When the route is updated or changed, add a new class to the 'body' element
    $rootScope.$on('$locationChangeSuccess', function () {
        addClass($location.path());
    });

    // On init add a class to the 'body' element
    addClass($location.path());

    // Reset scroll position on page 'reload'
    angular.element($window).on('beforeunload', function () {
        angular.element($window).scrollTop(0);
    });

    $scope.enterSignInMode = function () {
        if (arguments) {
            $rootScope.gaEvent.apply(this, arguments);
        }
        $scope.show.modal = 'sign-in';
    }
    
    $scope.enterSignUpMode = function () {
        if (arguments) {
            $rootScope.gaEvent.apply(this, arguments);
        }
        $scope.show.modal = 'register';
    }
    
    function initializeIntercom() {
        // Project specific Id for Intercom
        var intercomId = "cnp6v6cw";
        
        // switch Intercom Id for non-production environments
        if ($pyxconfig.backendUrl !== "https://api.pyxis.worldview.gallery") {
            intercomId = "v68e9p8e";
        }
        $pyxIntercom.registerAppId(intercomId);
        $pyxIntercom.boot();
        $scope.intercomEnabled = true;
    }

    initializeIntercom();
});
