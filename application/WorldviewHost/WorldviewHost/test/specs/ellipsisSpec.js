/// <reference path="/test/worldview-site-mocks.js" />
/// <reference path="/contents/scripts/site/directives.js" />

describe("ellipsis", function () {
    var $ellipsis;
    var $filter;
    beforeEach(module("worldViewSite"));
    beforeEach(inject(function (_$filter_) {
        $filter = _$filter_;
        $ellipsis = _$filter_("ellipsis");
    }));

    it("ellipsis: null string", function () {
        expect($ellipsis(null, 10)).toBeNull();
    });

    it("ellipsis: undefined string", function () {
        expect($ellipsis(undefined, 10)).not.toBeDefined();
    });

    it("ellipsis: empty string", function () {
        expect($ellipsis("", 10)).toBe("");
    });

    it("ellipsis: null length", function () {
        expect($ellipsis("12345", null)).toBe("12345");
    });

    it("ellipsis: undefined length", function () {
        expect($ellipsis("12345", undefined)).toBe("12345");
    });

    it("ellipsis: zero length", function () {
        expect($ellipsis("12345", undefined)).toBe("12345");
    });

    it("ellipsis: length longer than string", function () {
        expect($ellipsis("12345", 10)).toBe("12345");
    });

    it("ellipsis: length shorter than string", function () {
        expect($ellipsis("12345", 3)).toBe("123...");
    });

    it("ellipsis: length shorter than string with space", function () {
        expect($ellipsis("12345 7890", 3)).toBe("123...");
    });

    it("ellipsis: length longer than string with space", function () {
        expect($ellipsis("12345 7890", 8)).toBe("12345...");
    });
});