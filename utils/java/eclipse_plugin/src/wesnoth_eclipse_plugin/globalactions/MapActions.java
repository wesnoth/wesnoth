/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.globalactions;

import java.io.File;

import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.Path;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IWorkbenchWindow;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.utils.FileUtils;
import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class MapActions
{
	/**
	 * Import a map file into the current directory
	 */
	public static void importMap(){
		IWorkbenchWindow window = Activator.getDefault().getWorkbench().getActiveWorkbenchWindow();
		Shell shell = Activator.getShell();
		FileDialog mapDialog = new FileDialog(shell,SWT.OPEN);
		mapDialog.setText("Import wesnoth map");
		mapDialog.setFilterExtensions(new String[] {"*.map" });
		String file = mapDialog.open();

		if (file != null && WorkspaceUtils.getSelectedFolder(window) != null)
		{
			try
			{
				File source = new File(file);
				File target = new File(WorkspaceUtils.getSelectedFolder(window).getLocation().toOSString() +
						Path.SEPARATOR + source.getName());

				if (target.exists())
				{
					MessageBox confirmBox =new MessageBox(shell,
							SWT.ICON_QUESTION | SWT.YES | SWT.NO);
					confirmBox.setMessage("There is already an existing map with the same name. Overwrite?");
					if (confirmBox.open() == SWT.NO)
						return;
				}

				FileUtils.copyTo(source, target);
				WorkspaceUtils.getSelectedFolder(window).refreshLocal(IResource.DEPTH_INFINITE, null);
			}
			catch (Exception e)
			{
				e.printStackTrace();
			}
		}
	}
}
