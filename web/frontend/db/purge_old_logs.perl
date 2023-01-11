#!/usr/bin/perl -w

use DBI;
use lib '/home/pyxis/scripts';
use db_tools;

$db="pyxis_pyxnetlogging";
$user="pyxis_logging";
$pass="Innovation1";

$dbh=DBI->connect("DBI:mysql:$db", $user, $pass);

$query="DELETE FROM logging WHERE time < DATE_SUB(SYSDATE(), INTERVAL 1 MONTH)";
$sqlQuery=executeQuery($dbh,$query);

print "*** Logs Purged ***\n";

$sqlQuery->finish;
exit(0);
