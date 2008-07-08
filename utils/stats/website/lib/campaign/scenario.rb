require 'campaign/stats.rb'

module CampaignLib

    # Represents a scenario instance (for particular version, difficulty)
    # Has a set of games played associated with it
    class Scenario
    
        attr_accessor :games, :game_unit_counts
        attr_accessor :name
        attr_accessor :gold_start_game
        attr_accessor :status_wins, :status_losses, :status_quits
        attr_accessor :victory_percent_turns
        attr_accessor :stat_unit_counts

        
        def initialize
        
            @name = nil
            @games = []
            
            @gold_start_game = Stats.new
            
            @status_wins = Stats.new
            @status_losses = Stats.new     
            @status_quits = Stats.new       
            
            @victory_percent_turns = Stats.new
            
            @game_unit_counts = []
            
            @stat_unit_counts = {}
            3.times do |level|
            
                @stat_unit_counts["level_#{level + 1}".to_sym] = Stats.new
            end
        end
        
        
        # compute unit counts
        def compute_unit_counts
        
            # compute stat unit counts for each level
            3.times do |level|
           
                #@stat_unit_counts["level_#{level + 1}".to_sym].compute( game_unit_counts.map { |level_counts| level_counts[level] } )
                
                arr_counts = []
                level_symbol = "level_#{level + 1}".to_sym

                @game_unit_counts.each do |game_unit_counts|
                
                    game_unit_counts.each do |level_counts|
                    
                        # proper level
                        if level_counts.level.to_i == level + 1
                        
                            arr_counts << level_counts.level_count.to_i
                        end
                    end
                end
                
                @stat_unit_counts[level_symbol].compute( arr_counts )
            end
        end
        
        
        # compute start gold
        def compute_start_gold
        
            # compute gold at the start of the game
            @gold_start_game.compute( @games.map { |game| game.gold } )
        end
        
        
        # compute losses/quits
        def compute_wins_losses_quits
         
            @status_wins.compute_counts( @games.select { |game| game.status.to_i == 1 }, @games.size )
            @status_losses.compute_counts( @games.select { |game| game.status.to_i == 2}, @games.size )
            @status_quits.compute_counts( @games.select { |game| game.status.to_i == 0}, @games.size )
        end
        
        
        # compuet percent turns used on victory
        def compute_victory_percent_turns
        
            arr_ratios = []
            
            @games.each do |game|
            
                # only interested in wins (and not interested in games with unlimited or 0 # of turns)
                if game.status.to_i == 1 && game.number_turns.to_i > 0
                
                    arr_ratios << ((game.end_turn.to_f / game.number_turns.to_f) * 100).to_i
                end
            
            end
            
            @victory_percent_turns.compute(arr_ratios)
        end
    end
end