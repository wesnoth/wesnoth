/*******************************************************************************
 * Copyright (c) 2010 - 2011 by Timotei Dolean <timotei21@gmail.com>
 *
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.generator;

import java.util.List;

import org.eclipse.jface.wizard.IWizardPage;

import org.wesnoth.Constants;
import org.wesnoth.schema.SchemaParser;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.wizards.WizardTemplate;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLTag;

public class WizardGenerator extends WizardTemplate
{
    private String tagName_;
    private int    indent_;

    public WizardGenerator( String title, String tagName, int indent )
    {
        SchemaParser.getInstance( null ).parseSchema( false );
        setWindowTitle( title );
        WMLTag tagContent = SchemaParser.getInstance( null ).getTags( )
                .get( tagName );

        tagName_ = tagName;
        indent_ = indent;
        if( tagContent == null )
            addPage( new WizardGeneratorPage404( tagName ) );
        else {
            // keys section
            List< WMLKey > keys = tagContent.getWMLKeys( );
            int keysNr = keys.size( );
            int startKey = 0, pgsKey = ( keysNr / Constants.WIZ_MaxTextBoxesOnPage );
            WizardGeneratorPageKey tempPageKey;
            for( int i = 0; i < pgsKey; i++ ) {
                tempPageKey = new WizardGeneratorPageKey( tagName, keys,
                        startKey, startKey + Constants.WIZ_MaxTextBoxesOnPage,
                        indent_ + 1 );
                startKey += Constants.WIZ_MaxTextBoxesOnPage;
                addPage( tempPageKey );
            }

            if( keysNr - 1 > 0 ) {
                tempPageKey = new WizardGeneratorPageKey( tagName, keys,
                        startKey, keysNr - 1, indent_ + 1 );
                addPage( tempPageKey );
            }

            // tags section
            List< WMLTag > tags = tagContent.getWMLTags( );
            int tagsNr = tags.size( );
            int startTag = 0, pgsTag = ( tagsNr / Constants.WIZ_MaxGroupsOnPage );
            WizardGeneratorPageTag tempPageTag;
            for( int i = 0; i < pgsTag; i++ ) {
                tempPageTag = new WizardGeneratorPageTag( tagName, tags,
                        startTag, startTag + Constants.WIZ_MaxGroupsOnPage,
                        indent_ + 1 );
                startTag += Constants.WIZ_MaxTextBoxesOnPage;
                addPage( tempPageTag );
            }
            if( tagsNr - 1 > 0 ) {
                tempPageTag = new WizardGeneratorPageTag( tagName, tags,
                        startTag, tagsNr - 1, indent_ + 1 );
                addPage( tempPageTag );
            }

            if( getPageCount( ) == 0 ) {
                addPage( new WizardGeneratorPage404( tagName ) );
            }
        }
    }

    public int getIndent( )
    {
        return indent_;
    }

    @Override
    public boolean performFinish( )
    {
        // logic
        String result = StringUtils.multiples( "\t", indent_ ) + "[" + tagName_ + "]\n"; //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
        StringBuilder keys = new StringBuilder( );
        StringBuilder tags = new StringBuilder( );
        for( IWizardPage page: getPages( ) ) {
            if( page instanceof WizardGeneratorPageKey )
                keys.append( ( ( WizardGeneratorPageKey ) page ).getContent( ) );
            else if( page instanceof WizardGeneratorPageTag )
                tags.append( ( ( WizardGeneratorPageTag ) page ).getContent( ) );
            else
                ; // skip 404 pages
        }
        result += ( keys.toString( ) + tags.toString( ) );
        result += ( StringUtils.multiples( "\t", indent_ ) + "[/" + tagName_ + "]\n" ); //$NON-NLS-1$ //$NON-NLS-2$ //$NON-NLS-3$
        data_ = result;
        // for now let's just return tag's name
        objectName_ = tagName_;
        isFinished_ = true;
        return true;
    }
}
