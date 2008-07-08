class CreateUnits < ActiveRecord::Migration
  
  def self.up
    create_table :units do |t|

      t.column :game_id, :integer
      t.column :level, :integer
      t.column :unit_id, :integer
      t.column :count, :integer
    end
    
    # switch to 'MyISAM'
    execute 'ALTER TABLE units ENGINE = MyISAM'
  end

  def self.down
    
    drop_table :units
  end
  
end
