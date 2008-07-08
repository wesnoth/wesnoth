require 'rubygems'
require 'gchart'

require 'version/difficulty'

module Version

    # Represents a campaign with additional info (game count by difficulty)
    class Campaign
    
        attr_accessor :game_count, :difficulties, :name
        
        def initialize
        
            @game_count = 0
            @difficulties = []
            @name = nil
        end
        
        # create difficulties chart
        def create_difficulty_chart
        
            arr_data = []
            arr_labels = []
            
            # iterate through each difficulty present
            difficulties.each do |difficulty|
            
                if difficulty.game_count > 0
                
                    arr_data << difficulty.game_count 
                    
                    percentage = sprintf("%.2f", (difficulty.game_count.to_f / game_count.to_f) * 100.to_f)
                    
                    # proper pluralization
                    if 1 == difficulty.game_count

                        pluralization = "game" 
                    else
                
                        pluralization = "games"
                    end
                
                    label = "#{difficulty.name} (#{percentage}% , #{difficulty.game_count} #{pluralization})" 
                   
                    arr_labels << label
                end
            end
            
            # Construct pie chart for game counts
            Gchart.pie(
                        :data => arr_data,
                        :size => "600x200",
                        :labels => arr_labels,
                        :axis_with_labels => 'x,y'
                        )

        end
    end
end