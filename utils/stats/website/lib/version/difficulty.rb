module Version

    # Represents a difficulty (game counts, name)
    class Difficulty
     
        attr_accessor :game_count, :name
        
        def initialize
        
            @name = nil
            @game_count = 0
        end
    end
end
