#!/usr/bin/perl -w

use DBI;
use LWP::UserAgent qw( );
use URI::Escape    qw( uri_escape );
use lib '/home/pyxis/scripts/perl-lib';
use JSON;
use lib '/home/pyxis/scripts';
use db_tools;

$db="pyxis_clientdownloads";
$user="pyxis_clientdb";
$pass="a!2Sd3\$f";

$dbh=DBI->connect("DBI:mysql:$db;mysql_multi_statements=1", $user, $pass);

$query="SELECT ip, id FROM downloads WHERE country IS NULL";
$sqlQuery=executeQuery($dbh,$query);

my $insert = ""; 
my $count = 1;
while (@row=$sqlQuery->fetchrow_array()) {
	my $ip = $row[0];
  my $id = $row[1];

  print "Processing record ", $count, " IP ", $ip, "\n";
  my $ua = LWP::UserAgent->new();
  my $response = $ua->get(sprintf('http://freegeoip.net/json/%s', uri_escape($ip)));

  my $location;
  if (!$response->is_success()) {
    print $response->status_line(), "\n";
    $location{ 'country_name' } = '-';
    $location{ 'region_name' } = '-';
    $location{ 'city' } = '-';
    $location{ 'latitude' } = '0';
    $location{ 'longitude' } = '0';
  }
  else {
    $location = decode_json $response->content();
  }
  $insert .= sprintf("UPDATE downloads SET country='%s', region='%s', city='%s', lat=%lf, lon=%lf WHERE id=%d;\n", addSlashes($location->{country_name}), addSlashes($location->{region_name}), addSlashes($location->{city}), addSlashes($location->{latitude}), addSlashes($location->{longitude}), $id);
  $count++;
}
if ($insert ne "") {
$insertQuery=executeQuery($dbh,$insert);
}
$sqlQuery->finish;

print "Processing complete\n";

exit(0);
