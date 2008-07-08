class ModifyGamesCampaignNameId < ActiveRecord::Migration
 
    def self.up
    
        # rename campaign_id to campaign_name_id
        rename_column :games, :campaign_id, :campaign_name_id
    
    end

    def self.down
    
        # rename campaign_name_id to campaign_id
        rename_column :games, :campaign_name_id, :campaign_id
    end
  
end
