	package wesnoth_eclipse_plugin.builder;
import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.eclipse.swt.widgets.MessageBox;
import org.eclipse.ui.IWorkbenchWindow;

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

						bufferedReaderOutput_ = new BufferedReader(new InputStreamReader(process_.getInputStream()));
						bufferedReaderError_ = new BufferedReader(new InputStreamReader(process_.getErrorStream()));
					} catch (IOException e) {
						e.printStackTrace();
					}
				}
			});
	}

	public void run() throws IOException
	{
		if (processThread_ == null)
		{
			process_ = processBuilder_.start();

			bufferedReaderOutput_ = new BufferedReader(new InputStreamReader(process_.getInputStream()));
			bufferedReaderError_ = new BufferedReader(new InputStreamReader(process_.getErrorStream()));
		}
		else
			processThread_.start();
	}
	/**
	 * Waits for the current tool, and returns the return value
	 * @return the return value of the tool
	 */
	public int waitFor()
	{
		if (process_ == null)
			return 0;

		try{
			return process_.waitFor();
		}
		catch (Exception e) {
			e.printStackTrace();
			return -1;
		}
	}
	public String readOutputLine()
	{
		if (process_ == null ||  bufferedReaderOutput_ == null)
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
		if (process_ == null || bufferedReaderError_ == null)
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
		if (process_ == null)
			return null;

		return process_.getOutputStream();
	}
	public InputStream getInputStream()
	{
		if (process_ == null)
			return null;

		return process_.getInputStream();
	}
	public InputStream getErrorStream()
	{
		if (process_ == null)
			return null;

		return process_.getErrorStream();
	}

	public boolean processEnded()
	{
		try{
			if (process_ != null)
				process_.exitValue();
			else
				return false;
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
	 * @param workbenchWindow the workbench window used to show messages
	 * (if null no messages will be triggered)
	 * @return
	 */
	public static boolean launchTool(final String fileName,final Collection<String> args,final boolean showOutput,
			final boolean waitFor,final boolean useThread,final IWorkbenchWindow workbenchWindow)
	{
		// we need a new thread so we won't block the caller
		Thread launcherThread = new Thread(new Runnable() {
			@Override
			public void run()
			{
				try{
					final ExternalToolInvoker toolInvoker = new ExternalToolInvoker(fileName, args, useThread);
					toolInvoker.run();

					if (waitFor)
					{
						if (toolInvoker.waitFor() != 0 && workbenchWindow != null)
						{
							showMessageBox(workbenchWindow, "The tool returned a non-zero value.");
						}
					}

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
							System.out.println("tool exited.");
						}
						else {
							Thread outputStreamThread = new Thread(new Runnable() {
								@Override
								public void run()
								{
									String line="";
									while(!toolInvoker.processEnded())
									{
										if ((line = toolInvoker.readOutputLine()) != null)
											System.out.println(line);
									}
								}
							});
							Thread errorStreamThread = new Thread(new Runnable() {
								@Override
								public void run()
								{
									String line="";
									while(!toolInvoker.processEnded())
									{
										if ((line = toolInvoker.readErrorLine()) != null)
											System.out.println(line);
									}
									System.out.println("tool exited.");

									if (toolInvoker.waitFor() != 0)
									{
										showMessageBox(workbenchWindow, "The tool returned a non-zero value.");
									}
								}
							});
							outputStreamThread.start();
							errorStreamThread.start();
						}
					}
				}
				catch (Exception e) {
					e.printStackTrace();
				}
			}
		});
		launcherThread.start();
		return true;
	}

	/**
	 * Shows a message box with the specified message (thread-safe)
	 * @param window the window where to show the message box
	 * @param message the message to print
	 */
	private static void showMessageBox(final IWorkbenchWindow window,final String message)
	{
		try
		{
			window.getShell().getDisplay().asyncExec(new Runnable() {
				@Override
				public void run()
				{
					MessageBox box = new MessageBox(window.getShell());
					box.setMessage(message);
					box.open();
				}
			});
		}
		catch (Exception e)
		{
			e.printStackTrace();
		}
	}
}
