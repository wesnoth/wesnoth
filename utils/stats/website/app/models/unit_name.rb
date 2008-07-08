class UnitName < ActiveRecord::Base

    # there are multiple units for each unit_name
    has_many :units
end
