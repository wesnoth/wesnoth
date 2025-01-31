---@meta

---@class location
---@field x integer
---@field y integer
---@class loc_list
---@field [1] integer
---@field [2] integer

---@class location_triple : location
---@field value any

---@alias direction
---| "'n'"
---| "'nw'"
---| "'ne'"
---| "'s'"
---| "'sw'"
---| "'se'"

---@class wesnoth.map
wesnoth.map = {}

---@class terrain_map : wesnoth.map, {[location|loc_list]: string}
---@field width integer
---@field height integer
---@field playable_width integer
---@field playable_height integer
---@field border_size integer
---@field special_locations table<integer|string, location>
---@field data string

---@class terrain_filter_tag
---@class terrain_filter

---Converts a terrain code string to a special value that, when assigned to a hex on the map,
--- will first attempt to replace just the base or just the overlay, and if that produces an
--- invalid combination, will instead replace both.
---@param terrain string
---@param mode 'base'|'overlay'|'both'
---@return string
function wesnoth.map.replace_if_failed(terrain, mode) end

---Get a list of hexes matching a filter
---@param map terrain_map
---@param filter terrain_filter
---@param in_list location[]
---@return location[]
---@overload fun(map:terrain_map, filter:terrain_filter):location[]
---@overload fun(map:terrain_map, filter:terrain_filter, at_loc:location):location[]
---@overload fun(map:terrain_map, filter:terrain_filter, at_x:integer, at_y:integer):location[]
---@overload fun(map:terrain_map, filter:terrain_filter_tag):location[]
---@overload fun(map:terrain_map, filter:terrain_filter_tag, in_list:location[]):location[]
---@overload fun(map:terrain_map, filter:terrain_filter_tag, at_loc:location):location[]
---@overload fun(map:terrain_map, filter:terrain_filter_tag, at_x:integer, at_y:integer):location[]
function wesnoth.map.find(map, filter, in_list) end

---Get a list of hexes matching a filter within a given circles
---@param map terrain_map
---@param center location
---@param radius integer
---@param filter terrain_filter
---@return location[]
---@overload fun(map:terrain_map, centers:location[], radius:integer, filter:terrain_filter):location[]
---@overload fun(map:terrain_map, center_x:integer, center_y:integer, radius:integer, filter:terrain_filter):location[]
---@overload fun(map:terrain_map, center:location, radius:integer, filter:terrain_filter_tag):location[]
---@overload fun(map:terrain_map, centers:location[], radius:integer, filter:terrain_filter_tag):location[]
---@overload fun(map:terrain_map, center_x:integer, center_y:integer, radius:integer, filter:terrain_filter_tag):location[]
function wesnoth.map.find_in_radius(map, center, radius, filter) end

---Parse a mapgen location filter
---@param filter terrain_filter_tag
---@param data? table<string, location[]>
---@return terrain_filter
function wesnoth.map.filter(filter, data) end

---Iterate over the entire map.
---@param map terrain_map
---@param include_border boolean? Whether to include border hexes.
---@return fun():integer,integer
function wesnoth.map.iter(map, include_border) end

---Test if a location matches a filter
---@param location location
---@param filter WML
---@param ref_unit? unit
---@return boolean
---@overload fun(x:integer, y:integer, filter:WML, ref_unit?:unit):boolean
---@overload fun(map:terrain_map, location:location, filter:terrain_filter):boolean
---@overload fun(map:terrain_map, x:integer, y:integer, filter:terrain_filter):boolean
function wesnoth.map.matches(location, filter, ref_unit) end

---Test if a location is on the map
---@param map terrain_map
---@param location location
---@param include_border? boolean
---@return boolean
---@overload fun(map:terrain_map, x:integer, y:integer, include_border?:boolean):boolean
function wesnoth.map.on_board(map, location, include_border) end

---Test if a location is on the map border
---@param map terrain_map
---@param location location
---@return boolean
---@overload fun(map:terrain_map, x:integer, y:integer):boolean
function wesnoth.map.on_border(map, location) end

---Convert a list of locations into a two-dimensional bitmap
---(also known as a shroud data string)
---@param locs location[]
---@return string
function wesnoth.map.make_bitmap(locs) end

---Parse a two-dimensional bitmap (a shroud data string) into a list of locations
---@param data string
---@return location[]
function wesnoth.map.parse_bitmap(data) end

---@alias terrain_layer
---| "'overlay'"
---| "'base'"
---| "'both'"
---@class terrain_mask_rule
---@field old string
---@field new string
---@field terrain string
---@field layer terrain_layer
---@field replace_if_failed boolean
---@field use_old boolean

---@class terrain_mask_options
---@field is_odd? boolean
---@field ignore_special_location? boolean
---@field rules? terrain_mask_rule[]

---Overlays a terrain mask onto a map
---@param map terrain_map
---@param pivot location
---@param mask string
---@param options terrain_mask_options
---@overload fun(map:terrain_map, pivot_x:integer, pivot_y:integer, mask:string, options:terrain_mask_options)
function wesnoth.map.terrain_mask(map, pivot, mask, options) end

---@class label_info : location
---@field text tstring
---@field team_name? string
---@field color? color|integer[]
---@field visible_in_fog? boolean
---@field visible_in_shroud? boolean
---@field immutable? boolean
---@field category? string|tstring
---@field tooltip? tstring
---@field side? integer

---Place a label on the map
---@param label_info label_info
function wesnoth.map.add_label(label_info) end

---Removes any label from the map on the given hex
---@param location location
function wesnoth.map.remove_label(location) end

