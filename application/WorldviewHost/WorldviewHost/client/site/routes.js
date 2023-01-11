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

app.provider('contentManager', function  () {
    var collections = {};

    this.membership = function (space) {
        var collection;

        if (!collections.hasOwnProperty(space)) {
            collections[space] = {};

            collections[space].referenceId = ''
            collections[space].accessToken = '';
            collections[space].contentType = {}
         }  

        collection = collections[space];

        return ({
            registerGuid: function (guid) {
                collection.referenceId = guid;
                return this;
            },
            accessToken: function (token) {
                collection.accessToken = token;
                return this;
            }
        })
    };

    this.setTaxonomy = function (space) {
        var contentStore = collections[space].contentType;

        return ({
            makeSection: function (key, guid) {
                contentStore[key] = guid; 
                return this;
            }
        })
    };


    this.$get = function ($timeout, $q) {
        return new ContentManager(collections);
    };

    function ContentManager (collections) {
        var methods = {};
        var manager;

        methods.collections = collections;
 
        methods.getTaxonomy = function (space) {
            var contentStore = this.collections[space].contentType;
 
           return ({
                allSections: function () {
                    return contentStore;
                },
                pullSection: function (key) {
                    return contentStore[key] || ''; 
                }
            })
        };

        methods.makeClient = function (space) {
            var config = {};

            config.accessToken = this.collections[space].accessToken;
            config.space = this.collections[space].referenceId;

            manager = contentful.createClient(config);

            return ({
                getEntries : function (type, opts) {
                    opts = opts || {};
                    type = methods.getTaxonomy(space).pullSection(type);

                    return manager.entries(_.extend(opts, {content_type: type}));
                }
            })
        };


        return methods;
    }

});

