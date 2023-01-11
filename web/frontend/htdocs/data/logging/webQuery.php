<?php 
session_start();
if(!isset($_SESSION['authorized']))
        die("Unauthorized");
include '/home/pyxis/scripts/webuserHeader.php';
$query=$_POST["query"];
$query=trim($query);
if(!isset($query))
	die("No query");
if(strcasecmp(substr($query, 0, 6), "SELECT"))
	die("Insufficient privileges");
@mysql_select_db($database) or die( "Unable to select database");
$result=mysql_query($query);
$fields=mysql_num_fields($result);
$columnNum=0;
for ($i = 0; $i < $fields; $i++)
{
  $column=mysql_field_name($result, $i);
  if(strcmp($column,"ind"))
  {
    $columns[(int)$columnNum]=$column;
    $columnNum += 1;
  }
}
$num=mysql_numrows($result);
mysql_close();
$i=0;
while ($i < $num) {
  $record=array();
  for ($j = 0; $j < $columnNum; $j++)
  {
    $record[$j]=mysql_result($result,$i,$columns[$j]);
  }
  $model[]=(object)$record;
  $i++;
}
header('Content-type: application/json');
$data['columns']=$columns;
$data['model']=$model;
echo json_encode($data);
?>
