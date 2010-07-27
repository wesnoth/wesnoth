/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.schema;

import java.util.ArrayList;
import java.util.Comparator;
import java.util.List;

/**
 * This represents a wml tag parsed by the schema.
 */
public class Tag
{
	private String		Name = "";
	private String		ExtendedTagName = "";
	private List<Tag>	TagChildren;
	private List<TagKey>	KeyChildren;
	private Tag          Description;
	private char		    Cardinality = ' ';

	private boolean		NeedsExpanding = false;

	public Tag(String name, List<Tag> tagChildren, List<TagKey> keyChildren, char cardinality) {
		Name = name;
		TagChildren = tagChildren;
		KeyChildren = keyChildren;
		Cardinality = cardinality;
	}

	public Tag(String name, char cardinality) {
		Name = name;
		Cardinality = cardinality;
	}

	public Tag(String name, String extendedTagName, char cardinality) {
		Name = name;
		ExtendedTagName = extendedTagName;
		Cardinality = cardinality;
	}

	/**
	 * Ads a child tag
	 * @param tag the Tag to add
	 */
	public void addTag(Tag tag)
	{
		if (TagChildren == null)
			TagChildren = new ArrayList<Tag>();
		TagChildren.add(tag);
	}

	/**
	 * Adds a child key
	 * @param key the TagKey to add
	 */
	public void addKey(TagKey key)
	{
		if (KeyChildren == null)
			KeyChildren = new ArrayList<TagKey>();
		KeyChildren.add(key);
	}

	/**
	 * Adds a child key
	 * @param name the name of the key
	 * @param valueType the valuetype of the key
	 * @param cardinality the cardinality
	 * @param translate true if it's translatable
	 */
	public void addKey(String name, String valueType, char cardinality,
			boolean translatable)
	{
		addKey(new TagKey(name, cardinality, valueType, translatable));
	}

	@Override
	public String toString()
	{
		return new String(Name + " " + ExtendedTagName);
	}

	public String getName()
	{
		return Name;
	}

	public char getCardinality()
	{
		return Cardinality;
	}

	public void setDescription(Tag description)
	{
		Description = description;
	}
	public Tag getDescription()
	{
		return Description;
	}

	public String getExtendedTagName()
	{
		return ExtendedTagName;
	}
	public void setExtendedTagName(String extendedTagName)
	{
		ExtendedTagName = extendedTagName;
	}

	public void setNeedsExpanding(boolean needsExpanding)
	{
		NeedsExpanding = needsExpanding;
	}
	public boolean getNeedsExpanding()
	{
		return NeedsExpanding;
	}

	public List<TagKey> getKeyChildren()
	{
		if (KeyChildren == null)
			KeyChildren = new ArrayList<TagKey>();
		return KeyChildren;
	}

	public List<Tag> getTagChildren()
	{
		if (TagChildren == null)
			TagChildren = new ArrayList<Tag>();
		return TagChildren;
	}


	/**
	 * A tag comparator that sorts just after required cardinality.
	 */
	public static class CardinalityComparator implements Comparator<Tag>
	{
		@Override
		public int compare(Tag o1, Tag o2)
		{
			if (o1.Cardinality == o2.Cardinality)
				return 0;
			if (o1.Cardinality == '1')
				return 1;
			else if (o2.Cardinality == '1')
				return -1;
			return 0;
		}
	}

}
