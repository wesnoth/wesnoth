/** 
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.wizards;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.KeyListener;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import wesnoth_eclipse_plugin.Logger;

public class CampaignPage1 extends WizardPage {
	private Text _txtCampaignName;

	/**
	 * Create the wizard.
	 */
	public CampaignPage1() {
		super("wizardPage");
		setTitle("Create New Campaign");
		setDescription("Creates a new campaign and the files structure.");
		setPageComplete(false);
	}

	/**
	 * Create contents of the wizard.
	 * @param parent
	 */
	public void createControl(Composite parent) {
		Composite container = new Composite(parent, SWT.NULL);

		setControl(container);
		
		_txtCampaignName = new Text(container, SWT.BORDER);
		_txtCampaignName.setBounds(112, 7, 179, 21);
		_txtCampaignName.addModifyListener(new ModifyListener() {
			
			@Override
			public void modifyText(ModifyEvent e) {
				updateIsPageComplete();			
			}
		});
		
		Label _lblCampaignName = new Label(container, SWT.NONE);
		_lblCampaignName.setBounds(10, 10, 96, 15);
		_lblCampaignName.setText("Campaign name:");
	}
	/* (non-Javadoc)
	 * @see org.eclipse.jface.wizard.WizardPage#canFlipToNextPage()
	 */
	@Override
	public boolean canFlipToNextPage() {
		return getNextPage() != null ;
	}
	
	public void updateIsPageComplete()
	{
		boolean res = true;
		if (_txtCampaignName.getText().length() == 0)
			res = false;
		
		setPageComplete(res);
	}
	/**
	 * @return the Campaign Name
	 */
	public String getCampaignName() {
		return _txtCampaignName.getText();
	}
}
