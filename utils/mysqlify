#!/usr/bin/perl

open(F, "<$ARGV[0]") || die "Unable to open input file arg '$ARGV[0]'";
open(O, ">mysql/".$ARGV[0]) || die "Unable to open output file arg '$ARGV[0]'";
while(<F>) {
	s/dbo\.//g;
	s/([^a-zA-Z])true([^a-zA-Z])/${1}1${2}/g;
	s/([^a-zA-Z])false([^a-zA-Z])/${1}0${2}/g;
	s/([^a-zA-Z])true([^a-zA-Z])/${1}1${2}/g;
	s/([^a-zA-Z])false([^a-zA-Z])/${1}0${2}/g;

	#run twice for repeated NULLs.
	s/,,/,NULL,/g;
	s/,\)/,NULL)/g;
	s/\(,/(NULL,/g;
	s/,,/,NULL,/g;
	s/,\)/,NULL)/g;
	s/\(,/(NULL,/g;

	s/CHAR\(100\)/VARCHAR(100)/g;
	s/CHAR\(20\)/VARCHAR(20)/g;
	s/CHAR\(50\)/VARCHAR(50)/g;
	s/CHAR\(60\)/VARCHAR(60)/g;
	s/CHAR\(255\)/VARCHAR(255)/g;
	s/CHAR\(400\)/TEXT/g;
	s/CHAR\(1000\)/TEXT/g;
	s/CHAR\(3000\)/TEXT/g;
	s/CHAR\(4000\)/TEXT/g;
	s/CHAR\(7000\)/TEXT/g;
        s/UNKNOWN/INTEGER/g;

	if(/CREATE TABLE (.*)\(/) {
		print O "DROP TABLE IF EXISTS $1;\r\n";
	}
	print O;
}
close(O);
close(F);
