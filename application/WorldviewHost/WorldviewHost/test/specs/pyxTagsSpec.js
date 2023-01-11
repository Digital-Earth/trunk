/// <reference path="/scripts/jquery-2.1.1.js" />
/// <reference path="/scripts/angular-1.2.20.js" />
/// <reference path="/scripts/angular-cookies-1.2.20.js" />
/// <reference path="/test/angular-mocks-fix.js" />
/// <reference path="/scripts/pyxis/pyxis.js" />
/// <reference path="/scripts/pyxis/pyxis.gallery.js" />
/// <reference path="/scripts/pyxis/pyxis.user.js" />
/// <reference path="/scripts/pyxis/pyxis.area.js" />


describe("$pyx.tags", function () {
    var $pyx;
    beforeEach(module("pyxis"));
    beforeEach(module("ngCookies"));
    beforeEach(inject(function (_$pyx_) {
        $pyx = _$pyx_;
    }));

    function createItemWithTags(tags) {
        return {
            Metadata: {
                Tags: tags
            }
        };
    };

    function createItemWithTagsAndSystemTags(tags, systemTags) {
        return {
            Metadata: {
                Tags: tags,
                SystemTags: systemTags
            }
        };
    };

    it("$pyx.tags exists", function () {
        expect($pyx.tags).not.toBeNull();
    });

    it("$pyx.tags.ui.Favorite exists and correct", function () {
        expect($pyx.tags.ui.Favorite).toEqual("Favorite");
    });

    it("$pyx.tags.ui.Geotagable exists and correct", function () {
        expect($pyx.tags.ui.Geotagable).toEqual("Geotagable");
    });

    it("$pyx.tags.ui.Searchable exists and correct", function () {
        expect($pyx.tags.ui.Searchable).toEqual("Searchable");
    });

    it("$pyx.tags.exists work with empty array", function () {
        expect($pyx.tags([]).exists("tag")).toBeFalsy();
    });

    it("$pyx.tags.exists work with null array", function () {
        expect($pyx.tags().exists("tag")).toBeFalsy();
    });

    it("$pyx.tags.exists work with array with single item", function () {
        expect($pyx.tags(["tag"]).exists("tag")).toBeTruthy();
    });

    it("$pyx.tags.toggle add tag to input array", function () {
        var array = [];
        $pyx.tags(array).toggle("tag");
        expect(array).toEqual(["tag"]);
    });

    it("$pyx.tags.toggle remove tag input array", function () {
        var array = ["tag"];
        $pyx.tags(array).toggle("tag");
        expect(array).toEqual([]);
    });

    it("$pyx.tags.toggle works on empty array", function () {
        expect($pyx.tags().toggle("tag")).toEqual("tag");
    });

    it("$pyx.tags.toggle to return undefined if tag has been removed", function () {
        expect($pyx.tags(["tag"]).toggle("tag")).toBeFalsy();
    });

    it("$pyx.tags.toggle to return tag value if tag has been added", function () {
        expect($pyx.tags(["tag2"]).toggle("tag")).toEqual("tag");
    });
    
    it("$pyx.tags.add works on empty array", function () {
        expect($pyx.tags().add("tag")).toEqual("tag");
    });

    it("$pyx.tags.add to return null if no tag was added array", function () {
        expect($pyx.tags(["tag"]).add("tag")).toBeFalsy();
    });

    it("$pyx.tags.add to return null if no tag was added array", function () {
        expect($pyx.tags(["tag2"]).add("tag")).toEqual("tag");
    });

    it("$pyx.tags.add add tag to input array without a tag", function () {
        var array = [];
        $pyx.tags(array).add("tag");
        expect(array).toEqual(["tag"]);
    });

    it("$pyx.tags.add add tag to input array without a tag at last position", function () {
        var array = ["hello", "world"];
        $pyx.tags(array).add("tag");
        expect(array).toEqual(["hello", "world", "tag"]);
    });

    it("$pyx.tags.remove works if no tag found int array", function () {
        var array = ["tag2"];
        $pyx.tags(array).remove("tag");
        expect(array).toEqual(["tag2"]);
    });

    it("$pyx.tags.remove works if tag is the first tag in array", function () {
        var array = ["tag", "tag2"];
        $pyx.tags(array).remove("tag");
        expect(array).toEqual(["tag2"]);
    });

    it("$pyx.tags.remove works if tag is found in the middle of array", function () {
        var array = ["tag2", "tag", "tag3"];
        $pyx.tags(array).remove("tag");
        expect(array).toEqual(["tag2", "tag3"]);
    });

    it("$pyx.tags.remove works if tag is found in the middle of array", function () {
        var array = ["tag2", "tag3", "tag"];
        $pyx.tags(array).remove("tag");
        expect(array).toEqual(["tag2", "tag3"]);
    });

    it("$pyx.tags.remove works on empty array", function () {
        expect($pyx.tags().remove("tag")).toBeFalsy();
    });

    it("$pyx.tags.remove to return null if no tag has been removed", function () {
        expect($pyx.tags(["tag2"]).remove("tag")).toBeFalsy();
    });

    it("$pyx.tags.remove to return tag if tag has been removed", function () {
        expect($pyx.tags(["tag"]).remove("tag")).toEqual("tag");
    });

    it("$pyx.tags.all return array even if input is null", function () {
        expect($pyx.tags().all().length).toEqual(0);
    });

    it("$pyx.tags.all return the same input array ", function () {
        var array = ["hello", "world"];
        expect($pyx.tags(array).all()).toEqual(array);
    });

    it("$pyx.tags.itemTags works with null item", function () {        
        expect($pyx.tags.itemTags(null).all()).toEqual([]);
    });

    it("$pyx.tags.itemTags works with valid item tags", function () {
        var item = createItemWithTags(["A", "B"]);
        expect($pyx.tags.itemTags(item).all()).toEqual(["A","B"]);
    });

    it("$pyx.tags.itemTags modify tags of a valid item", function () {
        var item = createItemWithTags(["A", "B"]);
        $pyx.tags.itemTags(item).toggle("B");
        $pyx.tags.itemTags(item).toggle("C");
        expect(item.Metadata.Tags).toEqual(["A", "C"]);
    });

    it("$pyx.tags.itemTags create empty Tags if no Tags item defined for the Metadata", function () {
        var item = { 'Metadata': {} };
        $pyx.tags.itemTags(item).add("B");        
        expect(item.Metadata.Tags).toEqual(["B"]);
    });

    it("$pyx.tags.itemSystemTags works with valid item with tags but with no system tags", function () {
        var item = createItemWithTags(["A", "B"]);
        expect($pyx.tags.itemSystemTags(item).all()).toEqual([]);
    });

    it("$pyx.tags.itemSystemTags does't modify system tags", function () {
        var item = createItemWithTags(["A", "B"]);
        $pyx.tags.itemSystemTags(item).toggle("A");
        expect(item.Metadata.Tags).toEqual(["A", "B"]);
        expect(item.Metadata.SystemTags).toEqual(["A"]);
    });

    it("$pyx.tags.itemSystemTags modify tags of a valid item", function () {
        var item = createItemWithTagsAndSystemTags(["A", "B"],["C","D"]);
        $pyx.tags.itemSystemTags(item).toggle("B");
        $pyx.tags.itemSystemTags(item).toggle("C");
        expect(item.Metadata.SystemTags).toEqual(["D", "B"]);
    });

    it("$pyx.tags.itemSystemTags create empty Tags if no Tags item defined for the Metadata", function () {
        var item = { 'Metadata': {} };
        $pyx.tags.itemSystemTags(item).add("B");
        expect(item.Metadata.SystemTags).toEqual(["B"]);
    });
});