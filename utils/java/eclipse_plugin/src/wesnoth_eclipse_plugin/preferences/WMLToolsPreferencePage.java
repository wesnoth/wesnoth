package wesnoth_eclipse_plugin.preferences;

import org.eclipse.jface.preference.BooleanFieldEditor;
import org.eclipse.jface.preference.FieldEditorPreferencePage;
import org.eclipse.jface.preference.ScaleFieldEditor;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPreferencePage;
import org.eclipse.xtext.ui.editor.preferences.fields.LabelFieldEditor;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Constants;

public class WMLToolsPreferencePage extends FieldEditorPreferencePage implements
		IWorkbenchPreferencePage
{
	public WMLToolsPreferencePage()
	{
		super(GRID);

		setPreferenceStore(Activator.getDefault().getPreferenceStore());
		setDescription("WML Tools preferences");
	}

	@Override
	protected void createFieldEditors()
	{
		addField(new LabelFieldEditor("WMLINDENT:", getFieldEditorParent()));
		addField(new BooleanFieldEditor(Constants.P_WMLINDENT_VERBOSE, "Verbose Output", 1,
				getFieldEditorParent()));
		addField(new LabelFieldEditor("", getFieldEditorParent()));

		addField(new LabelFieldEditor("WMLSCOPE:", getFieldEditorParent()));
		addField(new ScaleFieldEditor(Constants.P_WMLSCOPE_VERBOSE_LEVEL, "Verbose level:",
				getFieldEditorParent(), 0, 2, 1, 1));
		addField(new BooleanFieldEditor(Constants.P_WMLSCOPE_COLLISIONS, "Report collisions", 1,
				getFieldEditorParent()));
		addField(new LabelFieldEditor("", getFieldEditorParent()));

		addField(new LabelFieldEditor("WMLLINT:", getFieldEditorParent()));
		addField(new BooleanFieldEditor(Constants.P_WMLLINT_DRYRUN, "Dry run", 1,
				getFieldEditorParent()));
		addField(new BooleanFieldEditor(Constants.P_WMLLINT_SPELL_CHECK, "Spell check", 1,
				getFieldEditorParent()));
		addField(new ScaleFieldEditor(Constants.P_WMLLINT_VERBOSE_LEVEL, "Verbose level:",
				getFieldEditorParent(), 0, 3, 1, 1));
		addField(new LabelFieldEditor("", getFieldEditorParent()));
	}

	@Override
	public void init(IWorkbench workbench)
	{
	}
}
