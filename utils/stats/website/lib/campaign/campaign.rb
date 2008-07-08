require 'campaign/scenario'

require 'googlechart/chart_bar'
require 'googlechart/chart_xy'

module CampaignLib

    include GoogleChartLib
    
    # Represents a campaign instance, with a set of scenarios
    class Campaign
    
        attr_accessor :scenarios, :name, :difficulty, :version
        
        def initialize
        
            @scenarios = []
            @scenario_blocks = []
            
            @name = ""
            @difficulty = ""
            @version = ""
        end
        
        
        # compute stats for this campaign
        def compute_stats
        
            # compute stats for each scenario
            @scenarios.each do |scenario|
        
                # compute gold at the start of the game (across all scenarios)
                scenario.compute_start_gold
                
                # compute Losses/quits (across all scenarios)
                scenario.compute_wins_losses_quits
                
                # compute victory percent turns (across all scenarios)
                scenario.compute_victory_percent_turns
                
                # compute unit counts
                scenario.compute_unit_counts
            end
            
            # partition scenarios into blocks, so we can generate charts for each block
            create_scenario_blocks
        end
        
        
        # create chart for unit count (given level)
        def create_charts_unit_count(level)
        
            # we are going to generate an array of charts showing
            # unit counts for the given level
            arr_charts = []
            
            # for each block generate a chart
            @scenario_blocks.each do |scenario_block|
            
                arr_data = []
                
                sym_level = "level_#{level}".to_sym

                # store means and medians
                arr_data << scenario_block.map { |scenario| scenario.stat_unit_counts[sym_level].mean }
                arr_data << scenario_block.map { |scenario| scenario.stat_unit_counts[sym_level].median }
                
                # store names
                arr_labels = []
                
                scenario_block.each do |scenario|
                
                    if scenario.name.length > 20
                    
                        arr_labels << scenario.name[0, 20] + ".."
                    else
                    
                        arr_labels << scenario.name
                    end
                end
                
                #scenario_block.map { |scenario| scenario.name[0, 20] }

                obj_chart = chart_xy    :title => "Count of level #{level} units: #{arr_labels.first} - #{arr_labels.last}",
                                        :size => [700, 200],
                                        :labels => arr_labels,
                                        :data => arr_data,
                                        :legend => ['Mean', 'Median'],
                                        :colors => ['0033FF', 'FF6600'],
                                        :legend_position => :top

                # store chart (unit counts), median and mode
                arr_charts << obj_chart
                
                
                arr_data = []
                
                # store max and mode
                arr_data << scenario_block.map { |scenario| scenario.stat_unit_counts[sym_level].max }
                arr_data << scenario_block.map { |scenario| scenario.stat_unit_counts[sym_level].mode }
                
                obj_chart = chart_xy    :title => "Count of level #{level} units: #{arr_labels.first} - #{arr_labels.last}",
                                        :size => [700, 200],
                                        :labels => arr_labels,
                                        :data => arr_data,
                                        :legend => ['Maximum', 'Mode'],
                                        :colors => ['990000', '007700'],
                                        :legend_position => :top

                # store chart (unit counts), median and mode
                arr_charts << obj_chart
            end
            
            return arr_charts
        end
        
        
        # create chart for number of turns % wins
        def create_charts_victory_percent_turns
        
            # we are going to generate an array of charts showing
            # victory percent turns 
            arr_charts = []

            # for each block generate a chart
            @scenario_blocks.each do |scenario_block|
            
                arr_data = []
                
                # store means and medians
                arr_data << scenario_block.map { |scenario| scenario.victory_percent_turns.mean }
                arr_data << scenario_block.map { |scenario| scenario.victory_percent_turns.median }
            
                # store names
                #arr_labels = scenario_block.map { |scenario| scenario.name[0, 20] }
                
                # store names
                arr_labels = []
                
                scenario_block.each do |scenario|
                
                    if scenario.name.length > 20
                    
                        arr_labels << scenario.name[0, 20] + ".."
                    else
                    
                        arr_labels << scenario.name
                    end
                end
                
                # create chart
                obj_chart = chart_xy    :title => "Percent turns used on victory: #{arr_labels.first} - #{arr_labels.last}",
                                        :size => [700, 200],
                                        :labels => arr_labels,
                                        :data => arr_data,
                                        :legend => ['Mean', 'Median'],
                                        :colors => ['006400', 'FF6600'],
                                        :legend_position => :top
                                                            
                # store chart
                arr_charts << obj_chart
            end
        
            return arr_charts
        end
        
        
        # create chart for gold at the start of the game
        def create_charts_gold_start_game
        
            # we are going to generate an array of charts showing
            # gold at the start of the game
            arr_charts = []
            
            # for each block (batch) generate a chart
            @scenario_blocks.each do |scenario_block|

                arr_data = []
                
                arr_data << scenario_block.map { |scenario| scenario.gold_start_game.mean }
                arr_data << scenario_block.map { |scenario| scenario.gold_start_game.median }
                
                #arr_labels = scenario_block.map { |scenario| scenario.name[0, 20] }
                
                # store names
                arr_labels = []
                
                scenario_block.each do |scenario|
                
                    if scenario.name.length > 20
                    
                        arr_labels << scenario.name[0, 20] + ".."
                    else
                    
                        arr_labels << scenario.name
                    end
                end
                
                # create chart
                obj_chart = chart_xy    :data => arr_data,
                                        :labels => arr_labels,
                                        :size => [700, 200],
                                        :colors => ['FF0000', '0000FF'],
                                        :title => "Gold at start of the game: #{arr_labels.first} - #{arr_labels.last}",
                                        :legend => ['Mean', 'Median'],
                                        :legend_position => :top
                             
                
                # store chart
                arr_charts << obj_chart
            end
            
            # return all charts for the given campaign
            return arr_charts
        end
        
                  
        # create chart for wins/losses/quits
        def create_charts_wins_losses_quits
        
            arr_charts = []
            
            # for each block (batch) generate a chart
            @scenario_blocks.each do |scenario_block|
            
                arr_data = []
                
                arr_data << scenario_block.map { |scenario| scenario.status_wins.percentage }
                arr_data << scenario_block.map { |scenario| scenario.status_losses.percentage }
                arr_data << scenario_block.map { |scenario| scenario.status_quits.percentage }    
                
                #arr_labels = scenario_block.map { |scenario| scenario.name[0, 20] } 
                
                # store names
                arr_labels = []
                
                scenario_block.each do |scenario|
                
                    if scenario.name.length > 20
                    
                        arr_labels << scenario.name[0, 20] + ".."
                    else
                    
                        arr_labels << scenario.name
                    end
                end
            
                # create chart
                obj_chart = chart_bar   :title => "Wins, Losses, Quits percentage: #{arr_labels.first} - #{arr_labels.last}",
                                        :data => arr_data,
                                        :size => [700 * scenario_block.size / 4, 250],
                                        :grouped => true,
                                        :width_spacing => [30, 5, 50],
                                        :colors => ['006400', 'CC1100', 'EEAD0E'],
                                        :legend => ['Wins %', 'Losses %', 'Quits %'],
                                        :legend_position => :top,
                                        :grid => [100, 10],
                                        :axis_labels => [
                                                            {:axis => :y, :labels => Array.new(11) {|i| (i * 10).to_s}},
                                                            {:axis => :x, :labels => arr_labels}
                                                        ]
                                                        
                
                # store chart
                arr_charts << obj_chart
            end
            
            # return all charts for the given campaign
            return arr_charts
        end
        
        
        # partition list of scenarios into blocks
        # this helps to avoid data cluttering (create a series of charts)
        private
        def create_scenario_blocks
        
            @scenario_blocks = []
            arr_current_block = []
            
            obj_trans = nil
            pos = 0
            
            @scenarios.each do |scenario|
            
                if pos > 0 && 0 == pos % 3
                
                    arr_current_block << scenario 
                    
                    # store previous batch
                    @scenario_blocks << arr_current_block
                    
                    # create new batch
                    arr_current_block = []
                    
                    # store border element
                    if scenario != @scenarios.last
                    
                        arr_current_block << scenario
                    end
                else
                
                    # store scenario (current batch)
                    arr_current_block << scenario
                end
            
                pos += 1
            end
            
            # store last batch
            unless arr_current_block.empty?

                @scenario_blocks << arr_current_block
            end
        end
    end
end