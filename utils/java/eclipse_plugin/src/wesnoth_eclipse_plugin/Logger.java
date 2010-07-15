/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin;

import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.Status;

/**
 * @author Timotei Dolean
 *
 */
public class Logger {
	/**
	 * Prints a message to the error log (severity: info)
	 * @param message the message to print
	 */
	public static void print(String message)
	{
		print(message, IStatus.INFO);
	}

	/**
	 * Prints a message to the error log with the specified severity
	 * @param message the message to print
	 * @param severity the severity level from IStatus enum
	 */
	public static void print(String message, int severity)
	{
		Activator.getDefault().getLog().log(new Status(IStatus.INFO,"wesnoth_plugin",message));
	}
}
