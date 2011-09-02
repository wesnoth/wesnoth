/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.installs;

// TODO: for each install, when first used, we should get data from
// that installation:
// - lua wml tags
/**
 * This class represents a Wesnoth Install's data
 */
public class WesnothInstall
{
    private String name_;
    private String version_;

    /**
     * Creates a new install with the specified name and version
     * 
     * @param name
     *        The name of the wesnoth install
     * @param version
     *        The version of wesnoth
     * 
     */
    public WesnothInstall( String name, String version )
    {
        name_ = name;
        version_ = version;
    }

    /**
     * @return The name of the install given by the user
     */
    public String getName( )
    {
        return name_;
    }

    /**
     * @return The Wesnoth version of the install
     */
    public String getVersion( )
    {
        return version_;
    }

    /**
     * Sets this wesnoth install's version
     * 
     * @param newVersion
     *        The new version String
     */
    public void setVersion( String newVersion )
    {
        version_ = newVersion;
    }
}
