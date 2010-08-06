/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.preprocessor;

import java.util.List;

import wesnoth_eclipse_plugin.utils.ResourceUtils;

public class DefinesCache
{
	private static List<Define> defines_;

	public static void readDefines(String file)
	{
		DefinesSAXHandler handler = (DefinesSAXHandler) ResourceUtils.
				getWMLSAXHandlerFromResource(file, new DefinesSAXHandler());
		defines_ = handler.getDefines();
	}

	public static List<Define> getDefines()
	{
		return defines_;
	}
}
