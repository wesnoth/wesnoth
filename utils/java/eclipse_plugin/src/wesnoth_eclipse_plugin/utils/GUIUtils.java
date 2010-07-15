/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.utils;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.ui.IWorkbenchWindow;

import wesnoth_eclipse_plugin.Logger;

public class GUIUtils
{
	/**
	 * Shows a message box with the specified message (thread-safe)
	 *
	 * @param window the window where to show the message box
	 * @param message the message to print
	 */
	public static void showMessageBox(final String message)
	{
		showMessageBox(WorkspaceUtils.getWorkbenchWindow(), message, SWT.DEFAULT);
	}

	/**
	 * Shows a message box with the specified message and style(thread-safe)
	 *
	 * @param window the window where to show the message box
	 * @param style the style of the messageBox
	 */
	public static void showMessageBox(final String message, final int style)
	{
		showMessageBox(WorkspaceUtils.getWorkbenchWindow(), message, style);
	}

	/**
	 * Shows a message box with the specified message (thread-safe)
	 *
	 * @param window the window where to show the message box
	 * @param message the message to print
	 */
	public static void showMessageBox(final IWorkbenchWindow window, final String message)
	{
		showMessageBox(window, message, SWT.DEFAULT);
	}

	/**
	 * Shows a message box with the specified message (thread-safe)
	 *
	 * @param window the window where to show the message box
	 * @param message the message to print
	 */
	public static void showMessageBox(final IWorkbenchWindow window,
				final String message, final int style)
	{
		if (window == null || window.getShell() == null || message == null)
			return;
		try
		{
			window.getShell().getDisplay().asyncExec(new Runnable() {
				@Override
				public void run()
				{
					MessageBox box = new MessageBox(window.getShell(), style);
					box.setMessage(message);
					box.open();
				}
			});
		} catch (Exception e)
		{
			Logger.getInstance().logException(e);
		}
	}
}
