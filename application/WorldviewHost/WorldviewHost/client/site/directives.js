app.directive('cardTypeIcon', function () {
    return {
        restrict: 'E',
        scope: { 'type': '=' },
        replace: true,
        template: '<img ng-if="img" ng-src="{{img}}">',
        link: function (scope) {
            var root = "/assets/images";
            var imgs = {
                'Gallery': root + "/icon_gallery.png",
                'Map': root + "/icon_map.png",
                'Story': root + "/icon_story.png",
                'Channel': root + "/icon_channel.png",
                'Ogc': root + "/icon_ogc.png"
            };
            var updateIcon = function () {
                scope.img = imgs[scope.type];
            };
            updateIcon();
            scope.$watch('type', updateIcon);
        }
    };
});

app.filter('upgradeResourceType', function ($filter) {
    return function (value) {
        return value === 'Map' ? 'Globe' : value;
    };
});

app.directive('card', function ($window, $location, $timeout, imageServer, $filter) {
    upgradeResourceType = $filter('upgradeResourceType');
    return {
        restrict: 'E',
        scope: {
            'ngModel': '=',
            'tagClick': '&'
        },
        replace: true,
        template: '<div class="card"><a href="{{cardLink}}"><div class="card-img" ng-class="{small:isGeoSource}" ng-style="{\'background-image\': \'url(\'+getImage()+\')\'}"></div><!--<img ng-src="{{getImage()}}"/>--></a><div class="type" ng-hide="ngModel.Type==\'GeoSource\'"><card-type-icon type="ngModel.Type"></card-type-icon>{{(ngModel.Type | upgradeResourceType) | uppercase}}<span ng-if="count"> - {{count}} ITEMS</span></div><div class="name"><i class="fa fa-lock" ng-show="ngModel.Metadata.Visibility == \'Private\'"></i> {{ngModel.Metadata.Name}}</div><div ng-show="ngModel.Metadata.Description" class="desc">{{ngModel.Metadata.Description | ellipsis:100}}</div><div class="tags" ng-hide="ngModel.Metadata.Tags.length == 0"><span ng-repeat="tag in ngModel.Metadata.Tags"><a ng-click="handleTagClick($event,tag)">{{tag}}</a></span></div><div class="user" ng-hide="ngModel.Type==\'User\'"><i class="fa fa-user"></i>{{ngModel.Metadata.User.Name}}<span class="comments"><i class="fa fa-comments"></i>{{ngModel.Metadata.Comments.length}}</span></div>',
        link: function (scope, element, attr) {
            if (attr['ngClick']) {
                scope.cardLink = "";
            } else {
                // a workaround: explicitly replace 'Map' with 'Gobe' in the URL
                var type = upgradeResourceType(scope.ngModel.Type);
                scope.cardLink = type === 'Gallery' ? "/" + scope.ngModel.Metadata.Name : "/" + type + "/" + scope.ngModel.Id;
            }

            scope.imgMode = attr['imgMode'] || 'fast';


            if (attr['tagClick']) {
                scope.handleTagClick = function (event, tag) {
                    event.stopPropagation();
                    scope.tagClick({ '$tag': tag });
                };
            } else {
                scope.handleTagClick = function (event, tag) {
                    event.stopPropagation();
                    $location.search({ search: tag });
                };
            }

            if ('readonly' in attr) {
                scope.cardLink = "";
                scope.handleTagClick = function (event, tag) {
                    event.stopPropagation();
                };
            }

            scope.getImage = function () {
                if (scope.ngModel.Type === "Gallery" && scope.ngModel.Resources.length > 0) {
                    return imageServer.getImageThumbnailUrl(scope.ngModel.Resources[scope.imageIndex].ResourceId, scope.ngModel.Version);
                }
                if (scope.ngModel.Metadata.ExternalUrls && scope.ngModel.Metadata.ExternalUrls.length > 0 ) {
                    for (var i = 0; i < scope.ngModel.Metadata.ExternalUrls.length; i++) {
                        var url = scope.ngModel.Metadata.ExternalUrls[i];
                        if (url.Type === 'Image' && (url.Url.indexOf('asset:') === 0 || url.Url.indexOf('data:') === 0)) {
                            return url.Url;
                        }
                    }
                }
                if (scope.imgMode !== 'asset') {
                    return imageServer.getImageThumbnailUrl(scope.ngModel.Id);
                }
                return imageServer.getImageThumbnailUrl(scope.ngModel.Id, scope.ngModel.Version);
            };
            scope.isGeoSource = scope.ngModel.Type === "GeoSource";
            scope.imageIndex = 0;
            if (scope.ngModel.Type === "Gallery") {
                var imageElement = angular.element($(element[0]).find('.card-img')[0]);
                var updateImageIndex = function (e) {
                    scope.$apply(function () {
                        scope.imageIndex = Math.min(Math.max(0, Math.floor(e.offsetX * scope.ngModel.Resources.length / element.width())), scope.ngModel.Resources.length - 1);
                    });
                };

                imageElement.bind('mousemove', updateImageIndex);
                scope.$on('$destroy', function () {
                    imageElement.off('mousemove', updateImageIndex);
                });
                scope.count = scope.ngModel.Resources.length;
            }
        }
    };
});

app.directive('masonry', function () {
    return {
        restrict: 'C',
        require: '^scrollbar',
        scope: {
            'itemWidth': '=',
            'hSpace': '=',
            'vSpace': '=',
            'shouldExpand': '&'
        },
        link: function (scope, elem, attrs, scrollbar) {
            scope.scrollbar = scrollbar;
            scrollbar.onScroll(scope.$id, function() { scope.invokeShouldExpand(); });

            scope.$on("$destroy", function () {
                scrollbar.unregister(scope.$id);
            });
        },
        controller: function ($scope, $element, $window, $timeout) {
            $element.css({ 'position': 'relative' });
            this.elements = [];
            this.elementsHeights = [];

            var maxColumns = function () {
                var result = Math.floor(($element.parent().width() + $scope.hSpace) / ($scope.itemWidth + $scope.hSpace));
                if (result == 0) return 1;
                return result;
            };
            this.needLayout = false;
            var self = this;

            $scope.invokeShouldExpand = function () {
                var content = $($scope.scrollbar.content[0]);
                if (content[0].scrollHeight - content.height() - content[0].scrollTop < content.height()) {
                    $scope.shouldExpand();
                }
            };
            var heightCheckIndex = 0;
            this.layoutCheck = function () {
                if (self.needLayout) {
                    $timeout(self.orderItems);
                    return;
                }

                var checksPerFunc = Math.min(20, self.elements.length);
                for (var i = 0; i < checksPerFunc; i++) {
                    heightCheckIndex++;
                    if (heightCheckIndex >= self.elements.length) {
                        heightCheckIndex = 0;
                    }
                    if (self.elements[heightCheckIndex].height() !== self.elementsHeights[heightCheckIndex]) {
                        self.needLayout = true;
                        break;
                    }
                }

                // stop checking layout when parent scope is destroyed
                if (!$scope.$parent.$$destroyed) {
                window.setTimeout(self.layoutCheck, 100);
                }
            };
            this.orderItems = function () {

                if (!self.needLayout) {
                    window.setTimeout(self.layoutCheck, 100);
                    return;
                }
                self.needLayout = false;

                var columns = maxColumns();
                var maxTop = 0;
                var width = $scope.itemWidth + $scope.hSpace;
                var tops = [];
                for (var x = 0; x < columns; x++) {
                    tops.push(0);
                }

                var left = 0;
                //var left = (max - (width * tops.length))/2;

                for (var i = 0; i < self.elements.length; i++) {
                    //var column = i % tops.length;
                    var column = tops.indexOf(Math.min.apply(Math, tops));
                    self.elements[i].css({
                        position: 'absolute',
                        top: tops[column] + "px",
                        left: (left + column * width) + "px"
                    });

                    var elmHeight = self.elements[i].height();
                    self.elementsHeights[i] = elmHeight;
                    tops[column] += elmHeight + $scope.vSpace;

                    if (maxTop < tops[column]) {
                        maxTop = tops[column];
                    }
                }

                $element.height(maxTop);
                $element.width(columns * width - $scope.hSpace);

                $scope.invokeShouldExpand();

                var startFade = function () {
                    var count = 0;
                    for (var i = 0; i < self.elements.length; i++) {
                        if (self.elements[i].hasClass('masonry-loading')) {
                            self.elements[i].removeClass('masonry-loading');
                            count++;
                            if (count === 2) {
                                $timeout(startFade, 50);
                                return;
                            }
                        }
                    }
                };
                $timeout(startFade, 30);

                //$timeout(self.orderItems, 500);
                window.setTimeout(self.layoutCheck, 100);
            };
            $timeout(self.orderItems, 100);

            $scope.$watch(maxColumns, function () {
                self.needLayout = true;
            });

            this.addItem = function (element) {
                self.elements.push(element);
                self.elementsHeights.push(element.height());
                self.needLayout = true;
            };
            this.removeItem = function (element) {
                var index = self.elements.indexOf(element);
                self.elements.splice(index, 1);
                self.elementsHeights.splice(index, 1);
                self.needLayout = true;
            };
        }
    };
});

app.directive('masonryItem', function () {
    return {
        restrict: 'A',
        require: '^masonry',
        link: function (scope, element, attr, ctrl) {
            ctrl.addItem(element);

            element.addClass('masonry-loading');

            scope.$on('$destroy', function () {
                ctrl.removeItem(element);
            });
        }
    };
});

app.directive('replies', function (RecursionHelper) {
    return {
        restrict: 'E',
        scope: { 'comments': '=' },
        replace: true,
        template: '<comment ng-repeat="reply in comments" comment="reply"></comment>',
        link: function (scope, element, attr) {
        },
        compile: function (element) {
            // Use the compile function from the RecursionHelper,
            // And return the linking function(s) which it returns
            return RecursionHelper.compile(element, function (scope, element, attr) { });
        }
    };
});

app.directive('comment', function (RecursionHelper) {
    return {
        restrict: 'E',
        scope: { 'comment': '=' },
        replace: true,
        template: '<div class="comment"><div class="user"><i class="fa fa-user"></i>{{comment.Comment.User.Name}} Says <span class="gray-text">{{comment.Comment.Created | timedelta}}</span>' +
            //'<button class="gray-hover gray-text">Reply</button>' +
            '</div>{{comment.Comment.Message}}<replies class="replies" comments="comment.Replies"></replies></div>',
        link: function (scope, element, attr) {
        },
        compile: function (element) {
            // Use the compile function from the RecursionHelper,
            // And return the linking function(s) which it returns
            return RecursionHelper.compile(element, function (scope, element, attr) { });
        }
    };
});


