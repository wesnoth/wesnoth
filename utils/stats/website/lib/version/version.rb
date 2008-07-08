require 'version/campaign'

module Version

    # Represents a version with additional info (game count, dates)
    class Version
    
        attr_accessor :game_count, :name, :campaigns
        
        def initialize
        
            @name = nil
            @game_count = 0
            @campaigns = []
        end
        
    end
end