package wesnoth_eclipse_plugin.preferences;

import java.util.HashMap;
import java.util.Map.Entry;

import org.eclipse.jface.preference.BooleanFieldEditor;
import org.eclipse.jface.preference.FieldEditorPreferencePage;
import org.eclipse.jface.preference.StringFieldEditor;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPreferencePage;
import org.eclipse.xtext.ui.editor.preferences.fields.LabelFieldEditor;

import wesnoth_eclipse_plugin.Activator;
import wesnoth_eclipse_plugin.Constants;
import wesnoth_eclipse_plugin.jface.RegexStringFieldEditor;

public class AddonUploadPreferencePage extends FieldEditorPreferencePage implements
		IWorkbenchPreferencePage
{
	HashMap<String, String> ports_;

	public AddonUploadPreferencePage()
	{
		super(GRID);
		ports_ = new HashMap<String, String>();
		ports_.put("15002", "1.9.x");
		ports_.put("15001", "1.8.x");
		ports_.put("15003", "1.6.x");
		ports_.put("15005", "1.4.x");
		ports_.put("15004", "trunk");

		setPreferenceStore(Activator.getDefault().getPreferenceStore());
		setDescription("Addon uploader preferences");
	}

	@Override
	protected void createFieldEditors()
	{
		addField(new StringFieldEditor(Constants.P_WAU_PASSWORD,
				"Password (stored in plain text)", getFieldEditorParent()));
		addField(new BooleanFieldEditor(Constants.P_WAU_VERBOSE,
				"Verbose", 1, getFieldEditorParent()));

		addField(new RegexStringFieldEditor(Constants.P_WAU_ADDRESS,
				"Address", "[^:]*",
				"The address must not contain any port number as its set below.",
				getFieldEditorParent()));

		StringBuilder ports = new StringBuilder();
		StringBuilder portsRegex = new StringBuilder();
		portsRegex.append("(");
		for (Entry<String, String> item : ports_.entrySet())
		{
			portsRegex.append(item.getKey() + "|");
			ports.append(String.format("\t%s - %s\n", item.getKey(), item.getValue()));
		}
		portsRegex.deleteCharAt(portsRegex.length() - 1);
		portsRegex.append(")*");

		System.out.println(portsRegex.toString());
		addField(new RegexStringFieldEditor(Constants.P_WAU_PORT,
				"Port", portsRegex.toString(),
				"Invalid port number", getFieldEditorParent()));
		addField(new LabelFieldEditor("Ports available by wesnoth's version:\n" +
				ports.toString(), getFieldEditorParent()));
	}

	@Override
	public void init(IWorkbench workbench)
	{
	}
}
