/* ===================================================================================
 * Root Controller
 * =================================================================================== 
 - @description
   This controller is used for all site-wide UI (@example toggling the site fullscreen menu )
   and Logic (@example adding a body class)
 */

app.controller('RootController', function($scope, $rootScope, $timeout, $injector, $window, $location) {
    var $validationProvider = $injector.get('$validation');

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

        $validationProvider.reset(form);
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
app.controller('RegisterFormController', function ($scope, $timeout, $q) {

    $scope.signUpForm = {};

    $scope.signUpForm['ConfirmPassword'] = '';
    $scope.signUpForm['Email'] = '';
    $scope.signUpForm['FirstName'] = '';
    $scope.signUpForm['GalleryName'] = '';
    $scope.signUpForm['LastName'] =  '';
    $scope.signUpForm['Password'] = '';

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
app.controller('HomeController', function($scope, $window, $timeout, $q, $http, newsStore) {
    var ctrlModel = this;
    ctrlModel.allPosts = $window.newsRepository;
    ctrlModel.showYear = newsStore.currentYear();  

    var consumerKey = 'oAmnFHZ0dCxJf4v6F7MhG9ZX7';

    newsStore.getPosts(ctrlModel.allPosts, ctrlModel.showYear, 0).then(function (post) {
        ctrlModel.post = post;
        ctrlModel.postPath = newsStore.postNameToUrl(ctrlModel.post.title);
    });
});

/* ===================================================================================
 * Features Controller 
 * =================================================================================== */
app.controller('FeaturesController', function($scope, $timeout, $q) {
    var ctrlModel = this;
    ctrlModel.moreFeatures = false;

});

/* ===================================================================================
 * News Controller
 * =================================================================================== */
app.controller('NewsController', function($scope, $window, $sce, $timeout, $q, $routeParams, newsStore) {
    var ctrlModel = this;

    ctrlModel.showYear = newsStore.currentYear();
    ctrlModel.allPosts = $window.newsRepository;
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

    ctrlModel.allPosts = $window.newsRepository;
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
    var mountTerms = settings.hasOwnProperty('mountTerms') ? settings.mountTerms : true

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