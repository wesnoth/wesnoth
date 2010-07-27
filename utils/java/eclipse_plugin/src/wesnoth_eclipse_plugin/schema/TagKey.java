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
	private String	 Name;
	/**
	 * Cardinality can be:
	 * 1 = required
	 * ? = optional
	 * * = repeated
	 * - = forbidden
	 */
	private char	 Cardinality;
	private String	 ValueType;
	private String   Value;
	private boolean IsEnum;
	private boolean IsTranslatable;

	public TagKey(String name, char cardinality, String valueType, boolean trans) {
		Name = name;
		Cardinality = cardinality;
		Value = "";

		if (valueType == null || valueType.isEmpty())
		{
			ValueType = "";
			IsEnum = false;
			IsTranslatable = false;
		}
		else
		{
			IsEnum = valueType.substring(1, valueType.indexOf(" ")).equals("enum");

			 // remove the " "
			ValueType = valueType.substring(valueType.indexOf(" ") + 1, valueType.length() - 1);
			IsTranslatable = trans;
		}
	}

	public TagKey(String name, char cardinality, String valueType,
			String defaultValue, boolean trans)
	{
		this(name, cardinality, valueType, trans);
		Value = defaultValue;
	}

	/**
	 * A tag comparator that sorts just after required cardinality.
	 */
	public static class CardinalityComparator implements Comparator<TagKey>
	{
		@Override
		public int compare(TagKey o1, TagKey o2)
		{
			if (o1.Cardinality == o2.Cardinality)
				return 0;
			if (o1.Cardinality == '1')
				return -1;
			else if (o2.Cardinality == '1')
				return 1;
			return 0;
		}
	}

	public char getCardinality()
	{
		return Cardinality;
	}
	public void setCardinality(char cardinality)
	{
		Cardinality = cardinality;
	}

	public String getValue()
	{
		return Value;
	}

	public void setDefaultValue(String defaultValue)
	{
		Value = defaultValue;
	}

	public String getName()
	{
		return Name;
	}

	public String getValueType()
	{
		return ValueType;
	}

	public boolean getIsTranslatable()
	{
		return IsTranslatable;
	}

	public boolean getIsEnum()
	{
		return IsEnum;
	}
}
