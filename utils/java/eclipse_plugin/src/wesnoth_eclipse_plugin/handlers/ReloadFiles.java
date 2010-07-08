/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.handlers;

import org.eclipse.core.commands.AbstractHandler;
import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;

import wesnoth_eclipse_plugin.wizards.TemplateProvider;
import wesnoth_eclipse_plugin.wizards.generator.SchemaParser;

public class ReloadFiles extends AbstractHandler
{
	@Override
	public Object execute(ExecutionEvent event) throws ExecutionException
	{
		SchemaParser.getInstance().parseSchema(true);
		TemplateProvider.getInstance().loadTemplates();
		return null;
	}
}
