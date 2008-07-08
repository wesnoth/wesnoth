class CreateUnitNames < ActiveRecord::Migration
  
  def self.up
    create_table :unit_names do |t|

      t.column :name, :string
    end
    
    # switch to 'MyISAM'
    execute 'ALTER TABLE unit_names ENGINE = MyISAM'
  end

  def self.down
    
    drop_table :unit_names
  end
  
end
