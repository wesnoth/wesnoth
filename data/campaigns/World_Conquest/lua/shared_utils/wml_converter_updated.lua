-- <<
-- scema based wml <-> lua table (lon) converter
print("wml_converter_updated loaded")
local schema = {}
local converter = {}
local function split_to_array(s, res)
	res = res or {}
	for part in tostring(s or ""):gmatch("[^%s,][^,]*") do
		table.insert(res, part)
	end
	return res
end

schema.lua =  {
	tags = {
		args = {
			type = "table" ,
			type2 = "preserve_order",
		}
	}
}

schema.time_area =  {
	tags = {
		time = {
			type = "list",
		}
	}
}

schema.scenario =  {
	tags = {
		music = {
			type = "list", 
		},
		label = {
			type = "list", 
		},
		load_resource = {
			type = "list", 
		},
		event = {
			type = "list",
			id = "event",
		},
		lua = {
			type = "list",
			id = "lua",
		},
		side = {
			type = "list",
			id = "side",
		},
		time = {
			type = "list",
			id = "time",
		},
		variables = {
			type = "single",
		},
		options = {
			type = "single",
		},
	}
}

schema.side =  {
	tags = {
		village = {
			type = "list", 
		},
		unit = {
			type = "list",
			id = "unit",
		},
		leader = {
			type = "list", 
			id = "unit",
		},
		variables = {
			type = "single",
		},
	}
}

schema.time =  {
	tags = {
	}
}

schema.mg_main =  {
	tags = {
		height = {
			type = "list", 
		},
		convert = {
			type = "list",
		},
		road_cost = {
			type = "list", 
		},
		village = {
			type = "list", 
		},
		castle = {
			type = "single",
		},
	}
}

schema.wct_enemy_group_recall =  {
	attributes = {
		level2 = "comma_list",
		level3 = "comma_list",
	}
}
schema.wct_enemy_group_commander =  {
	attributes = {
		level1 = "comma_list",
		level2 = "comma_list",
		level3 = "comma_list",
	}
}
schema.wct_enemy_group_leader =  {
	attributes = {
		recruit = "comma_list",
	}
}

schema.wct_enemy_group =  {
	tags = {
		recall = {
			type = "single",
			id = "wct_enemy_group_recall",
		},
		commander = {
			type = "single",
			id = "wct_enemy_group_commander",
		},
		leader = {
			type = "list", 
			id = "wct_enemy_group_leader",
		},
	},
	attributes = {
		recruit = "comma_list",
		allies_available = "comma_list",
	}
}

schema.wct_enemy =  {
	tags = {
		group = {
			type = "list", 
			id = "wct_enemy_group",
		},
	},
	attributes = {
		factions_available = "comma_list",
	}
}

schema.wct_artifact =  {
	tags = {
		animate_unit = {
			type = "list", 
		},
		effect = {
			type = "list",
			--id = "effect",
		},
		trait = {
			type = "list",
		},
		filter = {
			type = "single",
			--id = "standard_unit_filter",
		},
	},
	attributes = {
	}
}
schema.wct_artifact_list =  {
	tags = {
		artifact = {
			type = "list",
			id = "wct_artifact"
		},
	},
	attributes = {
	}
}


-- i cannot do this because the code in training.lua does 
-- variable subsutution on [chance] which only works on configs.
-- schema.wct_trainer_chance =  {
-- }

schema.wct_trainer_grade =  {
	tags = {
		chance = {
			type = "list",
			id = "wct_trainer_chance"
		},
	},
}

schema.wct_trainer =  {
	tags = {
		grade = {
			type = "list",
			id = "wct_trainer_grade"
		},
	},
	attributes = {
	}
}

schema.wct_trainer_list =  {
	tags = {
		trainer = {
			type = "list",
			id = "wct_trainer"
		},
	},
	attributes = {
	}
}

schema.__attributes = {}
schema.__attributes.comma_list = 
{
	to_lon = function(attr)
		return split_to_array(attr)
	end,
	to_wml = function(val)
		return table.concat(val, ",")
	end,
}
function converter.wml_to_lon(cfg, name)
	local tag_info = schema[name or "aasdfasdf"]
	if tag_info == nil or tag_info.type == "preserve_order" then
		return cfg
	end
	local attrs = tag_info.attributes or {}
	local tags = tag_info.tags or {}
	
	local res = {}
	for name2, info2 in pairs(tags) do
		if info2.type == "single" then
			res[name2] = converter.wml_to_lon(wml.get_child(cfg, name2), info2.id)
		elseif info2.type == "list" then
			local list = {}
			res[name2] = list
			for tag in wml.child_range(cfg, name2) do
				list[#list + 1] = converter.wml_to_lon(tag, info2.id)
			end
		end
	end
	
	for k,v in pairs(cfg) do
		if type(k) == "number" then
		else --string
			local conv = attrs[k] and schema.__attributes[attrs[k]]
			if conv then 
				res[k] = conv.to_lon(v)
			else
				res[k] = v
			end
		end
	end
	return res
end

function converter.lon_to_wml(t, name)
	local tag_info = schema[name]
	if tag_info == nil then
		return t
	end
	local attrs = tag_info.attributes or {}
	local tags = tag_info.tags or {}
	
	local res = {}
	for name2, info2 in pairs(tags) do
		if info2.type == "single" then
			local st = t[name2]
			if st ~= nil then
				res[#res + 1] = { name2, converter.lon_to_wml(st, info2.id) }
			end
		elseif info2.type == "list" then
			for i, v in ipairs(t[name2] or {}) do
				res[#res + 1] = { name2, converter.lon_to_wml(v, info2.id) }
			end
		end
	end
	
	for k,v in pairs(t) do
		if not tags[k] then
			local conv = attrs[k] and schema.__attributes[attrs[k]]
			if conv then 
				res[k] = conv.to_wml(v)
			else
				res[k] = v
			end
		end
	end
	return res
end
return converter
-->>
