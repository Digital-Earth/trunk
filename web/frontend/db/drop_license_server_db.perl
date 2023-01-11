#!/usr/bin/perl -w

use DBI;
use lib '/home/pyxis/scripts';
use db_tools;
use common_license_server;

$db="pyxis_licenseserver";
$user="pyxis_licensing";
$pass="Innovation1";

$dbh=DBI->connect("DBI:mysql:$db", $user, $pass);

dropLSTables($dbh);

$sqlQuery->finish;
exit(0);
