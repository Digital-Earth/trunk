/*
 - @name resize
 - @ngdoc directive
 *
 - @param {Object} $window
 - @param {Function} $timeout
 - @param {Function} $parse 
 *
 - @description 
   Watches for changes to window width and window height.
   Set element size based on window dimensions.
   Run a function(s) on resize event.
*
- @example
  <div class="banner surface" resize resize-set="height" resize-run="foo()"></div>
*/
// attribute 'resize-set', @type {String} can be 'height'/'width'/'both'
app.directive('resize', function ($timeout, $window, $parse) {
    function resizePostLink(scope, element, attrs) {
        var theWindow = angular.element($window);
        var resizeSet = attrs.resizeSet || 'none';
        var resizeRun = attrs.resizeRun ? $parse(attrs.resizeRun) : undefined;

        var handleGet = function () {
            return {
                height: theWindow.height(),
                width: theWindow.width()
            }
        };
        // Apply window dimension(s) to element dimension(s)
        var handleSet = function (newValue, oldValue) {
            switch (resizeSet) {
                case 'height':
                    element.css('height', newValue.height);
                    break;
                case 'width':
                    element.css('width', newValue.width);
                    break;
                case 'both':
                    element
                    .css('width', newValue.width)
                    .css('height', newValue.height);
                case 'none':
                    return;
                break;
            }

            // Execute a function when the viewport changes size
            if (resizeRun) {
                $timeout(function() {
                    resizeRun(scope);
                });
            }
        };

        scope.$watch(handleGet, handleSet, true);

        scope.$on('$destroy', function() {
            theWindow.off('resize');
        });

        theWindow.on('resize', function() {
            scope.$apply();
        });
    }

    return ({
        restrict: 'A',
        link: resizePostLink
    });
});

/*
 - @name scroll
 - @ngdoc directive
 *
 - @param {Object} $window
 - @param {Function} $parse 
 *
 - @description 
   Watches the window scroll top property.
*
- @example
  <body scroll scroll-off="{{obj.prop}}"></body>
*/
app.directive('scroll', function ($window, $parse) {
    function scrollController ($scope) {
        this.distanceFromTop = 0;
    }

    function scrollPostLink (scope, element, attrs, controller) {
        var theWindow = angular.element($window);
        var scrolling = 'enable';
        var throttleTime = 50;

        scope.distanceFromTop = 0;
    
        var preventDefault = function (e) {
            e = e || window.event;

            if (e.preventDefault) {
                e.preventDefault();
            }

            return false;
        };

        var cancelScroller = function () {
            if ($window.addEventListener) {
                $window.addEventListener('DOMMouseScroll', preventDefault, false);
            }

            $window.onwheel = preventDefault;
            $window.onmousewheel = document.onmousewheel = preventDefault;
            $window.ontouchmove  = preventDefault; 
        };

        var enableScroller = function () {
            if ($window.removeEventListener) {
                $window.removeEventListener('DOMMouseScroll', preventDefault, false);
            }

            $window.onmousewheel = document.onmousewheel = null; 
            $window.onwheel = null; 
            $window.ontouchmove = null;  
        };

        var handleScroller = function () {
            scope.distanceFromTop = controller.distanceFromTop = theWindow.scrollTop();
            scope.$apply();
        };

        // Disable scrolling if needed - attribute 'scroll-off', @type {Boolean}
        attrs.$observe('scrollOff', function (value) {
            if (value === 'false') {
                scrolling = 'enable';
                enableScroller();
            } else {
                scrolling = 'cancel';
                cancelScroller();
            }
        });

        //theWindow.on('scroll', _.throttle(handleScroller, throttleTime));
        theWindow.on('scroll', handleScroller);

        scope.$on('$destroy', function () {
            theWindow.off('scroll');
        });

       
    }
    return ({
        restrict: 'A',
        controller: scrollController,
        link: scrollPostLink
    });
});

