app.service('localSearch', function ($filter, $pyx, $q, searchServices, connectionIndex, networkServiceStateAlert, worldViewStudioConfig) {

    var i18n = $filter('i18n');
    var lastQuery = null;

    var setLocalCatalogs = function (catalogs) {
        angular.forEach(catalogs, function (catalog) {
            // set the resource type
            catalog.DataType = "Local";
            catalog.Type = "Catalog";
            // set the type for each data  set
            angular.forEach(catalog.DataSets, function (dataSet) {
                dataSet.Type = "DataSet";
            });
        });
        return catalogs;
    };
    
    var saveConnectionIndexIfNeeded = function(item) {
        if (connectionIndex.findItem(item)) {
            connectionIndex.save();
        }
    }

    var reloadLocalResource = function (localCatalog) {
        return $pyx.when($pyx.engine.openLocalResource(localCatalog.Uri)).then(function (catalog) {
            // if the catalog doesn't contain data sets or sub-catalogs, consider the request failed
            if (!catalog ||
                ((!catalog.DataSets || !catalog.DataSets.length) && (!catalog.SubCatalogs || !catalog.SubCatalogs.length))) {
                localCatalog.Status = 'Failed';
                return localCatalog;
            }
            localCatalog.Uri = catalog.Uri;
            localCatalog.DataSets = catalog.DataSets;
            localCatalog.SubCatalogs = catalog.SubCatalogs;
            localCatalog.Metadata = catalog.Metadata;
            localCatalog.Status = 'Ready';
            saveConnectionIndexIfNeeded(localCatalog);
            return localCatalog;
        }, function (error) {
            // inform the network verification service about the request failure (the request may depend on the network stack)
            networkServiceStateAlert("localSearch", false, i18n("Failed to complete a local search"));
            localCatalog.Status = 'Failed';
            saveConnectionIndexIfNeeded(localCatalog);
            return error;
        });
    }

    var localSearch = searchServices.register(i18n("My Computer"),
        // if the query represents a file or directory, get the file list
        function (query) {
            localSearch.resultLimit = worldViewStudioConfig.search.defaultResultCount;
            localSearch.results = [];
            lastQuery = query;
            // verify that local search is supported
            if (!("toLocalResource" in $pyx.engine)) {
                // just return the connection index search results
                return connectionIndex.search(query, "Local") || [];
            }
            // verify the path
            var path = $pyx.engine.toLocalResource(query.text);
            if (path) {
                // look in the connection index
                var indexed = connectionIndex.findItem(path);
                if (indexed) {
                    // no need to scan the file system again, just return this particular item
                    return [indexed];
                } else {
                    // Let the UI know about the upcoming data
                    var localCatalog = { 'Uri': path, 'Metadata': { 'Name': path }, 'Status': 'Loading', 'DataSets': [], 'SubCatalogs': [] };
                    localSearch.results = setLocalCatalogs([localCatalog]);
                    // Now start the actual search
                    // Important: just copies the data, doesn't update the result object (to let UI pick up on the updates)
                    return reloadLocalResource(localCatalog).then(function (catalog) {
                        return [catalog];
                    }, function (error) {
                        // note: the search failed, but we are returning the same object
                        // TODO: when we have design for error reporting, work on passing the error message over
                        return [localCatalog];
                    }).then(function (results) {
                        // If we have direct hits, don't search the connection index
                        if (!results.length) {
                            // append the connection index search results without updating the array object
                            results.push.apply(results, connectionIndex.search(query, "Local"));
                        }
                        // TODO: this is a workaround until cancellation of a search request is implemented
                        if (lastQuery !== query) {
                            return localSearch.results;
                        }
                        return results;
                    }, function(results) {
                        // Searching the file system failed, so always search the connection index
                        // (include information about the failed search in the results as well)
                        results.push.apply(results, connectionIndex.search(query, "Local"));
                        // TODO: this is a workaround until cancellation of a search request is implemented
                        if (lastQuery !== query) {
                            return localSearch.results;
                        }
                        return results;
                    });
                }
            } else {
                // the query is not a path; return the connection index search results
                return connectionIndex.search(query, "Local") || [];
            }
        },
        //on-result
        function (results) {
            localSearch.results = results;
        },
        //on-error
        function (results) {
            localSearch.results = results;
        });

    // YK: temporary disable the complete file system scan;
    // if a decision is made to remove it completely, remove the code
    /* if ('scanLocalComputer' in $pyx.engine) {
        $scope.scanLocalComputer = function () {
            $scope.scanningLocalComputer = true;
            $scope.notifyInfo(i18n('Beginning to search your computer'));
            $pyx.engine.scanLocalComputer().success(function (scanResults) {
                $timeout(function () {
                    $scope.search = "";
                    var catalog = setLocalCatalogs(scanResults);
                    $scope.localSearch.results = catalog;
                    $scope.showSearchWindow();
                    $scope.scanningLocalComputer = false;
                    $scope.notifyInfo(i18n('Finished searching your computer'));
                });
            }).error(function (error) {
                $timeout(function () {
                    $scope.scanningLocalComputer = false;
                    $scope.notifyError(i18n('Searching your computer failed with error : ') + error);
                });
            });
        };
    } */

    localSearch.cancel = function() {
        // TODO: implement actual cancellation of the search in progress
        // https://trello.com/c/kfmm7e2o/608-2-auto-cancel-discovery-operation-when-the-search-string-changes
    }

    localSearch.reloadLocalResource = reloadLocalResource;
    return localSearch;
});