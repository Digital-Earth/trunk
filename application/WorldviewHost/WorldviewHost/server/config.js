// a place to put any defaults and hard-coded values
// using this instead of JSON because it supports comments
var _ = require('underscore');
var pkginfo = require('pkginfo')(module, 'service');

var config = {
    defaultPort: 52023,
    backends: {
        default: "https://api.pyxis.worldview.gallery/api/v1",
        live: "https://api.pyxis.worldview.gallery/api/v1",
        production: "https://api.pyxis.worldview.gallery/api/v1",
        test: "https://api.test.pyxis.worldview.gallery/api/v1",
        dev: "https://api.dev.pyxis.worldview.gallery/api/v1"
    },

    // if the map doesn't have an image associated with it use this
    defaultMapImage: 'https://worldview.gallery/Contents/Images/welcome-banner-3.png',

    // default environment variables
    env: {
        AZURE_STORAGE_CONNECTION_STRING: 'DefaultEndpointsProtocol=https;AccountName=pyxisdiagnosis;AccountKey=KsHNqWv/MOlkqVe1Y4vDXtgYaLhVRok7Uhf1Ic3LNq1hlXhGWpq/Ap549QDOUZAcfWP59tynqCC8Ey659bWi5Q=='
    }

};

// use these backends for non-production sites unless backend query parameter is provided
config.nonProductionHostBackends = {
    'localhost': config.backends.dev,
    'dev.worldview.gallery': config.backends.dev,
    'test.worldview.gallery': config.backends.test
}

_.extend(module.exports, config);