//e2e test, run using http://localhost/studio?dev=test

if (!window.tests) {
    window.tests = {}
}

window.tests.import = function () {
    describe("importing gtopo30", function () {
        it("search gtopo30 should find a single geosource", function () {
            FS.complex.createNewMap();
            FS.searchBox().type("");            
            FS.searchBox().type("gtopo30");
            FS.searchBox().wait(2000);
            FS.searchResultsForService("WorldView Gallery", "GTOPO30").visible("no search results found");
        });

        it("click import button on search results", function () {
            FS.searchResultsForService("WorldView Gallery", "GTOPO30").click();
            FS.searchResultsDetails().visible("result visible");
            FS.button("Import").click();
        });

        it("new map should be created with one item", function () {
            FS.currentMap().visible();
            FS.currentMapItems().visible();
        });

        it("search gtopo30 should display same results but with goto and hide buttons", function () {
            FS.searchBox().type("");
            FS.searchBox().type("gtopo30");
            FS.searchBox().wait(2000);
            FS.searchResultsForService("WorldView Gallery", "GTOPO30").visible("no search results found");
            FS.searchResultsForService("WorldView Gallery", "GTOPO30").click();
            FS.button("Goto").visible();
            FS.button("Hide").visible();            
        });

        it("click hide button should hide the item and button should change to show", function () {
            FS.button("Hide").click();
            FS.button("Hide").missing();            
            FS.button("Show").visible();
            //make sure the item is not active
            FS.currentMapItems("GTOPO30").hasClass("active", false);
        });

        it("click hide button should hide the item and button should change to show", function () {
            FS.button("Show").click();
            FS.button("Show").missing();
            FS.button("Hide").visible();
            //make sure the item is not active
            FS.currentMapItems("GTOPO30").hasClass("active", true);
        });

        it("remove item from map should display an import button again", function () {
            FS.currentMapItems("GTOPO30").showMenu();
            FS.menuItem("Remove").click();
            FS.currentMapItems("GTOPO30").missing();
            FS.button("Import").visible();            
        });

        it("click import again should import the map item", function () {            
            FS.button("Import").click();
            FS.currentMapItems("GTOPO30").visible();
        });

        it("remove map should clear the map", function () {
            FS.currentMap().visible();
            FS.showCurrentMapMenu();
            FS.menuItem("Remove").click();            
        });

        it("no current map visible", function () {
            FS.currentMap().missing();
        });
    });

    describe("importing ogc", function () {
        url = "http://portal.cubewerx.com/cubewerx/cubeserv/cubeserv.cgi?service=WMS&version=1.3.0&request=GetCapabilities";
        layer = "GTOPO30";

        it("search gtopo30 should find a single geosource", function () {
            FS.complex.createNewMap();
            FS.searchBox().type("");
            FS.searchBox().type(url);
            FS.searchBox().wait(2000);
            FS.searchResults("CubeSERV").visible("no search results found");
        });

        it("click import button on search results", function () {
            //open server
            FS.searchBox().blur();

            FS.searchResults("CubeSERV").click();
 
            var searchResultDetails = FS.searchResultsDetailsInScrollView(layer);

            //let view update it self
            searchResultDetails.visible(function () {
                //scroll the item into view once it visible
                $(searchResultDetails.selector)[0].scrollIntoView();
            });

            //click import to their GTOPO30 dataset
            var importButton = FS.searchResultsDetailsInScrollView(layer).find(".selector");
            importButton.click();
            FS.button("Import").click();
            
            FS.currentMapItems(layer).visible();
        });

        it("remove test map", function () {
            FS.complex.removeCurrentMap();
            FS.currentMap().missing();
        });
    });
}