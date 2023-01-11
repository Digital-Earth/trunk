<?php 
session_start();
if(!isset($_SESSION['authorized']))
  die("Unauthorized");
include '/home/pyxis/scripts/dataHeader.php';
@mysql_select_db($database) or die( "Unable to select database");
$query="SELECT COUNT(client) AS numClients, SUM(numDownloads) AS numDownloads FROM clientDownloads";
$clientResult=mysql_query($query);
$data=mysql_fetch_assoc($clientResult);
$numClients=(int)$data['numClients'];
$numDownloads=(int)$data['numDownloads'];

$query="SELECT date_format(time,'%d/%c') AS date, count(id) AS downloads FROM downloads WHERE time >= SYSDATE() - INTERVAL 15 DAY GROUP BY date ORDER BY time LIMIT 100";
$clientResult=mysql_query($query);
$activityPerDay = array();
while($row=mysql_fetch_array($clientResult, MYSQLI_ASSOC)){
  $activity[0] = $row['date'];
  $activity[1] = (int)$row['downloads'];
  $activityPerDay[] = $activity;
}
mysql_close();

$model = (object) array('clientCount' => $numClients, 'downloadCount' => $numDownloads, 'activityPerDay' => $activityPerDay);
header('Content-type: application/json');
echo json_encode($model, JSON_FORCE_OBJECT);
?>
