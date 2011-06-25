package org.wesnoth.wml.core;

import java.util.LinkedHashMap;
import java.util.Map;

import org.wesnoth.Messages;
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
		scenario.setName("scenario"); //$NON-NLS-1$
		scenario.setDescription(Messages.SchemaFactory_1);
		schemaTagsMap.put("scenario", scenario); //$NON-NLS-1$

		SchemaTagImpl campaign = new SchemaTagImpl();
		campaign.setName("campaign"); //$NON-NLS-1$
		campaign.setDescription(Messages.SchemaFactory_4);

		SchemaAttributeImpl string = new SchemaAttributeImpl("string"); //$NON-NLS-1$
		schemaAttributesMap.put("string", string); //$NON-NLS-1$
		SchemaAttributeImpl tstring = new SchemaAttributeImpl("tstring"); //$NON-NLS-1$
		schemaAttributesMap.put("tstring", tstring); //$NON-NLS-1$
		SchemaAttributeImpl identifier = new SchemaAttributeImpl("identifier"); //$NON-NLS-1$
		schemaAttributesMap.put("identifier", identifier); //$NON-NLS-1$
		SchemaAttributeImpl path = new SchemaAttributeImpl("path"); //$NON-NLS-1$
		schemaAttributesMap.put("path", path); //$NON-NLS-1$
		SchemaAttributeImpl integer = new SchemaAttributeImpl("integer"); //$NON-NLS-1$
		schemaAttributesMap.put("integer", integer); //$NON-NLS-1$

		campaign.addAttribute(new SchemaAttributeChildImpl("abbrev",identifier,SchemaChild.REQUIRED)); //$NON-NLS-1$

		campaign.addAttribute(new SchemaAttributeChildImpl("define",identifier,SchemaChild.REQUIRED)); //$NON-NLS-1$

		campaign.addAttribute(new SchemaAttributeChildImpl("description",tstring,SchemaChild.REQUIRED)); //$NON-NLS-1$

		campaign.addAttribute(new SchemaAttributeChildImpl("difficulties",string,SchemaChild.REQUIRED)); //$NON-NLS-1$

		campaign.addAttribute(new SchemaAttributeChildImpl("difficulty_descriptions",string,SchemaChild.REQUIRED)); //$NON-NLS-1$

		campaign.addAttribute(new SchemaAttributeChildImpl("extra_defines",string,SchemaChild.OPTIONAL)); //$NON-NLS-1$

		campaign.addAttribute(new SchemaAttributeChildImpl("first_scenario",identifier,SchemaChild.REQUIRED)); //$NON-NLS-1$

		campaign.addAttribute(new SchemaAttributeChildImpl("icon",path,SchemaChild.OPTIONAL)); //$NON-NLS-1$

		campaign.addAttribute(new SchemaAttributeChildImpl("id",string,SchemaChild.REQUIRED)); //$NON-NLS-1$

		campaign.addAttribute(new SchemaAttributeChildImpl("image",path,SchemaChild.OPTIONAL)); //$NON-NLS-1$

		campaign.addAttribute(new SchemaAttributeChildImpl("name",tstring,SchemaChild.REQUIRED)); //$NON-NLS-1$

		campaign.addAttribute(new SchemaAttributeChildImpl("rank",integer,SchemaChild.REQUIRED)); //$NON-NLS-1$


		schemaTagsMap.put("campaign", campaign); //$NON-NLS-1$
	}

}
