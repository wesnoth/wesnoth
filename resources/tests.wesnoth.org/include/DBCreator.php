<?php
/*
   Copyright (C) 2008 by Pauli Nieminen <paniemin@cc.hut.fi>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/


class DBForeignKey {
	private $name;
	private $field_name;
	private $reference;

	function __construct($name, $field_name, $reference)
	{
		$this->field_name = $field_name;
		$this->name = $name;
		$this->reference = $reference;
	}
	public function getCreateSQL()
	{
		return "CONSTRAINT $this->name FOREIGN KEY ($this->field_name) REFERENCES $this->reference";
	}

	public function match_and_alter($db, &$create_sql, $table_name)
	{
		$m = array();
		$needle = preg_replace("/[()]/","\\\\$0","/^.*CONSTRAINT `$this->name` FOREIGN KEY ($this->field_name) REFERENCES $this->reference.*$/im");
		if (!preg_match($needle,$create_sql))
		{
//			echo "$needle\n$create_sql\n";
//			echo "Creating foreign key $this->field_name in $table_name!\n";
			if (!$db->Execute("ALTER TABLE $table_name 
				ADD CONSTRAINT $this->name 
				FOREIGN KEY ($this->field_name) 
				REFERENCES $this->reference"))
			{
//				echo "Failed to create foreign key!";
			}
		} else {
			$create_sql = preg_replace($needle,'',$create_sql);
			$create_sql = preg_replace("/^.*KEY (`$this->name`)? \\($this->field_name\\).*$/im",'',$create_sql);
		}
	}

}

class DBIndex {
	private $field_name;
	private $type;
	private $key_name;

	function __construct($field_name, $type, $key_name = '')
	{
		$this->field_name = $field_name;
		$this->type = $type;
		if (empty($key_name))
			$key_name = preg_replace('/^`([^`]*)`.*$/','$1',$field_name);
		$this->key_name = $key_name;
	}
	public function getCreateSQL()
	{
		return "$this->type $this->key_name ($this->field_name)";
	}

	public function match_and_alter($db, &$create_sql, $table_name)
	{
		$m = array();
		$needle = "/^.*$this->type (`$this->key_name`)? \\($this->field_name\\).*$/im";
		if (!preg_match($needle,$create_sql))
		{
//			echo "$needle\n$create_sql\n";
//			echo "Creating index $this->field_name in $table_name!\n";
			if (!$db->Execute("ALTER TABLE $table_name 
				ADD $this->type $this->key_name ($this->field_name)"))
			{
//				echo "Failed to create key!\n";
			}
		} else {
			$create_sql = preg_replace($needle,'',$create_sql);
		}
	}
}

class DBField {
	private $name;
	private $type;
	private $default;

	function __construct($name, $type, $default = '')
	{
		$this->name = $name;
		$this->type = $type;
		$this->default = $default;
	}

	public function getCreateSQL()
	{
		$default = '';
		if (!empty($this->default))
		{
			$default = 'DEFAULT ' . $this->default;
		}
		return "$this->name $this->type $default";
	}

	public function match_and_alter($db, &$create_sql, $table_name)
	{
		$m = array();
		
		$default = '';
		if (!empty($this->default))
		{
			$default = ' DEFAULT ' . $this->default;
		}
		$needle = "/^\\s*`$this->name`(.*),?$/im";
		if (preg_match($needle,$create_sql, $m))
		{
			// Found the field checking for correct type
			$type = preg_replace(array("/[()]/","/^([a-zA-Z1-9_]*) /"),array("\\\\$0","$1(\\(\\d*\\))? "), $this->type);
			if (!preg_match("/$type$default/i",$m[1]))
			{
//				echo $m[1] . "\n" . $type . "\n";
//				echo "Updateing column $this->name in $table_name!\n";
				if (!$db->Execute("ALTER TABLE $table_name 
					 MODIFY COLUMN $this->name $this->type $default"))
				{
//					echo "Failed to create column!\n";
				}
			}
			$create_sql = preg_replace($needle,'',$create_sql);
		} else {
			// Add new field
//			echo "Creating column $this->name in $table_name!\n";
			
			if (!$db->Execute("ALTER TABLE $table_name 
				ADD COLUMN $this->name $this->type $default"))
			{
//				echo "Failed to modify column!\n";
			}
		}

	}
}

class DBTable {
	private $fields;

	private $name;
	private $engine;

	function __construct($name, $engine = 'MYISAM')
	{
		$this->name = $name;
		$this->engine = $engine;
		$this->fields = array();
	}

	public function addChild($child)
	{
		$this->fields[] = $child;
	}

	public function getName()
	{
		return $this->name;
	}

	public function check($db)
	{
		$result = $db->Execute("SHOW CREATE TABLE $this->name");

		if ($result === false)
		{
			echo "Error reading create info for $this->name table\n";
			exit;
		}

		$create_sql = $result->fields['Create Table'];

		foreach($this->fields as $id => $field)
		{
			$field->match_and_alter($db,$create_sql, $this->name);
		}
		$m = array();
		if (preg_match_all('/^\s*`(.*)` .*(int|varchar|text|blob|char|NULL|NOT NULL|timestamp|date).*$/m',$create_sql,$m,PREG_SET_ORDER))
		{
			foreach($m as $col)
			{
				try {
				$colname = $col[1];
				$db->Execute("ALTER TABLE $this->name
					DROP COLUMN $colname");
				} catch(exception $e) {
				}
			}
		}
		if (preg_match_all('/^\s*(CONSTRAINT|KEY|PRIMARY KEY|UNIQUE KEY) (`(.*)`)? .*$/m',$create_sql,$m,PREG_SET_ORDER))
		{
			foreach($m as $index)
			{
				try {
				$indexname = '';
				if (isset($index[3]))
					$indexname = $index[3];
				$type = $index[1];
				switch($type)
				{
				case "CONSTRAINT":
					$db->Execute("ALTER TABLE $this->name
						DREP FOREIGN KEY $indexname");
					break;
				case "PRIMARY KEY":
					$db->Execute("ALTER TABLE $this->name
						DROP PRIMARY KEY");
					break;
				case "UNIQUE KEY":
				case "KEY":
					$db->Execute("ALTER TABLE $this->name
						DROP KEY $indexname");
					break;
				}
				} catch(exception $e)
				{
				}
			}
		}
	}

	public function createTable($db)
	{
//		echo "Creating table '$this->name'\n";
		$sql = "CREATE TABLE $this->name (";
		foreach($this->fields as $field)
		{
			$sql .= $field->getCreateSQL() . ',';
		}
		$sql = rtrim($sql, ',') . ") ENGINE=$this->engine";

		if($db->Execute($sql) === false)
		{
			echo "Error creating table $this->name!\n";
			exit;
		}
	}

}

class DBFormat {
	private $tables;

	function __construct()
	{
		$this->tables = array();
	}

	public function addChild($table)
	{
		$this->tables[] = $table;
	}

	public function checkDB($db)
	{
		$result = $db->Execute('show tables');
		$dbtables = array();
		if ($result !== false)
		{
			while(!$result->EOF())
			{
				$dbtables[$result->fields['Tables_in_wesnoth_unit_test']] = true;
				$result->MoveNext();
			}
		} else {
			echo 'show tables failed';
			exit;
		}

		// check for missing and old tables
		foreach($this->tables as $table)
		{
			if (!isset($dbtables[$table->getName()]))
			{
				// create missing table
				$table->createTable($db);
			} else {
				// check if this table needs updateing
				$table->check($db);
				unset($dbtables[$table->getname()]);
			}
		}

		foreach($dbtables as $name => $bool)
		{
			$db->Execute('DROP TABLE ' . $name);
		}
	}
}


class DBCreator {
	private $db;
	private $format;
	function __construct()
	{
		global $db;
		$this->format = new DBFormat();
		
		$configtable = new DBTable('configs', 'InnoDB');
		$configtable->addChild(new DBField('name', 'VARCHAR(255) NOT NULL'));
		$configtable->addChild(new DBField('value', 'VARCHAR(255) NOT NULL'));
		$configtable->addChild(new DBIndex('`name`', 'PRIMARY KEY'));
		$this->format->addChild($configtable);

		$buildstable = new DBTable('builds', 'InnoDB');
		$buildstable->addChild(new DBField('id', 'INT NOT NULL AUTO_INCREMENT'));
		$buildstable->addChild(new DBField('svn_version', 'INT NOT NULL'));
		$buildstable->addChild(new DBField('time', 'TIMESTAMP NOT NULL', 'CURRENT_TIMESTAMP'));
		$buildstable->addChild(new DBField('status', 'INT NOT NULL'));
		$buildstable->addChild(new DBField('error_msg', 'BLOB NOT NULL'));
		$buildstable->addChild(new DBIndex('`id`', 'PRIMARY KEY'));
		$buildstable->addChild(new DBIndex('`time`', 'KEY'));
		$buildstable->addChild(new DBIndex('`svn_version`', 'KEY'));
		$this->format->addChild($buildstable);

		$errortable = new DBTable('test_errors', 'InnoDB');
		$errortable->addChild(new DBField('id', 'INT NOT NULL AUTO_INCREMENT'));
		$errortable->addChild(new DBField('before_id', 'INT NOT NULL'));
		$errortable->addChild(new DBField('last_id', 'INT NOT NULL'));
		$errortable->addChild(new DBField('error_type', 'VARCHAR(10) NOT NULL'));
		$errortable->addChild(new DBField('file', 'VARCHAR(64) NOT NULL'));
		$errortable->addChild(new DBField('line', 'INT NOT NULL'));
		$errortable->addChild(new DBField('error_msg', 'BLOB NOT NULL'));
		$errortable->addChild(new DBIndex('`id`', 'PRIMARY KEY'));
//		$errortable->addChild(new DBIndex('`error_type`', 'KEY')); // posible key alternative for next one
		$errortable->addChild(new DBIndex('`error_type`,`file`', 'KEY', 'error_type_and_file'));
		$errortable->addChild(new DBForeignKey('test_errors_before_id_key','`before_id`', '`builds` (`id`)'));
		$errortable->addChild(new DBForeignKey('test_errors_last_id_key','`last_id`', '`builds` (`id`)'));
		$this->format->addChild($errortable);
		
		$resulttable = new DBTable('test_results', 'InnoDB');
		$resulttable->addChild(new DBField('id', 'INT NOT NULL AUTO_INCREMENT'));
		$resulttable->addChild(new DBField('build_id', 'INT NOT NULL'));
		$resulttable->addChild(new DBField('result', 'VARCHAR(10) NOT NULL'));
		$resulttable->addChild(new DBField('assertions_passed', 'INT NOT NULL'));
		$resulttable->addChild(new DBField('assertions_failed', 'INT NOT NULL'));
		$resulttable->addChild(new DBField('test_cases_passed', 'INT NOT NULL'));
		$resulttable->addChild(new DBField('test_cases_failed', 'INT NOT NULL'));
		$resulttable->addChild(new DBField('test_cases_skipped', 'INT NOT NULL'));
		$resulttable->addChild(new DBField('test_cases_aborted', 'INT NOT NULL'));
		$resulttable->addChild(new DBIndex('`id`', 'PRIMARY KEY'));
		$resulttable->addChild(new DBForeignKey('test_results_build_id_key', '`build_id`', '`builds` (`id`)'));
		$this->format->addChild($resulttable);

		$this->db = $db;
	}

	public function checkDB()
	{
		$this->format->checkDB($this->db);

		$build = new Build();
		$build->insertNull();

		$config = new Config();
		$config->insertDefaults();
	}
}

?>
