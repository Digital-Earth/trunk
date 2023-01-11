app.controller('worldviewSiteController', function (
    $rootScope, $scope, $location, $route, $timeout, $http,
    $pyx, $pyxconfig, globalCookieStore, analytics, siteParams, userAuthResolve, userPlatform) {
   
    //use suggested backend from the front-end website if provided
    if (window.pyxisBackendUrl) {
        var backendUrl = window.pyxisBackendUrl.replace('/api/v1', '');
        $pyxconfig.backendUrl = backendUrl;
        $pyxconfig.baseUrl = $pyxconfig.backendUrl + '/api/v1';
    }

    $scope.app = siteParams.getApp();
    $scope.downloadAppNeeded = siteParams.downloadAppNeeded();

    // Are users on a supported OS
    $scope.worldViewSupported = userPlatform.isSupported('WorldView');

    // Check details for customized messages. 
    $scope.userDevice = userPlatform.get('device');
    $scope.userOS = userPlatform.get('platform');
    $scope.userOSVersion = userPlatform.get('version');
    $scope.userBrowser = userPlatform.get('browser');

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

    $scope.searchDisabled = true;
    $scope.signInDisabled = false;

    $scope.search = $location.search()['search'] || "";
    $scope.$on('$routeChangeSuccess', function (event, currentRoute) {
        $scope.search = $location.search()['search'] || "";
        if (currentRoute.title) {
            $rootScope.title = currentRoute.title + ' | WorldView.Gallery';
        } else {
            $rootScope.title = 'PYXIS WorldView.Gallery';
        }
        if (currentRoute.description) {
            $rootScope.description = currentRoute.description;
        } else {
            $rootScope.description = "";
        }
        $scope.searchDisabled = !!currentRoute.searchDisabled;
        $scope.signInDisabled = !!currentRoute.signInDisabled;
    });

    var searchChangePromise = undefined;

    var updatePageUrl = function (value) {
        if (value !== undefined && value !== "") {
            if ($route.current.controller !== "worldviewSearchController") {
                $location.path('');
            }
            $location.search({ 'search': value });
        } else if ($route.current.controller === "worldviewSearchController") {
            $location.search({});
        }
    }

    $scope.$watch('search', function (newValue) {
        if (searchChangePromise) {
            $timeout.cancel(searchChangePromise);
        }
        searchChangePromise = $timeout(function () { updatePageUrl(newValue); }, 500);
    });
    
    $scope.$on('search-list-changed', function (e, suggestions) {
        $scope.searchListSuggestions = suggestions;
        e.stopPropagation();
    });

    var processProfile = function (profile) {
        $scope.profile = profile;        
        $scope.admin = $scope.profile.Roles && $scope.profile.Roles.length &&
            ($scope.profile.Roles[0] === "admin" || $scope.profile.Roles[0] === "siteAdmin");

        $pyx.user.admin = $scope.admin;
    }

    $scope.user = $pyx.user.username();
    if ($scope.user) {
        $pyx.user.profile()
            .success(processProfile)
            .error(function () {
                $scope.$emit('wv-alert', { 'message': 'Your session has expired, Please log in again.', type: 'info' });
                $pyx.user.logout();
            });
    }

    $scope.$on('pyx-user-login', function () {
        $scope.user = $pyx.user.username();
        if ($scope.user) {
            $pyx.user.profile().success(processProfile);
        }
    });

    $scope.$on('pyx-user-registered', function () {
        $scope.user = $pyx.user.username();
        if ($scope.user) {
            $pyx.user.profile().success(processProfile);
        }
    });

    $scope.$on('pyx-user-logout', function () {
        delete $pyx.user.admin;
        $scope.user = "";
        $scope.profile = undefined;
        $route.reload();
    });

    $scope.signOut = function () {
        $pyx.user.logout();
    }

    $scope.gotoUserPage = function () {
        $location.path('User/' + $pyx.user.id);
    }

    $scope.alert = { 'message': '', 'type': 'info', id: 0, show: false };

    $scope.$on('wv-alert', function (e, message) {
        $scope.alert.show = true;
        $scope.alert.message = message.message || '';
        $scope.alert.type = message.type || 'info';
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
        $http.post('/api/feedback', $scope.feedbackForm)
            .success(function () {
                $scope.$emit('wv-alert', { 'message': 'Thank you for your feedback.', type: 'success' });
            })
            .error(function () {
                $scope.$emit('wv-alert', { 'message': 'Sorry, we have problem sending the feedback to our servers.', type: 'error' });
            });
    }

    $scope.cancelFeedbackMode = function () {
        $scope.feedback = false;
    }

    function externalLoginUrl(provider) {
        var index = $pyx.array.firstIndex($scope.externalProviders, function (p) {
            return p.Name === provider;
        });
        return $pyxconfig.backendUrl + $scope.externalProviders[index].Url;
    }

    // if there is a return_to URL parameter, pass it along (single sign-on target)
    var redirect = $location.search()['return_to'] ? '?return_to=' + $location.search()['return_to'] : '';
    $pyx.user.externalLoginProviders(siteParams.galleryUrl(), '/signUp' + redirect)
        .success(function (data) {
            $scope.externalProviders = data;
        })
        .error(function () {
            $scope.$emit('wv-alert', { 'message': 'Failed to get social sign in providers.', type: 'error' });
        });

    $scope.signIn = false;
   
    $scope.doLogin = function () {
        $scope.login.message = "";
        $pyx.user.login($scope.login.username, $scope.login.password)
            .success(function () {
                $scope.signIn = false;
                if (userAuthResolve.requestUrl) {
                    $location.path(userAuthResolve.requestUrl);
                } else {
                    $route.reload();
                }
            })
            .error(function () {
                $scope.login.password = "";
                $scope.$emit('wv-alert', { 'message': 'Username or password are invalid', type: 'error' });                
            });
    }

    $scope.enterSignInMode = function () {
        $scope.signIn = true;
        $scope.login = { username: '', password: '' };
    }

    $scope.enterSignUpMode = function () {
        $scope.signIn = false;
        $location.path('/signUp');
    }

    $scope.enterForgotPasswordMode = function () {
        $scope.signIn = false;
        $location.path('/signUp');
    }

    $scope.cancelSignIn = function () {
        $scope.signIn = false;
    }

    $scope.socialSignIn = function (provider) {
        window.location = externalLoginUrl(provider);
    }
});
