<?php

  $title = "Client Administration";
  $description = "PYXIS | ";
  $keywords = "";
  session_start();
  $_SESSION['authorized']=1;
  include('../includes/head.php');
  
?>
<?php $thisPage="clientadmin"; 
  $thissubPage="clientadmin"
?>
<script src="https://ajax.googleapis.com/ajax/libs/angularjs/1.0.7/angular.min.js"></script>
<script type="text/javascript">
<!--
function exportTbl(tbl)
{
	window.open('../data/admin/exportTable.php?table=' + tbl, '_blank');
	return false;
}

function exportClients()
{
	exportTbl("clients");
}

function exportDownloads()
{
	exportTbl("downloadDetails");
}

function exportClientDownloads()
{
	exportTbl("clientDownloads");
}
//-->
</script>

<script type="text/javascript">
function AdminCtrl($scope, $http) {
  $scope.addClient = function() {
    client=$('input[name=clientName]').val();
    if(!client)
      return;
    $('input[name=clientName]').val("");

    $http({method: 'POST', url: '../data/admin/addClient.php', data: $.param({company: client}),
      headers: {'Content-Type': 'application/x-www-form-urlencoded'}}).
      success(function(data, status, headers, config) {
        $scope.addMessage = "Send the following to the client";
        key = data['key'];
        $scope.addCode = "http://www.pyxisinnovation.com/gallery.php?key=" + key;
        $scope.addVisible = true;
      }).
        error(function(data, status, headers, config) {
          $scope.addMessage = "Unable to add client... Error message:";
          $scope.addCode = data;
          $scope.addVisible = true;
  });
  $scope.removeVisible = false;
  }


  $scope.removeClient = function() {
  client=$('input[name=clientName]').val();
  if(!client)
  {
  $('#embedCode').hide();
  return;
  }
  $('input[name=clientName]').val("");

  $http({method: 'POST', url: '../data/admin/removeClient.php', data: $.param({company: client}),
  headers: {'Content-Type': 'application/x-www-form-urlencoded'}}).
  success(function(data, status, headers, config) {
    $scope.addMessage = data['result']
    $scope.removeVisible = true;
      }).
      error(function(data, status, headers, config) {
        $scope.addMessage = "Unable to remove client... Error message: " + data;
        $scope.removeVisible = true;
    });
    $scope.addVisible = false;
  }


  $scope.refreshStats = function() {
  $scope.stats = {clientCount:'loading', downloadCount:'loading'};
  $http({method: 'GET', url: '../data/admin/statistics.php'}).
  success(function(data, status){
    $scope.stats = data;
    $scope.maxDownloadsPerDay = 0;
    for(var index in data.activityPerDay) {
      if (data.activityPerDay[index][1] > $scope.maxDownloadsPerDay) {
        $scope.maxDownloadsPerDay = data.activityPerDay[index][1];
      }
    }
  }).
  error(function(data, status){
  $scope.stats.clientCount = 'Failed request';
  $scope.stats.downloadCount = 'Failed request';
  });
  };

  $scope.refreshStats();

  $scope.listFilter = '';

  $scope.clientList = function() {
  $scope.clients = [{name:'loading', downloadCount:'loading', key:'loading'}];
  $http({method: 'GET', url: '../data/admin/listClients.php'}).
  success(function(data, status){
  $scope.clients = data;
  }).
  error(function(data, status){
  $scope.clients.name = 'Failed request';
  $scope.clients.downloadCount = 'Failed request';
  $scope.clients.key = 'Failed request';
  });
  };

  $scope.clientList();
  }

  angular.module('App',[]).filter('nameFilter', function(){

  return function(items, name){
  var re = new RegExp(name, 'i');
  var arrayToReturn = [];
  for (var i=0; i<items.length; i++){
            if (items[i].name.match(re)!=null) {
                arrayToReturn.push(items[i]);
            }
        }
        
        return arrayToReturn;
    };
});
</script>

</head>
<body ng-app="App">
  <?php require("../includes/header.php"); ?>
  <base href="http://www.pyxisinnovation.com">
  <!--  CONTENTS  -->

  <div id="content-wrapper">
    <div class="contents downloads">
      <div id="main_content">
        <h1>Client Administration</h1>
        <div ng-controller="AdminCtrl">
        <h4>Summary of Client Statistics</h4>
          <button ng-click="refreshStats()">Refresh</button>
          <table>
            <tr>
              <th># of Clients</th>
              <th># of Downloads</th>
            </tr>
            <tr>
              <td>{{stats.clientCount}}</td>
              <td>{{stats.downloadCount}}</td>
            </tr>
          </table>
          <h4>Daily Downloads</h4>
          <div>
             <table>
               <tr>
                 <td>Day:</td>
                 <td width="25px" ng-repeat="day in stats.activityPerDay">{{day[0]}}</td>
               </tr>
               <tr>
                 <td>Downloads:</td>
               <td align="center" ng-repeat="day in stats.activityPerDay"><div style="height:{{maxDownloadsPerDay*3-day[1]*3}}px;"></div><div style="height:{{day[1]*3}}px;background-color:blue;"/></div>{{day[1]}}</td>
               </tr>
             </table>
          </div> 
          <br/>
          <h4>Client List</h4>
          <button ng-click="clientList()">Refresh</button>
          <button onclick="exportClients()">Export Clients</button>
          <button onclick="exportClientDownloads()">Export Client Downloads</button>
          <button onclick="exportDownloads()">Export Download Details</button>
          <br/>
          Refine: <input type="text" size="50" name="listFilter" ng-model="listFilter">
            <table>
              <tr>
                <th>Client Name</th>
                <th># of Downloads</th>
                <th>Key</th>
              </tr>
              <tr ng-repeat="client in clients | nameFilter:listFilter">
                <td>{{client.name}}</td>
                <td>{{client.downloadCount}}</td>
                <td>{{client.key}}</td>
              </tr>
            </table>
        <br/>
        <h4>Add/Remove a Client</h4>
        Client Name: <input type="text" name="clientName">
          <button ng-click="addClient()">Add</button>
          <button ng-click="removeClient()">Remove</button>
          <br/>
            <h5>{{addMessage}}</h5>
            <textarea readonly rows="2" style="width:500px" ng-show=addVisible>{{addCode}}</textarea>
        </div>  
	<!--<p class="top-border">License info goes here.</p>-->
          <br/>
          <br/>
          <br/>
        </div>
    </div>
  </div>
  <!--end content-wrapper-->
  <?php require("../includes/footer.php"); ?>
</body>
</html>
