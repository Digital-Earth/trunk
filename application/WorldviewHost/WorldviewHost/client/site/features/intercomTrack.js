/*
 - @name $pyxIntercom
 - @desc Intercom wrapper service.  To initialize, call registerAppId before boot.  Call boot when tracking should commence. 
*/
app.service('$pyxIntercom', function ($rootScope, $window, $timeout) {
    var Intercom = function () { };
    var uniqueId = null;
    var pyxIntercom = {};
    var userId;
    var booted = false;
    
    /*
    - @name pyxIntercom.registerId
    - @desc track events and add info to a specific app (ex. development vs. production) 
    - @type {function}
    - @param {string} appId - Located in the Intercom Admin under Settings
    */
	pyxIntercom.registerAppId = function (appId) {
        uniqueId = appId;
    }
    
    /*
    - @name pyxIntercom.boot
    - @desc control when Intercom is loaded. If previously loaded it will do the right thing.
    - @type {function}
    - @param {object} user - user profile information, or null if not logged in 
    - @param {object} moreInfo - other data we want to include for context 
    */
	pyxIntercom.boot = function (user, moreInfo) {
        if (!uniqueId) return;
        
        // check if Intercom exists
        if (!$window.hasOwnProperty('Intercom')) {
            return;
        }
        
        // $window.Intercom can refer to something else here in the case of multiple boots per session
        Intercom = $window.Intercom;

        if (booted) {
            if ((userId && user && user.Id === userId) || (!userId && !user)) {
                return;
            } else {
                Intercom('shutdown');
            }
        }

        var settings;
        if (user) {
            userId = user.Id;
            var moreInfo = moreInfo || {};
            var userName = user.UserName;
            
            // Check if user object contains first and last name properties
            // if it does, set as userName
            if (user.FirstName && user.LastName) {
                userName = (user.FirstName + ' ' + user.LastName);
            }
            
            settings = {
                app_id: uniqueId,
                user_id: user.Id,
                name: userName, 
                email: user.Email, 
                created_at: user.Metadata.Created,
                signed_up_at: user.Metadata.Created,
                widget: {
                    activator: "#Intercom"
                }
            };
            
            angular.extend(settings, moreInfo);
        } else {
            userId = undefined;
            settings = {
                app_id: uniqueId,
                widget: {
                    activator: "#Intercom"
                }
            };
        }
        Intercom('boot', settings);
        booted = true;
    }
    
    /*
    - @name pyxIntercom.shutdown
    - @desc clear out any user data that has been passed through the JS API
    - @type {function}
    */
	pyxIntercom.shutdown = function () {
        Intercom('shutdown');
        uniqueId = null;
    }
    
    /*
    - @name pyxIntercom.composerWindow 
    - @desc hide, show, etc the default Intercom message window
    - @type {function}
    - @param {string} composerState - can be: 'hide' | 'show' | 'showMessages' | 'showNewMessage'
    - @param {string} composerContent - pre-populate the message composer
    */
	pyxIntercom.composerWindow = function (composerState, composerContent) {
        // Supported Composer states
        var composerStates = {
            hide: true,
            show: true,
            showMessages: true,
            showNewMessage: true
        };
        
        if (!composerStates[composerState]) {
            return;
        }
        
        if (composerState === "showNewMessage" && composerContent) {
            Intercom(composerState, composerContent);
        } else {
            Intercom(composerState);
        }
    }
    
    /*
    - @name pyxIntercom.track
    - @desc associate the event with the currently logged in user and send it to Intercom
    - @type {function}
    - @param {string} eventName - the unique name of event (can be anything) but for consistency  use-snake-case
    - @param {object} metadata - optional metadata about the event
    */
	pyxIntercom.track = function (eventName, metadata) {
        if (metadata) {
            Intercom('trackEvent', eventName, metadata);
        } else {
            Intercom('trackEvent', eventName);
        }
    }
    
    /*
    - @name pyxIntercom.updateFeed
    - @desc look for new messages that should be displayed to the current user
    - @type {function}
    */
	pyxIntercom.updateFeed = function () {
        Intercom('update');
    }
    
    /*
    - @name pyxIntercom.updateUser
    - @desc update those fields on the current user in addition to logging an impression 
    - at the current URL and looking for new messages for the user
    - @type {function}
    - @param {object} details - user info fields that you want to update
    */
	pyxIntercom.updateUser = function (details) {
        Intercom('update', details);
    }
    
    /*
    - @name pyxIntercom.bind
    - @desc hook into the show/hide/click event
    - @type {function}
    - @param {string} onEventName - the event to handle, can be 'onShow'| 'onHide' | 'onActivatorClick'
    - @param {function} callback - the function to execute when the event is triggered $pyxIntercom.bind('onShow', function () {});
    */
	pyxIntercom.bind = function (eventName, callback) {
        // Supported Intercom events
        var onEvent = {
            onShow: true,
            onHide: true,
            onActivatorClick: true
        };
        
        if (!onEvent[eventName]) {
            return;
        }
        
        function safeCallback() {
            return $timeout(callback);
        }
        
        Intercom(eventName, safeCallback);
    }
    
    return pyxIntercom;
});