class CreateSpecialUnits < ActiveRecord::Migration
  
  def self.up
    create_table :special_units do |t|

      t.column :game_id, :integer
      t.column :level, :integer
      t.column :special_unit_id, :integer
      t.column :experience, :integer
    end
    
    # switch to 'MyISAM'
    execute 'ALTER TABLE special_units ENGINE = MyISAM'
  end

  def self.down

    drop_table :special_units
  end

end
