app.service("userAuthResolve", function ($pyx, $q, $location, siteParams) {

    function resolveUser() {
        var deferred = $q.defer();
        if ($pyx.user.auth()) {
            if ($pyx.user.id) {
                deferred.resolve("auth");
            } else {
                $pyx.user.profile()
                    .success(function (profile) {
                        $pyx.user.id = profile.Id;
                        deferred.resolve("auth");
                    })
                    .error(function () {
                        deferred.reject("not auth");
                    });
            }
        } else {
            var token = siteParams.getToken();
            if (token) {
                $pyx.user.loginUsingToken(token)
                    .success(function (profile) {
                        $pyx.user.id = profile.Id;
                        deferred.resolve("auth");
                    })
                    .error(function () {
                        siteParams.clearToken();
                        deferred.reject("not auth");
                    });
            } else {
                deferred.reject("not auth");
            }
        }
        return deferred.promise;
    }

    return {
        requestUrl: "",
        auth: function () {
            var self = this;
            return resolveUser().then(
                function () {
                    self.requestUrl = "";
                    return "auth";
                },
                function () {
                    self.requestUrl = $location.path();
                    $location.path("info/access-denied").replace();
                    return $q.reject("not auth");
                });
        },
        userIfAvailable: function () {
            return resolveUser().then(
                function () {
                    return "auth";
                }, function () {
                    return "not auth";
                });
        },
        userProfileAndGalleries: function (id) {
            var deferred = $q.defer();
            var userProfilePromise;
            if (id) {
                userProfilePromise = $pyx.gallery.users().getById(id);
            } else {
                userProfilePromise = $pyx.user.profile();
            }

            userProfilePromise
                .success(function (profile) {
                    $pyx(profile).galleries().getAll().success(function (galleries) {
                        deferred.resolve({ profile: profile, galleries: galleries });
                    })
                    .error(function () {
                        deferred.reject("failed to get galleries");
                    });
                })
                .error(function () {
                    deferred.reject("failed to get galleries");
                });
            return deferred.promise;
        }
    };
});

