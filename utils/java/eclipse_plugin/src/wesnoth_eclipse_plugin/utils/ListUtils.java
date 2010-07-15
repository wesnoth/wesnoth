/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
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
		if (list == null || list.isEmpty())
			return "";
		String result = "";
		for (int i = 0; i < list.size() - 1; i++)
		{
			result += (list.get(i) + separator);
		}
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
		if (array == null || array.length == 0)
			return "";

		String result = "";
		for (int i = 0; i < array.length - 1; i++)
		{
			result += (array[i] + separator);
		}
		result += array[array.length - 1];
		return result;
	}
}
