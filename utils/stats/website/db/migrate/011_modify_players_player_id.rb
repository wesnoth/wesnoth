class ModifyPlayersPlayerId < ActiveRecord::Migration
  
  def self.up

    # fix a bug, player_id is supposed to be a string
    # also rename it, since it no longer will be used in indexing
    
    change_column :players, :player_id, :string
    rename_column :players, :player_id, :unique_player_id
    
  end

  def self.down
    
    # can't go back
    raise ActiveRecord::IrreversibleMigration
  end

end
