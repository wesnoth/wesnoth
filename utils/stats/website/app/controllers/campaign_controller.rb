require 'campaign/sort.rb'
require 'campaign/campaign.rb'

class CampaignController < ApplicationController

    # include lib campaign helper
    include CampaignLib
    
    def index
    
        # retrieve elements
        @name = params[:name] || nil
        @difficulty = params[:diff] || nil
        @version = params[:ver] || nil
        
        # now we need to delegate to a proper handler
        
        
    end
    
    
    # '/campaign/about?...' handler (version name must be supplied)
    def about
    
        # make sure all necessary params are present
        if params[:name].nil? || params[:diff].nil? || params[:ver].nil?
        
            redirect_to :controller => 'version', :action => 'index'
            return
        end
        
        # retrieve version_id, difficulty_id, and campaign_id
        version_id = VersionName.find_by_name(params[:ver]).id
        
        # retrieve campaign_id
        campaign_id = CampaignName.find_by_name(params[:name]).id
        
        # retrieve difficulty_id
        difficulty_id = DifficultyName.find_by_name(params[:diff]).id
        

        # grab all scenarios for the given campaign and apply scenario sorting
        scenarios = scenario_sort(CampaignName.all_scenarios_for_campaign(params[:name]))
        
        
        # create new campaign helper
        @obj_campaign = Campaign.new
        
        # store necessary info about this campaign
        @obj_campaign.name = params[:name]
        @obj_campaign.version = params[:ver]
        @obj_campaign.difficulty = params[:diff]
        
        # for every scenario grab all games
        scenarios.each do |scenario|
        
            # create new scenario
            obj_scenario = Scenario.new
            
            # store name
            obj_scenario.name = scenario.name
            
            # grab all games for this scenario, version, difficulty
            arr_games = Game.all_games_for_campaign_version_difficulty_and_scenario(campaign_id, version_id, difficulty_id, scenario.id) 
            
            # iterate through games and store them in this scenario object
            arr_games.each do |game|
            
                # store game
                obj_scenario.games << game
                
                # for each game, calculate number of units of each level
                arr_game_counts = []

                # store counts for this game
                obj_scenario.game_unit_counts << Unit.count_unit_levels(game.id)
                
            end
            
            # store scenario
            @obj_campaign.scenarios << obj_scenario
        end
        
        # compute necessary stats
        @obj_campaign.compute_stats
        
        
        
                
        # Wins (across all scenarios)
        
        # Percent turns used on victory (across all scenarios)
        
        # Total minutes per player (across all scenarios)
        
        # Count of Level 1 units at start of game (across all scenarios)
        
        # Count of Level 2 units at start of game (across all scenarios)
        
        # Count of Level 3 units at start of game (across all scenarios)
    end
end
