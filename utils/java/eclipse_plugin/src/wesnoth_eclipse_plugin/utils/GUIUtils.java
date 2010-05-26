/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.utils;

import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.ui.IWorkbenchWindow;

public class GUIUtils
{
	/**
	 * Shows a message box with the specified message (thread-safe)
	 * @param window the window where to show the message box
	 * @param message the message to print
	 */
	public static void showMessageBox(final IWorkbenchWindow window,final String message)
	{
		try
		{
			window.getShell().getDisplay().asyncExec(new Runnable() {
				@Override
				public void run()
				{
					MessageBox box = new MessageBox(window.getShell());
					box.setMessage(message);
					box.open();
				}
			});
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
