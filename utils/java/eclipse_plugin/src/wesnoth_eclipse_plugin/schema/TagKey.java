/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.schema;

import java.util.Comparator;

/**
 * This class represents a tag's key parsed by the schema
 */
public class TagKey
{
	private String name_;
	/**
	 * Cardinality can be:
	 * 1 = required
	 * ? = optional
	 * * = repeated
	 * - = forbidden
	 */
	private char cardinality_;
	private String	valueType_;
	private String value_;
	private boolean isEnum_;
	private boolean isTranslatable_;

	public TagKey(String name, char cardinality, String valueType, boolean trans) {
		name_ = name;
		cardinality_ = cardinality;
		value_ = "";

		if (valueType == null || valueType.isEmpty())
		{
			valueType_ = "";
			isEnum_ = false;
			isTranslatable_ = false;
		}
		else
		{
			isEnum_ = valueType.substring(1, valueType.indexOf(" ")).equals("enum");

			 // remove the " "
			valueType_ = valueType.substring(valueType.indexOf(" ") + 1, valueType.length() - 1);
			isTranslatable_ = trans;
		}
	}

	public TagKey(String name, char cardinality, String valueType,
			String defaultValue, boolean trans)
	{
		this(name, cardinality, valueType, trans);
		value_ = defaultValue;
	}

	/**
	 * A tag comparator that sorts just after required cardinality.
	 */
	public static class CardinalityComparator implements Comparator<TagKey>
	{
		@Override
		public int compare(TagKey o1, TagKey o2)
		{
			if (o1.cardinality_ == o2.cardinality_)
				return 0;
			if (o1.cardinality_ == '1')
				return -1;
			else if (o2.cardinality_ == '1')
				return 1;
			return 0;
		}
	}

	public char getCardinality()
	{
		return cardinality_;
	}
	public void setCardinality(char cardinality)
	{
		cardinality_ = cardinality;
	}

	public String getValue()
	{
		return value_;
	}

	public void setDefaultValue(String defaultValue)
	{
		value_ = defaultValue;
	}

	public String getName()
	{
		return name_;
	}

	public String getValueType()
	{
		return valueType_;
	}

	public boolean getIsTranslatable()
	{
		return isTranslatable_;
	}

	public boolean getIsEnum()
	{
		return isEnum_;
	}
}
