angular.module('pyxis').service('globalCookieStore', function ($cookieStore) {
    var globalCookieStore = {
        get: function (name) {
            return $cookieStore.get(name);
        },
        put: function (name, value, days) {
            //until $cookieStore will allow us to control cookie path - we create our own cookie...
            //we need to control the path as the SPA can start with different urls.
            //$cookieStore.put(name, name);            
            var expires = "";            
            if (days) {
                var d = new Date();
                d.setTime(d.getTime() + (days * 24 * 60 * 60 * 1000));
                var expires = "expires=" + d.toUTCString();                
            }

            document.cookie = encodeURIComponent(name) + '=' + encodeURIComponent(JSON.stringify(value)) + ';path=/;' + expires;
        },
        remove: function (name) {
            //until $cookieStore will allow us to control cookie path - we create our own cookie...
            //we need to control the path as the SPA can start with different urls.
            //$cookieStore.remove(name);
            document.cookie = encodeURIComponent(name) + '=;path=/;expires=Thu, 01 Jan 1970 00:00:00 GMT';
        }
    };
    return globalCookieStore;
});

angular.module('pyxis').service('$pyxuser', function ($q, $http, $pyxconfig, $location, $filter, globalCookieStore, $rootScope, siteParams) {
    var baseUrl = function () { return $pyxconfig.baseUrl; };
    var cookieName = 'pyx.user.token';
    var internalData = {};

    var token = globalCookieStore.get(cookieName);
    if (token) {
        var dayBeforeExpires = new Date(token[".expires"]);
        dayBeforeExpires.setDate(dayBeforeExpires.getDate() - 1);

        if (dayBeforeExpires > new Date()) {
            internalData.token = token;
        } else {
            globalCookieStore.remove(cookieName);
        }
    }

    //there is no need to ask profile every time - so we have a cache for 1 minute
    internalData.setProfileCache = function (profile) {
        if (profile) {
            internalData.profile = profile;
            internalData.profileDate = new Date();
        } else {
            delete internalData.profile;
            delete internalData.profileDate;
        }
    }

    internalData.getProfileCache = function () {
        if (!internalData.profile) {
            return undefined;
        }
        var now = new Date();
        if (now.getTime() - internalData.profileDate.getTime() > 60 * 1000) {
            delete internalData.profile;
            delete internalData.profileDate;
            return undefined;
        }
        return internalData.profile;
    }

    var validateToken = function (token) {
        return $http({
            method: 'GET',
            url: baseUrl() + '/User/Profile',
            headers: { 'Authorization': 'Bearer ' + token["access_token"] }
        });
    }

    var requestToken = function (username, password) {
        internalData.setProfileCache(undefined);

        var deferred = $q.defer();

        $http({
            method: 'POST',
            url: baseUrl() + '/Account/Login',
            data: {
                "UserName": username,
                "Password": password
            }
        }).success(function (data) {
            internalData.token = data;
            globalCookieStore.put(cookieName, internalData.token);
            deferred.resolve(data);
        }).error(function (error) {
            deferred.reject(error);
        });

        var promise = deferred.promise;
        promise.success = function (fn) {
            promise.then(function (data) { fn(data); });
            return promise;
        };
        promise.error = function (fn) {
            promise.then(null, function (error) { fn(error); });
            return promise;
        };
        return promise;
    }

    var deserializeLocationHash = function () {
        var params = {};
        var regex = /([^&=]+)=([^&]*)/g;
        var m;

        while (m = regex.exec($location.hash())) {
            params[decodeURIComponent(m[1])] = decodeURIComponent(m[2]);
        }

        return params;
    }

    // create a token that matches local authority tokens
    var createTokenObjectFromExternal = function (params, data) {
        var issued = new Date();
        var expires = new Date(issued.getTime());
        expires.setDate(expires.getDate() + params.expires_in / (24 * 60 * 60)); // add number of days (expires_in is provided in seconds)
        return {
            ".expires": expires.toUTCString(),
            ".issued": issued.toUTCString(),
            "access_token": params.access_token,
            "expires_in": params.expires_in,
            "token_type": params.token_type,
            "userName": data.UserName
        }
    }


    var user = {
        /* 
        @param {Object} registerOptions
        - 
        - "UserName"
        - "Password"
        - "ConfirmPassword"
        - "Email"
        - "FirstName" 
        - "LastName" 
        - "BirthDate" (optional)
        - "AcceptTerms"
        - "PromotionConsent" (optional)
        - 
        */
        register: function (registerOptions) {
            var self = this;
            var username = registerOptions.UserName;
            var password = registerOptions.Password;
        
            return $http({
                method: 'POST',
                url: baseUrl() + '/Account/Register',
                data: registerOptions
            }).then(function (response) {
                return requestToken(username, password);
            }).then(function (response) {
                var userProfile = {
                    "Seller": false,
                    "Metadata": {
                        "Name": username,
                        "Description": "",
                    }
                }

                return $http({
                    method: 'POST',
                    url: baseUrl() + "/User",
                    data: userProfile,
                    headers: self.authHeaders()
                });
            }).then(function (response) {
                $rootScope.$broadcast('pyx-user-registered');
            });
        },
        completeLocalRegistration: function (user) {
            if (user.Galleries.length > 0) {
                return $q.when();
            }

            var gallery = {
                "Metadata": {
                    "Name": user.Metadata.Name,
                    "Description": ""
                }
            }

            return $http({
                method: 'POST',
                url: baseUrl() + "/Gallery",
                data: gallery,
                headers: this.authHeaders()
            });
        },
        registerExternal: function (userName, acceptTerms, promotionConsent, externalToken, externalLoginUrl) {
            return $http({
                method: 'POST',
                url: baseUrl() + '/Account/RegisterExternal',
                data: {
                    "UserName": userName,
                    "AcceptTerms": acceptTerms,
                    "PromotionConsent": promotionConsent
                },
                headers: { 'Authorization': 'Bearer ' + externalToken }
            }).then(function (response) {
                window.location = externalLoginUrl;
            });
        },
        completeExternalRegistration: function (token, userName) {
            var userProfile = {
                "Seller": false,
                "Metadata": {
                    "Name": userName,
                    "Description": ""
                }
            }

            return $http({
                method: 'POST',
                url: baseUrl() + "/User",
                data: userProfile,
                headers: { 'Authorization': 'Bearer ' + token }
            }).then(function (response) {
                var gallery = {
                    "Metadata": {
                        "Name": userName,
                        "Description": ""
                    }
                }

                return $http({
                    method: 'POST',
                    url: baseUrl() + "/Gallery",
                    data: gallery,
                    headers: { 'Authorization': 'Bearer ' + token }
                });
            }).then(function (response) {
                $rootScope.$broadcast('pyx-user-registered');
            });
        },
        userInfo: function (token) {
            return $http({
                method: 'GET',
                url: baseUrl() + "/Account/UserInfo",
                headers: { 'Authorization': 'Bearer ' + token }
            });
        },
        attemptExternalLogin: function (providersPromise, deferredUserInfo, setIsExternal, setExternalToken, setUserInfo, handleNotRegistered, completeLogin, setError) {
            var self = this;
            var params = deserializeLocationHash();
            if (params.access_token) {
                setIsExternal();
                setExternalToken(params.access_token);
                providersPromise.then(function () {
                        
                        self.userInfo(params.access_token).success(function (data) {
                            setUserInfo(data);
                            if (deferredUserInfo != null) {
                                deferredUserInfo.resolve();
                                
                                // not using analytics because a circular dependency was created
                                $rootScope.gaEvent('signup', 'social signup success');
                            }
                            
                            if (data.HasRegistered) {
                                // attempt to login if the account is registered (may not have attached Resources)
                                var token = createTokenObjectFromExternal(params, data);
                                self.loginUsingToken(token)
                                    .success(function (profile) {
                                        completeLogin(profile, token);
                                    })
                                    .error(function (error) {
                                        // complete registration by adding attached Resources then login
                                        self.completeExternalRegistration(params.access_token, data.UserName)
                                            .then(function () {
                                                self.loginUsingToken(token)
                                                    .success(function (profile) {
                                                        completeLogin(profile, token);
                                                    })
                                                    .error(function (error2) {
                                                        setError('Unable to login');
                                                    });
                                            });
                                    });
                            } else {
                                handleNotRegistered(data);
                            }
                        })
                        .error(function (error) {
                            setError('Unable to get user info');
                        });
                    })
                    .catch(function () {
                        setError('Couldn\'t get social sign-in account information');
                        if (providersPromise != null && deferredUserInfo != null) {
                            deferredUserInfo.reject();
                        }
                    });
            } else if (params.error) {
                setError(params.error);
            }
        },
        login: function (username, password) {
            if (this.auth()) {
                this.logout();
            }

            var self = this;

            var promise = requestToken(username, password);
    
            promise.success(function () {
                $rootScope.$broadcast('pyx-user-login');
            });
            return promise;
        },
        loginUsingToken: function (token) {
            var deferred = $q.defer();

            var profileRequest = validateToken(token);
            profileRequest.success(function (data) {
                internalData.token = token;
                globalCookieStore.put(cookieName, internalData.token);
                deferred.resolve(data);
                internalData.setProfileCache(data);
            }).error(function (data) {
                deferred.reject(data);
                internalData.setProfileCache(undefined);
            });

            //reuse profile request if we can
            internalData.profileRequest = profileRequest;

            var promise = deferred.promise;
            promise.success = function (fn) {
                promise.then(function (data) { fn(data); });
                return promise;
            };
            promise.error = function (fn) {
                promise.then(null, function (error) { fn(error); });
                return promise;
            };

            promise.success(function () {
                $rootScope.$broadcast('pyx-user-login');
            });

            return promise;

        },
        externalLoginProviders: function (url, path) {
            var redirectUrl;
            if(!url) {
                redirectUrl = siteParams.galleryUrl();
            } else {
                redirectUrl = url;
            }

            return $http({
                method: 'GET',
                url: baseUrl() + '/Account/ExternalLogins?ReturnUrl=' + redirectUrl + path + '&GenerateState=true'
            });
          
        },
        jwt: function(application) {
            return $http({
                method: 'GET',
                url: baseUrl() + '/Account/Jwt?application=' + application,
                headers: this.authHeaders()
            });
        },
        changePassword: function (oldPassword, newPassword, confirmPassword) {
            return $http({
                method: 'POST',
                url: baseUrl() + '/Account/ChangePassword',
                data: {
                    "OldPassword": oldPassword,
                    "NewPassword": newPassword,
                    "ConfirmPassword": confirmPassword
                },
                headers: this.authHeaders()
            });
        },
        auth: function () {
            return angular.isDefined(internalData.token);
        },
        username: function () {
            if (this.auth()) {
                return internalData.token.userName;
            }
            return "";
        },
        logout: function () {
            delete internalData.token;
            delete this.id;
            globalCookieStore.remove(cookieName);
            internalData.setProfileCache(undefined);
            $rootScope.$broadcast('pyx-user-logout');
        },
        authHeaders: function () {
            if (internalData.token) {
                return { 'Authorization': 'Bearer ' + internalData.token["access_token"] };
            }
            return {};
        },
        token: function () {
            return internalData.token;
        },
        profile: function (forceRefresh) {
            var self = this;
            if (!forceRefresh) {
                var profile = internalData.getProfileCache();

                if (profile) {
                    this.id = profile.Id;
                    var response = { data: profile };
                    var promise = $q.when(response);
                    promise.success = function (fn) {
                        promise.then(function (p) { fn(p.data); });
                        return promise;
                    };
                    promise.error = function (fn) {
                        promise.then(null, function (error) { fn(error); });
                        return promise;
                    };

                    return promise;
                }
            }
            //check if someone is already requesting the profile
            if (internalData.profileRequest) {
                //return cached request
                return internalData.profileRequest;
            } else {
                //create new profile request
                internalData.profileRequest = $http({
                    method: 'GET',
                    url: baseUrl() + '/User/Profile',
                    headers: this.authHeaders()
                });

                internalData.profileRequest.success(function (profile) {
                    self.id = profile.Id;
                    internalData.setProfileCache(profile);
                    internalData.profileRequest = undefined;
                }).error(function () {
                    internalData.setProfileCache(undefined);
                    internalData.profileRequest = undefined;
                });
            }
            return internalData.profileRequest;
        },
        setConsent: function(promotionConsent) {
            return $http({
                method: 'POST',
                url: baseUrl() + '/Account/SetConsent',
                data: {
                    "PromotionConsent": promotionConsent
                },
                headers: this.authHeaders()
            });
        },
        quota: function () {
            return $http({
                'method': 'GET',
                'url': baseUrl() + "/User/Quota",
                'headers': this.authHeaders()
            });
        },
        storage: function () {
            return $http({
                'method': 'GET',
                'url': baseUrl() + "/User/Storage",
                'headers': this.authHeaders()
            });
        },
        licenses: function () {
            return $http({
                'method': 'GET',
                'url': baseUrl() + "/User/Licenses",
                'headers': this.authHeaders()
            });
        },
        loginService: function (pyx, i18n, onLoginComplete, onError, useExternalLogin, redirectUrl, postRedirect) {
            var status = {
                active: true,
                inProgress: false,
                user: "",
                password: "",
                local: true,
                externalProviders: []
            };

            var galleryUrl = siteParams.galleryUrl();
          
            function externalLoginUrl(provider) {
                var index = pyx.array.firstIndex(status.externalProviders, function (p) {
                    return p.Name === provider;
                });
                return $pyxconfig.backendUrl + status.externalProviders[index].Url;
            }

            var providersPromise = pyx.user.externalLoginProviders(galleryUrl, redirectUrl).then(function (response) {
                status.externalProviders = response.data;
                return;
            }, function () {
                onError(i18n('Failed to get social sign in providers'));
                return $q.reject();
            });


            function loginSuccess(loginToken) {
                pyx.user.loginUsingToken(loginToken).then(function () {
                    status.active = false;
                    onLoginComplete();
                }, function () {
                    status.inProgress = false;
                    onError(i18n('Failed to load user profile'));
                });
            }

            function loginError(error) {
                status.inProgress = false;
                onError(error);
            }

            var login = {
                status: function () {
                    return status;
                },
                externalProviders: function () {
                    return status.externalProviders;
                },
                isExternal: function () {
                    status.local = false;
                },
                setExternalToken: function (access_token) {
                    status.externalToken = access_token;
                },
                setUserInfo: function (userInfo) {
                    status.Provider = userInfo.LoginProvider;
                },
                handlePostRedirect: function (data) {
                    postRedirect(data);
                    status.local = false;
                },
                externalLoginError: function (error) {
                    onError(error);
                },
                socialSignIn: function (provider) {
                    window.location = externalLoginUrl(provider);
                },
                completeExternalLogin: function (profile, token) {
                    status.inProgress = true;
                    pyx.when(pyx.application.loginWithToken(token)).then(
                        loginSuccess,
                        function (error) {
                            var unrecoverableErrorPattern = /login.*only once/i;
                            if (unrecoverableErrorPattern.test(error)) {
                                loginError(i18n('Unrecoverable login error'));
                            } else {
                                loginError(i18n('Failed to authenticate using social sign in'));
                            }
                        });
                },
                attemptExternalLogin: function () {
                    pyx.user.attemptExternalLogin(providersPromise, null, this.isExternal, this.setExternalToken, this.setUserInfo, this.handlePostRedirect, this.completeExternalLogin, this.externalLoginError);  
                },
                signIn: function () {
                    if (!status.user || !status.password) {
                        onError(i18n('Please enter user name and password'));
                        return;
                    }

                    status.inProgress = true;

                    pyx.when(pyx.application.login(status.user, status.password)).then(
                        loginSuccess,
                        function (error) {
                            var unrecoverableErrorPattern = /login.*only once/i;
                            if (unrecoverableErrorPattern.test(error)) {
                                loginError(i18n('Unrecoverable login error'));
                            } else {
                                loginError(i18n('User name or password is invalid'));
                            }
                        });
                }
            };

            if (useExternalLogin) {
                login.attemptExternalLogin();
            }

            return login;
        }
    };

    return user;
});