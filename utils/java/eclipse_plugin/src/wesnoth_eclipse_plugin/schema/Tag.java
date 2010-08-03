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
	private String name_ = "";
	private String extendedTagName_ = "";
	private List<Tag> tagChildren_;
	private List<TagKey> keyChildren_;

	private Tag description_;
	private char cardinality_ = ' ';

	private boolean needsExpanding_ = false;

	public Tag(String name, List<Tag> tagChildren, List<TagKey> keyChildren, char cardinality) {
		name_ = name;
		tagChildren_ = tagChildren;
		keyChildren_ = keyChildren;
		cardinality_ = cardinality;
	}

	public Tag(String name, char cardinality) {
		name_ = name;
		cardinality_ = cardinality;
	}

	public Tag(String name, String extendedTagName, char cardinality) {
		name_ = name;
		extendedTagName_ = extendedTagName;
		cardinality_ = cardinality;
	}

	/**
	 * Ads a child tag
	 * @param tag the Tag to add
	 */
	public void addTag(Tag tag)
	{
		if (tagChildren_ == null)
			tagChildren_ = new ArrayList<Tag>();
		tagChildren_.add(tag);
	}

	/**
	 * Adds a child key
	 * @param key the TagKey to add
	 */
	public void addKey(TagKey key)
	{
		if (keyChildren_ == null)
			keyChildren_ = new ArrayList<TagKey>();
		keyChildren_.add(key);
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
		return new String(name_ + " " + extendedTagName_);
	}

	public String getName()
	{
		return name_;
	}

	/**
	 * Gets tag cardinality. Possible values:
	 * required = 1
	 * optional = ?
	 * repeated = *
	 * forbidden = -
	 * @return
	 */
	public char getCardinality()
	{
		return cardinality_;
	}

	/**
	 * Returns true if this tag is required
	 * @return
	 */
	public boolean isRequired()
	{
		return cardinality_ == '1';
	}

	/**
	 * Returns true if this tag is forbidden to appear
	 * @return
	 */
	public boolean isForbidden()
	{
		return cardinality_ == '-';
	}

	/**
	 * Returns true if this tag is repeatable
	 * @return
	 */
	public boolean isRepeatable()
	{
		return cardinality_ == '*';
	}

	/**
	 * Returns true if this tag is optional
	 * @return
	 */
	public boolean isOptional()
	{
		return cardinality_ == '?';
	}

	public void setDescription(Tag description)
	{
		description_ = description;
	}
	public Tag getDescription()
	{
		return description_;
	}

	public String getExtendedTagName()
	{
		return extendedTagName_;
	}
	public void setExtendedTagName(String extendedTagName)
	{
		extendedTagName_ = extendedTagName;
	}

	public void setNeedsExpanding(boolean needsExpanding)
	{
		needsExpanding_ = needsExpanding;
	}
	public boolean getNeedsExpanding()
	{
		return needsExpanding_;
	}

	public List<TagKey> getKeyChildren()
	{
		if (keyChildren_ == null)
			keyChildren_ = new ArrayList<TagKey>();
		return keyChildren_;
	}

	public List<Tag> getTagChildren()
	{
		if (tagChildren_ == null)
			tagChildren_ = new ArrayList<Tag>();
		return tagChildren_;
	}


	/**
	 * A tag comparator that sorts just after required cardinality.
	 */
	public static class CardinalityComparator implements Comparator<Tag>
	{
		@Override
		public int compare(Tag o1, Tag o2)
		{
			if (o1.cardinality_ == o2.cardinality_)
				return 0;
			if (o1.cardinality_ == '1')
				return 1;
			else if (o2.cardinality_ == '1')
				return -1;
			return 0;
		}
	}

}
