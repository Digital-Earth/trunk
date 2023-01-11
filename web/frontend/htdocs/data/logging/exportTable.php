<?php 
session_start();
if(!isset($_SESSION['authorized']))
        die("Unauthorized");
$table=$_GET["table"];
if(!isset($table))
	return;
include '/home/pyxis/scripts/webuserHeader.php';
@mysql_select_db($database) or die( "Unable to select database");
$query="SELECT * FROM $table";
$export = mysql_query ( $query ) or die ( "Sql error : " . mysql_error( ) );
$fields = mysql_num_fields ( $export );

for ( $i = 0; $i < $fields; $i++ )
{
    $header .= mysql_field_name( $export , $i ) . ",";
}
$header = rtrim($header, ",");
while( $row = mysql_fetch_row( $export ) )
{
    $line = '';
    foreach( $row as $value )
    {                                            
        if ( ( !isset( $value ) ) || ( $value == "" ) )
        {
            $value = ",";
        }
        else
        {
            $value = str_replace( '"' , '""' , $value );
            $value = '"' . $value . '"' . ",";
        }
        $line .= $value;
    }
    $line = rtrim($line, ",");
    $data .= trim( $line ) . "\r\n";
}

if ( $data == "" )
{
    $data = "\n(0) Records Found!\n";                        
}

header("Content-type: application/octet-stream");
header("Content-Disposition: attachment; filename=$table.csv");
header("Pragma: no-cache");
header("Expires: 0");
print "$header\r\n$data";
?>
