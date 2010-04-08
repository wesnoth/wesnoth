package org.wesnoth.wml.core;

import java.util.LinkedHashMap;
import java.util.Map;

import org.wesnoth.wml.schema.SchemaAttribute;
import org.wesnoth.wml.schema.SchemaChild;
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
		schemaAttributesMap = new LinkedHashMap<String,SchemaAttribute>();
		schemaTagsMap = new LinkedHashMap<String,SchemaTag>();
		SchemaTagImpl scenario = new SchemaTagImpl();
		scenario.setName("scenario");
		scenario.setDescription("this is a scenario tag");
		schemaTagsMap.put("scenario", scenario);

		SchemaTagImpl campaign = new SchemaTagImpl();
		campaign.setName("campaign");
		campaign.setDescription("this is a campaign tag");

		SchemaAttributeImpl string = new SchemaAttributeImpl("string");
		schemaAttributesMap.put("string", string);
		SchemaAttributeImpl tstring = new SchemaAttributeImpl("tstring");
		schemaAttributesMap.put("tstring", tstring);
		SchemaAttributeImpl identifier = new SchemaAttributeImpl("identifier");
		schemaAttributesMap.put("identifier", identifier);
		SchemaAttributeImpl path = new SchemaAttributeImpl("path");
		schemaAttributesMap.put("path", path);
		SchemaAttributeImpl integer = new SchemaAttributeImpl("integer");
		schemaAttributesMap.put("integer", integer);

		campaign.addAttribute(new SchemaAttributeChildImpl("abbrev",identifier,SchemaChild.REQUIRED));

		campaign.addAttribute(new SchemaAttributeChildImpl("define",identifier,SchemaChild.REQUIRED));

		campaign.addAttribute(new SchemaAttributeChildImpl("description",tstring,SchemaChild.REQUIRED));

		campaign.addAttribute(new SchemaAttributeChildImpl("difficulties",string,SchemaChild.REQUIRED));

		campaign.addAttribute(new SchemaAttributeChildImpl("difficulty_descriptions",string,SchemaChild.REQUIRED));

		campaign.addAttribute(new SchemaAttributeChildImpl("extra_defines",string,SchemaChild.OPTIONAL));

		campaign.addAttribute(new SchemaAttributeChildImpl("first_scenario",identifier,SchemaChild.REQUIRED));

		campaign.addAttribute(new SchemaAttributeChildImpl("icon",path,SchemaChild.OPTIONAL));

		campaign.addAttribute(new SchemaAttributeChildImpl("id",string,SchemaChild.REQUIRED));

		campaign.addAttribute(new SchemaAttributeChildImpl("image",path,SchemaChild.OPTIONAL));

		campaign.addAttribute(new SchemaAttributeChildImpl("name",tstring,SchemaChild.REQUIRED));

		campaign.addAttribute(new SchemaAttributeChildImpl("rank",integer,SchemaChild.REQUIRED));


		schemaTagsMap.put("campaign", campaign);
	}

}
