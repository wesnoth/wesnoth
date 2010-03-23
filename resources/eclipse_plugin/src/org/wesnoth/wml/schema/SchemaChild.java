package org.wesnoth.wml.schema;

public interface SchemaChild {
	public static boolean REQUIRED = true;
	public static boolean OPTIONAL = false;
	boolean isRequired();
	String getName();
}
