package org.wesnoth.wml.schema;

import java.util.List;

public interface SchemaTag extends SchemaElement{
	List<SchemaAttributeChild> getAttributes();
	List<SchemaTagChild> getTags();
}
