//e2e test, run using http://localhost/studio?dev=test

F.speed = 50;

//this is an helper function to overwrite the funcunit.find function
//the find function doesn't seem to be working - aka - it not async as promised
//therefore, the new find simply concatenate the selectors - which is not the correct thing
//to do but it works 90% of the times.
FFindWrapper = function (item) {
    //overwrite find operation to make it work
    item.find = function (selector) {
        return FFindWrapper(F(item.selector + " " + selector));
    }
    return item;
}

FS = {
    selectWithText: function (selector, text) {
        if (text) {
            return F(selector + ":contains(\"" + text + "\")");
        } else {
            return F(selector);
        }
    },
    menuItem: function (text) {
        return FFindWrapper(FS.selectWithText(".menu-item", text));
    },
    button: function (text) {
        return FS.selectWithText("button", text);
    },    
    modalDialog: function () {
        return FFindWrapper(F(".modal-dialog"));
    },

    showMainMenu: function () {
        return F(".header .header-nav-menu").click();
    },

    searchBox: function () {
        return F(".header .search .search-box input");
    },
    searchResults: function (text) {
        var result = FFindWrapper(FS.selectWithText(".header .suggestions .services .result", text));        
        result.click = function() {
            return result.find(".result-name").click();
        }
        return result;
    },
    searchResultsForService: function (service,text) {
        var result = FFindWrapper(FS.selectWithText(".header .suggestions .services .service:contains('"+service+"') .result", text));
        result.click = function() {
            return result.find(".result-name").click();
        }
        return result;
    },
    searchResultsDetails: function (text) {
        return FFindWrapper(FS.selectWithText(".header .suggestions .results .resource-result",text));
    },
    searchResultsDetailsInScrollView: function (text) {
        return FFindWrapper(FS.selectWithText(".header .suggestions .results .results-scroll .resource-result", text));
    },

    scroll: function(direction, amount, callback) {
        return F(".scrollbar-content").last().scroll(direction, amount, callback);
    },

    showSelectionTools: function () {
        return FFindWrapper(F(".header").move(".header .select-tools"));
    },
    selectionTool: function (tool) {
        //tool: arrow , magic-wand , freehand , polygon , watershed
        return F(".header .menu-tool:not(.disabled) ."+tool);
    },
    disabledSelectionTool: function (tool) {
        //tool: arrow , magic-wand , freehand , polygon , watershed
        return F(".header .menu-tool.disabled ." + tool);
    },

    selectionTag: function (index) {
        return FFindWrapper(F(".header .search-box-tag:eq(" + index + ")"));
    },

    panel: function (panelClass) {
        var panel = F(panelClass);

        //show the panel if it is minimized
        panel.show = function () {
            if (panel.hasClass("minimized")) {
                panel.find(".docking-pin").click();
            }
        }
        //hide the panel if it is not already
        panel.hide = function () {
            if (!panel.hasClass("minimized")) {
                panel.find(".docking-pin").click();
            }
        }

        return panel;
    },
    library: function () {
        return this.panel(".library");
    },
    dashboard: function() {
        return this.panel(".dashboard-window");
    },

    currentMap: function () {
        return FFindWrapper(F(".library .map.current"));
    },
    showCurrentMapMenu: function () {
        return F(".library .map.current header .item-title").click();
    },

    currentMapItems: function (text) {
        var item = FFindWrapper(FS.selectWithText(".library .map.current .item", text));

        //get the "eye" icon for the map item.
        item.statusIcon = function () {
            return item.find(".status i");
        }
        //show the popup menu for the map item by clicking the map item
        item.showMenu = function () {
            item.find('.item-title').click();
        }
        //show the styling popup menu by clicking the style box
        item.showStylingMenu = function () {
            item.find('.style-palette-box-min').click();
        }
        //check if the current map item is active (visible)
        item.active = function () {
            return item.hasClass("active", true, "item " + text + " is active");
        }
        //check if the current map item is not active (not visible)
        item.notActive = function () {
            return item.hasClass("active", false, "item " + text + " is not active");
        }
        return item;
    },

    editCard: function () {
        var editTarget = FFindWrapper(F("[e2e-name=\"edit-resource-card\"]"));

        editTarget.input = function (text) {
            // use case-insensitive select and return the first match
            return editTarget.find(".form-control:not(div)[name*=\"" + text.toLowerCase() + "\"], .form-control [class*=\"" + text.toLowerCase() + "\"]");
        };

        return editTarget;
    },
    editProperties: function () {
        var editTarget = FFindWrapper(F("[e2e-name=\"edit-resource-properties\"]"));

        editTarget.input = function (index, text) {
            // use case-insensitive select and return the first match
            return editTarget.find(".field:eq(" + index + ") [name=\"" + text.toLowerCase() + "\"]");
        };

        return editTarget;
    },

    dashboardWidget: function (text) {
        return FFindWrapper(FS.selectWithText(".dashboard-window .grid-item", text));
    },
    dashboardWidgetTitle: function (text) {
        return FS.selectWithText(".dashboard-window .grid-item .title", text);
    }    
}

