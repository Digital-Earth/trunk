<?php
$title = "Logging Administration";
$description = "PYXIS | ";
$keywords = "";
session_start();
$_SESSION['authorized'] = 1;
include('../includes/head.php');
?>
<?php
$thisPage = "loggingadmin";
$thissubPage = "loggingadmin"
?>

<style type="text/css">
    table.thinborder
    {
        border-width: 0 0 1px 1px;
        border-spacing: 0;
        border-collapse: collapse;
        border-style: solid;
    }
    .thinborder td, .thinborder th
    {
        margin: 0;
        padding: 4px;
        border-width: 1px 1px 0 0;
        border-style: solid;
    }
    #pyxnetStatus
    {
        float: left;
    }
    #activeNodes
    {
        margin: 10px;
        float: right;
    }
    #logList
    {
        clear:left;

    }
    #content-wrapper
    {
        min-width: 1200px;
    }
</style>

<script type="text/javascript" src="https://ajax.googleapis.com/ajax/libs/angularjs/1.0.7/angular.min.js"></script>
<script type="text/javascript" src="https://www.google.com/jsapi"></script>

<script type="text/javascript">
    filterName = 'all';
    columnIndex = 0;
    google.load("visualization", "1", {packages: ["corechart"]});


    function AdminCtrl($scope, $http) {

        $scope.filterOptions = [
            {"id": "all"},
        ];
        $scope.filterOption = $scope.filterOptions[0].id;

        $scope.filterChange = function() {
            filterName = $scope.filterOption;
            if (filterName != "all")
                columnIndex = $scope.columns.indexOf(filterName);

            $scope.logs = $scope.logs;
        };

        $scope.listFilter = '';

        $scope.exportTbl = function(tbl) {
            window.open('data/logging/exportTable.php?table=' + tbl, '_blank');
            return false;
        };

        $scope.exportQueryResult = function() {
            window.open('data/logging/exportQueryResult.php?q=' + $scope.query, '_blank');
            return false;
        };

        $scope.submitQuery = function() {
            $scope.query = $.trim($scope.query);
            if ($scope.query == "")
                return;
            if ($.inArray($scope.query, $scope.queries) == -1)
                $scope.queries.push($scope.query);
            $http({method: 'POST', url: '../data/logging/webQuery.php', data: $.param({query: $scope.query}),
                headers: {'Content-Type': 'application/x-www-form-urlencoded'}}).
                    success(function(data, status, headers, config) {
                        $scope.logs = data.model;
                        $scope.columns = data.columns;
                        $scope.filterOptions = [{"id": "all"}];
                        for (var i = 0; i < data.columns.length; i++)
                            $scope.filterOptions.push({"id": data.columns[i]});
                    }).
                    error(function(data, status, headers, config) {
                        $scope.columns = [{time: 'Failed', id: 'Failed', ip: 'Failed', name: 'Failed', cat: 'Failed', key: 'Failed', val: 'Failed'}];
                        $scope.logs = [];
                    });
        };

        $scope.submitPyxnetStatus = function() {
            $http({
                method: 'POST',
                url: '../data/logging/webQuery.php',
                data: $.param({query: 'SELECT date_format(time,"%d/%c") as title,count(distinct(name)) as nodes from logging WHERE name != "PYXIS WorldView.exe" and name != "PYXIS WorldView.vshost.exe" and name != "PyxNetHub.exe" and time >= SYSDATE() - INTERVAL 4 WEEK GROUP BY title ORDER BY time LIMIT 100'}),
                headers: {'Content-Type': 'application/x-www-form-urlencoded'}
            }).
                    success(function(data, status, headers, config) {
                        $scope.activityPerDay = data.model;
                        $scope.maxNodesPerDay = 0;
                        for (var index in data.model) {
                            if (data.model[index][1] > $scope.maxNodesPerDay) {
                                $scope.maxNodesPerDay = data.model[index][1];
                            }
                        }
                    });
            $http({
                method: 'POST',
                url: '../data/logging/webQuery.php',
                data: $.param({query: 'SELECT name from logging WHERE name != "PYXIS WorldView.exe" and name != "PYXIS WorldView.vshost.exe" and name != "PyxNetHub.exe" and time >= SYSDATE() - INTERVAL 1 DAY GROUP BY name ORDER BY name'}),
                headers: {'Content-Type': 'application/x-www-form-urlencoded'}
            }).
                    success(function(data, status, headers, config) {
                        $scope.todayActiveNodes = data.model;

                    });

            $http({
                method: 'POST',
                url: '../data/logging/webQuery.php',
                data: $.param({query: 'SELECT val, COUNT(time) FROM   (SELECT  `key`, MAX(time)"lasttime" FROM logging WHERE cat="Gwss.GalleryTest"  AND (val = "Available" OR val = "Offline") AND (time >= SYSDATE() - INTERVAL 1 WEEK ) GROUP BY `key`) as T1    LEFT JOIN   (SELECT  `key`,val,time FROM logging   WHERE cat="Gwss.GalleryTest"  AND (val = "Available" OR val = "Offline")   GROUP BY `key`,time)  as T2    ON (T1.key=T2.key AND T1.lasttime=T2.time)   GROUP BY val'}),
                headers: {'Content-Type': 'application/x-www-form-urlencoded'}
            }).
                    success(function(data, status, headers, config) {
                        pipelinesStatus = data.model;
                        drawChart();
                    });
        };

        $scope.refresh = function() {
            $scope.query = $scope.queries[0];
            $scope.submitQuery();

        };

        $scope.showActivityForNode = function(node) {
            $scope.query =
                    "SELECT name,id,ip,time,cat,`key`,val FROM logging WHERE name='" + node + "' AND time >= SYSDATE() - INTERVAL 1 WEEK ORDER BY time DESC LIMIT 1000";
            $scope.submitQuery();
        };

        $scope.showErrorsForNode = function(node) {
            $scope.query =
                    "SELECT name,id,ip,`key`,COUNT(`key`) 'keyErrors',val FROM logging WHERE (`key` LIKE '%error%' OR `key` LIKE '%warning%') AND name='" + node + "' AND time >= SYSDATE() - INTERVAL 1 WEEK GROUP BY name,`key`,val ORDER BY COUNT(`key`) DESC LIMIT 100";
            $scope.submitQuery();
        };

        $scope.toggleDisply = function(item) {
            document.getElementById(item)
            var display1 = document.getElementById(item);
            display1.style.display = (display1.style.display == 'none' ? '' : 'none');
        };

        $scope.queries = [
            "SELECT name,id,ip,`key`,COUNT(`key`) 'keyErrors',val FROM logging WHERE (`key` LIKE '%error%' OR `key` LIKE '%warning%') AND time >= SYSDATE() - INTERVAL 1 WEEK GROUP BY name,`key`,val ORDER BY COUNT(`key`) DESC LIMIT 100",
            "SELECT name,id,ip,val,COUNT(val) 'valErrors',`key` FROM logging WHERE (val LIKE '%error%' OR val LIKE '%warning%') AND time >= SYSDATE() - INTERVAL 1 WEEK GROUP BY name,val,`key` ORDER BY COUNT(val) DESC LIMIT 100",
            "SELECT name,id,ip,COUNT(name) 'keyErrors' FROM logging WHERE (`key` LIKE '%error%' OR `key` LIKE '%warning%') AND time >= SYSDATE() - INTERVAL 1 WEEK GROUP BY name ORDER BY COUNT(name) DESC LIMIT 100",
            "SELECT name,id,ip,COUNT(name) 'valErrors' FROM logging WHERE (val LIKE '%error%' OR val LIKE '%warning%') AND time >= SYSDATE() - INTERVAL 1 WEEK GROUP BY name ORDER BY COUNT(name) DESC LIMIT 100",
            "SELECT * FROM logging ORDER BY time DESC LIMIT 100",
            "SELECT * FROM logging WHERE time < '2013-06-23 12:00:00' AND time > '2013-06-22 12:00:00' ORDER BY time DESC LIMIT 100",
            "SELECT id,ip,name,COUNT(name) numMessages FROM logging WHERE time >= SYSDATE() - INTERVAL 1 WEEK GROUP BY name ORDER BY COUNT(name) DESC LIMIT 100",
            "SELECT table_schema 'DB Name', sum(data_length+index_length)/1024/1024 'DB Size (MB)' FROM information_schema.TABLES GROUP BY table_schema"];

        $scope.togglePyxnetStatus = function() {
            if (document.getElementById('pyxnetStatus').style.display == 'none')
            {
                $scope.submitPyxnetStatus();
            }
            $scope.toggleDisply('pyxnetStatus');
            $scope.toggleDisply('activeNodes');
            $scope.toggleDisply('galleryStatus');
        }

        $scope.refresh();

        function drawChart() {
            var data = google.visualization.arrayToDataTable([
                ['Status', 'Number of Pipelines'],
                [pipelinesStatus[0][0], Number(pipelinesStatus[0][1])],
                [pipelinesStatus[1][0], Number(pipelinesStatus[1][1])]
            ]);

            var options = {
                title: 'Gallery Pipelines Availablity',
                backgroundColor: 'transparent',
                pieSliceText: 'value'
            };

            var chart = new google.visualization.PieChart(document.getElementById('piechart'));
            chart.draw(data, options);
            // Add our selection handler.
            google.visualization.events.addListener(chart, 'select', selectHandler);

            function selectHandler() {
                var selection = chart.getSelection();
                var message = '';
                for (var i = 0; i < selection.length; i++) {
                    var item = selection[i];
                    if (item.row == 1 && item.column == null) {
                        $scope.query =
                            "SELECT logging.name,logging.key,logging.val,logging.time FROM  (SELECT T1.key 'offlineProcRef' FROM (SELECT  `key`, MAX(time)'lasttime'  FROM logging  WHERE  ( val = 'Offline' OR val = 'Available') AND (time >= SYSDATE() - INTERVAL 1 WEEK )  GROUP BY `key`)AS T1 JOIN (SELECT  `key`,val,time  FROM logging  WHERE ( val = 'Offline')  GROUP BY `key`,time)AS T2 ON (T1.key=T2.key AND T1.lasttime=T2.time)) AS lastTimeOffline  JOIN  logging  ON (logging.key = offlineProcRef) ORDER BY `key`";
                        $scope.submitQuery();
                    }
                }
            }
        }

    }
    angular.module('App', []).filter('resultFilter', function() {

        return function(items, name) {
            var arrayToReturn = [];
            if (items != null)
            {
                var re = new RegExp(name, 'i');
                if (filterName == 'all')
                {
                    for (var i = 0; i < items.length; i++) {
                        for (var prop in items[i]) {
                            if (prop != "$$hashKey" && items[i][prop].match(re) != null) {
                                arrayToReturn.push(items[i]);
                                break;
                            }
                        }
                    }
                }
                else
                {
                    for (var i = 0; i < items.length; i++) {
                        if (items[i][columnIndex].match(re) != null) {
                            arrayToReturn.push(items[i]);
                        }
                    }
                }
            }

            return arrayToReturn;
        };

    }).directive('onEnter', function() {
        return function(scope, element, attrs) {
            element.bind("keydown keypress", function(event) {
                if (event.which === 13) {
                    scope.$apply(function() {
                        scope.$eval(attrs.onEnter);
                    });
                    event.preventDefault();
                }
            });
        };
    });



