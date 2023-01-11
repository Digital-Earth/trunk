app.service('geocodeService', function ($filter, $q, $pyxIntercom, searchServices, networkServiceStateAlert, worldViewStudioConfig) {
    var i18n = $filter('i18n');
    // using Google Maps client-side geocoding service: https://developers.google.com/maps/documentation/javascript/geocoding 
    var geocoder = new google.maps.Geocoder();

    var mapAddressToFeature = function (address) {

        var northEast = address.geometry.viewport.getNorthEast();
        var southWest = address.geometry.viewport.getSouthWest();
        var viewport = {
            type: "LineString",
            coordinates: [
                [southWest.lng(), southWest.lat()], // bottom-left
                [northEast.lng(), northEast.lat()] // top-right
            ]
        };
        var point = {
            type: "Point",
            coordinates: [address.geometry.location.lng(), address.geometry.location.lat()]
        };
        return {
            Type: "Feature",
            Metadata: {
                Name: address.address_components[0].long_name,
                Description: address.formatted_address
            },
            Feature: {
                // GeoJson feature
                type: "Feature",
                id: 1,
                geometry: point,
                properties: {
                    name: address.address_components[0].long_name,
                    description: address.formatted_address,
                    addressTypes: address.types.join(", "),
                    googlePlaceId: address.place_id
                }
            },
            Viewport: viewport
        };
    };

    var geocodeService = searchServices.register(i18n("Features"),
        // if the query represents an address that can be geocoded, list the geocoded features
        function (query) {
            geocodeService.resultLimit = worldViewStudioConfig.search.defaultResultCount;
            geocodeService.results = [];
            var deferredResults = $q.defer();
            geocoder.geocode({ 'address': query.text }, function(rawResults, status) {
                var results = [];
                if (status === google.maps.GeocoderStatus.OK && rawResults.length) {
                    results = [
                        {
                            Type: "FeatureCollection",
                            Metadata: {
                                Name: i18n("Addresses")
                            },
                            features: rawResults.map(mapAddressToFeature)
                        }
                    ];
                }
                deferredResults.resolve(results);
            });

            return deferredResults.promise;
        },
        //on-result
        function (results) {
            var metadata;
            if (results[0] && results[0].features[0].Metadata) {
                metadata = {
                    location: results[0].features[0].Metadata.Description
                };
            }
            
            $pyxIntercom.track('search-for-address', metadata || {});
            geocodeService.results = results;
        },
        //on-error
        function (query) {
            // inform the network verification service about the request failure
            networkServiceStateAlert("geocodeService", false, i18n("Failed to connect to Addresses service"));
            geocodeService.results = [];
        });

    return geocodeService;
});