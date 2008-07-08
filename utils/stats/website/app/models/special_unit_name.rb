class SpecialUnitName < ActiveRecord::Base

    # there are multiple special units for each special_unit_name
    has_many :special_units
end
