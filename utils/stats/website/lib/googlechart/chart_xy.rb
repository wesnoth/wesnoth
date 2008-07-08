require 'googlechart/chart'

def chart_xy(hash = {})

    #need to re-process several arguments
    # labels, data, markers
    param_data = hash[:data]
    param_labels = hash[:labels]
    
    
    
    # compute vertical max / min
    y_min = 0
    y_max = 0
        
    param_data.each do |data|
        
        data.each do |element|
            
            val = element.to_i
                
            y_min = val if y_min > val
            y_max = val if y_max < val
        end
    end
    
    
    
    
    # store original range
    y_min_orig = y_min
    y_max_orig = y_max
    
    #[-200, 565] -> [0, 765] -> [0*100, 765*100] -> [0/765, 76500/765]  -> [0, 100]
    y_max = y_max - y_min
        
    y_shift = y_min
    y_shift *= -1 if y_shift < 0
        
    y_min = 0
    
    # compute horizontal spacing
    x_start = 100 / (1 + param_labels.size)
    
    
    
    # reset passed data, we need to recompute it
    hash[:data] = []
    


    # recompute data
    param_data.each do |data|
            
        # horizontal data
        arr_data = []
        
        param_labels.size.times do |iter|
        
            arr_data << (x_start * (iter + 1)).to_s
        end
        
        # store
        hash[:data] << arr_data
        
        # vertical data
        arr_data = []
        
        data.each do |element|
        
            if y_max > 0
                
                arr_data << ((element + y_shift) * 100 / y_max).to_s
            else
            
                arr_data << "0"
            end
        end
        
        # store
        hash[:data] << arr_data
    end
    
    
    
    # process labels - horizontal
    arr_labels_horiz = hash[:labels]
       
    
    
    # compute horizontal label positions
    arr_positions = []
    param_labels.size.times do |iter|
        
        arr_positions << (x_start * (iter + 1)).to_s
    end


    
    # process grid
    if y_max_orig - y_min_orig == 0
    
        # set grid
        hash[:grid] = [x_start, 100]
        
        hash[:axis_labels] = [
                                {:axis => :y, :labels => ['0']},
                                {:axis => :x, :labels => hash[:labels], :positions => arr_positions}
                             ]
    else
    
        # set grid
        hash[:grid] = [x_start, 20]
        
        
        # compute vertical info
        arr_labels_vert = []
        step = (y_max_orig - y_min_orig) / 5.0
            
        6.times do |iter|
        
            arr_labels_vert << (y_min_orig + iter * step).to_s
        end
    
        #y_max_orig_extra_space = y_min_orig + 6 * step
        
        # compute horizontal info
        arr_positions = []
        param_labels.size.times do |iter|
        
            arr_positions << (x_start * (iter + 1)).to_s
        end
        
        hash[:axis_labels] = [
                                {:axis => :y, :labels => arr_labels_vert, :range => [y_min_orig, y_max_orig]},
                                {:axis => :x, :labels => hash[:labels], :positions => arr_positions}
                             ]
    end
    
    
    
    
    # process markers
    
    # reset existing markers
    hash[:markers] = []
    
    # generate circle markers for every point of every line
    param_data.size.times do |iter|
    
        hash[:markers] << {:type => :circle, :size => 5.0, :points => :all, :set => iter}
    end
    


    # generate final image
    GoogleChartLib::Chart.line_xy hash
end