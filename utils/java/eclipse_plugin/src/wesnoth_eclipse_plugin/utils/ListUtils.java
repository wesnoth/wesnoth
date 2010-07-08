/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.utils;

import java.util.List;

public class ListUtils
{
	/**
	 * Concatenates the list of strings using the provided separator
	 * 
	 * @param list the list to concatenate
	 * @param separator the separator used to concatenate the elements of the list
	 * @return
	 */
	public static String concatenateList(List<String> list, String separator)
	{
		String result = "";
		for (int i = 0; i < list.size() - 1; i++)
		{
			result += (list.get(i) + separator);
		}
		if (!list.isEmpty())
			result += list.get(list.size() - 1);
		return result;
	}

	/**
	 * Concatenates the array of strings using the provided separator
	 * 
	 * @param array the array to concatenate
	 * @param separator the separator used to concatenate the elements of the list
	 * @return
	 */
	public static String concatenateArray(String[] array, String separator)
	{
		String result = "";
		for (int i = 0; i < array.length - 1; i++)
		{
			result += (array[i] + separator);
		}
		if (array.length > 0)
			result += array[array.length - 1];
		return result;
	}
}
