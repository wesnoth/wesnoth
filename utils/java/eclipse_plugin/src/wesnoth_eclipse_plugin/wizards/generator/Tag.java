/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

import java.util.ArrayList;
import java.util.List;

class Tag
{
	public String		Name;
	public String		ExtendedTagName	= "";
	public List<Tag>	TagChildren;
	public List<TagKey>	KeyChildren;

	public boolean		NeedsExpanding	= false;

	public Tag(String name, List<Tag> tagChildren, List<TagKey> keyChildren) {
		Name = name;
		TagChildren = tagChildren;
		KeyChildren = keyChildren;
	}

	public Tag(String name) {
		Name = name;
		TagChildren = new ArrayList<Tag>();
		KeyChildren = new ArrayList<TagKey>();
	}

	public Tag(String name, String extendedTagName) {
		Name = name;
		ExtendedTagName = extendedTagName;
		TagChildren = new ArrayList<Tag>();
		KeyChildren = new ArrayList<TagKey>();
	}

	public void addTag(Tag tag)
	{
		TagChildren.add(tag);
	}

	public void addKey(TagKey key)
	{
		KeyChildren.add(key);
	}

	public void addKey(String name, String regex, char cardinality)
	{
		addKey(new TagKey(name, cardinality, regex));
	}
}
