package org.wesnoth.wml.schema.impl;

import java.util.ArrayList;
import java.util.List;

import org.wesnoth.wml.schema.SchemaAttributeChild;
import org.wesnoth.wml.schema.SchemaTag;
import org.wesnoth.wml.schema.SchemaTagChild;

public class SchemaTagImpl implements SchemaTag {
	private List<SchemaAttributeChild> attributes = new ArrayList<SchemaAttributeChild>();
	private List<SchemaTagChild> tags = new ArrayList<SchemaTagChild>();
	private String name;
	private String description;

	public List<SchemaAttributeChild> getAttributes() {
		return this.attributes;
	}
	public void setAttributes(List<SchemaAttributeChild> attributes) {
		this.attributes = attributes;
	}
	public List<SchemaTagChild> getTags() {
		return this.tags;
	}
	public void setTags(List<SchemaTagChild> tags) {
		this.tags = tags;
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
		return true;
	}

	public void addAttribute(SchemaAttributeChild attribute) {
		this.attributes.add(attribute);
	}

	public void addTag(SchemaTagChild tag) {
		this.tags.add(tag);
	}

}
