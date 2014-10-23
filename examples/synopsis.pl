#!/home/ben/software/install/bin/perl
use warnings;
use strict;
use Text::Fuzzy;
my $tf = Text::Fuzzy->new ('boboon');
print "Distance is ", $tf->distance ('babboon'), "\n";
# Prints "Distance is 2"
my @words = qw/the quick brown fox jumped over the lazy dog/;
my $nearest = $tf->nearest (\@words);
print "Nearest array entry is ", $words[$nearest], "\n";
# Prints "Nearest array entry is brown"

