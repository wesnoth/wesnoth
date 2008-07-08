module CampaignLib

    # Represents statistics information regarding certain aspect
    class Stats
    
        attr_accessor :mean, :median, :mode, :min, :max, :percentage
        
        def initialize
        
            @mean = 0
            @median = 0
            @mode = 0
            
            @min = 0
            @max = 0
            
            @percentage = 0
        end
        
        
        # compute % count on the given set
        def compute_counts(data, set_size)
        
            if set_size > 0
            
                @percentage = ((data.size.to_f / set_size.to_f) * 100).to_i
            end
        end
        
        
        # compute mean, median, min, max, mode on the given set
        def compute(data)
        
            # compute only if we have data
            if data.size > 0
            
                # first sort the data
                data.sort!           
                
                # frequency
                frequency_table = {}
                frequency_max = 0
                
                # go through data and compute necessary stats
                data.each do |element|
                
                    el_value = element.to_i
                    
                    # check min max
                    if el_value > @max
                        
                        @max = el_value
                    end
                    
                    if el_value < @min
                    
                        @min = el_value
                    end
                    
                    # computing mean
                    @mean += el_value
                    
                    # computing mode
                    frequency_table[element] ||= 0
                    frequency_table[element] += 1
                    
                    if frequency_table[element] > frequency_max
                    
                        # calculating element that repeats the most
                        frequency_max = frequency_table[element]
                        @mode = element
                    end
                end
                
                # compute mean
                @mean = @mean / data.size
                
                # compute median
                @median = data[data.size / 2]
            end
        end
        
    end
end