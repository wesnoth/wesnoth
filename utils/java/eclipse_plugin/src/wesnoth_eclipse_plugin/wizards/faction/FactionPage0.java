/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package wesnoth_eclipse_plugin.wizards.faction;

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

public class FactionPage0 extends NewWizardPageTemplate
{
	private ISelection	selection_;
	private Text		txtFileName_;
	private Text		txtDirectory_;
	private Text		txtFactionId_;
	private Text		txtFactionName_;
	private Text		txtType_;
	private Text		txtLeader_;
	private Text		txtRandomLeader_;
	private Text		txtTerrainLiked_;
	private Text		text;

	/**
	 * Create the wizard.
	 */
	public FactionPage0(ISelection selection) {
		super("factionPage0");
		setTitle("New faction wizard");
		setDescription("Create a new faction");
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
		Composite container = new Composite(parent, SWT.NULL);

		ModifyListener modifyListener = new ModifyListener() {
			@Override
			public void modifyText(ModifyEvent e)
			{
				updatePageIsComplete();
			}
		};

		setControl(container);
		container.setLayout(new GridLayout(3, false));

		Label label = new Label(container, SWT.NONE);
		GridData gd_label = new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1);
		gd_label.widthHint = 95;
		label.setLayoutData(gd_label);
		label.setText("Directory* :");

		txtDirectory_ = new Text(container, SWT.BORDER);
		txtDirectory_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		txtDirectory_.addModifyListener(modifyListener);

		Button button = new Button(container, SWT.NONE);
		button.setText("Browse...");
		button.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e)
			{
				handleBrowse();
			}
		});

		Label label_4 = new Label(container, SWT.NONE);
		label_4.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		label_4.setText("File name* :");

		txtFileName_ = new Text(container, SWT.BORDER);
		txtFileName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		new Label(container, SWT.NONE);
		txtFileName_.addModifyListener(modifyListener);

		Label lblFactionId = new Label(container, SWT.NONE);
		lblFactionId.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblFactionId.setText("Faction Id*:");

		txtFactionId_ = new Text(container, SWT.BORDER);
		txtFactionId_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);
		txtFactionId_.addModifyListener(modifyListener);

		Label lblName = new Label(container, SWT.NONE);
		lblName.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblName.setText("Faction name*:");

		txtFactionName_ = new Text(container, SWT.BORDER);
		txtFactionName_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);
		txtFactionName_.addModifyListener(modifyListener);

		Label lblType = new Label(container, SWT.NONE);
		lblType.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblType.setText("Type:");

		txtType_ = new Text(container, SWT.BORDER);
		txtType_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);

		Label lblLeader = new Label(container, SWT.NONE);
		lblLeader.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblLeader.setText("Leader:");

		txtLeader_ = new Text(container, SWT.BORDER);
		txtLeader_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);

		Label lblRandomLeaders = new Label(container, SWT.NONE);
		lblRandomLeaders.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblRandomLeaders.setText("Random leader:");

		txtRandomLeader_ = new Text(container, SWT.BORDER);
		txtRandomLeader_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);

		Label lblTerrainLiked = new Label(container, SWT.NONE);
		lblTerrainLiked.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblTerrainLiked.setText("Terrain liked:");

		txtTerrainLiked_ = new Text(container, SWT.BORDER);
		txtTerrainLiked_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);

		Label lblRecruit = new Label(container, SWT.NONE);
		lblRecruit.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
		lblRecruit.setText("Recruit:");

		text = new Text(container, SWT.BORDER);
		text.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
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

		if (getFactionId().isEmpty())
		{
			setErrorMessage("The faction ID cannot be empty.");
			return;
		}

		if (getFactionName().isEmpty())
		{
			setErrorMessage("The faction name cannot be empty.");
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
		setPageComplete(true);
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
		ContainerSelectionDialog dialog =
				new ContainerSelectionDialog(getShell(), ResourcesPlugin.getWorkspace().getRoot(), false,
						"Select a campaign project");
		if (dialog.open() == ContainerSelectionDialog.OK)
		{
			Object[] result = dialog.getResult();
			if (result.length == 1)
			{
				txtDirectory_.setText(((Path) result[0]).toString());
			}
		}
	}

	public String getDirectoryName()
	{
		return txtDirectory_.getText();
	}

	public String getFactionId()
	{
		return txtFactionId_.getText();
	}

	public String getFactionName()
	{
		return txtFactionName_.getText();
	}

	public String getFileName()
	{
		return txtFileName_.getText();
	}

	public String getLeader()
	{
		return txtLeader_.getText();
	}

	public String getTerrainLiked()
	{
		return txtTerrainLiked_.getText();
	}

	public String getRandomLeader()
	{
		return txtRandomLeader_.getText();
	}

	public String getType()
	{
		return txtType_.getText();
	}
}
