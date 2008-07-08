class CampaignName < ActiveRecord::Base

    # there are multiple games for each campaign name
    has_many :games
    
    # there are multiple scenarios for this campaign name
    has_many :scenario_names, :through => :games
    
    
    # Return all campaigns, ordered by name
    def self.all_order_by_name
            
        CampaignName.find :all, :order => "name ASC"
                            
    end
    
    
    # find all scenarios for a given campaign
    def self.all_scenarios_for_campaign(campaign_name)
    
        #CampaignName.find_all_by_name   campaign_name,
        #                                :first,
        #                                :select => "scenario_names.name",
        #                                :include => [:scenario_names]
        
        CampaignName.find(  :first,
                            :conditions => ["campaign_names.name = :campaign_name", {:campaign_name => campaign_name}],
                            :select => "scenario_names.*",
                            :include => [:scenario_names]).scenario_names
                
    end
    
end
