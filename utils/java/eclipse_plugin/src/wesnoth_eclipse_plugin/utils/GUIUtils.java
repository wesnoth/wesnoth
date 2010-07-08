/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.utils;

import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.ui.IWorkbenchWindow;

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
	public static void showMessageBox(final IWorkbenchWindow window, final String message, final int style)
	{
		if (window == null || window.getShell() == null)
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
			e.printStackTrace();
		}
	}
}
