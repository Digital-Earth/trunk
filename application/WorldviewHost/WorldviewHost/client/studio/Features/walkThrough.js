/*
featureWalkThrough allow to create a walk through tutorials inside WV

featureWalkThrough.register($scope) - to enable this feature

featureWalkThrough.makeWalkThroughStep(
    {left:"20%",top:"20%},                      - position
    "Message",                                  - content/message
    function() { console.log("we made it") });  - action to invoke to setup the walkThrough step
*/
app.service("featureWalkThrough", function ($pyx, $filter) {
    
    function register($scope) {
        $scope.addTemplate('dialogs', '/client/templates/studio/template/feature-walk-through.html');

        var walkThrough = {
            steps: [],
            content: "",
            style: {},
            step: 0,
            active: false,
            start: function (steps) {
                if (steps !== undefined) {
                    walkThrough.steps = steps;
                }                
                walkThrough.moveToStep(0);
                walkThrough.active = true;
            },
            previous: function () {
                if (walkThrough.step > 0) {
                    walkThrough.moveToStep(walkThrough.step - 1);
                }
            },
            resume: function () {
                walkThrough.moveToStep(walkThrough.step);
            },
            next: function () {
                if (walkThrough.step < walkThrough.steps.length) {
                    walkThrough.moveToStep(walkThrough.step + 1);
                }
            },
            hasPrevious: function () {
                return walkThrough.step > 0;
            },
            hasNext: function () {
                return walkThrough.step + 1 < walkThrough.steps.length;
            },
            finish: function () {
                walkThrough.active = false;
            },
            moveToStep: function (stepIndex) {
                if (stepIndex >= 0 && stepIndex < walkThrough.steps.length) {
                    walkThrough.step = stepIndex;
                    var step = walkThrough.steps[walkThrough.step]();
                    walkThrough.content = step.content;
                    walkThrough.style = step.style;
                    walkThrough.active = true;
                    walkThrough.currentMap = angular.copy($pyx.obj.get($scope.currentMap, 'model'));
                } else {
                    walkThrough.active = false;
                }
            },
            resumeNeeded: function () {
                return !angular.equals(walkThrough.currentMap, $pyx.obj.get($scope.currentMap, 'model'));
            }
        }

        $scope.walkThrough = walkThrough;
    };

    var makeWalkThroughStep = function (position, content, action) {
        return function () {
            var w = $(window).width();
            var h = $(window).height();
            var dialogWidth = 400;
            var dialogHeight = 200;

            function clamp(value, min, max) {
                return Math.min(max, Math.max(min, value));
            }

            if (action) {
                action();
            }

            return {
                content: content,
                style: {
                    left: clamp(w * position.left / 100 - dialogWidth / 2, 0, w - dialogWidth) + "px",
                    top: clamp(h * position.top / 100 - dialogHeight / 2, 0, h - dialogHeight) + "px"
                }
            };
        }
    }

    return {
        register: register,
        makeWalkThroughStep: makeWalkThroughStep
    };
});