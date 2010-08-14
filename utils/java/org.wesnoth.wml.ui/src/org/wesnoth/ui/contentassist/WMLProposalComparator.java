/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.ui.contentassist;

import org.eclipse.jface.text.contentassist.ICompletionProposal;
import org.eclipse.xtext.ui.editor.contentassist.ConfigurableCompletionProposal;
import org.eclipse.xtext.ui.editor.contentassist.ICompletionProposalComparator;

public class WMLProposalComparator implements ICompletionProposalComparator
{
	public int compare(ICompletionProposal arg0, ICompletionProposal arg1)
	{
		if (arg0 instanceof ConfigurableCompletionProposal &&
			arg1 instanceof ConfigurableCompletionProposal)
		{
			ConfigurableCompletionProposal tmp0 = (ConfigurableCompletionProposal)arg0;
			ConfigurableCompletionProposal tmp1 = (ConfigurableCompletionProposal)arg1;
			return tmp0.compareTo(tmp1);
		}
		return 0;
	}
}
