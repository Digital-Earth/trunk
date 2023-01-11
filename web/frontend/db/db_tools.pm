package db_tools;

require Exporter;
@ISA = qw(Exporter);
@EXPORT = qw(executeQuery addSlashes);

sub executeQuery {
  $dbh=$_[0];
  $query=$_[1];
  $sqlQuery=$dbh->prepare($query) or die "Can't prepare $query: $dbh->errstr\n";
  $sqlQuery->execute or die "Can't execute $query: $sqlQuery->errstr\n";
  return $sqlQuery;
}

sub addSlashes {
  $text = shift;
  ## Make sure to do the backslash first!
  $text =~ s/\\/\\\\/g;
  $text =~ s/'/\\'/g;
  $text =~ s/"/\\"/g;
  $text =~ s/\\0/\\\\0/g;
  return $text;
}

1;
