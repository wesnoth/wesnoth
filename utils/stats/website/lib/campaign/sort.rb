module CampaignLib

    # return minimum of two numbers (built-in min is confusing)
    def min(a, b)
        
        a <= b ? a : b
    end
    
    
    # return maximum of two numbers (built-in max is confusing)
    def self.max(a, b)
        
        a >= b ? a : b
    end
    
    
    # return the type of token
    def get_token_type(token)
        
        if token =~ /\d+[a-zA-Z]*/
        
            # (number followed by letters)
            :token_number
        else
        
            # everything else
            :token_alpha
        end
    end


    # helper comparison method (emulating <=> operator)
    def compare(left, right)
    
        # do smallest number of times
        min(left.size, right.size).times do |i|
        
            # get token types
            type_left = get_token_type(left[i].first)
            type_right = get_token_type(right[i].first)
            
            int_left = left[i].first.to_i
            int_right = right[i].first.to_i
            
            str_left = left[i].first.downcase
            str_right = right[i].first.downcase
            
            # check if types are equal
            if type_left == type_right
            
                # if both start with number
                if :token_number == type_left
                    
                    if int_left == int_right
                        
                        # compare strings
                        if str_left == str_right
                        
                            # if match, skip to next
                            next
                        end
                        
                        # not equal, return string based
                        return str_left <=> str_right
                    else
                    
                        # if not, return int comparison
                        return int_left <=> int_right
                    end
                else
                
                    # compare strings
                    if str_left == str_right
                    
                        # if match, skip to next
                        next
                    end
                    
                    #both are words, we compare strings
                    return str_left <=> str_right
                end
            else
            
                if :token_alpha == type_right
                
                    # no need swapping
                    return 1
                else
                
                    # need to swap
                    return -1
                end
            end
        end
        
        return 0
    
    end
    

    # sort scenarios (we'll try to sort based on number comparison and not string)
    def scenario_sort(arr)
    
        # destructively sort the collection
        arr.sort! do |left, right| 
        
            # tokenize left and right params
            arr_left = left.name.scan /([^_]+)/
            arr_right = right.name.scan /([^_]+)/
            
            # compare tokenized arrays
            
            # This will yield order like this:
            # "Drake_Council"
            # 1_Epilogue
            # 1_3_Out_of_the_Swamps
            # 1_4t_Berdssenhold
            # 2_2_South_Across_the_Plains
            
            compare(arr_left, arr_right)
        end
    end
    
end
