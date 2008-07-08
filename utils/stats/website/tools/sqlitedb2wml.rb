#!/usr/local/bin/ruby
#
# Mykola 'radx' Konyk, gamedev_AT_gmail_DOT_com (c) 2008
# Battle of Wesnoth, www.wesnoth.org
#
# Create a WML stats upload file for a given game in Wesnoth SQLite3 dbase.
#
# Example of a raw WML stats file:
#
# format_version="1"
# id="1610977976358634"
# serial="120624248400000001"
# version="1.4"
# [game]
#   campaign="TUTORIAL"
#   difficulty="NORMAL"
#   gold="100"
#   num_turns="12"
#   scenario="tutorial"
#   time="1097"
#   [special-unit]
#       experience="0"
#       level="1"
#       name="student"
#   [/special-unit]
#   [units-by-level]
#       [1]
#           [Fighter]
#               count="1"
#           [/Fighter]
#       [/1]
#   [/units-by-level]
#   [defeat]
#       end_turn="2"
#       time="1125"
#   [/defeat]
# [/game]

require 'rubygems'
require 'sqlite3'

class SQLiteExtractor   
    
    #--
    # Static method - check if params are satisfied / valid
    def self.check_params()
        
        # musth have >= 2 params
        if ARGV.size < 2
            
            puts ""
            puts "Battle of Wesnoth: Create a WML stats upload file for a given game."
            puts ""
            puts "Usage: sqlitedb2wml.rb <database_name> <game_id>"
            puts "Example: sqlitedb2wml.rb my_sqlite_dbase.db 1234"
            
            exit
        end
        
        # check if db param is a valid db file
        if not FileTest.exist?(ARGV[0])
            
            puts ""
            puts "Battle of Wesnoth: Create a WML stats upload file for a given game."
            puts ""
            puts "Cannot perform action: #{ARGV[0]} does not exist!"
            puts "Usage: sqlitedb2wml.rb <database_name> <game_id>"
            
            exit
        end
    end
    
    
    #--
    def self.do_query()
        
        # Open SQLite3 database
        objDB = SQLite3::Database.open(ARGV[0])
        
        # Execute query
        objQuery = objDB.execute("SELECT version_names.name, \
                                            players.id, \
                                            campaign_names.name, \
                                            difficulty_names.name, \
                                            scenario_names.name, \
                                            games.gold, \
                                            games.start_time, \
                                            games.start_turn, \
                                            games.num_turns, \
                                            games.result, \
                                            games.end_time, \
                                            games.end_gold, \
                                            games.end_turn, \
                                            games.game_number \
                                            FROM players, campaign_names, difficulty_names, scenario_names, campaigns, scenarios, \
                                            games, version_names WHERE games.rowid='#{ARGV[1]}' AND games.scenario_ref = scenarios.rowid \
                                            AND scenarios.campaign_ref = campaigns.rowid AND \
                                            campaigns.difficulty_name_ref = difficulty_names.rowid \
                                            AND campaigns.campaign_name_ref = campaign_names.rowid \
                                            AND games.version_name_ref = version_names.rowid \
                                            AND campaigns.player_ref = players.rowid \
                                            AND scenario_names.rowid = scenarios.scenario_name_ref;")
        
        # Check if query is valid (params were valid)
        if objQuery.length == 0
           
            puts ""
            puts "Battle of Wesnoth: Create a WML stats upload file for a given game."
            puts ""
            puts "Cannot perform action: database_name does not contain a valid Wesnoth SQLite3 dbase or game_id (range) is invalid!"
            puts "Usage: sqlitedb2wml.rb <database_name> <game_id>"
           
            # Close dbase
            objDB.close
            exit
        end
        
        objQuery.each do |objRow|
            
            puts "format_version=\"1\""
            puts "version=\"#{objRow[0]}\""
            puts "id=\"#{objRow[1]}\""
            
            # Serial is not required
            puts "serial=\"0\""
            
            puts "[game]"
            puts "\tcampaign=\"#{objRow[2]}\""
            puts "\tdifficulty=\"#{objRow[3]}\""
            puts "\tscenario=\"#{objRow[4]}\""
            
            # Fix negative gold
            #iGold = objRow[5].to_i
            #if iGold < 0
            #    iGold = 0    
            #end
            
	    puts "\tgold=\"#{objRow[5]}\""
            puts "\ttime=\"#{objRow[6]}\""
            puts "\tstart_turn=\"#{objRow[7]}\""
            
            # Handle "-1" for num_turns for some old versions
            #if objRow[8].to_i != -1
            #               
            #    puts "\tnum_turns=\"#{objRow[8]}\""
            #end

            puts "\tnum_turns=\"#{objRow[8]}\""


            
            # Fix negative gold
            #iGold = objRow[11].to_i
            #if iGold < 0
            #    iGold = 0    
            #end
            
            # Produce proper result block
            case objRow[9].to_i
               
                # Quit 
                when 0
                    
                    puts "\t[quit]"
                    puts "\t\ttime=\"#{objRow[10]}\""
                    #puts "\t\tgold=\"#{objRow[11]}\""                    
                    puts "\t\tend_turn=\"#{objRow[12]}\""
                    puts "\t[/quit]"
               
                # Victory
                when 1
                    
                    puts "\t[victory]"
                    puts "\t\ttime=\"#{objRow[10]}\""
                    puts "\t\tgold=\"#{objRow[11]}\""
                    puts "\t\tend_turn=\"#{objRow[12]}\""
                    puts "\t[/victory]"
            
                # Otherwise Defeat        
                else
                    
                    puts "\t[defeat]"
                    puts "\t\ttime=\"#{objRow[10]}\""
                    #puts "\t\tgold=\"#{objRow[11]}\""
                    puts "\t\tend_turn=\"#{objRow[12]}\""
                    puts "\t[/defeat]"
            end
                        
            # We only do one row
            break
        end
        
        
        # Execute special units query
        objQuery = objDB.execute("SELECT unit_names.name, \
                                            special_units.level, \
                                            special_units.experience \
                                            FROM unit_names, special_units WHERE special_units.game_ref = '#{ARGV[1]}' \
                                            AND unit_names.rowid = special_units.unit_name_ref;")
        
        # Check if query is valid (params were valid)
        if objQuery.length == 0
                      
            puts "[/game]"
            
            # Close dbase
            objDB.close
            exit
        end
        
        objQuery.each do |objRow|
            
            #puts objRow
            puts "\t[special-unit]"
            puts "\t\tname=\"#{objRow[0]}\""
            puts "\t\tlevel=\"#{objRow[1]}\""
            puts "\t\texperience=\"#{objRow[2]}\""
            puts "\t[/special-unit]"
            
        end
        
        # Execute special units query
        objQuery = objDB.execute("SELECT unit_types.level, \
                                            unit_types.name, \
                                            unit_tallies.count \
                                            FROM unit_tallies, unit_types WHERE unit_tallies.game_ref = '#{ARGV[1]}' \
                                            AND unit_types.rowid = unit_tallies.unit_type_ref ORDER BY unit_types.level ASC;")

        # Check if query is valid (params were valid)
        if objQuery.length == 0
            
            puts "[/game]"
            
            # Close dbase
            objDB.close
            exit
        end
        
        puts "\t[units-by-level]"
        
        iCurrentLevel = -1
        iCount = 0
        
        objQuery.each do |objRow|
            
            if objRow[0].to_i != iCurrentLevel
                
                # Terminate previous block, if needed
                if iCount != 0
                    
                    puts "\t\t[/#{iCurrentLevel}]"
                end
                
                iCurrentLevel = objRow[0].to_i
                
                # Add new block
                puts "\t\t[#{iCurrentLevel}]"
            end
            
            puts "\t\t\t[#{objRow[1]}]"
            puts "\t\t\t\tcount=\"#{objRow[2]}\""
            puts "\t\t\t[/#{objRow[1]}]"
            
            #puts objRow
            iCount = iCount + 1
        end
        
        # Terminate last block
        if iCount != 0
            
            puts "\t\t[/#{iCurrentLevel}]"
        end
        
        puts "\t[/units-by-level]"
        puts "[/game]"
        
        # Close dbase
        objDB.close
        
    end

end

# 
SQLiteExtractor.check_params
SQLiteExtractor.do_query













