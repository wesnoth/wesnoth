package wesnoth_eclipse_plugin.wizards.scenario;

import org.eclipse.core.resources.IContainer;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;
import org.eclipse.core.runtime.Path;
import org.eclipse.jface.dialogs.IDialogPage;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.IStructuredSelection;
import org.eclipse.jface.wizard.WizardPage;
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
import org.eclipse.swt.widgets.Spinner;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.dialogs.ContainerSelectionDialog;

/**
 * The "New" wizard page allows setting the container for the new file as well
 * as the file name. The page will only accept file name without the extension
 * OR with the extension that matches the expected one (cfg).
 */
public class ScenarioPage0 extends WizardPage {
	private Text txtProject_;

	private Text txtFileName_;
	private Text txtScenarioId_;
	private Text txtScenarioName_;

	private ISelection selection;
	private GridData gd_txtProject_;
	private Label lblproject;
	private GridData gd_txtFileName_;
	private GridData gd_txtScenarioId_;
	private GridData gd_txtScenarioName_;
	private Label lblFileName;
	private Label lblScenarioId;
	private Label lblScenarioName;
	private Label lblNextScenarioId;
	private Text txtNextScenarioId_;
	private Label lblNumberOfTurns;
	private Spinner txtTurns_;
	private Label lblMapData;
	private Text txtMapData_;

	/**
	 * Constructor for SampleNewWizardPage.
	 *
	 * @param pageName
	 */
	public ScenarioPage0(ISelection selection) {
		super("wizardPage");
		setTitle("Scenario File");
		setDescription("Create a new scenario file.");
		this.selection = selection;
	}

	/**
	 * @see IDialogPage#createControl(Composite)
	 */
	public void createControl(Composite parent) {
		Composite container = new Composite(parent, SWT.NULL);
		GridLayout layout = new GridLayout();
		container.setLayout(layout);
		layout.numColumns = 3;
		layout.verticalSpacing = 9;

		ModifyListener modifyListener =new ModifyListener() {
			public void modifyText(ModifyEvent e) {
				updatePageIsComplete();
			}
		};

		lblproject = new Label(container, SWT.NULL);
		lblproject.setText("Project* :");
		txtProject_ = new Text(container, SWT.BORDER | SWT.SINGLE);
		gd_txtProject_ = new GridData(GridData.FILL_HORIZONTAL);
		txtProject_.setLayoutData(gd_txtProject_);
		txtProject_.addModifyListener(modifyListener);

		Button btnBrowse_ = new Button(container, SWT.PUSH);
		btnBrowse_.setText("Browse...");
		btnBrowse_.addSelectionListener(new SelectionAdapter() {
			@Override
			public void widgetSelected(SelectionEvent e) {
				handleBrowse();
			}
		});

		lblFileName = new Label(container, SWT.NULL);
		lblFileName.setText("File name* :");
		txtFileName_ = new Text(container, SWT.BORDER | SWT.SINGLE);
		gd_txtFileName_ = new GridData(GridData.FILL_HORIZONTAL);
		txtFileName_.setLayoutData(gd_txtFileName_);
		txtFileName_.addModifyListener(modifyListener);
		new Label(container, SWT.NONE);

		lblScenarioId = new Label(container, SWT.NULL);
		lblScenarioId.setText("Scenario Id* :");
		txtScenarioId_ = new Text(container, SWT.BORDER | SWT.SINGLE);
		gd_txtScenarioId_ = new GridData(GridData.FILL_HORIZONTAL);
		txtScenarioId_.setLayoutData(gd_txtScenarioId_);
		txtScenarioId_.addModifyListener(modifyListener);

		initialize();
		setControl(container);
		new Label(container, SWT.NONE);

		lblScenarioName = new Label(container, SWT.NULL);
		lblScenarioName.setText("Scenario name:");
		txtScenarioName_ = new Text(container, SWT.BORDER | SWT.SINGLE);
		gd_txtScenarioName_ = new GridData(GridData.FILL_HORIZONTAL);
		txtScenarioName_.setLayoutData(gd_txtScenarioName_);
		txtScenarioName_.addModifyListener(modifyListener);
		new Label(container, SWT.NONE);

		lblNextScenarioId = new Label(container, SWT.NONE);
		lblNextScenarioId.setText("Next scenario Id :");

		txtNextScenarioId_ = new Text(container, SWT.BORDER);
		txtNextScenarioId_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);

		lblNumberOfTurns = new Label(container, SWT.NONE);
		lblNumberOfTurns.setText("Number of turns:");

