package org.wesnoth.wml.schema.impl;

import org.wesnoth.wml.schema.SchemaTag;
import org.wesnoth.wml.schema.SchemaTagChild;

public class SchemaTagChildImpl implements SchemaTagChild {
	SchemaTag child;
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

}
