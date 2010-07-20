/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.utils;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class StringUtils
{
	/**
	 * Returns true if the 'target' starts with 'sequence'.
	 * The tabs or spaces in front are skipped when checking for the 'sequence'
	 * @param target
	 * @param sequence
	 * @return
	 */
	public static boolean startsWith(String target, String sequence)
	{
		if (target == null || sequence == null)
			return false;

		Pattern pattern = Pattern.compile("[\t ]*" + Pattern.quote(sequence));
		Matcher matcher = pattern.matcher(target);
		return (matcher.find() && matcher.start() == 0);
	}

	/**
	 * Returns the number of 'character' characters in the target string
	 *
	 * @param target
	 * @param character
	 * @return
	 */
	public static int countOf(String target, char character)
	{
		if (target == null)
			return 0;

		if (target.indexOf(character) == -1)
			return 0;

		int cnt = 0;
		String tmpString = target;
		while (tmpString.contains(new String(new char[] { character })))
		{
			++cnt;
			tmpString = tmpString.substring(tmpString.indexOf(character) + 1);
		}
		return cnt;
	}

	/**
	 * Returns the n-th occurrence index of the specified character
	 *
	 * @param target
	 * @param character
	 * @param n
	 * @return
	 */
	public static int indexOfNth(String target, char character, int n)
	{
		if (target == null)
			return -1;

		if (countOf(target, character) < n)
			return -1;

		int index = -1, cnt = 0;
		for (int i = 0; i < target.length() && cnt <= n; ++i)
		{
			if (target.charAt(i) == character)
			{
				index = i;
				++cnt;
			}
		}
		return index;
	}

	/**
	 * Removes all consecutive aparitions of a character in the specified string
	 * so that only one appearance remains in each past duplications of that
	 * string
	 *
	 * @param target the string to process
	 * @param character the character to remove
	 * @param removeTrailing removes or not the trailing 'character' characters
	 * @param removeTrailing removes or not the preceding 'character' characters
	 * @return
	 */
	public static String removeIncorrectCharacters(String target, char character,
			boolean removeTrailing, boolean removePreceding)
	{
		if (target == null)
			return "";

		StringBuilder resString = new StringBuilder();

		for (int i = 0; i < target.length(); i++)
		{
			// pass over successive repetitions:
			// abbbac will become: abac
			if (i > 0 && (target.charAt(i) == target.charAt(i - 1)))
			{
				continue;
			}

			if (target.charAt(i) == character &&
				((removeTrailing && i == target.length()) || (removePreceding && i == 0)))
			{
				continue;
			}

			resString.append(target.charAt(i));
		}
		return resString.toString();
	}

	/**
	 * Gets an array of the lines (without line breakes) which compund the string
	 * @param string The string
	 * @return
	 */
	public static String[] getLines(String string)
	{
		if (string == null)
			return new String[0];
		return string.split("\\r?\\n");
	}

	/**
	 * Removes all path separators from the string
	 * @param string
	 * @return
	 */
	public static String trimPathSeparators(String string)
	{
		if (string == null || string.isEmpty())
			return "";

		while (string.charAt(string.length() - 1) == '/' ||
				string.charAt(string.length() - 1) == '\\')
			string = string.substring(0, string.length() - 1);
		return string;
	}

	/**
	 * Normalizez the path given by the string, removing repeated separators
	 * and replacing them by '|'
	 *
	 * @param path the string that represents the path to be normalized
	 * @return
	 */
	public static String normalizePath(String path)
	{
		if (path == null)
			return "";

		String str = StringUtils.trimPathSeparators(path);
		StringBuilder sb = new StringBuilder(str.length());

		// normalize strings - remove repeating separators and replace them by |
		for (int i = 0, sw = 0; i < str.length(); i++)
		{
			if (str.charAt(i) == '/' || str.charAt(i) == '\\')
			{
				if (sw == 0) // we haven't added already the | token
				{
					sb.append('|');
					sw = 1;
				}
				continue;
			}
			sb.append(str.charAt(i));
			sw = 0; // reset the switch when meeting non-separators
		}
		return sb.toString();
	}

	/**
	 * Replaces the source with the target in the specified string,
	 * adding the existing indentation (if any) to all lines (if any) in the target
	 *
	 * @param string The string where to replace the source
	 * @param source The source string to be replaced
	 * @param target The target string to be replaced in the string
	 * @return
	 */
	public static String replaceWithIndent(String string, String source, String target)
	{
		if (string == null)
			return "";

		// get the current indentation
		Pattern pattern = Pattern.compile("[\t ]*");
		Matcher matcher = pattern.matcher(string);
		String indent = "";
		if (matcher.find())
		{
			int end = matcher.end();
			indent = string.substring(matcher.start(), end);
		}

		String[] tmpTarget = getLines(target);
		for (int i = 1; i < tmpTarget.length; i++)
			tmpTarget[i] = indent + tmpTarget[i];

		return string.replace(source, ListUtils.concatenateArray(tmpTarget, "\n"));
	}

	/**
	 * Returns a sequence multiplied by the specified number of times
	 * @param sequence The sequence to multiply
	 * @param times The number of times to multiply
	 * @return
	 */
	public static String multiples(String sequence, int times)
	{
		if (sequence == null)
			return "";

		StringBuilder res = new StringBuilder(sequence.length() * times);
		for (int i = 0; i < times; i++)
		{
			res.append(sequence);
		}
		return res.toString();
	}
}
