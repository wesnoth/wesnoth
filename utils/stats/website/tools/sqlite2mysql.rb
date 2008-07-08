#!/usr/local/bin/ruby
#
# Mykola 'radx' Konyk, gamedev_AT_gmail_DOT_com (c) 2008
# Battle of Wesnoth, www.wesnoth.org
#
# Process wesnoth stats sqlite3 dbase and load it into mysql stats dbase

require 'rubygems'
require 'mysql'

require 'wmlparser'


# Connection defs       
DB_UPLOAD_ADDRESS                   = "localhost"
DB_UPLOAD_SCHEMA                    = "wesnoth"
DB_UPLOAD_USER                      = "my_user_name_here"
DB_UPLOAD_PASS                      = "my_long_passwd_here"

DB_UPLOAD_TABLE_PLAYERS             = "players"
DB_UPLOAD_TABLE_VERSION_NAMES       = "version_names"
DB_UPLOAD_TABLE_CAMPAIGN_NAMES      = "campaign_names"
DB_UPLOAD_TABLE_SCENARIO_NAMES      = "scenario_names"
DB_UPLOAD_TABLE_DIFFICULTY_NAMES    = "difficulty_names"
DB_UPLOAD_TABLE_GAMES               = "games"
DB_UPLOAD_TABLE_SPECIAL_UNITS       = "special_units"
DB_UPLOAD_TABLE_SPECIAL_UNIT_NAMES  = "special_unit_names"
DB_UPLOAD_TABLE_UNITS               = "units"
DB_UPLOAD_TABLE_UNIT_NAMES          = "unit_names"

