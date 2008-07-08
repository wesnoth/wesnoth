class Game < ActiveRecord::Base
    
    # game has a particular version
    belongs_to :version_name
    
    # game has a particular campaign
    belongs_to :campaign_name
    
    # game has a particular scenario
    belongs_to :scenario_name
    
    # game belongs to a particular difficulty
    #belongs_to :difficulty_name#, :class_name => "DifficultyNames", :foreign_key => "difficulty_name_id"    
    
    # there are multiple games present for each unit
    has_many :units

    # there are multiple games present for each special_unit
    has_many :special_units
    

        
    
    # count how many games of a particular version there are
    def self.count_games_for_version(version_id)
        
        Game.count :conditions => ["version_name_id = :version_id", {:version_id => version_id}]
                    
    end
    
    
    # count how many games for a particular campaign there are
    def self.count_games_for_campaign(campaign_id)
    
        Game.count :conditions => ["campaign_name_id = :campaign_id", {:campaign_id => campaign_id}]
    
    end
    
    
    # count how many games for a particular campaign and version there is
    def self.count_games_for_campaign_and_version(campaign_id, version_id)
    
        Game.count :conditions => ["campaign_name_id = :campaign_id AND version_name_id = :version_id", {:campaign_id => campaign_id, :version_id => version_id}]
    end
    
    
    # count how many games for a particular campaign, version and difficulty there is
    def self.count_games_for_campaign_version_and_difficulty(campaign_id, version_id, difficulty_id)
    
        Game.count :conditions => ["campaign_name_id = :campaign_id AND version_name_id = :version_id AND difficulty_name_id = :difficulty_id", {:campaign_id => campaign_id, :version_id => version_id, :difficulty_id => difficulty_id}]
    end
    
    
    # retrieve all games with given 
    def self.all_games_for_campaign_version_difficulty_and_scenario(campaign_id, version_id, difficulty_id, scenario_id)
     
        Game.find   :all,
                    :conditions => ["campaign_name_id = :campaign_id AND version_name_id = :version_id AND scenario_name_id = :scenario_id AND difficulty_name_id = :difficulty_id", {:campaign_id => campaign_id, :version_id => version_id, :scenario_id => scenario_id, :difficulty_id => difficulty_id}]
    end
                    
    
    # find all unique scenario names for given campaign
    #def self.find_scenarios_for_campaign(name)
    #
    #    # CampaignName.find_by_name(params[:name]).scenario_names.uniq
    #    Game.find   :all,
    #                :select => "DISTINCT scenario_names.name",
    #                :conditions => ["campaign_names.name = :name AND games.campaign_name_id = campaign_names.id AND games.scenario_name_id = scenario_names.id", {:name => name}],
    #                :include => [:campaign_name, :scenario_name]
    #                
    #end

    #def self.bah(version_id)
    #
    #    Games.find( :all,
    #                :conditions => ["version_name_id = :version_id", {:version_id => version_id}],
    #                :select => "version_names.id, games.id",
    #                :include => [:version_name]#[:campaign_name, :version_name]
    #                ) 
    #    
    #end
    
    
    # 
    #def self.find_games_for_version(version_id)
    # 
    #    Games.find( :all,
    #                :conditions => ["version_name_id = :version_id", {:version_id => version_id}],
    #                :select => "
    #                )
    # 
    #end
    
    
    # count how many games for each version there is
    #def self.count_games_for_each_version
    #    
    #    #Games.count(
    #    #            :joins => "JOIN version_names on version_name_id = version_names.id",
    #    #            :group => "version_name_id",
    #    #            :order => "version_names.name"
    #    #            )
    #    
    #    Games.find( :all,
    #                :joins => "JOIN version_names on version_name_id = version_names.id",
    #                #:select => "version_names.id, version_names.name, COUNT(*) as 'game_count'",
    #                :select => "version_names.name, COUNT(*) as 'game_count'",
    #                :group => "version_name_id",
    #                :order => "version_names.name"
    #                #:offset => offset,
    #                #:limit => limit
    #                )
    #end
    
    
    # count how many games for each branch there is
    #def self.count_games_for_branch(branch)
    #
    #    branch_query = branch + "%"
    #            
    #    Games.find( :all,
    #                :conditions => ["version_names.name LIKE :branch_query", {:branch_query => branch_query}],
    #                :joins => "JOIN version_names on version_name_id = version_names.id",
    #                :select => "COUNT(*) as 'game_count'"
    #                #:group => "version_name_id",
    #                #:order => "version_names.name"
    #                ) 
    #
    #end
    
end
