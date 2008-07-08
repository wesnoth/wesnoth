class ModifyGamesDifficultyNameId < ActiveRecord::Migration

    def self.up
    
        # rename difficulty_id to difficulty_name_id
        rename_column :games, :difficulty_id, :difficulty_name_id
    
    end

    def self.down
    
        # rename difficulty_name_id to difficulty_id
        rename_column :games, :difficulty_name_id, :difficulty_id
    end
end
