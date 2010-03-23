package org.wesnoth.wml.schema.impl;

import org.wesnoth.wml.schema.SchemaAttribute;

public class SchemaAttributeImpl implements SchemaAttribute {

	private String name;
	private String description;
	private String valueExpression;

	public SchemaAttributeImpl(String name) {
		this.name = name;
	}

	public String getValueExpression() {
		return this.valueExpression;
	}

	public void setValueExpression(String valueExpression) {
		this.valueExpression = valueExpression;
	}

	public String getName() {
		return this.name;
	}

	public void setName(String name) {
		this.name = name;
	}

	public String getDescription() {
		return this.description;
	}

	public void setDescription(String description) {
		this.description = description;
	}

	@Override
	public boolean isTag() {
		return false;
	}

}
