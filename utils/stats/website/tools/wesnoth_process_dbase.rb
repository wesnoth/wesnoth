#!/usr/local/bin/ruby
#
# Mykola 'radx' Konyk, gamedev_AT_gmail_DOT_com (c) 2008
# Battle of Wesnoth, www.wesnoth.org

require 'rubygems'
require 'sqlite3'

require 'sqlite2mysql'

class WesnothProcessStats
    
    def self.process(dbase_name, position_start, position_end)
        
        # Open database
        objDB = SQLite3::Database.open(dbase_name)
        
        # Get total number of rows in this sqlite database
        objQuery = objDB.execute("SELECT COUNT(*) AS 'counter' FROM games")
        
        iGameCount = 0
        
        if objQuery.length == 0
            
            puts "Empty/non-existant SQLite database detected!"
            
            objDB.close
            exit
        else
           
            objQuery.each do |objRow|
               
               puts "#{objRow[0]} total games in this dbase."
               iGameCount = (objRow[0]).to_i
               
               break
            end 
        end
        
        objDB.close
        
        
        
        # Start processing
        iStart = position_start.to_i
        iEnd = position_end.to_i
        
        while (iStart < iGameCount and iStart <= iEnd)
            
            puts "Processing game #{iStart}.."
            
            WesnothSQLiteToMySQL.upload(dbase_name, iStart)
            
            iStart = iStart + 1
        end
        
        puts ""
        puts "Processed #{iEnd - position_start.to_i} game(s)."
    end
end


# execution
#if ARGV.length == 2
#    
#    WesnothProcessStats.process(ARGV[0], ARGV[1], ARGV[1])
#    
#elsif ARGV.length == 3
#    
#    WesnothProcessStats.process(ARGV[0], ARGV[1], ARGV[2])
#    
#else
#    
#    puts "Syntax: script <dbase_name> <start_game_id> <end_game_id>"
#end

