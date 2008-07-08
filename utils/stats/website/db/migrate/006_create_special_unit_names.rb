class CreateSpecialUnitNames < ActiveRecord::Migration
  
  def self.up
    create_table :special_unit_names do |t|

      t.column :name, :string
    end
    
    # switch to 'MyISAM'
    execute 'ALTER TABLE special_unit_names ENGINE = MyISAM'
  end

  def self.down
    
    drop_table :special_unit_names
  end
  
end
