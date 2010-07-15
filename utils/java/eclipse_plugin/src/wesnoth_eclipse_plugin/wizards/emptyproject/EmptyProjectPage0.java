package wesnoth_eclipse_plugin.wizards.emptyproject;

import org.eclipse.swt.widgets.Composite;
import org.eclipse.ui.dialogs.WizardNewProjectCreationPage;

public class EmptyProjectPage0 extends WizardNewProjectCreationPage
{
	public EmptyProjectPage0() {
		super("emptyProjectPage0");
	}

	@Override
	public void createControl(Composite parent)
	{
		super.createControl(parent);
		setMessage("Specify the name of the new project.");
	}
}
