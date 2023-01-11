/*
!!!!!!!!!!  CURRENTLY IN PROTOTYPE
!!!!!!!!!!  PLEASE IGNORE THIS FILE FOR CODE REVIEW UNTIL
!!!!!!!!!!  MOVED FROM PROTOTYPE TO PRODUCTION

worldViewStudioController aimed to be the skin for web demos (not fully working studio)
*/
app.controller("worldViewStudioController", function(
    $scope, $pyx, $pyxconfig, worldViewStudioBootstrap,
    $window, $timeout, $location,
    userPlatform,
    featureInspectorTools,
    featureEmailCapture) {

    $scope.excludedTemplates = ["/client/templates/studio/template/feature-dashboard.html"];

    worldViewStudioBootstrap($scope, {
        mode: 'viewer'
    });

    // check if user is on a device that supports WorldView Studio
    $scope.isSupported = userPlatform.isSupported('WorldView');

    // temporary debug
    $window.$scope = $scope;

    featureInspectorTools.register($scope);
    featureEmailCapture.register($scope);

    $scope.uiActive = true;
    $scope.uiReadonly = true;

    //Webgl support 
    $scope.supports = {};
    $scope.supports['webgl'] = (function() {
        try {
            var canvas = document.createElement('canvas');
            return !!(window.WebGLRenderingContext && (canvas.getContext('webgl') || canvas.getContext('experimental-webgl')));
        } catch (e) {
            return false;
        }
    })();

    $scope.demoInfo = {
        exploreIsActive: false,
        tourIsActive: false,
        overlayIsActive: false,
        showLegend: false,
        showCloseModalButton: false,
        maps: {},
        set: function(key, state) {
            return $scope.demoInfo[key] = state;
        },
        stopGlobeSpin: function() {},
        hideOverlay: function() {
            $scope.demoInfo.set('overlayIsActive', false);
        }
    };

    $scope.reloadPage = function() {
        $window.location.reload();
    };

    $scope.shareTour = {
        isActive: false
    };

    $scope.globeTour = {
        sharkAttacks: {
            onComplete: '',
            steps: [{
                nextEvent: null,
                headline: "Feast on the world's shark attacks!",
                caption: 'To search out the tastiest attacks use the legend in connection with the globe. <span class="italic">Click on one and the other reacts.</span>',
                element: '[data-id="2426f917-455a-4351-8d34-411a580af45c"]',
                situate: 'left',
                arrowUrl: '/site/assets/bubble-arrow-down-left.svg',
                offsets: {
                    bubble: {
                        top: -84,
                        left: 16
                    },
                    arrow: {
                        width: 48,
                        top: 128,
                        left: -28
                    }
                }
            }, {
                nextEvent: null,
                headline: "Explore",
                caption: 'Dive in by <span class="italic">scrolling, zooming, tilting and rotating</span> the globe. You can also use the controls to surf around.',
                element: '.nav-tools',
                situate: 'right-top',
                buffer: 16,
                arrowUrl: '/site/assets/bubble-arrow-down-right.svg',
                offsets: {
                    bubble: {
                        top: -20,
                        right: -16
                    },
                    arrow: {
                        width: 64,
                        top: 20,
                        right: -56
                    }
                }
            }, {
                nextEvent: null,
                headline: "Share",
                caption: 'Once you’ve customized your globe <span class="italic">send it</span> to your colleagues, classmates and friends. Sharing shark attacks is good karma.',
                element: '.static-legend__header',
                situate: 'left',
                arrowUrl: '/site/assets/bubble-arrow-up-left.svg',
                offsets: {
                    bubble: {
                        top: 124,
                        left: 30
                    },
                    arrow: {
                        width: 64,
                        top: -33,
                        left: -56
                    }
                }
            }]
        },
        campgrounds: {
            steps: [{
                nextEvent: null,
                headline: "Explore nature from the comforts of home.",
                caption: 'To search out North America’s campgrounds use the legend in connection with the globe. <span class="italic">Click on one and the other reacts.</span>',
                element: '[data-id="6c733647-8e86-454f-a4d9-6a8ed153347d"]',
                situate: 'left',
                arrowUrl: '/site/assets/bubble-arrow-down-left--short.svg',
                offsets: {
                    bubble: {
                        top: -74,
                        left: 16
                    },
                    arrow: {
                        width: 48,
                        top: 160,
                        left: -32
                    }
                }
            }, {
                nextEvent: null,
                headline: "Explore",
                caption: 'Hike around by <span class="italic">scrolling, zooming, tilting and rotating</span> the globe. You can also use the controls to surf around.',
                element: '.nav-tools',
                situate: 'right-top',
                buffer: 16,
                arrowUrl: '/site/assets/bubble-arrow-down-right.svg',
                offsets: {
                    bubble: {
                        top: -20,
                        right: -16
                    },
                    arrow: {
                        width: 64,
                        top: 20,
                        right: -56
                    }
                }
            }, {
                nextEvent: null,
                headline: "Share",
                caption: 'Once you’ve customized your globe <span class="italic">send it</span> to your colleagues, classmates and friends. Then everybody can plan their next excursion.',
                element: '.static-legend__header',
                situate: 'left',
                arrowUrl: '/site/assets/bubble-arrow-up-left.svg',
                offsets: {
                    bubble: {
                        top: 124,
                        left: 30
                    },
                    arrow: {
                        width: 64,
                        top: -33,
                        left: -56
                    }
                }
            }]
        },
        temperature: {
            steps: [{
                nextEvent: null,
                headline: "Take in the world’s temperatures.",
                caption: 'To search out weather near and far use the legend in connection with the globe. <span class="italic">Click on one and the other reacts.</span>',
                element: '[data-id="033b0de3-5050-40cb-bc49-cca74a06a292"]',
                situate: 'left',
                arrowUrl: '/site/assets/bubble-arrow-down-left.svg',
                offsets: {
                    bubble: {
                        top: -84,
                        left: 16
                    },
                    arrow: {
                        width: 48,
                        top: 128,
                        left: -28
                    }
                }
            }, {
                nextEvent: null,
                headline: "Explore",
                caption: 'Weather around by <span class="italic">scrolling, zooming, tilting and rotating</span> the globe. You can also use the controls to surf around.',
                element: '.nav-tools',
                situate: 'right-top',
                buffer: 16,
                arrowUrl: '/site/assets/bubble-arrow-down-right.svg',
                offsets: {
                    bubble: {
                        top: -20,
                        right: -16
                    },
                    arrow: {
                        width: 64,
                        top: 20,
                        right: -56
                    }
                }
            }, {
                nextEvent: null,
                headline: "Share",
                caption: 'Once you’ve customized your globe <span class="italic">send it</span> to your colleagues, classmates and friends. We all love a good weather report.',
                element: '.static-legend__header',
                situate: 'left',
                arrowUrl: '/site/assets/bubble-arrow-up-left.svg',
                offsets: {
                    bubble: {
                        top: 124,
                        left: 30
                    },
                    arrow: {
                        width: 64,
                        top: -33,
                        left: -56
                    }
                }
            }]
        },
        finalMessage: {
            steps: [{
                nextEvent: null,
                headline: 'Amazing has only just begun. Sign up now for deeper access.',
                caption: 'With data at your disposal you get closer to the globe then ever before. When information like shark attacks can easily be accessed, understood and used, imagine what  PYXIS can do for your business, classroom or even spare time? The possibilities are truly infinite.<br/><br/> Sign up now for early access on MAC and a download on PC.',
                element: '.signup-or-download',
                situate: 'right-bottom',
                arrowUrl: '/site/assets/bubble-arrow-up-right.svg',
                offsets: {
                    bubble: {
                        width: 436,
                        top: 44
                    },
                    arrow: {
                        width: 64,
                        top: -32,
                        right: -56
                    }
                }
            }]
        }
    };

    $scope.$on("pyx-engine-ready", function() {
        if (!$window.resourceObject) return;
        $window.resourceObject.Groups[0].Items = $window.demoBaseLayers;
        $scope.demoInfo.stopGlobeSpin = $window.GC.stopGlobeSpin.bind($window.GC);

        function itemExists(item) {
            var itExists = false;

            _.each($window.resourceObject.Groups[0].Items, function(value) {
                if (value.Resource.Id === item[0].Resource.Id) itExists = true;
            });

            return itExists;
        }

        $scope.demoInfo.maps = {
            sharkAttacksLoaded: false,
            campgroundsLoaded: false,
            temperatureLoaded: false,
            numberLoaded: 0,
            loadSharkAttacks: function() {
                if (itemExists($window.demoLayersSharkAttacks)) return;

                $scope.demoInfo.hideOverlay();

                $window.resourceObject.Groups[0].Items.push($window.demoLayersSharkAttacks);
                $window.resourceObject.Groups[0].Items = _.flatten($window.resourceObject.Groups[0].Items);
                $scope.setCurrentMap($window.resourceObject);

                $scope.gotoItem($window.demoLayersSharkAttacks[0]);
                $scope.demoInfo.maps.sharkAttacksLoaded = true;
                $scope.demoInfo.maps.numberLoaded += 1;

                $window.GC.config.enableGlobeSpin = false;
                $scope.demoInfo.stopGlobeSpin();

            },
            loadCampgrounds: function() {
                if (itemExists($window.demoLayersCampgrounds)) return;

                $scope.demoInfo.hideOverlay();

                $window.resourceObject.Groups[0].Items.push($window.demoLayersCampgrounds);
                $window.resourceObject.Groups[0].Items = _.flatten($window.resourceObject.Groups[0].Items);
                $scope.setCurrentMap($window.resourceObject);

                $scope.gotoItem($window.demoLayersCampgrounds[0]);
                $scope.demoInfo.maps.campgroundsLoaded = true;
                $scope.demoInfo.maps.numberLoaded += 1;

                $window.GC.config.enableGlobeSpin = false;
                $scope.demoInfo.stopGlobeSpin();
            },
            loadTemperature: function() {
                if (itemExists($window.demoLayersTemperature)) return;

                $scope.demoInfo.hideOverlay();

                $window.resourceObject.Groups[0].Items.push($window.demoLayersTemperature);
                $window.resourceObject.Groups[0].Items = _.flatten($window.resourceObject.Groups[0].Items);
                $scope.setCurrentMap($window.resourceObject);

                $scope.gotoItem($window.demoLayersTemperature[0]);
                $scope.demoInfo.maps.temperatureLoaded = true;
                $scope.demoInfo.maps.numberLoaded += 1;

                $window.GC.config.enableGlobeSpin = false;
                $scope.demoInfo.stopGlobeSpin();
            }
        }

        angular.element('.loading-bar > span').css('width', '10%');

        if ($window.resourceObject && $window.resourceObject.Camera) {
            // TODO :: support an on load event for map/globe interface
            $timeout(function() {
                $pyx.globe.setCamera($window.resourceObject.Camera, 2000);
            }, 3000);
        }
        // redefine the "addGeoSource" method to delete
        // the currentMap so that opens the geoSource rather than importing on top
        // also go to the geosource at the end
        var oldAddFunction = $scope.addGeoSource;

        $scope.addGeoSource = function(geoSource, style) {

            // pass in geoSource nwame to new map
            $scope.createNewMap(geoSource.Metadata.Name);
            oldAddFunction(geoSource, style);
            $pyx.globe.gotoGeoSource(geoSource, 1000);
        }
    });
});

