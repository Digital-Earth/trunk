app.directive('setPosition', function ($q, $timeout, $parse) {

    var callbacks = [];

    function setPositionPostLink (scope, element, attrs) {
            var onSuccess;
            var onFailure;
            var immediate = attrs.immediate || false;
            var useRegion = $parse(attrs.setPosition)(scope);
            var _position = element.css('position');
            var parentEle = element.parent();
            var mediaList = element.find('[media-load]');
            var loaderObj = {};
                
            if (_position === 'fixed') {
                parentEle = angular.element('body');
            }

            var valueType = function (value) {
                return value.match(/(%|px)$/)[0] || 'px';
            };

            var buildOpts = function (prop, props, dims) {
                if (prop !== 'center') {
                    var optsValue = parseInt(useRegion[prop]);
                    var checkType = valueType(useRegion[prop]);

                    if (checkType === '%') {
                        optsValue = optsValue / 100;
                        props[prop] = (dims['wid'] * optsValue) * -1;
                    } else if (checkType === 'px') {
                        props[prop] = (dims['wid'] - optsValue) * -1;
                    } 
                } else {

                    var prev = element.prev().length ? element.prev() : element.parent();
                    var next = element.next().length ? element.next() : element.parent();
                    
                    console.log(Math.round(next.position().top - prev.position().top) - dims['hei'])

                }

            };
        
            var setPosition = function () {
                var dimension = {};
                var styleOpts = {};

                dimension['wid'] = element.width();
                dimension['hei'] = element.height();

                if ('top' in useRegion) {
                    buildOpts('top', styleOpts, dimension);
                }

                if ('bottom' in useRegion) {
                    buildOpts('bottom', styleOpts, dimension);
                }

                if ('left' in useRegion) {
                    buildOpts('left', styleOpts, dimension);
                }

                if ('right' in useRegion) {
                    buildOpts('right', styleOpts, dimension);
                }

                if ('center' in useRegion) {
                    buildOpts('center', styleOpts, dimension);
                }
              
                element.css(styleOpts);
            };  

            loaderObj['callback'] = setPosition;
            loaderObj['runOnLoad'] = !!immediate;

            callbacks.push(loaderObj);

            scope.setPosition = function (load) {
                for (var i = 0, total = callbacks.length; i < total; i += 1) {
                    var callback = callbacks[i];

                    if (load && callback.onLoad) {
                        continue;
                    } else {
                        callback['callback']();
                    }
                }
            };

            if (!!immediate) {
                setPosition();
            }
    
    }

    return {
        restrict: 'A',
        scope: {},
        link: setPositionPostLink
    }
});

app.service('animateFX', function() {
    var options = {
        props: {},
        setup: '',
        start: ''
    };

    var methods = {
        library: {},
        register: function (name, settings) {
            this.library[name] = angular.extend({}, options, settings);
        }
    };

    methods.library['slide-in'] = {
        props: {},
        setup: function (element, options) {
            if ('bearing' in this) {
                switch (this.bearing) {
                    case 'bottom':
                        this.props.origin = {y: 100};
                        this.props.finish = {y: 0}; 
                    break;
                    case 'top':
                        this.props.origin = {y: -100};
                        this.props.finish = {y: 0}; 
                    break;
                    case 'left':
                        this.props.origin = {x: -100};
                        this.props.finish = {x: 0}; 
                    break;
                    case 'right':
                        this.props.origin = {x: 100};
                        this.props.finish = {x: 0}; 
                    break;
                    case 'top':
                        this.props.origin = {y: 100};
                        this.props.finish = {y: 0}; 
                    break;
                }
            }

            TweenMax.set(element, this.props.origin);
        },
        start: function (element, options) {    
            TweenMax.to(element, 1, this.props.finish);
        }
    };

    methods.library['fade-in'] = {
            setup: function (element, options) {
                TweenMax.set(element, {autoAlpha: 0});
            },
            start: function (element, options) {    
                console.log('start')
                TweenMax.to(element, 1, {autoAlpha: 1});
            }
        };

    methods.library['fade-out'] = {
        setup: function (element, options) {
            TweenMax.set(element, {autoAlpha: 1});
        },
        start: function (element, options) {
            TweenMax.to(element, 1, {autoAlpha: 0});
        }
    };

    return methods;
});

