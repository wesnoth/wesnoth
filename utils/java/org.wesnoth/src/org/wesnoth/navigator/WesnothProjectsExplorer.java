/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.navigator;

import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.IAdaptable;
import org.eclipse.ui.IElementFactory;
import org.eclipse.ui.IMemento;
import org.eclipse.ui.IPersistableElement;
import org.eclipse.ui.model.IWorkbenchAdapter;
import org.eclipse.ui.navigator.CommonNavigator;

public class WesnothProjectsExplorer extends CommonNavigator implements IAdaptable,
	IPersistableElement, IElementFactory
{
	public WesnothProjectsExplorer()
	{
	}

	@Override
	@SuppressWarnings("rawtypes")
	public Object getAdapter(Class adapter)
	{
		if (adapter == IPersistableElement.class) return this;
		if (adapter == IWorkbenchAdapter.class)
			return ResourcesPlugin.getWorkspace().getRoot().getAdapter(adapter);
		return null;
	}

	public String getFactoryId()
	{
		return this.getClass().getCanonicalName();
	}

	@Override
	public void saveState(IMemento memento)
	{
		return;
	}

	public IAdaptable createElement(IMemento memento)
	{
		return ResourcesPlugin.getWorkspace().getRoot();
	}
}
