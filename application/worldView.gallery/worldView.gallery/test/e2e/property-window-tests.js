//e2e test, run using http://localhost/studio?dev=test

if (!window.tests) {
    window.tests = {}
}


window.tests.properties = function () {
    describe("property window", function () {
        var coverage = "GTOPO30";
        var polygons = "World Political Boundaries";

        it("import geoSources", function () {
            FS.complex.createNewMap();
            FS.complex.importGeoSource(coverage);
            FS.complex.importGeoSource(polygons);
            FS.searchBox().type("[escape]");
        });

        it("right click show property window", function () {
            FS.globe.rightClick('2-6060040040');
            F('.properties-popup').visible();
            F('.properties-popup .value:contains("910 [m]")').visible();
            F('.properties-popup .value:contains("Alberta")').visible();
        });

        it("right click again moves property window", function () {
            FS.globe.rightClick('3-3010004040');
            F('.properties-popup').visible();
            F('.properties-popup .value:contains("316 [m]")').visible();
            F('.properties-popup .value:contains("Ontario")').visible();
        });
  
        it("change gradient inside property window", function () {

            //check if coverage has properties style visible
            F(".properties-popup .style.active").visible();

            //get current style
            var oldStyle;

            //get the style...
            FS.globe.expectGeoSourceStyle(coverage, function (style) {
                oldStyle = style;
                return true;
            });

            F(".properties-popup").move(".properties-popup .style.active");
            F(".properties-popup .style.active .menu-item:eq(0)").visible().click();

            //check that style has changed
            FS.globe.expectGeoSourceStyle(coverage, function (style) {
                return !angular.equals(oldStyle, style);
            });
        });

        it("change gradient to a new field inside property window", function () {            

            //check if polygons has a field without styling...
            F(".properties-popup .field:contains('HIT') .style .style-palette-box-min").missing();

            //move mouse into a field
            F(".properties-popup").move(".properties-popup .field:contains('HIT')");

            //check the style "+" icon appears
            F(".properties-popup .field:contains('HIT') .style .add-palette-box-min").visible();

            //move mouse on the style "+" icon so palette popup menu appear
            F(".properties-popup .field:contains('HIT')").move(".properties-popup .field:contains('HIT') .style");

            //get current style
            var oldStyle;

            //get the style...
            FS.globe.expectGeoSourceStyle(polygons, function (style) {
                oldStyle = style;
                return true;
            });

            F(".properties-popup .field:contains('HIT') .style .menu-item:eq(1)").visible().click();

            //check that style has changed
            FS.globe.expectGeoSourceStyle(polygons, function (style) {
                return !angular.equals(oldStyle, style);
            });

            //make sure we get the field been styled.
            F(".properties-popup .field:contains('HIT') .style .style-palette-box-min").visible();
        });


        it("adding widget from property window", function () {
            //click the add to dashboard
            F(".properties-popup .field:contains('Elevation') .on-dashboard").click();

            //click the "avg. elevation"
            F(".add-widget-layout .grid-item:contains('Avg.')").click();

            //check if we got a new dashboard widget
            FS.dashboardWidgetTitle("Elevation").visible();
        });

        it("remove current map", function () {
            FS.complex.removeCurrentMap();
        });
    });
};