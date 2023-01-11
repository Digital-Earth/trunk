
/*
	a SHIM / Interface to get the WorldView Studio running in a local context
 */
app.service('studioShim', function($q, $pyxuser, globalCookieStore, $http, $window, dispatcher) {
  var PyxisApplicationInterface, PyxisEngineInterface, PyxisGlobeInterface, Url, cameraFromCircle, elevationGeoSource, geoWebCoreUrl, localStorageLayer, promiseShim, refreshGlobe;
  console.log("SHIM LOADED ");
  geoWebCoreUrl = window.gwcHost;
  localStorageLayer = {
    get: function(id, defaultValue) {
      if (localStorage.getItem(id)) {
        return JSON.parse(localStorage.getItem(id));
      } else {
        return defaultValue;
      }
    },
    set: function(id, value) {
      return localStorage[id] = JSON.stringify(value);
    }
  };
  promiseShim = function() {
    var dfd, promise;
    dfd = $q.defer();
    promise = dfd.promise;
    promise.success = function(fn) {
      promise.then(function(data) {
        return fn(data);
      });
      return promise;
    };
    promise.error = function(fn) {
      promise.then(null, function(error) {
        return fn(error);
      });
      return promise;
    };
    return dfd;
  };
  Url = (function() {
    function Url(url1) {
      var queryStart;
      this.url = url1;
      this.query = "";
      queryStart = this.url.indexOf('?');
      if (queryStart !== -1) {
        this.query = this.url.substring(queryStart + 1);
        this.url.splice(queryStart, this.url.length - queryStart);
      }
      if (this.url.lastIndexOf('/') === this.url.length - 1) {
        this.url.slice(0, -1);
      }
      if (this.query.lastIndexOf('&') === this.query.length - 1) {
        this.query.slice(0, -1);
      }
    }

    Url.prototype.dir = function(value) {
      this.url += '/' + encodeURIComponent(value);
      return this;
    };

    Url.prototype.search = function(params) {
      var key, value;
      for (key in params) {
        value = params[key];
        this.query += "&" + encodeURIComponent(key) + "=" + encodeURIComponent(value);
      }
      return this;
    };

    Url.prototype.toString = function() {
      if (this.query.indexOf('&') === 0) {
        this.query = this.query.substring(1);
      }
      if (this.query) {
        return this.url + "?" + this.query;
      }
      return this.url;
    };

    return Url;

  })();

  /*
  		$pyx.application
   */
  PyxisApplicationInterface = (function() {
    function PyxisApplicationInterface() {}

    PyxisApplicationInterface.prototype.bringToFront = function() {
      return console.log("INTERFACE bringToFront");
    };

    PyxisApplicationInterface.prototype.load = function(name, defaultValue) {
      var dfd;
      console.log("INTERFACE load", name, defaultValue);
      dfd = promiseShim();
      dfd.resolve(localStorageLayer.get(name, defaultValue));
      return dfd.promise;
    };

    PyxisApplicationInterface.prototype.save = function(name, value) {
      return localStorageLayer.set(name, value);
    };

    PyxisApplicationInterface.prototype.getToken = function() {
      return globalCookieStore.get('pyx.user.token');
    };

    PyxisApplicationInterface.prototype.login = function(username, password) {
      return $pyxuser.login(username, password);
    };

    PyxisApplicationInterface.prototype.loginWithToken = function(token) {
      return $pyxuser.loginWithToken(token);
    };

    PyxisApplicationInterface.prototype.dragDropSupport = function() {
      return console.log("INTERFACE dragDropSupport");
    };

    PyxisApplicationInterface.prototype.getVersion = function() {
      return '1.0.0.1230';
    };

    return PyxisApplicationInterface;

  })();
  elevationGeoSource = null;
  refreshGlobe = function(layers) {
    var layersOrdering, sortedLayers;
    layersOrdering = function(layer) {
      if (layer.style.ShowAsElevation) {
        elevationGeoSource = layer.geoSource.Id;
        return 0;
      }
      if (layer.geoSource.Specification && layer.geoSource.Specification.OutputType === "Coverage") {
        if (!layer.style.Fill) {
          return 1;
        }
        return 2;
      }
      return 3;
    };
    sortedLayers = _.values(layers);
    sortedLayers = _.sortBy(sortedLayers, layersOrdering);
    return window.GC.preloadLayers(sortedLayers, void 0, function() {
      return window.GC.doSwapScene();
    });
  };
  cameraFromCircle = function(boundingCircle, fov) {
    var radius;
    radius = Math.min(boundingCircle.radius, wgs84.earthRadius);
    return {
      latitude: boundingCircle.coordinates[1],
      longitude: boundingCircle.coordinates[0],
      altitude: 0,
      range: radius / Math.sin(fov / 2 * Math.PI / 180.0),
      heading: 0,
      tilt: 0
    };
  };

  /*
  		Step 1 of integration is to setup the GlobeCanvas module from within the interface
  
  		Core functionality exposed from :  WorldView.Studio / ApplicationForm.cs :  RegisterGlobeJsApi()
   */
  PyxisGlobeInterface = (function() {
    function PyxisGlobeInterface() {
      console.log("GLOBE INTERFACE constructor");
      this.visibleId = {};
      this.styles = {};
      this.layers = {};
      this.lastId = 0;
      this.createGlobeWidget();
    }

    PyxisGlobeInterface.prototype._newID = function() {
      this.lastId++;
      return "id-" + this.lastId;
    };

    PyxisGlobeInterface.prototype.createGlobeWidget = function() {
      var elementId, randomId, template;
      randomId = Math.round(Math.random() * 9999);
      elementId = "globetest-" + randomId;
      template = "<div id=\"" + elementId + "\" geo-source-id=\"geoSourceId\" style=\"left: 0; top: 0px; position: absolute;\"></div>";
      $('body').prepend(template);
      this.globe = new $window.PYXIS.GlobeCanvas($("#" + elementId)[0], elementId);
      return $window.GC = this.globe;
    };

    PyxisGlobeInterface.prototype.renderReport = function() {
      console.log("INTERFACE renderReport");
      return {};
    };

    PyxisGlobeInterface.prototype.cursor = function() {
      console.log("INTERFACE cursor");
      return '';
    };

    PyxisGlobeInterface.prototype.pickArea = function(pixelWidth) {
      console.log("INTERFACE pickArea");
      return null;
    };

    PyxisGlobeInterface.prototype.show = function(geoSource) {
      var apiRequest, dfd, id;
      console.log("INTERFACE show ", geoSource);
      id = this._newID();
      dfd = promiseShim();
      apiRequest = $http.post(geoWebCoreUrl + "/api/v1/Local/Style", {
        geoSource: geoSource
      });
      apiRequest.success((function(_this) {
        return function(style) {
          _this.layers[id] = {
            geoSource: geoSource,
            style: style
          };
          refreshGlobe(_this.layers);
          return dfd.resolve(id);
        };
      })(this));
      apiRequest.error((function(_this) {
        return function(error) {
          return dfd.reject(error);
        };
      })(this));
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.showWithStyle = function(geoSource, style) {
      var dfd, id;
      console.log("INTERFACE showWithStyle ", geoSource);
      id = this._newID();
      this.layers[id] = {
        geoSource: geoSource,
        style: style
      };
      refreshGlobe(this.layers);
      dfd = promiseShim();
      dfd.resolve(id);
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.createDefaultStyle = function(geoSource, style) {
      var apiRequest, dfd;
      console.log("INTERFACE createDefaultStyle");
      dfd = promiseShim();
      apiRequest = $http.post(geoWebCoreUrl + "/api/v1/Local/Style", {
        geoSource: geoSource
      });
      apiRequest.success((function(_this) {
        return function(style) {
          return dfd.resolve(style);
        };
      })(this));
      apiRequest.error((function(_this) {
        return function(error) {
          return dfd.reject(error);
        };
      })(this));
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.getVisibleId = function(geoSource) {
      console.log("INTERFACE getVisibleId");
      return null;
    };

    PyxisGlobeInterface.prototype.hide = function(id) {
      var dfd;
      console.log("INTERFACE hide");
      dfd = promiseShim();
      delete this.layers[id];
      refreshGlobe(this.layers);
      dfd.resolve(id);
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.getStyle = function(id) {
      var dfd;
      console.log("INTERFACE getStyle");
      dfd = promiseShim();
      dfd.resolve(this.layers[id].style);
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.setStyle = function(id, style) {
      var dfd;
      console.log("INTERFACE setStyle", id, style);
      this.layers[id].style = style;
      refreshGlobe(this.layers);
      dfd = promiseShim();
      dfd.resolve(id);
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.isVisibleIdLoading = function(id) {
      console.log("INTERFACE isVisibleIdLoading");
      if (!this.layers[id]) {
        return false;
      }
      return this.globe.isGeoSourceLoading(this.layers[id].geoSource.Id);
    };

    PyxisGlobeInterface.prototype.getAllLoadingVisibleIds = function() {
      var dfd, key, ref, result, value;
      result = [];
      ref = this.layers;
      for (key in ref) {
        value = ref[key];
        if (this.globe.isGeoSourceLoading(value.geoSource.Id)) {
          result.push(key);
        }
      }
      dfd = promiseShim();
      dfd.resolve(result);
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.setStyleByFieldWithPalette = function(id, fieldName, palette) {
      var apiRequest, dfd, layer;
      console.log("INTERFACE setStyleByFieldWithPalette", id, fieldName, palette);
      layer = this.layers[id];
      dfd = promiseShim();
      apiRequest = $http.post(geoWebCoreUrl + "/api/v1/Local/AutoStyle", {
        geoSource: layer.geoSource,
        style: layer.style,
        field: fieldName,
        palette: palette
      });
      apiRequest.success((function(_this) {
        return function(style) {
          _this.layers[id].style = style;
          dfd.resolve(style);
          return refreshGlobe(_this.layers);
        };
      })(this));
      apiRequest.error((function(_this) {
        return function(error) {
          return dfd.reject(error);
        };
      })(this));
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.setStyleByFieldWithPaletteForScreen = function(id, fieldName, palette) {
      console.log("INTERFACE setStyleByFieldWithPaletteForScreen", id, fieldName, palette);
      return setStyleByFieldWithPalette(id, fieldName, palette);
    };

    PyxisGlobeInterface.prototype.getLayers = function() {
      var dfd, key;
      console.log("INTERFACE getLayers");
      dfd = promiseShim();
      dfd.resolve((function() {
        var results;
        results = [];
        for (key in this.layers) {
          results.push(key);
        }
        return results;
      }).call(this));
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.getGeoSources = function() {
      var dfd, key, value;
      console.log("INTERFACE getGeoSources");
      dfd = promiseShim();
      dfd.resolve((function() {
        var ref, results;
        ref = this.layers;
        results = [];
        for (key in ref) {
          value = ref[key];
          results.push(value.geoSource);
        }
        return results;
      }).call(this));
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.gotoGeometry = function(geometry, duration) {
      var apiRequest, dfd, url;
      console.log("INTERFACE gotoGeometry", geometry, duration);
      dfd = promiseShim();
      url = new Url(geoWebCoreUrl + "/api/v1/Local/BoundingCircle");
      apiRequest = $http.post(url.toString(), geometry);
      apiRequest.success((function(_this) {
        return function(boundingCircle) {
          var camera;
          console.log("INTERFACE gotoGeometry success", geometry, boundingCircle);
          camera = cameraFromCircle(boundingCircle, _this.globe.getCamera().fov);
          _this.setCamera(camera, duration);
          return dfd.resolve(camera);
        };
      })(this));
      apiRequest.error(function(error) {
        console.log("INTERFACE gotoGeometry error", geometry, error);
        return dfd.reject(error);
      });
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.gotoGeoSource = function(geoSource, duration) {
      var apiRequest, dfd, url;
      console.log("INTERFACE gotoGeoSource", geoSource, duration);
      dfd = promiseShim();
      url = new Url(geoWebCoreUrl + "/api/v1/GeoSource");
      url.dir(geoSource.Id).dir('BoundingCircle');
      apiRequest = $http.get(url.toString());
      apiRequest.success((function(_this) {
        return function(boundingCircle) {
          var camera;
          console.log("INTERFACE gotoGeoSource success", geoSource, boundingCircle);
          camera = cameraFromCircle(boundingCircle, _this.globe.getCamera().fov);
          _this.setCamera(camera, duration);
          return dfd.resolve(camera);
        };
      })(this));
      apiRequest.error(function(error) {
        console.log("INTERFACE gotoGeoSource error", geoSource, error);
        return dfd.reject(error);
      });
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.getCamera = function() {
      var camera;
      camera = new GlobeCamera(this.globe.getCamera());
      return camera.asPyxisCamera();
    };

    PyxisGlobeInterface.prototype.setCamera = function(camera, duration) {
      var currentCamera, distanceInMeters, endRange, globe, globeCamera, middleRange, startRange, target, targetCamera;
      globe = this.globe;
      globeCamera = globe.getCamera();
      currentCamera = new GlobeCamera(globeCamera);
      targetCamera = new GlobeCamera(camera);
      if (targetCamera.heading - currentCamera.heading > 180) {
        targetCamera.heading -= 360;
      }
      if (targetCamera.heading - currentCamera.heading < -180) {
        targetCamera.heading += 360;
      }
      if (targetCamera.longitude - currentCamera.longitude > 180) {
        targetCamera.longitude -= 360;
      }
      if (targetCamera.longitude - currentCamera.longitude < -180) {
        targetCamera.longitude += 360;
      }
      target = wgs84.latLonToXyz({
        lat: targetCamera.latitude,
        lon: targetCamera.longitude
      });
      distanceInMeters = target.angleTo(globeCamera.position) * wgs84.earthRadius;
      if (distanceInMeters / Math.max(targetCamera.range, currentCamera.range) > 2) {
        currentCamera.rangeAnim = 0;
        startRange = currentCamera.range;
        middleRange = distanceInMeters / 2;
        endRange = targetCamera.range;
        TweenMax.to(currentCamera, duration / 1000.0, {
          rangeAnim: 1,
          latitude: targetCamera.latitude,
          longitude: targetCamera.longitude,
          altitude: targetCamera.altitude,
          heading: targetCamera.heading,
          range: targetCamera.range,
          tilt: targetCamera.tilt,
          ease: Cubic.easeInOut,
          onUpdate: function() {
            var d;
            if (currentCamera.rangeAnim < 0.5) {
              d = Math.sin(Math.PI * currentCamera.rangeAnim);
              currentCamera.range = startRange * (1 - d) + middleRange * d;
            } else {
              d = Math.sin(Math.PI * currentCamera.rangeAnim);
              currentCamera.range = endRange * (1 - d) + middleRange * d;
            }
            return currentCamera.apply(globe.getCamera());
          }
        });
      } else {
        TweenMax.to(currentCamera, duration / 1000.0, {
          latitude: targetCamera.latitude,
          longitude: targetCamera.longitude,
          altitude: targetCamera.altitude,
          heading: targetCamera.heading,
          range: targetCamera.range,
          tilt: targetCamera.tilt,
          ease: Cubic.easeInOut,
          onUpdate: function() {
            return currentCamera.apply(globe.getCamera());
          }
        });
      }
      return currentCamera;
    };

    PyxisGlobeInterface.prototype.screenToGeographicPosition = function(locations) {
      var dfd, offset, wgs84Points;
      console.log("INTERFACE screenToGeographicPosition", locations);
      wgs84Points = [];
      offset = $(this.globe.renderer.domElement).offset();
      _.each(locations, (function(_this) {
        return function(location) {
          var latlon, point3D;
          point3D = _this.globe.raycast(location[0] - offset.left, location[1] - offset.top);
          if (point3D) {
            latlon = wgs84.xyzToLatLon(point3D);
            return wgs84Points.push([latlon.lon, latlon.lat]);
          } else {
            return wgs84Points.push(null);
          }
        };
      })(this));
      dfd = promiseShim();
      dfd.resolve(wgs84Points);
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.capture = function(screenCaptureArgs) {
      console.log("INTERFACE capture", screenCaptureArgs);
      return 'http://image-url';
    };

    PyxisGlobeInterface.prototype.click = function(callback) {
      var element, self;
      console.log("INTERFACE click", callback);
      element = $(this.globe.renderer.domElement);
      self = this;
      return element.click(function(event) {
        var circleGeometry, simplifedEvent;
        console.log("EVENT CLICK ", event);
        simplifedEvent = {
          clientX: event.clientX,
          clientY: event.clientY
        };
        circleGeometry = self.globe.getMouseGeometryOnGlobe();
        if (circleGeometry) {
            return callback(circleGeometry, JSON.stringify(simplifedEvent));
        }
      });
    };

    PyxisGlobeInterface.prototype.rightClick = function(callback) {
      var element, self;
      console.log("INTERFACE rightClick", callback);
      element = $(this.globe.renderer.domElement);
      self = this;
      return element.bind("contextmenu", function(event) {
        var circleGeometry, simplifedEvent;
        console.log("EVENT CONTEXT ", event);
        simplifedEvent = {
          clientX: event.clientX,
          clientY: event.clientY
        };
        circleGeometry = self.globe.getMouseGeometryOnGlobe();
        if (circleGeometry) {
          event.preventDefault();
          callback(circleGeometry, JSON.stringify(simplifedEvent));
          return false;
        }
      });
    };

    return PyxisGlobeInterface;

  })();

  /*
  		$pyx.engine Engine Interface
  		Core functionality exposed from :  WorldView.Studio / ApplicationForm.cs :  RegisterEngineJsApi()
   */
  PyxisEngineInterface = (function() {
    function PyxisEngineInterface() {}

    PyxisEngineInterface.prototype.getDefinition = function(geoSource) {
      var apiRequest, dfd;
      dfd = promiseShim();
      if (geoSource.Sepcification) {
        dfd.resolve(geoSource.Sepcification);
      } else {
        apiRequest = $http.post(geoWebCoreUrl + "/api/v1/Local/Specification", geoSource);
        apiRequest.success(function(spec) {
          console.log("INTERFACE getDefinition resolved", geoSource, spec);
          return dfd.resolve(spec);
        });
        apiRequest.error(function(error) {
          console.log("INTERFACE getDefinition failed", geoSource, error);
          return dfd.reject(error);
        });
      }
      return dfd.promise;
    };

    PyxisEngineInterface.prototype.getFeatures = function(geoSource, geometry) {
      var apiRequest, dfd, url;
      console.log("INTERFACE getFeatures", geoSource, geometry);
      dfd = promiseShim();
      url = new Url(geoWebCoreUrl + "/api/v1/GeoSource/");
      url.dir(geoSource.Id).dir('Data').search({
        '$intersects': JSON.stringify(geometry)
      });
      apiRequest = $http.get(url.toString());
      apiRequest.success(function(data) {
        console.log("INTERFACE getFeatures success", geoSource, geometry, data);
        return dfd.resolve(data);
      });
      apiRequest.error(function(error) {
        console.log("INTERFACE getFeatures error", geoSource, geometry, error);
        return dfd.reject(error);
      });
      return dfd.promise;
    };

    PyxisEngineInterface.prototype.getValue = function() {
      return console.log("INTERFACE getValue");
    };

    PyxisEngineInterface.prototype.createFeatureCollection = function(featureCollection) {
      var apiRequest, dfd;
      console.log("INTERFACE createFeatureCollection");
      dfd = promiseShim();
      apiRequest = $http.post(geoWebCoreUrl + "/api/v1/Local/Import/GeoJson", featureCollection);
      apiRequest.success(function(geoSource) {
        console.log("INTERFACE createFeatureCollection success", featureCollection, geoSource);
        return dfd.resolve(geoSource);
      });
      apiRequest.error(function(error) {
        console.log("INTERFACE createFeatureCollection error", featureCollection, error);
        return dfd.reject(error);
      });
      return dfd.promise;
    };

    PyxisEngineInterface.prototype.createWatershedGeometry = function(location) {
      var apiRequest, dfd, url;
      console.log("INTERFACE createWatershedGeometry");
      dfd = promiseShim();
      url = new Url(geoWebCoreUrl + "/api/v1/Local/Watershed").search({
        'geoSource': elevationGeoSource,
        'location': JSON.stringify(location)
      });
      apiRequest = $http.get(url.toString());
      apiRequest.success(function(watershed) {
        console.log("INTERFACE createWatershedGeometry success", elevationGeoSource, location, watershed);
        return dfd.resolve(watershed);
      });
      apiRequest.error(function(error) {
        console.log("INTERFACE createWatershedGeometry error", elevationGeoSource, location, error);
        return dfd.reject(error);
      });
      return dfd.promise;
    };

    PyxisEngineInterface.prototype.whereIntersection = function() {
      return console.log("INTERFACE whereIntersection");
    };

    PyxisEngineInterface.prototype.getAllFeatures = function(geoSource) {
      return console.log("INTERFACE getAllFeatures");
    };

    PyxisEngineInterface.prototype.getFieldStatistics = function(geoSource, fieldName, binCount) {
      var apiRequest, dfd, url;
      console.log("INTERFACE getFieldStatistics");
      dfd = promiseShim();
      url = new Url(geoWebCoreUrl + "/api/v1/GeoSource/");
      url.dir(geoSource.Id).dir('Stats').search({
        '$select': fieldName,
        'bins': binCount
      });
      apiRequest = $http.get(url.toString());
      apiRequest.success(function(stats) {
        console.log("INTERFACE getFieldStatistics success", geoSource, fieldName, stats);
        return dfd.resolve(stats);
      });
      apiRequest.error(function(error) {
        console.log("INTERFACE getFieldStatistics error", geoSource, fieldName, error);
        return dfd.reject(error);
      });
      return dfd.promise;
    };

    PyxisEngineInterface.prototype.getFieldStatisticsAt = function(geoSource, fieldName, geometry, binCount) {
      var apiRequest, dfd, url;
      console.log("INTERFACE getFieldStatisticsAt");
      dfd = promiseShim();
      url = new Url(geoWebCoreUrl + "/api/v1/GeoSource");
      url.dir(geoSource.Id).dir('Stats').search({
        '$select': fieldName,
        'bins': binCount,
        '$intersects': JSON.stringify(geometry)
      });
      apiRequest = $http.get(url.toString());
      apiRequest.success(function(stats) {
        console.log("INTERFACE getFieldStatisticsAt success", geoSource, fieldName, geometry, stats);
        return dfd.resolve(stats);
      });
      apiRequest.error(function(error) {
        console.log("INTERFACE getFieldStatisticsAt error", geoSource, fieldName, geometry, error);
        return dfd.reject(error);
      });
      return dfd.promise;
    };

    PyxisEngineInterface.prototype.getFieldValueCount = function(geoSource, fieldName, fieldValue) {
      return console.log("INTERFACE getFieldValueCount");
    };

    PyxisEngineInterface.prototype.getFieldValueCountAt = function(geoSource, fieldName, fieldValue, geometry) {
      return console.log("INTERFACE getFieldValueCountAt");
    };

    PyxisEngineInterface.prototype.searchQuery = function(query) {
      return console.log("INTERFACE searchQuery", query);
    };

    PyxisEngineInterface.prototype.getArea = function(geometry) {
      var apiRequest, dfd;
      console.log("INTERFACE getArea", geometry);
      dfd = promiseShim();
      apiRequest = $http.post(geoWebCoreUrl + "/api/v1/Local/Area", geometry);
      apiRequest.success(function(area) {
        console.log("INTERFACE getArea success", geometry, area);
        return dfd.resolve(area);
      });
      apiRequest.error(function(error) {
        console.log("INTERFACE getArea error", geometry, error);
        return dfd.reject(error);
      });
      return dfd.promise;
    };

    PyxisEngineInterface.prototype.getDataSize = function(geoSource) {
      return console.log("INTERFACE getDataSize", geoSource);
    };

    PyxisEngineInterface.prototype.setGeoTagReferenceGeoSources = function(references) {
      return console.log("INTERFACE setGeoTagReferenceGeoSources", references);
    };

    PyxisEngineInterface.prototype.provideImportSetting = function(url, settingType, value) {
      return console.log("INTERFACE provideImportSetting", url, settingType, value);
    };

    PyxisEngineInterface.prototype["import"] = function(url) {
      return console.log("INTERFACE import");
    };

    PyxisEngineInterface.prototype.supportedImportFileFormats = function() {
      var dfd, types;
      types = ["adf", "asc", "csv", "ddf", "dem", "dt0", "dt1", "dt2", "e00", "g98", "gif", "gml", "grb", "grd", "img", "jpg", "kml", "ntf", "png", "ppl", "shp", "sid", "tab", "tif", "tiff", "tl2", "toc", "vrt"];
      dfd = promiseShim();
      dfd.resolve(types);
      return dfd.promise;
    };

    PyxisEngineInterface.prototype.upgradeMap = function(mapResource) {
      var dfd;
      console.log("INTERFACE upgradeMap");
      dfd = promiseShim();
      dfd.resolve(mapResource);
      return dfd.promise;
    };

    PyxisEngineInterface.prototype.verifyMapSchema = function(map) {
      console.log("INTERFACE verifyMapSchema", map);
      return angular.copy(map);
    };

    PyxisEngineInterface.prototype.publishLocally = function(geoSource) {
      return console.log("INTERFACE publishLocally");
    };

    return PyxisEngineInterface;

  })();

  /*
  		Create our JSProxyFactory shim - the window.PYX object is what loadservice uses to pull in a service by name
   */
  console.log("APPPLY WINDOW PYX");
  $window.PYX = {
    get: function(name, callback) {
      console.log("NEW GET ", name);
      if (name === 'application') {
        return callback(new PyxisApplicationInterface);
      }
      if (name === 'globe') {
        return callback(new PyxisGlobeInterface);
      }
      if (name === 'engine') {
        return callback(new PyxisEngineInterface);
      }
      return callback(null);
    },
    has: function(name) {
      return name === 'application' || name === 'globe' || name === 'engine';
    }
  };
  return {};
});
