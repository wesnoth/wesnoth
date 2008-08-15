-- MySQL dump 10.11
--
-- Host: localhost    Database: wesnoth_unit_test
-- ------------------------------------------------------
-- Server version	5.0.51a-3ubuntu5.1

/*!40101 SET @OLD_CHARACTER_SET_CLIENT=@@CHARACTER_SET_CLIENT */;
/*!40101 SET @OLD_CHARACTER_SET_RESULTS=@@CHARACTER_SET_RESULTS */;
/*!40101 SET @OLD_COLLATION_CONNECTION=@@COLLATION_CONNECTION */;
/*!40101 SET NAMES utf8 */;
/*!40103 SET @OLD_TIME_ZONE=@@TIME_ZONE */;
/*!40103 SET TIME_ZONE='+00:00' */;
/*!40014 SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0 */;
/*!40014 SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0 */;
/*!40101 SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='NO_AUTO_VALUE_ON_ZERO' */;
/*!40111 SET @OLD_SQL_NOTES=@@SQL_NOTES, SQL_NOTES=0 */;

--
-- Table structure for table `builds`
--

DROP TABLE IF EXISTS `builds`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `builds` (
  `id` int(11) NOT NULL auto_increment,
  `svn_version` int(11) NOT NULL,
  `time` timestamp NOT NULL default CURRENT_TIMESTAMP,
  `status` int(11) NOT NULL,
  `error_msg` blob NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `time` (`time`),
  KEY `svn_version` (`svn_version`),
  KEY `id_status` (`id`,`status`)
) ENGINE=InnoDB AUTO_INCREMENT=87 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `builds`
--

