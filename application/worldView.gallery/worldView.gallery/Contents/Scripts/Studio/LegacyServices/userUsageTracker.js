// Name: User Usage Tracker
//------------------------------------------------------
// Desc: 
// If a user has launched Studio more than the 'launchLimit'
// or used Studio for longer than the trial period with out 
// confirming their email, this service will prompt them to 
// confirm before they can continue using Studio
// 
// Use: 
// userUsageTracker.incrementLaunchCount();  - Increments the number of times a user has launched Studio
// userUsageTracker.isTrialExpired();        - Checks if the time limit or launch limit has been reached
// userUsageTracker.isLaunchCountDone();     - Checks if the launch limit has been reached
// userUsageTracker.isDeltaExpired();        - Checks if the time limit has been reached
// userUsageTracker.stopUserTrial();         - Removes the cookies
// userUsageTracker.pollUserTrial()          - Checks if the delta cookie has expired

app.service('userUsageTracker', function (globalCookieStore, $cookieStore, $timeout, $interval) {
    var cookieTrialPeriod = 'user-trial-period';
    var cookieLaunchCount = 'user-launch-limit';
    var launchLimit = 2;
    var pollTimer;
    var checkFreq = 300000; // 5 minutes.
    var tracker = {};
  
    tracker.incrementLaunchCount = function () {
        var count = globalCookieStore.get(cookieLaunchCount);

        count = parseInt(count) + 1;
        globalCookieStore.put(cookieLaunchCount, count, 3);
    }

    tracker.isLaunchCountDone = function () {
        var count = globalCookieStore.get(cookieLaunchCount);
        count = parseInt(count);
      
        if(count === launchLimit){
            return true;
        } else {
            return false;
        }
    }

    tracker.isDeltaExpired = function () {
        if(globalCookieStore.get(cookieTrialPeriod) == null || !globalCookieStore.get(cookieTrialPeriod)) {
            return true;
        } else {
            return false;
        }
    }

    tracker.isTrialExpired = function () {
        if(this.isDeltaExpired() || this.isLaunchCountDone()) {
            return true;
        } else {
            return false;
        }
    }

    /* 
     - @param {function} onTrialStarted - optional callback after the trial has started
    */
    tracker.startUserTrial = function (onTrialStarted) {
        var count = 0;
        var state = 'active';

        if (!globalCookieStore.get(cookieTrialPeriod)) {
            globalCookieStore.put(cookieTrialPeriod, state, 3);
            globalCookieStore.put(cookieLaunchCount, count, 3);

            if(onTrialStarted) {
                onTrialStarted();
            }
        } 

    }

    /* 
     - @param {function} onTrialComplete - optional callback after the trial has ended
     - passed to tracker.stopUserTrial
     */
    tracker.pollUserTrial = function (onTrialComplete) {
        var self = this;

        if (this.isTrialExpired()) {
            if (pollTimer) {
                $interval.cancel(pollTimer);
            }
            else {
                return;
            }

        } else {
            pollTimer = $interval(function() {
                if (self.isDeltaExpired()) {
                    self.stopUserTrial(onTrialComplete);
                }
            }, checkFreq);
        }
    }

    /* 
     - @param {function} onTrialComplete - optional callback after the trial has ended
    */
    tracker.stopUserTrial = function (onTrialComplete) {
        globalCookieStore.remove(cookieTrialPeriod);
        globalCookieStore.remove(cookieLaunchCount);

        $interval.cancel(pollTimer);

        if(onTrialComplete) {
            onTrialComplete();
        }
    }

    return tracker;
});