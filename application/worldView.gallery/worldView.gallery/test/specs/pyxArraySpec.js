/// <reference path="/scripts/jquery-2.1.1.js" />
/// <reference path="/scripts/angular-1.2.20.js" />
/// <reference path="/scripts/angular-cookies-1.2.20.js" />
/// <reference path="/test/angular-mocks-fix.js" />
/// <reference path="/scripts/pyxis/pyxis.js" />
/// <reference path="/scripts/pyxis/pyxis.gallery.js" />
/// <reference path="/scripts/pyxis/pyxis.user.js" />
/// <reference path="/scripts/pyxis/pyxis.area.js" />


describe("$pyx.array", function () {    
    var $pyx;    
    beforeEach(module("pyxis"));
    beforeEach(module("ngCookies"));
    beforeEach(inject(function (_$pyx_) {
        $pyx = _$pyx_;
    }));

    var nameArray = [
        {
            name: "Alice"
        },
        {
            name: "Bob"
        },
        null,
        {
            name: null
        }
    ];

    var complexArray = [
        {
            name: { first: "Alice", last: "Red" },
            tags: ["A", "B", "C"]
        },
        null,
        {
            name: { first: "Bob", last: "Green" },
            tags: ["C", "D", "E"]
        }
    ];

    it("$pyx.array exists", function() {
        expect($pyx.array).not.toBeNull();
    });

    it("$pyx.array.firstIndex find right element index with simple condition", function () {
        expect($pyx.array.firstIndex(nameArray, "name", "Alice")).toEqual(0);
        expect($pyx.array.firstIndex(nameArray, "name", "Bob")).toEqual(1);
    });

    it("$pyx.array.firstIndex find right element index with condition searching value null", function () {       
        expect($pyx.array.firstIndex(nameArray, "name", null)).toEqual(3);
    });

    it("$pyx.array.firstIndex find right element index with complex condition", function () {
        expect($pyx.array.firstIndex(complexArray, "name.last", "Green")).toEqual(2);
    });

    it("$pyx.array.firstIndex find right element index with condition on array", function () {
        expect($pyx.array.firstIndex(complexArray, "tags", "D")).toEqual(2);
    });

    it("$pyx.array.firstIndex return -1 for condition on array with not match", function () {
        expect($pyx.array.firstIndex(complexArray, "tags", "not-a-tag")).toEqual(-1);
    });

    it("$pyx.array.firstIndex with empty array returns -1", function () {
        expect($pyx.array.firstIndex([], 'name', 'Alice')).toEqual(-1);
    });

    it("$pyx.array.firstIndex with null array returns -1", function () {
        expect($pyx.array.firstIndex(null, 'name', 'Alice')).toEqual(-1);
    });

    it("$pyx.array.firstIndex with null condition", function () {
        expect($pyx.array.firstIndex(null, null, 'Alice')).toEqual(-1);
    });

    it("$pyx.array.firstIndex with function condition works", function () {
        expect($pyx.array.firstIndex(nameArray,function (item) { return item.name == "Bob"; })).toEqual(1);
    });

    it("$pyx.array.equals without condition return true for null arrays", function () {
        expect($pyx.array.equals(null,null)).toBeTruthy();
    });

    it("$pyx.array.equals without condition return true for equal arrays", function () {
        expect($pyx.array.equals([0, 1, 2], [0, 1, 2])).toBeTruthy();        
    });

    it("$pyx.array.equals without condition return false for equal arrays that are not the same", function () {
        expect($pyx.array.equals([0, 1, 2], [0, 2, 1])).toBeFalsy();
    });

    it("$pyx.array.equals without condition return false on different size array", function () {
        expect($pyx.array.equals([0, 1, 2], [0, 1])).toBeFalsy();
    });

    it("$pyx.array.equals with condition return false on different size array", function () {
        expect($pyx.array.equals([0, 1, 2], [0, 1], function (a,b) { return a === b; })).toBeFalsy();
    });

    it("$pyx.array.equals with condition return true on same array", function () {
        expect($pyx.array.equals([0, 1, 2], [0, 1, 2], function (a, b) { return a === b; })).toBeTruthy();
    });

    it("$pyx.array.equals with condition return true on same array", function () {
        expect($pyx.array.equals([0, 1, 2], [0, 2, 1], function (a, b) { return a === b; })).toBeFalsy();
    });

    it("$pyx.array.equals with condition and one null array return false", function () {
        expect($pyx.array.equals(null, [0, 2, 1], function (a, b) { return a === b; })).toBeFalsy();
    });

    it("$pyx.array.equals without condition and one null array return false", function () {
        expect($pyx.array.equals(null, [0, 2, 1])).toBeFalsy();
    });
});