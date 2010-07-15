/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.generator;

import java.util.ArrayList;
import java.util.List;

class Tag
{
	public String		Name;
	public String		ExtendedTagName	= "";
	public List<Tag>	TagChildren;
	public List<TagKey>	KeyChildren;
	public char			Cardinality;

	public boolean		NeedsExpanding	= false;

	public Tag(String name, List<Tag> tagChildren, List<TagKey> keyChildren, char cardinality) {
		Name = name;
		TagChildren = tagChildren;
		KeyChildren = keyChildren;
		Cardinality = cardinality;
	}

	public Tag(String name, char cardinality) {
		Name = name;
		TagChildren = new ArrayList<Tag>();
		KeyChildren = new ArrayList<TagKey>();
		Cardinality = cardinality;
	}

	public Tag(String name, String extendedTagName, char cardinality) {
		Name = name;
		ExtendedTagName = extendedTagName;
		TagChildren = new ArrayList<Tag>();
		KeyChildren = new ArrayList<TagKey>();
		Cardinality = cardinality;
	}

	public void addTag(Tag tag)
	{
		TagChildren.add(tag);
	}

	public void addKey(TagKey key)
	{
		KeyChildren.add(key);
	}

	public void addKey(String name, String valueType, char cardinality, boolean translat)
	{
		addKey(new TagKey(name, cardinality, valueType, translat));
	}
}
