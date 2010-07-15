/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.utils;

public class MyRunnable<T> implements Runnable
{
	protected final T	runnableObject_;

	public MyRunnable(T t) {
		this.runnableObject_ = t;
	}

	@Override
	public void run()
	{
	}
}
