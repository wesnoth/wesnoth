package wesnoth_eclipse_plugin.action;

import java.io.File;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.action.IAction;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IObjectActionDelegate;
import org.eclipse.ui.IWorkbenchPart;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.utils.FileUtils;

public class ImportMap implements IObjectActionDelegate
{
	private IProject selectedProject_;
	private IContainer mapsFolder_;

	public ImportMap()
	{
	}

	@Override
	public void run(IAction action)
	{
		Shell shell = Activator.getShell();
		FileDialog mapDialog = new FileDialog(shell,SWT.OPEN);
		mapDialog.setText("Import wesnoth map");
		mapDialog.setFilterExtensions(new String[] {"*.map" });
		String file = mapDialog.open();
		if (file != null && selectedProject_ != null)
		{
			try
			{
				File source = new File(file);
				File target = new File(mapsFolder_.getLocation().toOSString() +
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
				mapsFolder_.refreshLocal(IResource.DEPTH_INFINITE, null);
			}
			catch (Exception e)
			{
				e.printStackTrace();
			}
		}
	}

	@Override
	public void selectionChanged(IAction action, ISelection selection){
		if (selection instanceof IStructuredSelection)
		{
			Object selection2 = ((IStructuredSelection)selection).getFirstElement();
			if (selection2 instanceof IResource) {
				mapsFolder_ = (IContainer)selection2;

				// get the project
				while(selection2 instanceof IContainer &&
						((IContainer)selection2).getParent() != null &&
						!(selection2 instanceof IProject))
					selection2 = ((IContainer) selection2).getParent();

				if (selection2 instanceof IProject)
					selectedProject_ = (IProject)selection2;
			}
		}
	}

	@Override
	public void setActivePart(IAction action, IWorkbenchPart targetPart) {
	}
}
