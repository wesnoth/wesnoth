/**
 * @author Timotei Dolean
 *
 */
package org.wesnoth.ui.syntax;

import org.eclipse.xtext.ui.editor.syntaxcoloring.antlr.DefaultAntlrTokenToAttributeIdMapper;

public class WMLAntlrTokenToAttributeIdMapper extends DefaultAntlrTokenToAttributeIdMapper
{
	@Override
	protected String calculateId(String tokenName, int tokenType)
	{
		if (tokenName.equals("'['") || tokenName.equals("'[/'") || tokenName.equals("']'"))
		{
			return WMLHighlightingConfiguration.RULE_WML_TAG;
		}
		if (tokenName.equals("'{'") || tokenName.equals("'}'"))
		{
			return WMLHighlightingConfiguration.RULE_WML_MACRO;
		}
		return super.calculateId(tokenName, tokenType);
	}
}
