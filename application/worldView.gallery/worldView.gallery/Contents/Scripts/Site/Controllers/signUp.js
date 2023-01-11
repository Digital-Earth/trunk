app.controller('worldviewSignUpController', function ($rootScope, $scope, $http, $q, $pyx, $pyxconfig, $timeout, $location, $routeParams, siteParams, wvAlerts, userAuthResolve, accountServices, inputValidator, analytics) {
    // Indicates whether the download page or the sign-up page is shown
    $scope.registerStepNumber = 1;  // DEAN: temporarily remove download step to link directly to signup
    $scope.registerSteps = [
        'Download',
        'Sign up'
    ];

    // Switches a registration step shown on the sign-up page
    $scope.showRegisterStep = function(registerStepNumber) {
        $scope.registerStepNumber = registerStepNumber;
    }

    // Check if an explicit step has been provided as URL query parameter
    var queryRegisterStep = $location.search()['step'];
    if (queryRegisterStep && queryRegisterStep.toLowerCase() === $scope.registerSteps[1].toLowerCase()) {
        $scope.showRegisterStep(queryRegisterStep);
    }

    // The user registration data
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

    $scope.galleryUrl = siteParams.galleryUrl();
    $scope.formError = "";
 
    $scope.showLicensingAgreement = false;
    $scope.showTerms = false;
  
    $scope.local = true;
    $scope.externalProviders = [];

    var deferredUserInfo = $q.defer();

    var providersPromise = $pyx.user.externalLoginProviders($scope.galleryUrl, '/signUp').then(function (response) {
            $scope.externalProviders = response.data;
            return;
        }, function() {
            wvAlerts.error('Failed to get social sign in providers');
            return $q.reject();
        });

    var createDateThresholdInYears = function (boundary) {
        return  new Date(new Date(Date.now()).getTime() - (365 * boundary) * 24 * 60 * 60 * 1000);
    }

    function externalLoginUrl(provider) {
        var index = $pyx.array.firstIndex($scope.externalProviders, function (p) {
            return p.Name === provider;
        });
        return $pyxconfig.backendUrl + $scope.externalProviders[index].Url;
    }

    function completeLogin(profile) {
        $pyx.user.id = profile.Id;
        // if there is a return_to URL parameter we are in the single sign-on flow. Return back to /sso to complete the flow.
        if ($routeParams['return_to']) {
            $location.url('/sso?return_to=' + $routeParams['return_to']);
        } else {
            $location.url('/');
            wvAlerts.success('Welcome to WorldView.Gallery!', $scope);
        }
        $scope.submitted = true;
    }

    $scope.isExternal = function() {
        $scope.local = false;
        // Let the users logged in externally skip the download step
        $scope.showRegisterStep(1);
    }

    $scope.setExternalToken = function (access_token) {
        $scope.externalToken = access_token;
    }

    $scope.setUserInfo = function (userInfo) {
        $scope.loginProvider = userInfo.LoginProvider;
        $scope.emailClaim = userInfo.EmailClaim;
    }

    $scope.handleUserRegistered = function(){
        $scope.local = false;
    }

    $scope.externalLoginError = function (error) {
        wvAlerts.error(error);
    }

    $scope.socialSignUp = function(provider) {
        window.location = externalLoginUrl(provider);
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

    $scope.showLicensingAgreementOpen = function() {
        $scope.showLicensingAgreement = true;
    }

    $scope.showLicensingAgreementClose = function() {
        $scope.showLicensingAgreement = false;
    }

    $scope.emailAlreadyRegistered = function() {
        $location.url('/signUp?step=Sign%20up');
    }

    $scope.emailNotProvided = function () {
        $location.url('/signUp?step=Sign%20up');
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

    /* Could use at a later date

    $scope.validateBirthDate = function (value) {
        // Create a Date object from the input data
        birthDate = new Date(value);
        now = new Date(Date.now());

        var minEntryDate = createDateThresholdInYears(13);
        var maxEntryDate = createDateThresholdInYears(150);

        $scope.minAgeRequirement = true;
     
        // The person must be no younger than 13 years
        if (birthDate > minEntryDate) {
            $scope.minAgeRequirement = false;
            return false;
        }

        // The person must be no older than 150 years
        if (birthDate < maxEntryDate) {
            return false;
        }

        return true;
    }
    */

    $scope.submitting = false;
    $scope.submitted = false;

    $scope.submit = function () {
        $scope.submitting = true;
        if ($scope.local) {
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
                $location.path('/signUp');
                wvAlerts.success($scope.signUpForm.GalleryName + ' - Welcome to WorldView.Gallery!', $scope);
                $scope.submitting = false;
                $scope.submitted = true;
                analytics.event('signup form', 'successful signup');
            })
            .catch(function (error) {
                wvAlerts.error(error, $scope);
                $scope.submitting = false;
                analytics.event('signup form', 'error', error.data.Message);
            });

        } else {
            deferredUserInfo.promise.then(function () {
                analytics.event('signup form', 'social signup');

                $pyx.user.registerExternal($scope.signUpForm.GalleryName,
                    $scope.signUpForm.AgreeTerms,
                    $scope.signUpForm.PromotionConsent,
                    $scope.externalToken,
                    externalLoginUrl($scope.loginProvider))
                .catch(function (error) {
                    if (error.status === 403) {
                        wvAlerts.error(error.data, $scope);
                    } else {
                        wvAlerts.error(error.data.Message, $scope);
                        $scope.emailRegistered = true;
                    }
                    $scope.submitting = false;

                    analytics.event('signup form', 'social signup error', error.data.Message);
                });
            });
        }
    }

    $pyx.user.attemptExternalLogin(providersPromise, deferredUserInfo, $scope.isExternal, $scope.setExternalToken, $scope.setUserInfo, $scope.handleUserRegistered, completeLogin, $scope.externalLoginError);

    analytics.event('signup form', 'form accessed');
});