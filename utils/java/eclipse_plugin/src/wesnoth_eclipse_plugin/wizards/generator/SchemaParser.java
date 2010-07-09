/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

import java.io.File;
import java.util.HashMap;
import java.util.Stack;

import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.preferences.Preferences;
import wesnoth_eclipse_plugin.utils.ResourceUtils;
import wesnoth_eclipse_plugin.utils.StringUtils;

public class SchemaParser
{
	private static SchemaParser	instance_;

	public static SchemaParser getInstance()
	{
		if (instance_ == null)
			instance_ = new SchemaParser();
		return instance_;
	}

	private HashMap<String, String>	primitives_		= new HashMap<String, String>();
	private HashMap<String, Tag>	tags_			= new HashMap<String, Tag>();
	private boolean					parsingDone_	= false;

	public void parseSchema(boolean force)
	{
		//TODO: sort tags's keys by cardinality (required first) ??
		if (parsingDone_ && !force)
			return;

		parsingDone_ = false;
		if (force)
		{
			primitives_.clear();
			tags_.clear();
		}

		Logger.print("parsing schema " + (force == true ? "forced" : ""));
		File schemaFile = new File(Preferences.getString(
						Constants.P_WESNOTH_WORKING_DIR) + "/data/schema.cfg");
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
				if (line.charAt(line.indexOf("[") + 1) == '/')
				{
					tagStack.pop();
				}
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
						//System.out.println(simpleTagName);
						if (tags_.containsKey(simpleTagName))
						{
							currentTag = tags_.get(simpleTagName);
							currentTag.ExtendedTagName = extendedTagName;
						}
						else
						{
							Tag tag = new Tag(simpleTagName, extendedTagName, '_');
							currentTag = tag;
							currentTag.NeedsExpanding = false;
							tags_.put(simpleTagName, tag);
						}
					}
				}
			}
			else
			{
				// skip descriptions for now
				if (tagStack.peek().equals("description"))
				{
					continue;
				}

				// top level - primitives defined
				if (tagStack.peek().equals("schema"))
				{
					String[] tokens = line.split("=");
					if (tokens.length != 2)
					{
						System.err.println("Error. invalid line :" + index);
						continue; //return;
					}
					primitives_.put(tokens[0].trim(), tokens[1].trim());
					//System.out.printf("[%s][%s]\n", tokens[0].trim(), tokens[1].trim());
				}
				else
				{
					String tmpLine = line.trim();
					if (line.contains("#"))
						tmpLine = line.substring(0, line.lastIndexOf("#")).trim();
					String[] tokens = tmpLine.split("=");

					if (tokens.length != 2)
					{
						System.err.println("Error. invalid line :" + index);
						continue; //return;
					}

					//						// this *should* happen only in [description]
					//						// multi-line string
					//						String value = tokens[1];
					//						if (StringUtils.countOf(value, '"') % 2 != 0)
					//						{
					//							++index;
					//							while (StringUtils.countOf(lines[index], '"') % 2 == 0 &&
					//									!StringUtils.startsWith(lines[index], "#") &&
					//									index < lines.length)
					//							{
					//								value += (lines[index] + "\n");
					//								++index;
					//							}
					//							value += lines[index];
					//						}

					String[] value = tokens[1].substring(1, tokens[1].length() - 1).split(" ");
					if (value.length != 2)
					{
						System.err.println("Error. invalid line :" + index);
						continue; //return;
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
								//System.err.println("creating missing tag: " + value[1]);
								tags_.put(value[1], targetTag);
							}

							currentTag.addTag(targetTag);
							currentTag.NeedsExpanding = true;
						}
						else
						{
							if (!(primitives_.containsKey(value[1])))
								currentTag.NeedsExpanding = true;
							if (primitives_.get(value[1]) == null)
								System.err.println("Undefined primitive type in schema.cfg for: " + value[1]);
							currentTag.addKey(tokens[0], primitives_.get(value[1]),
									getCardinality(value[0]), value[1].equals("tstring"));
						}
					}
					else
						System.err.println("can't find entry for: " + tagStack.peek());
				}
			}
		}
		//System.out.println("End parsing");
		parsingDone_ = true;
		//
		//		try
		//		{
		//			BufferedWriter bw = new BufferedWriter(new PrintWriter(new File("D:\\timo\\gw\\data\\schema-out.cfg")));
		//			// print primitives
		//			for (Entry<String, String> primitive : primitives.entrySet())
		//			{
		//				bw.write(primitive.getKey() + ": " + primitive.getValue() + "\n");
		//			}
		//			// print tags
		//			Tag root = tags.get("root");
		//			for (Tag tag : root.TagChildren)
		//			{
		//				bw.write(getOutput(tag, ""));
		//			}
		//			bw.close();
		//		} catch (Exception e)
		//		{
		//			e.printStackTrace();
		//		}
		//		System.out.println("End writing result");
	}

	public HashMap<String, Tag> getTags()
	{
		return tags_;
	}

	public HashMap<String, String> getPrimitives()
	{
		return primitives_;
	}

	public String getOutput(Tag tag, String indent)
	{
		String res = indent + "[" + tag.Name + "]\n";
		for (TagKey key : tag.KeyChildren)
		{
			res += (indent + "\t" + key.Name + "=" + key.ValueType + "\n");
		}
		for (Tag tmpTag : tag.TagChildren)
		{
			res += (getOutput(tmpTag, indent + "\t"));
		}
		res += (indent + "[/" + tag.Name + "]\n");
		return res;
	}

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
