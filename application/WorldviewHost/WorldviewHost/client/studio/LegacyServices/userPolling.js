// Name: User Polling
//------------------------------------------------------
// Desc: 
// Check if a user's email address has been confirmed
// so they don't have to manually login.
//
// Use: 
// userPolling.runPoll();

app.service('userPolling', function($pyx, $q, $timeout, worldViewStudioConfig) {
    var checkFreq = worldViewStudioConfig.userPolling.checkFrequency;
    var deferred = $q.defer();
    var poller = {};
    
    poller.runPoll = function () {
        var self = this;
        
        var handlePoll = function () {
           $pyx.user.profile().success(function (profile) {
                if(profile.EmailConfirmed) {
                    deferred.resolve('user-email-confirmed');
                } else {
                    $timeout(self.runPoll, checkFreq);
                }

            }).error(self.stopPoll);
        }

        var handlePollError = function (error) {
            self.stopPoll();
        }

        $pyx.when($pyx.application.getToken()).then(function (token) {
            if (token.access_token) {
                $pyx.user.loginUsingToken(token).then(handlePoll);
            } else {
                $pyx.when($pyx.application.loginWithToken($pyx.user.token())).then(handlePoll, handlePollError);
            }
        }, 
        function () {
            self.stopPoll();
        });

        return deferred.promise;   
    }

    poller.stopPoll = function() {
        deferred.reject('user-poll-canceled');
    }

    return poller;  

});