/*
 - @name scrollEnter
 - @ngdoc directive
 *
 - @param {Object} $window
 - @param {Function} $parse 
 *
 - @description 
   Trigger a function when the window scroll position enters the bounds 
   of the directive element or a child element.
*
- @example
  <div scroll-enter on-enter-then="foo" use-boundary="middle"></div> 
  <div scroll-enter on-enter-then="foo" use-bound-box="true"></div>
  <div scroll-enter on-enter-then="foo" use-selector=".blah" use-boundary="middle></div>  
*/
// 01 attribute 'on-enter-then', @type {Function} - Function literal to execute 
// 02 attribute 'use-selector', @type {String} - Class or Id of child elements, used for applying an animation or transition to 
// multiple elements 
// 03 attribute 'use-boundary', @type {String} can be 'top'/'middle'/'bottom' - Element boundary to use
// 04 attribute 'use-viewport', @type {String} can be 'top'/'middle'/'bottom' - Viewport boundary to use 
// 05 attribute 'use-bound-box', @type {Boolean} - Override window scroll top and use the elements bounding box. Useful for elements that are
// positioned using 3D transforms 
app.directive('scrollEnter', function ($parse, $window) {
    function scrollEnterPostLink (scope, element, attrs, controller) {
        var callback = scope.onEnterThen || undefined;
        var selector = scope.useSelector || undefined;
        var boundary = scope.useBoundary || 'top';
        var viewport = scope.useViewport || 'bottom';
        var boundBox = !!scope.useBoundBox || false;
        var complete = false;
        var setState = false;
        var position = 'offset';
        var threshold = 10;
        var firstEle;
       
        var handleGet = function () {
            return controller.distanceFromTop;
        };

        var handleSet = function (scrollPosition, lastPosition) {
            if (selector) {
                var bounds;
                var boundsViewport;

                selector = element.find(selector);
                firstEle = selector.first();

                if (boundBox) {
                    bounds = firstEle[0].getBoundingClientRect();
                } else {
                    bounds = firstEle.offset();
                }

                switch (boundary) {
                    case "top":
                        bounds = bounds.top; 
                        break;
                    case "middle":
                        bounds = bounds.top + (firstEle.height() / 2);
                        break;
                    case "bottom":
                        bounds = bounds.top + firstEle.height();
                        break;
                }
            } else {
                bounds = element.offset();

                if (boundBox) {
                    bounds = element[0].getBoundingClientRect();
                } else {
                    bounds = element.offset(); 
                }

                switch (boundary) {
                    case "top":
                        bounds = bounds.top;
                        break;
                    case "middle":
                        bounds = bounds.top + (element.height() / 2);
                        break;
                    case "bottom":
                        bounds = bounds.top + element.height();
                        break;
                }
            }

            var playTween = function () {
                complete = true;

                // If there's no callback specified use default animations 
                if (!callback) {
                    if (selector) {
                        TweenMax.staggerTo(selector, 0.3,  {opacity: 1}, 0.1);
                    } else {
                        TweenMax.to(element, 0.3, {opacity: 1});  
                    }
                } else {
                    // Angular doesn't allow DOM elements to be directly passed to
                    // controllers (to avoid potential security risks). To keep things
                    // safe we return the current selector / element
                    function safePayLoad () {
                        if (selector) {
                            return selector;
                        } else {
                            return element;
                        }
                    }
                    // Run the callback and pass it the current element.
                    // Note:
                    // The current element needs to be passed to the function as a parameter 
                    // because this directive will create an isolated scope.
                    // If a directive has an isolated scope, then any other directive sharing the
                    // same element can't have an isolated scope. So, if we wanted to run multiple
                    // versions of the same function attached to the parent or root scope, 
                    // for example 'scope.render', then we need some way to tell that function what
                    // element it should be targeting.
                    if (angular.isFunction(callback())) {
                        callback()(safePayLoad);
                    }
                }
            };

            switch (viewport) {
                case "top":
                    boundsViewport = 0;
                    break;
                case "middle":
                    boundsViewport = $window.innerHeight / 2;
                    break;
                case "bottom":
                    boundsViewport = $window.innerHeight;
                    break;
            }

            // Adjust the 'scrollPostion', @type {Number}
            scrollPosition = scrollPosition + boundsViewport; 
        
            if (!setState && !callback) {
                setState = true;
                TweenMax.set(selector || element, {opacity: 0});
            }

            if (!boundBox) {
                //console.log('Scroll = %s / Bounds = %s / Complete = %s', scrollPosition, bounds, complete)
                if (scrollPosition >= (bounds - threshold) && !complete) {
                    playTween();
                }
            } else {
                if ((bounds + threshold) <= $window.innerHeight && !complete) {
                    playTween();
                }
            }
        };

        scope.$watch(handleGet, handleSet);
    }

    return ({
        restrict: 'A',
        require: '^scroll',
        scope: {
            onEnterThen: '&',
            useBoundary: '@',
            useBoundBox: '@',
            useSelector: '@',
            useViewport: '@'
        },
        link: scrollEnterPostLink
    });
});

