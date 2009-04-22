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
) ENGINE=InnoDB AUTO_INCREMENT=131 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `builds`
--

LOCK TABLES `builds` WRITE;
/*!40000 ALTER TABLE `builds` DISABLE KEYS */;
INSERT INTO `builds` (`id`, `svn_version`, `time`, `status`, `error_msg`) VALUES (0,0,'2008-08-07 21:08:39',0,''),(3,28360,'2008-08-07 21:36:09',1,''),(12,28371,'2008-08-07 23:45:59',1,''),(19,28373,'2008-08-08 09:18:26',0,''),(20,28374,'2008-08-08 13:36:44',1,'errorerrorerror'),(21,28374,'2008-08-08 13:58:09',1,'g++ -o build/debug/tests/test_config_cache.o -c -I/usr/local/include/boost-1_35/ -W -Wall -Wno-unused -Wno-sign-compare -ansi -Werror -O0 -DDEBUG -ggdb3 -DHAVE_CONFIG_H -DUSE_EDITOR2 -D_X11 -DBOOST_TEST_DYN_LINK -I/usr/include/SDL -I/usr/include/pango-1.0 -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/freetype2 -I/usr/include/libpng12 -I/usr/include/pixman-1 -I/usr/include/python2.5 -Isrc -I. build/debug/tests/test_config_cache.cpp\n'),(22,28374,'2008-08-08 14:08:04',1,''),(23,28374,'2008-08-08 14:20:28',1,'/home/coren/wesnoth/trunk/build/debug/tests/test_config_cache.cpp:96: undefined reference to `config_cache::test_config_cache::getInt()\'\ncollect2: ld returned 1 exit status\n'),(24,28374,'2008-08-08 14:31:59',1,'/home/coren/wesnoth/trunk/build/debug/tests/test_config_cache.cpp:96: undefined reference to `config_cache::test_config_cache::getInt()\'\ncollect2: ld returned 1 exit status\n'),(25,28375,'2008-08-08 14:58:25',0,''),(26,28374,'2008-08-08 15:08:13',1,'build/debug/tests/test_team.cpp:21: error: ‘aBOOST_WARN_EQUAL’ was not declared in this scope\n'),(27,28375,'2008-08-08 15:24:46',0,''),(32,28396,'2008-08-08 21:02:59',0,''),(36,28399,'2008-08-08 23:54:49',0,''),(44,28417,'2008-08-09 21:18:13',0,''),(45,28420,'2008-08-10 10:19:27',0,''),(46,28422,'2008-08-10 11:19:52',0,''),(47,28424,'2008-08-10 12:17:21',1,'build/debug/scoped_resource.hpp:194: error: ‘fg’ was not declared in this scope\nbuild/debug/scoped_resource.hpp:194: error: ‘close’ was not declared in this scope\n'),(48,28426,'2008-08-10 13:22:33',1,'build/debug/network_worker.cpp:460: error: no matching function for call to ‘<unnamed>::cork_setter::cork_setter()’\nbuild/debug/network_worker.cpp:425: note: candidates are: <unnamed>::cork_setter::cork_setter(int)\nbuild/debug/network_worker.cpp:424: note:                 <unnamed>::cork_setter::cork_setter(const<unnamed>::cork_setter&)\nbuild/debug/network_worker.cpp:494: error: no match for ‘operator*’ in ‘*in_file’\n'),(49,28427,'2008-08-10 13:38:40',1,'network_worker.cpp:494: error: no match for ‘operator*’ in ‘*in_file’\n'),(50,28437,'2008-08-10 15:20:07',0,''),(52,28446,'2008-08-10 18:18:54',0,''),(53,28447,'2008-08-10 19:18:48',0,''),(54,28453,'2008-08-10 22:27:19',1,'playcampaign.cpp:133: error: no matching function for call to ‘playsingle_controller::playsingle_controller(const config&, game_state&, const int&, const int&, const config&, CVideo&, bool&)’\nplaysingle_controller.hpp:33: note: candidates are: playsingle_controller::playsingle_controller(config&, game_state&, int, int, const config&, CVideo&, bool)\nplaysingle_controller.hpp:30: note:                 playsingle_controller::playsingle_controller(const playsingle_controller&)\n'),(56,28453,'2008-08-11 00:47:48',0,''),(62,28464,'2008-08-11 16:24:23',0,''),(63,28465,'2008-08-11 17:18:38',1,'editor_controller.cpp:353: error: invalid operands of types ‘const char [2]’ and ‘const char* const’ to binary ‘operator+’\n'),(64,28466,'2008-08-11 17:33:23',1,'build/debug/editor2/editor_controller.cpp:353: error: invalid operands of types ‘const char [2]’ and ‘const char* const’ to binary ‘operator+’\n'),(66,28470,'2008-08-11 18:03:18',0,''),(67,28475,'2008-08-11 21:57:07',0,''),(101,28698,'2008-08-18 07:23:03',0,''),(102,28701,'2008-08-18 12:17:15',0,'campaign_server/campaign_server.cpp:317: warning: comparison is always true due to limited range of data type\n'),(104,28706,'2008-08-18 14:18:23',0,'campaign_server/campaign_server.cpp:317: warning: comparison is always true due to limited range of data type\n'),(106,28733,'2008-08-18 21:56:00',1,'serialization/parser.cpp:340: error: expected primary-expression before ‘(’ token\nserialization/parser.cpp:342: error: expected primary-expression before ‘<<’ token\nserialization/parser.cpp:342: error: expected primary-expression before ‘<<’ token\nserialization/parser.cpp:342: error: expected primary-expression before ‘<<’ token\nserialization/parser.cpp:342: error: expected primary-expression before ‘<’ token\nserialization/parser.cpp:342: error: expected primary-expression before ‘.’ token\nserialization/parser.cpp:344: error: expected primary-expression before ‘==’ token\nserialization/parser.cpp:344: error: expected primary-expression before ‘==’ token\nserialization/parser.cpp:344: error: expected primary-expression before ‘==’ token\nserialization/parser.cpp:344: error: expected primary-expression before ‘=’ token\nserialization/parser.cpp:345: error: expected `;\\\' before ‘)’ token\nserialization/parser.cpp:351: error: expected primary-expression before ‘>>’ token\nserialization/parser.cpp:351: error: expected primary-expression before ‘>>’ token\nserialization/parser.cpp:351: error: expected primary-expression before ‘>>’ token\nserialization/parser.cpp:351: error: expected primary-expression before ‘>’ token\nserialization/parser.cpp:351: error: expected primary-expression before ‘.’ token\nserialization/parser.cpp:352: error: expected `;\\\' before ‘}’ token\n'),(108,28734,'2008-08-18 22:16:59',0,''),(110,28760,'2008-08-19 08:21:58',0,''),(111,28764,'2008-08-19 10:23:32',0,''),(112,28767,'2008-08-19 11:24:07',0,''),(113,28768,'2008-08-19 12:27:13',0,''),(114,28769,'2008-08-19 13:23:12',0,''),(115,28770,'2008-08-19 16:26:38',0,''),(116,28773,'2008-08-19 20:24:51',0,''),(117,28776,'2008-08-19 21:28:19',0,''),(118,28783,'2008-08-20 07:46:12',0,''),(119,28784,'2008-08-20 08:20:36',0,''),(120,28785,'2008-08-20 10:50:12',0,''),(121,28792,'2008-08-20 17:41:46',0,''),(122,28797,'2008-08-20 18:38:27',0,''),(123,28805,'2008-08-20 19:28:14',0,''),(124,28818,'2008-08-21 06:28:56',0,''),(125,28819,'2008-08-21 07:24:16',0,''),(126,28821,'2008-08-21 10:23:40',0,''),(127,28823,'2008-08-21 11:23:03',0,''),(128,28824,'2008-08-21 13:28:56',0,''),(129,28837,'2008-08-22 06:38:06',0,'editor2/mouse_action.hpp:205: warning: ‘editor2::brush_drag_mouse_action::brush_’ will be initialized after\neditor2/mouse_action.hpp:191: warning:   ‘gamemap::location editor2::brush_drag_mouse_action::previous_drag_hex_’\neditor2/mouse_action.hpp:133: warning:   when initialized here\ndistcc[5584] ERROR: compile build/debug/editor2/editor_controller.cpp on localhost failed\n'),(130,28839,'2008-08-22 08:26:41',0,'');
/*!40000 ALTER TABLE `builds` ENABLE KEYS */;
UNLOCK TABLES;

