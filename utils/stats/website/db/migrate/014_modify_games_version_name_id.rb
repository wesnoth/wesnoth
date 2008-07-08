class ModifyGamesVersionNameId < ActiveRecord::Migration
  
  def self.up
    
    # rename version_id to version_name_id
    rename_column :games, :version_id, :version_name_id
    
  end

  def self.down
    
    # rename version_name_id to version_id
    rename_column :games, :version_name_id, :version_id
  end
  
end