/*
 - @name smartHeader
 - @ngdoc directive
 *
 - @param {Object} $window
 - @param {Function} $timeout
 - @param {Function} $parse 
 *
 - @description 
   Enables header to react to scroll postion and route.
*
- @example
  <header smart-header="distanceFromTop" format-style="page"></div>
*/
app.directive('smartHeader', function ($timeout, $parse, $window) {
    function smartHeaderPostLink (scope, element, attrs) {
        var oldHeaderHeight = element.outerHeight(); 
        // When location is 'home' style header appropriately. 
        // Could be extended for other locations - attribute 'format-style', @type {String} can be 'home'
        var styleHeaderWhen = scope.formatStyle || undefined;
        
        scope.$watch('topDistance', function (distanceFromTop) {
            if (distanceFromTop > oldHeaderHeight) {
                element.addClass('is-active');
            } else {
                element.removeClass('is-active');
            }

            // TODO: This needs to be pulled out - an elegant solution to this
            // has proven to be tricky
            if (styleHeaderWhen && styleHeaderWhen === 'home') {
                if (distanceFromTop >= $window.innerHeight) {
                    element.addClass('on-white');
                } else {
                    element.removeClass('on-white'); 
                }
            }

        });
    }

    return ({
        restrict: 'A',
        scope: {
            topDistance: '=smartHeader',
            formatStyle: '='
        },
        link: smartHeaderPostLink
    });
});

/*
 - @name mediaLoad
 - @ngdoc directive
 *
 - @param {Object} $window
 - @param {Object} requestQueue
 *
 - @description 
   Trigger a custom event when the image or svg has loaded. 
*
- @example
  <img ng-src="foo-blah.png" media-load after-load="foo() queue-load='true'" />
*/

// 01 attribute 'after-load', @type {Function} - Function to execute when the load has completed 
// 02 attribute 'queue-load', @type {Boolean} - Add load to global queue for site load progess
app.directive('mediaLoad', function ($parse, requestQueue) {
    function imageOnLoadPostLink (scope, element, attrs) {
        var mediaPath = attrs.ngSrc || attrs.src;
        var afterLoad = attrs.afterLoad ? $parse(attrs.afterLoad) : undefined;
        var queueLoad = attrs.queueLoad || false;
        
        queueLoad = !!queueLoad;
        mediaPath = mediaPath.split('/')[mediaPath.split('/').length - 1];

        var handleSet = function () {
            scope.$emit('media-loaded', {path: mediaPath});
            if (!!queueLoad) {
                requestQueue.survey();
            }

            if (afterLoad) {
                afterLoad(scope);
            }
        };

        if (queueLoad) {
            requestQueue.increment();
        }

        element.on('load', handleSet);

        scope.$on('$destroy', function () {
            element.off('load');
        });

    }
    return ({
        restrict: 'A',
        link: imageOnLoadPostLink
    });
});

/*
 - @name makeSquare
 - @ngdoc directive
 *
 - @description 
   Set element height and width to be the same.
   Useful for responsive elements that need have a 1:1 ratio
*
- @example
  <div make-square></div>
*/
app.directive('makeSquare', function () {
    function makeSquarePostLink (scope, element, attrs) {
        var maxDimension = Math.max(element.width(), element.height());
        element.height(element.width());

        var handleGet = function () {
            return element.width();
        };

        var handleSet = function (newValue, oldValue) {
            maxDimension = Math.max(element.width(), element.height());

            if (newValue !== oldValue) {
                element.height(element.width());
            } else {
                return;
            }
        };

        scope.$watch(handleGet, handleSet);

    }   
    return ({
        restrict: 'A',
        link: makeSquarePostLink
    });
});

/*
 - @name makeSticky
 - @ngdoc directive
 *
 - @description
   Enable an element to maintain an explicit position when the document is scrolled.
   Useful for menus that need to stay visible when a user is scrolling. 
*
- @example
  <div make-sticky></div>
*/
app.directive('makeSticky', function () {
    function makeStickyPostLink (scope, element, attrs, controller) {
        var initState = false;

        var handleGet = function () {
            return controller.distanceFromTop;
        };

        var handleSet = function (scrollPosition) {
            if (!initState) {
                initState = true;
                element.addClass('is-sticky');
            }

            if (scrollPosition === 0) {
                initState = false;
                element.removeClass('is-sticky');
            }

            element.css('transform', 'translate3d(0,' + scrollPosition + 'px' + ', 0)');
               
        };  

        scope.$watch(handleGet, _.debounce(handleSet, 100));
 
    }
    return ({
        restrict: 'A',
        require: '^scroll',
        link: makeStickyPostLink
    });
});

