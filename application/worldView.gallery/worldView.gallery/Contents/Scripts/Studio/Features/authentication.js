app.service("featureAuthentication", function ($pyx, $filter, $pyxconfig, $q, $timeout, $interval, $window, $pyxIntercom, worldViewStudioConfig, siteParams,
    inputValidator, accountServices, userUsageTracker, userPolling) {

    var i18n = $filter("i18n");

    function register($scope, options) {

        var defaultOptions = {
            forceLogin: true
        };

        options = angular.extend({}, defaultOptions, options);

        $scope.addTemplate('dialogs', '/template/feature-authentication.html');

        /*
        - @define {object}
        - toggle between the 'login', 'register' , or 'verify' modals
        - email confirmation button state : 'again', 'sending', or 'sent'
        */
        $scope.userInitiation = {
            mode: '',
            send: 'again'
        }

        $pyx.application.ready(function () {
            $timeout(function () {

                /*
                - @name initStudioSetup
                - @desc Setup the Studio
                - @type {function}
                */
                var startUpModal = localStorage.getItem("startUpModal");
                
                var initStudioSetup = function () {
                    $scope.$emit("studio-setup-started");
                    $scope.$emit("studio-setup-completed");
                }

                /*
                - @name checkUserProfile
                - @desc If a user's email is verified run Studio setup
                - @type {function}
                */
                var checkUserProfile = function () {
                    $pyx.user.profile().success(function (profile) {
                        if (profile.EmailConfirmed) {
                            initStudioSetup();
                        } else {
                            $scope.userInitiation.mode = "verify";

                            var handleUserAuth = function () {
                                $scope.userInitiation.mode = "";
                                initStudioSetup();
                            }

                            userPolling.runPoll().then(handleUserAuth).catch(userPolling.stopPoll);

                        }
                    });
                }


                /*
                - @name onLoginComplete
                - @desc After a user finishes  
                - @type {function}
                */
                var onLoginComplete = function () {
                    if ($pyx.user.auth()) {
                        //if this is a tracked session...

                        userUsageTracker.incrementLaunchCount();
                        $scope.userInitiation.mode = "";
                        localStorage["userInitiationMode"] = "";

                        var handleDeltaExpire = function () {
                            $scope.userInitiation.mode = "verify";
                        }

                        if (!userUsageTracker.isTrialExpired()) {
                            initStudioSetup();
                            userUsageTracker.pollUserTrial(handleDeltaExpire);
                        } else {
                            checkUserProfile();
                            userUsageTracker.stopUserTrial();
                        }

                       
                        $pyx.user.profile().success(function (profile) {
                            $pyxIntercom.boot(profile);
                            $pyxIntercom.composerWindow('hide'); 
                            $pyxIntercom.track('sign-in');

                            // When we need messaging - remove this comment 
                            // $interval(function () {
                            //     $pyxIntercom.updateFeed();
                            // }, 60000);
                        });
                    } else {
                        // anonymous session...
                        initStudioSetup();
                    }
                };

                $scope.closeApp = function () {
                    $pyx.application.ready(function () {
                        $pyxIntercom.shutdown();
                        $pyx.application.close();
                    });
                };

                if ("restart" in $pyx.application) {
                    $scope.signoutAndRestartApp = function () {
                        // when a user 'Signs out' - not relaunches
                        // they are prompted to confirm their email
                        userUsageTracker.stopUserTrial();
                        localStorage.removeItem("userInitiationMode");

                        $pyxIntercom.shutdown();

                        $pyx.user.logout();
                        $pyx.application.restart();
                    }

                }

                if ("getVersion" in $pyx.application) {
                    var version = $pyx.application.getVersion();
                    $scope.applicationVersion = i18n("Version %s", version);
                  
                    // the oldest version of the Pyxis API compatible with this UI version
                    var lastApiUpgradeVersion = "1.0.0.1230";
                    // check the API version format and compatibility with the UI
                    if (
                        (!/^1\./.test(version) || version < lastApiUpgradeVersion)
                        && version !== "DEV"
                        ) {
                        $scope.forceUpgrade = true;
                        return;
                    }
                }

                if (options.forceLogin && "login" in $pyx.application) {

                    $scope.sendConfirmationEmail = function () {
                        $scope.userInitiation.send = "sending";
                        $pyx.gallery.sendConfirmationEmail()
                            .success(function () {
                                $scope.notifyInfo("Email Sent");
                                $scope.userInitiation.send = "sent";
                            })
                                .error(function () {
                                    $scope.notifyError("Sorry, we had a problem sending your request to our servers.");
                                    scope.userInitiation.send = "again";
                                });
                    }

                    $scope.userFirstSignin = function () {
                        $pyx.when($pyx.application.loginWithToken($pyx.user.token()))
                        .then(onLoginComplete, onLoginComplete);
                        $scope.submitted = false;
                        $scope.userInitiation.mode = "";
                    }

                    if (!$pyx.user.auth()) {

                        var onError = function (error) {
                            $scope.notifyError(error);
                        };

                        $scope.userInitiation.mode = startUpModal === "login" ? startUpModal : "register";

                        /*
                        - @name handlePostRedirect
                        - @desc Because the external redirect (callback) lands on the same url
                        - this gives us a way of showing the different modals based on where the 
                        - Social sign in occurs 
                        - @type {Function}
                        */
                        var handlePostRedirect = function (data) {
                            var mode = localStorage.getItem("userInitiationMode");
                            var post = localStorage.getItem("postRedirect");

                            if (data) {
                                // Check for false positives on redirect 
                                if (!data.HasRegistered && !post) {
                                    mode = "register";
                                    localStorage["postRedirect"] = true;
                                } else if (!data.HasRegistered && post) {
                                    mode = "";
                                    localStorage.removeItem("postRedirect");
                                }
                            }

                            $scope.userInitiation.mode = mode;
                        }



                        $scope.loginService = $pyx.user.loginService($pyx, i18n, onLoginComplete, onError, true, "/Studio/beta-v1", handlePostRedirect);

                        $scope.login = $scope.loginService.status();
                        $scope.signIn = $scope.loginService.signIn;

                        // - TODO: Refactor Sign Up
                        $scope.signUpForm = {
                            GalleryName: "",
                            FirstName: "",
                            LastName: "",
                            Email: "",
                            Password: "",
                            ConfirmPassword: "",
                            AgreeTerms: false,
                            PromotionConsent: false
                        };

                        // For display on the Sign up Modal
                        $scope.displayUrl = siteParams.galleryUrl() || "http://worldview.gallery";

                        // For use in 'externalLoginProviders'
                        $scope.galleryUrl = siteParams.galleryUrl() || null;
                        $scope.formError = "";

                        $scope.showLicensingAgreement = false;
                        $scope.showTerms = false;

                        $scope.externalProviders = [];

                        var deferredUserInfo = $q.defer();

                        var providersPromise = $pyx.user.externalLoginProviders($scope.galleryUrl, "/Studio/beta-v1").then(function (response) {
                            $scope.externalProviders = response.data;
                            return;
                        }, function () {
                            $scope.notifyError("Failed to get social sign in providers");
                            return $q.reject();
                        });

                        function externalLoginUrl(provider) {
                            var index = $pyx.array.firstIndex($scope.externalProviders, function (p) {
                                return p.Name === provider;
                            });
                            return $pyxconfig.backendUrl + $scope.externalProviders[index].Url;
                        }

                        function completeLogin() {
                            $scope.userInitiation.mode = "";
                            $scope.submitted = true;
                            $scope.notifyInfo("Welcome to WorldView Studio!");
                        }

                        $scope.isExternal = function () {
                            $scope.login.local = false;
                        }

                        $scope.setExternalToken = function (access_token) {
                            $scope.externalToken = access_token;
                        }

                        $scope.setUserInfo = function (userInfo) {
                            $scope.loginProvider = userInfo.LoginProvider;
                            $scope.emailClaim = userInfo.EmailClaim;
                        }

                        $scope.externalLoginError = function (error) {
                            $scope.notifyError(error);
                        }

                        $scope.socialLoginIn = function (provider) {
                            $scope.loginService.socialSignIn(provider);
                            localStorage["userInitiationMode"] = "";
                        }

                        $scope.socialSignUp = function (provider) {
                            $scope.loginService.socialSignIn(provider);
                            localStorage["userInitiationMode"] = "register";
                        }

                        $scope.showTermsOpen = function () {
                            $scope.showTerms = true;
                        }

                        $scope.showTermsCommit = function () {
                            $scope.showTerms = false;
                            $scope.signUpForm.AgreeTerms = true;
                        }

                        $scope.showTermsCancel = function () {
                            $scope.showTerms = false;
                            $scope.signUpForm.AgreeTerms = false;
                        }

                        $scope.showLicensingAgreementOpen = function () {
                            $scope.showLicensingAgreement = true;
                        }

                        $scope.showLicensingAgreementClose = function () {
                            $scope.showLicensingAgreement = false;
                        }

                        $scope.emailAlreadyRegistered = function () {
                            $scope.emailRegistered = false;
                            $scope.login.local = true;
                            $scope.userInitiation.mode = "register";
                        }

                        $scope.emailNotProvided = function () {
                            $scope.userInitiation.mode = "register";
                        }

                        $scope.validEmail = function () {
                            return inputValidator.validateEmail($scope.signUpForm.Email);
                        }

                        $scope.checkGalleryName = function (value) {
                            return accountServices.checkUserNameAvailable(value);
                        }

                        $scope.checkEmail = function (value) {
                            return accountServices.checkEmailAvailable(value);
                        }

                        $scope.submitting = false;
                        $scope.submitted = false;

                        $scope.submit = function () {
                            $scope.submitting = true;
                            if ($scope.login.local) {
                                $pyx.user.register({
                                    "UserName": $scope.signUpForm.GalleryName,
                                    "Password": $scope.signUpForm.Password,
                                    "ConfirmPassword": $scope.signUpForm.ConfirmPassword,
                                    "Email": $scope.signUpForm.Email,
                                    "FirstName": $scope.signUpForm.FirstName,
                                    "LastName": $scope.signUpForm.LastName,
                                    "AcceptTerms": $scope.signUpForm.AgreeTerms,
                                    "PromotionConsent": $scope.signUpForm.PromotionConsent
                                })
                                .then(function () {
                                    $scope.notifyInfo($scope.signUpForm.GalleryName + " - Welcome to WorldView Studio!");
                                    $scope.submitting = false;
                                    $scope.submitted = true;

                                    // Start user trial period
                                    userUsageTracker.startUserTrial();

                                    // Set what modal is presented to the user's after the 
                                    // application has been been initialized 
                                    localStorage["startUpModal"] = "login";

                                })
                                .catch(function (error) {
                                    $scope.notifyError(error);
                                    $scope.submitting = false;
                                });

                            } else {
                                deferredUserInfo.promise.then(function () {
                                    $pyx.user.registerExternal($scope.signUpForm.GalleryName,
                                        $scope.signUpForm.AgreeTerms,
                                        $scope.signUpForm.PromotionConsent,
                                        $scope.externalToken,
                                        externalLoginUrl($scope.loginProvider))
                                    .catch(function (error) {
                                        if (error.status === 403) {
                                            $scope.notifyError(error.data);
                                        } else {
                                            $scope.notifyError(error.data.Message);
                                            $scope.emailRegistered = true;
                                        }
                                        $scope.submitting = false;
                                    });
                                });

                                localStorage["userInitiationMode"] = "";
                            }

                            $pyx.user.attemptExternalLogin(providersPromise, deferredUserInfo, $scope.isExternal, $scope.setExternalToken, $scope.setUserInfo, handlePostRedirect, completeLogin, $scope.externalLoginError);
                        }

                    } else if ("getToken" in $pyx.application) {
                        //TODO: move this to loginService?
                        $pyx.when($pyx.application.getToken()).then(function (token) {
                            if (token.access_token) {
                                //sync java script user with application token

                                $scope.notifyInfo(i18n("Welcome Back") + " " + token.userName);
                                $pyx.user.loginUsingToken(token).then(onLoginComplete, function (error) {
                                    //we got bad token
                                    //but the engine already using it... therefore - restart
                                    $scope.signoutAndRestartApp();
                                });
                            } else {
                                //sync application with current java script user token
                                $scope.notifyInfo(i18n("Welcome Back") + " " + $pyx.user.username());
                                $pyx.when($pyx.application.loginWithToken($pyx.user.token()))
                                    .then(onLoginComplete, onLoginComplete);
                            }
                        }, function () {
                            $pyx.user.logout();
                        });

                    } else {
                        onLoginComplete();
                    }
                } else {
                    onLoginComplete();
                }

                if ("fileDragDrop" in $pyx.application && $scope.activeFeatures.dragAndDrop) {
                    $pyx.application.fileDragEnter(function (files) {
                        $timeout(function () {
                            $scope.showDragFilesMessage = true;
                        });
                    });

                    $pyx.application.fileDragLeave(function (files) {
                        console.log("drag leave: " + files);
                        $timeout(function () {
                            $scope.showDragFilesMessage = false;
                        });
                    });

                    $pyx.application.fileDragDrop(function (files) {
                        $timeout(function () {
                            console.log("drag drop:" + files);
                            $scope.showDragFilesMessage = false;
                            $scope.startImportDataSets(JSON.parse(files));
                        });
                    });
                }

                if ("openFileDialog" in $pyx.application && $scope.activeFeatures.dragAndDrop) {
                    $scope.showImportFilesDialog = function () {
                        $pyx.application.openFileDialog().success(function (dataSets) {
                            $timeout(function () {
                                $scope.startImportDataSets(dataSets);
                            });
                        }).error(function () {
                        });
                    };
                }

                $scope.$emit("pyx-application-ready");
            });
        });
    };

    return {
        register: register
    };
});