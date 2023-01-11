<?php
  $title = "Client Registration";
  $description = "PYXIS | ";
  $keywords = "";
  $_SESSION['clientreg']=1;
  include('includes/head.php');
?>
<?php 
  $thisPage="clientreg"; 
  $thissubPage="clientreg"
?>
<style>
.error {
  color: red; 
}
</style>
<script src="https://ajax.googleapis.com/ajax/libs/angularjs/1.0.7/angular.min.js"></script>
<script type="text/javascript" src="http://code.jquery.com/jquery-1.5.2.js"></script>
<script type="text/javascript">
function MainCtrl($scope, $http) {
	  $scope.register = function() {
	  href='http://www.pyxisinnovation.com/gallery.php';
	  $.post("../data/admin/addClient.php", 
		{"first": $scope.mainForm.firstName.$viewValue.trim(),
		 "last": $scope.mainForm.lastName.$viewValue.trim(),
     "position": $scope.mainForm.position.$viewValue.trim(),
     "company": $scope.mainForm.company.$viewValue.trim(), 
		 "phone": $scope.mainForm.phone.$viewValue.trim(),
     "email": $scope.mainForm.email.$viewValue.trim(),
		 "last": $scope.mainForm.lastName.$viewValue.trim(),
		}, function(data) {
			//need to launch the download after the async event or the response was overwritten
			window.location.href = href + "?key=" + data.key;
		})
		.error( function() {
			 // still allow them to download if something is wrong
			window.location.href = href;
		});
	  };
};
</script>
</head>
<body ng-app ng-controller="MainCtrl">
<?php require("includes/header.php"); ?>
<!--  CONTENTS  -->
  
  <div id="content-wrapper">  
    <div class="contents downloads">
      <div id="main_content">  
        <h1>Client Registration</h1>
            <form name="mainForm" ng-submit="register()">
            <table>
            <tr><td colspan="2">
            <h4>Please register before adding the PYXIS WorldView&#153 GeoWeb Browser download button to your website:</h4>
            <br>
            </td></tr>
            <tr>
              <td width="120px"><label for="firstName">First Name</label></td>
              <td width="*"><input id="firstName" name="firstName" type="text" ng-model="client.firstName" required/>
              <span class="error" ng-show="mainForm.firstName.$error.required">required</span></td>
            </tr>
            <tr>
              <td><label for="lastName">Last Name</label></td>
              <td><input id="lastName" name="lastName" type="text" ng-model="client.lastName" required/>
              <span class="error" ng-show="mainForm.lastName.$error.required">required</span></td>
            </tr>
            <tr>
              <td><label for="position">Position</label></td>
              <td><input id="position" name="position" type="text" ng-model="client.position" required/>
              <span class="error" ng-show="mainForm.position.$error.required">required</span></td>
            </tr>
            <tr>
              <td><label for="company">Company</label></td>
              <td><input id="company" name="company" type="text" ng-model="client.company" required/>
              <span class="error" ng-show="mainForm.company.$error.required">required</span></td>
            </tr>
            <tr>
              <td><label for="phone">Phone Number</label></td>
              <td><input id="phone" name="phone" type="text" ng-model="client.phone" required/>
              <span class="error" ng-show="mainForm.phone.$error.required">required</span></td>
            </tr>
            <tr>
              <td><label for="email">Email</label></td>
              <td><input id="email" name="email" type="email" ng-model="client.email" required/>
              <span class="error" ng-show="mainForm.email.$error.required">required</span>
              <span class="error" ng-show="mainForm.email.$error.email">invalid email</span></td>
            </tr>
            <tr> 
              <td></td>
              <td><br><button style="border: 1px solid #888;padding: 5px;border-radius: 5px;" type="submit">Register</button></td>
            </tr>
            </table>
          </form>
        <!--<p class="top-border">License info goes here.</p>-->
        <br/><br/><br/>
      </div> 
    </div>
  </div><!--end content-wrapper-->
<?php require("includes/footer.php"); ?>
</body>
</html>
