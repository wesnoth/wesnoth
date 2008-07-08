class CreateDifficultyNames < ActiveRecord::Migration
  
  def self.up
    create_table :difficulty_names do |t|

      t.column :name, :string
    end
    
    # switch to 'MyISAM'
    execute 'ALTER TABLE difficulty_names ENGINE = MyISAM'
  end

  def self.down
    
    drop_table :difficulty_names
  end
  
end
