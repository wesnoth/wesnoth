/**
 * @author Timotei Dolean
 */
package wesnoth_eclipse_plugin;

import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class StringUtils {
	public static boolean startsWith(String target, String sequence)
	{
		Pattern pattern  = Pattern.compile("[\t| ]*"+sequence);
		Matcher matcher = pattern.matcher(target);
		return (matcher.find() && matcher.start() == 0);
	}
}