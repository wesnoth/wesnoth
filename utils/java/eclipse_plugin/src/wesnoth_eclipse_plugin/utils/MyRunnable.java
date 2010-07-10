/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.utils;

public class MyRunnable<T> implements Runnable
{
	protected final T	runnableObject_;

	public MyRunnable(T t) {
		this.runnableObject_ = t;
	}

	@Override
	public void run()
	{
	}
}