---Get the label on the given hex, if any
---@param location location
---@param side integer
---@return label_info?
---@overload fun(x:integer, y:integer, side:integer):label_info?
function wesnoth.map.get_label(location, side) end

---Place a new time area on the map
---@param id string
---@param filter WML
---@param schedule WML
function wesnoth.map.place_area(id, filter, schedule) end

---Remove a time area from the map
---@param id string
function wesnoth.map.remove_area(id) end

---Get information about an existing time area
---@param area string|location
---@return time_area
---@overload fun(x:integer, y:integer):time_area
function wesnoth.map.get_area(area) end

---Set the owner of a village hex
---@param loc location
---@param side integer
---@overload fun(x:integer, y:integer, side:integer)
function wesnoth.map.set_owner(loc, side) end

---Get the owner of a village hex
---@param loc location
---@return integer
---@overload fun(x:integer, y:integer):integer
function wesnoth.map.get_owner(loc) end

---Create a new game map
---@param width integer
---@param height integer
---@param terrain string
---@return terrain_map
function wesnoth.map.create(width, height, terrain) end
---@param data string
---@return terrain_map
function wesnoth.map.create(data) end

---@class mapgen_options
---@field nplayers integer
---@field nvillages integer
---@field iterations integer
---@field hill_size integer
---@field castle_size integer
---@field island_size integer
---@field island_off_center integer
---@field max_lakes integer
---@field link_castles boolean
---@field seed integer
---@class heightgen_options
---@field iterations integer
---@field hill_size integer
---@field island_size integer
---@field center_x integer
---@field center_y integer
---@field flip_format boolean
---@field seed integer

---Generate a map using the default map generator
---@param width integer
---@param height integer
---@param options mapgen_options
---@return terrain_map
function wesnoth.map.generate(width, height, options) end

---Generate a height map using the default map generator
---@param width integer
---@param height integer
---@param options heightgen_options
---@return table<integer, integer>
function wesnoth.map.generate_height_map(width, height, options) end

---Get the hex reached by travelling in the specified direction
---@param from location
---@param dir direction
---@param steps? integer
---@return location
---@overload fun(from_x:integer, from_y:integer, dir:direction):location
---@overload fun(from_x:integer, from_y:integer, dir:direction, steps:integer):location
function wesnoth.map.get_direction(from, dir, steps) end

---Get the direction you need to travel to get from one hex to another
---@param from location
---@param to location
---@return direction
---@overload fun(from:location, to_x:integer, to_y:integer):direction
---@overload fun(from_x:integer, from_y:integer, to:location):direction
---@overload fun(from_x:integer, from_y:integer, to_x:integer, to_y:integer):direction
function wesnoth.map.get_relative_dir(from, to) end

---Get the hex obtained by rotating a location around the specified center
---@param loc location
---@param center location
---@param angle integer An angle index from 1 to 6, where 6 represents 360 degrees
---@return location
function wesnoth.map.rotate_right_around_center(loc, center, angle) end

---Get a list of all potential adjacent hexes, including off-map locations
---@param loc location
---@return location n, location ne, location se, location s, location sw, location nw
---@overload fun(x:integer, y:integer):location,location,location,location,location,location
function wesnoth.map.get_adjacent_hexes(loc) end

---Get a list of all potential hexes within a given radius, including off-map locations
---@param center location
---@param radius integer
---@return location[]
function wesnoth.map.get_hexes_in_radius(center, radius) end

---Test if two hexes are adjacent
---@param loc1 location
---@param loc2 location
---@return boolean
function wesnoth.map.are_hexes_adjacent(loc1, loc2) end

---Calculate the distance between two hexes
---@param loc1 location
---@param loc2 location
---@return integer
---@overload fun(x1:integer, y1:integer, loc2:location):integer
---@overload fun(loc1:location, x2:integer, y2:integer):integer
---@overload fun(x1:integer, y1:integer, x2:integer, y2:integer):integer
function wesnoth.map.distance_between(loc1, loc2) end

---Represents a map location expressed in cubic coordinates.
---Each axis covers a "diagonal" direction on the hexagonal grid.
---To put it another way, it's the directions corresponding to
--- the hex's vertices, rather than the edges.
---@class location_cube
---@field q integer Horizontal (e/w) component
---@field r integer Positive diagonal (nne/ssw) component
---@field s integer Negative diagonal (nnw/sse) component

---Convert a hex location into cubic coordinates
---@param loc location
---@return location_cube
---@overload fun(x:integer, y:integer):location_cube
function wesnoth.map.get_cubic(loc) end

---Convert a cubic hex location back into standard hex coordinates
---@param vec location_cube
---@return location
function wesnoth.map.from_cubic(vec) end

---Add two hex vectors together
---@param v1 location
---@param v2 location
---@return location
---@overload fun(x1:integer, y1:integer, x2:integer, y2:integer):location
---@overload fun(loc1:location, x2:integer, y2:integer):location
---@overload fun(x1: integer, y1:integer, loc2:location):location
function wesnoth.map.hex_vector_sum(v1, v2) end

---Take the difference of two hex vectors
---@param v1 location
---@param v2 location
---@return location
---@overload fun(x1:integer, y1:integer, x2:integer, y2:integer):location
---@overload fun(loc1:location, x2:integer, y2:integer):location
---@overload fun(x1: integer, y1:integer, loc2:location):location
function wesnoth.map.hex_vector_diff(v1, v2) end

---Reverse the direction of a hex vector
---@param vec location
---@return location
---@overload fun(x:integer, y:integer):location
function wesnoth.map.hex_vector_negation(vec) end
