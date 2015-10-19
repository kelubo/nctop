#!/usr/bin/perl -w

# plotstat.pl
# Ralf Becker 2005
# $Id: plotstat.pl,v 1.1.2.1 2005/07/31 22:09:52 becker Exp $

use strict;

use POSIX qw(strftime tmpnam unlink ceil floor);
use CGI qw(param);

my $LOGDIR;
$LOGDIR="PATH";

my (@KNOWNHOSTS,$HOST);

sub log10($) {

	my $ln=2.30258509299404568401;

	return(log($_[0])/$ln);
}

sub log1024($) {

	my $ln=6.93147180559945309417;

	return(log($_[0])/$ln);
}

sub frac($) {

	return($_[0]-floor($_[0]));
}

sub plot($) {
	
	my $HOSTNAME = $_[0];
	my ($TMP, $LOG);
	my $DATE;
	my ($LOAD1,$LOAD2,$LOAD3,$YMAX,$Y2MAX,$MEM,$Y1,$Y2,$Y2TICS,$Y2LABEL);
	my ($FKTCMD,$PLTCMD);
	my ($EXP,$MR,$BASE1,$BASE2);
	my $N;

	$TMP=tmpnam();
	$DATE=(strftime "%d%m%Y",localtime);

	$LOG=$LOGDIR.$HOSTNAME."_".$DATE;

	# set y-range
	$YMAX=1;
	$Y2MAX=1;
	unless(open FIN,"<".$LOG) { print STDERR "$LOG: $!"; return -1;};
	while (<FIN>) {
		($LOAD1,$LOAD2,$LOAD3,$MEM) = (split("\ ",$_))[1,2,3,5];
		if ($LOAD1 > $YMAX) {
			$YMAX=$LOAD1;
		}
		if ($MEM > $Y2MAX) {
			$Y2MAX=$MEM;
		}
	}
	close FIN;

	# set marker lines
	$FKTCMD="";
	$PLTCMD="";
	if ($YMAX > 1) {
		for ($N=1; $N < $YMAX; ++$N) {
			$FKTCMD=$FKTCMD."; f".$N."(x) = ".$N;
			$PLTCMD=$PLTCMD.", f".$N."(x) notitle with l 0";
		}
		$YMAX="\*";
	}

	# calculate y2tics
	
	$MR=floor(log1024($Y2MAX));

	SWITCH: {
		if ($MR == 0) 
			{ $Y2LABEL="KB"; $BASE1=1; last SWITCH };
		if ($MR == 1) 
			{ $Y2LABEL="MB"; $BASE1=1024; last SWITCH };
		if ($MR >= 2) 
			{ $Y2LABEL="GB"; $BASE1=(1024*1024); last SWITCH };
	}

	$_=log10($Y2MAX/5/$BASE1);

	$EXP=10**ceil($_);
	$BASE2=10**frac($_);

	SWITCH: {
		if ($BASE2 <= 1.5)
			{ $BASE2 = 1.0; last SWITCH };
		if ((1.5 < $BASE2) && ($BASE2  <= 2.1))
			{ $BASE2 = 2.0; last SWITCH };
		if ((2.1 < $BASE2) && ($BASE2 <= 3.5))
			{ $BASE2 = 2.5; last SWITCH };
		if ((3.5 < $BASE2) && ($BASE2 <= 7.0))
			{ $BASE2 = 5; last SWITCH };
		if (7.0 < $BASE2)
			{ $BASE2 = 1.0; last SWITCH };
	}

	print STDERR $HOSTNAME." ".$BASE1."\n";
	print STDERR $HOSTNAME." ".$BASE2."\n";
	$BASE2=$BASE2*$EXP;
	$BASE1=$BASE1*$BASE2;

	if ($BASE1 > $Y2MAX) {
		$BASE1/=10;
		$BASE2/=10;
	}

	$Y2TICS="( \'".$BASE2."\' ".$BASE1;
	for ($N=2; $N <= 6; $N++) {
		$Y2TICS.=", \'".$N*$BASE2."\' ".$N*$BASE1." ";
	}
	$Y2TICS.=")";

	print STDERR $HOSTNAME." ".$Y2MAX."\n";
	print STDERR $HOSTNAME." ".$EXP."\n";
	print STDERR $HOSTNAME." ".$BASE1."\n";
	print STDERR $HOSTNAME." ".$BASE2."\n";
	print STDERR $HOSTNAME." ".$Y2TICS."\n";
	
	# gnuplot output
	open FOUT,">".$TMP or die "$TMP: $!";
	print FOUT "set terminal png small\n";
	print FOUT "set output\n";
	print FOUT "\n";
	print FOUT "set title \"".$HOSTNAME."\" 0.000000,0.000000  \n";
	print FOUT "set size 0.7\n";
	print FOUT "\n";
	print FOUT "set xdata time\n";
	print FOUT "set format x \"%H.%M\"\n";
	print FOUT "set key\n";
	print FOUT "set xlabel \"time\" 0.000000,0.000000  \n";
	print FOUT "set timefmt \"%H:\%M\"\n";
	print FOUT "set xrange [\"00:00\":\"23:59\"] noreverse nowriteback\n";
	print FOUT "\n";
	print FOUT "set yrange [ 0.00000 : ".$YMAX." ] noreverse nowriteback\n";
	print FOUT "set y2range [0:".$Y2MAX."]\n";
	print FOUT "set ylabel \"load\" 0.000000,0.000000  \n";
	print FOUT "set y2label \"memory [".$Y2LABEL."]\" 0.000000,0.000000  \n";
	print FOUT "set ytics nomirror\n";
	print FOUT "set y2tics ".$Y2TICS."\n";
	print FOUT "\n";
	print FOUT $FKTCMD."\n";
	print FOUT "plot \"".$LOG."\" using 1:2 title \"load\" with linespoints".$PLTCMD.", \"".$LOG."\" using 1:5 axes x1y2 title \"mem\" w lp lt 2 ";
	#print FOUT "set output \"".$PNG."\"\n";
	#print FOUT "set size 1.5\n";
	#print FOUT "replot\n";
	close FOUT;

	# flush stdout
	$|=1;
	print STDOUT "Content-type: image/png\n\n";
	system("/home/local/becker/bin/gnuplot $TMP");

	unlink $TMP;

	return 0;
}

