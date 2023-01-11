//e2e test, run using http://localhost/studio?dev=test

if (!window.tests) {
    window.tests = {}
}

window.tests.edit = function () {
    describe("Editing Resources", function () {

        it("import GeoSource", function () {
            FS.complex.createNewMap();
            FS.complex.importGeoSource("World Political Boundaries");
        });

        it("Scroll through edit menu", function () {
            var oldName = "World Political Boundaries";

            FS.complex.editCurrentMapItem(oldName);

            FS.scroll("top", 500);

            return F("body").wait(
                function () { //checker function
                    return F(".scrollbar-content").last().scrollTop() === 500;
                },
                5000, //timeout
                function () { }, //success
                "edit content could not be scrolled"); //message
        });

        it("edit GeoSource Name, Description, and Tags", function () {
            var oldName = "World Political Boundaries";
            var newName = "WPB";
            var newDescription = "WPB 2000";
            var newTag = "WPB";

            FS.complex.editCurrentMapItem(oldName);

            FS.editCard().input("Name").type("").type(newName);
            FS.editCard().input("Description").type("").type(newDescription);
            FS.editCard().input("Tag").type("").type(newTag);

            FS.button("Save").click();

            FS.complex.editCurrentMapItem(newName);

            //check that the GeoSource was updated
            F("body").wait(
                function () { //checker function
                    return FS.editCard().input("Name").val() === newName
                        && FS.editCard().input("Description").val() === newDescription;
                },
                5000, //timeout
                function() {                    
                }, //success
                "card metadata was not updated as expected"); //message

            FS.button("Cancel").click();
        });

        it("edit Map Name, Description, and Tags", function () {
            var newName = "WPB Map";
            var newDescription = "WPB 2000";
            var newTag = "WPB";

            FS.complex.editCurrentMap();
            
            FS.editCard().input("Name").type("").type(newName);
            FS.editCard().input("Description").type("").type(newDescription);
            FS.editCard().input("Tag").type("").type(newTag);

            FS.button("Save").click();

            FS.complex.editCurrentMap();

            //check that the GeoSource was updated
            F("body").wait(
                function () { //checker function
                    return FS.editCard().input("Name").val() === newName
                        && FS.editCard().input("Description").val() === newDescription;
                },
                5000, //timeout
                function () {
                }, //success
                "card metadata was not updated as expected"); //message

            FS.button("Cancel").click();
        });

        it("edit GeoSource Properties", function () {
            var name = "WPB";
            var newName = "Count";
            var newDescription = "The count";
            var newUnit = "Unit";

            FS.complex.editCurrentMapItem(name);

            FS.editProperties().input(1, "Name").type("").type(newName);
            FS.editProperties().input(1, "Unit").type("").type(newUnit);
            FS.editProperties().input(1, "Description").type("").type(newDescription);

            FS.button("Save").click();

            FS.complex.editCurrentMapItem(name);

            //check that the GeoSource was updated
            F("body").wait(
                function () { //checker function
                    return FS.editProperties().input(1, "Name").val() === newName
                        && FS.editProperties().input(1, "Unit").val() === newUnit
                        && FS.editProperties().input(1, "Description").val() === newDescription;
                },
                5000, //timeout
                function () {
                }, //success
                "Property metadata was not updated as expected"); //message

            FS.button("Cancel").click();
        });

        it("remove current map", function () {
            FS.complex.removeCurrentMap();
        });
    });
};

