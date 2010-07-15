/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards;

public class ReplaceableParameter{
	public String paramName;
	public String paramValue;
	public ReplaceableParameter(String name, String value)
	{
		paramName = name;
		paramValue = value;
	}
}