class WesnothSQLiteToMySQL
    
    #--
    # Static method - upload the given game
    def self.upload(dbase_name, game_id)
        
        # Generate WML stats file
        system("ruby sqlitedb2wml.rb #{dbase_name} #{game_id} > dump_wml.txt")
        
        # Read the file, line by line into array
        objFile = File.open("dump_wml.txt", File::RDONLY)
        
        arrBuf = []
        while (sLine = objFile.gets)
            
            arrBuf << sLine
        end
        
        objFile.close
        
        # Create WML DOM object
        objRoot = WMLParser.parse(arrBuf)
        
        # Debug dom dump
        #objFile = File.open("dump_wml_dom.txt", File::RDWR|File::TRUNC|File::CREAT)
        #objRoot.debug_print(objFile, 0)
        #objFile.close
        
        # Check if root is valid
        if not objRoot.get_value("id").nil?
            
            refPlayerId = nil
            refVersionId = nil
            refCampaignId = nil
            refScenarioId = nil
            refDifficultyId = nil
            refGameId = nil
            refSpecialUnitId = nil
            refUnitId = nil
            
            sVersion = objRoot.get_value("version")
            sId = objRoot.get_value("id")
            sSerial = objRoot.get_value("serial")
            sFormat = objRoot.get_value("format_version")
            
            
            # Check serial
            if sSerial.to_i == 0
                
                # If serial is 0 (coming from SQLite wesnoth database) generate a new one
                arrChars = ("0".."9").to_a
                sSerial = ""
                1.upto(16) { |i| sSerial << arrChars[rand(arrChars.size - 1)] }
                sSerial = "10" + sSerial;
            end
            
            
            
            # Open the connection
            objDB = Mysql.real_connect(DB_UPLOAD_ADDRESS, DB_UPLOAD_USER, DB_UPLOAD_PASS, DB_UPLOAD_SCHEMA) 
           
            
            
            
            
            # Add version
            sBuf = objDB.escape_string("#{sVersion}")
            objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_VERSION_NAMES}.id FROM #{DB_UPLOAD_TABLE_VERSION_NAMES} WHERE #{DB_UPLOAD_TABLE_VERSION_NAMES}.name = '#{sBuf}'")

            # We don't have such row, add version
            if 0 == objQuery.num_rows
                
                # Insert the data
                objDB.query("INSERT INTO #{DB_UPLOAD_TABLE_VERSION_NAMES} (name) VALUES ('#{sBuf}')")
                
            end
           
            # We need to get version id
            objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_VERSION_NAMES}.id FROM #{DB_UPLOAD_TABLE_VERSION_NAMES} WHERE #{DB_UPLOAD_TABLE_VERSION_NAMES}.name = '#{sBuf}'")
            refVersionId = (objQuery.fetch_row)[0]
           
           
           
           
           
           
           
            # Add player
            sBuf = objDB.escape_string("#{sId}")
            # SELECT players.id FROM players WHERE players.player_id = '5'
            objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_PLAYERS}.id FROM #{DB_UPLOAD_TABLE_PLAYERS} WHERE #{DB_UPLOAD_TABLE_PLAYERS}.unique_player_id = '#{sBuf}'")
                
            
            # We don't have such row, add player
            if 0 == objQuery.num_rows
                
                # Insert the data
                objDB.query("INSERT INTO #{DB_UPLOAD_TABLE_PLAYERS} (unique_player_id) VALUES ('#{sBuf}')")
                
                
            end
            
            # We need to get player_id
            objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_PLAYERS}.id FROM #{DB_UPLOAD_TABLE_PLAYERS} WHERE #{DB_UPLOAD_TABLE_PLAYERS}.unique_player_id = '#{sBuf}'")
            refPlayerId = (objQuery.fetch_row)[0]
            
            
            
            
            objRoot.get_children().each do |objNodeGame|
                
                # Check if we have 'game' node(s)
                if objNodeGame.get_name() == "game"
                    
                    # Get campaign
                    sCampaign = objNodeGame.get_value("campaign")
                    if not sCampaign.nil?
                        
                        sBuf = objDB.escape_string("#{sCampaign}")
                        objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_CAMPAIGN_NAMES}.id FROM #{DB_UPLOAD_TABLE_CAMPAIGN_NAMES} WHERE #{DB_UPLOAD_TABLE_CAMPAIGN_NAMES}.name = '#{sBuf}'")
                        
                        # We don't have such row, add campaign
                        if 0 == objQuery.num_rows
                
                            # Insert the data
                            objDB.query("INSERT INTO #{DB_UPLOAD_TABLE_CAMPAIGN_NAMES} (name) VALUES ('#{sBuf}')")
                        end
                        
                        # We need to get campaign_id
                        objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_CAMPAIGN_NAMES}.id FROM #{DB_UPLOAD_TABLE_CAMPAIGN_NAMES} WHERE #{DB_UPLOAD_TABLE_CAMPAIGN_NAMES}.name = '#{sBuf}'")
                        refCampaignId = (objQuery.fetch_row)[0]
                    end
                    
                    
                    
                    
                    
                    # Get scenario
                    sScenario = objNodeGame.get_value("scenario")
                    if not sScenario.nil?
                        
                        sBuf = objDB.escape_string("#{sScenario}")
                        objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_SCENARIO_NAMES}.id FROM #{DB_UPLOAD_TABLE_SCENARIO_NAMES} WHERE #{DB_UPLOAD_TABLE_SCENARIO_NAMES}.name = '#{sBuf}'")
                        
                        # We don't have such row, add scenario
                        if 0 == objQuery.num_rows
                
                            # Insert the data
                            objDB.query("INSERT INTO #{DB_UPLOAD_TABLE_SCENARIO_NAMES} (name) VALUES ('#{sBuf}')")
                        end
                        
                        # We need to get scenario id
                        objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_SCENARIO_NAMES}.id FROM #{DB_UPLOAD_TABLE_SCENARIO_NAMES} WHERE #{DB_UPLOAD_TABLE_SCENARIO_NAMES}.name = '#{sBuf}'")
                        refScenarioId = (objQuery.fetch_row)[0]
                    end
                    
                    
                    
                    
                    # Get difficulty
                    sDifficulty = objNodeGame.get_value("difficulty")
                    if not sDifficulty.nil?
                        
                        sBuf = objDB.escape_string("#{sDifficulty}")
                        objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_DIFFICULTY_NAMES}.id FROM #{DB_UPLOAD_TABLE_DIFFICULTY_NAMES} WHERE #{DB_UPLOAD_TABLE_DIFFICULTY_NAMES}.name = '#{sBuf}'")
                        
                        # We don't have such row, add difficulty
                        if 0 == objQuery.num_rows
                
                            # Insert the data
                            objDB.query("INSERT INTO #{DB_UPLOAD_TABLE_DIFFICULTY_NAMES} (name) VALUES ('#{sBuf}')")
                        end
                        
                        # We need to get difficulty id
                        objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_DIFFICULTY_NAMES}.id FROM #{DB_UPLOAD_TABLE_DIFFICULTY_NAMES} WHERE #{DB_UPLOAD_TABLE_DIFFICULTY_NAMES}.name = '#{sBuf}'")
                        refDifficultyId = (objQuery.fetch_row)[0]
                    end
                    
                    
                    
                    
                    # Get all necessary values for game
                    sNumTurns = objNodeGame.get_value("num_turns")
                    sStartTurn = objNodeGame.get_value("start_turn")
                    sGold = objNodeGame.get_value("gold")
                    sStartTime = objNodeGame.get_value("time")
                    
                    sEndTime = nil
                    sEndTurn = nil
                    sEndGold = nil
                    sStatus = "3" # 0 - quit, 1 - victory, 2 - defeat, 3 - unknown
                    
                    # Fix start turn
                    if sStartTurn.nil?
                        
                        sStartTurn = "0"
                    end
                    
                    # Fix num turns
                    if sNumTurns.nil?
                        
                        sNumTurns = "0"
                    end
                    
                    # Fix gold
                    if sGold.nil?
                        
                        sGold = "0"
                    end
                    
                    # Fix start time
                    if sStartTime.nil?
                        
                        sStartTime = "0"
                    end
                    
                    # Retrieve end result
                    objNodeResult = objNodeGame.get_child("quit")
                    if not objNodeResult.nil?
                        
                        sEndTime = objNodeResult.get_value("time")
                        sEndTurn = objNodeResult.get_value("end_turn")
                        
                        sEndGold = "0"
                        sStatus = "0" 
                    end
                    
                    objNodeResult = objNodeGame.get_child("victory")
                    if not objNodeResult.nil?
                        
                        sEndTime = objNodeResult.get_value("time")
                        sEndTurn = objNodeResult.get_value("end_turn")
                        sEndGold = objNodeResult.get_value("gold")
                        
                        sStatus = "1" 
                    end
                    
                    objNodeResult = objNodeGame.get_child("defeat")
                    if not objNodeResult.nil?
                        
                        sEndTime = objNodeResult.get_value("time")
                        sEndTurn = objNodeResult.get_value("end_turn")
                        
                        sEndGold = "0"
                        sStatus = "2" 
                    end
                    
                    
                    
                    # Add game
                    sEscPlayerId = objDB.escape_string("#{refPlayerId}")
                    sEscVersionId = objDB.escape_string("#{refVersionId}")
                    sEscCampaignId = objDB.escape_string("#{refCampaignId}")
                    sEscScenarioId = objDB.escape_string("#{refScenarioId}")
                    sEscDifficultyId = objDB.escape_string("#{refDifficultyId}")
                    sEscNumTurns = objDB.escape_string("#{sNumTurns}")
                    sEscStartTurn = objDB.escape_string("#{sStartTurn}")
                    sEscEndTurn = objDB.escape_string("#{sEndTurn}")
                    sEscStartTime = objDB.escape_string("#{sStartTime}")
                    sEscEndTime = objDB.escape_string("#{sEndTime}")
                    sEscGold = objDB.escape_string("#{sGold}")
                    sEscEndGold = objDB.escape_string("#{sEndGold}")
                    sEscStatus = objDB.escape_string("#{sStatus}")
                    sEscSerial = objDB.escape_string("#{sSerial}")
                    
                    objDB.query("INSERT INTO #{DB_UPLOAD_TABLE_GAMES} (player_id, version_name_id, campaign_id, \
                                scenario_id, difficulty_id, number_turns, start_turn, \
                                end_turn, start_time, end_time, gold, end_gold, status, serial) \
                                VALUES ('#{sEscPlayerId}', '#{sEscVersionId}', '#{sEscCampaignId}', \
                                '#{sEscScenarioId}', '#{sEscDifficultyId}', '#{sEscNumTurns}', \
                                '#{sEscStartTurn}', '#{sEscEndTurn}', '#{sEscStartTime}', \
                                '#{sEscEndTime}', '#{sEscGold}', '#{sEscEndGold}', '#{sEscStatus}', '#{sEscSerial}')")
                    
                    # need to get game id
                    objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_GAMES}.id FROM #{DB_UPLOAD_TABLE_GAMES} WHERE #{DB_UPLOAD_TABLE_GAMES}.serial = '#{sEscSerial}' AND #{DB_UPLOAD_TABLE_GAMES}.player_id = '#{sEscPlayerId}' AND #{DB_UPLOAD_TABLE_GAMES}.scenario_id = '#{sEscScenarioId}'")
                    refGameId = (objQuery.fetch_row)[0]
                    
                    
                    
                    
                    # Process special units
                    objNodeGame.get_children().each do |objSpecialUnit|
                        
                        if objSpecialUnit.get_name() == "special-unit"
                            
                            sUnitName = objSpecialUnit.get_value("name")
                            sUnitLevel = objSpecialUnit.get_value("level")
                            sUnitExperience = objSpecialUnit.get_value("experience")
                            
                            sBuf = objDB.escape_string("#{sUnitName}")
                            objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_SPECIAL_UNIT_NAMES}.id FROM #{DB_UPLOAD_TABLE_SPECIAL_UNIT_NAMES} WHERE #{DB_UPLOAD_TABLE_SPECIAL_UNIT_NAMES}.name = '#{sBuf}'")

                            # Check if we have such special unit
                            if 0 == objQuery.num_rows
                                
                                objDB.query("INSERT INTO #{DB_UPLOAD_TABLE_SPECIAL_UNIT_NAMES} (name) VALUES ('#{sBuf}')")
                            end
                            
                            # Get unit id
                            objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_SPECIAL_UNIT_NAMES}.id FROM #{DB_UPLOAD_TABLE_SPECIAL_UNIT_NAMES} WHERE #{DB_UPLOAD_TABLE_SPECIAL_UNIT_NAMES}.name = '#{sBuf}'")
                            refSpecialUnitId = (objQuery.fetch_row)[0]
 
 
 
                
                            # Insert unit into the given game
                            sEscGameId = objDB.escape_string("#{refGameId}")
                            sEscUnitLevel = objDB.escape_string("#{sUnitLevel}")
                            sEscUnitExperience = objDB.escape_string("#{sUnitExperience}")
                            sEscSpecialUnitId = objDB.escape_string("#{refSpecialUnitId}")
                            
                            objDB.query("INSERT INTO #{DB_UPLOAD_TABLE_SPECIAL_UNITS} (game_id, level, special_unit_id, experience) VALUES ('#{sEscGameId}', '#{sEscUnitLevel}', '#{sEscSpecialUnitId}', '#{sEscUnitExperience}')")

                        end
                        
                    end
                    
                    
                    
                    
                    # Process regular units
                    objRegularUnits = objNodeGame.get_child("units-by-level")
                    if not objRegularUnits.nil?
                        
                        # Iterate through levels
                        objRegularUnits.get_children().each do |objNodeLevel|
                           
                            sLevel =  objNodeLevel.get_name()
                           
                             # Iterate through units
                            objNodeLevel.get_children().each do |objRegularUnit|
                               
                                sUnitName = objRegularUnit.get_name()
                                sUnitCount = objRegularUnit.get_value("count")
                                
                                
                                sBuf = objDB.escape_string("#{sUnitName}")
                                objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_UNIT_NAMES}.id FROM #{DB_UPLOAD_TABLE_UNIT_NAMES} WHERE #{DB_UPLOAD_TABLE_UNIT_NAMES}.name = '#{sBuf}'")

                                # Check if we have such special unit
                                if 0 == objQuery.num_rows
                                
                                    objDB.query("INSERT INTO #{DB_UPLOAD_TABLE_UNIT_NAMES} (name) VALUES ('#{sBuf}')")
                                end
                            
                                # Get unit id
                                objQuery = objDB.query("SELECT #{DB_UPLOAD_TABLE_UNIT_NAMES}.id FROM #{DB_UPLOAD_TABLE_UNIT_NAMES} WHERE #{DB_UPLOAD_TABLE_UNIT_NAMES}.name = '#{sBuf}'")
                                refUnitId = (objQuery.fetch_row)[0]
                                
                                
                                # Insert unit into the given game
                                sEscGameId = objDB.escape_string("#{refGameId}")
                                sEscUnitLevel = objDB.escape_string("#{sLevel}")
                                sEscUnitCount = objDB.escape_string("#{sUnitCount}")
                                sEscUnitId = objDB.escape_string("#{refUnitId}")
                            
                                objDB.query("INSERT INTO #{DB_UPLOAD_TABLE_UNITS} (game_id, level, unit_id, count) VALUES ('#{sEscGameId}', '#{sEscUnitLevel}', '#{sEscUnitId}', '#{sEscUnitCount}')")

                            end
                           
                        end
                    end
                    
                end
            end
            
            
            
           
            # Close the connection
            objDB.close() 
        end
    end
    
end

#WesnothUpload.upload("wesnoth_2.db", ARGV[0])
