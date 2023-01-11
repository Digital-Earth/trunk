app.service('onlineGeospatialService', function ($filter, $pyx, $q, $pyxIntercom, searchServices, connectionIndex, networkServiceStateAlert, worldViewStudioConfig) {
    var i18n = $filter('i18n');

    var constructServerDataSets = function(dataSets) {   
        return dataSets.map(function(dataSet) {
            return {
                "Type": "DataSet",
                "Metadata": {
                    "Name": dataSet.Name,
                    "Description": dataSet.Description
                },
                "Services": dataSet.Services,
                "SelectedService": 0,
                get Uri() {
                    var metadata = {
                        name: dataSet.Name,
                        description: dataSet.Description
                    };

                    return dataSet.Services[this.SelectedService].ImportUri;
                }
            }
        });
    }

    var constructCatalogFromDataSets = function (dataSets, dataType) {
        if (dataType === "OGC") {
            return {
                "DataType": "OGC",
                "Type": "Catalog",
                "Metadata": {
                    "Name": i18n("Online OGC Catalog"),
                    "Description": i18n("OGC data found crawling the web"),
                    "Tags": ["OGC"]
                },
                "DataSets": constructServerDataSets(dataSets)
            }
        } else if (dataType === "ArcGIS") {
            return {
                "DataType": "ArcGIS",
                "Type": "Catalog",
                "Metadata": {
                    "Name": i18n("Online ArcGIS Catalog"),
                    "Description": i18n("ArcGIS data found crawling the web"),
                    "Tags": ["ArcGIS", "GeoServices"]
                },
                "DataSets": constructServerDataSets(dataSets)
            }
        } else {
            return null;
        }
    }

    var saveConnectionIndexIfNeeded = function (item) {
        if (connectionIndex.findItem(item)) {
            connectionIndex.save();
        }
    }

    var reloadServer = function(server) {
        return $pyx.when($pyx.engine.openOgcServer(server.Uri)).then(function (catalog) {
            // if the catalog doesn't contain data sets, consider the request failed (OGC catalogs can't have sub-catalogs in the current implementation)
            if (!catalog || !catalog.DataSets || !catalog.DataSets.length) {
                // the Pyxis engine may return an empty catalog when failed to connect to the server
                networkServiceStateAlert('onlineGeospatialService', false, i18n('Failed to connect to the server %s', server.Uri));
                server.Status = 'Failed';
                saveConnectionIndexIfNeeded(server);
                return server;
            }

            server.Uri = catalog.Uri;
            server.DataSets = catalog.DataSets;
            server.SubCatalogs = catalog.SubCatalogs;
            server.Metadata = catalog.Metadata;
            server.Status = 'Ready';
            saveConnectionIndexIfNeeded(server);
            return server;
        }, function (error) {
            // failed to connect to the server
            networkServiceStateAlert('onlineGeospatialService', false, i18n('Failed to connect to the server %s', server.Uri));
            server.Status = 'Failed';
            saveConnectionIndexIfNeeded(server);
            return error;
        });
    }

    var onlineGeospatialService = searchServices.register(i18n("Online Services"),
        // if the query represents an ArcGIS or OGC server, open it
        function(query) {
            onlineGeospatialService.resultLimit = worldViewStudioConfig.search.defaultResultCount;
            onlineGeospatialService.results = [];

            if (!query.text) {
                return [];
            }

            var dataType = null;
            if (('isOgcUrl' in $pyx.engine) && $pyx.engine.isOgcUrl(query.text)) {
                dataType = "OGC";
            } else if (('isGeoServiceUrl' in $pyx.engine) && $pyx.engine.isGeoServiceUrl(query.text)) {
                dataType = "ArcGIS";
            }

            // try to connect to the server and return information about it and its data sets
            if (dataType !== null) {
                // let the UI know about the upcoming data
                var server = { 'Uri': query.text, 'Metadata': { 'Name': query.text }, 'DataType': dataType, 'Type': 'Catalog', 'Status': 'Loading', 'DataSets': [], 'SubCatalogs': [] };
                onlineGeospatialService.results = [server];
                // Important: just copies the data, doesn't update the result object (to let UI pick up on the updates)
                return reloadServer(server).then(function (server) {
                    return [server];
                }, function(error) {
                    // loading the server failed, but return the object anyway
                    // TODO: when we have a design for returning errors to the user, the error here may be passed over
                    return [server];
                }).then(function (results) {
                    // no need to search the connection index as we have results from the server
                    return results;
                }, function (results) {
                    // communication with the server failed, search the connection index
                    // (append the connection index search results without updating the array object)
                    results.push.apply(results, connectionIndex.search(query, dataType));
                });
            } else {
                // if it's not an web data URL, perform a search on the Online OGC Catalog
                return $pyx.gallery.searchExternal(query.text, $pyx.globe.getCamera()).then(function (dataSets) {
                    dataSets = dataSets.data;
                    if (!dataSets.length) {
                        return [];
                    }
                    return [constructCatalogFromDataSets(dataSets, "OGC")];
                }, function (error) {
                    // alert the request failure
                    networkServiceStateAlert('onlineOGCSearch', false, i18n('Online OGC search failed'));
                    return error;
                }).then(function (results) {
                    // Online OGC Catalog search succeeded;
                    // apppend connection index search results
                    $pyxIntercom.track('search-ogc-catalog');
                    results.push.apply(results, connectionIndex.search(query, "OGC"));
                    return results;
                }, function (error) {
                    // failed to search Online OGC Catalog
                    return connectionIndex.search(query, "OGC") || [];
                });
            }
        },
        //on-result
        function (results) {
            onlineGeospatialService.results = results;
        },
        //on-error
        function (results) {
            // search the connection index
            onlineGeospatialService.results = results;
        });

    onlineGeospatialService.reloadServer = reloadServer;
    return onlineGeospatialService;
});