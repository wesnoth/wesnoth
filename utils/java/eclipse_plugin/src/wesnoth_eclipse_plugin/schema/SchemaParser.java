/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.schema;

import java.io.BufferedWriter;
import java.io.File;
import java.io.PrintWriter;
import java.util.Collections;
import java.util.HashMap;
import java.util.Map.Entry;
import java.util.Stack;

import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.preferences.Preferences;
import wesnoth_eclipse_plugin.utils.ResourceUtils;
import wesnoth_eclipse_plugin.utils.StringUtils;

/**
 * This is a 'schema.cfg' parser.
 */
public class SchemaParser
{
	private static SchemaParser	instance_;

	public static SchemaParser getInstance()
	{
		if (instance_ == null)
			instance_ = new SchemaParser();
		return instance_;
	}

	private HashMap<String, String> primitives_	    = new HashMap<String, String>();
	private HashMap<String, Tag>    tags_          = new HashMap<String, Tag>();
	private boolean                parsingDone_   = false;

	/**
	 * Parses the schema
	 * @param force True to force parsing the schema, skipping the existing cache
	 */
	public void parseSchema(boolean force)
	{
		parseSchema(force, Preferences.getString(
				Constants.P_WESNOTH_WORKING_DIR) + "/data/schema.cfg");
	}

	/**
	 * Parses the schema
	 * @param force True to force parsing the schema, skipping the existing cache
	 * @param schemaPath The path to the 'schema.cfg' file
	 */
	public void parseSchema(boolean force, String schemaPath)
	{
		if (parsingDone_ && !force)
		{
			Logger.getInstance().log("schema not parsed since there is already in cache.");
			return;
		}

		parsingDone_ = false;
		if (force)
		{
			primitives_.clear();
			tags_.clear();
		}

		Logger.getInstance().log("parsing schema " + (force == true ? "forced" : ""));
		File schemaFile = new File(schemaPath);
		String res = ResourceUtils.getFileContents(schemaFile);
		String[] lines = StringUtils.getLines(res);
		Stack<String> tagStack = new Stack<String>();

		Tag currentTag = null;
		for (int index = 0; index < lines.length; index++)
		{
			String line = lines[index];
			// skip comments and empty lines
			if (StringUtils.startsWith(line, "#") || line.matches("^[\t ]*$"))
				continue;

			if (StringUtils.startsWith(line, "["))
			{
				// closing tag
				if (line.charAt(line.indexOf("[") + 1) == '/')
				{
					// propagate the 'needsexpanding' property to upper levels
					boolean expand = false;
					if (!tagStack.isEmpty() &&
						tags_.containsKey(tagStack.peek()))
						expand = tags_.get(tagStack.peek()).getNeedsExpanding();

					tagStack.pop();

					if (!tagStack.isEmpty() &&
						tags_.containsKey(tagStack.peek()) &&
						expand == true)
						tags_.get(tagStack.peek()).setNeedsExpanding(expand);
				}
				// opening tag
				else
				{
					String tagName = line.substring(line.indexOf("[") + 1, line.indexOf("]"));
					String simpleTagName = tagName;
					String extendedTagName = "";
					if (tagName.split(":").length > 1)
					{
						simpleTagName = tagName.split(":")[0];
						extendedTagName = tagName.split(":")[1];
					}
					tagStack.push(simpleTagName);

					if (!tagName.equals("description"))
					{
						if (tags_.containsKey(simpleTagName))
						{
							// this tags was already refered in the schema
							// before they were declared
							currentTag = tags_.get(simpleTagName);
							currentTag.setExtendedTagName(extendedTagName);
							currentTag.setNeedsExpanding(!extendedTagName.isEmpty());
						}
						else
						{
							Tag tag = new Tag(simpleTagName, extendedTagName, '_');
							currentTag = tag;
							currentTag.setNeedsExpanding(!extendedTagName.isEmpty());
							tags_.put(simpleTagName, tag);
						}
					}
				}
			}
			else
			{
				// top level - primitives defined
				if (tagStack.peek().equals("schema"))
				{
					String[] tokens = line.split("=");
					if (tokens.length != 2)
					{
						Logger.getInstance().logError(
								"Error. invalid primitive on line :" + index);
						continue;
					}
					primitives_.put(tokens[0].trim(), tokens[1].trim());
				}
				else if (tagStack.peek().equals("description"))
				{
					String[] tokens = line.trim().split("=");
					String value = "";
					// this *should* happen only in [description]
					// multi-line string
					if (StringUtils.countOf(tokens[1], '"') % 2 != 0)
					{
						value = tokens[1] + "\n";
						++index;
						while (StringUtils.countOf(lines[index], '"') % 2 == 0 &&
								!StringUtils.startsWith(lines[index], "#") &&
								index < lines.length)
						{
							value += (lines[index] + "\n");
							++index;
						}
						value += lines[index];
					}
					else
					{
						value = tokens[1];
					}

					currentTag.setDescription(new Tag("description", '?'));
					currentTag.getDescription().getKeyChildren().add(
							new TagKey(tokens[0], '?', "", value, true));
				}
				else
				{
					String tmpLine = line.trim();
					if (line.contains("#"))
						tmpLine = line.substring(0, line.lastIndexOf("#")).trim();
					String[] tokens = tmpLine.split("=");

					if (tokens.length != 2)
					{
						Logger.getInstance().logError(
								"Error. invalid attribute on line :" + index);
						continue;
					}

					String[] value = tokens[1].substring(1, tokens[1].length() - 1).split(" ");
					if (value.length != 2)
					{
						Logger.getInstance().logError(
								"Error. invalid attribute value on line:" + index);
						continue;
					}

					if (currentTag != null)
					{
						if (tokens[0].startsWith("_")) // reference to another tag
						{
							Tag targetTag = null;
							if (tags_.containsKey(value[1]))
								targetTag = tags_.get(value[1]);
							else
							// tag wasn't created yet
							{
								targetTag = new Tag(value[1], getCardinality(value[0]));
								tags_.put(value[1], targetTag);
							}

							currentTag.addTag(targetTag);
						}
						else
						{
							if (primitives_.get(value[1]) == null)
								Logger.getInstance().logError(
								"Undefined primitive type in schema.cfg for: " + value[1]);

							currentTag.addKey(tokens[0], primitives_.get(value[1]),
									getCardinality(value[0]), value[1].equals("tstring"));
						}
					}
					else
					{
						//System.err.println("can't find entry for: " + tagStack.peek());
					}
				}
			}
		}

		sortTags();

		for (Tag tag : tags_.values())
		{
			expandTag(tag,0);
		}

		Logger.getInstance().log("parsing done");
		parsingDone_ = true;

		try
		{
			BufferedWriter bw = new BufferedWriter(new PrintWriter(new File("E:/work/gw/data/schema-out.cfg")));
			// print primitives
			for (Entry<String, String> primitive : primitives_.entrySet())
			{
				bw.write(primitive.getKey() + ": " + primitive.getValue() + "\n");
			}
			// print tags
			Tag root = tags_.get("root");
			for (Tag tag : root.getTagChildren())
			{
				bw.write(getOutput(tag, ""));
			}
			bw.close();
		} catch (Exception e)
		{
			Logger.getInstance().logException(e);
		}
		System.out.println("End writing result");

	}

