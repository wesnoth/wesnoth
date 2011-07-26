/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
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
public class WMLVariable
{
	private String name_;
	private String location_;
	private int offset_;
	private boolean isArray_;
	private int scopeStartIndex_;
	private int scopeEndIndex_;

	public WMLVariable()
	{
		this("", "", 0); //$NON-NLS-1$ //$NON-NLS-2$
	}

	public WMLVariable( String name, String location, int offset )
	{
	    this( name, location, offset, Integer.MIN_VALUE, Integer.MAX_VALUE );
	}

	public WMLVariable( String name, String location, int offset,
	        int startIndex, int endIndex )
    {
        name_ = name;
        location_ = location;
        offset_ = offset;

        scopeStartIndex_ = startIndex;
        scopeEndIndex_ = endIndex;
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

	public int getScopeEndIndex()
    {
        return scopeEndIndex_;
    }
	public int getScopeStartIndex()
    {
        return scopeStartIndex_;
    }
	public void setScopeEndIndex( int scopeEndIndex )
    {
        scopeEndIndex_ = scopeEndIndex;
    }
	public void setScopeStartIndex( int scopeStartIndex )
    {
        scopeStartIndex_ = scopeStartIndex;
    }

	@Override
	public String toString()
	{
	    return "Variable - Name: " + name_ +
	            "; Location:" + location_ +
	            "; Offset:" + offset_ +
	            "; Scope: " + scopeStartIndex_ + " -> " + scopeEndIndex_;
	}
}
