class CreateVersionNames < ActiveRecord::Migration
  
  def self.up
    create_table :version_names do |t|

      t.column :name, :string
    end
    
    # switch to 'MyISAM'
    execute 'ALTER TABLE version_names ENGINE = MyISAM'
  end

  def self.down
    
    drop_table :version_names
  end
  
end
