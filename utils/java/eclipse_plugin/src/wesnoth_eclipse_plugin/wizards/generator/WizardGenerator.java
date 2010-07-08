/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

import java.util.List;

import wesnoth_eclipse_plugin.wizards.NewWizardTemplate;
import wesnoth_eclipse_plugin.wizards.WizardsConstants;

public class WizardGenerator extends NewWizardTemplate
{
	List<WizardGeneratorPageKey>	pagesList_;
	private String					tagName_;

	public WizardGenerator(String title, String tagName) {
		SchemaParser.getInstance().parseSchema(false);
		setWindowTitle(title);
		Tag tagContent = SchemaParser.getInstance().getTags().get(tagName);
		tagName_ = tagName;

		// keys section
		int keysNr = tagContent.KeyChildren.size();
		int startKey = 0, pgsKey = (keysNr / WizardsConstants.MaxTextBoxesOnPage);
		WizardGeneratorPageKey tempPageKey;
		for (int i = 0; i < pgsKey; i++)
		{
			tempPageKey = new WizardGeneratorPageKey(tagName, tagContent.KeyChildren, startKey,
												startKey + WizardsConstants.MaxTextBoxesOnPage);
			startKey += WizardsConstants.MaxTextBoxesOnPage;
			addPage(tempPageKey);
		}
		if (keysNr - 1 > 0)
		{
			tempPageKey = new WizardGeneratorPageKey(tagName, tagContent.KeyChildren, startKey, keysNr - 1);
			addPage(tempPageKey);
		}

		// tags section
		int tagsNr = tagContent.TagChildren.size();
		int startTag = 0, pgsTag = (tagsNr / WizardsConstants.MaxGroupsOnPage);
		WizardGeneratorPageTag tempPageTag;
		for (int i = 0; i < pgsTag; i++)
		{
			tempPageTag = new WizardGeneratorPageTag(tagName, tagContent.TagChildren, startTag,
					startTag + WizardsConstants.MaxGroupsOnPage);
			startTag += WizardsConstants.MaxTextBoxesOnPage;
			addPage(tempPageTag);
		}
		if (tagsNr - 1 > 0)
		{
			tempPageTag = new WizardGeneratorPageTag(tagName, tagContent.TagChildren, startTag, tagsNr - 1);
			addPage(tempPageTag);
		}

		if (getPageCount() == 0)
		{
			addPage(new WizardGeneratorPage404(tagName));
		}
	}

	@Override
	public void addPages()
	{
		super.addPages();
	}

	@Override
	public boolean performFinish()
	{
		// logic

		data_ = "temp";

		// for now let's just return tag's name
		objectName_ = tagName_;
		isFinished_ = true;
		return true;
	}

}