app.config([
    "$routeProvider", "$locationProvider",
    function ($routeProvider, $locationProvider) {
        $routeProvider.
            when("/User/:id", {
                templateUrl: "/contents/templates/wv-user.html",
                controller: "worldviewUserController",
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    user: function (userAuthResolve, $route) {
                        return userAuthResolve.userProfileAndGalleries($route.current.params["id"])
                            .then(function (data) {
                                $route.current.title = data.profile.Metadata.Name;
                                $route.current.description = data.profile.Metadata.Description;
                                return data;
                            });
                    }
                }
            }).
            when("/GeoSource/", {
                templateUrl: "/contents/templates/wv-main.html",
                controller: "worldviewSearchController",
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    resourceType: function () { return "GeoSource"; }
                }
            }).
            when("/GeoSource/:id", {
                templateUrl: "/contents/templates/wv-details.html",
                controller: "worldviewDetailsController",
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    resource: function ($pyx, $route) {
                        return $pyx.gallery.geoSources().getById($route.current.params["id"])
                            .then(function (result) {
                                var data = result.data;
                                $route.current.title = data.Metadata.Name;
                                $route.current.description = data.Metadata.Description;
                                return data;
                            });
                    },
                    resourceStatus: function ($pyx, $route) {
                        return $pyx({ Id: $route.current.params["id"], Type: "GeoSource" }).status()
                            .then(function (result) {
                                return result.data;
                            });
                    }
                }
            }).
            when("/Globe/", {
                templateUrl: "/contents/templates/wv-main.html",
                controller: "worldviewSearchController",
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    resourceType: function () { return "Map"; }
                }
            }).
            when("/Globe/:id", {
                templateUrl: "/contents/templates/wv-details.html",
                controller: "worldviewDetailsController",
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    resource: function ($pyx, $route) {
                        return $pyx.gallery.maps().getById($route.current.params["id"])
                            .then(function (result) {
                                var data = result.data;
                                $route.current.title = data.Metadata.Name;
                                $route.current.description = data.Metadata.Description;
                                return data;
                            });
                    },
                    resourceStatus: function () { return undefined; }
                }
            }).
            when("/Gallery/", {
                templateUrl: "/contents/templates/wv-main.html",
                controller: "worldviewSearchController",
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    resourceType: function () { return "Gallery"; }
                }
            }).
            when("/Gallery/:id", {
                templateUrl: "/contents/templates/wv-gallery.html",
                controller: "worldviewGalleryController",
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    gallery: function ($pyx, $route, $q) {
                        var deferred = $q.defer();
                        var result = {};
                        $pyx.gallery.galleries().getById($route.current.params["id"]).
                            success(function (data) {
                                result.gallery = data;
                                $route.current.title = data.Metadata.Name;
                                $route.current.description = data.Metadata.Description;
                                $pyx(result.gallery).resources().success(function (resources) {
                                    result.resources = resources;
                                    deferred.resolve(result);
                                }).error(function () {
                                    deferred.reject("failed to load resources");
                                });
                            })
                            .error(function () {
                                deferred.reject("gallery not found");
                            });
                        return deferred.promise;
                    }
                }
            }).
            when("/admin/status", {
                templateUrl: "/contents/templates/wv-system-status.html",
                controller: "worldviewAdminController",
                title: "System Status",
                resolve: {
                    loggedin: function (userAuthResolve, $pyx) {
                        return userAuthResolve.auth()
                            .then(function () { return $pyx.user.admin; });
                    }
                }
            }).
            when("/create/GeoSource", {
                templateUrl: "/contents/templates/wv-resource-create.html",
                controller: "worldviewCreateController",
                title: "Publish Your GeoSource",
                searchDisabled: true,
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.auth(); },
                    user: function (userAuthResolve) { return userAuthResolve.userProfileAndGalleries(); },
                    resourceType: function () { return "GeoSource"; }
                }
            }).
            when("/create/Globe", {
                templateUrl: "/contents/templates/wv-resource-create.html",
                controller: "worldviewCreateController",
                title: "Publish Your Globe",
                searchDisabled: true,
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.auth(); },
                    user: function (userAuthResolve) { return userAuthResolve.userProfileAndGalleries(); },
                    resourceType: function () { return "Map"; }
                }
            }).
            when("/info/account", {
                templateUrl: "/contents/templates/wv-account.html",
                controller: "worldviewAccountController",
                title: "Account Info",
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.auth(); },
                    user: function ($pyx) {
                        return $pyx.user.profile().then(function (response) { return response.data; });
                    }
                }
            }).
            when("/info/access-denied", {
                searchDisabled: true,
                signInDisabled: true,
                templateUrl: "/contents/templates/wv-access-denied.html",
                controller: "worldviewInfoPageController",
                title: "Vision"
            }).
            when("/info/education-vision", {
                searchDisabled: true,
                templateUrl: "/contents/templates/wv-vision-education.html",
                controller: "worldviewInfoPageController",
                title: "GeoLiteracy Project"
            }).
            when("/info/vision", {
                searchDisabled: true,
                templateUrl: "/contents/templates/wv-vision.html",
                controller: "worldviewInfoPageController",
                title: "Vision"
            }).
            when("/info/contactus", {
                searchDisabled: true,
                templateUrl: "/contents/templates/wv-contact.html",
                controller: "worldviewInfoPageController",
                title: "Contact Us"
            }).
            when("/info/faq", {
                searchDisabled: true,
                templateUrl: "/contents/templates/wv-faq.html",
                controller: "worldviewFaqController",
                title: "Vision"
            }).
            when("/info/systemRequirements", {
                searchDisabled: true,
                templateUrl: "/contents/templates/wv-system-requirements.html",
                controller: "worldviewInfoPageController",
                title: "WorldView Studio System Requirements"
            }).
            when("/info/terms", {
                searchDisabled: true,
                templateUrl: "/contents/templates/wv-terms.html",
                controller: "worldviewInfoPageController",
                title: "Terms of Service"
            }).
            when("/info/privacy", {
                searchDisabled: true,
                templateUrl: "/contents/templates/wv-privacy.html",
                controller: "worldviewInfoPageController",
                title: "Privacy"
            }).
            when("/forgotPassword", {
                searchDisabled: true,
                signInDisabled: true,
                templateUrl: "/contents/templates/wv-forgot-password.html",
                controller: "worldviewForgotPasswordController",
                title: "Forgot Password"
            }).
            when("/resetPassword", {
                searchDisabled: true,
                signInDisabled: true,
                templateUrl: "/contents/templates/wv-reset-password.html",
                controller: "worldviewResetPasswordController",
                title: "Reset Password"
            }).
            when("/updatedPassword", {
                searchDisabled: true,
                templateUrl: "/contents/templates/wv-updated-password.html",
                controller: "worldviewUpdatedPasswordController",
                title: "Password Updated"
            }).
            when("/confirmEmail", {
                searchDisabled: true,
                templateUrl: "/contents/templates/wv-confirm-email.html",
                controller: "worldviewConfirmEmailController",
                resolve: {
                    user: function ($pyx) {
                        return $pyx.user.profile().then(function (response) { return response.data; }).catch(function () { return; });
                    }
                }
            }).
            when("/requestNews", {
                searchDisabled: true,
                templateUrl: "/contents/templates/wv-newsletter-request.html",
                controller: "worldviewNewsRequestController"
            }).
            when("/signUp", {
                searchDisabled: true,
                templateUrl: "/contents/templates/wv-sign-up.html",
                controller: "worldviewSignUpController"
            }).
            when("/sso", {
                searchDisabled: true,
                // use an empty template (not templateUrl)
                template: "",
                controller: "worldviewSingleSignOnController"
            }).
            when("/download", {
                searchDisabled: true,
                templateUrl: "/contents/templates/wv-download.html",
                controller: "downloadController"
            }).
            when("/:name", {
                templateUrl: "/contents/templates/wv-gallery.html",
                controller: "worldviewGalleryController",
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    gallery: function ($pyx, $route, $q, $location) {
                        var deferred = $q.defer();
                        var result = {};
                        $pyx.gallery.galleries().getByName($route.current.params["name"]).
                            success(function (data) {
                                result.gallery = data;
                                $route.current.title = data.Metadata.Name;
                                $route.current.description = data.Metadata.Description;
                                $pyx(result.gallery).resources().success(function (resources) {
                                    result.resources = resources;
                                    deferred.resolve(result);
                                }).error(function () {
                                    deferred.reject("failed to load resources");
                                });
                            })
                            .error(function () {
                                deferred.reject("gallery not found");
                                $location.path("/");
                            });
                        return deferred.promise;
                    }
                }
            }).
            when("/", {
                templateUrl: "/contents/templates/wv-main.html",
                controller: "worldviewSearchController",
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    resourceType: function () { return "All"; }
                },
                description: "Within millions of organizations, silos of data that describe places, things, and events are growing and growing. The WorldView™ Platform provides an unprecedented ability to release the value of this data helping individuals and organizations to make better decisions. Search, combine, analyze and share the world’s information on-demand. It is the next-generation Web experience."
            }).
            otherwise({
                redirectTo: "/"
            });

        $locationProvider.html5Mode(true);
    }
]);