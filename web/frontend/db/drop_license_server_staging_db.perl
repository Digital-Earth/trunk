#!/usr/bin/perl -w

use DBI;
use lib '/home/pyxis/scripts';
use db_tools;
use common_license_server;

$db="pyxis_licenseserver_staging";
$user="pyxis_licensing";
$pass="Innovation1";

$dbh=DBI->connect("DBI:mysql:$db", $user, $pass);

dropLSTables($dbh);

exit(0);
