/*
- @name delayEvent
- @desc wrapper service for delaying event execution
- @example - delayApply = delayEvent(scope.applyToGlobe, 500, false);
- ---------- delayApply();
- ---------- and, delayApply.then(function(){});
*/
app.factory('delayEvent', function ($timeout, $q) {
    /* 
    - @param {function} onComplete
    - @param {number} timeToUpdate
    - @param {boolean} isImmediate
    */
    return function delayEvent (onComplete, timeToUpdate, isImmediate) {
        var setTime;
        var deferred = $q.defer();
    
        return function () {
            var context = this;
            var args = arguments;

            var callLater = function () {
                setTime = null;

                if (!isImmediate) {
                    deferred.resolve(onComplete.apply(context, args));
                    deferred = $q.defer();
                }
            }

            var callNow = isImmediate && !setTime;

            if (setTime) {
                $timeout.cancel(setTime);
            }

            setTime = $timeout(callLater, timeToUpdate);

            if (callNow) {
                deferred.resolve(onComplete.apply(context, args));
                deferred = $q.defer();
            }

            return deferred.promise;
        }
    }
});

