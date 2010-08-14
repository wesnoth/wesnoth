/*******************************************************************************
 * Copyright (c) 2010 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.utils;

import java.util.HashMap;
import java.util.Map;
import java.util.Stack;

import org.wesnoth.wml.core.Variable;
import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

/**
 * A simple wml sax handler that parsers the xml of a config file
 */
public class WMLSaxHandler extends DefaultHandler
{
	public String CampaignId = null;
	public String ScenarioId = null;
	public Map<String, Variable> Variables = new HashMap<String, Variable>();

	private static Stack<String> stack_;
	private static final int SET_VARIABLE = 1;
	private static final int SET_VARIABLE_ARRAY = 2;
	private int STATUS = 0;
	private Variable tmpVar_;
	private String filePath_;

	public WMLSaxHandler(String filePath)
	{
		stack_ = new Stack<String>();
		filePath_ = filePath;
	}

    @Override
	public void startElement(String uri, String localName,
       String rawName, Attributes attributes)
    {
    	stack_.push(rawName);
    	if (rawName.equals("set_variable"))
    	{
    		STATUS = SET_VARIABLE;
    		tmpVar_ = new Variable();
    	} else if (rawName.equals("set_variables"))
    	{
    		STATUS = SET_VARIABLE_ARRAY;
    		tmpVar_ = new Variable();
    		tmpVar_.setIsArray(true);
    	}
    }

    @Override
    public void characters(char[] ch, int start, int length)
    		throws SAXException
    {
    	if (stack_.empty() == false)
    	{
    		String peek = stack_.peek();
    		if (peek.equals("id"))
    		{
    			if (stack_.get(stack_.size() - 2).equals("campaign"))
    				CampaignId = new String(ch, start, length);
    			else if (stack_.get(stack_.size() - 2).equals("scenario"))
    				ScenarioId = new String(ch, start, length);
    		}

			if (STATUS == SET_VARIABLE ||
				STATUS == SET_VARIABLE_ARRAY)
			{
				if (peek.equals("name"))
				{
					String name = new String(ch, start, length);
					if (name.contains("[")) // we have an array
					{
						tmpVar_.setIsArray(true);
						name = name.substring(0, name.indexOf('['));
					}
					System.out.println("added variable: [" + name +
							(tmpVar_.isArray()?" - array -]": "]") +
							" file: [" + filePath_ + "]");
					tmpVar_.setName(name);
					tmpVar_.setOffset(start);
					tmpVar_.setLocation(filePath_);

					Variables.put(name, tmpVar_);
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
    	if (qName.equals("set_variable"))
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
}
