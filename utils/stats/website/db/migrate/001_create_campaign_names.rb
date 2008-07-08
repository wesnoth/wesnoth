class CreateCampaignNames < ActiveRecord::Migration
  
  def self.up
    create_table :campaign_names do |t|

      t.column :name, :string
    end
    
    # switch to 'MyISAM'
    execute 'ALTER TABLE campaign_names ENGINE = MyISAM'
  end

  def self.down
    
    drop_table :campaign_names
  end
  
end
