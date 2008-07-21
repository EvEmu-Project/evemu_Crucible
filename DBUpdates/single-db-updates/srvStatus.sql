CREATE TABLE `srvStatus` (
	`config_name` VARCHAR( 32 ) NOT NULL ,
	`config_value` VARCHAR( 64 ) NOT NULL ,
	UNIQUE (`config_name`)
) ENGINE = innodb;

INSERT INTO `srvStatus` ( `config_name` , `config_value` ) VALUES ('startTime', 0 );
