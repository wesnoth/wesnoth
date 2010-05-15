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
import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

import wesnoth_eclipse_plugin.Logger;
import wesnoth_eclipse_plugin.StringUtils;
import wesnoth_eclipse_plugin.TemplateProvider;

public class CampaignNewWizard extends Wizard implements INewWizard {

	private org.eclipse.ui.IWorkbench workbench;
	private org.eclipse.jface.viewers.IStructuredSelection selection;

	protected CampaignPage0 page0_;
	protected CampaignPage1 page1_;
	protected CampaignPage2 page2_;

	protected int _lastPageHashCode=0;

	private IConfigurationElement fConfig;

	public void addPages() {
		page0_ = new CampaignPage0();
		addPage(page0_);

		page1_ = new CampaignPage1();
		addPage(page1_);

		page2_ = new CampaignPage2();
		addPage(page2_);

		_lastPageHashCode = getPages()[getPageCount()-1].hashCode();
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
					createProject(monitor);
				}
			});
		}
		catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		return true;
	}
	public void createProject(IProgressMonitor monitor)
	{
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
			createFolder(currentProject, "units");
			monitor.worked(5);

			// _main.cfg
			createFile(currentProject, "_main.cfg",prepareTemplate("campaign"));
			monitor.worked(2);

			// campaign_name.pbl - for uploading the campaign on the webserver
			createFile(currentProject, page1_.getCampaignName()+".pbl",prepareTemplate("pbl"));
		} catch (CoreException e) {
			e.printStackTrace();
		}

		monitor.done();
	}
	public String prepareTemplate(String templateName)
	{
		String tmpTemplate = TemplateProvider.getInstance().getTemplate(templateName);
		if (tmpTemplate == null)
		{
			MessageBox box = new MessageBox(this.getShell());
			box.setMessage(String.format("Template for %s not found.", templateName));
			box.open();
			return "";
		}

		// get the lines
		String[] template = tmpTemplate.split("\\r?\\n");

		replaceParameter(templateName,template,"$$campaign_name", page1_.getCampaignName());
		replaceParameter(templateName,template,"$$author", page1_.getAuthor());
		replaceParameter(templateName,template,"$$version", page1_.getVersion());
		replaceParameter(templateName,template,"$$description", page1_.getDescription());
		replaceParameter(templateName,template,"$$icon", page1_.getIconPath());
		replaceParameter(templateName,template,"$$email", page1_.getEmail());
		replaceParameter(templateName,template,"$$passphrase", page1_.getPassphrase());
		replaceParameter(templateName,template,"$$email", page1_.getEmail());
		replaceParameter(templateName,template,"$$translations_dir", page1_.getTranslationDir());

		replaceParameter(templateName,template,"$$abrev", page2_.getAbbrev());
		replaceParameter(templateName,template,"$$define", page2_.getDefine());
		replaceParameter(templateName,template,"$$difficulties", page2_.getDifficulties());
		replaceParameter(templateName,template,"$$first_scenario", page2_.getFirstScenario());

		replaceParameter(templateName,template,"$$project_name", page0_.getProjectName());
		replaceParameter(templateName,template,"$$type", page1_.isMultiplayer()?"campaign_mp":"campaign");

		tmpTemplate = "";
		for (String line: template) {
			tmpTemplate += (line +"\n");
		}
		return tmpTemplate;
	}
	public void replaceParameter(String templateName, String[] template, String paramName,String paramValue)
	{
		for (int i=0;i<template.length;++i) {
			if (template[i].contains(paramName))
			{
				template[i] = template[i].replace(paramName, paramValue);

				if (paramValue == null || paramValue.length() == 0)
				{
					// we don't have any value supplied -
					// let's comment that line (if it's not already commented)
					if (!(StringUtils.startsWith(template[i],"#")))
						template[i]= "#" + template[i];
				}
			}
		}
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
