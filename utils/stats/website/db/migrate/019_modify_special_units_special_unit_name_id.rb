class ModifySpecialUnitsSpecialUnitNameId < ActiveRecord::Migration

    def self.up
    
        # rename special_unit_id to special_unit_name_id
        rename_column :special_units, :special_unit_id, :special_unit_name_id
    
    end

    def self.down
    
        # rename special_unit_name_id to special_unit_id
        rename_column :special_units, :special_unit_name_id, :special_unit_id
    end
end
