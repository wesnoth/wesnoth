package org.wesnoth.wml.schema.impl;

import org.wesnoth.wml.schema.SchemaTag;
import org.wesnoth.wml.schema.SchemaTagChild;

public class SchemaTagChildImpl implements SchemaTagChild {
	SchemaTag child;
	String name;
	boolean repeated;
	boolean required;

	@Override
	public SchemaTag getChild() {
		return this.child;
	}

	@Override
	public boolean isRepeated() {
		return this.repeated;
	}

	@Override
	public boolean isRequired() {
		return this.required;
	}

	public void setChild(SchemaTag child) {
		this.child = child;
	}

	public void setRepeated(boolean repeated) {
		this.repeated = repeated;
	}

	public void setRequired(boolean required) {
		this.required = required;
	}

	public String getName() {
		return this.name;
	}

	public void setName(String name) {
		this.name = name;
	}

}
