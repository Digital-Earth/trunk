<?php 
$uuid=$_POST["key"];
if(!isset($uuid))
{
	echo "Unknown client";
	$uuid=null;
}
include '/home/pyxis/scripts/dataHeader.php';
@mysql_select_db($database) or die( "Unable to select database");
if($uuid!=null)
{
	$query="SELECT id FROM clients WHERE uuid='$uuid'";
	$result=mysql_query($query);
	if(!$result)
	{
        	die("Error: " . mysql_error($conn));
	}
	$data=mysql_fetch_assoc($result);
	$clientID=$data['id'];
}
else
{
	# Default tracking
	$clientID=1;
}
$ip=$_SERVER['REMOTE_ADDR'];
$userAgent=$_SERVER['HTTP_USER_AGENT'];
$query="INSERT INTO downloads (clientID,ip,userAgent) VALUES ($clientID,'$ip','$userAgent')";
$result=mysql_query($query);
if(!result)
{
	die("ERROR: " . mysql_error($conn));
}
mysql_close();
?>
