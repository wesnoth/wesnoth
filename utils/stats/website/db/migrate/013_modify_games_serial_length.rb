class ModifyGamesSerialLength < ActiveRecord::Migration
  
  def self.up
    
    # make serial max length equal to 30
    change_column :games, :serial, :string, :limit => 30
  end

  def self.down
    
    # remove serial length limit
    change_column :games, :serial, :string
  end
  
end
