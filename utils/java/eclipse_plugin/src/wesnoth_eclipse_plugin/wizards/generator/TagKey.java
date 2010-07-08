/**
 * @author Timotei Dolean
 *
 */
package wesnoth_eclipse_plugin.wizards.generator;

public class TagKey
{
	public String	Name;
	/**
	 * Cardinality can be:
	 * 1 = required
	 * ? = optional
	 * * = repeated
	 * - = forbidden
	 */
	public char		Cardinality;
	public String	Regex;

	public TagKey(String name, char cardinality, String regex) {
		Name = name;
		Cardinality = cardinality;
		Regex = regex;
	}
}
