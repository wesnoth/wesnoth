/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.preprocessor;

/**
 * This represents a WML preprocessor define:
 * [preproc_define]
 *		name = ""
 *		value = ""
 *		textdomain = ""
 *		linenum = ""
 *		location = ""
 * [/preproc_define]
 */
public class Define
{
	private String name_;
	private String value_;
	private String textdomain_;
	private int lineNum_;
	private String location_;

	public Define(String name, String value, String textdomain,
				int linenum, String location)
	{
		name_ = name;
		value_ = value;
		textdomain_ = textdomain;
		lineNum_ = linenum;
		location_ = location;
	}

	public int getLineNum()
	{
		return lineNum_;
	}
	public String getLocation()
	{
		return location_;
	}
	public String getName()
	{
		return name_;
	}
	public String getTextdomain()
	{
		return textdomain_;
	}
	public String getValue()
	{
		return value_;
	}
}
