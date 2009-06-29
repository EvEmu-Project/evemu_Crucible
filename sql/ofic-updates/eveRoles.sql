/*Table structure for table `eveRoles` */

DROP TABLE IF EXISTS `eveRoles`;

CREATE TABLE `eveRoles` (
  `roleID` int(10) unsigned NOT NULL default '0',
  `roleName` varchar(100) NOT NULL default '',
  `description` mediumtext NOT NULL,
  PRIMARY KEY  (`roleName`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

/*Data for the table `eveRoles` */

insert  into `eveRoles`(`roleID`,`roleName`,`description`) values (536870912,'ACCOUNTMANAGEMENT','May manage user accounts'),(32,'ADMIN','Administrator'),(67108864,'BROADCAST','Broadcast messages in ESP'),(2048,'CENTURION','Role for senior volunteers (Polaris)'),(2097152,'CHTADMINISTRATOR','Administrate chat channels'),(1048576,'CHTINVISIBLE','Invisible in chat channels'),(1073741824,'DUNGEONMASTER','Design and master dungeons'),(16384,'EBS','Eve Backend System'),(1024,'GDH','Game Design High'),(512,'GDL','Game Design Low'),(4,'GDNPC','Game Design NPC'),(16,'GMH','Game Master High'),(8,'GML','Game Master Low'),(8388608,'HEALOTHERS','/heal on others'),(4194304,'HEALSELF','/heal on yourself'),(33554432,'HOSTING','Hardware hosting and administration'),(128,'HTTP','HTTP/ESP'),(262144,'LEGIONEER','Role for junior volunteers'),(1,'LOGIN',''),(268435456,'N00BIE','Newbie dude'),(16777216,'NEWSREPORTER','Forum and in-game news reporting role.'),(256,'PETITIONEE','Users that may handle petitions'),(2,'PLAYER',''),(65536,'PROGRAMMER','Role for executing Python code in the context of the server'),(8192,'QA','Quality Assurance'),(131072,'REMOTESERVICE',''),(32768,'ROLEADMIN','Administration of roles'),(64,'SERVICE',''),(4294967295,'SPAWN','Allowing /entity and /spawn'),(524288,'TRANSLATION','Permission to do translations in the translation system'),(134217728,'TRANSLATIONADMIN','Permission to do administer translations in the translation system'),(4294967295,'TRANSLATIONEDITOR','Approves or Rejects translations'),(4096,'WORLDMOD','World Modeling');

