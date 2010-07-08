/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

import java.util.List;

import org.eclipse.jface.wizard.WizardPage;
import org.eclipse.swt.SWT;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Control;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;

import wesnoth_eclipse_plugin.utils.StringUtils;

public class WizardGeneratorPageKey extends WizardPage
{
	private List<TagKey>	keys_;
	private int				startIndex_, endIndex_;
	private Composite		container_;
	private byte			indent_;

	public WizardGeneratorPageKey(String tagName, List<TagKey> keys,
			int startIndex, int endIndex, byte indent) {
		super("wizardPageKey" + startIndex);
		setTitle(tagName + " new wizard");
		//setDescription(String.format("page %d to %d out of %d", startIndex, endIndex, keys.size()));

		indent_ = indent;

		startIndex_ = startIndex;
		endIndex_ = endIndex;
		keys_ = keys;
	}

	@Override
	public void createControl(Composite parent)
	{
		container_ = new Composite(parent, SWT.NULL);
		setControl(container_);
		container_.setLayout(new GridLayout(2, false));

		for (int i = startIndex_; i <= endIndex_; i++)
		{
			TagKey key = keys_.get(i);
			Label label = new Label(container_, SWT.NONE);
			label.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, false, false, 1, 1));
			// add star to required items
			label.setText(key.Name + (key.Cardinality == '1' ? "*" : "") + ":");

			Text textBox = new Text(container_, SWT.BORDER);
			textBox.setLayoutData(new GridData(SWT.FILL, SWT.CENTER, true, false, 1, 1));
			//textBox.setText(key.Regex);
			textBox.setData("name", key.Name);
			textBox.setData("regex", key.Regex);
			textBox.setData("card", key.Cardinality);

			//TODO: check for regex/cardinality
		}
		setPageComplete(true);
	}

	public String getContent()
	{
		StringBuilder result = new StringBuilder();
		for (Control child : container_.getChildren())
		{
			if (!(child instanceof Text))
				continue;

			result.append(StringUtils.multiples("\t", indent_) +
					child.getData("name") + "=" + ((Text) child).getText() + "\n");
		}
		return result.toString();
	}
}
