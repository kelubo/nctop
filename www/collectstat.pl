#!/usr/bin/perl

# collectstat.pl
# Ralf Becker 2005
# $Id: collectstat.pl,v 1.1.2.1 2005/07/31 22:09:52 becker Exp $

use POSIX (strftime);

$LOGDIR="PATH";
$NCTOP="PATH/nctop";
$NCTOPCONF="PATH/nctop.conf";

sub getinfo () {

# get date and time
	$DATE=(strftime "%d%m%Y",localtime);
	$H=(strftime "%H",localtime);
	$M=(strftime "%M",localtime);
	
# log load
	open (INFO,"$NCTOP -f $NCTOPCONF -b 2|");
	while(<INFO>) {
		($HOSTNAME,$LOAD1,$LOAD2,$LOAD3,$MEMUSED,$MEM) = (split("\ ",$_))[0,1,2,3,7,9];
		$FILE=$LOGDIR."collectload.log";
		unless(open FOUT,">>".$FILE) { print "$FILE: $!"; return -1;};
		print FOUT $_." ".$DATE." ".$LH."\n";
		close FOUT;


		$FILE=">>".$LOGDIR.$HOSTNAME."_".$DATE;

		unless (open FOUT,$FILE) { print "$FILE: $!"; return -1; };
		if ($LOAD1 !~ /[0-9.]+/) {
			print FOUT "\n";
			print FOUT "\n";
		} else {
			print FOUT "$H:$M\t$LOAD1\t$LOAD2\t$LOAD3\t$MEMUSED\t$MEM\n";
		}
		close FOUT;
	}
}

# main
getinfo();