FS.complex = {
    importGeoSource: function (search) {
        //search the string
        FS.searchBox().type("");
        FS.searchBox().type(search);
        //wait until results are visible
        F.wait(1000);
        FS.searchResults(search).visible("no search results found");

        //click on search result
        FS.searchResultsForService("WorldView Gallery", search).click();
        FS.searchResultsDetails().visible("result visible");

        //click the import button
        FS.button("Import").click();
    },
    createNewMap: function () {
        FS.showMainMenu();
        FS.menuItem("New Globe").click();
        FS.currentMap().visible();
    },
    removeCurrentMap: function () {
        FS.currentMap().visible();
        FS.showCurrentMapMenu();
        FS.menuItem("Remove").click();
    },
    editCurrentMap: function() {
        FS.showCurrentMapMenu();
        FS.menuItem("Edit").click();
        FS.modalDialog().visible();
    },
    editCurrentMapItem: function(name) {
        FS.currentMapItems(name).showMenu();
        FS.menuItem("Edit").click();
        FS.modalDialog().visible();
    },
    changeGeoSourceColor: function (name, colorIndex) {
        FS.currentMapItems(name).showStylingMenu();
        F('.geosource-style').visible();

        var color;
        var solidColorSelection = ".geosource-style .color-grid .grid-item:eq(" + colorIndex + ")";
        F(solidColorSelection).visible(function () {
            color = $(solidColorSelection).css('background-color');
        }).click();

        //check that style got updated with the right color (fill or line)
        FS.globe.expectGeoSourceStyle(name, function (style) {
            return style.Icon && FS.globe.sameColor(style.Icon.Color, color) ||
                    style.Line && FS.globe.sameColor(style.Line.Color, color) ||
                    style.Fill && FS.globe.sameColor(style.Fill.Color, color);
        });
    },
    changeGeoSourceIcon: function (name, iconIndex) {
        FS.currentMapItems(name).showStylingMenu();
        F('.geosource-style').visible();

        var icon;
        var iconSelection = ".geosource-style .icon-grid .grid-item:eq(" + iconIndex + ")";
        F(iconSelection).visible(function () {
            icon = $(iconSelection).find('img').prop('src');
        }).click();

        //check that style got updated with the right icon
        FS.globe.expectGeoSourceStyle(name, function (style) {
            return style.Icon && icon === style.Icon.IconDataUrl;
        });
    },
    changeGeoSourceFillType: function (name, fillTypeIndex) {
        FS.currentMapItems(name).showStylingMenu();
        F('.geosource-style').visible();

        var fillType;
        var fillTypeSelection = ".geosource-style .fill-grid .grid-item:eq(" + fillTypeIndex + ")";
        F(fillTypeSelection).visible(function () {
            fillType = $(fillTypeSelection).find('img').prop('src');
        }).click();

        //check that style got updated with the right fill type
        FS.globe.expectGeoSourceStyle(name, function (style) {
            return (fillTypeIndex === 0 && style.Fill)
                || (fillTypeIndex === 1 && style.Line);
        });
    }
}

//FS.globe is used to interact with the globe
//
//Note: FS.globe is defined inside test-controller, as it need access to $pyx.globe
//
//FS.globe = { 
//    click(index,event{clientX:10,clientY:10}),
//    rightClick(index,event{clientX:10,clientY:10}),
//    gotoCamera(camera [,duration]),
//    sameColor(colorA,colorB),
//    expectGeoSourceStyle(geoSourceName[,style][,styleCompareFunction(style)]),
//}