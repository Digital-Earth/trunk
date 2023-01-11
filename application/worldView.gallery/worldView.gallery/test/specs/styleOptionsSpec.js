/// <reference path="/test/worldview-studio-mocks.js" />
/// <reference path="/contents/scripts/Studio/LegacyServices/styleOptions.js" />

describe("styleOptions", function () {
    var styleOptions;
    var $filter;
    beforeEach(module("worldViewStudio"));
    beforeEach(inject(function (_styleOptions_) {
        styleOptions = _styleOptions_;        
    }));

    it("toRgba with '#ffffff' should return r=g=b=255 and a=1.0 ", function () {
        var rgba = styleOptions.toRgba("#ffffff");
        expect(rgba.r).toBe(255);
        expect(rgba.g).toBe(255);
        expect(rgba.b).toBe(255);
        expect(rgba.a).toBe(1.0);
    });

    it("toRgba with '#80ff00' should return r=128,g=255,b=0 and a=1.0", function () {
        var rgba = styleOptions.toRgba("#80ff00");
        expect(rgba.r).toBe(128);
        expect(rgba.g).toBe(255);
        expect(rgba.b).toBe(0);
        expect(rgba.a).toBe(1.0);
    });

    it("toRgba with 'white' should return undefined", function () {
        var rgba = styleOptions.toRgba("white");
        expect(rgba).toBeUndefined();        
    });

    it("toRgba with 'rgba(255, 128 ,0, 1.0)' should return {r:255,g:128,b:0,a:1.0}", function () {
        var rgba = styleOptions.toRgba("rgba(255, 128 ,0, 1.0)");
        expect(rgba.r).toBe(255);
        expect(rgba.g).toBe(128);
        expect(rgba.b).toBe(0);
        expect(rgba.a).toBe(1.0);
    });

    it("toRgba with 'rgba(255, 128 ,0, 50%)' should return {r:255,g:128,b:0,a:1.0}", function () {
        var rgba = styleOptions.toRgba("rgba(255, 128 ,0, 50%)");
        expect(rgba.r).toBe(255);
        expect(rgba.g).toBe(128);
        expect(rgba.b).toBe(0);
        expect(rgba.a).toBe(0.5);
    });

    it("toRgba with 'rgb( 255,128,0 )' should return {r:255,g:128,b:0,a:1.0}", function () {
        var rgba = styleOptions.toRgba("rgb( 255,128,0 )");
        expect(rgba.r).toBe(255);
        expect(rgba.g).toBe(128);
        expect(rgba.b).toBe(0);
        expect(rgba.a).toBe(1.0);
    });

    it("toRgba with {r:255,g:128,b:0,a:1.0} should return {r:255,g:128,b:0,a:1.0}", function () {
        expect(styleOptions.toRgba({ r: 255, g: 128, b: 0, a: 1.0 })).toEqual({ r: 255, g: 128, b: 0, a: 1.0 });
    });

    it("toRgba with {} should return undefined", function () {
        expect(styleOptions.toRgba({})).toBeUndefined();
    });

    it("toRgba with undefined should return undefined", function () {
        expect(styleOptions.toRgba(undefined)).toBeUndefined();
    });

    it("toRgba with 'rgba()' should return undefined", function () {
        expect(styleOptions.toRgba('rgba()')).toBeUndefined();
    });

    it("toCss with {r:255,g:128,b:0,a:1.0} should return #FF8000", function () {        
        expect(styleOptions.toCss({ r: 255, g: 128, b: 0, a: 1.0 })).toBe("#FF8000");        
    });

    it("toCss with {r:255,g:128,b:0,a:0.3} should return rgba(255,128,0,0.3)", function () {
        expect(styleOptions.toCss({ r: 255, g: 128, b: 0, a: 0.3 })).toBe("rgba(255,128,0,0.3)");
    });
});