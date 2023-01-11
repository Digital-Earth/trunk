#!/usr/bin/perl -w

use DBI;
use lib '/home/pyxis/scripts';
use db_tools;

$db="pyxis_clientdownloads";
$user="pyxis_clientdb";
$pass="a!2Sd3\$f";

$dbh=DBI->connect("DBI:mysql:$db", $user, $pass);

$query="DROP TABLE clients";
$sqlQuery=executeQuery($dbh,$query);
$query="DROP TABLE downloads";
$sqlQuery=executeQuery($dbh,$query);
$query="DROP VIEW clientDownloads";
$sqlQuery=executeQuery($dbh,$query);
$query="DROP VIEW downloadDetails";
$sqlQuery=executeQuery($dbh,$query);

$sqlQuery->finish;
exit(0);
