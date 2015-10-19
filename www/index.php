<?php 

/* index.php
 * Ralf Becker 2005
 * $Id: index.php,v 1.1.2.2 2005/08/03 13:56:56 becker Exp $
 */

$NCTOPCONF="PATH/nctop.conf";
$CGIBIN="./";

?>
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 TRANSITIONAL//EN"
        "http://www.w3.org/TR/html4/loose.dtd">
<html> 
<head>
<meta http-equiv="content-type" content="$content">
<meta name="author"   content="Ralf Becker">
<meta name="keywords" content="nctop web monitor">

<?php
echo "<meta http-equiv=\"refresh\" content=\"600; URL=".$_SERVER["REQUEST_URI"]."\">";
?>

<title>NCTOP System Monitor</title>
</head>

<body>

<h2>NCTOP System Monitor</h2>
<!-- just a dummy -->
<table border="0" width="100%" cellpadding="1" bgcolor="#6d79c5">
<tr><td></td></tr>
</table>
<br>

<?php
/* read config file */
function readconfig($file) {

	$handle = @fopen($file, "r") or die ("$file: non-existent");

	$line = fgets($handle,4096);
	while (!feof($handle)) {
		$index=strlen($line)-1;
		$line = trim($line);
		if (($line{0} != '#') && (strlen($line) != 0)) {
			$hosts[] = $line;
		}
		$line = fgets($handle,4096);
	}

	fclose($handle);

	return $hosts;
}

$hosts = readconfig($NCTOPCONF);

/* print table */
echo "<table border=\"0\" width=\"100%\">\n";

/* toggle columns - rows */
$newline = 0;
foreach ($hosts as $host) {
	if (!$newline) {
		echo "<tr>\n";
	}
	echo "<td width=\"50%\" align=center>\n";
	echo "<img src=\"".$CGIBIN."plotstat.pl?HOST=$host\" alt=\"$host\">\n";
	echo "</td>\n";
	if ($newline) {
		echo "</tr>\n";
	}
	$newline = !$newline;
}
echo "</table>\n";
?>

<table width="100%">
<tr bgcolor="#6d79c5"><td colspan=3></td></tr>
<tr>
<td align="left" valign="middle" width="10%">
<a href="http://validator.w3.org/check/referer">
<img src="http://www.w3.org/Icons/valid-html401"
          alt="Valid HTML 4.01!" height="31" width="88" border="0"/>
</a>
</td>
<td align="right">

<?php
echo "Last Change: ".date( "d.m.Y", getlastmod() )."\n";
?>

<br>
&copy; 2005 <a href="mailto:nctop@web.de">Ralf Becker</a>
</td>
</tr>
</table>

</body>
</html>