app.directive('fadeInIntro', function($window, $document, $parse, $timeout) {
    function postLink(scope, element, attrs) {
        var selectorFadeIn = element.find('.headline-landing__description');
        var selectorFadeOut = element.find('.headline-landing');
        var onDone = attrs.fadeInDone ? $parse(attrs.fadeInDone) : function() {};
        var onStart = attrs.fadeInInit ? $parse(attrs.fadeInInit) : function() {};
        var runComplete = true;

        function runAnimation() {
            var onComplete = function(duration) {
                $timeout(function() {
                    TweenMax.to(selectorFadeIn, 0.6, {
                        autoAlpha: 0,
                        onComplete: function() {
                            angular.element('.globe-controls, .legend-container').removeClass('hidden').show();

                            // set so that the lightTheme is enabled
                            $window.resourceObject.Metadata.Theme = window.PYXIS.themes.lightThemeConfig;

                            // override icon color to white
                            $window.resourceObject.Metadata.Theme.iconBorderColor = "#f0f0f0";

                            scope.$apply();

                            $timeout(function() {
                                if (runComplete) {
                                    onDone(scope);
                                    runComplete = false;
                                    element.remove();
                                } else {
                                    return;
                                }
                                $window.GC.stopGlobeSpin();
                            }, 250);
                        }
                    });
                }, duration);
            }

            /* Old
            var sequencer = new TimelineMax({
                onComplete: onComplete.bind(scope, 6000)
            });
            */
            var sequencer = new TimelineMax();

            scope.skipToEnd = function() {
                sequencer.time(sequencer.duration(), false);
                sequencer.clear();
                onComplete(250);

            }

            selectorFadeOut.children().each(function() {
                sequencer.to(this, 0.2, {
                    y: -188,
                    autoAlpha: 0
                }, '-=0.05');
            });

            if (selectorFadeIn.css('display') === 'none') {
                sequencer.set(selectorFadeOut, {
                    display: 'none'
                });
                sequencer.set(selectorFadeIn, {
                    display: 'block'
                });
                onStart(scope);
            }


            sequencer.fromTo(selectorFadeIn.find('p').first(), 1, {
                y: 188,
                autoAlpha: 0
            }, {
                y: 0,
                autoAlpha: 1,
                ease: Sine.easeOut
            });

            sequencer.fromTo(selectorFadeIn.find('a.navigation__item'), 1, {
                autoAlpha: 0
            }, {
                autoAlpha: 1,
                ease: Sine.easeOut
            }, "-=0.5");

            sequencer.fromTo(selectorFadeIn.find('p').eq(1), 0.8, {
                y: 44,
                autoAlpha: 0
            }, {
                y: 0,
                autoAlpha: 1,
                delay: 2,
                ease: Sine.easeOut
            });

            /* Old
            sequencer.staggerTo([selectorFadeIn.find('p').first(), selectorFadeIn.find('p').eq(1)], 0.3, {
                y: -188,
                autoAlpha: 0,
                delay: 4
            }, 0.15)

            sequencer.to(selectorFadeIn.find('a.navigation__item'), 0.3, {
                autoAlpha: 0,
                onComplete: function() {
                    TweenMax.set([selectorFadeIn.find('p').first(), selectorFadeIn.find('p').eq(1)], {
                        display: 'none'
                    });
                }
            });
            */

            sequencer.fromTo(selectorFadeIn.find('p').last(), 0.8, {
                autoAlpha: 0
            }, {
                autoAlpha: 1,
                delay: 2
            });

            /* Old
            sequencer.to(selectorFadeIn.find('a.navigation__item'), 1, {
                autoAlpha: 1,
                ease: Sine.easeOut
            }, "-=0.75");
            */
        }

        $document.on('click', '#demo-link', function(event) {
            event.preventDefault();
            event.stopPropagation();

            TweenMax.set(element, {
                zIndex: 1099
            });

            TweenMax.to(this, 0.3, {
                autoAlpha: 0,
                onComplete: function() {
                    TweenMax.set(this.target, {
                        display: 'none'
                    });
                }
            });

            runAnimation();
        });
    }

    return {
        restrict: 'A',
        scope: true,
        link: postLink
    };
});

