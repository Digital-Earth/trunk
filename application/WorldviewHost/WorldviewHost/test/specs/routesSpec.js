/// <reference path="/test/worldview-site-mocks.js"/>
/// <reference path="/scripts/pyxis/pyxis.js" />
/// <reference path="/scripts/pyxis/pyxis.gallery.js" />
/// <reference path="/scripts/pyxis/pyxis.user.js" />
/// <reference path="/scripts/pyxis/pyxis.area.js" />
/// <reference path="/contents/scripts/site/directives.js" />
/// <reference path="/contents/scripts/site/routes.js" />

'use strict';

describe('routes', function () {
    var $pyx;
    beforeEach(module('worldViewSite'));
    beforeEach(module("pyxis"));
    beforeEach(module("ngCookies"));
    beforeEach(inject(function (_$pyx_) {
        $pyx = _$pyx_;
    }));

    it('route with /info/faq should route to wv-faq.html', function () {
        inject(function ($route, $location, $rootScope, $httpBackend) {

            $httpBackend.whenGET('/contents/templates/wv-faq.html').respond(200);
            $location.path('/info/faq');
            $rootScope.$digest();

            expect($route.current.templateUrl).toBe('/contents/templates/wv-faq.html');
            expect($route.current.controller).toBe('worldviewFaqController');
            
        });
    });

    it('route with /info/invalid should route to wv-main.html', function () {
        inject(function ($route, $location, $rootScope, $httpBackend) {

            $httpBackend.whenGET('/contents/templates/wv-main.html').respond(200);
            $location.path('/info/invalid');
            $rootScope.$digest();

            expect($route.current.templateUrl).toBe('/contents/templates/wv-main.html');
            expect($route.current.controller).toBe('worldviewSearchController');

        });
    });

    it('route with /TestGallery should route to wv-gallery.html', function () {
        inject(function ($route, $location, $rootScope, $httpBackend) {

            $httpBackend.whenGET('/contents/templates/wv-gallery.html').respond(200);
            $httpBackend.whenGET('https://api.pyxis.worldview.gallery/api/v1/Gallery?name=TestGallery').respond(200);
            $location.path('/TestGallery');
            $rootScope.$digest();

            expect($route.current.templateUrl).toBe('/contents/templates/wv-gallery.html');
            expect($route.current.controller).toBe('worldviewGalleryController');

        });
    });

    it('route with /InvalidGallery should redirect to wv-main.html', function () {
        inject(function ($route, $location, $rootScope, $httpBackend) {

            $httpBackend.whenGET('/contents/templates/wv-gallery.html').respond(200);
            $httpBackend.whenGET('/contents/templates/wv-main.html').respond(200);
            $httpBackend.whenGET('https://api.pyxis.worldview.gallery/api/v1/Gallery?name=InvalidGallery').respond(404);
            $location.path('/InvalidGallery');
            $rootScope.$digest();
            $httpBackend.flush(1); // move fake async requests ahead one

            expect($route.current.templateUrl).toBe('/contents/templates/wv-main.html');
            expect($route.current.controller).toBe('worldviewSearchController');

        });
    });

});