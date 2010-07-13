/**
 * @author Timotei Dolean
 *
 */
package org.wesnoth.ui.syntax.bracketmatching;

import org.wesnoth.services.WMLGrammarAccess;
import org.wesnoth.wML.WMLTag;

import com.google.inject.Inject;

import java.util.List;

import org.eclipse.jface.text.IRegion;
import org.eclipse.jface.text.Region;
import org.eclipse.xtext.AbstractElement;
import org.eclipse.xtext.Keyword;
import org.eclipse.xtext.parsetree.AbstractNode;
import org.eclipse.xtext.parsetree.CompositeNode;
import org.eclipse.xtext.parsetree.LeafNode;
import org.eclipse.xtext.parsetree.NodeUtil;
import org.eclipse.xtext.resource.XtextResource;
import org.eclipse.xtext.ui.editor.bracketmatching.DefaultBracketMatcher;
import org.eclipse.xtext.util.Pair;

public class WMLBracketMatching extends DefaultBracketMatcher
{
	@Inject
	private WMLGrammarAccess	grammarAccess;

	@Override
	public void configure(IBracketPairAcceptor acceptor)
	{
		//TODO: fix highlighting multiple characters
		// look into: MatchingCharacterPainter : private void handleDrawRequest(GC gc)  -> length
		List<Pair<Keyword, Keyword>> pairs2 = grammarAccess.findKeywordPairs("[/", "]");
		for (Pair<Keyword, Keyword> pair : pairs2)
		{
			acceptor.accept(pair.getFirst(), pair.getSecond());
		}
		super.configure(acceptor);
	}

	@Override
	public IRegion computeMatchingRegion(XtextResource state, int offset)
	{
		if (state == null || state.getContents() == null || state.getContents().isEmpty())
			return null;
		CompositeNode rootNode = NodeUtil.getRootNode(state.getContents().get(0));
		if (rootNode == null)
			return null;
		AbstractNode node = NodeUtil.findLeafNodeAtOffset(rootNode, offset);
		if (node == null)
			return null;

		/* -- AbstractBracketMatcher.class -- */
		AbstractElement element = findElement(node, getPairs());
		boolean forwardSearch = true;
		if (element == null)
		{
			forwardSearch = false;
			element = findElement(node, getPairs().inverse());
		}
		if (element != null)
		{
			AbstractNode correspondingNode = findMatchingNode(node, element, forwardSearch);
			if (correspondingNode != null)
			{
				return new Region(correspondingNode.getOffset(), correspondingNode.getLength());
			}
		}

		/* -- WML Related -- */
		// search for opened/closed tag

		// find opened tag at this offset
		LeafNode wmlNode = findWMLLeafNodeAtOffset(rootNode, offset, false);
		if (wmlNode == null)
		{
			wmlNode = findWMLLeafNodeAtOffset(rootNode, offset, true);
		}
		if (wmlNode != null)
		{
			AbstractNode correspondingTag = null;
			for (AbstractNode tmpNode : wmlNode.getParent().getChildren())
			{
				if (!(tmpNode instanceof LeafNode))
					continue;
				if (((LeafNode) tmpNode).getText().equals(wmlNode.getText()) && tmpNode != wmlNode)
				{
					correspondingTag = tmpNode;
				}
			}
			if (correspondingTag != null)
			{
				return new Region(correspondingTag.getOffset(), correspondingTag.getLength());
			}
		}

		return null;
	}

	public LeafNode findWMLLeafNodeAtOffset(CompositeNode parseTreeRootNode, int offset, boolean findClosed)
	{
		boolean isClosed = false;
		for (AbstractNode node : parseTreeRootNode.getChildren())
		{
			if (node.getTotalOffset() <= offset)
			{
				if (node instanceof LeafNode && ((LeafNode) node).getText().equals("[/"))
				{
					isClosed = true;
				}
				if (node.getTotalOffset() + node.getTotalLength() >= offset)
				{
					if (node instanceof LeafNode && isWMLTag(node) && (isClosed == findClosed))
						return (LeafNode) node;

					else if (node instanceof CompositeNode)
						return findWMLLeafNodeAtOffset((CompositeNode) node, offset, findClosed);
				}
			}
		}
		return null;
	}

	private boolean isWMLTag(AbstractNode node)
	{
		return (node.eContainer() != null &&
				node.eContainer() instanceof CompositeNode && ((CompositeNode) node.eContainer()).getElement() instanceof WMLTag);
	}
}