app.directive('transitionProps', function($timeout) {
    function postLink(scope, element, attrs) {
        var duration = attrs.transitionDuration || 1;
        var properties = scope.$eval(attrs.transitionProps);

        if (!properties) return;

        $timeout(TweenMax.to.bind(TweenMax, element, duration, properties));
    }
    return {
        restrict: 'A',
        link: postLink
    };
})

app.directive('paintContext', function(numberUtils, paletteService) {
    function postLink(scope, element, attrs) {
        var item = scope.context;
        var palette = item.Style.Fill.Palette.Steps;
        var min = _.min(palette, rangeBounds);
        var max = _.max(palette, rangeBounds);
        var to = numberUtils.range(0, 1);
        var from = numberUtils.range(min.Value, max.Value);
        var colours = paletteService.toCSS(palette, from, to);

        function rangeBounds(obj) {
            return obj.Value;
        }

        element.css("background-image", "linear-gradient(to right," + colours + ")");
    }

    return {
        restrict: 'A',
        scope: {
            context: "=paintContext"
        },
        link: postLink
    };
})

app.directive('featureTour', function($timeout, $parse, $window, $document) {
    function postLink(scope, element, attrs) {
        var tourProgram = attrs.featureTour ? $parse(attrs.featureTour)(scope) : {};

        var tourDone = false;
        var tourAnimating = false;
        var hostNode;
        var stepIndex;

        var waitTimer = null;
        var durationTimer = 15 * 1000;
        var durationDelay = 500;
        var durationFadeIn = 0.35;
        var durationFadeOut = 0.2;

        scope.tourCommands = {};
        scope.tourCommands.currentPart = null;
        scope.tourCommands.partSection = null;
        scope.tourCommands.play = false;

        $timeout(function() {
            TweenMax.set('.nav-tools', {
                autoAlpha: 0
            });
        });

        scope.tourCommands.getIndex = function() {
            return stepIndex;
        }

        scope.tourCommands.load = function(partName) {
            if (tourDone) return;

            stepIndex = 0;

            TweenMax.to('.nav-tools', 0.4, {
                autoAlpha: 1
            });

            scope.tourCommands.currentPart = tourProgram[partName];

            scope.tourCommands.currentPart.steps.push(tourProgram['finalMessage'].steps[0]);

            scope.tourCommands.play = true;
            scope.tourCommands.goto(stepIndex);

            function handleEnter(index, event) {
                if (scope.tourCommands.play && !tourAnimating && stepIndex === index) {
                    $timeout(scope.tourCommands.next, durationDelay);
                }
            }
            _.each(scope.tourCommands.currentPart.steps, function(step, index) {
                var handler = _.debounce(handleEnter.bind(scope, index), durationDelay);
                if (step.nextEvent) {
                    $document.on(step.nextEvent, step.element, handler);
                }
            });

        }

        scope.tourCommands.insert = function(selector, textHtml) {
            var hostNode = element.find('.' + selector);
            hostNode.children().remove();
            hostNode.html(textHtml);
        }

        scope.tourCommands.goto = function(step) {
            if (tourDone) return;

            function runGoto() {
                scope.tourCommands.partSection = scope.tourCommands.currentPart.steps[step];

                TweenMax.set(element, {
                    zIndex: 999,
                    autoAlpha: 1
                });

                $timeout(positionBubble);

                // waitTimer = $timeout(scope.tourCommands.next, durationTimer);
            }

            $timeout(runGoto, durationDelay / 2);
        }

        scope.tourCommands.control = function(direction) {
            var bubbleEle = element.find('.tour-bubble__container');
            var bubbleArrow = element.find('.tour-bubble__arrow');
            var stepCount = scope.tourCommands.currentPart.steps.length;

            tourAnimating = true;

            if (bubbleEle.length) {
                TweenMax.set(bubbleArrow, {
                    clearProps: "all"
                });

                TweenMax.to(bubbleArrow, durationFadeOut, {
                    autoAlpha: 0,
                });

                TweenMax.to(bubbleEle, durationFadeIn, {
                    autoAlpha: 0,
                    scaleX: 1.15,
                    scaleY: 1.15,
                    ease: 'easeInBack',
                    onComplete: runComplete
                });

                function runComplete() {
                    removeBubble();

                    if (direction === 'next') {
                        stepIndex += 1;
                    } else {
                        stepIndex -= 1;
                    }

                    if (stepIndex >= stepCount) {
                        scope.tourCommands.currentPart = null;
                        scope.tourCommands.play = false;
                        tourDone = true;
                    }

                    $timeout(scope.tourCommands.goto.bind(scope, stepIndex), 500);
                    scope.$apply();
                }
            }
        }

        scope.tourCommands.stop = function() {
            $timeout.cancel(waitTimer);
            tourDone = true;
        }

        scope.tourCommands.kill = function() {
            removeBubble();
            scope.tourCommands.play = false;
            scope.tourCommands.currentPart = null;
            tourDone = true;
        }

        function removeBubble() {
            TweenMax.set(element, {
                zIndex: -99,
                autoAlpha: 0
            });

            if (waitTimer) {
                $timeout.cancel(waitTimer);
            }

            $timeout(function() {
                tourAnimating = false;
            }, durationDelay);

            scope.tourCommands.partSection = null;
        }

        function positionBubble() {
            hostNode = $document.find(scope.tourCommands.partSection.element);
            console.log(hostNode)
            var boundRect = hostNode[0].getBoundingClientRect();
            var bubbleEle = element.find('.tour-bubble__container');
            var bubbleArrow = element.find('.tour-bubble__arrow');
            var bubbleWid = bubbleEle.outerWidth(true);
            var bubbleHei = bubbleEle.outerHeight(true);
            var placement = scope.tourCommands.partSection.situate;

            var sequencer = new TimelineMax();

            var defaults = {
                offsets: {
                    arrow: {},
                    bubble: {}
                }
            };

            scope.tourCommands.partSection = _.extend(defaults, scope.tourCommands.partSection);

            var stylesObj = {
                zIndex: 999,
                autoAlpha: 0,
                scaleX: 1.15,
                scaleY: 1.15,
                ease: 'easeOutBack'

            };

            function getBubbleOffset(key) {
                if (_.has(scope.tourCommands.partSection.offsets.bubble, key)) {
                    return scope.tourCommands.partSection.offsets.bubble[key];
                } else {
                    return 0;
                }
            }

            if (getBubbleOffset('width')) {
                stylesObj.width = getBubbleOffset('width');
            }

            switch (placement) {
                case 'left':
                    stylesObj.top = (boundRect.top + getBubbleOffset('top')) - (bubbleHei / 2) + (boundRect.height / 2);
                    stylesObj.left = boundRect.right + getBubbleOffset('left');
                    break;

                case 'right-top':
                    stylesObj.top = boundRect.top - (bubbleHei + getBubbleOffset('top'));
                    stylesObj.right = boundRect.width + getBubbleOffset('right');
                    break;

                case 'right-bottom':
                    stylesObj.top = boundRect.bottom + getBubbleOffset('top');
                    stylesObj.right = boundRect.width + getBubbleOffset('right');
                    break;

                case 'center':
                    stylesObj.top = '50%';
                    stylesObj.left = '50%';
                    stylesObj.xPercent = -50;
                    stylesObj.yPercent = -50;
                    break;
            }

            sequencer.set(bubbleEle, stylesObj);

            if (!_.isEmpty(scope.tourCommands.partSection.offsets.arrow)) {
                sequencer.set(bubbleArrow, scope.tourCommands.partSection.offsets.arrow);
            }

            sequencer.to(bubbleEle, durationFadeIn, {
                autoAlpha: 1,
                scaleX: 1,
                scaleY: 1,
                ease: 'easeOutBack'
            });

            sequencer.to(bubbleArrow, durationFadeOut, {
                autoAlpha: 1
            });
        }

        function getListener() {
            var monitorWid = $window.innerWidth;
            var monitorHei = $window.innerHeight;

            return {
                wid: monitorWid,
                hei: monitorHei
            }
        }

        function setListener(value, valueLast) {
            if (angular.equals(value, valueLast) || !scope.tourCommands.partSection) {
                return;
            }

            positionBubble();
        }

        scope.$watch(getListener, setListener, true);

        scope.$on('$destroy', function() {
            $document.off('mouseenter');
        });
    }

    return {
        restrict: 'A',
        link: postLink
    }
});

