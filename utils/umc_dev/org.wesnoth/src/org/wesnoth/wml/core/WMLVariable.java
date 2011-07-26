/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wml.core;

import java.io.Serializable;
import java.util.List;

import org.wesnoth.utils.Pair;

/**
 * Represents a WML Variable
 */
public class WMLVariable implements Serializable
{
    private static final long serialVersionUID = 5293113569770337870L;

    private String name_;
	private String location_;
	private int offset_;
	private boolean isArray_;

	private List< Pair<Integer, Integer> > scopes_;

	public WMLVariable()
	{
		this("", "", 0); //$NON-NLS-1$ //$NON-NLS-2$
	}

	public WMLVariable( String name, String location, int offset )
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

	public List<Pair<Integer, Integer>> getScopes()
    {
        return scopes_;
    }

	@Override
	public String toString()
	{
	    StringBuilder res = new StringBuilder( );

	    res.append( "Variable - Name: " + name_ );
	    res.append( "; Location:" + location_ );
        res.append( "; Offset:" + offset_ );
        if ( ! scopes_.isEmpty( ) ) {
            res.append( "; Scopes: " );

            for ( Pair<Integer,Integer> scope : scopes_ ) {
                res.append( scope );
            }
        }

        return res.toString( );
	}
}
