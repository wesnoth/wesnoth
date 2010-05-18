package wesnoth_eclipse_plugin.builder;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;

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
	public ExternalToolInvoker(String fileName, String arguments, boolean useThread) throws IOException
	{
		processBuilder_ = new ProcessBuilder(fileName,arguments);
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
			return bufferedReaderOutput_.readLine();
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
			return bufferedReaderError_.readLine();
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

	public static boolean launchTool(String fileName, String args, boolean showOutput, boolean useThread){
		try{
			ExternalToolInvoker toolInvoker = new ExternalToolInvoker(fileName, args, useThread);
			toolInvoker.run();
			toolInvoker.waitFor();
			if (showOutput)
			{
				String line="";
				System.out.println("Output: ");
				while((line  = toolInvoker.readOutputLine()) != null)
				{
					System.out.println(line);
				}
				System.out.println("Errors: ");
				while((line  = toolInvoker.readErrorLine()) != null)
				{
					System.out.println(line);
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