app.directive('animate', function($animate, $timeout, animateFX) {
    function animateCompile (tElement, tAttrs) {
        var onEnter = tAttrs.onEnter || 'default-enter';
        var onLeave = tAttrs.onLeave || 'default-leave';
        var onEvent = tAttrs.onEvent || undefined;
        var animate = animateFX.library;
        var delimit = ':';
        var bearing = false;
        var setting;

        if (onEnter.indexOf(delimit) > -1) {
            setting = onEnter.split(delimit);
            onEnter = setting[0];
            bearing = setting[1];
        }
        
        var animatePreLink = function (scope, element, attrs) {
            if (onEnter in animate) {
                if (bearing) {
                  animate[onEnter].bearing = bearing;  
                }
                animate[onEnter].setup(element);
            }
            if (onLeave in animate) {
                animate[onLeave].setup(element);
            }
        };

        var animatePostLink = function (scope, element, attrs) {
            var parent = element.parent();
            var doLeave, doEnter;

            var options = attrs.options || {};
            if (onEnter in animate) {
                animate[onEnter].start(element, options);
            } 
            if (onLeave in animate) {
                animate[onLeave].start(element, options);
            }

            doLeave = function () {
               
            };

            doEnter = function () {
            
            };
            // $timeout(function(){
            //     $animate.enter(element);
            // }, 1000);

        };


        return ({
            pre: animatePreLink,
            post: animatePostLink

        });
    }

    return ({
        restrict: 'A',
        scope: {
            onEnter: '@',
            onLeave: '@'
        },
        compile: animateCompile
    });
});

/*
 - @name combineAnimate
 - @ngdoc directive
 *
 - @param {Function} $timeout
 - @param {Object} scrollDelta
 - @param {Object} numberUtils
 *
 - @description 
   Specific directive with logic for the parallax animation used in 
   the 'combine' section of the landing page. 
*
- @example
  <div combine-animate></div>
*/
app.directive('combineAnimate', function($timeout, numberUtils) {
    function combineAnimatePostLink (scope, element, attrs, controller) {

        function setupAnimate () {
            var oldRange;
            var newRange;
            var container = element.closest('.tour-combine');
            var offsetTop = container.offset().top; 
            var imgsMedia = element.find('.tour__media');
            var imgEleTop = element.find('img.top-layer');
            var imgEleMid = element.find('img.mid-layer');
            var imgEleBot = element.find('img.bot-layer');
            var eleHeight = container.height();
            var setState = false;
            var lastSpot = 0;

            // Set the height of the parent container after the images have loaded.
            // Used in conjunction with the mediaLoad directive.
            scope.setHeight = function () {
                imgsMedia.height(imgEleTop.height());
            }

            // We need to convert the range, because we'll want to kick-off the animation when the 
            // container enters the viewport. More importantly, this allows the animation to be 
            // driven by the converted scroll position.
            var getRange = {
                valueNew: function () {
                    var max = container.height();
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
                //var value = distance - offsetTop;
                var delta = 0.4;
                var value = distance - (offsetTop - eleHeight);
                var remap = numberUtils.transform(oldRange, newRange).map(value);
                var alpha = numberUtils.transform(newRange, numberUtils.range(0, 1)).map(value);
                
                if (direction(value) === 'down') {
                    if (remap >= (eleHeight / 3)) {
                        remap = eleHeight;
                    }
                } else {
                    delta = 1;
                    if (remap < eleHeight) {
                        remap = 0;
                        alpha = 0;
                    }
                }
              
                var runIt = function () {
                    TweenMax.to(imgEleTop, delta, {y: Math.round(remap - eleHeight) + 'px', opacity: alpha, force3D: true, ease: Sine.easeOut});
                    TweenMax.to(imgEleMid, delta, {x: Math.round(eleHeight - remap) + 'px', opacity: alpha, force3D: true, ease: Sine.easeOut});
                    TweenMax.to(imgEleBot, delta, {y: Math.round(eleHeight - remap) + 'px', opacity: alpha, force3D: true, ease: Sine.easeOut});

                };

                if (!setState) {
                    runIt();
                    TweenMax.set([imgEleTop, imgEleMid, imgEleBot], {opacity: 0});
                    setState = true;
                }

                if (remap === 0 & remap >= eleHeight) {
                    return;
                }

                _.debounce(runIt, 100)();

            };

            scope.$watch(handleGet, handleSet);
        };

        $timeout(setupAnimate);
    }
    return ({
        restrict: 'A',
        require: '^scroll',
        link: combineAnimatePostLink
    });
});



/*
        
        // var sequence = new TimelineMax();
        // var children = element.find('img');

        // sequence.from(children[0], 1.0, {yPercent: -100, scale: 0.9, opacity: 0, force3D: true, ease: Sine.easeOut});
        // sequence.from(children[1], 0.6, {xPercent: -100, opacity: 0, force3D: true, ease: Circ.easeOut}, '-=0.3');
        // sequence.from(children[2], 0.6, {yPercent:  100, opacity: 0, force3D: true, ease: Sine.easeOut}, '-=0.3')

        // scope.sequencePlay = function () {
        //     sequence.play();
        // };

        // sequence.stop();


   <div class="discover__content" scroll-enter scroll-enter-then="discoverCardMovie(nodes)" use-boundary="middle" use-selector=".discover-card">
            <div class="discover-card">
                <div class="discover-card__inner">
                    <div class="discover-card__media" ng-style="{'background-image':'url(../assets/img-card-demo01.jpg)'}"></div>
                    <div class="discover-card__content">
                        <div class="discover-card__title">Weather Satellite Infrared</div>
                        <div class="discover-card__owner caption-dark">NOAA</div>
                    </div>
                </div>
            </div>
*/

