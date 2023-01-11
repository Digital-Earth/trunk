app.service('positionHelper', function () {
    var positionHelper = {
        'currentPosition': function (element) {
            var offset = element.offset();
            var width = element.width();
            var height = element.height();

            return {
                top: offset.top,
                left: offset.left,
                width: width,
                height: height,
                bottom: offset.top + height,
                right: offset.left + width
            }
        },
        'ensureInsideScreen': function (elementPosition, padding) {
            var newElementPosition = {
                top: elementPosition.top,
                left: elementPosition.left,
                width: elementPosition.width,
                height: elementPosition.height,
            }

            var windowWidth = $(window).width();
            var windowHeight = $(window).height();

            //ensure left top position
            if (newElementPosition.top < padding) {
                newElementPosition.top = padding;
            }
            if (newElementPosition.left < padding) {
                newElementPosition.left = padding;
            }

            //ensure maximum width and height
            if (newElementPosition.width > windowWidth - padding * 2) {
                newElementPosition.width = windowWidth - padding * 2;
            }
            if (newElementPosition.height > windowHeight - padding * 2) {
                newElementPosition.height = windowHeight - padding * 2;
            }

            //ensure right bottom position
            if (newElementPosition.left + newElementPosition.width > windowWidth - padding) {
                newElementPosition.left = windowWidth - padding - newElementPosition.width;
            }

            if (newElementPosition.top + newElementPosition.height > windowHeight - padding) {
                newElementPosition.top = windowHeight - padding - newElementPosition.height;
            }

            newElementPosition.right = newElementPosition.left + newElementPosition.width;
            newElementPosition.bottom = newElementPosition.top + newElementPosition.height;

            return newElementPosition;
        },
        'positionBasedOn': function (host, target, placement, padding, options) {
            var hostPosition = this.currentPosition(host);
            var targetPosition = this.currentPosition(target);

            options = options || {};
            var topOffset = options.topOffset || 0;
            var leftOffset = options.leftOffset || 0;
            var arrowSize = 7;
            var arrowDefaultOffset = options.arrowDefaultOffset || arrowSize * 2;

            var windowWidth = $(window).width();
            var windowHeight = $(window).height();

            placement = placement.split(" ");
            var arrow = placement[0];

            var secondPlacement = placement.length > 1 ? placement[1] : "";
            var center = secondPlacement == 'center';

            //fix left/right and top/bottom arrow direction if not enough space.
            fixArrowPlacement = {
                'left': function () {
                    return (hostPosition.left - targetPosition.width < padding) ? 'right' : 'left';
                },
                'right': function () {
                    return (hostPosition.right + targetPosition.width > windowWidth - padding) ? 'left' : 'right';
                },
                'top': function () {
                    return (hostPosition.top - targetPosition.height < padding) ? 'bottom' : 'top';
                },
                'bottom': function () {
                    return (hostPosition.bottom + targetPosition.height > windowHeight - padding) ? 'top' : 'bottom';
                }
            }

            arrow = fixArrowPlacement[arrow]();
            var arrowOffset = {};

            var verticalPlacement = function () {
                if (center) {
                    targetPosition.top = hostPosition.top + hostPosition.height / 2 - targetPosition.height / 2;
                    arrowOffset.top = targetPosition.height / 2 - arrowSize;
                } else {
                    targetPosition.top = hostPosition.top + hostPosition.height / 2 - arrowDefaultOffset - topOffset;
                    arrowOffset.top = arrowDefaultOffset + topOffset - arrowSize;
                }
            }

            var horizontalPlacement = function () {
                if (center) {
                    targetPosition.left = hostPosition.left + hostPosition.width / 2 - targetPosition.width / 2;
                    arrowOffset.left = targetPosition.width / 2 - arrowSize;
                } else if (secondPlacement == 'left') {
                    targetPosition.left = hostPosition.left + leftOffset;
                    arrowOffset.left = arrowDefaultOffset - arrowSize;
                } else if (secondPlacement == 'right') {
                    targetPosition.left = hostPosition.left + hostPosition.width - targetPosition.width;
                    arrowOffset.left = targetPosition.width - arrowSize;
                } else {
                    targetPosition.left = hostPosition.left + hostPosition.width / 2 - arrowDefaultOffset;
                    arrowOffset.left = arrowDefaultOffset - arrowSize;
                }
            }

            //position the target
            placementBasedOnArrow = {
                'left': function () {
                    targetPosition.left = hostPosition.left - targetPosition.width - 2 * arrowSize;
                    arrowOffset.left = targetPosition.width;
                    verticalPlacement();
                },
                'right': function () {
                    targetPosition.left = hostPosition.right;
                    arrowOffset.left = -arrowSize;
                    verticalPlacement();
                },
                'top': function () {
                    targetPosition.top = hostPosition.top - targetPosition.height - 2 * arrowSize;
                    arrowOffset.top = targetPosition.height;
                    horizontalPlacement();
                },
                'bottom': function () {
                    targetPosition.top = hostPosition.bottom + arrowSize;
                    arrowOffset.top = 0;
                    horizontalPlacement();
                },
            };
            placementBasedOnArrow[arrow]();

            //ensure window is inside screen
            var fixedTargetPosition = this.ensureInsideScreen(targetPosition, padding);
            var xDelta = targetPosition.left - fixedTargetPosition.left;
            var yDelta = targetPosition.top - fixedTargetPosition.top;

            arrowOffset.left += xDelta;
            arrowOffset.top += yDelta;

            fixedTargetPosition.arrow = arrow;
            fixedTargetPosition.arrowOffset = arrowOffset;

            return fixedTargetPosition;
        }
    };

    return positionHelper;
});