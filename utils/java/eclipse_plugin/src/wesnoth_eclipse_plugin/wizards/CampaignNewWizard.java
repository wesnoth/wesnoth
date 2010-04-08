package wesnoth_eclipse_plugin.wizards;

import java.io.ByteArrayInputStream;
import java.io.InputStream;
import java.lang.reflect.InvocationTargetException;

import org.eclipse.core.resources.IFile;
import org.eclipse.core.resources.IFolder;
import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.runtime.CoreException;
import org.eclipse.core.runtime.IConfigurationElement;
import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.core.runtime.IStatus;
import org.eclipse.core.runtime.NullProgressMonitor;
import org.eclipse.core.runtime.Status;
import org.eclipse.jface.dialogs.ErrorDialog;
import org.eclipse.jface.operation.IRunnableWithProgress;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.IWizardPage;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.views.markers.MarkerField;

import wesnoth_eclipse_plugin.Logger;

public class CampaignNewWizard extends Wizard implements INewWizard {

	private org.eclipse.ui.IWorkbench workbench;
	private org.eclipse.jface.viewers.IStructuredSelection selection;

	protected CampaignPage0 page0_;
	protected CampaignPage1 page1_;

	protected int _lastPageHashCode=0;

	private IConfigurationElement fConfig;

	public void addPages() {
		page0_ = new CampaignPage0();
		addPage(page0_);

		page1_ = new CampaignPage1();
		addPage(page1_);

		_lastPageHashCode = page1_.hashCode();
	}

	public CampaignNewWizard() {
		setWindowTitle("Create a new Campaign");
		setNeedsProgressMonitor(true);
		
	}

	@Override
	public void init(IWorkbench workbench, IStructuredSelection selection) {
		this.workbench = workbench;
		this.selection = selection;
	}

	@Override
	public boolean performFinish() {

		try{
			// let's create the project
			getContainer().run(false, false, new IRunnableWithProgress() {
				@Override
				public void run(IProgressMonitor monitor) throws InvocationTargetException,
				InterruptedException {

					monitor.beginTask("Creating the project structure...", 10);

					try {						
						IProject currentProject = page0_.getProjectHandle();
						
						// the project
						currentProject.create(new NullProgressMonitor());						
						monitor.worked(2);
						
						// directory structure
						createFolder(currentProject, "ai");
						createFolder(currentProject, "images");
						createFolder(currentProject, "utils");
						createFolder(currentProject, "maps");
						createFolder(currentProject, "scenarios");
						monitor.worked(6);
						
						// _main.cfg
						createFile(currentProject, "_main.cfg",String.format("# Hello! \n# Campaign name: %s ",page1_.getCampaignName()));
						monitor.worked(2);

					} catch (CoreException e) {
						e.printStackTrace();
					}	

					monitor.done();
				}
			});
		}
		catch (Exception e) {
			return false;
		}
		return true;
	}

	/* (non-Javadoc)
	 * @see org.eclipse.jface.wizard.Wizard#canFinish()
	 */
	public boolean canFinish() {
		IWizardPage page = getContainer().getCurrentPage();
		return super.canFinish() && page.hashCode() == _lastPageHashCode && page.isPageComplete();
	}

	public void createFolder(IProject project,String folderName)
	{
		IFolder folder = project.getFolder(folderName);
		createResource(folder, project, folderName,null);
	}
	public void createFile(IProject project, String fileName, String fileContentsString)
	{
		IFile file = project.getFile(fileName);
		ByteArrayInputStream inputStream  = new ByteArrayInputStream(fileContentsString.getBytes());

		createResource(file, project, fileName,inputStream);
	}
	/**
	 * Creates the desired resource
	 * @param resource the resource to be created (IFile/IFolder)
	 * @param project the project where to be created the resource
	 * @param resourceName the name of the resource
	 * @param input the contents of the resource or null if no contents needed
	 */
	private void createResource(IResource resource, IProject project, String resourceName, InputStream input)
	{
		try{
			if (!project.isOpen())
				project.open(new NullProgressMonitor());

			if (resource.exists())
				return;

			if (resource instanceof IFile)
			{
				((IFile)resource).create(input, true, new NullProgressMonitor());
			}
			else if (resource instanceof IFolder)
			{
				((IFolder)resource).create(true, true,new  NullProgressMonitor());
			}

		}catch (CoreException e) {
			Logger.print("Error creating the resource"+resourceName, IStatus.ERROR);
			ErrorDialog dlgDialog = new ErrorDialog(getShell(), "Error creating the file", "There was an error creating the resource: "+resourceName, 
					new Status(IStatus.ERROR,"wesnoth_plugin","error"),0);
			dlgDialog.open();
			e.printStackTrace();
		}			
	}
}
