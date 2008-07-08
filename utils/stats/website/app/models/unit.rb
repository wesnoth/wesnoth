class Unit < ActiveRecord::Base

    # unit belongs to a particular game
    belongs_to :game

    # unit belongs to a particular unit_name
    belongs_to :unit_name    
    
    
    # count how many units of a given level there are for a given game
    #def self.count_units_of_level(game_id, level)
    #    
    #    Unit.sum :count, 
    #             :conditions => ["game_id = :game_id AND level = :level", {:game_id => game_id, :level => level}]
    #end
    
    # return unit counts for each level
    def self.count_unit_levels(game_id)
    
        Unit.find  :all,
                   :select => "level, SUM(count) AS level_count",
                   :conditions => ["game_id = :game_id", {:game_id => game_id}],
                   :group => 'level'
    end
end
