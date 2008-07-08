class CreatePlayers < ActiveRecord::Migration
  
  def self.up
    create_table :players do |t|

      t.column :player_id, :integer
    end
    
    # switch to 'MyISAM'
    execute 'ALTER TABLE players ENGINE = MyISAM'
  end

  def self.down
    
    drop_table :players
  end
  
end
