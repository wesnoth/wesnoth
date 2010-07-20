/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.era;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.ModifyEvent;
import org.eclipse.swt.events.ModifyListener;
import org.eclipse.swt.events.SelectionAdapter;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.dialogs.ContainerSelectionDialog;

import wesnoth_eclipse_plugin.wizards.NewWizardPageTemplate;

public class EraPage0 extends NewWizardPageTemplate
{
	private ISelection	selection_;
	private Text		txtDirectory_;
	private Text		txtFileName_;
	private Text		txtEraID_;
	private Text		txtEraName_;
	private Button		chkRequireEra_;

	/**
	 * Create the wizard.
	 */
	public EraPage0(ISelection selection) {
		super("eraPage0");
		setTitle("New era wizard");
		setDescription("Create a new era");
		selection_ = selection;
	}

	/**
	 * Create contents of the wizard.
	 *
	 * @param parent
	 */
	@Override
	public void createControl(Composite parent)
	{
		ModifyListener modifyListener = new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent e)
			{
				updatePageIsComplete();
			}
		};

		Composite container = new Composite(parent, SWT.NULL);

		setControl(container);
		container.setLayout(new GridLayout(3, false));

		Label lblProject = new Label(container, SWT.NONE);
		GridData gd_lblProject = new GridData(SWT.LEFT, SWT.CENTER, false, false, 1, 1);
		gd_lblProject.widthHint = 99;
		lblProject.setLayoutData(gd_lblProject);
		lblProject.setText("Directory* :");

		txtDirectory_ = new Text(container, SWT.BORDER);
		txtDirectory_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		txtDirectory_.addModifyListener(modifyListener);

		Button btnBrowse = new Button(container, SWT.NONE);
		btnBrowse.setText("Browse...");
		btnBrowse.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				handleBrowse();
			}
		});

		Label lblFileName = new Label(container, SWT.NONE);
		lblFileName.setText("File name* :");

		txtFileName_ = new Text(container, SWT.BORDER);
		txtFileName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);
		txtFileName_.addModifyListener(modifyListener);

		Label lblEraID = new Label(container, SWT.NONE);
		lblEraID.setText("Era Id*:");

		txtEraID_ = new Text(container, SWT.BORDER);
		txtEraID_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);
		txtEraID_.addModifyListener(modifyListener);

		Label lblEraName = new Label(container, SWT.NONE);
		lblEraName.setText("Era name*:");

		txtEraName_ = new Text(container, SWT.BORDER);
		txtEraName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);
		txtEraName_.addModifyListener(modifyListener);

		chkRequireEra_ = new Button(container, SWT.CHECK);
		chkRequireEra_
				.setToolTipText("whether clients are required to have this era installed beforehand to be allowed join a game using this era. Possible values 'yes' (the default) and 'no'. ");
		chkRequireEra_.setText("Require era");
		new Label(container, SWT.NONE);
		new Label(container, SWT.NONE);

		initialize();
		updatePageIsComplete();
	}

	private void updatePageIsComplete()
	{
		IResource container = ResourcesPlugin.getWorkspace().getRoot().findMember(new Path(getDirectoryName()));
		setPageComplete(false);
		String fileName = getFileName();

		if (getDirectoryName().isEmpty())
		{
			setErrorMessage("You need to specify a valid directory path first.");
			return;
		}

		if (container == null || !container.exists() || !(container instanceof IContainer))
		{
			setErrorMessage("The directory must be created first and the selected folder to exist.");
			return;
		}

		if (fileName.isEmpty())
		{
			setErrorMessage("File name must be specified.");
			return;
		}

		if (fileName.replace('\\', '/').indexOf('/', 1) > 0)
		{
			setErrorMessage("File name must be valid.");
			return;
		}

		int dotLoc = fileName.lastIndexOf('.');
		if (dotLoc == -1 || fileName.substring(dotLoc + 1).equalsIgnoreCase("cfg") == false)
		{
			setErrorMessage("File extension must be 'cfg'.");
			return;
		}

		if (getEraID().isEmpty())
		{
			setErrorMessage("The era ID cannot be empty.");
			return;
		}

		if (getEraName().isEmpty())
		{
			setErrorMessage("The era name cannot be empty.");
			return;
		}

		setErrorMessage(null);
		setPageComplete(true);
	}

	/**
	 * Tests if the current workbench selection is a suitable campaign to use.
	 */
	private void initialize()
	{
		if (selection_ != null && selection_.isEmpty() == false && selection_ instanceof IStructuredSelection)
		{
			IStructuredSelection ssel = (IStructuredSelection) selection_;
			if (ssel.size() > 1)
			{
				return;
			}
			Object obj = ssel.getFirstElement();
			if (obj instanceof IResource)
			{
				IContainer container;
				if (obj instanceof IContainer)
				{
					container = (IContainer) obj;
				}
				else
				{
					container = ((IResource) obj).getParent();
				}
				txtDirectory_.setText(container.getFullPath().toString());
			}
		}
	}

	/**
	 * Uses the standard container selection dialog to choose the new value for
	 * the directory field.
	 */
	private void handleBrowse()
	{
		ContainerSelectionDialog dialog = new ContainerSelectionDialog(getShell(), ResourcesPlugin.getWorkspace().getRoot(), false, "Select a campaign project");
		if (dialog.open() == ContainerSelectionDialog.OK)
		{
			Object[] result = dialog.getResult();
			if (result.length == 1)
			{
				txtDirectory_.setText(((Path) result[0]).toString());
			}
		}
	}

	/**
	 * @return true if this requires the era to be installed in user's game
	 */
	public boolean getRequiresEra()
	{
		return chkRequireEra_.getSelection();
	}

	/**
	 * @return the era id
	 */
	public String getEraID()
	{
		return txtEraID_.getText();
	}

	/**
	 * @return the era name
	 */
	public String getEraName()
	{
		return txtEraName_.getText();
	}

	/**
	 * @return the filename containing the era
	 */
	public String getFileName()
	{
		return txtFileName_.getText();
	}

	/**
	 * @return the directory where the file will be placed
	 */
	public String getDirectoryName()
	{
		return txtDirectory_.getText();
	}
}
