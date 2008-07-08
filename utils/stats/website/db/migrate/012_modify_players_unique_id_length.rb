class ModifyPlayersUniqueIdLength < ActiveRecord::Migration
  
  def self.up
    
    # make unique_id max length equal to 30
    change_column :players, :unique_player_id, :string, :limit => 30
  end

  def self.down
    
    # remove unique_id length limit
    change_column :players, :unique_player_id, :string
  end
  
end
