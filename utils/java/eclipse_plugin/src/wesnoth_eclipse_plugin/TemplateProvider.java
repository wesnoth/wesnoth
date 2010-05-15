/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileReader;
import java.util.HashMap;


public class TemplateProvider {
	private static TemplateProvider instance_;
	private HashMap<String, String> templates_ = new HashMap<String, String>();
	
	public static TemplateProvider getInstance(){
		if (instance_ == null)
			instance_ = new TemplateProvider();
		return instance_;
	}
	
	public final String templatesFile = "templatesIndex.txt";
	
	private final String pluginFullPath_  = getClass().getProtectionDomain().getCodeSource().getLocation().getPath();
	
	/** Loads the templates from the file system
	 */
	public void loadTemplates()
	{
		try {
			Logger.print("reading templates...");
			
			Logger.print(pluginFullPath_+ templatesFile);
			BufferedReader reader = new BufferedReader(
					new FileReader(getClass().getProtectionDomain().getCodeSource().getLocation().getPath()+
							templatesFile));
			BufferedReader tmpReader;
			String line,tmpLine,content;
			
			// read the main "templatesIndex.txt" file
			while((line = reader.readLine()) != null) 
			{
				// comment
				if (line.startsWith("#"))
					continue;
				
				// 0 - template name | 1 - template file
				String[] tokensStrings = line.split(" ");
				
				if (tokensStrings.length != 2)
					continue;

				content ="";
				
				if (new File(pluginFullPath_+tokensStrings[1]).exists())
				{
					tmpReader = new BufferedReader(new FileReader(pluginFullPath_+tokensStrings[1]));
					while((tmpLine = tmpReader.readLine())!=null)
					{
						content += tmpLine + '\n';
					}
					templates_.put(tokensStrings[0],content);
					System.out.println(String.format("read %s with content: %s\n",tokensStrings[0],content));
					tmpReader.close();
				}
			}
			reader.close();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	public String getTemplate(String name)
	{
		if (templates_.get(name) == null)
			return "";
		return templates_.get(name);
	}
}
