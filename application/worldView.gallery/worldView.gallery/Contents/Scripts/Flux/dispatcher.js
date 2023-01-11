var flux = angular.module("flux", []);

/* 
- @name dispatcher
- @desc a service that allows a developer to seperate UI interactions into actions,stores and services.
- @ref  https://facebook.github.io/flux/ - we use the flux concept and attached it into angular eco-system

api:

    //register actions - each action have a type
    dispatcher.registerAction("submitOrder");
    dispatcher.registerAction("submitOrderCompleted");

    //register stores and services using angular DI
    dispatcher.registerStore("storeName");
    dispatcher.registerService("ordersApi");

    //register an inline store
    dispatcher.registerStore("myStore", { //register a store object

        //handle actions vector
        handle: {
            //handle 'submitOrder' action - come from UI
            'submitOrder': function(action) {
                dispatcher.stores.services.ordersApi.submit(action.data);
                $scope.status = "submitting";
            },
            //handle 'submitOrderCompleted' action - come from the ordersApi when everything is awesome
            'submitOrderCompleted': function(action) {
                $scope.status = "ok, reference number: " + action.data.refNum;
            }
            'submitOrderFailed': function(action) {
                $scope.status = "failed due to " + action.data.error;
            }
        },

        //define store get functions here
        get: function() { ... }
        getOrder: function(id) { ... }
    });

    //register a store that handles all actions - and trace the action type - and without any get functions.
    dispatcher.registerStore("actionTrace", function(action) { console.log(action.type); });

    //attaching to angular events
    $scope.buttonClick = function() {
        dispatcher.actions.submitOrder.invoke({ orderId: $scope.Id, items: $scope.items });
    };

*/
flux.service("dispatcher", function ($injector, $timeout) {
    var invokeState = {
        invoking: false,
        handled: {},
        pending: {},

        invokeAction: function (action, stores) {
            var self = this;

            try {
                self.invoking = true;
                self.action = action;
                self.stores = stores;
                self.handled = {};
                self.pending = {};

                angular.forEach(self.stores, function (store, storeName) {
                    self.invokeActionOnStore(action, storeName);
                });
            } finally {
                self.invoking = false;
            }
        },

        invokeActionOnStore: function (action, storeName) {
            var self = this;

            try {
                //no reason to invoke store twice
                if (self.handled[storeName]) {
                    return;
                }
                //detect circular pending
                if (self.pending[storeName]) {
                    throw new Error("stores circular pending detected");
                }
                //start invoke action on store
                self.pending[storeName] = true;

                var store = self.stores[storeName];

                //let store handle action
                if (store.handle && angular.isFunction(store.handle)) {
                    store.handle(action);
                } else if (action.type in store.handle) {
                    store.handle[action.type](action);
                }
            } catch (error) {
                console.log("failed to invoke action '" + action.type + "' on store '" + storeName + "':\n" + error.stack);
            } finally {
                //mark action as handled and not pending on store.
                self.handled[storeName] = true;
                delete self.pending[storeName];
            }
        },

        ensureStoreHandleAction: function (storeName) {
            this.invokeActionOnStore(this.action, storeName);
        }
    };

    var safeInvokeQueue = [];

    var dispatcher = {
        stores: {},
        services: {},
        actions: {},

        /*
        - @name dispatcher.isInvoking
        - @desc return true if action is been invoked        
        - @type {function}
        */
        isInvoking: function () {
            return invokeState.invoking;
        },

        /*
        - @name dispatcher.safeInvoke
        - @desc dispatch an action safely for flux services
        - @param {action} action
        - @type {function}
        
        Note: don't use this function on a general case - use it on services only.

        safely have 2 aspects: 
            1)  this function ensure we run under angular $apply mode. there this function can be called safely from 3rd parties libraries.
            2)  queue the action if dispatcher is already invoking. this can happen if a service using $http and result is under cache.
                so, sometimes $http would simply call the success callback. which means the safeInvoke is get called while the previous action
                is still dispatching.
        */        
        safeInvoke: function (action) {
            var self = this;

            if (self.isInvoking()) {
                //queue action for later use
                safeInvokeQueue.push(action);
            } else {
                //ensure we are under angular context
                window.setTimeout(function () {
                    $timeout(function () {
                        self.invoke(action);
                    });
                }, 0);

            }
        },

        /*
        - @name dispatcher.invoke
        - @desc dispatch an action (must be called under angular scope)
        - @param {action} action
        - @type {function}
        */
        invoke: function (action) {
            var self = this;

            if (self.isInvoking()) {
                throw new Error("can't invoke a new action while studio is invoking another action");
            }

            if (!action) {
                throw new Error("can't invoke a null action");
            }

            if (action.type && !(action.type in self.actions)) {
                throw new Error("can't invoke a unregistered action with type " + action.type);
            }

            //invoke action.
            invokeState.invokeAction(action, self.stores);

            //invoke pending actions caused by services
            while (safeInvokeQueue.length) {
                var pendingAction = safeInvokeQueue.shift();
                
                if (!pendingAction) {
                    throw new Error("can't invoke a null action");
                }

                if (pendingAction.type && !(pendingAction.type in self.actions)) {
                    throw new Error("can't invoke a unregistered action with type " + pendingAction.type);
                }

                invokeState.invokeAction(pendingAction, self.stores);
            };
        },
        /*
        - @name dispatcher.waitForStore
        - @desc wait until the current action is been handled by the give store(s)
        - @param {string | array<string>} store
        - @type {function}
        */
        waitForStore: function (store) {
            var self = this;

            //if store is array of stores - call self.waitForStore on each store in array.
            if (angular.isArray(store)) {
                angular.forEach(store, function (singleStore) {
                    self.waitForStore(singleStore);
                });
                return;
            }

            //find store
            var storeName = undefined;
            if (store in self.stores) {
                storeName = store;
            } else {
                for (var name in self.stores) {
                    if (self.stores.hasOwnProperty(name) && self.stores[name] === store) {
                        storeName = name;
                        break;
                    }
                }
            }

            if (!storeName) {
                throw new Error("can't wait for unregistered store");
            }

            if (!self.isInvoking()) {
                throw new Error("can't wait for store while action is not invoking");
            }

            invokeState.ensureStoreHandleAction(storeName);
        },
        /*
        - @name dispatcher.registerAction
        - @desc register an action to the dispatcher registry to ensure unique actions Ids.
        - @param {string} name - action name/type
        - @param optional {object} action - provide a custom create for action
        - @type {function}
        */
        registerAction: function (name, action) {
            var self = this;
            if (name in self.actions) {
                throw new Error("can't register action with name " + name + " twice.");
            }

            if (!action) {
                //create default action
                action = function () {
                    this.type = name;
                }
                action.type = name;
                action.create = function (data) {
                    var newAction = new action();
                    if (data !== undefined) {
                        newAction.data = data;
                    }
                    return newAction;
                }
            }
            if (!action.type) {
                throw new Error("action.type must be set for registered action");
            }
            if (action.type !== name) {
                throw new Error("action.type must be equal to name");
            }
            self.actions[name] = action;

            //helper functions
            action.invoke = function (data) {
                self.invoke(action.create(data));
            }
            action.safeInvoke = function (data) {
                self.safeInvoke(action.create(data));
            }

            return action;
        },
        /*
        - @name dispatcher.registerStore
        - @desc register a store to the dispatcher registry to ensure unique actions Ids.
        - @param {string} name - store name
        - @param optional {object} store - provide a custom store, if undefined angular.$injector is used to resolve store
        - @type {function}
        */
        registerStore: function (name, store) {
            var self = this;
            if (!store) {
                //try to inject store
                store = $injector.get(name);
            }
            if (!name || !store) {
                throw new Error("can't register a null store");
            }
            if (name in self.stores) {
                throw new Error("can't register store with name " + name + " twice.");
            }
            if (!("handle" in store)) {

                if (angular.isFunction(store)) {
                    //wrap the handle function with a store object
                    store = { 'handle': store };
                } else {
                    throw new Error("can't register store with no handle function");
                }
            }

            self.stores[name] = store;
        },
        /*
        - @name dispatcher.registerService
        - @desc register a service to the dispatcher registry to ensure unique actions Ids.
        - @param {string} name - service name
        - @param optional {object} service - provide a custom service, if undefined angular.$injector is used to resolve store
        - @type {function}
        */
        registerService: function (name, service) {
            var self = this;
            if (name in self.services) {
                throw new Error("can't register service with name " + name + " twice.");
            }
            if (!service) {
                //try to inject service
                service = $injector.get(name);
            }
            self.services[name] = service;
        }
    };    
    return dispatcher;
});