app.config(["$routeProvider", "$locationProvider", "contentManagerProvider", function ($routeProvider, $locationProvider, contentManager) {
        // Both privacy and terms share the same template
        // this allows each to be displayed on their
        // respective routes.
        var privacyOptions = function (showTerms) {
            var options = {
                mountTerms: showTerms
            };

            return options;
        };

        contentManager.membership('Posts')
        .accessToken('059870c6f98794ef37309f3f7e40dfb14ff0dbf816c309a7241c27506ebda2a6')
        .registerGuid('01mffn6mq82w');

        contentManager.membership('Pages')
        .accessToken('ba78d95c852124bd8a07ed0d6a96c9b23f81ad03b3611b2fd5923390aae34dec')
        .registerGuid('bk47a4jctmvb');

        contentManager.setTaxonomy('Posts')
        .makeSection('Post', '2wKn6yEnZewu2SCCkus4as');

        contentManager.setTaxonomy('Pages')
        .makeSection('Home', '3sScOsWlbyseUo00cU20sG');

        $routeProvider.
            when('/', {
                controller: 'HomeController as homeCtrl',
                templateUrl: '/site/parts/home.html',
                newSite: true,
                resolve: {
                    content: function (contentManager) {
                        return contentManager.makeClient('Pages').getEntries('Home');
                    }
                }
            }).
            when('/features', {
                controller: 'FeaturesController as featuresCtrl',
                templateUrl: '/site/parts/features.html',
                resolve: {},
                newSite: true
            }).
            when('/news/:year?', {
                controller: 'NewsController as newsCtrl',
                templateUrl: '/site/parts/news.html',
                newSite: true
            }).
            when('/news/:year?/:post', {
                controller: 'PostController as postCtrl',
                templateUrl: '/site/parts/news-post.html',
                newSite: true
            }).
            when('/info/privacy', {
                controller: 'TermsController as termsCtrl',
                templateUrl: '/site/parts/terms-privacy.html',
                resolve: {
                    settings: function() {
                        return privacyOptions(false);
                    }
                },
                newSite: false
            }).
            when('/info/terms', {
                controller: 'TermsController as termsCtrl',
                templateUrl: '/site/parts/terms-privacy.html',
                resolve: {
                    settings: function() {
                        return privacyOptions(true);
                    }
                },
                newSite: false
            }).
            when('/contact', {
                controller: 'ContactController',
                templateUrl: '/site/parts/contact.html',
                newSite: true
            }).
            when('/mobile/register', {
                templateUrl: '/site/parts/modals/register.html',
                newSite: true
            }).
            when('/mobile/sign-in', {
                templateUrl: '/site/parts/modals/sign-in.html',
                newSite: true
            }).
            when("/User/:id", {
                templateUrl: "/client/templates/wv-user.html",
                controller: "worldviewUserController",
                caseInsensitiveMatch: true,
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
                templateUrl: "/client/templates/wv-main.html",
                controller: "worldviewSearchController",
                caseInsensitiveMatch: true,
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    resourceType: function () { return "GeoSource"; }
                }
            }).
            when("/GeoSource/:id", {
                templateUrl: "/client/templates/wv-details.html",
                controller: "worldviewDetailsController",
                caseInsensitiveMatch: true,
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
                templateUrl: "/client/templates/wv-main.html",
                controller: "worldviewSearchController",
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    resourceType: function () { return "Map"; }
                }
            }).
            when("/Globe/:id", {
                templateUrl: "/client/templates/wv-details.html",
                controller: "worldviewDetailsController",
                caseInsensitiveMatch: true,
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
                templateUrl: "/client/templates/wv-main.html",
                controller: "worldviewSearchController",
                caseInsensitiveMatch: true,
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    resourceType: function () { return "Gallery"; }
                }
            }).
            when("/Gallery/:id", {
                templateUrl: "/client/templates/wv-gallery.html",
                controller: "worldviewGalleryController",
                caseInsensitiveMatch: true,
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
                templateUrl: "/client/templates/wv-system-status.html",
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
                templateUrl: "/client/templates/wv-resource-create.html",
                controller: "worldviewCreateController",
                title: "Publish Your GeoSource",
                searchDisabled: true,
                caseInsensitiveMatch: true,
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.auth(); },
                    user: function (userAuthResolve) { return userAuthResolve.userProfileAndGalleries(); },
                    resourceType: function () { return "GeoSource"; }
                }
            }).
            when("/create/Globe", {
                templateUrl: "/client/templates/wv-resource-create.html",
                controller: "worldviewCreateController",
                title: "Publish Your Globe",
                searchDisabled: true,
                caseInsensitiveMatch: true,
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.auth(); },
                    user: function (userAuthResolve) { return userAuthResolve.userProfileAndGalleries(); },
                    resourceType: function () { return "Map"; }
                }
            }).
            when("/info/account", {
                templateUrl: "/client/templates/wv-account.html",
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
                templateUrl: "/client/templates/wv-access-denied.html",
                controller: "worldviewInfoPageController",
                title: "Vision"
            }).
            when("/info/education-vision", {
                searchDisabled: true,
                templateUrl: "/client/templates/wv-vision-education.html",
                controller: "worldviewInfoPageController",
                title: "GeoLiteracy Project"
            }).
            when("/info/vision", {
                searchDisabled: true,
                templateUrl: "/client/templates/wv-vision.html",
                controller: "worldviewInfoPageController",
                title: "Vision"
            }).
            when("/info/contactus", {
                searchDisabled: true,
                templateUrl: "/client/templates/wv-contact.html",
                controller: "worldviewInfoPageController",
                title: "Contact Us"
            }).
            when("/info/faq", {
                searchDisabled: true,
                templateUrl: "/client/templates/wv-faq.html",
                controller: "worldviewFaqController",
                title: "Vision"
            }).
            when("/info/systemRequirements", {
                searchDisabled: true,
                templateUrl: "/client/templates/wv-system-requirements.html",
                controller: "worldviewInfoPageController",
                caseInsensitiveMatch: true,
                title: "WorldView Studio System Requirements"
            }).
            when("/forgotPassword", {
                searchDisabled: true,
                signInDisabled: true,
                templateUrl: "/client/templates/wv-forgot-password.html",
                controller: "worldviewForgotPasswordController",
                caseInsensitiveMatch: true,
                title: "Forgot Password"
            }).
            when("/resetPassword", {
                searchDisabled: true,
                signInDisabled: true,
                templateUrl: "/client/templates/wv-reset-password.html",
                controller: "worldviewResetPasswordController",
                caseInsensitiveMatch: true,
                title: "Reset Password"
            }).
            when("/updatedPassword", {
                searchDisabled: true,
                templateUrl: "/client/templates/wv-updated-password.html",
                controller: "worldviewUpdatedPasswordController",
                caseInsensitiveMatch: true,
                title: "Password Updated"
            }).
            when("/confirmEmail", {
                searchDisabled: true,
                templateUrl: "/client/templates/wv-confirm-email.html",
                controller: "worldviewConfirmEmailController",
                caseInsensitiveMatch: true,
                resolve: {
                    user: function ($pyx) {
                        return $pyx.user.profile().then(function (response) { return response.data; }).catch(function () { return; });
                    }
                }
            }).
            when("/requestNews", {
                searchDisabled: true,
                templateUrl: "/client/templates/wv-newsletter-request.html",
                controller: "worldviewNewsRequestController",
                caseInsensitiveMatch: true
            }).
            when("/signUp", {
                searchDisabled: true,
                templateUrl: "/client/templates/wv-sign-up.html",
                controller: "worldviewSignUpController",
                caseInsensitiveMatch: true
            }).
            when("/sso", {
                searchDisabled: true,
                // use an empty template (not templateUrl)
                template: "",
                controller: "worldviewSingleSignOnController"
            }).
            when("/download", {
                searchDisabled: true,
                templateUrl: "/client/templates/wv-download.html",
                controller: "downloadController"
            }).
            when("/browse", {
                templateUrl: "/client/templates/wv-main.html",
                controller: "worldviewSearchController",
                resolve: {
                    loggedin: function (userAuthResolve) { return userAuthResolve.userIfAvailable(); },
                    resourceType: function () { return "All"; }
                },
                description: "Within millions of organizations, silos of data that describe places, things, and events are growing and growing. The WorldView™ Platform provides an unprecedented ability to release the value of this data helping individuals and organizations to make better decisions. Search, combine, analyze and share the world’s information on-demand. It is the next-generation Web experience."
            }).
            when("/:name", {
                templateUrl: "/client/templates/wv-gallery.html",
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
            otherwise({
                redirectTo: "/"
            });

        $locationProvider.html5Mode(true);
    }
]);