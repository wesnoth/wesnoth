/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.emf;

import org.eclipse.emf.common.notify.Adapter;
import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.common.notify.Notifier;

/**
 * A simple adapter that stores an object
 */
public class ObjectStorageAdapter implements Adapter
{
	private Object object_;
	private Notifier notifier_;

	public ObjectStorageAdapter(Object data)
	{
		object_ = data;
	}

	public Object getObject()
	{
		return object_;
	}

	public void setObject(Object obj)
	{
		object_ = obj;
	}

	public Notifier getTarget()
	{
		return notifier_;
	}

	public boolean isAdapterForType(Object arg0)
	{
		return true;
	}

	public void notifyChanged(Notification arg0)
	{
	}

	public void setTarget(Notifier arg0)
	{
		notifier_ = arg0;
	}
}
