/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards;

import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.IWizardPage;
import org.eclipse.jface.wizard.Wizard;
import org.eclipse.ui.INewWizard;
import org.eclipse.ui.IWorkbench;

public abstract class NewWizardTemplate extends Wizard implements INewWizard
{
	protected IStructuredSelection	selection_;
	protected int					lastPageHashCode_	= 0;

	@Override
	public void init(IWorkbench workbench, IStructuredSelection selection)
	{
		this.selection_ = selection;
	}

	@Override
	public boolean canFinish()
	{
		IWizardPage page = getContainer().getCurrentPage();
		return super.canFinish() && page.hashCode() == lastPageHashCode_ && page.isPageComplete();
	}

	@Override
	public void addPages()
	{
		lastPageHashCode_ = getPages()[getPageCount() - 1].hashCode();
	}
}
