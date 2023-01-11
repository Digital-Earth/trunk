#!/usr/bin/perl -w

use DBI;
use lib '/home/pyxis/scripts';
use db_tools;

$db="pyxis_clientdownloads";
$user="pyxis_clientdb";
$pass="a!2Sd3\$f";

$dbh=DBI->connect("DBI:mysql:$db", $user, $pass);

$query="CREATE TABLE IF NOT EXISTS clients (id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id), uuid VARCHAR(36), client VARCHAR(250) NOT NULL, first VARCHAR(100), last VARCHAR(100), position VARCHAR(250), email VARCHAR(250), phone VARCHAR(250))";
$sqlQuery=executeQuery($dbh,$query);

$query="CREATE TABLE IF NOT EXISTS downloads (id INT NOT NULL AUTO_INCREMENT, PRIMARY KEY(id), clientID INT, time TIMESTAMP DEFAULT CURRENT_TIMESTAMP, ip CHAR(15), country VARCHAR(128), region VARCHAR(128), city VARCHAR(128), lat DOUBLE, lon DOUBLE, userAgent TEXT)";
$sqlQuery=executeQuery($dbh,$query);

$query="CREATE VIEW clientDownloads AS SELECT client, uuid, COUNT(downloads.clientID) AS numDownloads FROM clients LEFT JOIN downloads ON (clients.id = downloads.clientID) GROUP BY clients.id";
$sqlQuery=executeQuery($dbh,$query);

$query="CREATE VIEW downloadDetails AS SELECT clients.client AS client, downloads.time AS time, downloads.ip AS ip, downloads.country as country, downloads.region as region, downloads.city as city, downloads.lat as lat, downloads.lon as lon, downloads.userAgent as userAgent FROM clients INNER JOIN downloads ON clients.id = downloads.clientID;";
$sqlQuery=executeQuery($dbh,$query);

$query="INSERT INTO clients (client,uuid) VALUES ('Unregistered', uuid())";
$sqlQuery=executeQuery($dbh,$query);

$query="INSERT INTO clients (client,uuid,first,last,position,email,phone) VALUES ('PYXIS', 'cb640524-df44-11e2-9cdd-002590a57c44', 'Perry', 'Peterson', 'President', 'info@pyxisinnovation.com', '613-389-6619')"; 

$query="SHOW TABLES";
$sqlQuery=executeQuery($dbh,$query);

print "*** Perl DBI Test ***\n";
while (@row=$sqlQuery->fetchrow_array()) {
	my $tables = $row[0];
	print "$tables\n";
}

$sqlQuery->finish;
exit(0);
