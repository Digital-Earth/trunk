
/*
  a SHIM / Interface to get the WorldView Studio running in a local context
 */
app.service('studioShim', function($rootScope, $q, $pyxuser, globalCookieStore, $http, $window, $timeout, dispatcher) {

  var PyxisApplicationInterface, PyxisEngineInterface, PyxisGlobeInterface, Url, cameraFromCircle, elevationGeoSource, geoWebCoreUrl, localStorageLayer, promiseShim;
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

  try {
    var electron = require('electron');
  } catch(e) {
    electron = undefined;
  }

  var isLocalGeoWebCoreInitializingGeoSource = function(id) { return false; }
  
  if (geoWebCoreUrl.isLocal && geoWebCoreUrl.isLocal()) {
    var localGeoWebCoreState = {
        geoSources: {}
    }
    dispatcher.registerStore("localGeoWebCoreStore", {
        handle: {
          resourceResolvedCompleted: function(action) {
            if (action.data.local) {
                localGeoWebCoreState.geoSources[action.data.resource.Id] = "initializing";
                var url = new Url(geoWebCoreUrl.post() + "/api/v1/Local/Import/GeoSource");
                //notify local geoWebCore about new resource
                $http.post(url.toString(), action.data.resource ).success(function() {
                    localGeoWebCoreState.geoSources[action.data.resource.Id] = "ok";
                }).error(function() {
                    localGeoWebCoreState.geoSources[action.data.resource.Id] = "failed";
                })
            }
          }
        },
        getGeoSourceState: function(id) {
          return localGeoWebCoreState.geoSources[id];
        }
    });
    
    isLocalGeoWebCoreInitializingGeoSource = function(id) { return dispatcher.stores.localGeoWebCoreStore.getGeoSourceState(id) == "initializing"; }
  }
  
  /*
      $pyx.application
   */
  PyxisApplicationInterface = (function() {
    function PyxisApplicationInterface() {}

    if (electron) {
        var dragDropCallbacks = {}
        var dragDropSupport = {
            binded: false,
            bind: function() {
                if (dragDropSupport.binded) {
                    return;
                }
                $('body').bind('dragover', function(e) {
                    e.stopPropagation();
                    e.preventDefault();
                    dragDropSupport.emit('dragEnter');
                });
                $('body').bind('dragleave', function(e) {
                    e.stopPropagation();
                    e.preventDefault();
                    dragDropSupport.emit('dragLeave');
                });
                $('body').bind('drop',  function(e) {
                    e.stopPropagation();
                    e.preventDefault();
                    var files = e.originalEvent.dataTransfer.files;
                    
                    dragDropSupport.emit('drop',files);
                });
                dragDropSupport.binded = true;
            },
            on: function(event,callback) {
                dragDropSupport.bind();
                dragDropCallbacks[event] = callback;
            },
            emit: function(event,info){
                if (dragDropCallbacks[event]) {
                    dragDropCallbacks[event](info);
                }
            }
        }
        
        dragDropSupport.bind();
    }

    PyxisApplicationInterface.prototype.bringToFront = function() {
      return console.log("INTERFACE bringToFront");
    };

    PyxisApplicationInterface.prototype.restart = function() {
      console.log("INTERFACE restart");
      return location.reload();
    };
    
    PyxisApplicationInterface.prototype.close = function(name, value) {
      if (electron) {
        electron.remote.app.quit();
      }
    };
    
    PyxisApplicationInterface.prototype.openFileDialog = function(name, value) {
      var dfd;
      dfd = promiseShim();
      if (electron && geoWebCoreUrl.isLocal && geoWebCoreUrl.isLocal()) {
        electron.remote.dialog.showOpenDialog({},function(files) {
          if (files.length>0) {
            var url = new Url(geoWebCoreUrl.post() + "/api/v1/Local/Catalog");
            var apiRequest = $http.post(url.toString(), {Uri: files[0]} );
            apiRequest.success(function(catalog) {
                dfd.resolve(catalog.DataSets);
            });
            apiRequest.error(function(error) {
               dfd.reject(error);
            });
          }
        });
      } else {
        dfd.resolve([]);
      }
      return dfd.promise;
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

    PyxisApplicationInterface.prototype.fileDragEnter = function(callback) {
      console.log("INTERFACE fileDragEnter");
      if (dragDropSupport) {
          dragDropSupport.on('dragEnter',function() {
              callback(["for backward compatibility"]); 
          });
      }
    };
    
    PyxisApplicationInterface.prototype.fileDragLeave = function(callback) {
      console.log("INTERFACE fileDragLeave");
      if (dragDropSupport) {
          dragDropSupport.on('dragLeave',function() {
              callback(["for backward compatibility"]); 
          });
      }
    };
    
    PyxisApplicationInterface.prototype.fileDragDrop = function(callback) {
      console.log("INTERFACE fileDragDrop");
      if (dragDropSupport) {
          dragDropSupport.on('drop',function(files) {
              angular.forEach(files,function(file) {
                var url = new Url(geoWebCoreUrl.post() + "/api/v1/Local/Catalog");
                var apiRequest = $http.post(url.toString(), {Uri: file.path} );
                apiRequest.success(function(catalog) {
                    //we need to stringify the results to keep up with the old studio behaviour
                    callback(JSON.stringify(catalog.DataSets));
                });
              }) 
          });
      }
    };

    PyxisApplicationInterface.prototype.getVersion = function() {
      return '1.0.0.1230';
    };

    return PyxisApplicationInterface;

  })();
  elevationGeoSource = null;
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
      this.layers = {};
      this.lastId = 0;
      this._createGlobeWidget();
      this._bindGlobeEvents();

      // use the default view for loading feedback
      this.loadView = new $window.PYXIS.LoadFeedbackView(this.globe);
    }

    PyxisGlobeInterface.options = {
      refereshGlobeDelayInMs: 100
    }

    PyxisGlobeInterface.prototype._newID = function() {
      this.lastId++;
      return "id-" + this.lastId;
    };

    PyxisGlobeInterface.prototype._createGlobeWidget = function() {
      var elementId, randomId, template;

      // allow using an existing globe
      if ($window.GC){
        return this.globe = $window.GC;
      } else {
        elementId = "globe-default";
        if ($('#globe-default').length){
          randomId = Math.round(Math.random() * 9999);
          elementId = "globe-" + randomId;
        }
        template = "<div id=\"" + elementId + "\" geo-source-id=\"geoSourceId\" style=\"left: 0; top: 0px; position: absolute;\"></div>";
        $('body').prepend(template);
        this.globe = new $window.PYXIS.GlobeCanvas($("#" + elementId)[0], elementId);
        return $window.GC = this.globe;
      }
    };


    /**
     *  Here we bind all events emitted from the GlobeCanvas so that they are
     *  broadcasted in an angular scope
     */
    PyxisGlobeInterface.prototype._bindGlobeEvents = function() {

      var _oldEmit = this.globe.emit;
      this.globe.emit = function(type, data){
        $rootScope.$broadcast(type, data);
        _oldEmit.apply(this, arguments);
      }
    };

    

    PyxisGlobeInterface.prototype._scheduleRefreshGlobe = function() {
      var self = this;
      if (self._scheduledCallback) {
        $timeout.cancel(self._scheduledCallback);
      }

      //start preparing layers
      self._prepareLayersForGlobe();

      self._scheduledCallback = $timeout(
        function() { self._refreshGlobe(); },
        PyxisGlobeInterface.options.refereshGlobeDelayInMs);
    }


    function identifyTextField(geoSource) {
      //find field that:
      //  1) field.Name = 'NAME'.
      //  2) if not, field.Name contains 'NAME'.
      //  3) if not, field.Metadata.Name contains 'NAME'.
      //  4) if not, any starred field that of type string.
      var field = 
        _.find(geoSource.Specification.Fields, function(field) {
          return field.Name.toUpperCase() == 'NAME';
        }) || _.find(geoSource.Specification.Fields, function(field) {
          return field.Name.toUpperCase().indexOf('NAME') != -1;
        }) || _.find(geoSource.Specification.Fields, function(field) {
          return field.Metadata.SystemTags && field.Metadata.Name.toUpperCase().indexOf('NAME') != -1;
        }) || _.find(geoSource.Specification.Fields, function(field) {
          return field.Metadata.SystemTags && field.Metadata.SystemTags[0] === 'Favorite' && field.FieldType === 'String';
        });
      if (field) {
        return field.Name;
      }
      return undefined;
    }

    PyxisGlobeInterface.prototype._prepareLayersForGlobe = function() {
      var layersOrdering, readyLayers, self, sortedLayers;
      self = this;
     
      layersOrdering = function(layer) {
        var baseScore;
        baseScore = 0.0;
        if (layer.characterization && layer.characterization.NativeResolution) {
          baseScore = layer.characterization.NativeResolution / 51.0;
        }
        if (layer.style.ShowAsElevation) {
          elevationGeoSource = layer.geoSource.Id;
          return baseScore + 0;
        }
        if (layer.geoSource.Specification && layer.geoSource.Specification.OutputType === "Coverage") {
          if (!layer.style.Fill) {
            return baseScore + 1;
          }
          return baseScore + 2;
        }
        return baseScore + 3;
      };
      sortedLayers = _.values(this.layers);
      sortedLayers = _.sortBy(sortedLayers, layersOrdering);
      
      _.forEach(sortedLayers, function(layer) {
        if (layer.style && layer.style.Icon && ! layer.style.Icon.HoverExpression) {
          var field = identifyTextField(layer.geoSource);
          if (field) {
            //Hack: change style inside $pyx.globe.show(geoSource,style) causes endless loop of show updates on angular level at the moment.
            layer.styleHint = { Icon: { HoverExpression: field } }
          }
        }
      });

      readyLayers = _.filter(sortedLayers, function(layer) {
        var apiRequest, url;
        if (layer.characterization) {
          return true;
        }
        if (isLocalGeoWebCoreInitializingGeoSource(layer.geoSource.Id)) {
          //try again half a second
          $timeout(function() {
              self._scheduleRefreshGlobe();
          }, 500);
          return false;
        } else if (!layer.requestingCharacterization) {
          layer.requestingCharacterization = true;
          url = new Url(geoWebCoreUrl.get(layer.geoSource.Id) + "/api/v1/GeoSource").dir(layer.geoSource.Id).dir('Characterize');
          apiRequest = $http.get(url.toString());
          apiRequest.success(function(characterization) {
            var boundingSphere, center;
            center = wgs84.latLonToXyz({
              lat: characterization.BoundingCircle.coordinates[1],
              lon: characterization.BoundingCircle.coordinates[0]
            });
            boundingSphere = new THREE.Sphere(center, characterization.BoundingCircle.radius);
            characterization.boundingSphere = boundingSphere;
            layer.characterization = characterization;
            layer.requestingCharacterization = false;
            return self._scheduleRefreshGlobe();
          });
          apiRequest.error((function(_this) {
            return function(error) {
              return layer.requestingCharacterization = false;
            };
          })(this));
        }
        return false;
      });

      return readyLayers;
    }




    // Note :: below this we are wrapping it with _.throttle
    PyxisGlobeInterface.prototype._refreshGlobe = function() {
      var readyLayers, self;
      self = this;
      
      delete self._scheduledCallback;

      readyLayers = self._prepareLayersForGlobe();
      
      return this.globe.preloadLayers(readyLayers, void 0, function() {
        return window.GC.doSwapScene();
      });
    };
    
    PyxisGlobeInterface.prototype.renderReport = function() {
      console.log("INTERFACE renderReport");
      return {};
    };

    PyxisGlobeInterface.prototype.cursor = function() {
      var circleGeometry = this.globe.getMouseGeometryOnGlobe()
      if (circleGeometry) {
        return JSON.stringify(circleGeometry); 
      }
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
      apiRequest = $http.post(geoWebCoreUrl.post(geoSource.Id) + "/api/v1/Local/Style", {
        geoSource: geoSource
      });
      apiRequest.success((function(_this) {
        return function(style) {
          _this.layers[id] = {
            geoSource: geoSource,
            style: style
          };
          _this._scheduleRefreshGlobe();
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
      this._scheduleRefreshGlobe();
      dfd = promiseShim();
      dfd.resolve(id);
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.createDefaultStyle = function(geoSource, style) {
      var apiRequest, dfd;
      console.log("INTERFACE createDefaultStyle");
      dfd = promiseShim();
      apiRequest = $http.post(geoWebCoreUrl.post(geoSource.Id) + "/api/v1/Local/Style", {
        geoSource: geoSource,
        style: style
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
      this._scheduleRefreshGlobe();
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
      this._scheduleRefreshGlobe();
      dfd = promiseShim();
      dfd.resolve(id);
      return dfd.promise;
    };

    PyxisGlobeInterface.prototype.isVisibleIdLoading = function(id) {
      console.log("INTERFACE isVisibleIdLoading");
      if (!this.layers[id]) {
        return false;
      }
      var geoSourceId = this.layers[id].geoSource.Id;
      return !this.layers[id].characterization || this.globe.isGeoSourceLoading(geoSourceId) || isLocalGeoWebCoreInitializingGeoSource(geoSourceId);
    };

    PyxisGlobeInterface.prototype.getAllLoadingVisibleIds = function() {
      var dfd, key, ref, result, value;
      result = [];
      ref = this.layers;
      for (key in ref) {
        value = ref[key];
        if (!value.characterization || this.globe.isGeoSourceLoading(value.geoSource.Id) || isLocalGeoWebCoreInitializingGeoSource(value.geoSource.Id)) {
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
      apiRequest = $http.post(geoWebCoreUrl.post(layer.geoSource.Id) + "/api/v1/Local/AutoStyle", {
        geoSource: layer.geoSource,
        style: layer.style,
        field: fieldName,
        palette: palette
      });
      apiRequest.success((function(_this) {
        return function(style) {
          _this.layers[id].style = style;
          dfd.resolve(style);
          return _this._scheduleRefreshGlobe();
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
      return this.setStyleByFieldWithPalette(id, fieldName, palette);
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
      url = new Url(geoWebCoreUrl.post() + "/api/v1/Local/BoundingCircle");
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
      url = new Url(geoWebCoreUrl.post(geoSource.Id) + "/api/v1/GeoSource");
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

    PyxisGlobeInterface.prototype._stopCameraAnimation = function() {
      var element;
      if (!this.stopOnMousedownRegistered) {
        element = $(this.globe.renderer.domElement);
        element.mousedown((function(_this) {
          return function() {
            return _this._stopCameraAnimation();
          };
        })(this));
        element.on('mousewheel', (function(_this) {
          return function() {
            return _this._stopCameraAnimation();
          };
        })(this));
        this.stopOnMousedownRegistered = true;
      }
      if (this.currentCameraAnimation) {
        this.currentCameraAnimation.kill();
        return this.currentCameraAnimation = void 0;
      }
    };

    PyxisGlobeInterface.prototype.setCamera = function(camera, options, callback) {
      options = options || {};

      if (!_.isObject(options)) {
        options = {duration: options};
      }  

      this._stopCameraAnimation();

      var res = this.globe.animateCamera(camera, options, callback);

      this.currentCameraAnimation = res.currentCameraAnimation;

      return res.currentCamera;
    };

    PyxisGlobeInterface.prototype.screenToGeographicPosition = function(locations) {
      var dfd, offset, wgs84Points;
      //console.log("INTERFACE screenToGeographicPosition", locations);
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

        //render the globe into a canvas readable form
        this.globe.render();
        var sourceCanvas = this.globe.renderer.context.canvas;

        //copy into a smaller canvas
        var canvas = document.createElement('canvas');
        canvas.width = Math.min(1280,sourceCanvas.width);
        canvas.height = Math.floor(sourceCanvas.height*(canvas.width/sourceCanvas.width));

        var context = canvas.getContext("2d");
        context.drawImage(sourceCanvas,0,0,canvas.width,canvas.height);

        //return as data url
        var dfd = promiseShim();

        //TODO: if (electron) { upload file localy and generate url }
        dfd.resolve(canvas.toDataURL());

        return dfd.promise;
    };

    PyxisGlobeInterface.prototype.click = function(callback) {
      var element, self, clicks;
      console.log("INTERFACE click", callback);
      element = $(this.globe.renderer.domElement);
      self = this;
      clicks = 0;
      return element.click(function(event) {
        var circleGeometry, simplifedEvent;
        
        simplifedEvent = {
          clientX: event.clientX,
          clientY: event.clientY
        };
        clicks++;

        //ensure we are not trigger 2 clicks when the user dblclick
        window.setTimeout(function() {
            if (clicks==1) {
                //TODO: this need to be imrpoved as it will not return correct results when zoom in deep and tilted
                //      this need to be replaced with ray tracing
                circleGeometry = self.globe.getMouseGeometryOnGlobe();
                var clickTrace = self.globe.getMouseClickTrace() || {};
                var clickMetadata = self.globe.getMouseClickMetadata();

                if (circleGeometry && clickTrace.dragLength < 2) {
                    callback(circleGeometry, JSON.stringify(simplifedEvent), clickMetadata);
                }
            }
            clicks = 0;
        },200);
        
        return false;
      });
    };

    PyxisGlobeInterface.prototype.dragstart = function(callback) {
        this.globe.on('dragstart', callback );
    }

    PyxisGlobeInterface.prototype.dragend = function(callback) {
        this.globe.on('dragend',callback );
    }

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
        var clickTrace = self.globe.getMouseClickTrace() || {};
        var clickMetadata = self.globe.getMouseClickMetadata();

        if (circleGeometry && clickTrace.dragLength < 2) {
          event.preventDefault();
          callback(circleGeometry, JSON.stringify(simplifedEvent), clickMetadata);
        }
        return false;
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

    var activeImportRequests = {}

    PyxisEngineInterface.prototype.getDefinition = function(geoSource) {
      var apiRequest, dfd;
      dfd = promiseShim();
      if (geoSource.Sepcification) {
        dfd.resolve(geoSource.Sepcification);
      } else {
        apiRequest = $http.post(geoWebCoreUrl.post(geoSource.Id) + "/api/v1/Local/Specification", geoSource);
        apiRequest.success(function(spec) {
          console.log("INTERFACE getDefintion resolved", geoSource, spec);
          return dfd.resolve(spec);
        });
        apiRequest.error(function(error) {
          console.log("INTERFACE getDefintion failed", geoSource, error);
          return dfd.reject(error);
        });
      }
      return dfd.promise;
    };

    PyxisEngineInterface.prototype.getFeatures = function(geoSource, geometry) {
      var apiRequest, dfd, url;
      console.log("INTERFACE getFeatures", geoSource, geometry);
      dfd = promiseShim();
      url = new Url(geoWebCoreUrl.get(geoSource.Id) + "/api/v1/GeoSource/");
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
      apiRequest = $http.post(geoWebCoreUrl.post() + "/api/v1/Local/Import/GeoJson", featureCollection);
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
      url = new Url(geoWebCoreUrl.get(elevationGeoSource) + "/api/v1/Local/Watershed").search({
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
      url = new Url(geoWebCoreUrl.get(geoSource.Id) + "/api/v1/GeoSource/");
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
      url = new Url(geoWebCoreUrl.get(geoSource.Id) + "/api/v1/GeoSource");
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
      apiRequest = $http.post(geoWebCoreUrl.post() + "/api/v1/Local/Area", geometry);
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
        var apiRequest, dfd, url;
        console.log("INTERFACE getDataSize", geoSource);
        dfd = promiseShim();
        url = new Url(geoWebCoreUrl.get(geoSource.Id) + "/api/v1/GeoSource").dir(geoSource.Id).dir('DataSize');
        apiRequest = $http.get(url.toString());
        apiRequest.success(function(size) {
            console.log("INTERFACE getDataSize success", geoSource, size);
            return dfd.resolve(size);
        });
        apiRequest.error(function(error) {
            console.log("INTERFACE getDataSize error", geoSource, error);
            return dfd.reject(error);
        });
        return dfd.promise;
    };

    PyxisEngineInterface.prototype.setGeoTagReferenceGeoSources = function(references) {
      console.log("INTERFACE setGeoTagReferenceGeoSources", references);
      var self = this;
      self._geoTagReferences = [];
      
      angular.forEach(references, function(ref) {
          self._geoTagReferences.push({
              ReferenceGeoSource: { Id: ref.ReferenceGeoSource.Id, Version: ref.ReferenceGeoSource.Version, Type: ref.ReferenceGeoSource.Type },
              ReferenceFieldName: ref.ReferenceFieldName
          });
      });
      if (self._geoTagReferences.length == 0)
      {
          delete self._geoTagReferences;
      }
    };

    PyxisEngineInterface.prototype.provideImportSetting = function(dataSet, settingType, value) {
        console.log("INTERFACE provideImportSetting", dataSet, settingType, value);
        var dfd = promiseShim();
        if (dataSet.Uri in activeImportRequests) {
            if (settingType == "SRS") {
                activeImportRequests[dataSet.Uri].request.SRS = value;
                //continue to import the dataset with new data
                if (value) { 
                    this.import(dataSet); 
                } else {
                    activeImportRequests[dataSet.Uri].deferred.reject("No SRS was provided.");
                }
            }
            if (settingType == "GeoTag") {
                if (!value) {
                    activeImportRequests[dataSet.Uri].request.GeoTag = value;
                    activeImportRequests[dataSet.Uri].deferred.reject("No GeoTag method was provided.");
                } else if (value.Point) {
                    activeImportRequests[dataSet.Uri].request.GeoTag = {
                        LatLon: {
                            LatitudeFieldName: value.Point.LatitudeFieldName,
                            LongitudeFieldName: value.Point.LongitudeFieldName,
                            Resolution: value.Point.Resolution,
                            ReferenceSystem: value.Point.ReferenceSystem
                        }
                    }
                    //continue to import the dataset with new data
                    this.import(dataSet);
                } else if (value.Lookup) {
                    activeImportRequests[dataSet.Uri].request.GeoTag = {
                        BasedOn: {
                            ReferenceGeoSource: { Id: value.Lookup.ReferenceGeoSource.Id, Version: value.Lookup.ReferenceGeoSource.Version, Type: value.Lookup.ReferenceGeoSource.Type },
                            ReferenceFieldName: value.Lookup.ReferenceFieldName,
                            TargetFieldName: value.Lookup.RecordCollectionFieldName
                        }
                    }
                    //continue to import the dataset with new data
                    this.import(dataSet); 
                }
            }
            dfd.resolve(true);
        } else {
            dfd.reject("can't find import request");
        }
        return dfd.promise;
    };
    
    PyxisEngineInterface.prototype.importSettingRequired = function(callback) {
        this._importSettingRequiredCallback = callback;
    }

    PyxisEngineInterface.prototype["import"] = function(dataSet) {
        console.log("INTERFACE import", dataSet);
        var self = this;
        var apiRequest, dfd;
        var importState = activeImportRequests[dataSet.Uri];
        
        if (!importState) {
            //start a new request
            activeImportRequests[dataSet.Uri] = importState = {
                request: angular.copy(dataSet),
                deferred: promiseShim(),
            };
        }
        
        var dfd = importState.deferred;
        
        importState.request.GeoTagReferences = this._geoTagReferences;
        
        apiRequest = $http.post(geoWebCoreUrl.post() + "/api/v1/Local/Import/DataSet", importState.request);
        apiRequest.success(function(geoSource) {
            console.log("INTERFACE import success", dataSet, geoSource);
            //request completed - we can remove it state
            delete activeImportRequests[dataSet.Uri];
            return dfd.resolve(geoSource);
        });
        apiRequest.error(function(error) {
            console.log("INTERFACE import error", dataSet, error);
            if (self._importSettingRequiredCallback) { 
                if (error.RequiredInformation) {
                    var info = ""
                    var missingField = "";
                    if ("SRS" in error.RequiredInformation) {
                        missingField = "SRS";
                    }
                    if ("GeoTag" in error.RequiredInformation) {
                        missingField = "GeoTag";
                        var oldGeoTagOptions = [];
                        angular.forEach(error.RequiredInformation.GeoTag.PossibleValues,function(option) {
                        if (option.BasedOn) {
                            oldGeoTagOptions.push({
                                Lookup: {
                                    ReferenceGeoSource: dispatcher.stores.resourcesStore.get(option.BasedOn.ReferenceGeoSource.Id),
                                    ReferenceFieldName: option.BasedOn.ReferenceFieldName,
                                    RecordCollectionFieldName: option.BasedOn.TargetFieldName,
                                }
                            });
                        }
                        if (option.LatLon) {
                            oldGeoTagOptions.push({
                                Point: {
                                    LatitudeFieldName: option.LatLon.LatitudeFieldName,
                                    LongitudeFieldName: option.LatLon.LongitudeFieldName,
                                    Resolution: option.LatLon.Resolution,
                                    ReferenceSystem: option.LatLon.ReferenceSystem
                                }
                            });
                        }
                        });
                        info = JSON.stringify(oldGeoTagOptions);
                    }
                    self._importSettingRequiredCallback(JSON.stringify(dataSet),missingField,info);
                    return;
                }
            } 
            return dfd.reject(error);
        });
        return dfd.promise;
    };

    PyxisEngineInterface.prototype.supportedImportFileFormats = function() {
      var dfd, types;
      types = ["adf", "asc", "csv", "ddf", "dem", "dt0", "dt1", "dt2", "e00", "g98", "gif", "gml", "grb", "grd", "img", "jpg", "kml", "ntf", "png", "ppl", "shp", "sid", "tab", "tif", "tiff", "tl2", "toc", "vrt", "json", "geojson" ];
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
    
    if (geoWebCoreUrl.isLocal && geoWebCoreUrl.isLocal()) {
        PyxisEngineInterface.prototype.isOgcUrl = function(url) {
            url = url.toLowerCase()
            
            return url.indexOf("http") == 0 && (
                url.indexOf("service=wms") > 0 ||
                url.indexOf("service=wfs") > 0 ||
                url.indexOf("service=wcs") > 0 );
        }
        
        PyxisEngineInterface.prototype.openOgcServer = function(ogcUrl) {
            var dfd;
            dfd = promiseShim();
            if (geoWebCoreUrl.isLocal()) {
                var url = new Url(geoWebCoreUrl.post() + "/api/v1/Local/Catalog");
                var apiRequest = $http.post(url.toString(), {Uri: ogcUrl } );
                apiRequest.success(function(catalog) {
                    dfd.resolve(catalog);
                });
                apiRequest.error(function(error) {
                    dfd.reject(error);
                });
            } else {
                dfd.reject("No local GeoWebCore available");
            }
            return dfd.promise;
        }
    }

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
