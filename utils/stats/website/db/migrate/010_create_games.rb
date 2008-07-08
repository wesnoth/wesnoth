class CreateGames < ActiveRecord::Migration
  
  def self.up
    create_table :games do |t|

      t.column :player_id, :integer
      t.column :version_id, :integer
      t.column :campaign_id, :integer
      t.column :scenario_id, :integer
      t.column :difficulty_id, :integer
      t.column :number_turns, :integer
      t.column :start_turn, :integer
      t.column :end_turn, :integer
      t.column :start_time, :integer
      t.column :end_time, :integer
      t.column :gold, :integer
      t.column :end_gold, :integer
      t.column :status, :integer
      t.column :serial, :string
    end
    
    # switch to 'MyISAM'
    execute 'ALTER TABLE games ENGINE = MyISAM'
  end

  def self.down
    
    drop_table :games
  end
  
end
