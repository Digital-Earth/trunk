app.config(function($routeProvider, $locationProvider) {

    $locationProvider.html5Mode({enabled: true, requireBase: false});

    // Both privacy and terms share the same template
    // this allows each to be displayed on their
    // respective routes.
    var privacyOptions = function () {
        var options = {
            mountTerms: false
        };

        return options;
    };
 
    $routeProvider.when('/', {
        controller: 'HomeController as homeCtrl',
        templateUrl: '/site/parts/home.html'
    })
    .when('/site', {
        controller: 'HomeController as homeCtrl',
        templateUrl: '/site/parts/home.html'
    })
    .when('/features', {
        controller: 'FeaturesController as featuresCtrl',
        templateUrl: '/site/parts/features.html',
        resolve: {}
    })
    .when('/news/:year?', {
        controller: 'NewsController as newsCtrl',
        templateUrl: '/site/parts/news.html'
    })
    .when('/news/:year?/:post', {
        controller: 'PostController as postCtrl',
        templateUrl: '/site/parts/news-post.html'
    })
    .when('/privacy', {
        controller: 'TermsController as termsCtrl',
        templateUrl: '/site/parts/terms-privacy.html',
         resolve: {
            settings: privacyOptions
        }
    })
    .when('/terms', {
        controller: 'TermsController as termsCtrl',
        templateUrl: '/site/parts/terms-privacy.html',
        resolve: {
            settings: privacyOptions
        }
    })
    .when('/contact', {
        controller: 'ContactController',
        templateUrl: '/site/parts/contact.html'
    });
});

app.run(function($rootScope, $location, $window, detectPlatform) {
    var rootNode = '.wrapper';
    var duration = 0.3;
    var isMobile;
    var developmentMode = false;
    

    // If in development mode - go to the worldview.gallery site
    $rootScope.$on('$locationChangeStart', function() {  
        if (developmentMode) {  
            $window.location.href = 'https://worldview.gallery';  
        }  
    });

    $rootScope.$on('$viewContentLoaded', function (event, viewConfig) {
        var timeline = new TimelineMax();
        timeline.to(rootNode, duration, {autoAlpha: 0});
        timeline.fromTo(rootNode, duration, {autoAlpha: 0}, {autoAlpha: 1});
    });

   $rootScope.mobileDevice = detectPlatform.isMobile();
  
});







