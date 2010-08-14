package org.wesnoth.wml.schema;


public interface SchemaTagChild extends SchemaChild {
	SchemaTag getChild();
	boolean isRepeated();
}
