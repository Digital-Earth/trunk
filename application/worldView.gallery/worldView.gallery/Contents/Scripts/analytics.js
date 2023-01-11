/* GA script */

(function (i, s, o, g, r, a, m) {
    i['GoogleAnalyticsObject'] = r; i[r] = i[r] || function () {
    (i[r].q=i[r].q||[]).push(arguments)},i[r].l=1*new Date();a=s.createElement(o),
    m=s.getElementsByTagName(o)[0];a.async=1;a.src=g;m.parentNode.insertBefore(a,m)
})(window,document,'script','//www.google-analytics.com/analytics.js','ga');

ga('create', 'UA-2262322-10', 'auto');

//captured in $viewContentLoaded.... this would cause it to be
//ga('send', 'pageview');

/* GA script end */

(function (angular) {

    angular.module('analytics', ['ng','pyxis']).service('analytics', [
      '$rootScope', '$window', '$location', '$pyx', function ($rootScope, $window, $location, $pyx) {
          var hasUid = false;
          // Set (or unset) the custom user id if a user signs in (or signs out)
          var setUid = function () {
              if (!hasUid && $pyx.user.auth()) {
                  hasUid = true;
                  ga('set', '&uid', $pyx.user.id);  // set (unexposed) google analytics user id for merging sessions
                  ga('set', 'dimension1', $pyx.user.id); // set custom user id dimension (exposed in google analytics reporting)
              } else if (hasUid && !$pyx.user.auth()) {
                  hasUid = false;
                  ga('create', 'UA-2262322-10', 'auto'); // create a new tracker object if the user signs out (clears previously set fields)
              }
          };
          var track = function () {
              setUid();
              ga('send', 'pageview', $location.path());
          };
          $rootScope.$on('$viewContentLoaded', track);



          var currentContext = 'gallery';
          var service = {
              setContext: function (newContext) {
                  if (newContext) currentContext = newContext;
              },

              // log event with google analytics
              // should events within the studio be opt-in?
              event: function () {
                  setUid();
                  var args = [].slice.call(arguments); // convert arguments to array so it can be concatenated
                  
                  // equivalent of ga('send', 'event', 'gallery', 'eventContext', 'eventName')
                  ga.apply(null, ['send', 'event'].concat([currentContext], args));
              }
          }
          


          // add shortcut to root scope so that it is accessible via HTML templates
          $rootScope.gaEvent = service.event;

          return service;
      }
    ]);

}(window.angular));