app.filter('ellipsis', function () {
    return function (str, length) {
        //if no string or no length
        if (str == undefined || str.length == undefined || !length) {
            return str;
        }

        //string is shorter than given length limit
        if (str.length < length) {
            return str;
        }

        //find where to truncate the string:
        // 1) the last space before the position we want to truncate.
        // 2) if we can't find any - truncate to the given length.
        var placeToTruncate = str.lastIndexOf(' ', length);
        if (placeToTruncate === -1) {
            placeToTruncate = length;
        }

        var limitedStr = str.substring(0, placeToTruncate).trim();
        return limitedStr + "...";
    };
});

app.filter('area', function ($filter) {
    var number = $filter('number');
    return function (area) {
        if (area < 30000) {
            return number(area, 0) + ' [m\u00B2]';
        } else if (area < 3000000) {
            return number(area / 10000, 0) + ' [ha]';
        } else {
            return number(area / 1000000, 0) + ' [km\u00B2]';
        }
    };
});

app.filter('datasize', function ($filter) {
    var number = $filter('number');

    return function bytesToSize(bytes) {
        if (bytes === 0) return '0 Bytes';
        var k = 1024;
        var sizes = ['Bytes', 'KB', 'MB', 'GB', 'TB', 'PB', 'EB', 'ZB', 'YB'];
        var i = Math.floor(Math.log(bytes) / Math.log(k));
        return number(bytes / Math.pow(k, i), 0) + ' ' + sizes[i];
    };
});

app.filter('escapehtml', ['$sce', function ($sce) {
    return function(htmlCode) {
        return $sce.trustAsHtml(htmlCode);
    };
}]);

app.filter('timedelta', function ($filter) {
    return function (date) {
        date = new Date(date);
        var now = new Date();
        var minutes = (now.getTime() - date.getTime()) / 1000 / 60;
        if (minutes < 1) {
            return "Now";
        }
        if (minutes < 2) {
            return "1 minute ago";
        }
        if (minutes < 60) {
            return Math.floor(minutes) + " minutes ago";
        }
        var hours = minutes / 60;
        if (hours < 2) {
            return "1 hour ago";
        }
        if (hours < 12) {
            return Math.floor(hours) + " hours ago";
        }
        var today = date.getDate() === now.getDate();
        if (today) {
            return "Today";
        }
        var yesterday = date.getDate() === (new Date(now.getTime() - 24 * 1000 * 60 * 60)).getDate();
        if (yesterday) {
            return "Yesterday";
        }
        return $filter('date')(date);
    };
});

//value format filter
//
// this filter uses number and date filters depend on the value type/
// it uses number filter if angular.isNumber(value) return true
// it uses date filter if angular.isDate(value) return true
// otherwise, it return the value with no change
app.filter('valueFormat', function ($filter) {
    var numberFilter = $filter('number');
    var dateFilter = $filter('date');

    return function (value) {
        if (angular.isNumber(value)) {
            return numberFilter(value);
        }
        if (angular.isDate(value)) {
            return dateFilter(value);
        }
        return value;
    };
});

app.filter('unitFormat', function () {
    return function (value) {
        if (value) {
            return "[" + value + "]";
        }
        return "";
    };
});

app.directive('formattedValue', function () {
    return {
        restrict: 'E',
        scope: {
            valueFn: '&value',
            unitFn: '&unit',
            maxLength: '='
        },
        replace: true,
        template: '<span><a ng-if="url" ng-href="{{url}}" target="_blank">{{value | valueFormat | ellipsis:maxLength}}</a><span ng-if="!url">{{value | valueFormat | ellipsis:maxLength}}</span> {{unit | unitFormat}}</span>',
        link: function (scope) {
            scope.$watch('unitFn()', function (unit) {
                scope.unit = unit;
            });
            scope.$watch('valueFn()', function (value) {
                scope.value = value;
                scope.url = undefined;
                if (angular.isString(value)) {
                    if (value.indexOf('http://') == 0 ||
                        value.indexOf('https://') == 0 ||
                        value.indexOf('file://') == 0 ||
                        value.indexOf('ftp://') == 0) {
                        scope.url = value;
                    }
                }
            });
        }
    };
});

app.service('keyboardBinder', function ($window) {
    return {
        bindKey: function (scope, element, callback) {
            function getBindingElement() {
                var tagName = element[0].tagName.toLowerCase();

                if (tagName === 'input' || tagName === 'textarea') {
                    return element;
                }
                return angular.element($window);
            }

            var bindingElement = getBindingElement();

            bindingElement.bind("keydown keypress", callback);

            scope.$on('$destroy', function () {
                bindingElement.off("keydown keypress", callback);
            });
        }
    };
});

app.directive('ngEnter', function (keyboardBinder) {
    return function (scope, element, attrs) {
        keyboardBinder.bindKey(scope, element, function (event) {
            if (event.which === 13) {
                scope.$apply(function () {
                    scope.$eval(attrs.ngEnter);
                });
                event.preventDefault();
            }
        });
    };
});

app.directive('ngEsc', function (keyboardBinder) {
    return function (scope, element, attrs) {
        keyboardBinder.bindKey(scope, element, function (event) {
            if (event.which === 27) {
                scope.$apply(function () {
                    scope.$eval(attrs.ngEsc);
                });
                event.preventDefault();
            }
        });
    };
});

app.directive('modalDialog', function () {
    return {
        restrict: 'E',
        replace: true,
        transclude: true,
        scope: {
            modalStyle: "="
        },
        template: '<div class="modal-dialog {{modalBackgroundClass}}"><scrollbar class="page-content"><div class="dialog-box {{modalClass}}" ng-style="modalStyle" ng-transclude></div></scrollbar></div>',
        link: function (scope, element, attrs) {
            scope.modalBackgroundClass = attrs["modalBackgroundClass"];
            scope.modalClass = attrs["modalClass"];
        }
    };
});

app.directive('modalDialogButtons', function () {
    return {
        restrict: 'E',
        replace: true,
        transclude: true,
        template: '<div><div style="float:right" ng-transclude></div><div style="clear:both"></div></div>'
    };
});

app.directive('inputTags', function () {
    return {
        restrict: 'E',
        scope: {
            ngModel: '='
        },
        require: "ngModel",
        replace: true,
        template: '\
<div class="form-control auto-height tags">\
    <span ng-repeat="tag in ngModel">{{tag}}<i class="fa fa-times" ng-click="removeTag($index)"></i></span>\
    <input class="new-tag" ng-model="newTag" ng-blur="addTag()" ng-keyup="handleKey($event)" ng-enter="addTag()"/>\
</div>',
        link: function (scope, elm, attrs, ngModel) {
            scope.newTag = '';
            scope.removeTag = function (index) {
                scope.ngModel.splice(index, 1);
            };
            scope.addTag = function () {
                if (scope.newTag) {
                    //split string into words and
                    angular.forEach(scope.newTag.split(' '), function (tag) {
                        //don't add empty tags
                        if (tag.length === 0) return;

                        //fix first letter
                        var fixedTag = angular.uppercase(tag[0]) + tag.substr(1);

                        //add if needed
                        if (scope.ngModel.indexOf(fixedTag) === -1) {
                            scope.ngModel.push(fixedTag);
                        }
                    });
                    scope.newTag = "";
                }
            };
            scope.handleKey = function ($event) {
                if ($event.keyCode === 32 || $event.keyCode === 13) {
                    $event.preventDefault();
                    scope.addTag();
                } else if ($event.keyCode === 27) {
                    scope.newTag = "";
                    $event.target.blur();
                }
            };
            if (attrs['minLength']) {
                var minLength = parseInt(attrs['minLength']);
                var checkMinLength = function () {
                    ngModel.$setValidity('minLength', scope.ngModel.length >= minLength);
                };
                scope.$watch('ngModel.length', checkMinLength);
                checkMinLength();
            }

            if (attrs['maxLength']) {
                var maxLength = parseInt(attrs['maxLength']);
                var checkMaxLength = function () {
                    ngModel.$setValidity('maxLength', scope.ngModel.length <= maxLength);
                };
                scope.$watch('ngModel.length', checkMaxLength);
                checkMaxLength();
            }
        }
    };
});

app.directive('inputPermission', function () {
    return {
        restrict: 'E',
        scope: {
            ngModel: '='
        },
        replace: true,
        template: '\
<div>\
<div>\
    <input type="radio" ng-model="ngModel" value="Public"><strong ng-click="ngModel=\'Public\'" style="cursor:default;">Public</strong> - Everyone has access to see and use the data.\
</div>\
<div>\
    <input type="radio" ng-model="ngModel" value="Private"><strong ng-click="ngModel=\'Private\'" style="cursor:default;">Private</strong> - Only you can see and use the data.\
</div>\
<\div>\
'
    };
});

app.directive('inputState', function () {
    return {
        restrict: 'E',
        scope: {
            ngModel: '='
        },
        replace: true,
        template: '\
<div>\
<div>\
    <input type="radio" ng-model="ngModel" value="Active"><strong ng-click="ngModel=\'Active\'" style="cursor:default;">Active</strong> - GeoSource can be subscribed to.\
</div>\
<div>\
    <input type="radio" ng-model="ngModel" value="Broken"><strong ng-click="ngModel=\'Broken\'" style="cursor:default;">Broken</strong> - GeoSource is undergoing maintenance and only administrators can subscribe to it.\
</div>\
<\div>\
'
    };
});

app.service('siteParams', function ($location, globalCookieStore) {
    var app = $location.search()['app'];
    var token = $location.search()['token'] ? JSON.parse($location.search()['token']) : undefined;
    var appName = 'wv';
    var downloadCookieName = 'pyx.download.app';
    var galleryUrl = $location.absUrl();
    if ($location.path() !== '/') {
        galleryUrl = galleryUrl.substr(0, galleryUrl.search($location.path()));
    }
    galleryUrl = galleryUrl.replace(/\/$/, '');
    return {
        getApp: function () {
            return app;
        },
        isWorldViewStudio: function () {
            return this.getApp() === appName;
        },
        getToken: function () {
            return token;
        },
        clearToken: function () {
            token = undefined;
        },
        downloadAppNeeded: function () {
            return !(this.isWorldViewStudio() || globalCookieStore.get(downloadCookieName) === appName);
        },
        setAppDownloaded: function () {
            globalCookieStore.put(downloadCookieName, appName, 1000);
        },
        galleryUrl: function() {
            return galleryUrl;
        }
    };
});

