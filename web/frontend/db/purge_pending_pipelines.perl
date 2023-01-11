#!/usr/bin/perl -w

use DBI;
use lib '/home/pyxis/scripts';
use db_tools;

$db="pyxis_licenseserver";
$user="pyxis_licensing";
$pass="Innovation1";

$dbh=DBI->connect("DBI:mysql:$db", $user, $pass);

$query="DELETE FROM PipelinePending WHERE Created < DATE_SUB(SYSDATE(), INTERVAL 45 MINUTE)";
$sqlQuery=executeQuery($dbh,$query);

$sqlQuery->finish;
$dbh->disconnect;

$db="pyxis_licenseserver_staging";

$dbh=DBI->connect("DBI:mysql:$db", $user, $pass);

$sqlQuery=executeQuery($dbh,$query);

print "*** Pending Pipelines Purged ***\n";

$sqlQuery->finish;
exit(0);
