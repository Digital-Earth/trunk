/// <reference path="/scripts/jquery-2.1.1.js" />
/// <reference path="/scripts/angular-1.2.20.js" />
/// <reference path="/scripts/angular-cookies-1.2.20.js" />
/// <reference path="/test/angular-mocks-fix.js" />
/// <reference path="/Contents/Scripts/Flux/dispatcher.js" />


describe("flux.dispatcher", function () {
    var $dispatcher;
    beforeEach(module("flux"));    
    beforeEach(inject(function (_dispatcher_) {
        $dispatcher = _dispatcher_;
    }));

    it("dispatcher can register action", function () {
        var action = {
            type: "sayHi",
            create: function () {
                return new action();
            }
        };

        $dispatcher.registerAction("sayHi", action);
        expect($dispatcher.actions["sayHi"]).toEqual(action);
    });

    it("dispatcher register a simple action", function () {
        expect($dispatcher.registerAction("sayHi").type).toEqual("sayHi");
    });

    it("dispatcher register a simple action that can be created", function () {
        $dispatcher.registerAction("sayHi");
        expect($dispatcher.actions.sayHi.create().type).toEqual("sayHi");
    });

    it("dispatcher throws when registering action with no type", function () {
        expect(function () {
            $dispatcher.registerAction("sayHi", {});
        }).toThrow();
    });

    it("dispatcher throws when registering action with wrong type", function () {
        expect(function () {
            $dispatcher.registerAction("sayHi", { type: "somethingElse "});
        }).toThrow();
    });

    it("dispatcher throws when registering action twice", function () {
        var action = {
            type: "sayHi",
            create: function () {
                return new action();
            }
        };
        $dispatcher.registerAction(action.type, action);
        expect(function () {
            $dispatcher.registerAction(action.type, action);
        }).toThrow();
    });

    it("dispatcher can register store", function () {
        var store = { handle: function (action) {} };
        $dispatcher.registerStore("store", store);
        expect($dispatcher.stores["store"]).toEqual(store);
    });

    it("dispatcher can register store with simple function", function () {
        var store = function (aciton) {};
        $dispatcher.registerStore("store", store);
        expect($dispatcher.stores["store"].handle).toEqual(store);
    });

    it("dispatcher throws when registering empty store", function () {
        expect(function () {
            $dispatcher.registerStore("store", null);
        }).toThrow();
    });

    it("dispatcher throws when registering store with no name", function () {
        expect(function () {
            $dispatcher.registerStore("", function (action) {});
        }).toThrow();
    });

    it("dispatcher throws when registering store with null name", function () {
        expect(function () {
            $dispatcher.registerStore(null, function (action) { });
        }).toThrow();
    });

    it("dispatcher invoke action on a single store", function () {
        var handled = false;

        var handleAction = function (actionReceived) {
            if (actionReceived.type === "sayHi") {
                handled = true;
            }
        }

        $dispatcher.registerAction("sayHi");
        $dispatcher.registerStore("store", handleAction);
        $dispatcher.invoke($dispatcher.actions["sayHi"].create());
        expect(handled).toBeTruthy();
    });

    it("dispatcher invoke action on two stores", function () {
        var handled1 = false;
        var handled2 = false;

        var handle1Action = function (actionReceived) {
            if (actionReceived.type === "sayHi") {
                handled1 = true;
            }
        }

        var handle2Action = function (actionReceived) {
            if (actionReceived.type === "sayHi") {
                handled2 = true;
            }
        }

        $dispatcher.registerAction("sayHi");
        $dispatcher.registerStore("store1", handle1Action);
        $dispatcher.registerStore("store2", handle2Action);
        $dispatcher.invoke($dispatcher.actions["sayHi"].create());
        expect(handled1 && handled2).toBeTruthy();
    });

    it("dispatcher invoke action in right order", function () {
        var state = "init";

        var handle1Action = function (actionReceived) {
            if (actionReceived.type === "sayHi") {
                state = "hello";
            }
        }

        var handle2Action = function (actionReceived) {
            if (actionReceived.type === "sayHi") {
                $dispatcher.waitForStore($dispatcher.stores.store1);
                state += " world";
            }
        }

        var handle3Action = function (actionReceived) {
            if (actionReceived.type === "sayHi") {
                $dispatcher.waitForStore($dispatcher.stores.store2);
                state += " my";
            }
        }

        var handle4Action = function (actionReceived) {
            if (actionReceived.type === "sayHi") {
                $dispatcher.waitForStore($dispatcher.stores.store3);
                state += " friend";
            }
        }

        $dispatcher.registerAction("sayHi");
        $dispatcher.registerStore("store2", handle2Action);
        $dispatcher.registerStore("store1", handle1Action);
        $dispatcher.registerStore("store4", handle4Action);
        $dispatcher.registerStore("store3", handle3Action);
        $dispatcher.invoke($dispatcher.actions["sayHi"].create());
        expect(state).toEqual("hello world my friend");
    });

    it("dispatcher detect circular invoke sequence", function () {
        var handled = 0;

        var handle1Action = function (actionReceived) {
            if (actionReceived.type === "sayHi") {
                handled++;
                $dispatcher.waitForStore($dispatcher.stores.store2);                
            }
        }

        var handle2Action = function (actionReceived) {
            if (actionReceived.type === "sayHi") {
                handled++;
                $dispatcher.waitForStore($dispatcher.stores.store1);               
            }
        }
      
        $dispatcher.registerAction("sayHi");
        $dispatcher.registerStore("store1", handle1Action);
        $dispatcher.registerStore("store2", handle2Action);
        
        expect(function () {
            $dispatcher.invoke($dispatcher.actions.sayHi.create());
        }).toThrow();
        expect(handled).toEqual(2);
        expect($dispatcher.isInvoking()).toBeFalsy();
    });

    it("dispatcher return isInvoking true while handling an action", function () {
        var invoking = false;
        var handled = false;

        var handle1Action = function (actionReceived) {
            if (actionReceived.type === "sayHi") {
                handled = true;
                invoking = $dispatcher.isInvoking();
            }
        }

        $dispatcher.registerAction("sayHi");
        $dispatcher.registerStore("store1", handle1Action);
        $dispatcher.invoke($dispatcher.actions.sayHi.create());

        expect(handled).toBeTruthy();
        expect(invoking).toBeTruthy();        
    });

    it("dispatcher throw when invoking an action while isInvoking", function () {
        $dispatcher.registerAction("sayHi");

        var handle1Action = function (actionReceived) {
            if (actionReceived.type === "sayHi") {
                $dispatcher.invoke($dispatcher.actions.sayHi.create());
            }
        }

        $dispatcher.registerStore("store1", handle1Action);        
        expect(function () {
            $dispatcher.invoke($dispatcher.actions.sayHi.create());
        }).toThrow();
    });

    it("dispatcher throws when registering service twice", function () {        
        $dispatcher.registerService("service", {});
        expect(function () {
            $dispatcher.registerService("service", {});
        }).toThrow();
    });
});