app.service('fileUploader', function ($http, $q) {
    return {
        upload: function (url, file, filename, contentType) {
            var fd = new FormData();
            fd.append(filename, file, filename);
            return $http.post(url, fd, {
                transformRequest: angular.identity,
                headers: { 'Content-Type': contentType }
            });
        },
        readAsDataURL: function (file) {
            var deferred = $q.defer();
            var reader = new FileReader();
            reader.onload = function (fileEvent) {
                deferred.resolve(fileEvent.target.result);
            };
            reader.onerror = function (fileEvent) {
                deferred.reject(fileEvent.target.error);
            };
            reader.readAsDataURL(file);
            return deferred.promise;
        }
    };
});

app.service('imageServer', function ($location) {
    var protocol = $location.protocol();
    var baseUrl = protocol + "://www.pyxisinnovation.com";

    return {
        getImageUrl: function (id, ext) {
            if (ext) {
                return baseUrl + "/images/pipelines/" + id + ext;
            } else {
                return baseUrl + "/images/pipelines/" + id;
            }
        },
        getImageThumbnailUrl: function (id, version) {
            return this.fixImageUrl(baseUrl + "/images/pipelines/thumbnails/" + id + ".jpg", version);
        },
        setImageUrl: function () {
            return baseUrl + "/data/catalogue/saveImage.php";
        },
        getAvatarUrl: function (id, ext) {
            if (ext) {
                return baseUrl + "/images/avatars/" + id + ext;
            } else {
                return baseUrl + "/images/avatars/" + id;
            }
        },
        getAvatarThumbnailUrl: function (id, version) {
            return this.fixImageUrl(baseUrl + "/images/avatars/thumbnails/" + id + ".jpg", version);
        },
        setAvatarUrl: function () {
            return baseUrl + "/data/catalogue/saveAvatar.php";
        },
        getBannerUrl: function (id, ext) {
            if (ext) {
                return baseUrl + "/images/galleries/" + id + ext;
            } else {
                return baseUrl + "/images/galleries/" + id;
            }
        },
        getBannerThumbnailUrl: function (id, version) {
            return this.fixImageUrl(baseUrl + "/images/galleries/thumbnails/" + id + ".jpg", version);
        },
        setBannerUrl: function () {
            return baseUrl + "/data/catalogue/saveBanner.php";
        },
        fixImageUrl: function (url, version) {
            var result = url;

            if (protocol === "https") {
                result = url.replace('http://', 'https://');
            }

            if (version) {
                //adding version to make sure that the browser cache is based on our resource version
                result += "?version=" + encodeURIComponent(version);
            }

            return result;
        }
    };
});

// Name: Src Error
//------------------------------------------------------
// Desc: Use this directive with 'ng-src' on the <img />
// element - if there's an error (404) loading it will
// fallback to the default image
// Use:
// - [View]  <img ng-src="{{getProviderImage(provider)}}" src-error="/assets/Images/avatar_default.gif" />

app.directive('srcError', function() {
    return {
        link: function(scope, element, attrs) {
            element.bind('error', function() {
                if (attrs.src !== attrs.srcError) {
                    if (attrs.srcError !== '') {
                        attrs.$set('src', attrs.srcError);
                    } else {
                        element.css('background-color', '#e4e4e4');
                    }
                }
            });
        }
    }
});

app.service('wvAlerts', function ($q, $rootScope) {
    var emit = function (message, scope, type) {
        if (scope) {
            scope.$emit('wv-alert', { message: message, type: type });
        } else {
            $rootScope.$broadcast('wv-alert', { message: message, type: type });
        }
    };
    return {
        error: function (message, scope) {
            emit(message, scope, 'error');
            return $q.reject(message);
        },
        success: function (message, scope) {
            emit(message, scope, 'success');
            return $q.when(message);
        },
        info: function (message, scope) {
            emit(message, scope, 'info');
            return $q.when(message);
        }
    };
});

// fileChange - allow angular to catch input[type=file] change events
//
// arguments:
//    file-change = function($files,$event) - will be called with the selected files and the orignal event trigger the input.onchange function
app.directive('fileChange', function ($parse) {
    return {
        restrict: 'A',
        link: function (scope, element, attrs) {
            var fn = $parse(attrs['fileChange']);

            element.bind('change', function (ev) {
                var files = ev.target.files;
                fn(scope, {
                    $files: files,
                    $event: ev
                });
            });
        }
    };
});

// imageDrop - allow angular to catch drag&drop file events
//
// arguments:
//    imageDrop = function($files,$event) - will be called with the selected files and the orignal event drop event
//
// this directive assign a css classes to the directive element when:
// * while drag happen - class 'dragover' is added to the element.
// * while drag ended/leave - class 'dragover' is removed from the element.
// * when drop happen - class 'drop' is added to the the elemnet.
//
app.directive('imageDrop', function ($parse) {
    return {
        restrict: 'A',
        link: function (scope, element, attrs) {
            var fn = $parse(attrs['imageDrop']);

            element.bind('dragover', function (ev) {
                ev.preventDefault();
                element.addClass('dragover');
            });
            element.bind('dragleave', function () {
                element.removeClass('dragover');
                return false;
            });
            element.bind('drop', function (ev) {
                ev.preventDefault();
                var files = ev.originalEvent.dataTransfer.files;
                //var reader = new FileReader();
                element.removeClass('dragover');
                element.addClass('drop');

                fn(scope, {
                    $files: files,
                    $event: ev
                });

                return false;
            });
        }
    };
});

// remote-validator - allow easy validation using promises.
//
// use case: a user name input field needs to be validated using an $http.get('isUserNameAvailble?name='+model.Name).
// example:
//
//    <input type="text" name="username" ng-model="values.name" remote-validator="checkName($value)" />
//
// controller function:
//
//     scope.checkName = function(name) { return $http.get('isUserNameAvailable?name='+name); }
//
// the validator would call the remote-validator function and would check if the promise has been resolved.
// If the promise was resolved, then the ngModel.$error.remote is set to be valid
// If the promise was rejected, then the ngModel.$error.remote is set to be invalid
//
// while the promise is been executed, ngModel.$error.remoteProgress is set to be invalid.
// if the value has changed during the promise exection, the removeValidator will invoke a new promise check with the new current update.
//
// arguments:
//
//    remote-validator = function($value) : promise
//                       the remote validator would invoke this function every time the value need to validated.
//
//    delay =            milliseconds :
//                       used to generate a delay before invoking the promise function
//
//    value-to-check   = expression : string
//                       used to check if the value has changed, can be used when the remote-validator function is based on complex model.
//                       example : "values.name + ',' + values.email"
//                       will cause the remote-validator function to be invoked when name or email has changed.
//
app.directive('remoteValidator', function ($q, $timeout, $parse) {
    return {
        restrict: 'A',
        require: 'ngModel',
        link: function (scope, elm, attrs, ngModel) {
            var delay = parseInt(attrs['delay'] || '0');
            var func = $parse(attrs['remoteValidator']);

            var validating = false;
            var delayedPromise = undefined;

            var delayedStart = function () {
                if (delayedPromise) {
                    $timeout.cancel(delayedPromise);
                }
                delayedPromise = $timeout(safeValidate, delay);
            };
            var getValue = attrs['valueToCheck'] ?
                $parse(attrs['valueToCheck']) :
                function () { return ngModel.$modelValue; };

            if (attrs['valueToCheck']) {
                //invoke value check when our custom value check function has changed...
                scope.$watch(function () { return getValue(scope); }, function () { delayedStart(); });
            }

            var safeValidate = function () {
                if (validating) {
                    return;
                }
                delayedPromise = undefined;
                validating = true;
                var currentValue = getValue(scope);
                $q.when(function () {
                    //we run the validating function inside the promise so it handle exceptions from func
                    try {
                        return func(scope, { '$value': currentValue });
                    } catch (error) {
                        return $q.reject(error);
                    }
                }()).
                    then(function (valid) {
                        validating = false;
                        if (currentValue == getValue(scope)) {
                            ngModel.$setValidity('remoteProgress', true);
                            ngModel.$setValidity('remote', valid);
                        } else {
                            delayedStart();
                        }
                    })
                    .catch(function (error) {
                        validating = false;
                        if (currentValue == getValue(scope)) {
                            ngModel.$setValidity('remoteProgress', true);
                            ngModel.$setValidity('remote', false);
                        } else {
                            delayedStart();
                        }
                    });
            };
            ngModel.$viewChangeListeners.unshift(function () {
                ngModel.$setValidity('remoteProgress', false);
                delayedStart();
            });
        }
    };
});

// match - allow to make sure 2 fields are the same.
//
// example:
//    <input type="password" name="password" ng-model="values.password" min-length="6" />
//    <input type="password" name="password2" ng-model="values.retypePassword" match="values.password" />
//
// the form.password2.$error.match - would make sure password2 has same value as password.
//
app.directive('match', function () {
    return {
        require: 'ngModel',
        restrict: 'A',
        scope: {
            match: '='
        },
        link: function (scope, elem, attrs, ngModel) {
            scope.$watch(function () {
                return (ngModel.$pristine && angular.isUndefined(ngModel.$modelValue)) || scope.match === ngModel.$modelValue;
            }, function (currentValue) {
                ngModel.$setValidity('match', currentValue);
            });
        }
    };
});


