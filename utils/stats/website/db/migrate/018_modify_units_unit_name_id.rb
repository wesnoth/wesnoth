class ModifyUnitsUnitNameId < ActiveRecord::Migration

    def self.up
    
        # rename unit_id to unit_name_id
        rename_column :units, :unit_id, :unit_name_id
    
    end

    def self.down
    
        # rename unit_name_id to unit_id
        rename_column :units, :unit_name_id, :unit_id
    end
end
