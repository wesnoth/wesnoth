package org.wesnoth.wml.schema.impl;

import org.wesnoth.wml.schema.SchemaAttribute;
import org.wesnoth.wml.schema.SchemaAttributeChild;

public class SchemaAttributeChildImpl implements SchemaAttributeChild
{

	SchemaAttribute	child;
	String			name;
	boolean			required;

	public SchemaAttributeChildImpl(String name, SchemaAttribute child, boolean required) {
		super();
		this.child = child;
		this.name = name;
		this.required = required;
	}

	@Override
	public SchemaAttribute getChild()
	{
		return this.child;
	}

	public void setChild(SchemaAttribute child)
	{
		this.child = child;
	}

	@Override
	public boolean isRequired()
	{
		return this.required;
	}

	public void setRequired(boolean required)
	{
		this.required = required;
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

}
