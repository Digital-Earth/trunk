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
  <div class="banner surface" resize resize-set="height" resize-if="greater-window-height" resize-run="foo()"></div>
*/
// attribute 'resize-set', @type {String} can be 'height'/'width'/'both'
app.directive('resize', function ($timeout, $window, $parse, Modernizr, ViewportCutoffs) {
    function resizePostLink(scope, element, attrs) {
        var theWindow = angular.element($window);
        var resizeSet = attrs.resizeSet || 'none';
        var resizeRun = attrs.resizeRun ? $parse(attrs.resizeRun) : false;
        var excludeFromCutoff = attrs.excludeFromCutoff || false;
        var resizeIf = attrs.resizeIf ? attrs. resizeIf : false;

        excludeFromCutoff = !!excludeFromCutoff;

        var handleGet = function () {
            return {
                height: theWindow.height(),
                width: theWindow.width()
            }
        };

        // Apply window dimension(s) to element dimension(s)
        var handleSet = function (newValue, oldValue) {
            var cutoffTablet = newValue.width < ViewportCutoffs.smallLaptop;
            var cutoffLaptop = newValue.width <= ViewportCutoffs.laptop;
            
            if (!resizeIf) {
                // Don't size the element
                if (cutoffLaptop && !excludeFromCutoff) {
                    return;
                }

                if (cutoffTablet) {
                    return;
                } 

            } else {
                switch(resizeIf) {
                    case 'greater-window-height' :
                        if (element.outerHeight(true) <= newValue.height) {
                            element.css(resizeSet, "");
                            return;
                        }
                        break;
                    case 'greater-window-width':
                      if (element.outerWidth(true) <= newValue.width) {
                            element.css(resizeSet, "");
                            return;
                        }
                        break;
                }
            }

            switch (resizeSet) {
                case 'height':
                    element.css('height', newValue.height);
                    console.log("SET HEIGHT", element);
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
 - @param {Function} $timeout
 *
 - @description 
   Watches the window scroll top property.
*
- @example
  <body scroll scroll-off="{{obj.prop}}"></body>
*/
app.directive('scroll', function ($window, $parse, $timeout, Modernizr) {
    function scrollController ($scope) {
        this.distanceFromTop = 0;
    }

    function scrollPostLink (scope, element, attrs, controller) {
        var theWindow = angular.element($window);
        var scrolling = 'enable';
        var hammerEle;
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
            if (Modernizr.touch) {
                scope.distanceFromTop = controller.distanceFromTop = window.pageYOffset;
            } else {
            scope.distanceFromTop = controller.distanceFromTop = theWindow.scrollTop();
            }
            scope.$apply();
        };

        // Disable scrolling if needed - attribute 'scroll-off', @type {Boolean}
        attrs.$observe('scrollOff', function (value) {
            if (value === 'false') {
                scrolling = 'enable';
                angular.element('body').removeClass('modal-is-open');
                enableScroller();
            } else {
                scrolling = 'cancel';
                $timeout(function(){
                    angular.element('body').addClass('modal-is-open');
                }, 100);
                cancelScroller();
            }
        });

        if (Modernizr.touch) {
            theWindow.on('touchmove', handleScroller);
        } else {
            theWindow.on('scroll', _.throttle(handleScroller, 100));
        }

        scope.$on('$destroy', function () {
            theWindow.off('scroll touchmove');
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
       
        // Because "ng-view" is wrapped in a switch statement
        // it's a known bug that Angular will call Directive
        // postLink functions twice
        if (!element.length) {
            return;
        }

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

app.directive('name', [function () {
    return {
        restrict: 'A',
        link: function (scope, iElement, iAttrs) {
            
        }
    };
}])
/*
 - @name combineAnimate
 - @ngdoc directive
 *
 - @param {Function} $timeout
 - @param {Object} EnableFeatures
 *
 - @description 
   Specific directive with logic in the 'combine' section of the landing page. 
    (TODO: combine with other Animate directives)
*
- @example
  <div combine-animate></div>
*/
app.directive('combineAnimate', function($timeout, EnableFeatures) {
    // Because "ng-view" is wrapped in a switch statement
    // it's a known bug that Angular will call Directive
    // postLink functions twice
    var hasBeenExecuted = false;

    function combineAnimatePostLink (scope, element, attrs) {
        var timer;
        var duration = 0.6;
        var distance = 80;
        var container = element.closest('.tour-combine');
        var imgsMedia = element.find('.tour__media');
        var imgEleTop = element.find('img.top-layer');
        var imgEleMid = element.find('img.mid-layer');
        var imgEleBot = element.find('img.bot-layer');
        var combineHeight = 0;

        function setupAnimate () {
            hasBeenExecuted = true;
         
            var offsetTop = container.offset().top; 
            var eleHeight = container.height();
    
            // Set the height of the parent container, wait until the images have loaded.
            // This function will get called a couple of times because on slow connections
            // the images take longer to load, so the parent elements height is 0
            scope.setCombineHeight = function (resize) {
                var mediaHeight;

                if (resize) {
                    imgsMedia.height(imgEleBot.height());
                    return;
                }
            
                imgsMedia.height(imgEleBot.height());

                mediaHeight = imgsMedia.height();

                combineHeight = mediaHeight > combineHeight ? mediaHeight : combineHeight;
                imgsMedia.height(combineHeight);
            };

            // Set the height 
            scope.setCombineHeight();

            scope.animateCombine = function (ele) {
                TweenMax.to(imgEleTop, duration, {y: -(distance / 2), ease: Quad.easeOut});
                TweenMax.to(imgEleMid, duration, {y: (distance / 2), ease: Quad.easeOut});
                TweenMax.to(imgEleBot, duration, {y: distance, ease: Quad.easeOut});

                hasBeenExecuted = false;
            };

            TweenMax.set(imgEleTop, {y: distance});
            TweenMax.set(imgEleMid, {y: 0});
            TweenMax.set(imgEleBot, {y: -distance});
        };

        if (!hasBeenExecuted && EnableFeatures.animationDOM) {
            timer = $timeout(setupAnimate, 500);
        } else {
            timer = $timeout(function() {
                imgsMedia.height(imgEleBot.height());
                TweenMax.set(imgEleTop, {y: -(distance / 2)});
                TweenMax.set(imgEleMid, {y: (distance / 2)});
                TweenMax.set(imgEleBot, {y: distance});
            }, 500);
    }

        scope.$on('$destroy', function(){
            $timeout.cancel(timer);
        });
        
    }
    return ({
        restrict: 'A',
        link: combineAnimatePostLink
    });
});

/*
 - @name discoverAnimate
 - @ngdoc directive
 *
 - @param {Function} $timeout
 *
 - @description 
   Specific directive with logic in the 'discover' section of the landing page. 
   (TODO: combine with other Animate directives)
*
- @example
  <div discover-animate></div>
*/
app.directive('discoverAnimate', function($timeout, EnableFeatures) {
    // Because "ng-view" is wrapped in a switch statement
    // it's a known bug that Angular will call Directive
    // postLink functions twice
    var hasBeenExecuted = false;

    function discoverAnimatePostLink (scope, element, attrs) {
        var timer;

        function setupAnimate () {
            hasBeenExecuted = true;

            var duration = 0.6;
            var cards = element.find('.discover-card');
            var total = cards.length;
        
            var randomNum = function () {
                var number;
                number  = Math.floor(Math.random() * 2999) + 1;
                number *= Math.floor(Math.random() * 2) === 1 ? 1 : -1; 
                return number;
                }

            for (var i = 0; i < total; i++) {
                TweenMax.set(cards[i], {x: randomNum(), y: randomNum(), opacity: 0});
            }

            scope.animateDiscover = function () {
                console.log('discover')
                TweenMax.staggerTo(cards, duration, {x: 0, y: 0, clearProps: 'x, y, opacity', opacity: 1, ease: Sine.easeOut}, 0.2);
                hasBeenExecuted = false;
            };
        };

        if (!hasBeenExecuted && EnableFeatures.animationDOM) {
            timer = $timeout(setupAnimate);
        }

        scope.$on('$destroy', function(){
            $timeout.cancel(timer);
        });
    }
    return ({
        restrict: 'A',
        link: discoverAnimatePostLink
    });
});

/*
 - @name embedAnimate
 - @ngdoc directive
 *
 - @param {Function} $timeout
 *
 - @description 
   Specific directive with logic in the 'embed' section. 
   (TODO: combine with other Animate directives)
*
- @example
  <div embed-animate></div>
*/
app.directive('embedAnimate', function ($timeout, EnableFeatures) {
    var hasBeenExecuted = false;

    function embedAnimatePostLink (scope, element, attrs) {
        var timer;

        function setupAnimate () {
            hasBeenExecuted = true;

            var delta = 1;
            var imgsMedia = element.find('.tour__media');
            var imgEleMac = element.find('img.frame-imac');
            var imgElePad = element.find('img.frame-ipad');
            var imgEleMob = element.find('img.frame-android');
            var topBotPad = parseInt(element.css('padding-top')) + parseInt(element.css('padding-bottom'));
            var eleHeight = element.height() + topBotPad;
            var offsetTop = element.offset().top - topBotPad;
            
            TweenMax.set([imgEleMac, imgElePad, imgEleMob], {x: '144px', opacity: 0});
                
            scope.animateEmbed = function (distance) {
                TweenMax.staggerTo([imgEleMac, imgElePad, imgEleMob], delta, {x: 0, force3D: true, opacity: 1, ease: Back.easeOut}, 0.33);
                hasBeenExecuted = false;
            };
        }
                
        if (!hasBeenExecuted && EnableFeatures.animationDOM) {
            timer = $timeout(setupAnimate);
        }

        scope.$on('$destroy', function(){
            $timeout.cancel(timer);
        });
    }

    return ({
        restrict: 'A',
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

        $timeout(setDimension, 100);

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

/*
 - @name loading
 - @ngdoc directive
 *
 - @description 
   A simple directive aimed at stopping FOUC for elements
   that shouldn't be visible unless triggered by an event
   for example wv-alerts.
*
- @example
  <body loading></body>
*/
app.directive('loading', function () {
    function loadingPostLink (scope, element, attrs) {
        element.addClass('is-loading');
        scope.$on('$viewContentLoaded', function() {
            element.removeClass('is-loading');
        });
    }
   return ({
        restrict: 'A',
        link: loadingPostLink
    });
});

