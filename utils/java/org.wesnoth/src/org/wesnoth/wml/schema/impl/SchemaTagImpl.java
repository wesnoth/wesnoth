package org.wesnoth.wml.schema.impl;

import org.wesnoth.wml.schema.SchemaAttributeChild;
import org.wesnoth.wml.schema.SchemaTag;
import org.wesnoth.wml.schema.SchemaTagChild;

import java.util.ArrayList;
import java.util.List;

public class SchemaTagImpl implements SchemaTag
{
	private List<SchemaAttributeChild>	attributes	= new ArrayList<SchemaAttributeChild>();
	private List<SchemaTagChild>		tags		= new ArrayList<SchemaTagChild>();
	private String						name;
	private String						description;

	@Override
	public List<SchemaAttributeChild> getAttributes()
	{
		return this.attributes;
	}

	public void setAttributes(List<SchemaAttributeChild> attributes)
	{
		this.attributes = attributes;
	}

	@Override
	public List<SchemaTagChild> getTags()
	{
		return this.tags;
	}

	public void setTags(List<SchemaTagChild> tags)
	{
		this.tags = tags;
	}

	@Override
	public String getName()
	{
		return this.name;
	}

	public void setName(String name)
	{
		this.name = name;
	}

	@Override
	public String getDescription()
	{
		return this.description;
	}

	public void setDescription(String description)
	{
		this.description = description;
	}

	@Override
	public boolean isTag()
	{
		return true;
	}

	public void addAttribute(SchemaAttributeChild attribute)
	{
		this.attributes.add(attribute);
	}

	public void addTag(SchemaTagChild tag)
	{
		this.tags.add(tag);
	}

}
