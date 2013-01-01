/*******************************************************************************
 * Copyright (c) 2010 - 2013 by Timotei Dolean <timotei21@gmail.com>
 * 
 * This program and the accompanying materials are made available
 * under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *******************************************************************************/
package org.wesnoth.wizards.generator;

import java.util.List;

import org.eclipse.jface.wizard.IWizardPage;

import org.wesnoth.schema.SchemaParser;
import org.wesnoth.utils.StringUtils;
import org.wesnoth.wizards.WizardTemplate;
import org.wesnoth.wml.WMLKey;
import org.wesnoth.wml.WMLTag;

/**
 * Wizard that generates WML code for a specific tag
 */
public class WizardGenerator extends WizardTemplate
{
    private static final int WIZ_MaxTextBoxesOnPage = 10;
    private static final int WIZ_MaxGroupsOnPage    = 4;

    private String           tagName_;
    private int              indent_;

    /**
     * Creates a new {@link WizardGenerator}
     * 
     * @param title
     *        The title of the wizard
     * @param tagName
     *        The name of the tag to generate WML code for
     * @param indent
     *        The indent used in the generated WML code
     */
    public WizardGenerator( String title, String tagName, int indent )
    {
        SchemaParser.getInstance( null ).parseSchema( false );
        setWindowTitle( title );
        WMLTag tagContent = SchemaParser.getInstance( null ).getTags( )
            .get( tagName );

        tagName_ = tagName;
        indent_ = indent;
        if( tagContent == null ) {
            addPage( new WizardGenerator404Page( tagName ) );
        }
        else {
            // keys section
            List< WMLKey > keys = tagContent.getWMLKeys( );
            int keysNr = keys.size( );
            int startKey = 0, pgsKey = ( keysNr / WIZ_MaxTextBoxesOnPage );
            WizardGeneratorKeysPage tempPageKey;
            for( int i = 0; i < pgsKey; i++ ) {
                tempPageKey = new WizardGeneratorKeysPage( tagName, keys,
                    startKey, startKey + WIZ_MaxTextBoxesOnPage,
                    indent_ + 1 );
                startKey += WIZ_MaxTextBoxesOnPage;
                addPage( tempPageKey );
            }

            if( keysNr - 1 > 0 ) {
                tempPageKey = new WizardGeneratorKeysPage( tagName, keys,
                    startKey, keysNr - 1, indent_ + 1 );
                addPage( tempPageKey );
            }

            // tags section
            List< WMLTag > tags = tagContent.getWMLTags( );
            int tagsNr = tags.size( );
            int startTag = 0, pgsTag = ( tagsNr / WIZ_MaxGroupsOnPage );
            WizardGeneratorTagPage tempPageTag;
            for( int i = 0; i < pgsTag; i++ ) {
                tempPageTag = new WizardGeneratorTagPage( tagName, tags,
                    startTag, startTag + WIZ_MaxGroupsOnPage,
                    indent_ + 1 );
                startTag += WIZ_MaxTextBoxesOnPage;
                addPage( tempPageTag );
            }
            if( tagsNr - 1 > 0 ) {
                tempPageTag = new WizardGeneratorTagPage( tagName, tags,
                    startTag, tagsNr - 1, indent_ + 1 );
                addPage( tempPageTag );
            }

            if( getPageCount( ) == 0 ) {
                addPage( new WizardGenerator404Page( tagName ) );
            }
        }
    }

    /**
     * @return The indent of the WML Code
     */
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
            if( page instanceof WizardGeneratorKeysPage ) {
                keys.append( ( ( WizardGeneratorKeysPage ) page ).getContent( ) );
            }
            else if( page instanceof WizardGeneratorTagPage ) {
                tags.append( ( ( WizardGeneratorTagPage ) page ).getContent( ) );
            }
            else {
                // skip 404 pages
            }
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
