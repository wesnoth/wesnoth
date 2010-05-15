/**
 *
 */
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