	/**
	 * Expands the tags that need to (the ones based on inheritance)
	 */
	private void expandTag(Tag tag, int ind)
	{
		if (tag.getNeedsExpanding())
		{
			tag.setNeedsExpanding(false);
			for (Tag child : tag.getTagChildren())
			{
				expandTag(child,ind+1);
			}

			if (tags_.containsKey(tag.getExtendedTagName()))
			{
				tag.getKeyChildren().addAll(tags_.get(tag.getExtendedTagName()).getKeyChildren());
				tag.getTagChildren().addAll(tags_.get(tag.getExtendedTagName()).getTagChildren());
			}
		}
	}

	/**
	 * Sorts the tags in the hashmap
	 */
	private void sortTags()
	{
		for(Tag tag : tags_.values())
		{
			sortChildren(tag);
		}
	}

	/**
	 * Sorts all tag's children by using the cardinality comparator
	 * @param tag
	 */
	private void sortChildren(Tag tag)
	{
		Collections.sort(tag.getTagChildren(), new Tag.CardinalityComparator());
		Collections.sort(tag.getKeyChildren(), new TagKey.CardinalityComparator());

		for (Tag childTag : tag.getTagChildren())
		{
			sortChildren(childTag);
		}
	}

	/**
	 * Gets the hasmap with parsed tags
	 */
	public HashMap<String, Tag> getTags()
	{
		return tags_;
	}

	/**
	 * Gets the hasmap with the parsed primitives
	 */
	public HashMap<String, String> getPrimitives()
	{
		return primitives_;
	}

	/**
	 * Returns the 'wmlwise' output of a specified tag using the specified indent
	 * @param tag The tag which contents to output
	 * @param indent The indentation space
	 */
	public String getOutput(Tag tag, String indent)
	{
		if (tag == null)
			return "";
		String res = indent + "[" + tag.getName() + "]\n";
		res += getOutput(tag.getDescription(), indent + "\t");
		for (TagKey key : tag.getKeyChildren())
		{
			res += (indent + "\t" + key.getName() + "=" +
					(tag.getName().equals("description") ?
							key.getValue() : key.getValueType())
					+ "\n");
		}
		for (Tag tmpTag : tag.getTagChildren())
		{
			// skip recursive calls
			if (tmpTag.getTagChildren().contains(tag))
				continue;
			res += (getOutput(tmpTag, indent + "\t"));
		}
		res += (indent + "[/" + tag.getName() + "]\n");
		return res;
	}

	/**
	 * Returns the cardinality as a character of the specified value
	 * @param value The value
	 */
	public char getCardinality(String value)
	{
		if (value.equals("required"))
			return '1';
		else if (value.equals("optional"))
			return '?';
		else if (value.equals("repeated"))
			return '*';
		else if (value.equals("forbidden"))
			return '-';
		return 'a';
	}
}
