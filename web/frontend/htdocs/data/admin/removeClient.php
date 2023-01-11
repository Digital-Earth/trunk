<?php 
session_start();
if(!isset($_SESSION['authorized']))
        die("Unauthorized");
$company=$_POST['company'];
if(!isset($company))
	die("No client");
include '/home/pyxis/scripts/dataHeader.php';
@mysql_select_db($database) or die( "Unable to select database");
$query="SELECT id FROM clients WHERE client='$company'";
$result=mysql_query($query);
if(!$result)
{
        die("Error: " . mysql_error($conn));
}
$data=mysql_fetch_assoc($result);
$clientID=$data['id'];
$query="DELETE FROM clients WHERE client='$company'";
$result=mysql_query($query);
$query="DELETE FROM downloads WHERE clientID=$clientID";
$result=mysql_query($query);
mysql_close();
$model=(object) array(result => 'Client Removed');
header('Content-type: application/json');
echo json_encode($model, JSON_FORCE_OBJECT);
?>
