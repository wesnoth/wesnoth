/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.navigation;

import java.io.File;

import org.eclipse.xtext.ui.editor.hyperlinking.XtextHyperlink;

import wesnoth_eclipse_plugin.utils.GameUtils;

public class MapOpenerHyperlink extends XtextHyperlink
{
	private String location_;

	public void setLocation(String location)
	{
		location_ = location;
	}

	public String getLocation()
	{
		return location_;
	}

	@Override
	public void open()
	{
		if (new File(location_).exists())
		{
			GameUtils.startEditor(location_);
		}
//		else
//		{
//			// create a new file
//			//TODO: create the map
//			Logger.getInstance().log("Creating new map: " + mapLocation);
//
//		}
	}
}