# empty plot
sub plotdummy($) {

	my $HOSTNAME = $_[0];
	my $TMP;
	
	$TMP=tmpnam();
	open FOUT,">".$TMP or die "$TMP: $!";
	print FOUT "set terminal png small\n";
	print FOUT "set output\n";
	print FOUT "\n";
	print FOUT "set title \"".$HOSTNAME."\" 0.000000,0.000000  \n";
	print FOUT "set size 0.7\n";
	print FOUT "\n";
	print FOUT "set xdata time\n";
	print FOUT "set format x \"%H.%M\"\n";
	print FOUT "set key\n";
	print FOUT "set xlabel \"time\" 0.000000,0.000000  \n";
	print FOUT "set timefmt \"%H:\%M\"\n";
	print FOUT "set xrange [\"00:00\":\"23:59\"] noreverse nowriteback\n";
	print FOUT "\n";
	print FOUT "set yrange [ 0.00000 : 1.0 ] noreverse nowriteback\n";
	print FOUT "set y2range [ 0.00000 : 1048576 ] noreverse nowriteback\n";
	print FOUT "set ylabel \"load\" 0.000000,0.000000  \n";
	print FOUT "set y2label \"memory [GB]\" 0.000000,0.000000  \n";
	print FOUT "set ytics nomirror\n";
	print FOUT "set y2tics ('0.5' 524288,'1.0' 1048576,'1.5' 1572864,'2.0'  2097152,'2.5' 2621440,'3.0' 3145728,'3.5' 3670016,'4.0' 4194304, '4.5' 4718592, '5.0' 5242880)\n";
	print FOUT "set label \"no data available\" at \"9:00\",0.5\n";
	print FOUT "\n";
	print FOUT "plot 0\n";
	close FOUT;

	# flush stdout
	$|=1;
	print STDOUT "Content-type: image/png\n\n";
	system("/home/local/becker/bin/gnuplot $TMP");

	unlink $TMP;
}

# main
$HOST = param("HOST");

if (plot($HOST) eq -1) {
	plotdummy($HOST);
}

exit 0;
