/* ===================================================================================
 * Constants + Values
 * =================================================================================== 
 */

app.constant('Modernizr', Modernizr);

app.value('EnableFeatures', {
    animationSVG: true,
    animationDOM: true
});

// Could grab these from 'media-queries' at some point - 
// but these are the same as 'css/response/index.styl'
app.value('ViewportCutoffs', {
    mobile: 320,
    tablet: 768,
    smallLaptop: 1024,
    laptop: 1440,
    desktop: 1600,
    bigDesktop: 1920,
    baseHeight: 900
});

/* ===================================================================================
 * Root Controller
 * =================================================================================== 
 - @description
   This controller is used for all site-wide UI (@example toggling the site fullscreen menu )
   and Logic (@example adding a body class)
 */

app.controller('RootController', function($scope, $rootScope, $timeout, $injector, $window, $location, detectPlatform) {
    var $validationProvider = $injector.get('$validation');
    // User
    $scope.user = false;

    // Mobile 
    $scope.mobile = detectPlatform.isMobile();
 
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
        if (toggleOn === 'exit'){
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
    var addClass = function(path) {
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
    $rootScope.$on('$locationChangeSuccess', function() {
        addClass($location.path());
    });

    // On init add a class to the 'body' element
    addClass($location.path());

    // Reset scroll position on page 'reload'
    angular.element($window).on('beforeunload', function() {
        angular.element($window).scrollTop(0); 
    });

});

/* ===================================================================================
 * Register Form Controller 
 * =================================================================================== */
app.controller('RegisterFormController', function ($scope, $timeout, $q, $location, $pyx, siteParams, accountServices, externalLogin, wvAlerts) {
    $scope.galleryUrl = siteParams.galleryUrl();
    $scope.social = /^\/signUp/.test($location.path());
    $scope.submitting = false;

    $scope.signUpForm = {};

    $scope.signUpForm['ConfirmPassword'] = '';
    $scope.signUpForm['Email'] = '';
    $scope.signUpForm['FirstName'] = '';
    $scope.signUpForm['GalleryName'] = '';
    $scope.signUpForm['LastName'] = '';
    $scope.signUpForm['Password'] = '';
    $scope.signUpForm['PromotionConsent'] = false;

    $scope.checkGalleryName = function (value) {
        if (!value) {
            return false;
        }
        return accountServices.checkUserNameAvailable(value);
    }

    $scope.register = function() {
        if (!$scope.social) {
            $scope.submitting = true;
            $pyx.user.register({
                    'UserName': $scope.signUpForm.GalleryName,
                    'Password': $scope.signUpForm.Password,
                    'ConfirmPassword': $scope.signUpForm.ConfirmPassword,
                    'Email': $scope.signUpForm.Email,
                    'FirstName': $scope.signUpForm.FirstName,
                    'LastName': $scope.signUpForm.LastName,
                    'AcceptTerms': true,
                    'PromotionConsent': $scope.signUpForm.PromotionConsent
                })
                .then(function() {
                    $location.path("/browse");
                    $scope.submitting = false;
                    $timeout($scope.cleanUpForm);

                })
                .catch(function(error) {
                    var errorMessage = "Email and/or password are invalid";

                    // 'ModelState' should contain the error message;
                    if (error.data && error.data.ModelState) {
                        var modelState = error.data.ModelState;
                        for (var key in modelState) {
                            errorMessage = modelState[key][0];
                        }
                    }
                    wvAlerts.error(errorMessage, $scope);
                    $scope.submitting = false;
                });
        } else {
            $scope.$emit("external-registration-submit", $scope.signUpForm);
        }
    }

    $scope.socialSignIn = externalLogin.socialSignIn;

    // Clear registration fields 
    $scope.cleanUpForm = function() {
        for (var field in $scope.signUpForm) {
            $scope.signUpForm[field] = "";
        }

        $scope.registerForm.$setPristine();
        $scope.signUpForm['PromotionConsent'] = false;
    }

});

/* ===================================================================================
 * Sign In Form Controller 
 * =================================================================================== */
app.controller('SignInFormController', function ($scope, $timeout, $q, $location, $route, $pyx, $pyxconfig, externalLogin, siteParams, userAuthResolve, wvAlerts) {
    
    $scope.submitting = false;

    $scope.signInForm = {};
        
    $scope.signInForm['Email'] = '';
    $scope.signInForm['Password'] = '';

    $scope.submitSignIn = function() {
        $scope.submitting = true;
        $pyx.user.login($scope.signInForm.Email, $scope.signInForm.Password)
            .success(function() {
                $scope.submitting = false;
                $scope.signIn = false;
                if (userAuthResolve.requestUrl) {
                    $location.path(userAuthResolve.requestUrl);
                } else {
                    if ($location.path() === "/") {
                        $location.path("/browse");
                    } else {
                        $route.reload();
                    }
                }
            })
            .error(function() {
                $scope.submitting = false;
                $scope.signInForm.Password = "";
                wvAlerts.error("Email or password are invalid", $scope);
            });
    }

    $scope.socialSignIn = externalLogin.socialSignIn;

});

/* ===================================================================================
 * Newsletter Form Controller 
 * =================================================================================== */
app.controller('NewsletterFormController', function ($scope, $http, wvAlerts) {

    $scope.submitting = false;

    $scope.submitNewsletterRequest = function () {

        $scope.submitting = true;
        $http.post('/api/requestNews', $scope.newsletterForm)
            .success(function (data) {
                $scope.submitting = false;
                $scope.newsletterForm.Email = "";
                wvAlerts.success('Thank you for subscribing to the e-newsletter.', $scope);
            })
            .error(function () {
                $scope.submitting = false;
                wvAlerts.error('Sorry, we had a problem sending your request to our servers.', $scope);
            });
    }

});

/* 
 - Note on: Using 'var ctrlModel = this;'
 - 'controllerAs' is syntactic sugar over '$scope'. You can still bind to the View and still access $scope methods.
 - Helps avoid the temptation of using $scope methods inside a controller when it may otherwise be better to avoid 
 - them or move the method to a factory, and reference them from the controller.
*/

/* ===================================================================================
 * Home Controller 
 * =================================================================================== */
app.controller('HomeController', function($rootScope, $scope, $window, $timeout, $q, $http, $sce, newsStore, EnableFeatures, ViewportCutoffs, content) {
    var ctrlModel = this;
    var runFetchCount = 0;

    ctrlModel.allPosts = newsStore.getAllPosts();
    ctrlModel.showYear = newsStore.currentYear();  

    // CMS content container
    ctrlModel.entry = {};

    // Format the CMS content 
    _.each(content[0].fields, function(result, field) {
         if (_.isObject(result)) {
            ctrlModel.entry[field] = {};

            _.each(result.fields, function (value, key) {
                ctrlModel.entry[field][key] = value;
            });

        } else {
            ctrlModel.entry[field] = result;
        }
    });

    // Twitter Fetch config
    var config = {
        id: '649728469480304640',
        lang: 'en',
        maxTweets: 1,
        customCallback: handleFetch
    };
  
    function handleFetch (tweets) {
        ctrlModel.latestTweet = tweets[0];
    };

    // If a user has a slow connection this 'twitterFetcher'
    // has a tendency to not load
    function runFetcher () {
        if (twitterFetcher) {
            runFetchCount = 0;
            twitterFetcher.fetch(config);
        // Fail-safe
        } else if (runFetchCount > 5) {
            return;
        } else {
            runFetchCount += 1;
            runFetcher();
        }
    };

    // Get the latest Tweet 
    $timeout(runFetcher);
    
    // Sanitize and properly render the HTML content 
    ctrlModel.renderHtml = $sce.trustAsHtml;

    newsStore.getPosts(ctrlModel.allPosts, ctrlModel.showYear, 0).then(function (post) {
        ctrlModel.post = post;
        ctrlModel.postPath = newsStore.postNameToUrl(ctrlModel.post.title);
    });

    if ($window.innerWidth < ViewportCutoffs.tablet || Modernizr.touch) {
        EnableFeatures.animationDOM = false;
    }

});

/* ===================================================================================
 * Features Controller 
 * =================================================================================== */
app.controller('FeaturesController', function($scope,  $window, $timeout, $q, EnableFeatures, ViewportCutoffs) {
    var ctrlModel = this;
    ctrlModel.moreFeatures = false;

    if ($window.innerWidth < ViewportCutoffs.tablet || Modernizr.touch) {
        EnableFeatures.animationDOM = false;
    }

});

/* ===================================================================================
 * News Controller
 * =================================================================================== */
app.controller('NewsController', function($scope, $window, $sce, $timeout, $q, $routeParams, newsStore) {
    var ctrlModel = this;

    ctrlModel.showYear = newsStore.currentYear();
    ctrlModel.allPosts = newsStore.getAllPosts();
    // Total number of years we have News articles for
    ctrlModel.maxYears = ctrlModel.allPosts.length - 1;

    // When there is no defined year '/news' vs '/news/2014' - use the current year
    if (!_.isEmpty($routeParams)) {
        ctrlModel.showYear = $routeParams.year;
    }

    // Array index that corresponds to the year
    ctrlModel.yearIndex = newsStore.getYearIndex(ctrlModel.showYear);

    // Sanitize and properly render the HTML content 
    ctrlModel.renderHtml = $sce.trustAsHtml;
    
    // Get all the posts for a specific year
    newsStore.getPosts(ctrlModel.allPosts, ctrlModel.showYear).then(function (posts) {
        angular.forEach(posts, function (post, index) {
            // Create a safe post slug
            post['path'] = newsStore.postNameToUrl(post.title);
        });

        ctrlModel.posts = posts;
    });
   
});

/* ===================================================================================
 * News Post Controller 
 * =================================================================================== */
app.controller('PostController', function($scope, $window, $sce, $timeout, $q, $routeParams, newsStore) {
    var ctrlModel = this;

    ctrlModel.allPosts = newsStore.getAllPosts();
    ctrlModel.postName = $routeParams.post;
    ctrlModel.postYear = $routeParams.year;
    ctrlModel.yearIndex = newsStore.getYearIndex(ctrlModel.postYear);
    ctrlModel.renderHtml = $sce.trustAsHtml;    

    // Get an individual post in a specific year 
    newsStore.getPosts(ctrlModel.allPosts, ctrlModel.postYear, ctrlModel.postName).then(function (post) {
        ctrlModel.post = post;
    });

});

/* ===================================================================================
 * Terms Controller 
 * =================================================================================== */
app.controller('TermsController', function($scope, $timeout, $q, settings) {
    var ctrlModel = this;
    var mountTerms = settings.hasOwnProperty('mountTerms') ? settings.mountTerms : true;

    ctrlModel.renderTerms = mountTerms;

    // Toggle terms and privacy element visibility
    // @param {Boolean} visible 
    ctrlModel.toggleTerms = function (visible) {
        ctrlModel.renderTerms = visible;
    };

});

/* Contact Controller */
app.controller('ContactController', function($scope, $timeout, $) {

});