angular.module('pyxis-ui', ['pyxis'])
    .directive('pyxLibrary', function () {
        return {
            restrict: 'E',
            scope: { library: '=' },
            controller: function ($scope) {
                $scope.route = '';
                this.navigateTo = function (route) {
                    console.log("Navigating to " + route);
                    $scope.route = route;
                    $scope.createSection();
                }
                $scope.createSection = function () {
                    if ($scope.route == '') {
                        $scope.library.sections = [
                            {
                                title: 'My Useful Stuff',
                                groups: $scope.library.recentylUsed,
                                items: []
                            },
                            {
                                title: 'My Subscriptions',
                                groups: $scope.library.subscriptions,
                                items: []
                            }
                        ];
                    } else {
                        var group = $scope.library.groups[$scope.route];
                        $scope.library.sections = [
                            {
                                title: group.View.name,
                                groups: group.getGroups(),
                                item: group.getItems(),
                            }
                        ];
                    }
                }

                $scope.createSection();
            },
            template: '\
<div class="pyx-library">\
 <div ng-repeat="section in sections">\
  <div class="header">{{section.title}}</div>\
  <pyx-library-group-item item="item" ng-repeat="item in section.groups"></pyx-library-group-item>\
  <pyx-library-item item="item" ng-repeat="item in section.items"></pyx-library-item>\
 </div>\
</div>'
        };
    })
    .directive('pyxLibraryGroupItem', function () {
        return {
            restrict: 'E',
            scope: { item: '=' },
            require: '^pyxLibrary',
            link: function (scope, element, attrs, pyxLibraryCtrl) {
                console.log("linking " + scope.item.Id);
                scope.navigate = function () {
                    console.log("item navigating " + scope.item.Id);
                    pyxLibraryCtrl.navigateTo(scope.item.Id);
                };
            },
            controller: function ($scope) {
            },
            template: '\
<div class="pyx-library-item-gallery" ng-click="navigate()">\
 <div class="pyx-library-item-gallery-title">{{item.View.Name}}</div>\
 <img class="pyx-thumb-big" ng-src="{{item.View.Image}}">\
 <div class="pyx-library-subitem-gallery" ng-repeat="subitem in item.View.SubItems">\
  <img class="pyx-thumb-small" ng-src="{{subitem.Image}}">\
 </div>\
</div>'
        };
    })
    .directive('pyxTagList', function () {
        return {
            restrict: 'E',
            transclude: true,
            template: '\
        <div class="pyx-tags-list" ng-transclude>\
        </div>'
        };
    })
    .directive('pyxTag', function () {
        return {
            restrict: 'E',
            scope: { tagName: '@', tagColor: '@' },
            template: '\
        <span class="pyx-tag">\
          <span class="pyx-tag-icon" style="background-color: {{tagColor}};"></span>{{tagName}}\
        </span>'
        };
    })
    .directive('editable', function () {
        return {
            restrict: 'E',
            replace: true,
            require: '?ngModel', // get a hold of NgModelController
            scope: {
                model: '=ngModel',
            },
            template: '<span ng-switch on="editable"><div ng-switch-when="true"><textarea style="width:100%;" ng-model="$parent.model" ng-blur="toggleEdit()"></textarea></div><span ng-switch-when="false" ng-click="toggleEdit()">{{$parent.model}}</span></span>',
            link: function (scope, element) {
                scope.editable = false;
                scope.toggleEdit = function () {
                    scope.editable = !scope.editable;
                }
            }
        };
    })
    .directive('stopEvent', function () {
        return {
            restrict: 'A',
            link: function (scope, element, attr) {
                element.bind('click', function (e) {
                    e.stopPropagation();
                });
            }
        };
    })
    .factory('RecursionHelper', ['$compile', function ($compile) {
        return {
            /**
             * Manually compiles the element, fixing the recursion loop.
             * @param element
             * @param [link] A post-link function, or an object with function(s) registered via pre and post properties.
             * @returns An object containing the linking functions.
             */
            compile: function (element, link) {
                // Normalize the link parameter
                if (angular.isFunction(link)) {
                    link = { post: link };
                }

                // Break the recursion loop by removing the contents
                var contents = element.contents().remove();
                var compiledContents;
                return {
                    pre: (link && link.pre) ? link.pre : null,
                    /**
                     * Compiles and re-adds the contents
                     */
                    post: function (scope, element) {
                        // Compile the contents
                        if (!compiledContents) {
                            compiledContents = $compile(contents);
                        }
                        // Re-add the compiled contents to the element
                        compiledContents(scope, function (clone) {
                            element.append(clone);
                        });

                        // Call the post-linking function, if any
                        if (link && link.post) {
                            link.post.apply(null, arguments);
                        }
                    }
                };
            }
        };
    }])
    .directive('scopeTree', function (RecursionHelper, $rootScope) {
        return {
            restrict: 'E',
            scope: { model: '=ngModel' },
            template: '\
    <div ng-repeat="(key, value) in model" ng-switch on="type(value)">\
        <span ng-switch-when="function"></span>\
        <span ng-switch-when="object">\
            <div style="position:relative;" class="scope-tree-key" ng-click="expanded[key] = !expanded[key]">\
                <i style="position:absolute;left:-12px;top:3px;" class="fa fa-caret-right" ng-hide="expanded[key]"></i>\
                <i style="position:absolute;left:-15px;top:3px;" class="fa fa-caret-down" ng-show="expanded[key]"></i>\
                {{key}}\
                <span class="scope-tree-key-remove" ng-click="removeKey(key)"><i class="fa fa-times-circle"></i></span>\
                <span class="scope-tree-key-add" ng-click="newKey(key)" stop-event ><i class="fa fa-plus-circle"></i></span>\
                <div stop-event ng-show="newKey[key]"><input ng-model="newKey[key].key" type=text />:<input ng-model="newKey[key].value" type=text /><button ng-click="addKey(key)">Add</button></div>\
            </div>\
            <div style="margin-left:20px;" ng-switch on="expanded[key]">\
                <scope-tree ng-switch-when="true" ng-model="value"></scope-tree>\
            </div>\
        </span>\
        <span ng-switch-default>\
            <span ng-switch on="editing[key]" class="scope-tree-key">{{key}} :\
                <span ng-switch-when="true" class="scope-tree-value"><input ng-model="model[key]" type="text" ng-blur="editing[key]=false"/></span>\
                <span ng-switch-default class="scope-tree-value" ng-click="editing[key]=true">{{value | limitTo:40}}</span>\
                <span class="scope-tree-key-remove" ng-click="removeKey(key)"><i class="fa fa-times-circle"></i></span>\
            </span>\
        </span>\
    </div>',
            compile: function (element) {
                // Use the compile function from the RecursionHelper,
                // And return the linking function(s) which it returns
                return RecursionHelper.compile(element, function (scope, element, attr) {
                    scope.expanded = {};
                    scope.editing = {};
                    scope.newKey = {};
                    if (attr["root"]) {
                        console.log($rootScope);
                        scope.model = $rootScope;
                    }
                    scope.type = function (value) { return typeof (value) };
                    scope.removeKey = function (key) { delete scope.model[key] };
                    scope.newKey = function (key) { scope.newKey[key] = { 'key': '', 'value': '' } };
                    scope.addKey = function (key) {
                        var kv = scope.newKey[key];
                        if (kv.value === '') {
                            scope.model[key][kv.key] = {};
                        } else {
                            if (kv.value == "true") {
                                scope.model[key][kv.key] = true;
                                return;
                            }
                            if (kv.value == "false") {
                                scope.model[key][kv.key] = false;
                                return;
                            }
                            var f = Number(kv.value);
                            if (!isNaN(f)) {
                                scope.model[key][kv.key] = f;
                            } else {
                                scope.model[key][kv.key] = kv.value;
                            }
                        }
                    };
                });
            }
        };
    })
    .directive('pyxLibraryItem', function (RecursionHelper, $rootScope) {
        return {
            'restrict': 'E',
            'replace': true,
            'templateUrl': '/templates/pyx-library-item.html',
            'scope': {
                'resource': '=',
                'selected': '&',
                'eyeClick': '&',
                'expanded': '&',
            },
            'link': function (scope, element) {
                scope.arrowClick = function () {
                    scope.resource.expanded = !scope.resource.expanded;
                    if (scope.resource.expanded) {
                        scope.expanded();
                    }
                }
            }
        };
    });