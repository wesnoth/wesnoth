/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.List;

public class WizardGeneratorPageTag extends WizardPage
{
	private java.util.List<Tag>	tags_;
	private int					startIndex_, endIndex_;
	private Composite			container_;

	public WizardGeneratorPageTag(String tagName, java.util.List<Tag> tags, int startIndex, int endIndex) {
		super("wizardPageTag" + startIndex);
		setTitle(tagName + " new wizard");
		//setDescription(String.format("page %d to %d out of %d", startIndex, endIndex, tags.size()));

		startIndex_ = startIndex;
		endIndex_ = endIndex;
		tags_ = tags;
	}

	@Override
	public void createControl(Composite parent)
	{
		container_ = new Composite(parent, SWT.NULL);
		setControl(container_);
		container_.setLayout(new GridLayout(2, false));

		for (int i = startIndex_; i <= endIndex_; i++)
		{
			Tag tag = tags_.get(i);
			Group tagGroup = new Group(container_, SWT.NONE);
			tagGroup.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
			tagGroup.setText("[" + tag.Name + "]");
			tagGroup.setLayout(new GridLayout(2, false));

			List list = new List(tagGroup, SWT.BORDER);
			GridData gd_list = new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 2);
			gd_list.heightHint = 81;
			gd_list.widthHint = 150;
			list.setLayoutData(gd_list);

			Button btnAdd = new Button(tagGroup, SWT.NONE);
			GridData gd_btnAdd = new GridData(SWT.FILL, SWT.FILL, false, false, 1, 1);
			gd_btnAdd.heightHint = 40;
			btnAdd.setLayoutData(gd_btnAdd);
			btnAdd.setText("Add");

			Button btnRemove = new Button(tagGroup, SWT.NONE);
			btnRemove.setLayoutData(new GridData(SWT.FILL, SWT.FILL, false, false, 1, 1));
			btnRemove.setText("Remove");
		}
		setPageComplete(true);
	}
}
