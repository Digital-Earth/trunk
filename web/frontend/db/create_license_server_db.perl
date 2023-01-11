#!/usr/bin/perl -w

use DBI;

print "Content-type: text/html\n\n";
use lib '/home/pyxis/scripts';
use db_tools;
use common_license_server;

$db="pyxis_licenseserver";
$user="pyxis_licensing";
$pass="Innovation1";

$dbh=DBI->connect("DBI:mysql:$db", $user, $pass);

createLSTables($dbh);

$query="SHOW TABLES";
$sqlQuery=executeQuery($dbh,$query);

print "*** Perl DBI Test ***\n";
while (@row=$sqlQuery->fetchrow_array()) {
	my $tables = $row[0];
	print "$tables\n";
}

$sqlQuery->finish;
exit(0);