</script>


</head>
<body ng-app="App">
    <?php require("../includes/header.php"); ?>
    <!--  CONTENTS  -->
<base href="http://www.pyxisinnovation.com">
<div id="content-wrapper" align="left">
    <h1>Logging Administration</h1>
    <div ng-controller="AdminCtrl">
        <button ng-click="togglePyxnetStatus()">PyxNet Status</button>
        <br>
        <div id="pyxnetStatus" style="display : none">
            <h4>PyxNet Status</h4>
            <table class="thinborder">
                <tr>
                    <td>Day:</td>
                    <td width="25px" ng-repeat="day in activityPerDay">{{day[0]}}</td>
                </tr>
                <tr>
                    <td>Active Nodes:</td>
                    <td ng-repeat="day in activityPerDay"><div style="height:{{maxNodesPerDay * 2 - day[1] * 2}}px;"></div><div style="height:{{day[1] * 2}}px;background-color:blue;"/></div>{{day[1]}}</td>
                </tr>
            </table>
        </div>



        <div  id="activeNodes" style="display : none">
            <h4>Today Active nodes:</h4>
            <table class="thinborder">
                <tr ng-repeat="node in todayActiveNodes">
                    <td>{{node[0]}}</td>
                    <td>
                        <button ng-click="showErrorsForNode(node[0])">Errors</button>
                        <button ng-click="showActivityForNode(node[0])">Activity</button>
                    </td>
                </tr>
            </table>
            <br>
        </div>

        <div id="galleryStatus" style="clear:left;display : none">
            <div id="piechart" style="float:left; clear : left; width: 500px; height: 500px;"></div> 
        </div>

        <div id="logList">
            <h4>Logs List</h4>
            <button ng-click="refresh()">Refresh</button>
            <button ng-click="exportTbl('logging')">Export Logs</button>
            <button ng-click="exportQueryResult()">Export Query Result</button>
            <br/>
            Query: 
            <input list="queries" type="text" rows="4" name="query" size="250" ng-model="query" on-enter="submitQuery()"> -
           <!-- <textarea list="queries" rows="4" name="query" ng-model="query" on-enter="submitQuery()"> -->

            <datalist id="queries">
                <option ng-repeat="query in queries" value="{{query}}"></option>
            </datalist>
            <button ng-click="submitQuery()">Submit</button>
            <br />
            Refine: <input type="text" name="listFilter" ng-model="listFilter" size="100">
            <select ng-change="filterChange()" ng-model="filterOption" ng-options="filterOption.id as filterOption.id for filterOption in filterOptions">
            </select>
            <table class="thinborder">
                <tr>
                    <th ng-repeat="col in columns">{{col}}</th>
                </tr>
                <tr ng-repeat="log in logs| resultFilter:listFilter">
                    <td ng-repeat="field in log">{{field}}</td>
                </tr>
            </table>
        </div>
    </div>  
    <!--<p class="top-border">License info goes here.</p>-->
    <br/>
    <br/>
    <br/>

</div>
<!--end content-wrapper-->
<?php require("../includes/footer.php"); ?>
</body>
</html>
