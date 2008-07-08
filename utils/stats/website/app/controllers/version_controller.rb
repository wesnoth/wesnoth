require 'version/branch'

class VersionController < ApplicationController
       

    # general '/version' handler
    #def index
    #    
    #    # redirect to 'list_all'
    #    redirect_to :action => 'list_all'
    #end
  
                    
    
    # '/version/list_all' handler
    # provides a generic listing of branches / versions with links
    def index
            
        # reset branch array
        @arr_branches = []    
            
        # grab a sorted list of all versions
        # group versions by branch i.e. (1.4.0 and 1.4.1 are grouped under 1.4)
        hash_versions = VersionName.all_branches
    
        # iterate through hash
        hash_versions.each do |iter_branch, iter_versions|
        
            # create new branch
            obj_branch = Version::Branch.new
            
            # store name of this branch
            obj_branch.name = iter_branch
                
            # iterate through all versions in this branch
            iter_versions.each do |iter_version|
            
                # create new version
                obj_version = Version::Version.new
                
                # store version name
                obj_version.name = iter_version.name
                
                # retrieve and store number of games for this version
                obj_version.game_count = Game.count_games_for_version(iter_version.id)
                #obj_version.game_count = iter_version.games.size
                
                # increment branch game count
                obj_branch.game_count += obj_version.game_count
            
                # store version for in this branch
                obj_branch.versions << obj_version
            end
            
            # index versions in this branch
            obj_branch.index_versions_by_subversion
            
            # store branch
            @arr_branches << obj_branch
        end
    end
    
    
    # '/version/about/<name>' handler (version name must be supplied)
    def about
            
        # make sure we have 'name' param present and is valid    
        if params[:name].nil?
        
            redirect_to :action => 'index'
            return
        end
            
        # find version with the given name (we need id)
        hash_version = VersionName.find_by_name(params[:name])
        
        if hash_version.nil?
        
            redirect_to :action => 'index'
            return
        end

        # make sure we have 'name' is valid/present  
        #begin
        #    
        #    hash_version = VersionNames.find_by_name(params[:name])
        #    
        #    logger.info("--")
        #    logger.info(hash_version)
        #
        #rescue ActiveRecord::RecordNotFound
        #
        #    # redirect to 'list_all' if missing the 'id'
        #    redirect_to :action => 'list_all'
        #    return
        #end
        
        # grab a sorted list of all campaigns
        hash_campaigns = CampaignName.all_order_by_name
        
        # create new version
        @obj_version = Version::Version.new
        
        # store version name
        @obj_version.name = hash_version.name
        
        # iterate through hash
        hash_campaigns.each do |iter_campaign|
                    
            # calculate how many games for this version and this campaign
            game_count = Game.count_games_for_campaign_and_version(iter_campaign.id, hash_version.id)
        
            if game_count > 0
            
                # create new campaign
                obj_campaign = Version::Campaign.new
                
                # store game count, name
                obj_campaign.game_count = game_count
                obj_campaign.name = iter_campaign.name
                
                # increment game count for this version
                @obj_version.game_count += obj_campaign.game_count
                
                # now we need to retrieve difficulties and game counts for this campaign
                hash_difficulties = DifficultyName.all_order_by_name
                
                # iterate through hash
                hash_difficulties.each do |iter_difficulty|
                
                    # create new difficulty
                    obj_difficulty = Version::Difficulty.new
                    
                    # calculate how many games for this version, difficulty and this campaign
                    game_count = Game.count_games_for_campaign_version_and_difficulty(iter_campaign.id, hash_version.id, iter_difficulty.id)
                    
                    # store game count, name
                    obj_difficulty.game_count = game_count
                    obj_difficulty.name = iter_difficulty.name
                
                    # store difficulty for this campaign
                    obj_campaign.difficulties << obj_difficulty
                end
                               
                # store this campaign
                @obj_version.campaigns << obj_campaign 
            end
        end
    end

end
