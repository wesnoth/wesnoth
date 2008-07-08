class ScenarioName < ActiveRecord::Base

    # there are multiple games for each scenario
    has_many :games
    
end
