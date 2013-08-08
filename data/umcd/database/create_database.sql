
-- ---
-- Database creation.
-- ---

DROP DATABASE IF EXISTS umcd;
CREATE DATABASE IF NOT EXISTS umcd;
USE umcd;

-- ---
-- Table 'Addon'
-- 
-- ---

CREATE TABLE Addon (
  id int unsigned NOT NULL AUTO_INCREMENT,
  type smallint unsigned NOT NULL,
  email varchar(254) NOT NULL, -- see RFC Erratum: http://www.rfc-editor.org/errata_search.php?rfc=3696&eid=1690
  password varchar(256) NOT NULL,
  native_language smallint unsigned NOT NULL,
  CONSTRAINT pk_Addon PRIMARY KEY (id)
);

-- ---
-- Table 'AddonType'
-- 
-- ---
    
CREATE TABLE AddonType (
  value smallint unsigned NOT NULL AUTO_INCREMENT,
  name varchar(50) NOT NULL UNIQUE,
  CONSTRAINT pk_AddonType PRIMARY KEY (value)
);

-- ---
-- Table 'Author'
-- 
-- ---

CREATE TABLE Author (
  id int unsigned NOT NULL AUTO_INCREMENT,
  name varchar(100) NOT NULL UNIQUE,
  CONSTRAINT pk_Author PRIMARY KEY (id)
);

-- ---
-- Table 'AddonMaintainers'
-- 
-- ---

CREATE TABLE AddonMaintainers (
  addon int unsigned NOT NULL AUTO_INCREMENT,
  author int unsigned NOT NULL,
  CONSTRAINT pk_AddonMaintainers PRIMARY KEY (addon, author)
);

-- ---
-- Table 'AddonVersion'
-- 
-- ---

CREATE TABLE AddonVersion (
  id int unsigned NOT NULL AUTO_INCREMENT,
  name varchar(256) NOT NULL,
  description text NOT NULL,
  version varchar(50) NOT NULL,
  translation int unsigned NOT NULL,
  path_to_addon_data varchar(512) NOT NULL,
  upload_date date NOT NULL,
  uploader_ip varchar(50) NOT NULL,
  downloads int unsigned NOT NULL,
  uploads int unsigned NOT NULL,
  CONSTRAINT pk_AddonVersion PRIMARY KEY (id)
);

-- ---
-- Table 'Historic'
-- 
-- ---
    
CREATE TABLE Historic (
  main_addon int unsigned NOT NULL AUTO_INCREMENT,
  addon_version varchar(50) NOT NULL,
  CONSTRAINT pk_Historic PRIMARY KEY (main_addon, addon_version)
);

-- ---
-- Table 'Dependencies'
-- 
-- ---
    
CREATE TABLE Dependencies (
  addon_version int unsigned NOT NULL AUTO_INCREMENT,
  dependency int unsigned NOT NULL,
  version_mask varchar(110) NOT NULL,
  CONSTRAINT pk_Dependencies PRIMARY KEY (addon_version, dependency)
);

-- ---
-- Table 'Language'
-- 
-- ---
    
CREATE TABLE Language (
  value smallint unsigned NOT NULL AUTO_INCREMENT,
  name varchar(50) NOT NULL UNIQUE,
  CONSTRAINT pk_Language PRIMARY KEY (value)
);

-- ---
-- Table 'Translation'
-- 
-- ---
    
CREATE TABLE Translation (
  id int unsigned NOT NULL AUTO_INCREMENT,
  language smallint unsigned NOT NULL,
  translated_addon int unsigned NOT NULL,
  path_to_po_file varchar(512) NOT NULL UNIQUE,
  fuzzy int unsigned NOT NULL,
  translated int unsigned NOT NULL,
  untranslated int unsigned NOT NULL,
  upload_date date NOT NULL,
  CONSTRAINT pk_Translation PRIMARY KEY (id)
);

-- Foreign keys

ALTER TABLE Addon ADD CONSTRAINT fk_AddonAddonType FOREIGN KEY (type) REFERENCES AddonType(value);
ALTER TABLE Addon ADD CONSTRAINT fk_AddonLanguage FOREIGN KEY (native_language) REFERENCES Language (value);

ALTER TABLE AddonMaintainers ADD CONSTRAINT fk_AddonMaintainersAddon FOREIGN KEY (addon) REFERENCES Addon(id);
ALTER TABLE AddonMaintainers ADD CONSTRAINT fk_AddonMaintainersAuthor FOREIGN KEY (author) REFERENCES Author(id);

ALTER TABLE Historic ADD CONSTRAINT fk_HistoricAddon FOREIGN KEY (main_addon) REFERENCES Addon(id);
ALTER TABLE Historic ADD CONSTRAINT fk_HistoricAddonVersion FOREIGN KEY (addon_version) REFERENCES AddonVersion(id);

ALTER TABLE Dependencies ADD CONSTRAINT fk_DependenciesAddon FOREIGN KEY (dependency) REFERENCES Addon(id);
ALTER TABLE Dependencies ADD CONSTRAINT fk_DependenciesAddonVersion FOREIGN KEY (addon_version) REFERENCES AddonVersion(id);

ALTER TABLE Translation ADD CONSTRAINT fk_TranslationLanguage FOREIGN KEY (language) REFERENCES Language(value);
ALTER TABLE Translation ADD CONSTRAINT fk_TranslationAddonVersion FOREIGN KEY (translated_addon) REFERENCES AddonVersion (id);

-- ---
-- Test Data
-- ---

-- INSERT INTO Addon (id,type,email,password,native_language) VALUES
-- ('','','','','');
-- INSERT INTO AddonType (value,name) VALUES
-- ('','');
-- INSERT INTO Author (id,name) VALUES
-- ('','');
-- INSERT INTO AddonMaintainers (addon,author) VALUES
-- ('','');
-- INSERT INTO AddonVersion (id,name,description,version,translation,path_to_addon_data,timestamp,uploader_ip,downloads,uploads) VALUES
-- ('','','','','','','','','','');
-- INSERT INTO Historic (main_addon,addon_version) VALUES
-- ('','');
-- INSERT INTO Dependencies (addon_version,dependency,version_mask) VALUES
-- ('','','');
-- INSERT INTO Language (value,name) VALUES
-- ('','');
-- INSERT INTO Translation (id,language,translated_addon,path_to_po_file,fuzzy,translated,untranslated,timestamp) VALUES
-- ('','','','','','','','');
