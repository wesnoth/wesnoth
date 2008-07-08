class SpecialUnit < ActiveRecord::Base

    # special unit belongs to a particular game
    belongs_to :game
    
    # special unit belongs to a particular special_unit_name
    belongs_to :special_unit_name
    
end
