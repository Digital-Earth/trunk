#!/usr/bin/perl -w

use DBI;
use lib '/home/pyxis/scripts';
use db_tools;

$db="pyxis_pyxnetlogging";
$user="pyxis_logging";
$pass="Innovation1";

$dbh=DBI->connect("DBI:mysql:$db", $user, $pass);

$query="CREATE TABLE IF NOT EXISTS logging (ind INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(ind), time DATETIME, id VARCHAR(36) NOT NULL, ip VARCHAR(39) NOT NULL, name VARCHAR(1024) NOT NULL, cat VARCHAR(1024) NOT NULL, `key` VARCHAR(1024) NOT NULL, val TEXT)";
$sqlQuery=executeQuery($dbh,$query);

$query="CREATE INDEX logging_time_idx ON logging (time);";
$sqlQuery=executeQuery($dbh,$query);

$query="SHOW TABLES";
$sqlQuery=executeQuery($dbh,$query);

print "*** Perl DBI Test ***\n";
while (@row=$sqlQuery->fetchrow_array()) {
	my $tables = $row[0];
	print "$tables\n";
}

$query="SHOW INDEX FROM logging FROM pyxis_pyxnetlogging";
$sqlQuery=executeQuery($dbh,$query);

print "INDEXES\n";
while (@row=$sqlQuery->fetchrow_array()) {
        my $tables = $row[0];
        print "$tables\n";
}


$sqlQuery->finish;
exit(0);
