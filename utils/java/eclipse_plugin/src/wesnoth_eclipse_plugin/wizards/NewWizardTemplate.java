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
	protected boolean				isFinished_			= false;
	protected Object				data_				= null;
	protected String				objectName_			= "";

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

	public boolean isFinished()
	{
		return isFinished_;
	}

	/**
	 * Gets the data associated with this wizard
	 */
	public Object getData()
	{
		return data_;
	}

	/**
	 * Sets the data associated with this wizard
	 */
	public void setData(Object data)
	{
		data_ = data;
	}

	/**
	 * Gets the name of the object created by the wizard
	 * 
	 * @return
	 */
	public String getObjectName()
	{
		return objectName_;
	}
}
