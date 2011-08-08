/*******************************************************************************
 * Copyright (c) 2011 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.installs;

public class WesnothInstall
{
    private String name_;
    private String version_;

    public WesnothInstall( String name, String version )
    {
        name_ = name;
        version_ = version;
    }

    public String getName( )
    {
        return name_;
    }

    public String getVersion( )
    {
        return version_;
    }

    public void setVersion( String newVersion )
    {
        version_ = newVersion;
    }
}