// IE9 placeholder support
//
// this directive would only run if placeholder is not supported by the DOM.
//
app.directive('placeholder', function () {
    if ('placeholder' in document.createElement('input')) {
        return {
            restrict: 'A',
            link: function (scope, elem, attrs) { }
        };
    }

    return {
        restrict: 'A',
        require: 'ngModel',
        link: function (scope, elem, attrs, ngModel) {
            var placeholder = $("<span class='placeholder' UNSELECTABLE='on'></span>");
            var offset = elem.position();
            placeholder.css({
                'position': 'absolute',
                'display': 'none',
                'left': offset.left + "px",
                'top': offset.top + "px"
            });

            placeholder.text(attrs['placeholder']);
            elem.parent().append(placeholder[0]);

            var visible = false;

            var setPlaceholder = function () {
                var shouldBeVisible = $(elem).val() === "";
                if (visible !== shouldBeVisible) {
                    if (shouldBeVisible) {
                        placeholder.css({ 'display': 'inline' });
                    } else {
                        placeholder.css({ 'display': 'none' });
                    }
                    visible = shouldBeVisible;
                }
            };
            setPlaceholder();

            var setFocus = function () {
                elem.focus();
            };
            scope.$watch(function () { return ngModel.$modelValue; }, setPlaceholder);

            elem.on('input focus keyup blur paste', setPlaceholder);
            placeholder.bind('click', setFocus);

            scope.$on('$destroy', function () {
                elem.off('input focus keyup blur paste', setPlaceholder);
                placeholder.off('click', setFocus);
            });
        }
    };
});

app.directive('scrollbar', function () {
    return {
        restrict: 'E',
        replace: true,
        transclude: true,
        template: '<div class="scrollbar-area"><span class="scrollbar-widget-back"></span><scrollbar-widget></scrollbar-widget><div class="scrollbar-content" ng-transclude></div></div>',
        controller: function ($element) {
            this.element = $element;
            this.content = angular.element($element.find('div')[1]);

            var callbacks = [];
            this.onScroll = function (scopeId, f) { callbacks.push({ scopeId: scopeId, f: f })};
            this.invokeOnScroll = function (scrollTop) {
                angular.forEach(callbacks, function (c) { c.f(scrollTop) });
            };
            // Unregister any callback previously registered to scopeId
            this.unregister = function(scopeId) {
                callbacks = callbacks.filter(function(c) {
                    return c.scopeId !== scopeId;
                });
        }
        }
    };
});

app.directive('scrollbarWidget', function ($timeout, $window) {
    return {
        restrict: 'E',
        replace: true,
        require: '^scrollbar',
        template: '<div class="widget"></div>',
        link: function (scope, elem, attrs, scrollbar) {
            var parent = scrollbar.element;
            var content = scrollbar.content;
            var recoverPosition = false;
            var scrollAlive = false;

            var recalculate = function () {
                if (scrollAlive && !elem.is(':visible')) {
                    //when item become not visible - it scrollTop is set to 0 by the browser.
                    //we mark that we need to recover the old scrollTop when element become visible once again
                    recoverPosition = true;
                    return;
                }
                var ratio = content.height() / content[0].scrollHeight;
                var top = content[0].scrollTop / content[0].scrollHeight;

                if (ratio < 0.99) {
                    scrollAlive = true;

                    if (recoverPosition) {
                        //we reverse the logic...
                        //recover scrollTop from the elem css top attribute.
                        top = elem.position().top / content.height();
                        content[0].scrollTop = top * content[0].scrollHeight;
                        recoverPosition = false;
                    }

                    parent.addClass('active');
                    elem.css('height', (content.height() * ratio) + 'px');
                    elem.css('top', (content.height() * top) + 'px');
                } else {
                    scrollAlive = false;
                    parent.removeClass('active');
                }
                scrollbar.invokeOnScroll(content[0].scrollTop);
            };
            recalculate();

            var startY;
            var startTop;
            var win = angular.element($window);

            var mousemove = function (event) {
                var currentY = event.clientY;
                var ratio = content.height() / content[0].scrollHeight;
                var newTop = startTop + (currentY - startY) / ratio;
                content[0].scrollTop = newTop;
                recalculate();
                event.preventDefault();
            };
            var mouseup = function (event) {
                parent.removeClass('scrolling');
                win.off('mousemove', mousemove);
                win.off('mouseup', mouseup);
                event.preventDefault();
            };
            var mousedown = function (event)
            {
                parent.addClass('scrolling');
                startY = event.clientY;
                startTop = content[0].scrollTop;
                win.bind('mousemove', mousemove);
                win.bind('mouseup', mouseup);
                event.preventDefault();
            };
            var mousewheel = function (event) {
                if (event.originalEvent.wheelDelta < 0) {
                    content[0].scrollTop += 40;
                } else {
                    content[0].scrollTop -= 40;
                }
                event.stopPropagation();
                recalculate();
            };
            var keyup = function (event) {
                if (event.target.nodeName === "INPUT" || event.target.nodeName === "TEXTBOX") {
                    return;
                }
                if (event.keyCode === 33) { //pageup
                    content[0].scrollTop -= content.height();
                    recalculate();
                }
                if (event.keyCode === 34) { //pagedown
                    content[0].scrollTop += content.height();
                    recalculate();
                }
            };
            var mouseenter = function () {
                win.bind('keyup', keyup);
            };
            var mouseleave = function () {
                win.off('keyup', keyup);
            };
            parent.bind('mousewheel', mousewheel);
            parent.bind('mouseenter', mouseenter);
            parent.bind('mouseleave', mouseleave);


            elem.bind('mousedown', mousedown);

            scope.$on('$destroy', function () {
                parent.off('mousewheel', mousewheel);
                parent.off('mouseenter', mouseenter);
                parent.off('mouseleave', mouseleave);

                elem.off('mousedown', mousedown);
            });

            scope.$watch(function () { return content.height(); }, function () { recalculate(); });
            scope.$watch(function () { return content[0].scrollHeight; }, function () { recalculate(); });
            scope.$watch(function () { return content[0].scrollTop; }, function () { recalculate(); });
        }
    };
});

