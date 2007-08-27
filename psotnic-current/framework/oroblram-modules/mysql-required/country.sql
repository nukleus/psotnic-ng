-- MySQL dump 10.9
--
-- Host: localhost    Database: psotnic
-- ------------------------------------------------------
-- Server version	4.1.10-standard

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `country`
--

DROP TABLE IF EXISTS `country`;
CREATE TABLE `country` (
  `iso` char(2) default NULL,
  `tld` varchar(10) NOT NULL default '',
  `name` varchar(50) NOT NULL default ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

--
-- Dumping data for table `country`
--


/*!40000 ALTER TABLE `country` DISABLE KEYS */;
LOCK TABLES `country` WRITE;
INSERT INTO `country` VALUES ('af','af','Afghanistan'),('ax','ax','Aland Islands'),('al','al','Albania'),('dz','dz','Algeria'),('as','as','American Samoa'),('ad','ad','Andorra'),('ao','ao','Angola'),('ai','ai','Anguilla'),('aq','aq','Antarctica'),('ag','ag','Antigua and Barbuda'),('ar','ar','Argentina'),('am','am','Armenia'),('aw','aw','Aruba'),('au','au','Australia'),('at','at','Austria'),('az','az','Azerbaijan'),('bs','bs','Bahamas'),('bh','bh','Bahrain'),('bd','bd','Bangladesh'),('bb','bb','Barbados'),('by','by','Belarus'),('be','be','Belgium'),('bz','bz','Belize'),('bj','bj','Benin'),('bm','bm','Bermuda'),('bt','bt','Bhutan'),('bo','bo','Bolivia'),('ba','ba','Bosnia and Herzegovina'),('bw','bw','Botswana'),('bv','bv','Bouvet Island'),('br','br','Brazil'),('io','io','British Indian Ocean Territory'),('bn','bn','Brunei Darussalam'),('bg','bg','Bulgaria'),('bf','bf','Burkina Faso'),('bi','bi','Burundi'),('kh','kh','Cambodia'),('cm','cm','Cameroon'),('ca','ca','Canada'),('cv','cv','Cape Verde'),('ky','ky','Cayman Islands'),('cf','cf','Central African Republic'),('td','td','Chad'),('cl','cl','Chile'),('cn','cn','China'),('cx','cx','Christmas Island'),('cc','cc','Cocos (Keeling) Islands'),('co','co','Colombia'),('km','km','Comoros'),('cg','cg','Republic of Congo'),('cd','cd','The Democratic Republic of the Congo'),('ck','ck','Cook Islands'),('cr','cr','Costa Rica'),('ci','ci','Cote d\'Ivoire'),('hr','hr','Croatia/Hrvatska'),('cu','cu','Cuba'),('cy','cy','Cyprus'),('cz','cz','Czech Republic'),('dk','dk','Denmark'),('dj','dj','Djibouti'),('dm','dm','Dominica'),('do','do','Dominican Republic'),('ec','ec','Ecuador'),('eg','eg','Egypt'),('sv','sv','El Salvador'),('gq','gq','Equatorial Guinea'),('er','er','Eritrea'),('ee','ee','Estonia'),('et','et','Ethiopia'),('fk','fk','Falkland Islands (Malvinas)'),('fo','fo','Faroe Islands'),('fj','fj','Fiji'),('fi','fi','Finland'),('fr','fr','France'),('gf','gf','French Guiana'),('pf','pf','French Polynesia'),('tf','tf','French Southern Territories'),('ga','ga','Gabon'),('gm','gm','Gambia'),('ge','ge','Georgia'),('de','de','Germany'),('gh','gh','Ghana'),('gi','gi','Gibraltar'),('gr','gr','Greece'),('gl','gl','Greenland'),('gd','gd','Grenada'),('gp','gp','Guadeloupe'),('gu','gu','Guam'),('gt','gt','Guatemala'),('gn','gn','Guinea'),('gw','gw','Guinea-Bissau'),('gy','gy','Guyana'),('ht','ht','Haiti'),('hm','hm','Heard Island and McDonald Islands'),('va','va','Vatican City State'),('hn','hn','Honduras'),('hk','hk','Hong Kong'),('hu','hu','Hungary'),('is','is','Iceland'),('in','in','India'),('id','id','Indonesia'),('ir','ir','Islamic Republic of Iran'),('iq','iq','Iraq'),('ie','ie','Ireland'),('il','il','Israel'),('it','it','Italy'),('jm','jm','Jamaica'),('jp','jp','Japan'),('jo','jo','Jordan'),('kz','kz','Kazakhstan'),('ke','ke','Kenya'),('ki','ki','Kiribati'),('kp','kp','Democratic People\'s Republic of Korea'),('kr','kr','Republic of Korea'),('kw','kw','Kuwait'),('kg','kg','Kyrgyzstan'),('la','la','Lao People\'s Democratic Republic'),('lv','lv','Latvia'),('lb','lb','Lebanon'),('ls','ls','Lesotho'),('lr','lr','Liberia'),('ly','ly','Libyan Arab Jamahiriya'),('li','li','Liechtenstein'),('lt','lt','Lithuania'),('lu','lu','Luxembourg'),('mo','mo','Macao'),('mk','mk','The Former Yugoslav Republic of Macedonia'),('mg','mg','Madagascar'),('mw','mw','Malawi'),('my','my','Malaysia'),('mv','mv','Maldives'),('ml','ml','Mali'),('mt','mt','Malta'),('mh','mh','Marshall Islands'),('mq','mq','Martinique'),('mr','mr','Mauritania'),('mu','mu','Mauritius'),('yt','yt','Mayotte'),('mx','mx','Mexico'),('fm','fm','Federated States of Micronesia'),('md','md','Republic of Moldova'),('mc','mc','Monaco'),('mn','mn','Mongolia'),('ms','ms','Montserrat'),('ma','ma','Morocco'),('mz','mz','Mozambique'),('mm','mm','Myanmar'),('na','na','Namibia'),('nr','nr','Nauru'),('np','np','Nepal'),('nl','nl','Netherlands'),('an','an','Netherlands Antilles'),('nc','nc','New Caledonia'),('nz','nz','New Zealand'),('ni','ni','Nicaragua'),('ne','ne','Niger'),('ng','ng','Nigeria'),('nu','nu','Niue'),('nf','nf','Norfolk Island'),('mp','mp','Northern Mariana Islands'),('no','no','Norway'),('om','om','Oman'),('pk','pk','Pakistan'),('pw','pw','Palau'),('ps','ps','Palestinian Territory, Occupied'),('pa','pa','Panama'),('pg','pg','Papua New Guinea'),('py','py','Paraguay'),('pe','pe','Peru'),('ph','ph','Philippines'),('pn','pn','Pitcairn Island'),('pl','pl','Poland'),('pt','pt','Portugal'),('pr','pr','Puerto Rico'),('qa','qa','Qatar'),('re','re','Reunion Island'),('ro','ro','Romania'),('ru','ru','Russian Federation'),('rw','rw','Rwanda'),('sh','sh','Saint Helena'),('kn','kn','Saint Kitts and Nevis'),('lc','lc','Saint Lucia'),('pm','pm','Saint Pierre and Miquelon'),('vc','vc','Saint Vincent and the Grenadines'),('ws','ws','Western Samoa'),('sm','sm','San Marino'),('st','st','Sao Tome and Principe'),('sa','sa','Saudi Arabia'),('sn','sn','Senegal'),('cs','cs','Serbia and Montenegro'),('sc','sc','Seychelles'),('sl','sl','Sierra Leone'),('sg','sg','Singapore'),('sk','sk','Slovak Republic'),('si','si','Slovenia'),('sb','sb','Solomon Islands'),('so','so','Somalia'),('za','za','South Africa'),('gs','gs','South Georgia and the South Sandwich Islands'),('es','es','Spain'),('lk','lk','Sri Lanka'),('sd','sd','Sudan'),('sr','sr','Suriname'),('sj','sj','Svalbard and Jan Mayen Islands'),('sz','sz','Swaziland'),('se','se','Sweden'),('ch','ch','Switzerland'),('sy','sy','Syrian Arab Republic'),('tw','tw','Taiwan'),('tj','tj','Tajikistan'),('tz','tz','United Republic of Tanzania'),('th','th','Thailand'),('tl','tl','Timor-Leste'),('tg','tg','Togo'),('tk','tk','Tokelau'),('to','to','Tonga'),('tt','tt','Trinidad and Tobago'),('tn','tn','Tunisia'),('tr','tr','Turkey'),('tm','tm','Turkmenistan'),('tc','tc','Turks and Caicos Islands'),('tv','tv','Tuvalu'),('ug','ug','Uganda'),('ua','ua','Ukraine'),('ae','ae','United Arab Emirates'),('gb','uk','United Kingdom'),('us','us','United States'),('um','um','United States Minor Outlying Islands'),('uy','uy','Uruguay'),('uz','uz','Uzbekistan'),('vu','vu','Vanuatu'),('ve','ve','Venezuela'),('vn','vn','Vietnam'),('vg','vg','Virgin Islands, British'),('vi','vi','Virgin Islands, U.S.'),('wf','wf','Wallis and Futuna Islands'),('eh','eh','Western Sahara'),('ye','ye','Yemen'),('zm','zm','Zambia'),('zw','zw','Zimbabwe'),(NULL,'com','Commercial'),(NULL,'net','Network Providers'),(NULL,'org','Non-profit Organisations'),(NULL,'mil','American Military'),(NULL,'example','RESERVED Used in examples'),(NULL,'invalid','RESERVED For use in obviously invalid domains'),(NULL,'localhost','RESERVED Unused as may conflict with localhost'),(NULL,'test','RESERVED For tests'),(NULL,'arpa','Arpanet (Infrastructure)'),(NULL,'aero','Air Transport'),(NULL,'biz','Business'),(NULL,'cat','Catalan Language'),(NULL,'coop','Cooperative'),(NULL,'edu','American Education'),(NULL,'gov','American Government'),(NULL,'info','Informational'),(NULL,'int','International Organizations established by treaty'),(NULL,'mil','American Military'),(NULL,'museum','Museums'),(NULL,'name','Families and Individuals'),(NULL,'pro','Professional'),(NULL,'travel','Anything linked to travel'),(NULL,'xxx','Sex'),(NULL,'jobs','Employment'),(NULL,'mobi','Mobile'),(NULL,'post','American Postal services'),(NULL,'tel','American Telephone services');
UNLOCK TABLES;
/*!40000 ALTER TABLE `country` ENABLE KEYS */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

