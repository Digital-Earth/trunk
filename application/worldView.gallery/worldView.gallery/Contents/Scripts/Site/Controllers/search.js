app.controller("worldviewSearchController", function ($scope, $pyx, $location, $routeParams, resourceType) {
    $scope.resourceType = resourceType || "GeoSource";
    $scope.search = $routeParams.search || "";

    $scope.currentId = 0;
    $scope.loading = false;
    $scope.done = false;
    $scope.noResultsFound = false;

    // logic to determine if welcome banner is shown
    $scope.welcomeBanner = false;
    $scope.bannerClass = '';
    if (!$pyx.user.auth() && $scope.search === '') {
        $scope.welcomeBanner = true;
        $scope.bannerClass = 'tags-filter-banner tags-filter-bannerhide';
    }
    


    //split search into words
    $scope.searchWords = $pyx.array.where(
        $scope.search.split(" ").map(function (x) { return angular.lowercase(x); }),
        function (word) { return word.length; });
    $scope.casedSearchWords = $scope.search.split(" ");

    var extractResourceType = function () {
        ["GeoSource", "Map", "Gallery"].forEach(function(type) {
            var index = $scope.searchWords.indexOf("type:" + type.toLowerCase());
            if (index !== -1) {
                $scope.resourceType = type;
                $scope.searchWords.splice(index, 1);
                $scope.casedSearchWords.splice(index, 1);
            }
        });

        $scope.fixedSearch = $scope.searchWords.join(" ");
    }

    extractResourceType();

    //utility function to check if a tag is inside search
    //Note: not working for tags with space inside of them ("Natural Resource")
    $scope.insideSearch = function (word) {
        return $scope.searchWords.indexOf(angular.lowercase(word)) !== -1;
    }

    //add/remove a tag into search string
    $scope.toggleSearchWord = function (word) {
        var wordRemoved = false;

        var words = $pyx.array.where(
            $scope.search.split(" "),
            function (word) { return word.length; });

        words = $pyx.array.where(words,
            function (searchWord) {
                if (angular.lowercase(searchWord) === angular.lowercase(word)) {
                    wordRemoved = true;
                    return false;
                }
                return true;
            });

        if (!wordRemoved) {
            words.push(word);
        }

        if (words.length) {
            $location.search({ 'search': words.join(" ") });
        } else {
            $location.search({});
        }
    }

    $scope.getResources = function () {
        if ($scope.resourceType === "Gallery") {
            return $pyx.gallery.galleries();
        }
        if ($scope.resourceType === "Map") {
            return $pyx.gallery.maps();
        }
        if ($scope.resourceType === "GeoSource") {
            return $pyx.gallery.geoSources();
        }
        return $pyx.gallery.resources().format("view");
    }

    $scope.getTagsResources = function () {
        if ($scope.resourceType === "Gallery") {
            return $pyx.gallery.galleries();
        }
        if ($scope.resourceType === "Map") {
            return $pyx.gallery.maps();
        }
        return $pyx.gallery.geoSources();
    }

    $scope.searchMatchSuggestion = function() {
        $location.search({ 'search': $scope.matchSuggestion });
    }

    var suggestTerms = function (searchWords) {
        // suggest for the last searchWord
        $pyx.gallery.suggestTerms(searchWords[searchWords.length - 1]).then(function(listSuggestions) {
            if (listSuggestions.data.length) {
                if (searchWords.length > 1) {
                    // prefix suggested term with the other searchWords 
                    var prefixWords = searchWords.slice(0, searchWords.length - 1);
                    var prefix = prefixWords.join(" ") + " ";
                    for (var i = 0; i < listSuggestions.data.length; i++) {
                        listSuggestions.data[i] = prefix + listSuggestions.data[i];
                    }
                }
                $scope.$emit("search-list-changed", listSuggestions.data);
            } else {
                $scope.$emit("search-list-changed", []);
            }
        });
    }

    //triggered when search was changed
    $scope.searchChanged = function () {
        $scope.currentId++;
        $scope.cards = [];
        $scope.tags = [];
        if ($scope.fixedSearch === "") {
            $scope.query = $scope.getResources().orderByDesc("Metadata.Updated");
            $scope.query.top = 30;

            $scope.tagsQuery = $scope.getTagsResources().group("Tags");

            $scope.$emit("search-list-changed", []);
        } else {
            $scope.query = $scope.getResources().search($scope.fixedSearch).orderByDesc("Metadata.Updated");
            $scope.query.top = 30;

            $scope.tagsQuery = $scope.getTagsResources().search($scope.fixedSearch).group("Tags");

            $pyx.gallery.suggestCompletions($scope.fixedSearch).then(function(listSuggestions) {
                if (listSuggestions.data.length) {
                    $scope.$emit("search-list-changed", listSuggestions.data);
                } else {
                    suggestTerms($scope.casedSearchWords);
                }
            });
        }

        $scope.query.id = $scope.currentId;
        $scope.loading = false;
        $scope.done = false;

        $scope.loadMore();
        $scope.loadTags();
    }

    $scope.noResultSearch = function () {
        $scope.currentId++;
        $scope.cards = [];
        $scope.tags = [];

        var types = $scope.resourceType !== "All" ? [$scope.resourceType] : undefined;
        $pyx.gallery.suggestMatches($scope.search, types).then(function(matchSuggestions) {
            if (matchSuggestions.data.length) {
                // Don't suggest if it is the same as what was previously suggested
                if ($scope.fixedSearch !== matchSuggestions.data[0].toLowerCase()) {
                    $scope.matchSuggestion = matchSuggestions.data[0];
                } else {
                    if (matchSuggestions.data.length > 1) {
                        $scope.matchSuggestion = matchSuggestions.data[1];
                    } else {
                        $scope.matchSuggestions = undefined;
                    }
                }
            }
        });

        $scope.query = $pyx.gallery.galleries().orderByDesc("Metadata.Updated");
        $scope.query.top = 30;

        //$scope.tagsQuery = $pyx.gallery.galleries().group('Tags');

        $scope.query.id = $scope.currentId;
        $scope.loading = false;
        $scope.done = false;

        $scope.loadMore();
        //$scope.loadTags();
        $scope.tags = [];
    }

    //called when more cards are needed
    $scope.loadMore = function () {
        if (!$scope.loading && !$scope.done && $scope.currentId === $scope.query.id) {
            $scope.loading = true;
            var query = $scope.query;
            query.get().success(function (sources) {
                $scope.loading = false;
                if (query.id === $scope.currentId) {
                    //collect all empty galleries we found.
                    var emptyGalleries = [];

                    for (var i = 0; i < sources.length; i++) {
                        if (sources[i].Type === "GeoSource" && sources[i].State === "Removed") continue;
                        if (sources[i].Type === "Map" && sources[i].State === "Removed") continue;
                        if (sources[i].Type === "Gallery" && sources[i].Resources.length === 0) {
                            if ($pyx.user.username() !== sources[i].Metadata.User.Name && $scope.cards.length > 1) {
                                emptyGalleries.push(sources[i]);
                            }
                            continue;
                        }
                        if (sources[i].Type === "User") continue;
                        if (sources[i].Type === "Pipeline") continue;

                        $scope.cards.push(sources[i]);
                    }

                    //add all empty galleries we found if there is no other results
                    if ($scope.cards.length === 0 && emptyGalleries.length > 0) {
                        angular.forEach(emptyGalleries, function (gallery) {
                            $scope.cards.push(gallery);
                        });
                    }

                    if (sources.length === 0) {
                        $scope.done = true;
                        if ($scope.cards.length === 0) {
                            $scope.noResultsFound = true;
                            $scope.noResultSearch();
                        }
                    }
                }
                else {
                    $scope.loadMore();
                }
            });
            $scope.query.nextPage();
        }
    }

    //called when tags needed to be loaded
    $scope.loadTags = function () {
        var query = $scope.query;
        $scope.tagsQuery.get().success(function (tags) {
            if (query.id === $scope.currentId) {
                var inSearchTags = [];

                //trim names
                angular.forEach(tags, function (tag) {
                    tag.Name = tag.Name.trim();
                });

                //remove empty tags
                tags = $pyx.array.where(tags, function (tag) { return tag.Name.length; });

                //move insideSearch tags to the front of the tags list
                angular.forEach($scope.searchWords, function (word) {
                    var index = $pyx.array.firstIndex(tags, function (tag) {
                        return angular.lowercase(tag.Name) === word;
                    });
                    if (index !== -1) {
                        inSearchTags.push(tags[index]);
                        tags.splice(index, 1);
                    }
                });

                //remove duplicated tags
                $scope.tags = [];
                angular.forEach(inSearchTags.concat(tags), function (tag) {
                    var index = $pyx.array.firstIndex($scope.tags, function (existingTag) {
                        return angular.lowercase(tag.Name) === angular.lowercase(existingTag.Name);
                    });
                    if (index === -1) {
                        $scope.tags.push(tag);
                    }
                });
            }
        });
    }

    //start loading cards...
    $scope.searchChanged();
});