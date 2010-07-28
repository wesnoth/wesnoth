/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.emf;

import java.util.HashMap;

import org.eclipse.emf.common.notify.Adapter;
import org.eclipse.emf.common.notify.Notification;
import org.eclipse.emf.common.notify.Notifier;

/**
 * An adapter that provides storage for data in a hashmap
 */
public class HashMapStorageAdapter implements Adapter
{
	private HashMap<String, Object> data_;
	private Notifier notifier_;

	public HashMapStorageAdapter()
	{
		data_ = new HashMap<String, Object>();
	}

	public void setData(String name, Object data)
	{
		data_.put(name, data);
	}

	public Object getData(String name)
	{
		return data_.get(name);
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
