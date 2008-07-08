class VersionName < ActiveRecord::Base
    
    # there are multiple games for each version
    has_many :games
    
    
    # count how many different versions there are
    def self.count_versions
        
        VersionName.count
    end
    
    
    # return all branches
    def self.all_branches
    
        VersionName.all_order_by_name.group_by { |version| version.name[0, 3] }
        #VersionName.all.group_by { |version| version.name[0, 3] }
    end
    
    
    # return all versions ordered by name
    def self.all_order_by_name
            
        VersionName.find :all, :order => "name DESC"
    end
end
