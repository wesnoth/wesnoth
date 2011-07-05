/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.schema;

import java.io.File;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Stack;

import org.wesnoth.Logger;
import org.wesnoth.Messages;
import org.wesnoth.installs.WesnothInstall;
import org.wesnoth.installs.WesnothInstallsUtils;
import org.wesnoth.preferences.Preferences;
import org.wesnoth.utils.ResourceUtils;
import org.wesnoth.utils.StringUtils;

/**
 * This is a 'schema.cfg' parser.
 */
public class SchemaParser
{
    protected static Map< String, SchemaParser > parsers_ =
        new HashMap<String, SchemaParser>();

	/**
	 * Returns a SchemaParser instance based on the specified install
	 * @param installName The name of the install
	 * @return A SchemaParser singleton instance
	 */
	public static SchemaParser getInstance( String installName )
	{
	    // null not allowed
	    if ( installName == null )
	        installName = "";

	    SchemaParser parser = parsers_.get( installName );
		if (parser == null) {
			parser = new SchemaParser( installName );
			parser.parseSchema( false );
			parsers_.put( installName, parser );
		}

		return parser;
	}

	/**
	 * Reloads all currently stored schemas
	 * @param force True to force reloading schemas
	 */
	public static void reloadSchemas( boolean force )
	{
        List< WesnothInstall > installs = WesnothInstallsUtils.getInstalls( );
        for ( WesnothInstall install : installs ) {
            getInstance( install.getName( ) ).parseSchema( force );
        }
	}

	private Map<String, String> primitives_;
	private Map<String, Tag>    tags_;
	private boolean             parsingDone_;
    private String              installName_;

    private SchemaParser( String installName )
    {
        installName_ = installName;
        primitives_ = new HashMap<String, String>();
        tags_ = new HashMap<String, Tag>();
        parsingDone_ = false;
    }

	/**
	 * Parses the schema
	 *
	 * @param installName The install to use. It will get the default schema
	 * path based on that
	 *
	 * @param force True to force parsing the schema, skipping the existing cache
	 */
	public void parseSchema( boolean force )
	{
		parseSchemaFile( force, Preferences.getPaths( installName_ ).getSchemaPath( ) );
	}

