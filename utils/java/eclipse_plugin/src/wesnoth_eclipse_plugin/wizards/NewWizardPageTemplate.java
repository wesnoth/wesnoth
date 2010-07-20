/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.wizards;

import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.dialogs.ContainerSelectionDialog;

import wesnoth_eclipse_plugin.Activator;

public class NewWizardPageTemplate extends WizardPage
{
	protected NewWizardPageTemplate(String pageName)
	{
		super(pageName);
	}

	@Override
	public void createControl(Composite parent)
	{
		Activator.getDefault().getWorkbench().getHelpSystem().setHelp(getShell(),
				"Wesnoth_Eclipse_Plugin.wizardHelp");
	}

	@Override
	public NewWizardTemplate getWizard()
	{
		return (NewWizardTemplate)super.getWizard();
	}

	/**
	 * Uses the standard container selection dialog to choose the new value for
	 * the project field.
	 */
	public Path handleBrowseContainer()
	{
		ContainerSelectionDialog dialog = new ContainerSelectionDialog(getShell(),
				ResourcesPlugin.getWorkspace().getRoot(), false, "Select a container");
		if (dialog.open() == ContainerSelectionDialog.OK)
		{
			Object[] result = dialog.getResult();
			if (result.length == 1)
			{
				try{
					getWizard().selectionContainer_ =
						ResourcesPlugin.getWorkspace().getRoot().getFolder((Path)result[0]);
				}catch (IllegalArgumentException e) {
					// the path is a project
					getWizard().selectionContainer_ =
						ResourcesPlugin.getWorkspace().getRoot().getProject(result[0].toString());
				}
				return (Path) result[0];
			}
		}
		return null;
	}

	@Override
	public void performHelp()
	{
		//TODO: create all helps in 'contextHelp.xml'
		//PlatformUI.getWorkbench().getHelpSystem().displayHelp(" .wizardHelp");
	}
}
