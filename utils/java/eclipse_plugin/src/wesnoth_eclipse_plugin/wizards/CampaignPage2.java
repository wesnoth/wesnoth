/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.wizards;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

public class CampaignPage2 extends WizardPage {
	private Text txtAbbrev_;
	private Text txtDefine_;
	private Text txtDifficulties_;
	private Text txtFirstScenario_;

	/**
	 * Create the wizard.
	 */
	public CampaignPage2() {
		super("wizardPage");
		setTitle("Campaign details");
		setDescription("Set the campaign details");
	}

	/**
	 * Create contents of the wizard.
	 * @param parent
	 */
	public void createControl(Composite parent) {
		Composite container = new Composite(parent, SWT.NULL);

		setControl(container);
		ModifyListener modifyListener =  new ModifyListener() {
			
			@Override
			public void modifyText(ModifyEvent e) {
				updatePageIsComplete();
			}
		};
		
		Label lblAbbreviation = new Label(container, SWT.NONE);
		lblAbbreviation.setText("Abbreviation* :");
		lblAbbreviation.setBounds(10, 13, 76, 13);
		
		txtAbbrev_ = new Text(container, SWT.BORDER);
		txtAbbrev_.setBounds(92, 10, 179, 19);
		txtAbbrev_.addModifyListener(modifyListener);
		
		Label lblDefine = new Label(container, SWT.NONE);
		lblDefine.setText("Define* :");
		lblDefine.setBounds(10, 35, 49, 13);
		
		txtDefine_ = new Text(container, SWT.BORDER);
		txtDefine_.setBounds(92, 32, 179, 19);
		txtDefine_.addModifyListener(modifyListener);
		
		Label lblDifficulties = new Label(container, SWT.NONE);
		lblDifficulties.setText("Difficulties:");
		lblDifficulties.setBounds(10, 57, 63, 13);
		
		txtDifficulties_ = new Text(container, SWT.BORDER);
		txtDifficulties_.setBounds(92, 54, 179, 19);
		
		Label lblFirstScenario = new Label(container, SWT.NONE);
		lblFirstScenario.setText("First Scenario:");
		lblFirstScenario.setBounds(10, 90, 76, 13);
		
		txtFirstScenario_ = new Text(container, SWT.BORDER);
		txtFirstScenario_.setBounds(92, 87, 179, 19);
		
		updatePageIsComplete();
	}
	public void updatePageIsComplete()
	{
		setPageComplete(false);		
		
		if (txtAbbrev_.getText().length() == 0)
		{
			setErrorMessage("Please specify an abbreviation.");
			return;
		}
		
		if (txtDefine_.getText().length() == 0)
		{
			setErrorMessage("Please specify a define.");
			return;
		}
		
		setErrorMessage(null);
		setPageComplete(true);
	}
	/**
	 * @return returns the abbreviation of the campaign
	 */
	public String getAbbrev() {
		return txtAbbrev_.getText();
	}
	/**
	 * @return returns the define of the campaign
	 */
	public String getDefine() {
		return txtDefine_.getText();
	}
	/**
	 * @return returns the difficulties of the campaign
	 */
	public String getDifficulties() {
		return txtDifficulties_.getText();
	}
	/**
	 * @return returns the first scenario of the campaign
	 */
	public String getFirstScenario() {
		return txtFirstScenario_.getText();
	}
}