/* 
- @name dispatcherPromise
- @desc a service that allows a developer to easily create angular promise that get resolved based on dispatcher actions

api:
- @name dispatcherPromise.promise
- @desc create a promise object
- @param {function} handler - handler function that receive 2 arguments:
                               1) action - an action that been invoked
                               2) deferred - the deferred object to resolve or reject
- @type {function}

usage:
  var promise = dispatcherPromise.promise(function(action,deferred) {
    if (action.type == "calculationCompleted") {
        deferred.resolve(action.data);
    } else if (action.type === "calculationFailed") {
        deferred.reject(action.error);
    } else {
        //do nothing
    }
  });
  
  //use promise as you would in angular
  promise.then(function(data) { ... }, function(error) { .... });
*/
flux.service("dispatcherPromise", function ($q,dispatcher) {
    //angular $q.defer() connector to dispatcher.
    var pendingPromises = [];

    var dispatcherPromiseStore = {
        handle: function (action) {
            pendingPromises = pendingPromises.filter(function (pendingPromise) {
                //allow each pending promise to handle this action
                pendingPromise.handle(action);

                //keep it on the list if it no been completed
                return !pendingPromise.isCompleted();
            });
        }
    }

    dispatcher.registerStore("dispatcherPromiseStore", dispatcherPromiseStore);

    var dispatcherPromise = {};

    dispatcherPromise.promise = function (handleFunction) {
        var deferred = $q.defer();
        var completed = false;

        var wrappedDeferred = {
            resolve: function (value) {
                deferred.resolve(value);
                completed = true;
            },
            reject: function (value) {
                deferred.reject(value);
                completed = true;
            }
        };
        var pendingPromise = {
            handle: function (action) {
                handleFunction(action, wrappedDeferred);
            },
            isCompleted: function () {
                return completed;
            }
        }
        pendingPromises.push(pendingPromise);

        return deferred.promise;
    }
   
    return dispatcherPromise;
})