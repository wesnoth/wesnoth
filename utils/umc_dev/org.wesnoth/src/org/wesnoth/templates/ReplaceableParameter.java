/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.templates;

/**
 * The class represents a parameter that should be replace in a template
 */
public class ReplaceableParameter
{
    /**
     * The parameter name to replace
     */
    public String paramName;

    /**
     * The value to replace the parameter with
     */
    public String paramValue;

    /**
     * Creates a new {@link ReplaceableParameter}
     * 
     * @param name
     *        The name to replace
     * @param value
     *        The value to replace with
     */
    public ReplaceableParameter( String name, String value )
    {
        paramName = name;
        paramValue = value;
    }
}
