<?php 
include '/home/pyxis/scripts/loggingHeader.php';

$json=json_decode(file_get_contents('php://input'),true);
if(!isset($json))
	die("No payload");

@mysql_select_db($database) or die( "Unable to select database");
$query="INSERT INTO logging (time,id,ip,name,cat,`key`,val) VALUES ";
foreach ($json as $record)
{
    if(stripos($record['name'], 'gwss') === false && stripos($record['name'], 'hub') === false)
	$query .= "('{$record['time']}', '{$record['id']}', '{$_SERVER['REMOTE_ADDR']}', '{$record['name']}', '{$record['cat']}', '{$record['key']}', '{$record['val']}'),";
}
$query = substr($query, 0, -1);
$clientResult=mysql_query($query);
if(!$clientResult)
{
        die("Error: " . mysql_error($conn));
}
?>
