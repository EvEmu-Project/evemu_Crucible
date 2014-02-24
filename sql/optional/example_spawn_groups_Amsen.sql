-- phpMyAdmin SQL Dump
-- version 3.5.2.2
-- http://www.phpmyadmin.net
--
-- Host: 127.0.0.1
-- Generation Time: Feb 24, 2014 at 06:36 AM
-- Server version: 5.5.27
-- PHP Version: 5.4.7

SET SQL_MODE="NO_AUTO_VALUE_ON_ZERO";
SET time_zone = "+00:00";


/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;

-- --------------------------------------------------------
--
-- Dumping data for table `spawnbounds`
--

INSERT INTO `spawnbounds` (`spawnID`, `pointIndex`, `x`, `y`, `z`) VALUES
(1, 0, 273625620480, -12937666560, -50608005120),
(2, 0, -1250080481280, 59107123200, -392342200320),
(3, 0, -377472737280, 17869086720, -72740904960),
(4, 0, 274126970880, -12959293440, -50991882240),
(5, 0, 127470673920, -6047784960, 130608046080),
(6, 0, 810284359680, 63690301440, -110746951680),
(7, 0, 4182116474880, 129673175040, 5585440235520);

-- --------------------------------------------------------
--
-- Dumping data for table `spawngroupentries`
--

INSERT INTO `spawngroupentries` (`spawnGroupID`, `npcTypeID`, `quantity`, `probability`, `ownerID`, `corporationID`) VALUES
(1, 13533, 3, 1, 1000124, 1000124),
(1, 22843, 2, 1, 1000124, 1000124),
(2, 22830, 4, 1, 1000124, 1000124),
(2, 22842, 1, 1, 1000124, 1000124),
(3, 11021, 2, 1, 1000124, 1000124),
(3, 22832, 3, 1, 1000124, 1000124),
(4, 10015, 5, 1, 1000124, 1000124),
(5, 22827, 3, 1, 1000124, 1000124),
(5, 22830, 2, 1, 1000124, 1000124),
(6, 22830, 3, 1, 1000124, 1000124),
(6, 26679, 1, 1, 1000124, 1000124),
(6, 28863, 1, 1, 1000124, 1000124),
(7, 22832, 2, 1, 1000124, 1000124),
(7, 26679, 1, 1, 1000124, 1000124),
(7, 26844, 1, 1, 1000124, 1000124),
(7, 28863, 1, 1, 1000124, 1000124);

-- --------------------------------------------------------
--
-- Dumping data for table `spawngroups`
--

INSERT INTO `spawngroups` (`spawnGroupID`, `spawnGroupName`, `formation`) VALUES
(1, 'Angel Cartel 2BS 3C', 0),
(2, 'Angel Cartel 1BS 4C', 0),
(3, 'Angel Cartel 3C 2F', 0),
(4, 'Angel Cartel 5F', 0),
(5, 'Angel Cartel 3BS 2C', 0),
(6, 'Angel Cartel Officer 2BS 3C', 0),
(7, 'Angel Cartel Officer 3BS 2C', 0);

-- --------------------------------------------------------
--
-- Dumping data for table `spawns`
--

INSERT INTO `spawns` (`spawnID`, `solarSystemID`, `spawnGroupID`, `spawnBoundType`, `spawnTimer`, `respawnTimeMin`, `respawnTimeMax`) VALUES
(1, 30001392, 1, 0, 300, 120, 360),
(2, 30001392, 2, 0, 300, 120, 360),
(3, 30001392, 3, 0, 300, 120, 360),
(4, 30001392, 4, 0, 300, 120, 360),
(5, 30001392, 5, 0, 300, 120, 360),
(6, 30001392, 6, 0, 300, 120, 360),
(7, 30001392, 7, 0, 300, 120, 360);

/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
