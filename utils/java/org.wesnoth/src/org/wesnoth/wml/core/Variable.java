/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wml.core;

/**
 * Represents a WML Variable
 */
public class Variable
{
	private String name_;
	private String location_;
	private int offset_;
	private boolean isArray_;

	public Variable()
	{
		this("", "", 0); //$NON-NLS-1$ //$NON-NLS-2$
	}

	public Variable(String name, String location, int offset)
	{
		name_ = name;
		location_ = location;
		offset_ = offset;
	}

	public String getName()
	{
		return name_;
	}
	public void setName(String name)
	{
		name_ = name;
	}
	public String getLocation()
	{
		return location_;
	}
	public void setLocation(String location)
	{
		location_ = location;
	}
	public int getOffset()
	{
		return offset_;
	}
	public void setOffset(int offset)
	{
		offset_ = offset;
	}
	public boolean isArray()
	{
		return isArray_;
	}
	public void setIsArray(boolean isArray)
	{
		isArray_ = isArray;
	}
}
