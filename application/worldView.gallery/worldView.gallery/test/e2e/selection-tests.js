//e2e test, run using http://localhost/studio?dev=test

if (!window.tests) {
    window.tests = {}
}


window.tests.selection = function () {
    describe("selection tags", function () {
        var coverage = "GTOPO30";
        var polygons = "World Political Boundaries";

        it("magic-wand and watershed tool disabled state", function () {
            FS.complex.createNewMap();

            FS.globe.gotoCamera({
                "Latitude": 54,
                "Longitude": -112,
                "Heading": 0,
                "Altitude": 0,
                "Tilt": 0,
                "Range": 6187500
            });

            //on empty map magic-wand and watershed tools are disabled
            FS.showSelectionTools();
            FS.disabledSelectionTool('magic-wand').visible();
            FS.disabledSelectionTool('watershed').visible();
        });

        it("import coverage enable watershed tool", function () {
            FS.complex.importGeoSource(coverage);

            //map with elevation the watershed tool should be enabled
            FS.showSelectionTools();
            FS.disabledSelectionTool('magic-wand').visible();
            FS.selectionTool('watershed').visible();
        });

        it("import polygon enables magic-wand tool", function () {            

            FS.complex.importGeoSource(polygons);
            FS.searchBox().type("[escape]");

            //map with elevation and vectors both tools should be enabled
            FS.showSelectionTools();
            FS.selectionTool('magic-wand').visible();
            FS.selectionTool('watershed').visible();
        });

        it("selecting Alberta create a selection tag", function () {
            FS.selectionTag(0).missing();

            FS.showSelectionTools();
            FS.selectionTool('magic-wand').click();
            FS.globe.click('2-6060040040');

            //wait for selection to complete
            FS.searchBox().wait(1000);

            //check that we have a selection tag
            FS.selectionTag(0).visible();
        });

        it("type elevation return elevation property", function () {
            FS.searchBox().type("Elev");
            FS.searchResultsForService("Properties", "Elevation").click();
            
            //wait until we see some stats appearing
            FS.searchResultsDetails().find(".stats-item").visible();
        });

        it("click on stats item on the result add to widget dashboard", function () {
            FS.searchResultsDetails().find(".stats-item:contains('Average') .on-dashboard").click();

            //check if we got a new dashboard widget
            FS.dashboardWidgetTitle("Elevation").visible();

            //check if we got the right answer as well (it around 773)
            FS.dashboardWidget("773").visible();
        });

        it("create where is query using the range widget", function () {
            FS.searchResultsDetails().find('.selector-area .left-handle').drag("+100 +0");

            FS.button('Where is it').click();

            //wait for selection to complete
            FS.searchBox().wait(1000);

            //check that we have a new elevation range selection tag
            FS.selectionTag(1).visible();

            //check that dashboard widget has changed
            FS.dashboardWidget("1,866").visible();
        });

        it("click on range selection tag open inspector window", function () {
            //open menu
            FS.selectionTag(1).click();

            //change from value
            F(".menu-row input:eq(0)").type("");
            F(".menu-row input:eq(0)").type("1000");

            //wait until inspector window hidden
            F(".menu-row").missing();

            //check that dashboard widget has changed
            FS.dashboardWidget("1,492").visible();
        });
        
        it("click on remove button remove a search-tag", function () {
            //click the remove tag button
            FS.selectionTag(1).find('.remove-tag i').click();

            //wait until the tag disappeared
            FS.selectionTag(1).missing();

            //check that dashboard widget has changed
            FS.dashboardWidget("773").visible();
        });

        it("click [backspace] on the dashboard remove tags", function () {
            //backspace on search box will remove search tag
            FS.searchBox().type("abc[backspace][backspace][backspace][backspace]");

            //wait until the tag disappeared
            FS.selectionTag(0).missing();

            //close search box
            FS.searchBox().type('[escape]');
        });

        it("use watershed tool", function () {
            //select watershed tool.
            FS.showSelectionTools();
            FS.selectionTool('watershed').click();
            FS.globe.click('F-01002010060302');

            //wait for selection to complete
            FS.searchBox().wait(1000);

            //check that we have a selection tag
            FS.selectionTag(0).visible();
        });
        
        it("remove map remove search tags", function () {                        
            //remove current map
            FS.complex.removeCurrentMap();

            //make sure selection tag disappeared
            FS.selectionTag(0).missing();
        });
    });
};