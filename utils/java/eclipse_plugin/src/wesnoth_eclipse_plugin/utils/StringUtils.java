/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.utils;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class StringUtils
{
	public static boolean startsWith(String target, String sequence)
	{
		Pattern pattern = Pattern.compile("[\t| ]*" + Pattern.quote(sequence));
		Matcher matcher = pattern.matcher(target);
		return (matcher.find() && matcher.start() == 0);
	}

	/**
	 * Returns the number of 'character' characters in the target string
	 * @param target
	 * @param character
	 * @return
	 */
	public static int countOf(String target, char character)
	{
		if (target.indexOf(character) == -1)
			return -1;
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
	 * @param target
	 * @param character
	 * @param n
	 * @return
	 */
	public static int indexOfNth(String target, char character, int n)
	{
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
	 * @param target the string to process
	 * @param character the character to remove
	 * @param removeTrailing removes or not the trailing 'character' characters
	 * @param removeTrailing removes or not the preceding 'character' characters
	 * @return
	 */
	public static String removeIncorrectCharacters(String target, char character, boolean removeTrailing, boolean removePreceding)
	{
		StringBuilder resString = new StringBuilder();

		for (int i = 0; i < target.length(); i++)
		{
			// pass over successive repetitions:
			// abbbac will become: abac
			if (i > 0 && (target.charAt(i) == target.charAt(i - 1)))
			{
				continue;
			}

			if (target.charAt(i) == character && ((removeTrailing && i == target.length()) || (removePreceding && i == 0)))
			{
				continue;
			}

			resString.append(target.charAt(i));
		}
		return resString.toString();
	}

	public static String[] getLines(String string)
	{
		return string.split("\\r?\\n");
	}
}
