require 'rubygems'
require 'gchart'

require 'version/version'

module Version

    # Represents a version branch (e.g. 1.4.x, 1.5.x)
    class Branch
    
        attr_accessor :name, :game_count, :versions
    
        def initialize
        
            @name = nil
            @game_count = 0
            @versions = []
        end
        
        
        # Index subversions in decreasing order
        def index_versions_by_subversion
    
            # index versions in decreasing order
            @versions.sort! do |left, right| 
                
                # extract sub-version values
                left_scan = left.name.scan(/\d+\.\d+\.(\d+)/)
                right_scan = right.name.scan(/\d+\.\d+\.(\d+)/)
                
                if left_scan.size == 0 || right_scan.size == 0
                
                    # if we can't extract - use string comparison
                    left.name <=> right.name
                else
                
                    left_scan.to_s.to_i <=> right_scan.to_s.to_i 
                end
            end
        end
        
        
        # Create branch chart
        def create_branch_chart
    
            arr_data = []
            arr_labels = []
        
            # Go through versions, grab game counts for each
            @versions.each do |version|
        
                # store count
                arr_data << version.game_count
        
                # no need to check for 0
                percentage = sprintf("%.2f", (version.game_count.to_f / game_count.to_f) * 100.to_f)
                
                # proper pluralization
                if 1 == version.game_count

                    pluralization = "game" 
                else
                
                    pluralization = "games"
                end


                label = "#{version.name} (#{percentage}% , #{version.game_count} #{pluralization})" 
                                       
                arr_labels << label
            end
        
            # Construct pie chart for game counts
            Gchart.pie(
                        :data => arr_data,
                        :size => "600x200",
                        :labels => arr_labels,
                        :axis_with_labels => 'x,y'
                        )
        end
    end
end