/*
 - @name svgAnimate
 - @ngdoc directive
 *
 - @param {Object} $window
 - @param {Function} $parse 
 - @param {Function} $timeout
 - @param {Object} jsonLoader
 *
 - @description 
   Setup and run a 'bodymovin' svg animation.
   Used in conjunction with the 'scrollEnter' directive
*
- @example
  <div svg-animate svg-uid="unique-svg-id" json-path="/animations/foo.json" scroll-enter on-enter-then="render" use-boundary="middle"></div>
*/
// 01 attribute 'json-path', @type {String} - Relative path to the 'json' file, which is exported from the Adobe
// After Effects companion plugin 
// 02 attribute 'svg-uid', @type {String} - A unique svg identifier so we can play multiple animations 
// on the same page 
app.directive('svgAnimate', function ($timeout, $parse, $window, httpLoader) {
    function svgAnimatePostLink (scope, element, attrs, controller) {
        var animateOpt;
        var pathToJson = attrs.jsonPath;
        var animateUID = attrs.svgUid || '';
        var delayStart = 300;

        var handleLoad = function (response) {
            var animateOpt = {
                wrapper: element[0],
                animType: 'svg',
                loop: false,
                autoplay: false,
                prerender: true,
                animationData: response.data
            };
            
            scope.render = function (ele) {
                var elementSVG = angular.element(ele());
                var animateUID = elementSVG.attr('svg-uid');

                $timeout(function(){
                    bodymovin.play(animateUID);
                }, delayStart);
            };

            $timeout(function() {
                bodymovin.loadAnimation(animateOpt, animateUID);
                bodymovin.stop();
            });
        };

        // Request the json settings and add each request to the site progress queue
        httpLoader.get(pathToJson, 'queue').then(handleLoad);
        scope.$on('$destroy', bodymovin.destroy);
    }
    return ({
        restrict: 'A',
        require: '^scroll',
        link: svgAnimatePostLink
    });
});

/*
 - @name combineAnimate
 - @ngdoc directive
 *
 - @param {Function} $timeout
 - @param {Object} numberUtils
 *
 - @description 
   Specific directive with logic for the parallax animation used in 
   the 'combine' section of the landing page. 
*
- @example
  <div combine-animate></div>
*/
app.directive('combineAnimate', function($timeout) {
    function combineAnimatePostLink (scope, element, attrs) {
        function setupAnimate () {
            var duration = 1;
            var container = element.closest('.tour-combine');
            var offsetTop = container.offset().top; 
            var imgsMedia = element.find('.tour__media');
            var imgEleTop = element.find('img.top-layer');
            var imgEleMid = element.find('img.mid-layer');
            var imgEleBot = element.find('img.bot-layer');
            var eleHeight = container.height();
    
            // Set the height of the parent container, wait until the images have loaded.
            imgsMedia.height(imgEleBot.height());
            
            scope.animate = function () {
                TweenMax.to(imgEleTop, duration, {y: -40, ease: Sine.easeOut});
                TweenMax.to(imgEleMid, duration, {y: 40, ease: Sine.easeOut});
                TweenMax.to(imgEleBot, duration, {y: 80, ease: Sine.easeOut});
            };

            TweenMax.set(imgEleTop, {y: 80});
            TweenMax.set(imgEleMid, {y: 0});
            TweenMax.set(imgEleBot, {y: -80});
        };

        $timeout(setupAnimate, 250);
    }
    return ({
        restrict: 'A',
        link: combineAnimatePostLink
    });
});

