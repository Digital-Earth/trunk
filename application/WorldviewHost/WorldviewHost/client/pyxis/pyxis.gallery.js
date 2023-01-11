angular.module('pyxis')
    .factory('$pyxgallery', function ($q, $http, $pyxuser, $pyxconfig) {
        var baseUrl = function () { return $pyxconfig.baseUrl; };

        var addFilters = function (queryString, filters) {
            for (var key in filters) {
                if (queryString != "") {
                    queryString += "&";
                }
                queryString += key + "=" + encodeURIComponent(filters[key]);
            }
            return queryString;
        };

        var modifyHttpPromise = function (promise, modifyFunc) {
            var newPromise = promise.then(function (response) {
                response.data = modifyFunc(response.data);
                return response;
            });
            newPromise.success = function (fn) {
                promise.then(function (response) { fn(response.data, response.status, response.headers, response.config); });
                return promise;
            };
            newPromise.error = function (fn) {
                promise.then(null, function (response) { fn(response.data, response.status, response.headers, response.config); });
                return promise;
            };
            return newPromise;
        }

        var Query = function (resource) {

            return {
                resource: resource,
                filters: "",
                top: 50,
                skip: 0,
                search: function (searchString) {
                    searchString = searchString.trim();
                    if (!searchString) {
                        return this;
                    }
                    var words = searchString.split(" ");
                    var filter = "";
                    if (words.length > 1) {
                        filter = '"' + words.join('" AND "') + '"';
                    } else {
                        filter = words[0];
                    }
                    this.filters = addFilters(this.filters, { "Search": filter });
                    return this;
                },
                format: function (format) {
                    this.filters = addFilters(this.filters, { "Format": format });
                    return this;
                },
                group: function (group) {
                    this.filters = addFilters(this.filters, { "Grouping": group });
                    return this;
                },
                select: function () {
                    var args = Array.prototype.slice.call(arguments);
                    this.filters = addFilters(this.filters, { "$select": args.join() });
                    return this;
                },
                expand: function () {
                    var args = Array.prototype.slice.call(arguments);
                    this.filters = addFilters(this.filters, { "$expand": args.join() });
                    return this;
                },
                filterEq: function (field, value) {
                    if (angular.isNumber(value)) {
                        this.filters = addFilters(this.filters, { "$filter": field.replace(/\./g, "/") + " eq " + value });
                    } else if (angular.isString(value)) {
                        this.filters = addFilters(this.filters, { "$filter": field.replace(/\./g, "/") + " eq '" + value.replace("'", "\\'") + "'" });
                    } else {
                        throw "unsupported value type (number or string)";
                    }
                    return this;
                },
                filter: function (condition) {
                    this.filters = addFilters(this.filters, { "$filter": condition });
                    return this;
                },
                orderBy: function (field) {
                    this.filters = addFilters(this.filters, { "$orderby": field.replace(/\./g, "/") });
                    return this;
                },
                orderByDesc: function (field) {
                    this.filters = addFilters(this.filters, { "$orderby": field.replace(/\./g, "/") + " desc" });
                    return this;
                },
                activeOnly: function () {
                    this.filters = addFilters(this.filters, { "$filter": 'State eq "Active"' });
                    return this;
                },
                getById: function (id) {
                    return $http({
                        'method': 'GET',
                        'url': baseUrl() + "/" + this.resource + "/" + id,
                        'headers': $pyxuser.authHeaders()
                    });
                },
                getByName: function (name) {
                    return $http({
                        'method': 'GET',
                        'url': baseUrl() + "/" + this.resource + "?name=" + encodeURIComponent(name),
                        'headers': $pyxuser.authHeaders()
                    });
                },
                getVersions: function (id) {
                    return modifyHttpPromise(
                        $http({
                            'method': 'GET',
                            'url': baseUrl() + "/" + this.resource + "/" + id + "/Versions",
                            'headers': $pyxuser.authHeaders()
                        }),
                        function (data) { return data.Items; });
                },
                getByIdAndVersion: function (id, version) {
                    return $http({
                        'method': 'GET',
                        'url': baseUrl() + "/" + this.resource + "/" + id + "?" + addFilters("", { "Version": version }),
                        'headers': $pyxuser.authHeaders()
                    });
                },
                get: function () {
                    return modifyHttpPromise(
                        $http({
                            'method': 'GET',
                            'url': baseUrl() + "/" + this.resource + "?" + addFilters(this.filters, { "$top": this.top, "$skip": this.skip }),
                            'headers': $pyxuser.authHeaders()
                        }),
                        function (data) { return data.Items; });
                },
                getAll: function () {
                    var deferred = $q.defer();
                    var result = [];
                    var self = this;
                    var doRequest = function () {
                        self.get().then(function (response) {
                            try {
                                var items = response.data;
                                if (items.length) {
                                    deferred.notify(items);
                                    result = result.concat(items);
                                    if (items.length < self.top) {
                                        //not a full page, query was completed
                                        deferred.resolve(result);
                                    } else {
                                        //full page, lets ask the next page
                                        self.nextPage();
                                        doRequest();
                                    }
                                } else {
                                    //no results, query was completed
                                    deferred.resolve(result);
                                }
                            } catch (error) {
                                deferred.reject(error);
                            }
                        }, function (response) {
                            deferred.reject(response.data);
                        });
                    }
                    var promise = deferred.promise;

                    //$http respone fake...
                    promise.success = function (fn) {
                        promise.then(function (data) { fn(data); });
                        return promise;
                    }
                    promise.error = function (fn) {
                        promise.then(null, function (data) { fn(data); });
                        return promise;
                    }

                    //begin fetching results
                    doRequest();

                    return promise;
                },
                nextPage: function () {
                    this.skip += this.top;
                    return this;
                },
                getFirst: function () {
                    return modifyHttpPromise(
                        $http({
                            'method': 'GET',
                            'url': baseUrl() + "/" + this.resource + "?" + addFilters(this.filters, { "$top": 1, "$skip": 0 }),
                            'headers': $pyxuser.authHeaders()
                        }),
                        function (data) { return data.Items.length ? data.Items[0] : undefined; });
                }
            };
        }

        var gallery = {
            'resourceTypes': function () {
                return ["GeoSource", "Map", "Gallery", "User", "License"];
            },
            'search': function (searchString) {
                return this.geoSources().search(searchString).activeOnly().getAll();
            },
            'searchExternal': function (searchString, camera, bbox) {
                var queryParameters = {};
                if (searchString) {
                    queryParameters['search'] = searchString;
                }
                if (camera) {
                    queryParameters['centerLat'] = camera.Latitude;
                    queryParameters['centerLon'] = camera.Longitude;
                }
                if (bbox) {
                    queryParameters['upperLat'] = bbox[0][0];
                    queryParameters['leftLon'] = bbox[0][1];
                    queryParameters['lowerLat'] = bbox[1][0];
                    queryParameters['rightLon'] = bbox[1][1];
                }
                return modifyHttpPromise(
                    $http({
                        'method': 'GET',
                        'url': baseUrl() + "/ExternalData?" + angular.element.param(queryParameters),
                        'headers': $pyxuser.authHeaders()
                    }),
                    function (data) { return data.Items; });
            },
            'suggestTerms': function (searchString) {
                return modifyHttpPromise(
                    $http({
                        'method': 'GET',
                        'url': baseUrl() + "/Metadata/SuggestTerms?search=" + encodeURIComponent(searchString),
                        'headers': $pyxuser.authHeaders()
                    }),
                    function (data) { return data.Items; });
            },
            'suggestCompletions': function (searchString) {
                return modifyHttpPromise(
                    $http({
                        'method': 'GET',
                        'url': baseUrl() + "/Metadata/SuggestCompletions?search=" + encodeURIComponent(searchString),
                        'headers': $pyxuser.authHeaders()
                    }),
                    function (data) { return data.Items; });
            },
            'suggestMatches': function (searchString, types) {
                if (!types) {
                    types = ['GeoSource', 'Map', 'Gallery'];
                }
                return modifyHttpPromise(
                    $http({
                        'method': 'GET',
                        'url': baseUrl() + "/Metadata/SuggestMatches?search=" + encodeURIComponent(searchString) + "&types=" + encodeURIComponent(types.join(',')),
                        'headers': $pyxuser.authHeaders()
                    }),
                    function (data) { return data.Items; });
            },
            'update': function (resource) {
                if (this.resourceTypes().indexOf(resource.Type) !== -1) {
                    var fixedResource = angular.copy(resource);
                    delete fixedResource['Id'];
                    delete fixedResource['Version'];
                    delete fixedResource['Type'];

                    return $http({
                        'method': 'PUT',
                        'data': fixedResource,
                        'url': baseUrl() + "/" + resource.Type + "/" + resource.Id + "?version=" + resource.Version,
                        'headers': $pyxuser.authHeaders()
                    });
                }
                return undefined;
            },
            'delete': function (resource) {
                if (this.resourceTypes().indexOf(resource.Type) !== -1) {
                    return $http({
                        'method': 'DELETE',
                        'url': baseUrl() + "/" + resource.Type + "/" + resource.Id,
                        'headers': $pyxuser.authHeaders()
                    });
                }
                return undefined;
            },
            'create': function (resource,options) {
                options = options || {};
                if (this.resourceTypes().indexOf(resource.Type) !== -1) {
                    return $http({
                        'method': 'POST',
                        'data': resource,
                        'url': baseUrl() + "/" + resource.Type,
                        'headers': options.anonymous ? undefined : $pyxuser.authHeaders()
                    });
                }
                return undefined;
            },
            'resources': function (types) {
                types = types || ["GeoSource", "Map", "Gallery"];
                var resources = Query("Metadata");
                resources.filters = addFilters(resources.filters, { "types": types.join(',') });
                resources.getByProcRef = function (procref) {
                    return $http({
                        'method': 'GET',
                        'url': baseUrl() + "/Pipeline/Resource/ProcRef/" + encodeURIComponent(procref),
                        'headers': $pyxuser.authHeaders()
                    });
                }
                return resources;
            },
            'geoSources': function () {
                return Query("GeoSource");
            },
            'maps': function () {
                return Query("Map");
            },
            'users': function () {
                var query = Query("User");

                query.nameAvailable = function (name) {
                    return $http({
                        'method': 'GET',
                        'url': baseUrl() + "/User/Available?Name=" + encodeURIComponent(name),
                        'headers': $pyxuser.authHeaders()
                    });
                }

                return query;
            },
            'galleries': function () {
                var query = Query("Gallery");

                query.nameAvailable = function (name) {
                    return $http({
                        'method': 'GET',
                        'url': baseUrl() + "/Gallery/Available?Name=" + encodeURIComponent(name),
                        'headers': $pyxuser.authHeaders()
                    });
                }

                return query;
            },
            'licenses': function () {
                return Query("License");
            },
            'licensedAccess': function (resourceId) {
                return $http({
                    'method': 'GET',
                    'url': baseUrl() + "/Agreement/User/LicensedAccess?ResourceId=" + resourceId,
                    'headers': $pyxuser.authHeaders()
                });
            },
            'recordLicenseAgreement': function (license, decision) {
                return $http({
                    'method': 'POST',
                    'url': baseUrl() + "/Agreement/License/" + license.Id + "?LicenseVersion=" + license.Version + '&Decision=' + decision,
                    'headers': $pyxuser.authHeaders()
                });
            },
            'terms': function () {
                return $http({
                    'method': 'GET',
                    'url': baseUrl() + "/License/Terms",
                    'headers': $pyxuser.authHeaders()
                });
            },
            'forgotPassword': function (userName) {
                var requestBody = { UserName: userName };
                return $http({
                    'method': 'POST',
                    'data': requestBody,
                    'url': baseUrl() + "/Account/ForgotPassword"
                });
            },
            'resetPassword': function (userName, resetToken, newPassword, confirmPassword) {
                var requestBody = { UserName: userName, ResetToken: resetToken, NewPassword: newPassword, ConfirmPassword: confirmPassword };
                return $http({
                    'method': 'POST',
                    'data': requestBody,
                    'url': baseUrl() + "/Account/ResetForgottenPassword"
                });
            },
            'sendConfirmationEmail': function () {
                return $http({
                    'method': 'GET',
                    'url': baseUrl() + "/Account/SendConfirmationEmail",
                    'headers': $pyxuser.authHeaders()
                });
            },
            'confirmEmail': function (email, confirmationToken) {
                var requestBody = { Email: email, ConfirmationToken: confirmationToken };
                return $http({
                    'method': 'POST',
                    'data': requestBody,
                    'url': baseUrl() + "/Account/ConfirmEmail"
                });
            },
            'comment': function (resource, message, replyTo) {
                var comment = { Message: message };
                if (replyTo) {
                    comment["ReplyTo"] = replyTo;
                }
                return $http({
                    'method': 'POST',
                    'data': comment,
                    'url': baseUrl() + "/Metadata/Comment/" + resource.Id,
                    'headers': $pyxuser.authHeaders()
                });
            },
            'rate': function (resource, value) {
                return $http({
                    'method': 'POST',
                    'url': baseUrl() + "/Metadata/Rating/" + resource.Id + "?Value=" + value,
                    'headers': $pyxuser.authHeaders()
                });
            },
            'geoSource': function (id) {
                return Query("GeoSource").getById(id);
            }
        };

        var ResourceAgent = function (resource) {
            var self = {
                'json': resource,
                'comment': function (comment, replyTo) { return gallery.comment(this.json, comment, replyTo) },
                'rate': function (value) { return gallery.rate(this.json, value) },
                'refresh': function () { return Query(this.json.Type).getById(this.json.Id).success(function (data) { self.json = data; }); },
                'versions': function () { return Query(this.json.Type).getVersions(this.json.Id); },
                'version': function (version) { return Query(this.json.Type).getByIdAndVersion(this.json.Id, version); },
                'updateImage': function (url) {
                    var self = this;
                    var deferred = $q.defer();
                
                    imgUrlToDataUrl(url).then(function (dataUrl) {
                        var blob = dataURItoBlob(dataUrl);
                    
                        var formData = new FormData();
                        formData.append(self.json.Id + ".png", blob);
                    
                        $http.post('https://www.pyxisinnovation.com/data/catalogue/saveImage.php', formData, {
                            transformRequest: angular.identity,
                            headers: { 'Content-Type': undefined }
                        }).success(function () {
                            if (!self.json.Metadata.ExternalUrls) {
                                self.json.Metadata.ExternalUrls = [];
                            }
                            var urlValue = 'https://www.pyxisinnovation.com/images/pipelines/' + self.json.Id + '.png';
                        
                            var foundUrl = false;
                            angular.forEach(self.json.Metadata.ExternalUrls, function (external) {
                                if (external.Type == 'Image') {
                                    external.Url = urlValue;
                                    foundUrl = true;
                                }
                            });
                        
                            if (!foundUrl) {
                                self.json.Metadata.ExternalUrls.push({ Type: 'Image', Url: urlValue });
                            }
                        
                            deferred.resolve(self.json);
                        }).error(function () {
                            deferred.reject('failed to upload image to sever');
                        });
                    }, function (error) {
                        deferred.reject(error);
                    });
                
                    return deferred.promise;
                }
            };
            return self;
        }

        function imgUrlToDataUrl(url) {
            if (url.indexOf('data:') == 0) {
                //url is already data url.
                return $q.when(url);
            }

            var deferred = $q.defer();

            var canvas = document.createElement('CANVAS');
            var ctx = canvas.getContext('2d');
            var img = new Image;
            img.onload = function () {
                canvas.height = img.height;
                canvas.width = img.width;
                ctx.drawImage(img, 0, 0);

                var dataURL = canvas.toDataURL('image/png');
                deferred.resolve(dataURL);
                // Clean up
                canvas = null;
            };
            img.onerror = function () {
                deferred.reject('failed to load image');
            }
            img.src = url;

            return deferred.promise;
        }

        function dataURItoBlob(dataURI) {
            // convert base64/URLEncoded data component to raw binary data held in a string
            var byteString;
            if (dataURI.split(',')[0].indexOf('base64') >= 0) {
                byteString = atob(dataURI.split(',')[1]);
            } else {
                byteString = unescape(dataURI.split(',')[1]);
            }

            // separate out the mime component
            var mimeString = dataURI.split(',')[0].split(':')[1].split(';')[0];

            // write the bytes of the string to a typed array
            var a = new ArrayBuffer(byteString.length);
            var ia = new Uint8Array(a);
            for (var i = 0; i < byteString.length; i++) {
                ia[i] = byteString.charCodeAt(i);
            }

            if (window.WebKitBlobBuilder) {
                var bb = new WebKitBlobBuilder();
                bb.append(a);
                return bb.getBlob(mimeString);
            } else {
                return new Blob([ia], { type: mimeString });
            }
        }

        var GeoSourceAgent = function (resource) {
            var agent = ResourceAgent(resource);
            angular.extend(agent, {
                'status': function () {
                    return $http({
                        'method': 'GET',
                        'url': baseUrl() + "/GeoSource/" + this.json.Id + "/Status",
                        'headers': $pyxuser.authHeaders()
                    });
                }
            });
            return agent;
        }

        var GalleryAgent = function (gallery) {
            var update = function (gallery, method, section, resource) {
                return $http({
                    'method': method,
                    'url': baseUrl() + "/" + gallery.Type + "/" + gallery.Id + "/" + section + "?ResourceId=" + resource.Id,
                    'headers': $pyxuser.authHeaders()
                });
            }

            var agent = ResourceAgent(gallery);
            angular.extend(agent, {
                'resources': function () {
                    return modifyHttpPromise(
                        $http({
                            'method': 'GET',
                            'url': baseUrl() + "/Gallery/" + this.json.Id + "/Expanded",
                            'headers': $pyxuser.authHeaders()
                        }),
                        function (data) { return data.Resources; });
                },
                'publish': function (resource) { return update(this.json, "PUT", "Resource", resource); },
                'unpublish': function (resource) { return update(this.json, "DELETE", "Resource", resource); },
                'feature': function (resource) { return update(this.json, "PUT", "Feature", resource); },
                'unfeature': function (resource) { return update(this.json, "DELETE", "Feature", resource); }
            });
            return agent;
        }

        var MapAgent = function (map) {
            var agent = ResourceAgent(map);
            angular.extend(agent, {
                'geoSources': function () {
                    return Query("Map/" + this.json.Id + "/GeoSources");
                }
            });
            return agent;
        }

        var UserAgent = function (user) {
            var agent = ResourceAgent(user);
            angular.extend(agent, {
                'geoSources': function () {
                    return Query("User/" + this.json.Id + "/GeoSources");
                },
                'galleries': function () {
                    return Query("User/" + this.json.Id + "/Galleries");
                },
            });
            return agent;
        }

        //register resource agents
        $pyxconfig.resourceAgents["GeoSource"] = function (resource) { return GeoSourceAgent(resource); };
        $pyxconfig.resourceAgents["Map"] = function (resource) { return MapAgent(resource); };
        $pyxconfig.resourceAgents["User"] = function (resource) { return UserAgent(resource); };
        $pyxconfig.resourceAgents["License"] = function (resource) { return ResourceAgent(resource); };
        $pyxconfig.resourceAgents["Gallery"] = function (resource) { return GalleryAgent(resource); };

        return gallery;
    });