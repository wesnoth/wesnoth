/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin.utils;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.PrintStream;
import java.util.HashMap;
import java.util.Iterator;

import org.apache.tools.ant.BuildLogger;
import org.apache.tools.ant.DefaultLogger;
import org.apache.tools.ant.Project;
import org.apache.tools.ant.ProjectHelper;

public class AntUtils
{
	/**
	 * Runs the specified ant file, and returns the output of the runned file
	 * @param antFile
	 * @param properties the hasmap with userproperties to be added to the ant file
	 * @return
	 */
	public static String runAnt(String antFile, HashMap<String,String> properties)
    {
         final Project project = new Project();
         ByteArrayOutputStream out=null;

         try
         {
              out = new ByteArrayOutputStream();
              project.addBuildListener(AntUtils.createLogger(out));
              project.init();
              File buildFile = new File(antFile);
              ProjectHelper.configureProject(project, buildFile);

              Iterator<String> iterator = properties.keySet().iterator();
              while (iterator.hasNext())
              {
                   String key = iterator.next();
                   String value = properties.get(key);
                   project.setUserProperty(key,value);
              }
              project.executeTarget(project.getDefaultTarget());
         }
         catch (RuntimeException exc)
         {
              exc.printStackTrace();
         }

         return out.toString();
    }

	/**
	 *  Creates the default build logger for sending build events to the ant log.
	 */
     private static BuildLogger createLogger( ByteArrayOutputStream out )
     {
          DefaultLogger logger = new DefaultLogger();

          logger.setMessageOutputLevel(Project.MSG_INFO);
          logger.setOutputPrintStream(new PrintStream(out));
          logger.setErrorPrintStream(new PrintStream(out));
          logger.setEmacsMode(false);

          return logger;
     }
}