	/**
	 * Parses the schema
	 * @param force True to force parsing the schema, skipping the existing cache
	 * @param schemaPath The path to the 'schema.cfg' file
	 */
	public void parseSchemaFile( boolean force, String schemaPath )
	{
		if (parsingDone_ && !force)
		{
			Logger.getInstance().log(Messages.SchemaParser_1);
			return;
		}

		parsingDone_ = false;
		if (force)
		{
			primitives_.clear();
			tags_.clear();
		}

		Logger.getInstance().log(Messages.SchemaParser_2 + (force == true ? Messages.SchemaParser_3 : "")); //$NON-NLS-1$
		File schemaFile = new File(schemaPath);
		String res = ResourceUtils.getFileContents(schemaFile);
		String[] lines = StringUtils.getLines(res);
		Stack<String> tagStack = new Stack<String>();

		Tag currentTag = null;
		for (int index = 0; index < lines.length; index++)
		{
			String line = lines[index];
			// skip comments and empty lines
			if (StringUtils.startsWith(line, "#") || line.matches("^[\t ]*$")) //$NON-NLS-1$ //$NON-NLS-2$
				continue;

			if (StringUtils.startsWith(line, "[")) //$NON-NLS-1$
			{
				// closing tag
				if (line.charAt(line.indexOf("[") + 1) == '/') //$NON-NLS-1$
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
					String tagName = line.substring(line.indexOf("[") + 1, line.indexOf("]")); //$NON-NLS-1$ //$NON-NLS-2$
					String simpleTagName = tagName;
					String extendedTagName = ""; //$NON-NLS-1$
					if (tagName.split(":").length > 1) //$NON-NLS-1$
					{
						simpleTagName = tagName.split(":")[0]; //$NON-NLS-1$
						extendedTagName = tagName.split(":")[1]; //$NON-NLS-1$
					}
					tagStack.push(simpleTagName);

					if (!tagName.equals("description")) //$NON-NLS-1$
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
				if (tagStack.peek().equals("schema")) //$NON-NLS-1$
				{
					String[] tokens = line.split("="); //$NON-NLS-1$
					if (tokens.length != 2)
					{
						Logger.getInstance().logError(
								Messages.SchemaParser_18 + index);
						continue;
					}
					primitives_.put(tokens[0].trim(), tokens[1].trim());
				}
				else if (tagStack.peek().equals("description")) //$NON-NLS-1$
				{
					String[] tokens = line.trim().split("="); //$NON-NLS-1$
					StringBuilder value = new StringBuilder(); //$NON-NLS-1$

					// this *should* happen only in [description]
					// multi-line string
					if (StringUtils.countOf(tokens[1], '"') % 2 != 0)
					{
						value.append(tokens[1] + "\n"); //$NON-NLS-1$
						++index;
						while (StringUtils.countOf(lines[index], '"') % 2 == 0 &&
								!StringUtils.startsWith(lines[index], "#") && //$NON-NLS-1$
								index < lines.length)
						{
							value.append(lines[index] + "\n"); //$NON-NLS-1$
							++index;
						}
						value.append(lines[index]);
					}
					else
					{
						value.append(tokens[1]);
					}

					// get rid of the quotes
					if (value.length() >= 2)
					{
						value = new StringBuilder(value.substring(1, value.length() - 1));
					}

					currentTag.setDescription(new Tag(Messages.SchemaParser_25, '?'));
					currentTag.getDescription().getKeyChildren().add(
							new TagKey(tokens[0], '?', "", value.toString(), true)); //$NON-NLS-1$
				}
				else
				{
					String tmpLine = line.trim();
					if (line.contains("#")) //$NON-NLS-1$
						tmpLine = line.substring(0, line.lastIndexOf("#")).trim(); //$NON-NLS-1$
					String[] tokens = tmpLine.split("="); //$NON-NLS-1$

					if (tokens.length != 2)
					{
						Logger.getInstance().logError(
								Messages.SchemaParser_30 + index);
						continue;
					}

					String[] value = tokens[1].substring(1, tokens[1].length() - 1).split(" "); //$NON-NLS-1$
					if (value.length != 2)
					{
						Logger.getInstance().logError(
								Messages.SchemaParser_32 + index);
						continue;
					}

					if (currentTag != null)
					{
						if (tokens[0].startsWith("_")) // reference to another tag //$NON-NLS-1$
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
								Messages.SchemaParser_34 + value[1]);

							currentTag.addKey(tokens[0], primitives_.get(value[1]),
									getCardinality(value[0]), value[1].equals("tstring")); //$NON-NLS-1$
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

		Logger.getInstance().log(Messages.SchemaParser_36);
		parsingDone_ = true;
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
	public Map<String, Tag> getTags()
	{
		return tags_;
	}

	/**
	 * Gets the hasmap with the parsed primitives
	 */
	public Map<String, String> getPrimitives()
	{
		return primitives_;
	}

	/**
	 * Returns the 'wmlwise' output of a specified tag using the specified indent
	 * @param tag The tag which contents to output
	 * @param indent The indentation space
	 */
	public String getOutput(Tag tag, int indent)
	{
		if (tag == null)
			return ""; //$NON-NLS-1$
		StringBuilder res = new StringBuilder();
		// tag name
		res.append(StringUtils.multiples("\t", indent) + "[" + tag.getName() + "]\n"); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
		// tag description (if any)
		res.append(getOutput(tag.getDescription(), indent + 1));

		for (TagKey key : tag.getKeyChildren())
		{
			res.append(StringUtils.multiples("\t", indent) + //$NON-NLS-1$
					"\t" + key.getName() + "=" + key.getValue() + "\n"); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
		}
		for (Tag tmpTag : tag.getTagChildren())
		{
			// skip recursive calls
			if (tmpTag.getTagChildren().contains(tag))
				continue;
			res.append(getOutput(tmpTag, indent + 1));
		}

		// closing tag
		res.append(StringUtils.multiples("\t", indent) + //$NON-NLS-1$
				"[/" + tag.getName() + "]\n"); //$NON-NLS-1$ //$NON-NLS-2$
		return res.toString();
	}

	/**
	 * Returns the cardinality as a character of the specified value
	 * required = 1
	 * optional = ?
	 * repeated = *
	 * forbidden = -
	 * @param value The value
	 */
	public char getCardinality(String value)
	{
		if (value.equals("required")) //$NON-NLS-1$
			return '1';
		else if (value.equals("optional")) //$NON-NLS-1$
			return '?';
		else if (value.equals("repeated")) //$NON-NLS-1$
			return '*';
		else if (value.equals("forbidden")) //$NON-NLS-1$
			return '-';
		return 'a';
	}
}
