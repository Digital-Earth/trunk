<?php 
session_start();
if(!isset($_SESSION['authorized']))
	die("Unauthorized");
include '/home/pyxis/scripts/dataHeader.php';
@mysql_select_db($database) or die( "Unable to select database");
$query="SELECT * FROM clientDownloads ORDER BY client ASC";
$result=mysql_query($query);
$num=mysql_numrows($result);
mysql_close();
$i=0;
while ($i < $num) {
	$name=mysql_result($result,$i,"client");
	$downloads=mysql_result($result,$i,"numDownloads");
	$uuid=mysql_result($result,$i,"uuid");
	$model[]=(object)array('name' => $name, 'downloadCount' => (int)$downloads, 'key' => $uuid);
	$i++;
}
header('Content-type: application/json');
echo json_encode($model);
?>
