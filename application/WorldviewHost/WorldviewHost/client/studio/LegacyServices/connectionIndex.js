/* 
- @name connectionIndex
- @desc a service that stores connections to external (different from the gallery) data catalogs
- @type {service}
*/
app.service('connectionIndex', function ($pyx, $timeout, $filter) {
    // Stores all data that currently belongs to the catalog index
    var model = {
        "groups": []
    }
    var i18n = $filter('i18n');

    function findItemInIndex(value) {
        if (!value) {
            return null;
        }
        for (var i = 0; i < model.groups.length; i++) {
            var group = model.groups[i];
            for (var j = 0; j < group.results.length; j++) {
                var item = group.results[j];
                // the value may be either the object itself or URI or Id
                // note: apply case-insensitive comparison for URI
                if (item === value
                    || $pyx.obj.equalsCaseInsensitive(item.Uri, value)
                    || item.Id === value) {
                    return item;
                }
            }
        }
        return null;
    }

    // performs a regex match of given terms with item's Metadata, Uri, Id (if applicable)
    function regexMatchItemGeneralData(item, terms) {
        // check Tags
        if (item.Metadata && item.Metadata.Tags) {
            for (var i = 0; i < item.Metadata.Tags; i++) {
                if (terms.test(item.Metadata.Tags[i])) {
                    return true;
                }
            }
        }
        return item.Metadata &&
                (item.Metadata.Name && terms.test(item.Metadata.Name)
                || item.Metadata.Description && terms.test(item.Metadata.Description))
                || item.Uri && terms.test(item.Uri)
                || item.Id && terms.test(item.Id);
    }

    // recursively (if it is a catalog) performs simple regex matching inside an item (a data set or a catalog)
    // (lunr.js could be used for more sophisticated searching)
    function regexSearchItemRecursively(results, item, terms) {
        // if the item Metadata or URI fits the search, return the whole item
        if (regexMatchItemGeneralData(item, terms)) {
            results.push(item);
        } else {
            // otherwise, filter the data sets and the sub-catalogs, if applicable
            var filteredCatalog = {
                "Type": item.Type,
                "DataType": item.DataType,
                // note: not setting URI, as this filtered catalog doesn't actually correspond to it
                "Metadata": angular.copy(item.Metadata),
                "DataSets": [],
                "SubCatalogs": []
            };
            if (item.DataSets) {
                filteredCatalog.DataSets = item.DataSets.filter(function (dataSet) {
                    // check Fields
                    if (dataSet.Fields) {
                        for (var j = 0; j < dataSet.Fields; j++) {
                            if (terms.test(dataSet.Fields[j])) {
                                return true;
                            }
                        }
                    }
                    // check Uri, Name, Description, Tags, Layer
                    return regexMatchItemGeneralData(dataSet, terms)
                            || dataSet.Layer && terms.test(dataSet.Layer);
                });
            }
            if (item.SubCatalogs) {
                angular.forEach(item.SubCatalogs, function (subCatalog) {
                    regexSearchItemRecursively(filteredCatalog.SubCatalogs, subCatalog, terms);
                });
            }
            filteredCatalog.Metadata.Description = i18n('Found {0} data set(s) and {1} data container(s)')
                .replace('{0}', filteredCatalog.DataSets ? filteredCatalog.DataSets.length : 0)
                .replace('{1}', filteredCatalog.SubCatalogs ? filteredCatalog.SubCatalogs.length : 0)
                .replace('(s)', filteredCatalog.DataSets ? filteredCatalog.DataSets.length === 1 ? '' : 's' : 's')
                .replace('(s)', filteredCatalog.SubCatalogs ? filteredCatalog.SubCatalogs.length === 1 ? '' : 's' : 's');
            if (filteredCatalog.DataSets.length || filteredCatalog.SubCatalogs.length) {
                results.push(filteredCatalog);
            }
        }
    }

    // performs simple regex matching inside the connection index model
    function regexSearch(text, category) {
        if (!text) {
            return [];
        }
        var terms = text.match(/\w{3,}/g);
        if (!terms) {
            // if the query is non-empty, don't return any results
            if (text.trim().length) {
                return [];
            }
        }
        terms = new RegExp("(" + terms.join("|") + ")", "i");

        var filteredItems = [];
        angular.forEach(model.groups, function (group) {
            // if the desired category is specified and it's not the current group, skip it
            if (category && group.name !== category) {
                return;
            }
            angular.forEach(group.results, function (item) {
                regexSearchItemRecursively(filteredItems, item, terms);
            });
        });
        return filteredItems;
    }

    return {
        getIndex: function () {
            return model;
        },
        addItem: function (item, category) {
            // only add valid items
            if (item) {
                // verify the category name
                var name = category || "Other";
                // find the group that corresponds to the category
                var categoryGroup = null;
                angular.forEach(model.groups, function (group) {
                    // note: case-sensitive comparison of the categories
                    if (group.name === name) {
                        categoryGroup = group;
                    }
                });
                // if the category is new, create a group
                if (!categoryGroup) {
                    categoryGroup = {
                        "name": name,
                        "results": []
                    }
                    model.groups.push(categoryGroup);
                }
                // add the item to the container inside the group
                categoryGroup.results.push(item);
                // save the index state
                this.save();
            }
        },
        removeItem: function (value) {
            if (!value) {
                return;
            }
            angular.forEach(model.groups, function (group) {
                angular.forEach(group.results, function (item) {
                    // the value may be either the object itself or URI or Id
                    // note: apply case-insensitive comparison for URI
                    if (item === value || $pyx.obj.equalsCaseInsensitive(item.Uri, value)) {
                        $pyx.array.removeFirst(group.results, item);
                    }
                });
                // get rid of empty groups
                if (group.results.length === 0) {
                    $pyx.array.removeFirst(model.groups, group);
                }
            });
            // save the index state
            this.save();
        },
        search: function (query, category) {
            if (!query || !("text" in query)) {
                return [];
            }
            // if the query is an empty string, return all results that belong to the specified category
            if (!query.text.length) {
                var results = [];
                angular.forEach(model.groups, function (group) {
                    // if no category specified, include eveything in the results;
                    // if the category is specified, the group should match it
                    if (!category || category && group.name === category) {
                        results.push.apply(results, group.results);
                    }
                });
                return results;
            }
            // otherwise, perform a simple regex matching over query terms
            return regexSearch(query.text, category);
        },
        load: function () {
            $pyx.application.load("connectionIndex", model.groups).success(function (value) {
                $timeout(function () {
                    model.groups = angular.copy(value);
                });
            });
        },
        save: function () {
            $pyx.application.save('connectionIndex', model.groups);
        },
        findItem: function (value) {
            return findItemInIndex(value);
        }
    };
});