app.service('inputValidator', function () {
    var validators = {
        validateUserName: function (userName) {
            return userName && userName.length >= 5;
        },
        validateEmail: function (email) {
            var re = /^(([^<>()[\]\\.,;:\s@\"]+(\.[^<>()[\]\\.,;:\s@\"]+)*)|(\".+\"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
            return email && re.test(email);
        },
        validateUrl: function (url) {
            var pattern = new RegExp("^(http[s]?:\\/\\/(www\\.)?){1}([0-9A-Za-z-\\.@:%_\+~#=]+)+((\\.[a-zA-Z]{2,3})+)(/(.)*)?(\\?(.)*)?", "i");
            return url && pattern.test(url);
        }
    };
    return validators;
});

app.service('accountServices', function ($q, $pyxconfig, $timeout, $http, inputValidator) {
    function checkAvailable(field, value, validate) {
        var deferred = $q.defer();
        if (!validate(value)) {
            deferred.reject(false);
        } else {
            $http.get($pyxconfig.baseUrl + '/Account/Available?' + field + '=' + encodeURIComponent(value))
                .success(function () {
                    deferred.resolve(true);
                })
                .error(function () {
                    deferred.reject(false);
                });
        }
        return deferred.promise;
    }

    var accountServicesObject = {
        checkUserNameNotAvailable: function (value) {
            var deferred = $q.defer();
            if (!inputValidator.validateUserName(value)) {
                deferred.reject(false);
            } else {
                $http.get($pyxconfig.baseUrl + '/Account/Available?UserName=' + encodeURIComponent(value))
                .success(function () {
                    deferred.reject(false);
                })
                .error(function () {
                    deferred.resolve(true);
                });
            }
            return deferred.promise;
        },
        checkUserNameAvailable: function (value) {
            return checkAvailable('UserName', value, inputValidator.validateUserName);
        },
        checkEmailAvailable: function (value) {
            return checkAvailable('Email', value, inputValidator.validateEmail);
        }
    };
    return accountServicesObject;
});

app.service('termsService', function ($pyx, $q, $timeout) {
    var terms;
    var promise;
    var waiting = false;
    var found = false;
    var termsObject = {
        hasTerms: function () {
            return found;
        },
        getTerms: function () {
            if (waiting) {
                return promise;
            }
            if (terms) {
                promise = $q.when(terms);
                return promise;
            }
            promise = $pyx.gallery.terms().then(function (payload) {
                found = true;
                terms = payload.data.Text;
                return terms;
            },
                function () {
                    waiting = true;
                    promise = $timeout(termsObject.resetGetTerms, 10000);
                    return promise;
                });
            waiting = true;
            return promise;
        },
        resetGetTerms: function () {
            waiting = false;
            termsObject.getTerms();
        }
    };
    return termsObject;
});

app.filter('toParagraph', function () {
    return function (text) {
        return "<p>" + text.replace(/(\r\n|\n|\r)/gm, "</p><p>") + "</p>";
    };
});

app.filter('toFormattedParagraph', ['toParagraphFilter', function (toParagraph) {
    return function (text) {
        var formattedText = text
            .replace(/(^|[^\.])([1-9]+\.\t.*)/g, '<b>$2</b>')
            .replace(/(^|[^\.])([1-9]+\.[1-9]+\.\t.*)/g, '<i>$2</i>')
            .replace(/(https?:\/\/((?![,:;\.]\s)[^\s]+\w))/g, '<a href="$1">$1</a>');
        return toParagraph(formattedText);
    };
}]);

// terms
// -------------
// Display the PYXIS terms of service
app.directive('terms', ['termsService', 'toFormattedParagraphFilter', function (termsService, toFormattedParagraph) {
    var termsText = "Waiting for license terms...";
    return {
        restrict: 'E',
        link: function (scope, element) {
            var alive = true;
            var getTerms = function () {
                termsService.getTerms().then(function (t) {
                    scope.termsText = t;
                    element.html(toFormattedParagraph(scope.termsText));
                }, function (error) {
                    if (alive) {
                        $timeout(function () { getTerms(); }, 1000);
                    }
                });
            };
            scope.$on('$destroy', function () {
                alive = false;
            });

            element.html(toFormattedParagraph(termsText));
            getTerms();
        }
    };
}]);

// license-agreement
// -----------------
// Present the terms of a specified License and allow the user to agree or disagree.
// The user's decision will be recorded.
// usage: <license-agreement license-id="licenseId" on-accept="acceptCallback()" on-decline="declineCallback()" />
app.directive('licenseAgreement', ['$pyx', '$timeout', 'toFormattedParagraphFilter', function ($pyx, $timeout, toFormattedParagraph) {
    return {
        restrict: 'E',
        scope: {
            licenseId: '=',
            onAgree: '&',
            onDisagree: '&'
        },
        templateUrl: '/client/templates/wv-license-agreement.html',
        link: function (scope, element) {
            scope.license = {
                Terms: { Text: "Waiting for license terms..." }
            }
            scope.submitting = false;

            var alive = true;

            var displayText = function() {
                $('.license-content').html(toFormattedParagraph(scope.license.Terms.Text));
            }

            var getLicense = function () {
                $pyx.gallery.licenses().getById(scope.licenseId).then(function (payload) {
                    scope.license = payload.data;
                    displayText();
                }, function (error) {
                    if (alive) {
                        $timeout(function () { getLicense(); }, 3000);
                    }
                });
            };

            scope.recordDecision = function (decision) {
                scope.submitting = true;
                scope.errorMessage = "";
                $pyx.gallery.recordLicenseAgreement(scope.license, decision).then(function() {
                    alive = false;
                    scope.submitting = false;
                    if (decision === 'Agree') {
                        scope.onAgree();
                    } else {
                        scope.onDisagree();
                    }
                }, function (error) {
                    scope.submitting = false;
                    // if decision was already recorded a Not Modified response is returned
                    if (error.status === 304) {
                        if (decision === 'Agree') {
                            scope.onAgree();
                        } else {
                            scope.onDisagree();
                        }
                    } else {
                        scope.errorMessage = "Unable to record your decision.  Please try again.";
                    }
                });
            }

            scope.$on('$destroy', function () {
                alive = false;
            });

            displayText();
            getLicense();
        }
    };
}]);

// follow mouse drag
// -----------------
//
// it is not simple creating a directive that tracks a mouse down-move-up when the mouse is move out side of the element.
// In order to make this function, the mousedown event need to register mousemove and mouseup event handlers on the
// window object. A good practice is also to unbind the window events once they are no longer needed.
//
// This directive does make it easy to follow a mouse drag events.
//
// usage:
//   <div follow-mouse-drag="handleEvents($event)">I will follow you<div>
//
// the assigned callback will receive a $event (which is not the normal $event):
//
//  $event.state = "mousedown" | "mousemove" | "mouseup"
//  $event.originalEvent - pointer to the angular $event that started the follow (mousedown)
//  $event.currentEvent - pointer to the current angular $event that was invoked (mousemove,mouseup)
//  $event.delta = { left: int, top: int } - the mouse x,y delta in pixels
//
app.directive('followMouseDrag', function ($window,$parse) {
    return {
        restrict: 'A',
        link: function (scope, element, attr) {
            var win = angular.element($window);
            var fn = $parse(attr.followMouseDrag);

            var followEvent = {
                state: undefined,
                orignialEvent: undefined,
                currentEvent: undefined,
                delta: { left: 0, top: 0}
            };

            function notify(eventType,event) {
                followEvent.currentEvent = event;
                followEvent.state = eventType;
                followEvent.delta.left = followEvent.currentEvent.clientX - followEvent.orignialEvent.clientX;
                followEvent.delta.top = followEvent.currentEvent.clientY - followEvent.orignialEvent.clientY;
                fn(scope, { $event: followEvent });
            }

            function followMouse($event) {
                notify("mousemove", $event);
                return false;
            }

            function stopFollow($event) {
                win.off('mousemove', followMouse);
                win.off('mouseup', stopFollow);
                notify("mousedown", $event);
                return false;
            }

            function startFollow($event) {
                win.bind('mousemove', followMouse);
                win.bind('mouseup', stopFollow);
                followEvent.orignialEvent = $event;
                notify("mousedown", $event);
                return false;
            }

            element.bind('mousedown', startFollow);

            scope.$on('$destroy', function () {
                element.off('mousedown', startFollow);
            });
        }
    }
});

// range-selector
// --------------
//
// allow the user to create a range selection
//
// usage:
//   <range-selector min-range="0" max-range="4500" min="range.min" max="range.max"></range-selector>
//   <div>{{range.min}} - {{range.max}}</div>
//
// require: css classes: range-selector, range-selector>range, range-selector>selector
//
app.directive('rangeSelector', function () {
    return {
        restrict: 'E',
        scope: {
            minRange: "=",
            maxRange: "=",
            min: "=",
            max: "="
        },
        replace: true,
        template: '<div class="range-selector"><div class="range"></div><div class="selector" follow-mouse-drag="mouseDrag($event)"><div class="left-handle"></div><div class="right-handle"></div></div></div>',
        link: function (scope, element) {
            var edgeSize = 10; //in pixels

            var width = 0;
            var pixelSize = 0;
            var original = 0;
            var update = undefined;

            var selector = element.find('.selector');

            scope.mouseDrag = function (event) {

                if (event.state === "mousedown") {
                    width = element.width();
                    var offset = selector.offset();
                    var minX = offset.left;
                    var maxX = offset.left + selector.width();

                    pixelSize = (scope.maxRange - scope.minRange) / Math.max(1,width);

                    if (Math.abs(event.currentEvent.clientX - minX) < edgeSize) {
                        original = scope.min;
                        update = function (e) {
                            scope.min = original + e.delta.left * pixelSize;
                            if (scope.min > scope.max) {
                                scope.min = scope.max - pixelSize;
                            }
                            if (scope.min < scope.minRange) {
                                scope.min = scope.minRange;
                            }
                        }
                    } else if (Math.abs(event.currentEvent.clientX - maxX) < edgeSize) {
                        original = scope.max;
                        update = function (e) {
                            scope.max = original + e.delta.left * pixelSize;
                            if (scope.max < scope.min) {
                                scope.max = scope.min + pixelSize;
                            }
                            if (scope.max > scope.maxRange) {
                                scope.max = scope.maxRange;
                            }
                        }
                    } else {
                        original = [scope.min,scope.max];
                        update = function (e) {
                            scope.min = original[0] + e.delta.left * pixelSize;
                            scope.max = original[1] + e.delta.left * pixelSize;

                            if (scope.min < scope.minRange) {
                                scope.min = scope.minRange;
                                scope.max = scope.min + original[1] - original[0];
                            }

                            if (scope.max > scope.maxRange) {
                                scope.max = scope.maxRange;
                                scope.min = scope.max - (original[1] - original[0]);
                            }
                        }
                    }
                } else {
                    scope.$apply(function () {
                        update(event);
                    });
                }
            }

            function valueToPercent(value) {
                return (100 * (value - scope.minRange) / (scope.maxRange - scope.minRange));
            }

            function updateSelector() {
                selector.css({
                    left: valueToPercent(scope.min) + "%",
                    right: (100 - valueToPercent(scope.max)) + "%"
                });
            };

            scope.$watch(function () { return valueToPercent(scope.min); }, updateSelector);
            scope.$watch(function () { return valueToPercent(scope.max); }, updateSelector);
        }
    }
});


// [theme related directive]
// fade in background directive
//
// example:
//  <div style="opacity:0" fade-in-background="model.imgUrl" target-opacity="1"></div>
//
// this directive preloads the image and then sets the following css attributes:
//   background-image: url
//   opacity: attr[target-opacity] (default 1)
//
app.directive('fadeInBackground', function ($timeout) {
    var imageSwitchDelay = 250; //milliseconds
    return {
        restrict: 'A',
        link: function (scope, elem, attrs) {
            var sourceOpacity = elem.css('opacity');
            var targetOpacity = attrs.targetOpacity || 1.0;
            var lastValue = undefined;
            var currentPromise = undefined;
            var imageReady = false;

            scope.$watch(attrs.fadeInBackground, function (value) {
                imageReady = false;
                lastValue = value;


                function replaceImage() {
                    if (imageReady) {
                        if (lastValue === value) {
                            elem.css({
                                'background-image': 'url(' + value + ')',
                                'opacity': targetOpacity
                            });
                        }
                    }
                }

                //start hiding current element
                elem.css({
                    'opacity': sourceOpacity
                });

                //start switch delay
                if (currentPromise) {
                    $timeout.cancel(currentPromise);
                    currentPromise = null;
                }
                currentPromise = $timeout(function () {
                    replaceImage();
                    currentPromise = null;
                }, imageSwitchDelay);

                //start loading image
                var imageObj = new Image();
                imageObj.onload = function () {
                    if (lastValue === value) {
                        imageReady = true;
                        if (!currentPromise) {
                            replaceImage();
                        }
                    }
                };
                imageObj.src = value;
            });
        }
    };
});

//pop-up window directive.
//----------------------
//goal: allow to set a pop-up window.
//require: css classes:
//  popup-window,
//  popup-window-content + (up,down,left,right)
//  popup-window-arrow  + (up,down,left,right)
//example:
//
// <div style="position:fixed;top:100px;left:200px">
//   <popup-window arrow="top">
//     <div>content</div>
//   </popup-window>
// </div>
app.directive('popupWindow', function ($timeout, positionHelper) {
    return {
        restrict: 'E',
        scope: {
        },
        replace: true,
        transclude: true,
        template: '<div class="popup-window"><div class="popup-window-content {{arrow}}"><div class="popup-window-arrow {{arrow}}"></div><div ng-transclude></div></div></div>',
        link: function (scope, element, attr) {
            scope.defaultArrow = attr['arrow'] || 'top';
            scope.arrow = scope.defaultArrow;

            var placePopup = function () {
                var content = element.children(0);
                var contentPosition = positionHelper.positionBasedOn(element, content, scope.defaultArrow, 20);

                scope.arrow = contentPosition.arrow;
                content.offset({ 'left': contentPosition.left, 'top': contentPosition.top });

                var arrow = content.children(0);
                arrow.css({ 'left': contentPosition.arrowOffset.left + 'px', 'top': contentPosition.arrowOffset.top + 'px' });

                element.removeClass('not-ready');
            }

            element.addClass('not-ready');

            $timeout(placePopup, 10);
        }
    }
});

//popup-template-singleton service.
//----------------------
//this service allow popup templates to act as a singleton.
//
//usage:
//  var group = popupTemplateSingleton.get('menus');
//
//  group.setCurrentPopup(popup) -  mark the given popup as the singleton in that group.
//                                  if previous singleton exists, it would be hidden.
//
//  group.popupWasHidden(popup) -   mark that the given popup was hidden. this clears
//                                  the current singleton if needed.
app.service('popupTemplateSingleton', function () {
    var groups = {};

    return {
        get: function (name) {
            if (!(name in groups)) {
                groups[name] = {
                    current: null,
                    setCurrentPopup: function (popup) {
                        if (this.current) {
                            this.current.hide();
                        }
                        this.current = popup;
                    },
                    popupWasHidden: function (popup) {
                        if (this.current === popup) {
                            this.current = null;
                        }
                    }
                };
            }
            return groups[name];
        }
    }
});

//popup-template-linker service.
//----------------------
//this service allow to create a linking function for a tooltip.
//
//usage:
//  var linkFn = popupTemplateLinker.compile(settings);
//
//  the linkFn can be used a link function for a directive.
//
// must settings:
//   settings.content - html to put inside the tooltip. this would be part of the template used to generate the tooltip
//   settings.onPopupCreate(scope, element, attr, ctrl, popup, popupLinkedScope) - will be called when popup element has been created
//   settings.onPopupRemove(scope, element, attr, ctrl, popup, popupLinkedScope) - will be called when popup element has been removed
// optional settings:
//   settings.css - class to mark the popup-window element. default: '',
//   settings.placement - default placement of the tooltip. will be override using attr['placement']
//   settings.popupGroup - default group. will be override using attr['popupGroup']
//   settings.showTrigger - default show trigger. will be override using attr['showTrigger']
//   settings.showDelay - default show delay. will be override using attr['showDelay']
//   settings.hideTrigger - default hide trigger. will be override using attr['hideTrigger']
//   settings.hideDelay - default hide delay. will be override using attr['hideDelay']
//   settings.placementBasedOnMouse - place the popup based on the current mouse position and not based on element position.
//
app.service('popupTemplateLinker', function ($timeout, $compile, $document, $parse, positionHelper, popupTemplateSingleton) {
    return {
        compile: function (compileSettings) {
            return function (scope, element, attr, ctrl) {
                var popupScope = scope.$new();

                compileSettings.css = compileSettings.css || '';

                var template = '<div class="popup-window ' + compileSettings.css + '"><div class="popup-window-content {{$popup.arrow}}"><div class="popup-window-arrow {{$popup.arrow}}"></div>' + compileSettings.content + '</div></div>';
                var popupLinker = $compile(template);
                var foundNgIncludeInContent = compileSettings.content.indexOf("ng-include") != -1;

                var popup;
                var popupLinkedScope;

                var appendToBody = element[0].tagName.toLowerCase() === "input" || attr['appendToBody'];

                var onShow = $parse(attr['onShow']);
                var onHide = $parse(attr['onHide']);

                var showTrigger = attr['showTrigger'] || compileSettings.showTrigger || 'mouseenter';
                var showDelay = parseInt(attr['showDelay'] || compileSettings.showDelay || 0);

                var hideTrigger = attr['hideTrigger'] || compileSettings.hideTrigger || 'mouseleave';
                var hideDelay = parseInt(attr['hideDelay'] || compileSettings.hideDelay || 500);

                var showPromise;
                var hidePromise;

                var placementSettings = {
                    'topOffset': parseInt(attr['popupTopOffset'] || '0'),
                    'leftOffset': parseInt(attr['popupLeftOffset'] || '0')
                };

                var popupGroup = attr['popupGroup'] || compileSettings.popupGroup;
                var padding = parseInt(attr['padding'] || compileSettings.padding || 10);

                var mouseElement = {
                    left: 0,
                    right: 0,
                    offset: function () { return { left: this.left, top: this.top } },
                    width: function () { return 16; },
                    height: function () { return 16; }
                };

                function updateMouseElement(event) {
                    if (event && event.pageX && event.pageY) {
                        mouseElement.left = event.pageX;
                        mouseElement.top = event.pageY;
                    }
                }

                var popupOperations = {
                    createPopup: function () {
                        // There can only be one popup element per directive shown at once.
                        if (popup) {
                            popupOperations.removePopup();
                        }
                        popupLinkedScope = popupScope.$new();
                        popupLinkedScope.$popup = {
                            active: false,
                            placement: attr['placement'] || compileSettings.placement || 'top',
                            show: function () { $timeout(popupOperations.show) },
                            hide: function () { $timeout(popupOperations.hide) }
                        }

                        popupLinkedScope.$popup.arrow = popupLinkedScope.$popup.defaultArrow;

                        popup = popupLinker(popupLinkedScope, function (clonePopup) {
                            if (appendToBody) {
                                $document.find('body').append(clonePopup);
                                clonePopup.css({'position': 'fixed', 'z-index': 10000 });
                            } else {
                                element.append(clonePopup);
                            }
                        });

                        if (!popupLinkedScope.$$phase) {
                            popupLinkedScope.$digest();
                        }

                        if (compileSettings.onPopupCreate) {
                            compileSettings.onPopupCreate(scope, element, attr, ctrl, popup, popupLinkedScope);
                        }

                    },

                    removePopup: function () {
                        if (compileSettings.onPopupRemove) {
                            compileSettings.onPopupRemove(scope, element, attr, ctrl, popup, popupLinkedScope);
                        }

                        if (popup) {
                            if (appendToBody && !compileSettings.placementBasedOnMouse && hideTrigger === "mouseleave") {
                                var content = popup.children(0);
                                content.off('mouseleave', popupOperations.beginHide);
                                content.off('mouseenter', popupOperations.cancelHide);
                            }

                            popup.remove();
                            popup = null;
                        }
                        if (popupLinkedScope) {
                            popupLinkedScope.$destroy();
                            popupLinkedScope = null;
                        }
                    },

                    placePopup: function () {
                        if (!popup) {
                            return;
                        }
                        var content = popup.children(0);

                        //contiune to reposition until ng-include able to render it self
                        if (foundNgIncludeInContent && content.find("[ng-include]").length == 0)
                        {
                            $timeout(popupOperations.placePopup, 1);
                            return;
                        }

                        var contentPosition = positionHelper.positionBasedOn(compileSettings.placementBasedOnMouse ? mouseElement : element, content, popupLinkedScope.$popup.placement, padding, placementSettings);

                        popupLinkedScope.$popup.arrow = contentPosition.arrow;

                        content.offset({ 'left': contentPosition.left, 'top': contentPosition.top });

                        var arrow = content.children(0);
                        arrow.css({ 'left': contentPosition.arrowOffset.left + 'px', 'top': contentPosition.arrowOffset.top + 'px' });

                        popup.removeClass('not-ready');

                        //if we want a popup menu that will be added to body - we need a speical case for hideTrigger===mouseleave
                        //because the popup is no longer a "child" of element- we need to cancelHide/beginHide on the popup window it self
                        if (appendToBody && !compileSettings.placementBasedOnMouse && hideTrigger === "mouseleave") {
                            content.bind('mouseleave', popupOperations.beginHide);
                            content.bind('mouseenter', popupOperations.cancelHide);
                        }
                    },


                    show: function () {
                        showPromise = null;
                        if (!popup && popupScope != null) {
                            popupOperations.createPopup();
                            popup.addClass('not-ready');

                            $timeout(function () {
                                if (popupLinkedScope) {
                                    onShow(popupLinkedScope, { $popup: popupLinkedScope.$popup });
                                    if (popupGroup) {
                                        popupTemplateSingleton.get(popupGroup).setCurrentPopup(popupLinkedScope.$popup);
                                    }
                                }
                            });
                            $timeout(popupOperations.placePopup, 1);
                        }
                    },

                    hide: function () {
                        hidePromise = null;
                        if (popup && popupScope != null) {
                            onHide(popupLinkedScope, { $popup: popupLinkedScope.$popup });
                            if (popupGroup) {
                                popupTemplateSingleton.get(popupGroup).popupWasHidden(popupLinkedScope.$popup);
                            }

                            popupOperations.removePopup();
                        }
                    },

                    beginShow: function (event) {
                        updateMouseElement(event);
                        showPromise = $timeout(popupOperations.show, showDelay);
                        popupOperations.cancelHide();
                    },

                    cancelShow: function () {
                        if (showPromise) {
                            $timeout.cancel(showPromise);
                            showPromise = null;
                        }
                    },

                    beginHide: function () {
                        hidePromise = $timeout(popupOperations.hide, hideDelay);
                        popupOperations.cancelShow();
                    },

                    cancelHide: function () {
                        if (hidePromise) {
                            $timeout.cancel(hidePromise);
                            hidePromise = null;
                        }
                    }
                };

                if (showTrigger === 'click') {
                    element.bind('click', popupOperations.show);
                } else if(showTrigger === 'rightClick') {
                    element.bind('contextmenu', popupOperations.beginShow);
                } else {
                    element.bind('mouseenter', popupOperations.beginShow);
                }

                if (compileSettings.placementBasedOnMouse) {
                    element.bind('mousemove', updateMouseElement);
                }

                if (hideTrigger === 'mouseleave') {
                    element.bind('mouseleave', popupOperations.beginHide);
                    element.bind('mouseenter', popupOperations.cancelHide);
                }

                scope.$on('$destroy', function () {
                    if (showTrigger === 'click') {
                        element.off('click', popupOperations.show);
                    } else if(showTrigger === 'rightClick') {
                        element.off('contextmenu', popupOperations.beginShow);
                    } else {
                        element.off('mouseenter', popupOperations.beginShow);
                    }
                    if (compileSettings.placementBasedOnMouse) {
                        element.off('mousemove', updateMouseElement);
                    }
                    if (hideTrigger === 'mouseleave') {
                        element.off('mouseleave', popupOperations.beginHide);
                        element.off('mouseenter', popupOperations.cancelHide);
                    }
                    if (popup) {
                        popupOperations.hide();
                    }
                    if (popupScope) {
                        popupScope.$destroy();
                    }
                    popupScope = null;
                });

                if (showTrigger === 'auto') {
                    popupOperations.beginShow();
                }
            }
        }
    };
});

//menu item directive.
//--------------------
//
// when menu-item class is used - click event would hide the
// active popup if exists.
app.directive('menuItem', function () {
    return {
        restrict: 'C',
        link: function (scope, element) {
            var hideFn = function () {
                if (scope.$popup) {

                    if (element.hasClass('gray') || element.hasClass('sub-menu')) {
                        return;
                    }
                    scope.$popup.hide();
                }
            };
            element.bind('click', hideFn);

            scope.$on('$destroy', function () {
                element.off('click', hideFn);
            });
        }
    };
});


//popup-template window directive.
//----------------------
//goal: allow to set a pop-up window.
//require: css classes:
//  popup-window, popup-window.not-ready
//  popup-window-content + (up,down,left,right)
//  popup-window-arrow  + (up,down,left,right)
//example:
//
// <div popup-template="/popup-menu/my-template.html" placement="top">I have a popup template</div>
//
//attributes:
//  popup-template - string for the template to be used to the content of the popup
//  placement      - [left|right|top|bottom] + [|center] - where to place the window. example:
//                   left - place the popup on the left side of the element
//                   bottom center - place the popup at the bottom center of the element
//  on-show        - expression to invoke when popup is shown
//  on-hide        - expression to invoke when popup is hidden
//  hide-trigger   - never | mouseleave (default)
//  hide-delay     - int milliseconds (default:500)
//  show-trigger   - auto | click | rightClick | mouseenter (default)
//  show-delay     - int milliseconds (default:0)
//  popup-group    - make sure only 1 popup is visible per group.
//  popup-enabled  - evalaute this expression to determined if to link the popup to item (only on ng-init)
app.directive('popupTemplate', function (popupTemplateLinker) {

    var linkFn = popupTemplateLinker.compile({
        content: '<div ng-include src="$popup.template"></div>',
        onPopupCreate: function (scope, element, attr, ctrl, popup, popupScope) {
            popupScope.$popup.template = attr['popupTemplate'];
        }
    });

    return {
        restrict: 'A',
        link: function (scope, element, attr, ctrl) {
            //check if popup is enabled before we start
            if (attr["popupEnabled"] && !scope.$eval(attr["popupEnabled"])) {
                return;
            }
            linkFn(scope, element, attr, ctrl);
        }
    }
});

app.directive('tooltip', function (popupTemplateLinker) {
    var linkFn = popupTemplateLinker.compile({
        css: 'tooltip',
        content: '<div>{{tooltip}}</div>',
        placementBasedOnMouse: true,
        placement: 'bottom left',
        popupGroup: 'tooltip',
        showDelay: 1000,
        hideDelay: 10,
        showTrigger: 'mouseenter'
    });

    return {
        restrict: 'A',
        scope: {
            tooltip: '@'
        },
        link: function (scope, element, attr, ctrl) {
            linkFn(scope, element, attr, ctrl);
        }
    }
});


//ellipsis tooltip directive.
//----------------------
//goal: allow to set a tooltip that is visible if ellipsis overflow is active.
//require: css classes:
//  popup-window, popup-window.not-ready
//  popup-window-content + (top,bottom,left,right)
//  popup-window-arrow  + (top,bottom,left,right)
//example:
//
// <div ellipsis-tooltip="tool tip" placement="top">I have a popup template</div>
//
//attributes:
//  ellipsis-tooltip - content of the tooltip
//  placement      - [left|right|top|bottom] + [|center] - where to place the window. example:
//                   left - place the popup on the left side of the element
//                   bottom center - place the popup at the bottom center of the element
app.directive('ellipsisTooltip', function (popupTemplateLinker, $timeout) {

    var linkFn = popupTemplateLinker.compile({
        css: 'tooltip',
        content: '<div>{{ellipsisTooltip}}</div>',
        placementBasedOnMouse: true,
        placement: 'bottom left',
        popupGroup: 'tooltip',
        showDelay: 250,
        hideDelay: 10,
        showTrigger: 'auto'
    });

    return {
        restrict: 'A',
        scope: {
            ellipsisTooltip: '@'
        },
        link: function (scope, element, attr, ctrl) {

            function isEllipsisActive(elm) {
                return (elm.outerWidth() < elm[0].scrollWidth);
            }

            var createTooltip = function () {
                element.off('mouseenter', createTooltip);
                if (isEllipsisActive(element)) {
                    $timeout(function () {
                        linkFn(scope, element, attr, ctrl);
                    });
                }
            }

            element.on('mouseenter', createTooltip);

            scope.$on('$destroy', function () {
                element.off('mouseenter', createTooltip);
            });
        }
    }
});
/**
 * angular-elastic-input
 * A directive for AngularJS which automatically resizes the width of input field according to the content, while typing.
 * @author: Jacek Pulit <jacek.pulit@gmail.com>
 * @license: MIT License
 */
app.directive('elasticInput', function ($timeout) {
    return {
        restrict: 'A',
        link: function postLink(scope, element, attrs) {
            var wrapper = angular.element('#pu-elastic-input-wrapper');
            if (!wrapper.length) {
                wrapper = angular.element('<div id="pu-elastic-input-wrapper" style="position:fixed; top:-999px; left:0;"></div>');
                angular.element('body').append(wrapper);
            }
            var mirror = angular.element('<span style="white-space:nowrap;"></span>');
            wrapper.append(mirror);

            $timeout(function () {
                element.css('minWidth', attrs.elasticInputMinwidth || element.css('minWidth'));
                element.css('maxWidth', attrs.elasticInputMaxwidth || element.css('maxWidth'));
                angular.forEach([
                    'fontFamily',
                    'fontSize',
                    'fontWeight',
                    'fontStyle',
                    'letterSpacing',
                    'textTransform',
                    'wordSpacing',
                    'textIndent',
                    'boxSizing',
                    'borderRightWidth',
                    'borderLeftWidth',
                    'borderLeftStyle',
                    'borderRightStyle',
                    'paddingLeft',
                    'paddingRight',
                    'marginLeft',
                    'marginRight'
                ], function (value) {
                    mirror.css(value, element.css(value));
                });
            }, 10);

            var widthMethod = attrs.widthMethod || "min";

            function update() {
                mirror.text(element.val() || attrs.placeholder || element.attr('placeholder'));
                var textWidth = mirror.outerWidth() + 10;

                var maxWidth = element.parent().innerWidth() - (attrs.widthPadding || 0) - element.position().left;

                if (attrs.elasticInputMaxwidth && attrs.elasticInputMaxwidth.indexOf('px')!== -1) {
                    maxWidth = parseFloat(attrs.elasticInputMaxwidth);
                }

                var width = widthMethod === "min" ? Math.min(textWidth, maxWidth) : maxWidth;

                element.css('width', width + "px");
            }
            update();
            if (attrs.ngModel) {
                scope.$watch(attrs.ngModel, function () {
                    update();
                });
            } else {
                element.on('keydown keyup focus input propertychange change', update);
            }
            scope.$watch(function () { return element.position().left; }, function () {
                update();
            });

            scope.$on('$destroy', function () {
                element.off('keydown keyup focus input propertychange change', update);
                mirror.remove();
            });
        }
    };
});


// Name: User Platform
//------------------------------------------------------
// Desc: Simple check to determine what OS a visitor is using.
// Use:
// - [Controller] $scope.isWorldViewSupported = userPlatform.isSupported('WorldView');
app.service('userPlatform', function ($window) {
    // Cache the user platform results
    var user = null;
    // Constants
    var OS = {
        WINDOWS: 'Win',
        MAC: 'Mac',
        LINUX: 'Linux',
        UNIX: 'X11'
    };
    var MOBILE = {
        ANDROID: /Android/i,
        BLACKBERRY: /BlackBerry/i,
        IOS: /iPhone|iPad|iPod/i,
        OPERA: /Opera Mini/i,
        WINDOWS: /IEMobile/i,
    };
    var VERSION = {
        WINDOWS_95: /(Windows 95|Win95|Windows_95)/,
        WINDOWS_ME: /(Win 9x 4.90|Windows ME)/,
        WINDOWS_98: /(Windows 98|Win98)/,
        WINDOWS_CE: /Windows CE/,
        WINDOWS_2000: /(Windows NT 5.0|Windows 2000)/,
        WINDOWS_XP: /(Windows NT 5.1|Windows XP)/,
        WINDOWS_VISTA: /Windows NT 6.0/,
        WINDOWS_7: /(Windows 7|Windows NT 6.1)/,
        WINDOWS_8_1: /(Windows 8.1|Windows NT 6.3)/,
        WINDOWS_8: /(Windows 8|Windows NT 6.2)/,
        WINDOWS_NT_4_0: /(Windows NT 4.0|WinNT4.0|WinNT|Windows NT)/
    };

    var BROWSER = {
        CHROME: !!window.chrome && !this.OPERA,
        SAFARI: Object.prototype.toString.call(window.HTMLElement).indexOf('Constructor') > 0,
        MSIE: /*@cc_on!@*/false || !!document.documentMode,
        IE11: !!navigator.userAgent.match(/Trident.*rv[ :]*11\./),
        FIREFOX: typeof InstallTrigger !== 'undefined',
        OPERA: !!window.opera || navigator.userAgent.indexOf(' OPR/') >= 0
    };

    var getDeviceInfo = function () {
        var platform = $window.navigator.platform;
        var userAgent = $window.navigator.userAgent;
        var appVersion = $window.navigator.appVersion;
        var oscpu = $window.navigator.oscpu;

        user = {
            platform: {
                uniqueId: 'UNKNOWN',
                friendly: 'Unknown'
            },
            device: {
                uniqueId: 'DESKTOP',
                friendly: 'Desktop'
            },
            version: {
                uniqueId: 'UNKNOWN',
                friendly: 'Unknown'
            },
            browser: {
                uniqueId: 'UNKNOWN',
                friendly: 'Unknown'
            }
        };

        // Check OS
        angular.forEach(OS, function(value, key) {
            if(platform.indexOf(value) >= 0) {
                user.platform.uniqueId = key;
                user.platform.friendly = value === 'Win' ? 'Windows' : value;
            }
        });

        // If the user is on a mobile device
        angular.forEach(MOBILE, function(value, key) {
            if (userAgent.match(value)) {
                user.device.uniqueId = 'MOBILE';
                user.device.friendly = 'Mobile';
            }
        });

        // Check OS version: when Windows (and device isn't mobile) add version
        if (user.platform.uniqueId === 'WINDOWS' && user.device.uniqueId === 'DESKTOP') {
            angular.forEach(VERSION, function(value, key) {
                if (appVersion.match(value)) {
                    var match = value.exec(value);
                    user.version.uniqueId = key;
                    user.version.friendly = match[0];
                }

                // The newest version of Mozilla Firefox uses this instead of appVersion
                if (oscpu) {
                    if (oscpu.match(value)) {
                        if (match === undefined) {
                            match = value.exec(value);
                            user.version.uniqueId = key;
                            user.version.friendly = match[0];
                        }
                    }
                }

            });
        }

        // Check the user's browser
        angular.forEach(BROWSER, function(value, key) {
            if (value) {
                user.browser.uniqueId = key;
                user.browser.friendly = key.toLowerCase();
            }
        });

        return user;
    };

    return {
        get : function (attribute) {
            if (user) {
                return user[attribute].friendly;
            } else {
                return getDeviceInfo()[attribute].friendly;
            }
        },
        isMobile: function () {
            // Determine if the user is on a mobile device
            // @returns {Boolean}
            return (/android|webos|iphone|ipad|ipod|blackberry|windows phone/).test(navigator.userAgent.toLowerCase());
        },
        isSupported : function (context) {
            var info = getDeviceInfo();
            var supported = false;
            var supportedPlatform = 'WINDOWS';
            var supportedVersions = [
                'WINDOWS_VISTA',
                'WINDOWS_7',
                'WINDOWS_8_1',
                'WINDOWS_8',
                'WINDOWS_NT_4_0'
                ];

            if (context === 'WorldView') {
                if (info.platform.uniqueId === supportedPlatform) {
                    if (supportedVersions.indexOf(info.version.uniqueId ) >= 0) {
                        supported = true;
                    }
                }

                return supported;
            }
        }
    }

});

// Name: Set Focus (auto)
//------------------------------------------------------
// Desc: Use this directive to set focus on <input />
// or <textarea /> elements on pages or modals
// Use:
// - [View] <input type='text' set-focus />
app.directive('setFocus', function($timeout) {
    return {
        restrict: 'AC',
        link: function(scope, element){
            var handleFocus = function(){
                return element[0].focus();
            }
            // Wait 100ms for DOM to render
            $timeout(handleFocus, 100);
        }
    }
});

// Name: Detect and Launch Studio, if user isn't authenticated
//-------------------------------------------------------------
// Desc: Check if the protocal exists - if it does
// attempt to launch the application. If it doesn't
// then go to the provided path.
// Use:
// - [Controller] detectAndLaunchStudio.launchStudio().then(handleExists, handleVapour)

app.service('detectAndLaunchStudio', function ($document, $window, $timeout, $q, userPlatform) {
    // @const {object} messages
    var messages = {
        NS_ERROR_PROTOCOL: 'NS_ERROR_UNKNOWN_PROTOCOL',
        UNKNOWN_PROTOCOL: 'Unknown Protocol',
        MSIE_APP_NAME: 'Microsoft Internet Explorer'
    }

    // @define {string} protocol - the pyxis protocol
    var protocol = 'pyxis:';

    var timerSlow = 500;
    var timerFast = 100;

    // @template - temporary DOM needed to determine if protocol exists
    var template = [
        '<div class="detect-and-launch-studio">',
        '<input value="" placeholder="custom protocol"/>',
        '<iframe src="about:blank"></iframe>',
        '<a href="#">custom protocol</a>',
        '</div>'
        ];

    var tempDOM;
    var docBody = $document.find('body');
    var methods = {};

    var runOnBrowser =  {
        firefox: function (deferred) {
            var $frame = tempDOM.find('iframe')[0];

            try {
                $frame.contentWindow.location.href = protocol;
                deferred.resolve({success: true, browser: 'firefox'});
            } catch (e) {
                if (e.name === messages.NS_ERROR_PROTOCOL) {
                    deferred.reject({success: false, browser: 'firefox'});
                }
            }

        },
        chrome: function (deferred) {
            var $protocol = tempDOM.find('input')[0];

            $protocol.focus();

            $protocol.onblur = function () {
                deferred.resolve({success: true, browser: 'chrome'});
            };

            // Triggers onblur()
            location.href = protocol;

            $timeout(function() {
                $protocol.onblur = null;
                deferred.reject({success: false, browser: 'chrome'});
            }, timerSlow);

        },
        msie: function (deferred) {
            var $link = tempDOM.find('a')[0];
            var popWin;
            var exists = false;

            $link.href = protocol;

            // Use case: IE <= IE 11 on Windows 7 and below
            if (navigator.appName === messages.MSIE_APP_NAME && $link.protocolLong === messages.UNKNOWN_PROTOCOL) {
                exists = false;
                deferred.reject({success: false, browser: 'msie'});
                return;
            }

            // Use case: IE 10+ on Windows 8
            if (navigator.msLaunchUri) {
                var handleSuccess = function () {
                    exists = true;
                    deferred.resolve({success: true, browser: 'msie'});
                }

                var handleFailure = function () {
                    exists = false
                    deferred.reject({success: false, browser: 'msie'});
                }

                navigator.msLaunchUri(protocol, handleSuccess, handleFailure);
                return;
            }

            popWin = window.open('', '', 'width=0, height=0', '_self');
            popWin.document.write('<iframe src=' + protocol + '></iframe>');

            $timeout(function(){
                try {
                    popWin.location.href;
                    exists = true;
                    deferred.resolve({success: true, browser: 'msie'});
                } catch(e) {
                    exists = false;
                    deferred.reject({success: false, browser: 'msie'});
                }

                if (exists) {
                    popWin.setTimeout('window.close()', timerFast);
                } else {
                    popWin.close();
                }

            }, timerFast);

        },
        unknown: function () {
            var $frame = tempDOM.find('iframe')[0];

            $frame.contentWindow.location = protocol;

            $timeout(function(){
                try {
                    deferred.resolve({success: true, browser: 'unknown'});
                } catch(e) {
                    deferred.reject({success: false, browser: 'unknown'});
                }

            });
        }
    }


    var beforeLaunch = function () {
        if (!tempDOM) {
            docBody.append(template.join(''));
            tempDOM = docBody.find('.detect-and-launch-studio');
        }
    }

    methods.launchStudio = function () {
        var self = this;
        var browser = userPlatform.get('browser');
        var deferred = $q.defer();

        beforeLaunch();

        if (browser === 'ie11') {
            browser = 'msie';
        }

        $timeout(function(){
            runOnBrowser[browser](deferred);
        });

        return deferred.promise;
    }

    return methods;

});


// Name: Welcome Banner (auto)
//------------------------------------------------------
// Desc: Creates a TweenMax animation to show the steps on the welcome banner
//
app.directive('welcomeBanner', function ($window, $timeout) {
    return {
        restrict: 'C',
        link: function (scope, element) {

            var animateFadeIn = function(){
                TweenMax.staggerFromTo(
                    element.find('.welcome-panel'),
                    0.75,  // duration of animation
                    { opacity: 0, y:'-50', ease: Quad.easeOut },
                    { opacity: 1, y: 0 },
                    0.3  // time to stagger
                 );
            };

            $timeout(animateFadeIn, 1200); // wait to start fadeIn

        }
    }
});


// Name: Tag Filter Menu (auto)
//------------------------------------------------------
// Desc: Handles showing and hiding the fixed nav at the top when there is
// a welcome banner present. Split from welcome banner directive from Idan's suggestion.
//
app.directive('tagFilterMenu', function ($window) {
    return {
        restrict: 'A',
        require: '^scrollbar',
        link: function (scope, element, attribute, controller) {

            // setup scroll handler only when welcomeBanner is visible
            if (scope.welcomeBanner) {
                var filterTopVisible = false;
                controller.onScroll('tagFilterMenu', function (scrollTop) {
                    if (scrollTop >= 1000 && !filterTopVisible) {
                        element.removeClass('tags-filter-bannerhide');
                        filterTopVisible = true;
                    }

                    if (scrollTop < 1000 && filterTopVisible) {
                        element.addClass('tags-filter-bannerhide');
                        filterTopVisible = false;
                    }
                });
            }

        }
    }
});

// Name: Animate
//------------------------------------------------------
// Desc: Define element animation in response to an event
// @example - default with source <div animate="{source: '.resource-featured-image'}" on-enter="{duration: 0.4, opacity: 1, ease: 'Power1.easeOut'}" on-leave="{duration: 0.3, opacity: 0.5, ease: 'Back.easeOut'}">
// @example - default no source <div animate on-enter="{duration: 0.4, opacity: 1, ease: 'Power1.easeOut'}" on-leave="{duration: 0.3, opacity: 0.5, ease: 'Back.easeOut'}">
// @example - custom event
// - [View] <div animate on-event="enter: 'open-menu', leave: 'close-menu'">
// - [Controller] $scope.$broadcast('open-menu', {type: 'from', duration: 0.4, opacity: 1, ease: 'Power1.easeOut'});
app.directive('animate', function ($timeout, $parse) {
    function animatePostLink (scope, element, attrs) {
        var doEnter;
        var doLeave;
        var setType;
        var setTime;
        var eventEl;
        //properties that TweenMax doesn't support
        var toPurge = {duration: null, type: null};

        //evaluate object expressions
        var options = attrs.options ? $parse(attrs.options)(scope) : undefined;
        var onEnter = attrs.onEnter ? $parse(attrs.onEnter)(scope) : undefined;
        var onLeave = attrs.onLeave ? $parse(attrs.onLeave)(scope) : undefined;
        var onEvent = attrs.onEvent ? $parse(attrs.onEvent)(scope) : undefined;

        if (options && options.source) {
            if (element.closest(options.source).length) {
                eventEl = element.closest(options.source);
            } else if (element.find(options.source).length) {
                eventEl = element.find(options.source);
            } else {
                eventEl = element;
            }
        } else {
            eventEl = element;
        }

        // - @param {object} source
        // - @param {object} exceptions - hash of properties to omit
        var pruneObj = function (source, exceptions) {
            var destination = {};

            for (var prop in source) {
                if (source.hasOwnProperty(prop)) {
                    if (exceptions && (prop in exceptions)) {
                        continue;
                    }

                    destination[prop] = source[prop];
                }
            }

            return destination;
        }

        // - @param {object} element - the directive 'element'
        // - @param {object} animateProps
        var createTween = function (element, animateProps) {
            setTime = animateProps['duration'] || 1;
            setType = animateProps['type'] || 'to';

            animateProps = pruneObj(animateProps, toPurge);
            return TweenMax[setType](element, setTime, animateProps);

        }

        if (onEnter) {
            doEnter = function () {
               createTween(element, onEnter);
            }
        }

        if (onLeave) {
            doLeave = function () {
                createTween(element, onLeave);
            }
        }

        //custom event or default to a 'hover' event
        if (onEvent) {
            scope.$on(onEvent.enter, function (event, animateProps) {
                if (angular.isObject(animateProps)) {
                    createTween(element, animateProps);
                }
            });
            scope.$on(onEvent.leave, function (event, animateProps) {
                if (angular.isObject(animateProps)) {
                    createTween(element, animateProps);
                }
            });
        } else {
            eventEl.hover(doEnter, doLeave);
        }

        scope.$on('$destroy', function () {
            eventEl.off('mouseenter mouseleave');
        });
    }

    return {
        restrict: 'A',
        link: animatePostLink
    }
});

