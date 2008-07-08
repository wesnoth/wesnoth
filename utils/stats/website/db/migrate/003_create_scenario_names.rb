class CreateScenarioNames < ActiveRecord::Migration
  
  def self.up
    create_table :scenario_names do |t|

      t.column :name, :string
    end
    
    # switch to 'MyISAM'
    execute 'ALTER TABLE scenario_names ENGINE = MyISAM'
  end

  def self.down
    
    drop_table :scenario_names
  end
  
end
