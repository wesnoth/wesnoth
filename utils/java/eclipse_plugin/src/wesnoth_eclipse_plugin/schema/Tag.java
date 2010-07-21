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
	public String		Name;
	public String		ExtendedTagName;
	public List<Tag>	TagChildren;
	public List<TagKey>	KeyChildren;
	public char		Cardinality;

	public boolean		NeedsExpanding;

	public Tag(String name, List<Tag> tagChildren, List<TagKey> keyChildren, char cardinality) {
		Name = name;
		ExtendedTagName = "";
		TagChildren = tagChildren;
		KeyChildren = keyChildren;
		Cardinality = cardinality;
		safeInit();
	}

	public Tag(String name, char cardinality) {
		Name = name;
		ExtendedTagName = "";
		TagChildren = new ArrayList<Tag>();
		KeyChildren = new ArrayList<TagKey>();
		Cardinality = cardinality;
		safeInit();
	}

	public Tag(String name, String extendedTagName, char cardinality) {
		Name = name;
		ExtendedTagName = extendedTagName;
		TagChildren = new ArrayList<Tag>();
		KeyChildren = new ArrayList<TagKey>();
		Cardinality = cardinality;
		safeInit();
	}

	private void safeInit()
	{
		if (TagChildren == null)
			TagChildren = new ArrayList<Tag>();
		if (KeyChildren == null)
			KeyChildren = new ArrayList<TagKey>();
	}

	/**
	 * Ads a child tag
	 * @param tag the Tag to add
	 */
	public void addTag(Tag tag)
	{
		TagChildren.add(tag);
	}

	/**
	 * Adds a child key
	 * @param key the TagKey to add
	 */
	public void addKey(TagKey key)
	{
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
