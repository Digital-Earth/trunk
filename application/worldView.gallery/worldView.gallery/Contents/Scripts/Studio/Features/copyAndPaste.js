// Name: Copy and Paste
//------------------------------------------------------
app.service('featureCopyAndPaste', function ($pyx, $timeout) {

    function register ($scope) { 
        //get the field spec for a give map item.
        function findFieldByName(mapItem, name) {
            return $pyx.spec(mapItem.Specification).field(name);
        }

        //validate we can copy palette style for a style type (Fill/Icon)
        function validatePaletteType(targetMapItem, sourceMapItem, styleType) {
            var sourceField = $pyx.obj.get(sourceMapItem, 'Style', styleType, 'PaletteExpression');
            var targetField = $pyx.obj.get(targetMapItem, 'Style', styleType, 'PaletteExpression');

            //if there is not source field - no style will be copied
            if (!sourceField) {
                return true;
            }
            var sourceFieldSpec = findFieldByName(sourceMapItem, sourceField);
            var targetHasSameField = findFieldByName(targetMapItem, sourceField);
            //if source and target share the same field name, make sure they are of the same type
            if (targetHasSameField) {
                return sourceFieldSpec.Type === targetHasSameField.Type;
            }

            //if we have source field and no suggested target field... we can't paste
            if (!targetField) {
                return false;
            }
            //make sure target and source field have the same spec
            var targetFieldSpec = findFieldByName(targetMapItem, targetField);
            return sourceFieldSpec.Type === targetFieldSpec.Type;
        }

        //return true if we can paste style from one map item to the other
        function canPasteStyle (sourceMapItem, targetMapItem) {
            if (!sourceMapItem) {
                return false;
            }

            if (targetMapItem.Specification.OutputType !== sourceMapItem.Specification.OutputType) {
                return false;
            }
            if (targetMapItem.Specification.OutputType === "Coverage") {
                return true;
            } else {
                return validatePaletteType(targetMapItem, sourceMapItem, 'Icon') &&
                       validatePaletteType(targetMapItem, sourceMapItem, 'Fill');
            }
        }

        //return a new style object that will match style of the source item and the spec of the target item
        function createPastedStyle(sourceMapItem, targetMapItem) {
            var newStyle = sourceMapItem.Style;

            if (targetMapItem.Specification.OutputType === "Coverage") {
                var requestedField = findFieldByName(targetMapItem, newStyle.Fill.PaletteExpression);
                //if the target item doesn't have the requested field 
                if (!requestedField) {
                    //try to use previously used field
                    if ($pyx.obj.get(targetMapItem.Style, 'Fill', 'PaletteExpression')) {
                        newStyle.Fill.PaletteExpression = targetMapItem.Style.Fill.PaletteExpression;
                    } else {
                        //take the first field
                        newStyle.Fill.PaletteExpression = targetMapItem.Specification.Fields[0].Name;
                    }
                }
            } else { //vectors
                var iconField = $pyx.obj.get(newStyle, 'Icon', 'PaletteExpression');

                if (iconField) {
                    var requestIconField = findFieldByName(targetMapItem, iconField);
                    if (!requestIconField) {
                        //use the old icon field - but with new style
                        newStyle.Icon.PaletteExpression = targetMapItem.Style.Icon.PaletteExpression;
                    }
                }
                var fillField = $pyx.obj.get(newStyle, 'Fill', 'PaletteExpression');

                if (fillField) {
                    var requestFillField = findFieldByName(targetMapItem, fillField);
                    if (!requestFillField) {
                        //use the old icon field - but with new style
                        newStyle.Fill.PaletteExpression = targetMapItem.Style.Fill.PaletteExpression;
                    }
                }
            }
            return newStyle;
        }

        // Exposed Copy and Paste interface
        $scope.copyAndPaste = {};
        // Enable the functionality 
        $scope.copyAndPaste.enable = $scope.activeFeatures.copyAndPaste;

        // Cache the source map 
        $scope.copyAndPaste.sourceMapItem = null;

        $scope.copyAndPaste.canPasteStyle = canPasteStyle;

        // Select the source item to copy
        // @param {object} mapItem - source item 
        $scope.copyAndPaste.copyFrom = function(mapItem) {
            $scope.copyAndPaste.sourceMapItem = angular.copy(mapItem);
            $scope.notifyInfo("Palette copied");
        }

        // Paste to all compatible items in the map
        $scope.copyAndPaste.pasteAll = function () {
            var items = $scope.currentMap.items();
            var total = items.length;

            for (var index = 0; index < total; index++) {
                var mapItem = items[index];
                try {
                    if (canPasteStyle($scope.copyAndPaste.sourceMapItem, mapItem)) { 
                        mapItem.Style = createPastedStyle($scope.copyAndPaste.sourceMapItem, mapItem);
                        var palette = mapItem.Style.Fill.Palette || mapItem.Style.Icon.Palette;
                        $scope.styleModifier.setItemPalette(mapItem, palette);
                    }
                } catch (e) {
                    continue;
                }
            }

            $scope.notifyInfo("Palette pasted to all compatible geosources in this globe.");   
        }

        // Paste to another compatible item 
        // @param {object} mapItem - target item 
        $scope.copyAndPaste.pasteTo = function (mapItem) {
            if (canPasteStyle($scope.copyAndPaste.sourceMapItem, mapItem)) {
                mapItem.Style = createPastedStyle($scope.copyAndPaste.sourceMapItem, mapItem);
                $scope.notifyInfo("Palette pasted");

            } else {
                $scope.notifyError("Sorry, can't complete the paste operation.");
            }
        }
    
    }
    return {
        register: register
    }
});