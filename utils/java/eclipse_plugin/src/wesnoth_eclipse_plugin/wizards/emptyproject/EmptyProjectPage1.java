/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.emptyproject;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import wesnoth_eclipse_plugin.wizards.NewWizardPageTemplate;

public class EmptyProjectPage1 extends NewWizardPageTemplate
{
	private Text txtTitle_;
	private Text txtVersion_;
	private Text txtTranslationDir_;
	private Text txtAuthor_;
	private Text txtEmail_;
	private Text txtDescription_;
	private Text txtPassphrase_;
	private Text txtIcon_;
	private Text txtType_;


	public EmptyProjectPage1() {
		super("emptyProjectPage1");
		setTitle("Create new empty project");
		setDescription("Specify PBL related information.");
		setPageComplete(false);
	}

	@Override
	public void createControl(Composite parent)
	{
		Composite container = new Composite(parent, SWT.NULL);

		setControl(container);
		ModifyListener updatePageCompleteListener = new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent e)
			{
				updateIsPageComplete();
			}
		};
		container.setLayout(new GridLayout(3, false));

		Label _lblTitle = new Label(container, SWT.NONE);
		_lblTitle.setText("Title:");

		txtTitle_ = new Text(container, SWT.BORDER);
		GridData gd_txtTitle_ = new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1);
		gd_txtTitle_.heightHint = 15;
		txtTitle_.setLayoutData(gd_txtTitle_);
		txtTitle_.addModifyListener(updatePageCompleteListener);
		new Label(container, SWT.NONE);

		Label lblVersion = new Label(container, SWT.NONE);
		lblVersion.setText("Version* :");

		txtVersion_ = new Text(container, SWT.BORDER);
		txtVersion_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		txtVersion_.addModifyListener(updatePageCompleteListener);

		Label lblFormat = new Label(container, SWT.NONE);
		lblFormat.setToolTipText("Displayed to the right of the title, it is just text." +
				" However,\r\nstarting with Wesnoth 1.6, the required format is x.y.z " +
				"\r\nwhere x, y and z are numbers and a value for x greater than 0 \r\n" +
				"implies the campaign is complete and balanced. \r\nTrailing non-numeric" +
				" elements are ok, but nothing should\r\nappear before the numbers. This" +
				" is necessary for the Update \r\nadd-ons button to work correctly.");
		lblFormat.setText("Format: x.y.z");

		Label lblTranslationsDir = new Label(container, SWT.NONE);
		lblTranslationsDir.setText("Translations folder:");

		txtTranslationDir_ = new Text(container, SWT.BORDER);
		txtTranslationDir_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));

		Label lblRelativeToThe = new Label(container, SWT.NONE);
		lblRelativeToThe.setText("Relative to the data folder");

		Label lblAuthor = new Label(container, SWT.NONE);
		lblAuthor.setText("Author:");

		txtAuthor_ = new Text(container, SWT.BORDER);
		txtAuthor_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		new Label(container, SWT.NONE);

		Label lblDescription = new Label(container, SWT.NONE);
		lblDescription.setText("Email:");

		txtEmail_ = new Text(container, SWT.BORDER);
		txtEmail_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		new Label(container, SWT.NONE);

		Label lblDescription_1 = new Label(container, SWT.NONE);
		lblDescription_1.setText("Description:");

		txtDescription_ = new Text(container, SWT.BORDER);
		txtDescription_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		new Label(container, SWT.NONE);

		Label lblType = new Label(container, SWT.NONE);
		lblType.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblType.setText("Type:");

		txtType_ = new Text(container, SWT.BORDER);
		txtType_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);

		Label lblIcon = new Label(container, SWT.NONE);
		lblIcon.setText("Passphrase:");

		txtPassphrase_ = new Text(container, SWT.BORDER);
		txtPassphrase_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		new Label(container, SWT.NONE);

		Label lblIcon_1 = new Label(container, SWT.NONE);
		lblIcon_1.setText("Icon:");

		txtIcon_ = new Text(container, SWT.BORDER);
		GridData gd_txtIcon_ = new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1);
		gd_txtIcon_.widthHint = 163;
		txtIcon_.setLayoutData(gd_txtIcon_);

		Label lblRelativeToThe_1 = new Label(container, SWT.NONE);
		GridData gd_lblRelativeToThe_1 = new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1);
		gd_lblRelativeToThe_1.widthHint = 285;
		lblRelativeToThe_1.setLayoutData(gd_lblRelativeToThe_1);
		lblRelativeToThe_1.setToolTipText("An image, displayed leftmost on the " +
				"\"add-ons\" screen.\r\nIt must be a standard Wesnoth graphic and not a " +
				"custom one. \r\n(Well, a custom graphic will work if the user already " +
				"has the campaign \r\ninstalled, or if it is a custom graphic from a" +
				" different campaign that the \r\nuser has installed but others won't " +
				"see it!) (Note that the icon used to \r\ndisplay your campaign for when" +
				" it is played can be custom; for more\r\ninformation see CampaignWML.) " +
				"If the icon is a unit with magenta color,\r\nplease use " +
				"ImagePathFunctionWML to team-color it. ");
		lblRelativeToThe_1.setText("Relative to the 'data/core/images' directory");

		updateIsPageComplete();
	}

	/**
	 * Checks the mandatory fields and updates the isPageComplete status
	 */
	public void updateIsPageComplete()
	{
		setPageComplete(false);
		if (txtTitle_.getText().isEmpty())
		{
			setErrorMessage("Project name is mandatory");
			return;
		}

		// match the pattern x.y.z
		if (txtVersion_.getText().isEmpty() ||
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
	public String getCampaignName()
	{
		return txtTitle_.getText();
	}

	/**
	 * @return the author
	 */
	public String getAuthor()
	{
		return txtAuthor_.getText();
	}

	/**
	 * @return the version
	 */
	public String getVersion()
	{
		return txtVersion_.getText();
	}

	/**
	 * @return the description
	 */
	public String getPBLDescription()
	{
		return txtDescription_.getText();
	}

	/**
	 * @return the Icon
	 */
	public String getIconPath()
	{
		return txtIcon_.getText();
	}

	/**
	 * @return the email
	 */
	public String getEmail()
	{
		return txtEmail_.getText();
	}

	/**
	 * @return the passphrase
	 */
	public String getPassphrase()
	{
		return txtPassphrase_.getText();
	}

	/**
	 * @return the translation directory
	 */
	public String getTranslationDir()
	{
		return txtTranslationDir_.getText();
	}

	public String getType()
	{
		return txtType_.getText();
	}
}