app.directive('demoLegend', function($rootScope, $window, $q, $timeout, $parse) {
    function postLink(scope, element, attrs) {
        var legendPrune = attrs.legendPrune ? $parse(attrs.legendPrune)(scope) : null;
        var legendEvent = attrs.legendEvent ? $parse(attrs.legendEvent)(scope) : {};

        var regExpMatch = function(string, toMatch) {
            var re = new RegExp(toMatch, 'gi');
            var matches = string.match(re);

            if (matches) return true;

            return false;
        }

        element.addClass('static-legend-container');

        $timeout(TweenMax.to.bind(TweenMax, element, 1, {
            opacity: 1
        }));

        scope.$watch(attrs.legendMap, primeListener);

        if (!_.isEmpty(legendEvent)) {
            scope.$on(legendEvent.on, hoverListener);
            scope.$on(legendEvent.off, hoverListener);
        }

        function replaceIcon(card, cardOpts) {
            var validKey = 'Icon';
            if (_.has(card.Style, validKey) && _.has(cardOpts, validKey)) {
                card.Style[validKey].Alias = cardOpts[validKey];
            }
        }

        function friendlyAlias(cards) {
            var matchHash = {
                attacks: {
                    Name: 'Attacks',
                    Icon: '/site/assets/icon-shark-attacks.svg'
                },
                temperature: {
                    Name: 'Mean Temperature'
                },
                campgrounds: {
                    Name: 'Campgrounds',
                    Icon: '/site/assets/icon-triangle.svg'
                }
            };

            for (var key in matchHash) {
                _.each(cards, function(card, index) {
                    var alias = regExpMatch(card.Metadata.Name, key);
                    if (alias) {
                        card.Metadata.Alias = matchHash[key].Name;
                        replaceIcon(card, matchHash[key]);
                    }
                });
            }

            return cards;
        }

        function primeListener(value) {
            if (!value) return;

            var items = value.items();
            var cards = legendPrune ? pruneItems(items, legendPrune) : items;
            cards = friendlyAlias(cards)
            scope.cards = cards.reverse();
        }

        function hoverListener($event, data) {
            var name = $event.name;
            var resourceId = data.resourceId || data.geoSource;
            var node = element.find('[data-id="' + resourceId + '"]')

            if (name === 'iconHoverStart') {
                node.addClass('is-active');
            } else {
                node.removeClass('is-active');
            }
        }

        function pruneItems(items, omitIds) {
            var store = [];

            _.each(items, function(item, index) {
                var uniqueId = item.Resource.Id;
                var indexFound = _.indexOf(omitIds, uniqueId);

                if (indexFound === -1) store.push(item);

            });

            return store;
        }
    }

    return {
        restrict: 'A',
        scope: true,
        link: postLink
    }
});