<?php
/************************************
 * EVE Emulator: Server Status Script
 * Copyright 2008
 * No license - do whatever you want with it
 * ------------------
 * status.php
 * Began: Wed Apr 2 2008
 *
 * $Id: status.php,v 1.03 2008/04/02 19:18:17 john adams Exp $
 *
 * Author Notes:
 * See the comments at the end of this script for SQL queries and special instructions.
 ************************************/

// You MUST set these up, or the script will die.
$database = array(
'host'=>'',					// Your DNS hostname or IP address
'user'=>'',					// MySQL User account with access to SELECT on your Eve database
'password'=>'',			// MySQL Password
'db'=>''						// Name of your EVE Emulator database
);
foreach($database as $db_check) {
	if( $db_check=="" ) die("CHANGE YOUR DB CONFIGS!");
}

// Init the database connection
$db = mysql_connect($database['host'], $database['user'], $database['password']); mysql_select_db($database['db']);

// get current status - Need to have Johnsus' serverStartTime patch applied
$query="select config_value as StartTime from srvStatus where config_name = 'serverStartTime';";
if($result=mysql_query($query,$db)) {
	$row=mysql_fetch_array($result);
	if( $row['StartTime'] ) {
		// might need to do some local time translations if your server is in a different timezone than your webhost
		$time=time()-$row['StartTime'];
		// probably a better way to dice up the seconds since 1970... but I was in a hurry
		$weeks=$time/604800;
		$days=($time%604800)/86400;
		$hours=(($time%604800)%86400)/3600;
		$minutes=((($time%604800)%86400)%3600)/60;
		$seconds=(((($time%604800)%86400)%3600)%60);

		$uptime='';
		if(intval($days)) 		$uptime .= ( intval($days)>1 ) 		? intval($days)		. " Days " 		: intval($days)	. " Day "; 
		if(intval($hours)) 	$uptime .= ( intval($hours)>1 ) 		? intval($hours)		. " Hours " 	: intval($hours)	. " Hour ";
		if(intval($minutes)) $uptime .= ( intval($minutes)>1 )	? intval($minutes)	. " Minutes " : intval($minutes)	. " Minutes ";
		if(!intval($minutes)&&!intval($hours)&&!intval($days)) $uptime.=" ".intval($seconds)." Seconds";
		//printf("Time: %s<br>Weeks: %s<br>Days: %s<br>Hours: %s<br>Minutes: %s<br>Seconds: %s",time(),$weeks,$days,$hours,$minutes,$seconds);
	} else {
		$uptime="Offline";
		$offline=true;
	}
} else {
	die("Horrible SQL error here!");
}

// get count of active players - Need to have Johnsus' Online Player patch applied
if( !$offline ) {
	$query="select count(Online) as online from character_ where Online = 1;";
	if( $result=mysql_query($query,$db) ) {
		$row=mysql_fetch_array($result);
		$num_players = $row['online'];
	} else {
		die("Horrible SQL error here!");
	}
}
// Start the server status table, showing server uptime and player count
print('<table align="center" cellspacing="2" border="1" width="60%">');
printf('<tr><td colspan="6" align="center"><strong>Server Uptime:</strong> %s</td></tr>',$uptime);

// If there is at least 1 player online, draw the Player Status stuff
if( $num_players || !$offline ) {
	printf('<tr><td colspan="6" align="center"><strong>Players Online:</strong> %s</td></tr>',$num_players);	

	print('
		<tr>
			<td align="left" width="1%">&nbsp;</td>
			<td align="left" width="20%">&nbsp;<strong>Name</strong></td>
			<td align="center" width="10%">&nbsp;<strong>Race</strong></td>
			<td align="center" width="40%">&nbsp;<strong>Corporation</strong></td>
			<td align="right" width="15%"><strong>SP</strong>&nbsp;</td>
			<td align="right" width="15%"><strong>Security</strong>&nbsp;</td>
		</tr>');
	$query="select characterID,characterName,raceName,securityRating,corporationName
					from character_ c
					join invTypes i on c.typeID = i.typeID
					join chrRaces r on i.raceID = r.raceID
					join corporation co on c.corporationID = co.corporationID
					where Online=1;";
	if($result=mysql_query($query,$db)) {
		while($row=mysql_fetch_array($result)) {
			print('<tr>');
			printf('<td>&nbsp;</td>');
			printf('<td>&nbsp;%s</td>',$row['characterName']);
			printf('<td>&nbsp;%s</td>',$row['raceName']);
			printf('<td>&nbsp;%s</td>',$row['corporationName']);
			printf('<td align="right">%s&nbsp;</td>',getSkillPoints($row['characterID']));
			printf('<td align="right">%s&nbsp;</td>',number_format($row['securityRating'],2));
			print('</tr>');
		}
	} else {
		die("Horrible SQL error here!");
	}
}
print("</table>");
mysql_free_result($result);


function getSkillPoints($id) {
	global $db;
	$sp="select SUM(valueInt) as SP from entity e 
				left join entity_attributes ea on e.itemID = ea.itemID 
				where attributeID = 276 and ownerID = ".$id;
	if($result=mysql_query($sp,$db)) {
		return ( $row=mysql_fetch_array($result) ) ? number_format($row['SP']) : 0;
	} else {
		die("something went wrong");
	}
	mysql_free_result($result);	
}


/************************************

	For this script to be of any value to you, you must apply both the ServerStartTime and CharacterOnline mods by Johnsus (Johns Adams).
	During the mod applications, you are instructed to create a new table (srvStatus) and alter the character_ table adding the Online field.
	If you do not do run these queries, this script will fail.
	If you do not apply the above mentioned mods, this script is useless.

	--
	-- [Step 1] Create the new table to hold various configuration values
	--
	CREATE TABLE `srvStatus` (
		`config_name` VARCHAR( 32 ) NOT NULL ,
		`config_value` VARCHAR( 32 ) NOT NULL ,
		UNIQUE (`config_name`)
	) ENGINE = innodb;


	--
	-- [Step 2] Inserts the first configuration name and value
	--
	INSERT INTO `srvStatus` VALUES ('serverStartTime','0');


	--
	-- [Step 3] Alters the character_ table for the Character Online mod
	--
	ALTER TABLE `character_` ADD `Online` TINYINT(1) NOT NULL DEFAULT '0';
	
*************************************/
?>