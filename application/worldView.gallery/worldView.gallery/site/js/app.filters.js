/*
 - @name scrollTo
 - @ngdoc filter
 *
 - @param {Object} $window
 *
 - @description 
   Enables click event on element to automatically scroll to another element. 
*
- @example
  <a  href="" ng-click="'.tour-clarity' | scrollTo : 0.6 : Power4.easeInOut"></a>
*/
app.filter('scrollTo', function ($window) {
	function scrollToFilter (selector, duration, easingCurve) {
		// Class or Id of element to scroll to - @param {String} 'destination' 
		var destination = angular.element(selector);
		// Duration of the animation - @param {Number} 'duration'
		var duration = duration || 0.5;
		// Type of easing to use - @param {Object} 'easingCurve'
		var easingCurve = easingCurve || Power4.easeInOut;
		
		TweenMax.to($window, duration, {scrollTo: {y: destination.offset().top, x:0}, ease: easingCurve});
			
	}

	return scrollToFilter;
});