/*
 - @name embedAnimate
 - @ngdoc directive
 *
 - @param {Function} $timeout
 - @param {Object} scrollDelta
 - @param {Object} numberUtils
 *
 - @description 
   Similar to @name combineAnimate - TODO: could be merged at some point
*
- @example
  <div embed-animate></div>
*/
app.directive('embedAnimate', function ($timeout, numberUtils) {
    function embedAnimatePostLink (scope, element, attrs, controller) {
        function setupAnimate () {
            var oldRange;
            var newRange; 
            var imgsMedia = element.find('.tour__media');
            var imgEleMac = element.find('img.frame-imac');
            var imgElePad = element.find('img.frame-ipad');
            var imgEleMob = element.find('img.frame-android');
            var topBotPad = parseInt(element.css('padding-top')) + parseInt(element.css('padding-bottom'));
            var eleHeight = element.height() + topBotPad;
            var offsetTop = element.offset().top - topBotPad;
            var setState = false;
            var complete = false;
            var lastSpot = 0;
        
            var getRange = {
                valueNew: function () {
                    var max = eleHeight;
                    var min = 0;
                    return numberUtils.range(min, max);
                },
                valueOld: function() {
                    var max = angular.element(window).height();
                    var min = 0;
                    return numberUtils.range(min, max);
                }
            };

            // Cache the range
            oldRange = getRange.valueOld();
            newRange = getRange.valueNew();


            // Check what direction the user is scrolling 
            var direction = function (value) {
                var scrolling = '';

                if (value > lastSpot) {
                    scrolling = 'down';
                } else {
                    scrolling = 'up';
                }

                lastSpot = value;
                return scrolling;
            }

            var handleGet = function () {
                return controller.distanceFromTop;
            };

            var handleSet = function (distance) {
                var delta = 1;
                var value = distance - (offsetTop - (eleHeight / 2));
                var remap = numberUtils.transform(oldRange, newRange).map(value);
                
                var runIt = function () {
                    TweenMax.staggerTo([imgEleMac, imgElePad, imgEleMob], delta, {x: 0, force3D: true, opacity: 1, ease: Back.easeOut}, 0.33);
                };
                
                if (!setState) {
                    TweenMax.set([imgEleMac, imgElePad, imgEleMob], {x: '144px', opacity: 0});
                    setState = true;
                }

                if (!complete && remap > 0 && direction(value) === 'down') {
                    runIt();
                    complete = true;
                }
                
            };

            scope.$watch(handleGet, handleSet);
        }

        $timeout(setupAnimate);
    }

    return ({
        restrict: 'A',
        require: '^scroll',
        link: embedAnimatePostLink
    });
});


/*
 - @name ensureEvenDimensions
 - @ngdoc directive
 *
 - @param {Function} $timeout
 *
 - @description 
   Used on elements that are centered via 3D transform. If the element's
   height is odd the anti-aliasing will not be correct leading to
   blurry child elements.
*
- @example
  <div ensure-even-dimensions use-dimension="width"></div>
*/
// attribute 'use-dimension', @type {String} can be 'height'/'width'/'both'
app.directive('ensureEvenDimensions', function ($timeout) {
    function ensureEvenDimensionsPostLink (scope, element, attrs) {
        var useDimension = attrs.useDimension || 'height';
        var setDimension = function () {
            var elementWid = element.width();
            var elementHei = element.height();

            var ensureEven = function (oldDimension) {
                var newDimension;

                if (oldDimension % 2 === 0) {
                    newDimension = oldDimension;
                } else {
                    newDimension = Math.floor(oldDimension / 2) * 2;
                }

                return newDimension;
            }; 

            if (useDimension === 'width') {
                element.css('width', ensureEven(elementWid));
            } else if (useDimension === 'height') {
                element.css('height', ensureEven(elementHei));
            } else {
                element.css('width', ensureEven(elementWid));
                element.css('height', ensureEven(elementHei));
            }
        };

        $timeout(setDimension, 50);

    }
    return ({
        restrict: 'A',
        link: ensureEvenDimensionsPostLink
    });
});

/*
 - @name truncateContent
 - @ngdoc directive
 *
 - @param {Function} $timeout
 *
 - @description 
   Used on elements that are centered via 3D transform. If the element's
   height is odd the anti-aliasing will not be correct leading to
   blurry child elements.
*
- @example
  <div ng-bind-html="ctrl.renderHtml(post.text)" truncate-content></div>
*/
app.directive('truncateContent', function ($timeout) {
    function truncateContentPostLink (scope, element, attrs, controller) {
        scope.isTruncated = false;

        var truncateOpt = {
            truncateAfterLinks: true, 
            maxLines: 2,
            truncateString: '...'
        };

        var handleGet = function () {
            return element.html();
        };

        var handleSet = function (value) {
             if (!scope.isTruncated) {
                $timeout(function(){
                    element.truncate(truncateOpt);
                }, 100);
               
                scope.isTruncated = true;
            }
        }

        scope.$watch(handleGet, handleSet);
       
    }

    return ({
        required: 'ngBindHtml',
        restrict: 'A',
        priority: 100,
        link: truncateContentPostLink
    });
});










