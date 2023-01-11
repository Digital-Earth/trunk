//e2e test, run using http://localhost/studio?dev=test

if (!window.tests) {
    window.tests = {}
}

window.tests.basic = function () {
    describe("main menu", function () {
        it("click menu item show the menu", function () {
            FS.showMainMenu();
            FS.menuItem().visible("menu items are visible");
        });
        it("menu should hide when mouse leave", function () {
            FS.menuItem().move("+200 +200");
            FS.menuItem().missing("menu disappeared");
        });
        it("show menu item and click about", function () {
            FS.showMainMenu();
            FS.menuItem("About").click();
            FS.modalDialog().visible("about window appeared");
            FS.button("OK").click();
            FS.modalDialog().missing("about window disappeared");
        });
    });

    describe("search box", function () {
        it("search result should appear if text is typed", function () {
            FS.searchBox().type("");
            FS.searchBox().type("world");
            FS.searchResultsForService("WorldView Gallery").visible();

        });

        it("search result should disappear if press escape and search box text will be clear", function () {
            FS.searchBox().type("[escape]");
            FS.searchResults().invisible();
            FS.searchBox().text("");
        });
    });


    describe("map control", function () {
        it("main menu show map should create new map", function () {
            FS.complex.createNewMap();
        });

        it("create current map header should show menu", function () {
            FS.currentMap().visible();
            FS.showCurrentMapMenu();
            FS.menuItem().visible();
        });

        it("mouse leave of current map menu should hide menu", function () {
            FS.menuItem().move("+200 -20");
            FS.menuItem().invisible();
        });

        it("remove new map", function () {
            FS.currentMap().visible();
            FS.showCurrentMapMenu();
            FS.menuItem("Remove").click();
            FS.currentMap().missing();
        });

    });

}
