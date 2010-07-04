package wesnoth_eclipse_plugin.handlers;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;

import wesnoth_eclipse_plugin.utils.WorkspaceUtils;

public class SetupWorkspaceHandler extends AbstractHandler {

	@Override
	public Object execute(ExecutionEvent event) throws ExecutionException {
		WorkspaceUtils.setupWorkspace();
		return null;
	}

}