--
-- Table structure for table `configs`
--

DROP TABLE IF EXISTS `configs`;
SET @saved_cs_client     = @@character_set_client;
SET character_set_client = utf8;
CREATE TABLE `configs` (
  `name` varchar(40) NOT NULL,
  `value` varchar(100) NOT NULL,
  PRIMARY KEY  (`name`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `configs`
--

LOCK TABLES `configs` WRITE;
/*!40000 ALTER TABLE `configs` DISABLE KEYS */;
INSERT INTO `configs` (`name`, `value`) VALUES ('build_previous_prune_time','2008-08-19 11:26:42'),('last_autotest_run_time','2008-08-22 11:26:43');
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
) ENGINE=InnoDB AUTO_INCREMENT=25 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `test_errors`
--

LOCK TABLES `test_errors` WRITE;
/*!40000 ALTER TABLE `test_errors` DISABLE KEYS */;
INSERT INTO `test_errors` (`id`, `before_id`, `last_id`, `error_type`, `file`, `line`, `error_msg`) VALUES (7,12,19,'Message','./boost/test/impl/results_collector.ipp',220,'Test case test_user_team_name doesn\'t include any assertions'),(8,12,66,'Error','tests/test_config_cache.cpp',84,'check defines_map.size() == cache.get_preproc_map().size() failed [2 != 0]'),(9,52,53,'Exception','',0,'memory access violation at address: 0x00000118: no mapping at fault address'),(10,52,53,'Message','./boost/test/impl/unit_test_log.ipp',157,'Test is aborted'),(15,104,113,'Error','build/debug/tests/test_config_cache.cpp',114,'check test_config == cache.get_config() failed [[textdomain]\\n	name = wesnoth\\n[/textdomain]\\n[test_key]\\n	define = test\\n[/test_key]\\n != ]'),(16,104,113,'Error','build/debug/tests/test_config_cache.cpp',122,'check test_config == cache.get_config() failed [[textdomain]\\n	name = wesnoth\\n[/textdomain]\\n[test_key]\\n	define = test\\n[/test_key]\\n[test_key]\\n	define = testing translation reset\\n[/test_key]\\n != ]'),(17,104,110,'Error','build/debug/tests/test_config_cache.cpp',145,'No languages found!'),(18,104,110,'FatalError','build/debug/tests/test_config_cache.cpp',149,'German translation not found'),(19,110,113,'Error','build/debug/tests/test_config_cache.cpp',157,'check test_config == cache.get_config() failed [[textdomain]\\n	name = wesnoth\\n[/textdomain]\\n[test_key]\\n	define = test\\n[/test_key]\\n[test_key]\\n	define = test translation reset\\n[/test_key]\\n != ]'),(20,113,130,'Error','tests/test_config_cache.cpp',114,'check test_config == cache.get_config() failed [[textdomain]\n	name = wesnoth\n[/textdomain]\n[test_key]\n	define = test\n[/test_key]\n != ]'),(21,113,130,'Error','tests/test_config_cache.cpp',122,'check test_config == cache.get_config() failed [[textdomain]\n	name = wesnoth\n[/textdomain]\n[test_key]\n	define = test\n[/test_key]\n[test_key]\n	define = testing translation reset\n[/test_key]\n != ]'),(22,113,120,'Error','tests/test_config_cache.cpp',145,'No languages found!'),(23,113,130,'FatalError','tests/test_config_cache.cpp',149,'German translation not found'),(24,120,123,'Error','tests/test_config_cache.cpp',157,'check test_config == cache.get_config() failed [[textdomain]\n	name = wesnoth\n[/textdomain]\n[test_key]\n	define = test\n[/test_key]\n[test_key]\n	define = test translation reset\n[/test_key]\n != ]');
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
) ENGINE=InnoDB AUTO_INCREMENT=94 DEFAULT CHARSET=latin1;
SET character_set_client = @saved_cs_client;

