-- MySQL dump 10.9
--
-- Host: localhost    Database: psotnic
-- ------------------------------------------------------
-- Server version	4.1.10-standard

--
-- Table structure for table `seen`
--

DROP TABLE IF EXISTS `seen`;
CREATE TABLE `seen` (
  `nick` varchar(30) NOT NULL default '',
  `chan` varchar(30) NOT NULL default '',
  `host` varchar(100) NOT NULL default '',
  `part` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  PRIMARY KEY  (`nick`,`chan`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