		txtTurns_ = new Spinner(container, SWT.BORDER);
		GridData gd_txtTurns_ = new GridData(SWT.LEFT, SWT.CENTER, false, false, 1, 1);
		gd_txtTurns_.widthHint = 60;
		txtTurns_.setLayoutData(gd_txtTurns_);
		new Label(container, SWT.NONE);

		lblMapData = new Label(container, SWT.NONE);
		lblMapData.setText("Map data:");

		txtMapData_ = new Text(container, SWT.BORDER);
		txtMapData_.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
		new Label(container, SWT.NONE);

		updatePageIsComplete();
	}

	/**
	 * Tests if the current workbench selection is a suitable campaign to use.
	 */
	private void initialize() {
		if (selection != null && selection.isEmpty() == false
				&& selection instanceof IStructuredSelection) {
			IStructuredSelection ssel = (IStructuredSelection) selection;
			if (ssel.size() > 1) {
				return;
			}
			Object obj = ssel.getFirstElement();
			if (obj instanceof IResource) {
				IContainer container;
				if (obj instanceof IContainer) {
					container = (IContainer) obj;
				} else {
					container = ((IResource) obj).getParent();
				}
				txtProject_.setText(container.getFullPath().toString());
			}
		}
	}

	/**
	 * Uses the standard container selection dialog to choose the new value for
	 * the project field.
	 */
	private void handleBrowse() {
		ContainerSelectionDialog dialog = new ContainerSelectionDialog(
				getShell(), ResourcesPlugin.getWorkspace().getRoot(), false,
		"Select a campaign project");
		if (dialog.open() == ContainerSelectionDialog.OK) {
			Object[] result = dialog.getResult();
			if (result.length == 1) {
				txtProject_.setText(((Path) result[0]).toString());
			}
		}
	}

	/**
	 * Checks the mandatory fields and updates the isPageComplete status
	 */
	private void updatePageIsComplete() {
		setPageComplete(false);

		IResource container = ResourcesPlugin.getWorkspace().getRoot().findMember(new Path(getProjectName()));
		String fileName = getFileName();
		String warningMessage = "";

		if (getProjectName().isEmpty()) {
			setErrorMessage("An existing campaign must be specified");
			return;
		}
		if (container == null ||
				(container.getType() & (IResource.PROJECT | IResource.FOLDER)) == 0) {
			setErrorMessage("The campaign must be created first and the selected folder to exist.");
			return;
		}

		if (!container.isAccessible()) {
			setErrorMessage("The campaign project must be writable");
			return;
		}

		if (!getProjectName().contains("scenarios"))
		{
			warningMessage += "The scenario *should* be created in the \"campaign_project/scenarios\" folder.";
		}

		if (fileName.isEmpty()) {
			setErrorMessage("File name must be specified.");
			return;
		}
		if (fileName.replace('\\', '/').indexOf('/', 1) > 0) {
			setErrorMessage("File name must be valid.");
			return;
		}

		int dotLoc = fileName.lastIndexOf('.');
		if (dotLoc != -1) {
			String ext = fileName.substring(dotLoc + 1);
			if (ext.equalsIgnoreCase("cfg") == false) {
				setErrorMessage("File extension must be \"cfg\".");
				return;
			}
		}

		if (txtScenarioId_.getText().isEmpty())
		{
			setErrorMessage("The scenario ID cannot be empty.");
			return;
		}
		setErrorMessage(null);
		setMessage(warningMessage.isEmpty()? null : warningMessage, WARNING);
		setPageComplete(true);
	}

	/**
	 * @return the project this new scenario will belong to
	 */
	public String getProjectName() {
		return txtProject_.getText();
	}
	/**
	 * @return the file name of the scenario
	 */
	public String getFileName() {
		return txtFileName_.getText();
	}
	/**
	 * @return the scenario id
	 */
	public String getScenarioId() {
		return txtScenarioId_.getText();
	}
	/**
	 * @return the scenario name
	 */
	public String getScenarioName() {
		return txtScenarioName_.getText();
	}
	/**
	 * @return the next scenario's id
	 */
	public String getNextScenarioId() {
		return txtNextScenarioId_.getText();
	}
	/**
	 * @return the number of scenario's turns
	 */
	public int getTurnsNumber()	{
		return txtTurns_.getSelection();
	}
	/**
	 * @return the map data for the current scenario
	 */
	public String getMapData() {
		return txtMapData_.getText();
	}
}