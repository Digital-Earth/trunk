app.service("featureStyle", function ($pyx, $pyxIntercom, dispatcher) {

    function register($scope) {
        // style modifier 
        $scope.styleModifier = {
            styleOptions: $scope.styleOptions,

            updatePalleteStep: function (item, step, newColor) {
                step.Color = newColor;
                dispatcher.actions.updateItemStyle.invoke({ item: item });
                $scope.notifyLibraryChange();
            },
            setItemPalette: function (item, palette) {
                if (item.Style.Icon && item.Style.Icon.PaletteExpression) {
                    item.Style.Icon.Palette = palette;
                    dispatcher.actions.updateItemStyle.invoke({ item: item });
                    $scope.notifyLibraryChange();
                } else if (item.Style.Fill && item.Style.Fill.PaletteExpression) {
                    item.Style.Fill.Palette = palette;
                    dispatcher.actions.updateItemStyle.invoke({ item: item });
                    $scope.notifyLibraryChange();
                }

                $pyxIntercom.track('set-palette');
            },

            toggleShowAsElevation: function (item) {
                item.Style.ShowAsElevation = !item.Style.ShowAsElevation;
                dispatcher.actions.updateItemStyle.invoke({ item: item });
                $scope.notifyLibraryChange();
            },

            updatePalette: function (item, palette, options) {
                options = options || {}
                if (item.Style.Icon && item.Style.Icon.PaletteExpression) {
                    dispatcher.actions.calculateItemStyle.invoke({
                        'item': item,
                        'field': item.Style.Icon.PaletteExpression,
                        'palette': palette,
                        'onScreen': options.useScreenBasedStyling
                    });
                }
                if (item.Style.Fill && item.Style.Fill.PaletteExpression) {
                    dispatcher.actions.calculateItemStyle.invoke({
                        'item': item,
                        'field': item.Style.Fill.PaletteExpression,
                        'palette': palette,
                        'onScreen': options.useScreenBasedStyling
                    });
                }
   
                $pyxIntercom.track('update-palette');
            },

            updateIconColor: function (item, color) {
                item.Style.Icon.Color = color;
                item.Style.Icon.Style = "SolidColor";
                delete item.Style.Icon.Palette;
                delete item.Style.Icon.PaletteExpression;

                dispatcher.actions.updateItemStyle.invoke({ item: item });
                $scope.notifyStyleChange(item);
                $scope.notifyLibraryChange();
            },

            updateFillColor: function (item, color) {
                item.Style.Fill = {
                    Style: "SolidColor",
                    Color: color
                };
                delete item.Style.Line;

                dispatcher.actions.updateItemStyle.invoke({ item: item });
                $scope.notifyStyleChange(item);
                $scope.notifyLibraryChange();

            },
            updateLineColor: function (item, color) {
                item.Style.Line = {
                    Style: "SolidColor",
                    Color: color
                };
                delete item.Style.Fill;

                dispatcher.actions.updateItemStyle.invoke({ item: item });
                $scope.notifyLibraryChange();
            },
            updateIconScale: function (item, scale) {
                item.Style.Icon.Scale = scale;
                dispatcher.actions.updateItemStyle.invoke({ item: item });
                $scope.notifyLibraryChange();
            },
            updateIcon: function (item, icon) {
                item.Style.Icon.IconDataUrl = icon.oldDataUrl;
                dispatcher.actions.updateItemStyle.invoke({ item: item });
                $scope.notifyLibraryChange();
            },
            isSameGeoSource: function (objResource, domResource) {
                return (objResource.Resource.Id === domResource.Resource.Id);
            },
            hasPaletteStyle: function (objResource) {
                return ($pyx.obj.has(objResource.Style, 'Fill', 'Palette') || $pyx.obj.has(objResource.Style, 'Icon', 'Palette'));
            }
        };

        //application

        $scope.notifyStyleChange = function (item, palette) {
            $scope.$broadcast("item-style-changed", { item: item, palette: palette });
        }
    };

    return {
        register: register
    };
});