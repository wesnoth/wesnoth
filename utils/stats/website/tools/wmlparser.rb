#!/usr/local/bin/ruby
#
# Mykola 'radx' Konyk, gamedev_AT_gmail_DOT_com (c) 2008
# Battle of Wesnoth, www.wesnoth.org
   

# Required classes
require 'wmlnode'


# Parse the log into the structured document
class WMLParser

    #--
    # Static method - parse the given log (array of lines)
    def self.parse(log)

        # Crate general root object, which has no parent
        objRoot = WMLNode.new("root", nil)
        objCurrent = objRoot

        # Iterate over the array
        log.each do |sLine|

            # Remove leading and trailing whitespaces
            sData = sLine.lstrip.rstrip


            # Check if we have a data/value pair
            if sData.include?("=")

                # Process the pair
                arrData = sData.split("=")

                # Process values
                sDataName = arrData[0]
                sDataValue = arrData[1].delete("\"")


                # Insert the pair
                objCurrent.add_data(sDataName, sDataValue)

            else

                # Delete tag braces
                sData = sData.delete("[").delete("]")

                # Check if this is a closing tag
                if sData.include?("/")

                    # We need to go back to the parent node
                    objCurrent = objCurrent.get_parent()

                else

                    # This is a new node
                    objTemp = WMLNode.new(sData, objCurrent)

                    # Add this node to the list of children
                    objCurrent << objTemp

                    # Switch to this new node
                    objCurrent = objTemp

                end

            end

        end


	return objRoot
    end


end

