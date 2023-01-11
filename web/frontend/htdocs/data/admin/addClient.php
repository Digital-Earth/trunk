<?php 
session_start();
if(!isset($_SESSION['authorized']) && !isset($_SESSION['clientreg']))
        die("Unauthorized");
include '/home/pyxis/scripts/dataHeader.php';
$company=$_POST["company"];
if(!isset($company))
	die("Required company");

@mysql_select_db($database) or die( "Unable to select database");
$query="INSERT INTO clients (client,first,last,position,email,phone,uuid) VALUES ('$company','{$_POST["first"]}','{$_POST["last"]}','{$_POST["position"]}','{$_POST["email"]}','{$_POST["phone"]}',uuid())";
$clientResult=mysql_query($query);
if(!$clientResult)
{
        die("Error: " . mysql_error($conn));
}
$query="SELECT uuid FROM clients WHERE client='$company'";
$clientResult=mysql_query($query);
$data=mysql_fetch_assoc($clientResult);
mysql_close();
$model=(object) array(key => $data['uuid']);
header('Content-type: application/json');
echo json_encode($model, JSON_FORCE_OBJECT);
?>
