-- MySQL dump 10.9
--
-- Host: localhost    Database: psotnic
-- ------------------------------------------------------
-- Server version	4.1.10-standard

--
-- Table structure for table `explains`
--

DROP TABLE IF EXISTS `explains`;
CREATE TABLE `explains` (
  `word` varchar(30) NOT NULL default '',
  `chan` varchar(30) NOT NULL default '',
  `who` varchar(30) NOT NULL default '',
  `added` timestamp NOT NULL default CURRENT_TIMESTAMP on update CURRENT_TIMESTAMP,
  `detail` text NOT NULL,
  PRIMARY KEY  (`word`,`chan`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

