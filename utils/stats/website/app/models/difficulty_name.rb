class DifficultyName < ActiveRecord::Base

    # there are multiple games present for each difficulty name
    has_many :games
    
    def self.all_order_by_name
    
        DifficultyName.find :all, :order => "name ASC"
    end
end
