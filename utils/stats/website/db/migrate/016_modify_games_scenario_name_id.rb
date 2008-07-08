class ModifyGamesScenarioNameId < ActiveRecord::Migration

    def self.up
    
        # rename scenario_id to scenario_name_id
        rename_column :games, :scenario_id, :scenario_name_id
    
    end

    def self.down
    
        # rename scenario_name_id to scenario_id
        rename_column :games, :scenario_name_id, :scenario_id
    end
    
end
