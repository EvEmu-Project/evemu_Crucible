#!/usr/bin/perl

open(IN, "<$ARGV[0]") || die ("Unable to open $ARGV[0]");

$pname = "";
$pdata = "";
while(<IN>) {
	chomp;
	s/\r//g;
	next unless(/^([0-9]+)\s+([0-9\.]{10})\s+([0-9:]{12})\s+/);
	my $line = $1;
	my $time = $2;
	s/^([0-9]+)\s+[0-9\.]{10}\s+[0-9:]{12}\s+//g;
	
	if(/Packet::([^s]+)\s(.*)$/) {
		if($pname ne "") {
			print_packet($pname, $pdata);
		}
		$pname = $1;
		$pdata = $2;
	} elsif($pname ne "" && /^- (.*)$/) {
		$pdata .= $1;
	} elsif($pname ne "") {
		#finished with this packet...
		print_packet($pname, $pdata);
		$pname = "";
		$pdata = "";
	}
}
close(IN);

sub print_packet {
	(my $pname, my $pdata) = @_;
	$pdata =~ s/\xFA//g;	#no idea why these are here.
	$pdata =~ s/\xF9//g;
	$pdata =~ s/\\x[0-9A-Fa-f]{2}/_/g;
	
	if($pdata =~ /\(Address::([A-Za-z]+)\(([^\)]+)\),Address::([A-Za-z]+)\(([^\)]+)\),(.*)/) {
		my $src_type = $1;
		my $src_args = $2;
		my $dest_type = $3;
		my $dest_args = $4;
		$pdata = $5;

		if($dest_type eq "BroadCast") {
			if($dest_args =~ /broadcastID="([^"]+)"/) {
				$pdata = "$1: $pdata";
			}
		}
	}
	print "$pname: $pdata\n";
}

