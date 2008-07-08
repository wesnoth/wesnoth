#!/usr/local/bin/ruby
#
# Mykola 'radx' Konyk, gamedev_AT_gmail_DOT_com (c) 2008
# Battle of Wesnoth, www.wesnoth.org
        
               
# Node in the WML document
class WMLNode

	
    #--     
    # Constructor
    def initialize(name, parent)

        # Create new Hash map to hold pairs, default value for non-existing is nil
        @hashData = Hash.new(nil)
    
        # Create new Array to hold children nodes
        @arrNodes = Array.new
        
        # Store node name
        @sName = name

        # Store parent
        @objParent = parent
    
    end

        
    #attr_accessor :@hashData, :arrNodes, :sName, :objParent
       


    #--
    # Add pair (name, value)
    # 
    # For pair scenario="01_The_Elves_Besieged"
    # Will store "scenario" as a name and "01_The_Elves_Besieged" as a value
    def add_data(name, value)

       @hashData[name] = value

    end
       
    
    #--
    # Get value of the given name
    #
    # For pair scenario="01_The_Elves_Besieged"
    # Given "scenario" will return "01_The_Elves_Besieged"
    def get_value(name)

        return @hashData[name]
    
    end

        
    #--
    # Add given node as a child
    def << (objNode)

        @arrNodes << objNode
        
    end
    
    
    #--
    # Get the child node with the given name
    def get_child(name)  

        @arrNodes.each do |node|

            if node.get_name == name
                                                                                    
                return node

            end

        end

	return nil

    end



    #--
    # Return children array
    def get_children()

        return @arrNodes

    end


    #--
    # Return the parent node
    def get_parent()

        return @objParent

    end


    #--
    # Return the name
    def get_name()

        return @sName

    end


    #--
    # Checks if this node is empty - has no pairs and no children
    def is_empty()

       return (@arrNodes.empty? and @hashData.empty?)

    end


    #--
    # Debug - print the tree, starting at this node
    def debug_print(file, ident)

        # Calculate ident
        sIdent = " " * ident

        file.puts("#{sIdent}[#{@sName}]")

        # Print pairs
        file.puts("#{sIdent} |")
        file.puts("#{sIdent} + #{@hashData.length} pairs:")

        @hashData.each do |key, value|

            file.puts("#{sIdent} + name: #{key} -> value:#{value}")

            bIdent = true

        end
        
        # Print nodes - children
        file.puts("#{sIdent} |")
        file.puts("#{sIdent} + #{@arrNodes.length} children:")
        
        @arrNodes.each do |node|
       
            # Execute recursively
            node.debug_print(file, 4 + ident)

        end
    
    end

end

