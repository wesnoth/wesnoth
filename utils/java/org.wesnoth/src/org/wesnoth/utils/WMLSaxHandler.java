/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.util.Stack;

import org.wesnoth.wml.core.ConfigFile;
import org.wesnoth.wml.core.Variable;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

/**
 * A simple wml sax handler that parsers the xml of a config file
 * and gets some information about it
 */
public class WMLSaxHandler extends DefaultHandler
{
	private static Stack<String> stack_;
	private Variable tmpVar_;
	private String filePath_;
	private ConfigFile cfg_;

	private int STATUS = 0;
	/** states list */
	private static final int SET_VARIABLE = 1;
	private static final int SET_VARIABLE_ARRAY = 2;

	/**
	 * @param filePath
	 */
	public WMLSaxHandler(String filePath)
	{
		stack_ = new Stack<String>();
		filePath_ = filePath;
		cfg_ = new ConfigFile(filePath);
	}

    @Override
	public void startElement(String uri, String localName,
       String rawName, Attributes attributes)
    {
    	stack_.push(rawName);
    	if (rawName.equals("set_variable")) //$NON-NLS-1$
    	{
    		STATUS = SET_VARIABLE;
    		tmpVar_ = new Variable();
    	}
    	else if (rawName.equals("set_variables")) //$NON-NLS-1$
    	{
    		STATUS = SET_VARIABLE_ARRAY;
    		tmpVar_ = new Variable();
    		tmpVar_.setIsArray(true);
    	}
    	else if (rawName.equals("campaign")) //$NON-NLS-1$
    	{
    		cfg_.setIsCampaign(true);
    	}
    	else if (rawName.equals("scenario")) //$NON-NLS-1$
    	{
    		cfg_.setIsScenario(true);
    	}
    }

    @Override
    public void characters(char[] ch, int start, int length)
    		throws SAXException
    {
    	if (stack_.empty() == false)
    	{
    		String peek = stack_.peek();
    		if (peek.equals("id")) //$NON-NLS-1$
    		{
    			if (stack_.get(stack_.size() - 2).equals("campaign")) //$NON-NLS-1$
    				cfg_.setCampaignId(new String(ch, start, length));
    			else if (stack_.get(stack_.size() - 2).equals("scenario")) //$NON-NLS-1$
    				cfg_.setScenarioId(new String(ch, start, length));
    		}

			if (STATUS == SET_VARIABLE ||
				STATUS == SET_VARIABLE_ARRAY)
			{
				if (peek.equals("name")) //$NON-NLS-1$
				{
					String name = new String(ch, start, length);
					if (name.contains("[")) // we have an array //$NON-NLS-1$
					{
						tmpVar_.setIsArray(true);
						name = name.substring(0, name.indexOf('['));
					}
					System.out.println("added variable: [" + name + //$NON-NLS-1$
							(tmpVar_.isArray()?" - array -]": "]") + //$NON-NLS-1$ //$NON-NLS-2$
							" file: [" + filePath_ + "]"); //$NON-NLS-1$ //$NON-NLS-2$
					tmpVar_.setName(name);
					tmpVar_.setOffset(start);
					tmpVar_.setLocation(filePath_);

					cfg_.getVariables().add(tmpVar_);
				}
    		}
    	}
    	super.characters(ch, start, length);
    }
    @Override
    public void endElement(String uri, String localName, String qName)
    		throws SAXException
    {
    	super.endElement(uri, localName, qName);
    	stack_.pop();
    	if (qName.equals("set_variable")) //$NON-NLS-1$
    		STATUS = 0;
    }

    @Override
	public void endDocument()
    {
    }

    public String getFilePath()
	{
		return filePath_;
	}

    /**
     * Gets the Config resulted by parsing the file
     * @return
     */
    public ConfigFile getConfigFile()
	{
		return cfg_;
	}
}
