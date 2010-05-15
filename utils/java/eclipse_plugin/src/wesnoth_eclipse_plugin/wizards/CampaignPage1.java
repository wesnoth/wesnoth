/** 
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.wizards;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import wesnoth_eclipse_plugin.StringUtils;

public class CampaignPage1 extends WizardPage {
	private Text txtCampaignName_;
	private Text txtVersion_;
	private Text txtTranslationDir_;
	private Text txtAuthor_;
	private Text txtEmail_;
	private Text txtDescription_;
	private Text txtPassphrase_;
	private Text txtIcon_;
	private Button chkMultiCampaign_;

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
		ModifyListener updatePageCompleteListener = new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent e) {
				updateIsPageComplete();			
			}
		};

		txtCampaignName_ = new Text(container, SWT.BORDER);
		txtCampaignName_.setBounds(122, 7, 206, 21);
		txtCampaignName_.addModifyListener(updatePageCompleteListener);
		
		Label _lblCampaignName = new Label(container, SWT.NONE);
		_lblCampaignName.setBounds(10, 10, 96, 15);
		_lblCampaignName.setText("Campaign name* :");

		Label lblVersion = new Label(container, SWT.NONE);
		lblVersion.setText("Version* :");
		lblVersion.setBounds(10, 37, 96, 15);

		txtVersion_ = new Text(container, SWT.BORDER);
		txtVersion_.setBounds(122, 34, 206, 21);
		txtVersion_.addModifyListener(updatePageCompleteListener);

		Label lblTranslationsDir = new Label(container, SWT.NONE);
		lblTranslationsDir.setText("Translations folder:");
		lblTranslationsDir.setBounds(10, 64, 106, 15);

		txtTranslationDir_ = new Text(container, SWT.BORDER);
		txtTranslationDir_.setBounds(122, 61, 206, 21);

		Label lblRelativeToThe = new Label(container, SWT.NONE);
		lblRelativeToThe.setBounds(334, 64, 174, 15);
		lblRelativeToThe.setText("Relative to the data folder");

		txtAuthor_ = new Text(container, SWT.BORDER);
		txtAuthor_.setBounds(122, 121, 206, 21);

		Label lblAuthor = new Label(container, SWT.NONE);
		lblAuthor.setText("Author:");
		lblAuthor.setBounds(10, 124, 96, 15);

		txtEmail_ = new Text(container, SWT.BORDER);
		txtEmail_.setBounds(122, 148, 206, 21);

		Label lblDescription = new Label(container, SWT.NONE);
		lblDescription.setText("Email:");
		lblDescription.setBounds(10, 151, 96, 15);

		txtDescription_ = new Text(container, SWT.BORDER);
		txtDescription_.setBounds(122, 175, 206, 21);

		Label lblDescription_1 = new Label(container, SWT.NONE);
		lblDescription_1.setText("Description:");
		lblDescription_1.setBounds(10, 178, 96, 15);

		txtPassphrase_ = new Text(container, SWT.BORDER);
		txtPassphrase_.setBounds(122, 202, 206, 21);

		Label lblIcon = new Label(container, SWT.NONE);
		lblIcon.setText("Passphrase:");
		lblIcon.setBounds(10, 205, 96, 15);

		Label lblFormat = new Label(container, SWT.NONE);
		lblFormat.setToolTipText("Displayed to the right of the title, it is just text. However,\r\nstarting with Wesnoth 1.6, the required format is x.y.z \r\nwhere x, y and z are numbers and a value for x greater than 0 \r\nimplies the campaign is complete and balanced. \r\nTrailing non-numeric elements are ok, but nothing should\r\nappear before the numbers. This is necessary for the Update \r\nadd-ons button to work correctly.");
		lblFormat.setBounds(334, 37, 72, 15);
		lblFormat.setText("Format: x.y.z");

		Label lblIcon_1 = new Label(container, SWT.NONE);
		lblIcon_1.setText("Icon:");
		lblIcon_1.setBounds(10, 232, 96, 15);

		txtIcon_ = new Text(container, SWT.BORDER);
		txtIcon_.setBounds(122, 229, 206, 21);

		Label lblRelativeToThe_1 = new Label(container, SWT.NONE);
		lblRelativeToThe_1.setToolTipText("An image, displayed leftmost on the \"download campaigns\" screen.\r\nIt must be a standard Wesnoth graphic and not a custom one. \r\n(Well, a custom graphic will work if the user already has the campaign \r\ninstalled, or if it is a custom graphic from a different campaign that the \r\nuser has installed but others won't see it!) (Note that the icon used to \r\ndisplay your campaign for when it is played can be custom; for more\r\ninformation see CampaignWML.) If the icon is a unit with magenta color,\r\nplease use ImagePathFunctionWML to team-color it. ");
		lblRelativeToThe_1.setBounds(334, 232, 230, 15);
		lblRelativeToThe_1.setText("Relative to the data/core/images folder");

		chkMultiCampaign_ = new Button(container, SWT.CHECK);
		chkMultiCampaign_.setBounds(10, 86, 181, 16);
		chkMultiCampaign_.setText("This is a multiplayer campaign");

		updateIsPageComplete();
	}
	/* (non-Javadoc)
	 * @see org.eclipse.jface.wizard.WizardPage#canFlipToNextPage()
	 */
	@Override
	public boolean canFlipToNextPage() {
		return (isPageComplete() && getNextPage() != null);
	}
	
	public void updateIsPageComplete()
	{
		setPageComplete(false);
		if (txtCampaignName_.getText().length() == 0)
		{
			setErrorMessage("Campaign name is mandatory");
			return;
		}
		
		// match the pattern x.y.z
		if (txtVersion_.getText().length() == 0 ||
			!(txtVersion_.getText().matches("[\\d]+\\.[\\d]+\\.\\d[\\w\\W\\d\\D\\s\\S]*")))
		{
			setErrorMessage("The version must have the format: x.y.z");
			return;
		}

		setErrorMessage(null);
		setPageComplete(true);
	}
	/**
	 * @return the Campaign Name
	 */
	public String getCampaignName() {
		return txtCampaignName_.getText();
	}
	/**
	 * @return the author
	 */
	public String getAuthor() {
		return txtAuthor_.getText();
	}
	/**
	 * @return the version
	 */
	public String getVersion() {
		return txtVersion_.getText();
	}
	/**
	 * @return the description
	 */
	public String getDescription() {
		return txtDescription_.getText();
	}
	/**
	 * @return the Icon
	 */
	public String getIconPath() {
		return txtIcon_.getText();
	}
	/**
	 * @return the email
	 */
	public String getEmail() {
		return txtEmail_.getText();
	}
	/**
	 * @return the passphrase
	 */
	public String getPassphrase() {
		return txtPassphrase_.getText();
	}
	/**
	 * @return the translation directory
	 */
	public String getTranslationDir() {
		return txtTranslationDir_.getText();
	}
	/**
	 * @return true if the campaign is multiplayer
	 */
	public boolean isMultiplayer(){
		return chkMultiCampaign_.getSelection();
	}
}
