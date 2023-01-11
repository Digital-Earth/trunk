//e2e test, run using http://localhost/studio?dev=test

if (!window.tests) {
    window.tests = {}
}


window.tests.style = function () {
    describe("basic styling", function () {
        var coverage = "GTOPO30";
        var polygons = "World Political Boundaries";
        var points = "Global Shark Attacks";

        it("import geoSources", function () {
            FS.complex.createNewMap();
            FS.complex.importGeoSource(coverage);
            FS.complex.importGeoSource(polygons);
            FS.complex.importGeoSource(points);
        });

        it("apply gradient to coverage", function () {
            //get current style
            var oldStyle;

            //get the style...
            FS.globe.expectGeoSourceStyle(coverage, function (style) {
                oldStyle = style;
                return true;
            });

            FS.currentMapItems(coverage).showStylingMenu();
            F('.geosource-style').visible();
            F('.geosource-style .palette-item:eq(0)').click();

            //check that style has changed
            FS.globe.expectGeoSourceStyle(coverage, function (style) {
                return !angular.equals(oldStyle, style);
            });

        });

        it("apply solid to polygons", function () {
            FS.complex.changeGeoSourceColor(polygons, 3);
        });

        it("change icon image to points", function () {
            FS.complex.changeGeoSourceIcon(points, 2);
        });

        it("change icon color to points", function () {
            FS.complex.changeGeoSourceColor(points, 6);
        });

        it("apply fill style to polygons", function () {
            FS.complex.changeGeoSourceFillType(polygons, 0);
        });

        it("remove current map", function () {
            FS.complex.removeCurrentMap();
        });
    });
};

