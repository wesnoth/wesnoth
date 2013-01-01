/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.quickfix;

import org.eclipse.xtext.ui.editor.quickfix.DefaultQuickfixProvider;

/**
 * 
 */
public class WMLQuickfixProvider extends DefaultQuickfixProvider
{

    // @Fix(MyJavaValidator.INVALID_NAME)
    // public void capitalizeName(final Issue issue, IssueResolutionAcceptor
    // acceptor) {
    // acceptor.accept(issue, "Capitalize name", "Capitalize the name.",
    // "upcase.png", new IModification() {
    // public void apply(IModificationContext context) throws
    // BadLocationException {
    // IXtextDocument xtextDocument = context.getXtextDocument();
    // String firstLetter = xtextDocument.get(issue.getOffset(), 1);
    // xtextDocument.replace(issue.getOffset(), 1, firstLetter.toUpperCase());
    // }
    // });
    // }

}
