package wesnoth_eclipse_plugin.builder;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

/**
 * @author Timotei Dolean
 *
 */
public class ExternalToolInvoker {

	private Process process_;
	private ProcessBuilder processBuilder_;
	private Thread processThread_;
	private BufferedReader bufferedReaderOutput_;
	private BufferedReader bufferedReaderError_;

	/**
	 * Creates an external tool invoker with specified options
	 * @param fileName the file name to be invoked
	 * @param arguments the arguments passed to the file
	 * @param useThread true if the process will run in a thread
	 * @throws IOException
	 */
	public ExternalToolInvoker(String fileName, Collection<String> arguments, boolean useThread) throws IOException
	{
		List<String> commandline = new ArrayList<String>();
		commandline.add(fileName);
		commandline.addAll(arguments);

		processBuilder_ = new ProcessBuilder(commandline);
		if (useThread)
			processThread_ = new Thread(new Runnable() {

				@Override
				public void run() {
					try {
						process_ = processBuilder_.start();
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			});
	}

	public void run() throws IOException
	{
		if (processThread_ == null)
			process_ = processBuilder_.start();
		else
			processThread_.run();

		bufferedReaderOutput_ = new BufferedReader(new InputStreamReader(process_.getInputStream()));
		bufferedReaderError_ = new BufferedReader(new InputStreamReader(process_.getErrorStream()));
	}
	public void waitFor() throws InterruptedException
	{
		process_.waitFor();
	}
	public String readOutputLine()
	{
		if (process_ == null)
			return null;

		try {
			if (bufferedReaderOutput_.ready())
				return bufferedReaderOutput_.readLine();
			else
				return null;
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}
	}
	public String readErrorLine()
	{
		if (process_ == null)
			return null;

		try {
			if (bufferedReaderError_.ready())
				return bufferedReaderError_.readLine();
			else
				return null;
		} catch (IOException e) {
			e.printStackTrace();
			return null;
		}
	}
	public OutputStream getOutputStream()
	{
		return process_.getOutputStream();
	}
	public InputStream getInputStream()
	{
		return process_.getInputStream();
	}
	public InputStream getErrorStream()
	{
		return process_.getErrorStream();
	}

	public boolean processEnded()
	{
		try{
			process_.exitValue();
		}
		catch (IllegalThreadStateException e) {
			// the process hasn't exited
			return false;
		}
		return true;
	}

	/**
	 * Launches the specified tool, with the specified argument list
	 * @param fileName the full path to the executable to be launched
	 * @param args the arguments list
	 * @param showOutput true to show tool's ouput (stdout and stderr)
	 * @param waitFor true to wait till the program ends and show the output
	 * at the end of the program or false to show it as it arrises
	 * @param useThread true to launch the tool on a separate thread
	 * @return
	 */
	public static boolean launchTool(String fileName, Collection<String> args, boolean showOutput,
			boolean waitFor,boolean useThread)
	{
		try{
			final ExternalToolInvoker toolInvoker = new ExternalToolInvoker(fileName, args, useThread);
			toolInvoker.run();
			if (waitFor)
				toolInvoker.waitFor();

			if (showOutput)
			{
				if (waitFor)
				{
					String line="";
					while((line  = toolInvoker.readOutputLine()) != null)
					{
						System.out.println(line);
					}
					while((line  = toolInvoker.readErrorLine()) != null)
					{
						System.out.println(line);
					}
				}
				else {
					// we need a new thread so we won't block the caller
					Thread thread = new Thread(new Runnable() {
						@Override
						public void run()
						{
							String line="";
							while(!toolInvoker.processEnded())
							{
								line = toolInvoker.readOutputLine();
								if (line!= null)
									System.out.println(line);

								line = toolInvoker.readErrorLine();
								if (line!= null)
									System.out.println(line);

								try{
									Thread.sleep(10);
								}
								catch (InterruptedException e){
									e.printStackTrace();
								}
							}

							System.out.println("tool exited.");
						}
					});
					thread.start();
				}
			}
		}
		catch (Exception e) {
			e.printStackTrace();
			return false;
		}
		return true;
	}
}