LOCK TABLES `builds` WRITE;
/*!40000 ALTER TABLE `builds` DISABLE KEYS */;
INSERT INTO `builds` (`id`, `svn_version`, `time`, `status`, `error_msg`) VALUES (0,0,'2008-08-07 21:08:39',0,''),(3,28360,'2008-08-07 21:36:09',1,''),(4,28363,'2008-08-07 21:53:09',1,''),(5,28364,'2008-08-07 21:58:58',1,''),(6,28365,'2008-08-07 22:04:03',1,''),(7,28366,'2008-08-07 22:17:29',1,''),(8,28367,'2008-08-07 23:27:27',1,''),(9,28368,'2008-08-07 23:32:02',1,''),(10,28369,'2008-08-07 23:39:36',1,''),(11,28370,'2008-08-07 23:43:28',1,''),(12,28371,'2008-08-07 23:45:59',1,''),(19,28373,'2008-08-08 09:18:26',0,''),(20,28374,'2008-08-08 13:36:44',1,'errorerrorerror'),(21,28374,'2008-08-08 13:58:09',1,'g++ -o build/debug/tests/test_config_cache.o -c -I/usr/local/include/boost-1_35/ -W -Wall -Wno-unused -Wno-sign-compare -ansi -Werror -O0 -DDEBUG -ggdb3 -DHAVE_CONFIG_H -DUSE_EDITOR2 -D_X11 -DBOOST_TEST_DYN_LINK -I/usr/include/SDL -I/usr/include/pango-1.0 -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/pixman-1 -I/usr/include/python2.5 -Isrc -I. build/debug/tests/test_config_cache.cpp\n'),(22,28374,'2008-08-08 14:08:04',1,''),(23,28374,'2008-08-08 14:20:28',1,'/home/coren/wesnoth/trunk/build/debug/tests/test_config_cache.cpp:96: undefined reference to `config_cache::test_config_cache::getInt()\'\ncollect2: ld returned 1 exit status\n'),(24,28374,'2008-08-08 14:31:59',1,'/home/coren/wesnoth/trunk/build/debug/tests/test_config_cache.cpp:96: undefined reference to `config_cache::test_config_cache::getInt()\'\ncollect2: ld returned 1 exit status\n'),(25,28375,'2008-08-08 14:58:25',0,''),(26,28374,'2008-08-08 15:08:13',1,'build/debug/tests/test_team.cpp:21: error: ‘aBOOST_WARN_EQUAL’ was not declared in this scope\n'),(27,28375,'2008-08-08 15:24:46',0,''),(28,28376,'2008-08-08 16:09:09',0,''),(29,28378,'2008-08-08 16:46:31',0,''),(30,28383,'2008-08-08 17:43:44',0,''),(31,28393,'2008-08-08 20:29:49',0,''),(32,28396,'2008-08-08 21:02:59',0,''),(36,28399,'2008-08-08 23:54:49',0,''),(37,28401,'2008-08-09 07:20:04',0,''),(38,28402,'2008-08-09 08:19:54',0,''),(39,28405,'2008-08-09 09:22:39',0,''),(40,28408,'2008-08-09 10:20:25',0,''),(41,28409,'2008-08-09 12:19:53',0,''),(42,28410,'2008-08-09 17:19:58',0,''),(43,28413,'2008-08-09 20:22:11',0,''),(44,28417,'2008-08-09 21:18:13',0,''),(45,28420,'2008-08-10 10:19:27',0,''),(46,28422,'2008-08-10 11:19:52',0,''),(47,28424,'2008-08-10 12:17:21',1,'build/debug/scoped_resource.hpp:194: error: ‘fg’ was not declared in this scope\nbuild/debug/scoped_resource.hpp:194: error: ‘close’ was not declared in this scope\n'),(48,28426,'2008-08-10 13:22:33',1,'build/debug/network_worker.cpp:460: error: no matching function for call to ‘<unnamed>::cork_setter::cork_setter()’\nbuild/debug/network_worker.cpp:425: note: candidates are: <unnamed>::cork_setter::cork_setter(int)\nbuild/debug/network_worker.cpp:424: note:                 <unnamed>::cork_setter::cork_setter(const<unnamed>::cork_setter&)\nbuild/debug/network_worker.cpp:494: error: no match for ‘operator*’ in ‘*in_file’\n'),(49,28427,'2008-08-10 13:38:40',1,'network_worker.cpp:494: error: no match for ‘operator*’ in ‘*in_file’\n'),(50,28437,'2008-08-10 15:20:07',0,''),(51,28445,'2008-08-10 17:31:42',0,''),(52,28446,'2008-08-10 18:18:54',0,''),(53,28447,'2008-08-10 19:18:48',0,''),(54,28453,'2008-08-10 22:27:19',1,'playcampaign.cpp:133: error: no matching function for call to ‘playsingle_controller::playsingle_controller(const config&, game_state&, const int&, const int&, const config&, CVideo&, bool&)’\nplaysingle_controller.hpp:33: note: candidates are: playsingle_controller::playsingle_controller(config&, game_state&, int, int, const config&, CVideo&, bool)\nplaysingle_controller.hpp:30: note:                 playsingle_controller::playsingle_controller(const playsingle_controller&)\n'),(56,28453,'2008-08-11 00:47:48',0,''),(57,28454,'2008-08-11 02:18:19',0,''),(58,28456,'2008-08-11 07:20:47',0,''),(59,28458,'2008-08-11 09:18:25',0,''),(60,28461,'2008-08-11 14:20:18',0,''),(61,28462,'2008-08-11 15:22:46',0,''),(62,28464,'2008-08-11 16:24:23',0,''),(63,28465,'2008-08-11 17:18:38',1,'editor_controller.cpp:353: error: invalid operands of types ‘const char [2]’ and ‘const char* const’ to binary ‘operator+’\n'),(64,28466,'2008-08-11 17:33:23',1,'build/debug/editor2/editor_controller.cpp:353: error: invalid operands of types ‘const char [2]’ and ‘const char* const’ to binary ‘operator+’\n'),(66,28470,'2008-08-11 18:03:18',0,''),(67,28475,'2008-08-11 21:57:07',0,''),(68,28476,'2008-08-12 06:30:49',0,''),(69,28478,'2008-08-12 07:31:17',0,''),(70,28479,'2008-08-12 09:21:14',0,''),(71,28480,'2008-08-12 10:25:23',0,''),(72,28483,'2008-08-12 12:20:14',0,''),(73,28486,'2008-08-12 13:18:19',0,''),(74,28487,'2008-08-12 15:20:39',0,''),(75,28488,'2008-08-12 17:19:01',0,''),(76,28495,'2008-08-12 18:27:45',0,''),(77,28506,'2008-08-12 19:26:20',0,''),(78,28521,'2008-08-12 22:20:52',0,''),(79,28524,'2008-08-12 23:17:44',0,''),(80,28567,'2008-08-13 19:23:34',0,''),(81,28578,'2008-08-14 14:21:36',0,''),(82,28579,'2008-08-14 16:22:46',0,''),(83,28589,'2008-08-14 18:23:34',0,''),(84,28594,'2008-08-14 20:22:24',0,''),(85,28597,'2008-08-15 12:03:55',0,''),(86,28598,'2008-08-15 12:21:05',0,'');
/*!40000 ALTER TABLE `builds` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `configs`
--

DROP TABLE IF EXISTS `configs`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `configs` (
  `name` varchar(255) NOT NULL,
  `value` varchar(255) NOT NULL,
  PRIMARY KEY  (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `configs`
--

LOCK TABLES `configs` WRITE;
/*!40000 ALTER TABLE `configs` DISABLE KEYS */;
/*!40000 ALTER TABLE `configs` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_errors`
--

DROP TABLE IF EXISTS `test_errors`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `test_errors` (
  `id` int(11) NOT NULL auto_increment,
  `before_id` int(11) NOT NULL,
  `last_id` int(11) NOT NULL,
  `error_type` varchar(10) NOT NULL,
  `file` varchar(64) NOT NULL,
  `line` int(11) NOT NULL,
  `error_msg` blob NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `test_errors_before_id_key` (`before_id`),
  KEY `test_errors_last_id_key` (`last_id`),
  KEY `error_type_and_file` (`error_type`,`file`),
  CONSTRAINT `test_errors_before_id_key` FOREIGN KEY (`before_id`) REFERENCES `builds` (`id`),
  CONSTRAINT `test_errors_last_id_key` FOREIGN KEY (`last_id`) REFERENCES `builds` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=11 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `test_errors`
--

LOCK TABLES `test_errors` WRITE;
/*!40000 ALTER TABLE `test_errors` DISABLE KEYS */;
INSERT INTO `test_errors` (`id`, `before_id`, `last_id`, `error_type`, `file`, `line`, `error_msg`) VALUES (7,12,19,'Message','./boost/test/impl/results_collector.ipp',220,'Test case test_user_team_name doesn\'t include any assertions'),(8,12,66,'Error','tests/test_config_cache.cpp',84,'check defines_map.size() == cache.get_preproc_map().size() failed [2 != 0]'),(9,52,53,'Exception','',0,'memory access violation at address: 0x00000118: no mapping at fault address'),(10,52,53,'Message','./boost/test/impl/unit_test_log.ipp',157,'Test is aborted');
/*!40000 ALTER TABLE `test_errors` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `test_results`
--

DROP TABLE IF EXISTS `test_results`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `test_results` (
  `id` int(11) NOT NULL auto_increment,
  `build_id` int(11) NOT NULL,
  `assertions_passed` int(11) NOT NULL,
  `test_cases_passed` int(11) NOT NULL,
  `test_cases_skipped` int(11) NOT NULL,
  `result` varchar(10) NOT NULL,
  `assertions_failed` int(11) NOT NULL,
  `test_cases_failed` int(11) NOT NULL,
  `test_cases_aborted` int(11) NOT NULL,
  PRIMARY KEY  (`id`),
  KEY `test_results_build_id_key` (`build_id`),
  CONSTRAINT `test_results_build_id_key` FOREIGN KEY (`build_id`) REFERENCES `builds` (`id`)
) ENGINE=InnoDB AUTO_INCREMENT=52 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `test_results`
--

LOCK TABLES `test_results` WRITE;
/*!40000 ALTER TABLE `test_results` DISABLE KEYS */;
INSERT INTO `test_results` (`id`, `build_id`, `assertions_passed`, `test_cases_passed`, `test_cases_skipped`, `result`, `assertions_failed`, `test_cases_failed`, `test_cases_aborted`) VALUES (1,19,112,17,0,'failed',1,1,0),(2,25,113,17,0,'failed',1,1,0),(3,27,113,17,0,'failed',1,1,0),(4,28,113,17,0,'failed',1,1,0),(5,29,113,17,0,'failed',1,1,0),(6,30,113,17,0,'failed',1,1,0),(7,31,113,17,0,'failed',1,1,0),(8,32,113,17,0,'failed',1,1,0),(9,36,114,18,0,'failed',1,1,0),(10,37,114,18,0,'failed',1,1,0),(11,38,114,18,0,'failed',1,1,0),(12,39,114,18,0,'failed',1,1,0),(13,40,114,18,0,'failed',1,1,0),(14,41,114,18,0,'failed',1,1,0),(15,42,114,18,0,'failed',1,1,0),(16,43,114,18,0,'failed',1,1,0),(17,44,114,18,0,'failed',1,1,0),(18,45,144,24,0,'failed',1,1,0),(19,46,144,24,0,'failed',1,1,0),(20,50,204,24,0,'failed',1,1,0),(21,51,204,24,0,'failed',1,1,0),(22,52,204,24,0,'failed',1,1,0),(23,53,18,4,1,'aborted',1,1,1),(24,56,174,22,0,'failed',1,1,0),(25,57,174,22,0,'failed',1,1,0),(26,58,174,22,0,'failed',1,1,0),(27,59,174,22,0,'failed',1,1,0),(28,60,174,22,0,'failed',1,1,0),(29,61,174,22,0,'failed',1,1,0),(30,62,174,22,0,'failed',1,1,0),(31,66,174,22,0,'failed',1,1,0),(32,67,174,23,0,'passed',0,0,0),(33,68,174,23,0,'passed',0,0,0),(34,69,174,23,0,'passed',0,0,0),(35,70,174,23,0,'passed',0,0,0),(36,71,174,23,0,'passed',0,0,0),(37,72,174,23,0,'passed',0,0,0),(38,73,174,23,0,'passed',0,0,0),(39,74,174,23,0,'passed',0,0,0),(40,75,174,23,0,'passed',0,0,0),(41,76,174,23,0,'passed',0,0,0),(42,77,174,23,0,'passed',0,0,0),(43,78,174,23,0,'passed',0,0,0),(44,79,174,23,0,'passed',0,0,0),(45,80,174,23,0,'passed',0,0,0),(46,81,174,23,0,'passed',0,0,0),(47,82,174,23,0,'passed',0,0,0),(48,83,174,23,0,'passed',0,0,0),(49,84,174,23,0,'passed',0,0,0),(50,85,174,23,0,'passed',0,0,0),(51,86,174,23,0,'passed',0,0,0);
/*!40000 ALTER TABLE `test_results` ENABLE KEYS */;
UNLOCK TABLES;
/*!40103 SET TIME_ZONE=@OLD_TIME_ZONE */;

/*!40101 SET SQL_MODE=@OLD_SQL_MODE */;
/*!40014 SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS */;
/*!40014 SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS */;
/*!40101 SET CHARACTER_SET_CLIENT=@OLD_CHARACTER_SET_CLIENT */;
/*!40101 SET CHARACTER_SET_RESULTS=@OLD_CHARACTER_SET_RESULTS */;
/*!40101 SET COLLATION_CONNECTION=@OLD_COLLATION_CONNECTION */;
/*!40111 SET SQL_NOTES=@OLD_SQL_NOTES */;

-- Dump completed on 2008-08-15 14:22:54
