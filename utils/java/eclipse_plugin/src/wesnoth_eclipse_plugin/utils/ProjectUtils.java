/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.utils;

import java.io.File;

import org.eclipse.core.resources.IProject;

public class ProjectUtils
{
	public static String getPropertyValue(String fileName, String propertyName)
	{
		String value = "";
		File file = new File(fileName);
		if (!file.exists())
			return null;

		String fileContents = ResourceUtils.getFileContents(file);
		if (fileContents == null)
			return null;
		int index = fileContents.indexOf(propertyName + "=");
		if (index == -1)
			return null;
		index += (propertyName.length() + 1); // jump over the property name characters

		// skipp spaces between the property name and value (if any)
		while(index < fileContents.length() && fileContents.charAt(index) == ' ')
			++index;

		while(index < fileContents.length() && fileContents.charAt(index) != '#' &&
				fileContents.charAt(index) != ' ' &&
				fileContents.charAt(index) != '\r' && fileContents.charAt(index) != '\n')
		{
			value += fileContents.charAt(index);
			++index;
		}

		return value;
	}
	public static String getCampaignID() throws Exception
	{
		if (WorkspaceUtils.getSelectedResource() == null)
		{
			return null;
		}

		IProject project = WorkspaceUtils.getSelectedProject();
		if (project == null)
			project = WorkspaceUtils.getSelectedFile().getProject();
		if (project == null)
			project = WorkspaceUtils.getSelectedFolder().getProject();

		return getPropertyValue(WorkspaceUtils.getPathRelativeToUserDir(project.getFile("_main.cfg")),"id");
	}

	public static String getScenarioID(String fileName)
	{
		return getPropertyValue(fileName,"id");
	}

	public static boolean isCampaignFile(String fileName)
	{
		//TODO: replace this with a better checking
		//TODO: check extension
		String fileContentString = ResourceUtils.getFileContents(new File(fileName));
		return (fileContentString.contains("[campaign]") && fileContentString.contains("[/campaign]"));
	}
	public static boolean isScenarioFile(String fileName)
	{
		//TODO: replace this with a better checking
		//TODO: check extension
		String fileContentString = ResourceUtils.getFileContents(new File(fileName));
		return (fileContentString.contains("[scenario]") && fileContentString.contains("[/scenario]"));
	}
}