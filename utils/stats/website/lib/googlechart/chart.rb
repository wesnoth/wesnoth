#require 'extensions/enumerable'

module GoogleChartLib

    class Chart
    
        # allowed args for each chart type
        @@allowed_args = {}
        
        # accessors
        attr_reader :url
        
        attr_accessor :type
        attr_accessor :colors, :size, :data, :data_encoding
        attr_accessor :title, :title_color, :title_fontsize
        attr_accessor :legend, :legend_position
        attr_accessor :orientation, :grouped
        attr_accessor :labels
        attr_accessor :axis_labels, :grid
        attr_accessor :zero_line, :width_spacing
        attr_accessor :markers
        
        
        # helper, registers allowed arguments for each chart type
        private
        def self.register_type_args(type, args)
        
            if @@allowed_args[type].nil?
            
                @@allowed_args[type] = []
            end
            
            @@allowed_args[type] += args
        end
        
        
        # register allowed arguments for each
        def self.register
        
            # register bar chart arguments
            register_type_args(:bar, [:colors, :size, :data, :data_encoding])
            register_type_args(:bar, [:title, :title_color, :title_fontsize])
            register_type_args(:bar, [:legend, :legend_position])
            register_type_args(:bar, [:orientation, :grouped])
            register_type_args(:bar, [:axis_labels])
            register_type_args(:bar, [:zero_line, :width_spacing])
            register_type_args(:bar, [:axis_labels, :grid, :markers])
            
            # register line chart arguments
            register_type_args(:line, [:colors, :size, :data, :data_encoding])
            register_type_args(:line, [:title, :title_color, :title_fontsize])
            register_type_args(:line, [:legend, :legend_position])
            register_type_args(:line, [:axis_labels, :grid, :markers])
            
            # register line chart arguments
            register_type_args(:line_xy, [:colors, :size, :data, :data_encoding])
            register_type_args(:line_xy, [:title, :title_color, :title_fontsize])
            register_type_args(:line_xy, [:legend, :legend_position])
            register_type_args(:line_xy, [:axis_labels, :grid, :markers])
            
            # register pie chart arguments
            register_type_args(:pie, [:colors, :size, :data, :data_encoding])
            register_type_args(:pie, [:title, :title_color, :title_fontsize])
            register_type_args(:pie, [:labels])
            
            # register pie 3d chart arguments
            register_type_args(:pie3d, [:colors, :size, :data, :data_encoding])
            register_type_args(:pie3d, [:title, :title_color, :title_fontsize])
            register_type_args(:pie3d, [:labels])

            
        end
        
                        
        # invoke registration
        register
        
        
        # helper, used to filter out unsupported args
        def self.filter(args, allowed_args)
        
            # go through all arguments and check which ones apply
            args.each do |key|
            
                # check if arg is supported
                unless allowed_args.include? key
                
                    # not supported - delete it
                    args.delete(key)
                end
            end
            
            return args
        end

                      
        # handle missing calls
        public
        def self.method_missing(method, args = {})
            
            # check if this is a valid chart
            unless @@allowed_args[method].nil?
            
                # filter unsupported args
                valid_args = filter(args, @@allowed_args[method])
                
                # insert type arg
                valid_args[:type] = method
                
                # create Google Chart URL from given args
                return create(valid_args)
            else
                
                raise "Unsupported chart type: #{method}"
            end
        end


        # constructor
        private
        def initialize(args = {})
            
            # go through arguments and set if they are applicable / valid
            args.each do |key, value|
                
                if self.respond_to? "#{key}="
                
                    unless (value.nil? || value.class.to_s == "Array" && value.empty?)
                    
                        self.send "#{key}=", value
                    end
                end
            end
            
            # call process - this will process arguments and generate the corresponding URL, 
            # providing everything is valid
            process
        end
        
        
        # helper, create instance of this class, process args and return
        # final Google Chart URL
        def self.create(args)
        
        	# create new chart, based on supported args and process arguments
        	chart = Chart.new(args)
        	
			# return final Google Chart URL
            return chart.url
        end
                
        
        # process all arguments (mapped to instance variables) and return 
        # final Google Chart URL
        def process
        
            # go through all instance variables
            arr_processed = instance_variables.map do |variable|
            
                # if the value of this variable is non-null
                unless self.instance_variable_get(variable).nil?
                
                    # generate corresponding handler
                    handler = "process_#{variable.to_s.delete('@')}"
                    
                    # check if handler exists and if so, execute it
                    if self.private_methods(false).include? handler
                    
                        self.send handler
                    end
                end
            end.compact
            
            # construct the Google Chart URL from processed arguments
            @url = "http://chart.apis.google.com/chart?" + arr_processed.join("&")
        end
        
        
        # returns whether the specified data is one-dimensional (array) or
        # multi-dimensional (array of arrays)
        def multi_dimensional?(object)
        
            if check_type(object, :array)
            
                if object.empty?
                
                    return false
                else
                
                    return check_type(object.first, :array)
                end
            end
            
            return false
        end
        
        
        # checks the type of object
        def check_type(object, object_type)
        
            case object_type
            
                when :string
                
                    return object.class.to_s == "String"
                
                when :array
                
                    return object.class.to_s == "Array"
                    
                when :hash
                
                    return object.class.to_s == "Hash"
                    
                when :fixnum
                
                    return object.class.to_s == "Fixnum"
                    
                when :symbol
                
                    return object.class.to_s == "Symbol"
                else
                
                    return false
            end
        end
        
        
        # retrieves value from the hash, use supplied value if does not exist
        def retrieve_value(value, default = nil)
        
            if value.nil?
            
                return default
            end
        
            return value
        end
        
        
        # process chart type
        def process_type
        
            "cht=" + case @type
                    
                        when :bar
                        
                            if @grouped
                                
                                if @orientation == :vertical
                                
                                    "bvg"
                                elsif @orientation == :horizontal
                                
                                    "bhg"
                                else
                                
                                    "bvg"
                                end
                            else
                            
                                if @orientation == :vertical
                                
                                    "bvs"
                                elsif @orientation == :horizontal
                                
                                    "bhs"
                                else
                                
                                    "bvs"
                                end
                            end
                            
                        when :pie
                        
                            "p"
                            
                        when :pie3d
                        
                            "p3"
                            
                        when :line
                        
                            "lc"
                            
                        when :line_xy
                        
                            "lxy"
                    end
        end
        
        
        # process chart size
        # two possible forms:
        # array form [200, 100]
        # string form "700x200"
        def process_size
        
            if check_type(@size, :string)
            
                "chs=" + @size
            elsif check_type(@size, :array)
            
                if @size.size >= 2
               
                    "chs=#{@size[0]}x#{@size[1]}"
                end
            end
        end
        
        
        # process chart title
        def process_title
        
            "chtt=" + @title
        end
        
        
        # process title color/fontsize
        def process_title_color
        
            unless @title_fontsize.nil?
            
                "chts=#{@title_color},#{@title_fontsize}"
            end
        end
        
        
        # process chart legend
        def process_legend
        
            if check_type(@legend, :array)
            
                "chdl=" + @legend.join("|")
            elsif check_type(@legend, :string)
            
                "chdl=" + @legend.gsub(",", "|")
            end
        end
        
        
        # process chart legend position
        def process_legend_position
        
            "chdlp=" + case @legend_position
                            
                            when :left
                                
                                "l"
                            when :right
                                
                                "r"
                            when :top
                            
                                "t"
                            when :bottom
                            
                                "b"
                        end
        end
        
        
        # process chart colors
        # two possible variants: specified as a string (one color - 'ff00ff')
        # or specified as an array of strings ['ff00ff', 'ff0000']
        def process_colors
        
            if check_type(@colors, :string)
            
                "chco=" + @colors
            elsif check_type(@colors, :array)
                      
                "chco=" + @colors.join(",")
            end
        end
        
        
        # process chart labels (pie, pie3d, google-o-meter only)
        def process_labels
        
            # pie charts have special labels
            "chl=" + @labels.join("|")
        end
        
        
        # helper used to process axis information
        # {:axis => :x, :labels => ["Jan", "July"]}, :positions => [10, 20], :range => [0, 20, 40]}
        def process_axis_labels_helper(axis_hash, index)

            ret_axis = nil
            ret_labels = nil
            ret_positions = nil
            ret_range = nil

            # process :axis
            if axis_hash.has_key? :axis
            
                # specified as symbol
                if check_type(axis_hash[:axis], :symbol)
                
                    case axis_hash[:axis]
                            
                        when :x
                                
                            ret_axis = "x"
                        when :bottom
                        
                            ret_axis = "x"
                        when :right
                                
                            ret_axis = "r"
                        when :top
                            
                            ret_axis = "t"
                        when :left
                        
                            ret_axis = "y"
                        when :y
                            
                            ret_axis = "y"
                    end
                    
                elsif check_type(axis_hash[:axis], :string)
                
                    ret_axis = axis_hash[:axis]
                end
            end

            # process :labels
            if axis_hash.has_key? :labels
            
                ret_labels = "#{index}:|#{axis_hash[:labels].join('|')}"
            end
            
            # process :positions
            if axis_hash.has_key? :positions
            
                ret_positions = "#{index},#{axis_hash[:positions].join(',')}"
            end
            
            # process :range
            if axis_hash.has_key? :range
            
                ret_range = "#{index},#{axis_hash[:range].join(',')}"
            end


            # :axis, :labels, :positions, :range
            return ret_axis, ret_labels, ret_positions, ret_range
        end
        
        
        # process chart labels axis
        # [{:axis => :x, :labels => ["Jan", "July"]}, :positions => [10, 20]}, ...]
        def process_axis_labels
        
            if check_type(@axis_labels, :hash)
            
                # only have one axis
                
                # use helper to extract data
                str_axis, str_labels, str_positions, str_range = process_axis_labels_helper(@axis_labels, 0)

                # must have axis and labels
                if str_axis.nil? || str_labels.nil?
                
                    return nil
                end
                
                # pre-process positions info
                if str_positions
                
                    str_positions = "&chxp=" + str_positions
                else
                
                    str_positions = ""
                end
                
                # pre-process range info
                if str_range
                
                    str_range = "&chxr=" + str_range
                else
                
                    str_range = ""
                end
                
                # construct substring from these arguments
                "chxt=#{str_axis}&chxl=#{str_labels}#{str_positions}#{str_range}"

            elsif check_type(@axis_labels, :array)
            
                index = 0
                
                arr_axis = []
                arr_labels = []
                arr_positions = []
                arr_ranges = []
                
                # for every specified axis
                @axis_labels.each do |axis_data|
                
                    # use helper to extract data
                    str_axis, str_labels, str_positions, str_range = process_axis_labels_helper(axis_data, index)
                    
                    # must have axis and labels
                    unless (str_axis.nil? || str_labels.nil?)
                    
                        # store axis
                        arr_axis << str_axis
                        
                        # store labels
                        arr_labels << str_labels
                        
                        # store positions
                        if str_positions
                        
                            arr_positions << str_positions
                        end
                        
                        # store range
                        if str_range
                        
                            arr_ranges << str_range
                        end                    
                    end
                    
                    # increment index
                    index += 1
                end
                
                # pre-process positions info
                str_positions = ""
                
                if arr_positions.size > 0
                
                    str_positions = "&chxp=" + arr_positions.join("|")
                end
                
                # pre-process range
                str_range = ""
                
                if arr_ranges.size > 0
                
                    str_range = "&cxhr=" + arr_ranges.join("|")
                end
                
                # construct substring from these arguments
                "chxt=#{arr_axis.join(',')}&chxl=#{arr_labels.join('|')}#{str_positions}#{str_range}"
                
            end
        end
                
        
        # process chart data
        def process_data
        
            if @data_encoding == :simple
            
                encoding = "s:"
            else
                        
                encoding = "t:"
            end
                
            "chd=" + encoding + if multi_dimensional? @data
                                                    
                                    # data is multi dimensional
                                    @data.collect do |row|
                                    
                                        row.join(",")                                    
                                    end.join("|")
                                else
            
                                    # data is one dimensional
                                    @data.join(",")
                                end
        end
        
        
        # process grid data
        # three ways to specify grid
        # array: [20, 20]
        # array: [20, 20, 1, 20], hash: {:x => 20, :y => 20, :line => 1, :blank => 20}
        def process_grid
        
            if check_type(@grid, :hash)
            
                if (@grid.has_key? :x) && (@grid.has_key? :y)
                
                    arr_params = []
                    
                    # store x / y
                    arr_params << @grid[:x]
                    arr_params << @grid[:y]
                
                    # process line segment
                    if @grid[:line]
                        
                        arr_params << @grid[:line]
                    end
                    
                    # process blank segment
                    if @grid[:blank]
                    
                        arr_params << @grid[:blank]
                    end
                    
                    "chg=" + arr_params.join(",")
                end
                
            elsif check_type(@grid, :array) 
            
                if @grid.size >= 2
                
                    # if we have more than 4 params, ignore them
                    "chg=" + @grid[0, 4].join(",")
                end
            end
        end
        
        
        # process zero line
        # single value: 5
        # array form: [1, 2, ..]
        def process_zero_line
        
            if check_type(@zero_line, :array)
            
                "chp=" + @zero_line.join(",")
            else
            
                "chp=#{@zero_line}"
            end
        end
        
        
        # process bar width and spacing
        # single number: 20
        # array form: [20, 5, 6]
        # hash form: {:width => 20, :bars => 5, :groups => 10}
        def process_width_spacing
        
            if check_type(@width_spacing, :hash)
            
                if @width_spacing.has_key? :width
                
                    arr_params = []
                    
                    # process width
                    arr_params << @width_spacing[:width]
                    
                    # process bars
                    if @width_spacing.has_key? :bars
                        
                        arr_params << @width_spacing[:bars]
                    end
                    
                    # process groups
                    if @width_spacing.has_key? :groups
                    
                        arr_params << @width_spacing[:groups]
                    end
                    
                    "chbh=" + arr_params.join(",")
                end
                
            elsif check_type(@width_spacing, :array)
            
                "chbh=" + @width_spacing[0, 3].join(",")
            else
            
                "chbh=#{@width_spacing}"
            end
        end
        
        
        # helper used to process markers
        def process_markers_helper(marker_hash)
        	
            # type, set, points are mandatory
            if ((marker_hash.has_key? :type) && (marker_hash.has_key? :points) && (marker_hash.has_key? :set))
            
            	# find what type of marker it is
                str_type = case marker_hash[:type]
                
                            when :plus
                        
                                "c"
                            when :diamond
                    
                                "d"
                            when :arrow
                    
                                "a"
                            when :circle
                    
                                "o"
                            when :square
                    
                                "s"
                            when :projection
                    
                                "v"
                            when :vertical_line
                    
                                "V"
                            when :horizontal_line
                    
                                "h"
                            when :cross
                    
                                "x"
                            when :vertical_range
                    
                                "R"
                            when :horizontal_range
                    
                                "r"
                            else
                    
                                "o"
                        end
                        
                # process color
                #str_color = retrieve_value(marker_hash[:color], "000000")
                str_color = retrieve_value(marker_hash[:color], nil)
                
                # no color supplied
                if str_color.nil?
                
                	# check if color has been supplied
                	unless (@colors.nil? && @colors[marker_hash[:set]].nil?)
                	
                		# color is present - use that
                		str_color = @colors[marker_hash[:set]]
                	else
                	
                		# use default black
                		str_color = "000000"
                	end
                end
                
                # process set
                str_set = marker_hash[:set]
                
                # process size
                str_size = retrieve_value(marker_hash[:size], "5.0")
                
                # process priority
                #str_priority = retrieve_value(marker_hash[:priority], "")
                #str_priority = "0"
                    
                # process points
                points = marker_hash[:points]
                
                # generated markers
                arr_markers = []
                
                if check_type(points, :array)
                
                    points.each do |point|
                    
                        arr_markers << "#{str_type},#{str_color},#{str_set},#{point},#{str_size}"
                    end
                    
                elsif points == :all
                
                    # for every point
                    
                    if multi_dimensional? @data
                                                                        
                        # retrieve the set
                        arr_row = @data[str_set.to_i]
                        
                        unless arr_row.nil?
                        
                            arr_row.each_index do |element_index|
                            	
                                arr_markers << "#{str_type},#{str_color},#{str_set},#{element_index},#{str_size}"
                            end
                        end
                    else
            
                        @data.each_index do |element_index|
                        
                            arr_markers << "#{str_type},#{str_color},0,#{element_index},#{str_size}"
                        end
                    end
                else
                
                    arr_markers << "#{str_type},#{str_color},#{str_set},#{points},#{str_size}"
                end
                    
                # return produced markers
                return arr_markers
            end
        end
        
        
        # process markers
        # array form: [{...}, {....}]
        # hash form: {:type => :plus, :color => 'FF00FF', :set => 1, 
        #               :points => 1 or [1,2,3] or :all, :size => 20, :priority => -1, 0, 1} 
        # types: plus, diamond, arrow, circle, square, projection
        # vertical_line, horizontal_line, cross, vertical_range, horizontal_range
        def process_markers
        
            if check_type(@markers, :hash)
            
                arr_markers = process_markers_helper(@markers)
                
                if arr_markers
                
                    "chm=" + arr_markers.join("|")
                end
                
            elsif check_type(@markers, :array)
            
                arr_markers = []
                
                @markers.each do |marker|
                
                    arr_row = process_markers_helper(marker)
                    
                    unless arr_row
                    
                        return nil
                    end
                    
                    arr_markers += arr_row
                end
                
                "chm=" + arr_markers.join("|")
            end
        end
            
    end
end