package wesnoth_eclipse_plugin.editors;

import org.eclipse.core.runtime.IPath;
import org.eclipse.ui.IEditorLauncher;

import wesnoth_eclipse_plugin.utils.GameUtils;

public class MapFileEditor implements IEditorLauncher
{
	@Override
	public void open(IPath file)
	{
		GameUtils.startEditor(file.toOSString());
	}
}
