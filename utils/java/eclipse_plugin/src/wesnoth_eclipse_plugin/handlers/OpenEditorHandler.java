package wesnoth_eclipse_plugin.handlers;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;

import wesnoth_eclipse_plugin.globalactions.EditorActions;

public class OpenEditorHandler extends AbstractHandler
{
	@Override
	public Object execute(ExecutionEvent event) throws ExecutionException
	{
		// no map selected
		EditorActions.startEditor("");
		return null;
	}
}