--
-- Dumping data for table `test_results`
--

LOCK TABLES `test_results` WRITE;
/*!40000 ALTER TABLE `test_results` DISABLE KEYS */;
INSERT INTO `test_results` (`id`, `build_id`, `assertions_passed`, `test_cases_passed`, `test_cases_skipped`, `result`, `assertions_failed`, `test_cases_failed`, `test_cases_aborted`) VALUES (1,19,112,17,0,'failed',1,1,0),(2,25,113,17,0,'failed',1,1,0),(3,27,113,17,0,'failed',1,1,0),(8,32,113,17,0,'failed',1,1,0),(9,36,114,18,0,'failed',1,1,0),(17,44,114,18,0,'failed',1,1,0),(18,45,144,24,0,'failed',1,1,0),(19,46,144,24,0,'failed',1,1,0),(20,50,204,24,0,'failed',1,1,0),(22,52,204,24,0,'failed',1,1,0),(23,53,18,4,1,'aborted',1,1,1),(24,56,174,22,0,'failed',1,1,0),(30,62,174,22,0,'failed',1,1,0),(31,66,174,22,0,'failed',1,1,0),(32,67,174,23,0,'passed',0,0,0),(66,101,174,23,0,'passed',0,0,0),(67,102,174,23,0,'passed',0,0,0),(69,104,174,23,0,'passed',0,0,0),(71,108,176,22,0,'failed',4,2,1),(73,110,176,22,0,'failed',4,2,1),(74,111,179,22,0,'failed',3,2,0),(75,112,179,22,0,'failed',3,2,0),(76,113,179,22,0,'failed',3,2,0),(77,114,176,22,0,'failed',4,2,1),(78,115,176,22,0,'failed',4,2,1),(79,116,176,22,0,'failed',4,2,1),(80,117,176,22,0,'failed',4,2,1),(81,118,176,22,0,'failed',4,2,1),(82,119,176,22,0,'failed',4,2,1),(83,120,176,22,0,'failed',4,2,1),(84,121,179,22,0,'failed',3,2,0),(85,122,179,22,0,'failed',3,2,0),(86,123,179,22,0,'failed',3,2,0),(87,124,177,22,0,'failed',3,2,1),(88,125,177,22,0,'failed',3,2,1),(89,126,177,22,0,'failed',3,2,1),(90,127,177,22,0,'failed',3,2,1),(91,128,177,22,0,'failed',3,2,1),(92,129,177,22,0,'failed',3,2,1),(93,130,177,22,0,'failed',3,2,1);
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

-- Dump completed on 2008-08-22  9:21:07
