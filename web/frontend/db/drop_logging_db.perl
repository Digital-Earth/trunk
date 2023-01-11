#!/usr/bin/perl -w

use DBI;
use lib '/home/pyxis/scripts';
use db_tools;

$db="pyxis_pyxnetlogging";
$user="pyxis_logging";
$pass="Innovation1";

$dbh=DBI->connect("DBI:mysql:$db", $user, $pass);

$query="DROP TABLE logging";
$sqlQuery=executeQuery($dbh,$query);

$sqlQuery->finish;
exit(0);
