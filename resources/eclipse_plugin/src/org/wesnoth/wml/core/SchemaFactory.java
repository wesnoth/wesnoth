package org.wesnoth.wml.core;

import java.util.LinkedHashMap;
import java.util.Map;

import org.wesnoth.wml.schema.SchemaAttribute;
import org.wesnoth.wml.schema.SchemaTag;
import org.wesnoth.wml.schema.impl.SchemaAttributeChildImpl;
import org.wesnoth.wml.schema.impl.SchemaAttributeImpl;
import org.wesnoth.wml.schema.impl.SchemaTagImpl;

public class SchemaFactory {
	private static Map<String,SchemaTag> schemaTagsMap;
	private static Map<String,SchemaAttribute> schemaAttributesMap;

	public static SchemaTag getSchemaForTag(String name) {
		return schemaTagsMap.get(name);
	}
	public static SchemaAttribute getSchemaForAttribute(String name) {
		return schemaAttributesMap.get(name);
	}

	{
		//stub code
		schemaTagsMap = new LinkedHashMap<String,SchemaTag>();
		SchemaTagImpl scenario = new SchemaTagImpl();
		scenario.setName("scenario");
		scenario.setDescription("this is a scenario tag");
		schemaTagsMap.put("scenario", scenario);

		SchemaTagImpl campaign = new SchemaTagImpl();
		campaign.setName("campaign");
		campaign.setDescription("this is a campaign tag");

		SchemaAttributeImpl abbrev = new SchemaAttributeImpl("abbrev");
		campaign.addAttribute(new SchemaAttributeChildImpl(abbrev,true));
		SchemaAttributeImpl define = new SchemaAttributeImpl("define");
		campaign.addAttribute(new SchemaAttributeChildImpl(define,true));
		SchemaAttributeImpl description = new SchemaAttributeImpl("description");
		campaign.addAttribute(new SchemaAttributeChildImpl(description,true));
		SchemaAttributeImpl difficulties = new SchemaAttributeImpl("difficulties");
		campaign.addAttribute(new SchemaAttributeChildImpl(difficulties,true));
		SchemaAttributeImpl difficulty_descriptions = new SchemaAttributeImpl("difficulty_descriptions");
		campaign.addAttribute(new SchemaAttributeChildImpl(difficulty_descriptions,true));
		SchemaAttributeImpl extra_defines = new SchemaAttributeImpl("extra_defines");
		campaign.addAttribute(new SchemaAttributeChildImpl(extra_defines,false));
		SchemaAttributeImpl first_scenario = new SchemaAttributeImpl("first_scenario");
		campaign.addAttribute(new SchemaAttributeChildImpl(first_scenario,true));
		SchemaAttributeImpl icon = new SchemaAttributeImpl("icon");
		campaign.addAttribute(new SchemaAttributeChildImpl(icon,false));
		SchemaAttributeImpl id = new SchemaAttributeImpl("id");
		campaign.addAttribute(new SchemaAttributeChildImpl(id,true));
		SchemaAttributeImpl image = new SchemaAttributeImpl("image");
		campaign.addAttribute(new SchemaAttributeChildImpl(image,false));
		SchemaAttributeImpl rank = new SchemaAttributeImpl("rank");
		campaign.addAttribute(new SchemaAttributeChildImpl(rank,true));
		SchemaAttributeImpl name = new SchemaAttributeImpl("name");
		campaign.addAttribute(new SchemaAttributeChildImpl(name,true));
		schemaTagsMap.put("campaign", campaign);
	}

}
