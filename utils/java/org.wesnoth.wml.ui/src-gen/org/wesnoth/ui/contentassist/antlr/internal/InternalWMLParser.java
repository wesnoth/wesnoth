package org.wesnoth.ui.contentassist.antlr.internal; 

import java.io.InputStream;
import org.eclipse.xtext.*;
import org.eclipse.xtext.parser.*;
import org.eclipse.xtext.parser.impl.*;
import org.eclipse.xtext.parsetree.*;
import org.eclipse.emf.ecore.util.EcoreUtil;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.parser.antlr.XtextTokenStream;
import org.eclipse.xtext.parser.antlr.XtextTokenStream.HiddenTokens;
import org.eclipse.xtext.ui.editor.contentassist.antlr.internal.AbstractInternalContentAssistParser;
import org.eclipse.xtext.ui.editor.contentassist.antlr.internal.DFA;
import org.wesnoth.services.WMLGrammarAccess;



import org.antlr.runtime.*;
import java.util.Stack;
import java.util.List;
import java.util.ArrayList;

@SuppressWarnings("all")
public class InternalWMLParser extends AbstractInternalContentAssistParser {
    public static final String[] tokenNames = new String[] {
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_STRING", "RULE_IINT", "RULE_SL_COMMENT", "RULE_WS", "'_'", "':'", "'-'", "'.'", "'('", "')'", "'='", "'/'", "'n'", "'s'", "'w'", "'e'", "'sw'", "'se'", "'ne'", "'nw'", "'{'", "'}'", "'['", "']'", "'[/'", "' '", "','", "'~'"
    };
    public static final int RULE_ID=4;
    public static final int RULE_STRING=5;
    public static final int RULE_IINT=6;
    public static final int RULE_WS=8;
    public static final int EOF=-1;
    public static final int RULE_SL_COMMENT=7;

        public InternalWMLParser(TokenStream input) {
            super(input);
        }
        

    public String[] getTokenNames() { return tokenNames; }
    public String getGrammarFileName() { return "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g"; }


     
     	private WMLGrammarAccess grammarAccess;
     	
        public void setGrammarAccess(WMLGrammarAccess grammarAccess) {
        	this.grammarAccess = grammarAccess;
        }
        
        @Override
        protected Grammar getGrammar() {
        	return grammarAccess.getGrammar();
        }
        
        @Override
        protected String getValueForTokenName(String tokenName) {
        	return tokenName;
        }




    // $ANTLR start entryRuleWMLRoot
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:61:1: entryRuleWMLRoot : ruleWMLRoot EOF ;
    public final void entryRuleWMLRoot() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:62:1: ( ruleWMLRoot EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:63:1: ruleWMLRoot EOF
            {
             before(grammarAccess.getWMLRootRule()); 
            pushFollow(FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot61);
            ruleWMLRoot();
            _fsp--;

             after(grammarAccess.getWMLRootRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLRoot68); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end entryRuleWMLRoot


    // $ANTLR start ruleWMLRoot
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:70:1: ruleWMLRoot : ( ( rule__WMLRoot__Alternatives )* ) ;
    public final void ruleWMLRoot() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:74:2: ( ( ( rule__WMLRoot__Alternatives )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:75:1: ( ( rule__WMLRoot__Alternatives )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:75:1: ( ( rule__WMLRoot__Alternatives )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:76:1: ( rule__WMLRoot__Alternatives )*
            {
             before(grammarAccess.getWMLRootAccess().getAlternatives()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:77:1: ( rule__WMLRoot__Alternatives )*
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( (LA1_0==25||LA1_0==27) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:77:2: rule__WMLRoot__Alternatives
            	    {
            	    pushFollow(FOLLOW_rule__WMLRoot__Alternatives_in_ruleWMLRoot94);
            	    rule__WMLRoot__Alternatives();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop1;
                }
            } while (true);

             after(grammarAccess.getWMLRootAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLRoot


    // $ANTLR start entryRuleWMLMacro
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:89:1: entryRuleWMLMacro : ruleWMLMacro EOF ;
    public final void entryRuleWMLMacro() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:90:1: ( ruleWMLMacro EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:91:1: ruleWMLMacro EOF
            {
             before(grammarAccess.getWMLMacroRule()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_entryRuleWMLMacro122);
            ruleWMLMacro();
            _fsp--;

             after(grammarAccess.getWMLMacroRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacro129); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end entryRuleWMLMacro


    // $ANTLR start ruleWMLMacro
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:98:1: ruleWMLMacro : ( ( rule__WMLMacro__Group__0 ) ) ;
    public final void ruleWMLMacro() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:102:2: ( ( ( rule__WMLMacro__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:103:1: ( ( rule__WMLMacro__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:103:1: ( ( rule__WMLMacro__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:104:1: ( rule__WMLMacro__Group__0 )
            {
             before(grammarAccess.getWMLMacroAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:105:1: ( rule__WMLMacro__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:105:2: rule__WMLMacro__Group__0
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__0_in_ruleWMLMacro155);
            rule__WMLMacro__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLMacro


    // $ANTLR start entryRuleWMLTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:117:1: entryRuleWMLTag : ruleWMLTag EOF ;
    public final void entryRuleWMLTag() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:118:1: ( ruleWMLTag EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:119:1: ruleWMLTag EOF
            {
             before(grammarAccess.getWMLTagRule()); 
            pushFollow(FOLLOW_ruleWMLTag_in_entryRuleWMLTag182);
            ruleWMLTag();
            _fsp--;

             after(grammarAccess.getWMLTagRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTag189); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end entryRuleWMLTag


    // $ANTLR start ruleWMLTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:126:1: ruleWMLTag : ( ( rule__WMLTag__Group__0 ) ) ;
    public final void ruleWMLTag() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:130:2: ( ( ( rule__WMLTag__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:131:1: ( ( rule__WMLTag__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:131:1: ( ( rule__WMLTag__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:132:1: ( rule__WMLTag__Group__0 )
            {
             before(grammarAccess.getWMLTagAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:133:1: ( rule__WMLTag__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:133:2: rule__WMLTag__Group__0
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0_in_ruleWMLTag215);
            rule__WMLTag__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLTagAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLTag


    // $ANTLR start entryRuleWMLStartTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:145:1: entryRuleWMLStartTag : ruleWMLStartTag EOF ;
    public final void entryRuleWMLStartTag() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:146:1: ( ruleWMLStartTag EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:147:1: ruleWMLStartTag EOF
            {
             before(grammarAccess.getWMLStartTagRule()); 
            pushFollow(FOLLOW_ruleWMLStartTag_in_entryRuleWMLStartTag242);
            ruleWMLStartTag();
            _fsp--;

             after(grammarAccess.getWMLStartTagRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLStartTag249); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end entryRuleWMLStartTag


    // $ANTLR start ruleWMLStartTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:154:1: ruleWMLStartTag : ( ( rule__WMLStartTag__Group__0 ) ) ;
    public final void ruleWMLStartTag() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:158:2: ( ( ( rule__WMLStartTag__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:159:1: ( ( rule__WMLStartTag__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:159:1: ( ( rule__WMLStartTag__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:160:1: ( rule__WMLStartTag__Group__0 )
            {
             before(grammarAccess.getWMLStartTagAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:161:1: ( rule__WMLStartTag__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:161:2: rule__WMLStartTag__Group__0
            {
            pushFollow(FOLLOW_rule__WMLStartTag__Group__0_in_ruleWMLStartTag275);
            rule__WMLStartTag__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLStartTagAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLStartTag


    // $ANTLR start entryRuleWMLEndTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:173:1: entryRuleWMLEndTag : ruleWMLEndTag EOF ;
    public final void entryRuleWMLEndTag() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:174:1: ( ruleWMLEndTag EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:175:1: ruleWMLEndTag EOF
            {
             before(grammarAccess.getWMLEndTagRule()); 
            pushFollow(FOLLOW_ruleWMLEndTag_in_entryRuleWMLEndTag302);
            ruleWMLEndTag();
            _fsp--;

             after(grammarAccess.getWMLEndTagRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLEndTag309); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end entryRuleWMLEndTag


    // $ANTLR start ruleWMLEndTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:182:1: ruleWMLEndTag : ( ( rule__WMLEndTag__Group__0 ) ) ;
    public final void ruleWMLEndTag() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:186:2: ( ( ( rule__WMLEndTag__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:187:1: ( ( rule__WMLEndTag__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:187:1: ( ( rule__WMLEndTag__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:188:1: ( rule__WMLEndTag__Group__0 )
            {
             before(grammarAccess.getWMLEndTagAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:189:1: ( rule__WMLEndTag__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:189:2: rule__WMLEndTag__Group__0
            {
            pushFollow(FOLLOW_rule__WMLEndTag__Group__0_in_ruleWMLEndTag335);
            rule__WMLEndTag__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLEndTagAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLEndTag


    // $ANTLR start entryRuleWMLKey
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:201:1: entryRuleWMLKey : ruleWMLKey EOF ;
    public final void entryRuleWMLKey() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:202:1: ( ruleWMLKey EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:203:1: ruleWMLKey EOF
            {
             before(grammarAccess.getWMLKeyRule()); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey362);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLKeyRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey369); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end entryRuleWMLKey


    // $ANTLR start ruleWMLKey
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:210:1: ruleWMLKey : ( ( rule__WMLKey__Group__0 ) ) ;
    public final void ruleWMLKey() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:214:2: ( ( ( rule__WMLKey__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:215:1: ( ( rule__WMLKey__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:215:1: ( ( rule__WMLKey__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:216:1: ( rule__WMLKey__Group__0 )
            {
             before(grammarAccess.getWMLKeyAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:217:1: ( rule__WMLKey__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:217:2: rule__WMLKey__Group__0
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey395);
            rule__WMLKey__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLKey


    // $ANTLR start entryRuleWMLKeyValue
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:229:1: entryRuleWMLKeyValue : ruleWMLKeyValue EOF ;
    public final void entryRuleWMLKeyValue() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:230:1: ( ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:231:1: ruleWMLKeyValue EOF
            {
             before(grammarAccess.getWMLKeyValueRule()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue422);
            ruleWMLKeyValue();
            _fsp--;

             after(grammarAccess.getWMLKeyValueRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue429); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end entryRuleWMLKeyValue


    // $ANTLR start ruleWMLKeyValue
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:238:1: ruleWMLKeyValue : ( ( rule__WMLKeyValue__Alternatives ) ) ;
    public final void ruleWMLKeyValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:242:2: ( ( ( rule__WMLKeyValue__Alternatives ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:243:1: ( ( rule__WMLKeyValue__Alternatives ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:243:1: ( ( rule__WMLKeyValue__Alternatives ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:244:1: ( rule__WMLKeyValue__Alternatives )
            {
             before(grammarAccess.getWMLKeyValueAccess().getAlternatives()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:245:1: ( rule__WMLKeyValue__Alternatives )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:245:2: rule__WMLKeyValue__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue455);
            rule__WMLKeyValue__Alternatives();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyValueAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLKeyValue


    // $ANTLR start entryRuleFLOAT
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:257:1: entryRuleFLOAT : ruleFLOAT EOF ;
    public final void entryRuleFLOAT() throws RecognitionException {

        	HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();

        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:261:1: ( ruleFLOAT EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:262:1: ruleFLOAT EOF
            {
             before(grammarAccess.getFLOATRule()); 
            pushFollow(FOLLOW_ruleFLOAT_in_entryRuleFLOAT487);
            ruleFLOAT();
            _fsp--;

             after(grammarAccess.getFLOATRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleFLOAT494); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	myHiddenTokenState.restore();

        }
        return ;
    }
    // $ANTLR end entryRuleFLOAT


    // $ANTLR start ruleFLOAT
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:272:1: ruleFLOAT : ( ( rule__FLOAT__Group__0 ) ) ;
    public final void ruleFLOAT() throws RecognitionException {

        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();
        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:277:2: ( ( ( rule__FLOAT__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:278:1: ( ( rule__FLOAT__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:278:1: ( ( rule__FLOAT__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:279:1: ( rule__FLOAT__Group__0 )
            {
             before(grammarAccess.getFLOATAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:280:1: ( rule__FLOAT__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:280:2: rule__FLOAT__Group__0
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__0_in_ruleFLOAT524);
            rule__FLOAT__Group__0();
            _fsp--;


            }

             after(grammarAccess.getFLOATAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);
            	myHiddenTokenState.restore();

        }
        return ;
    }
    // $ANTLR end ruleFLOAT


    // $ANTLR start entryRuleTSTRING
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:293:1: entryRuleTSTRING : ruleTSTRING EOF ;
    public final void entryRuleTSTRING() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:294:1: ( ruleTSTRING EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:295:1: ruleTSTRING EOF
            {
             before(grammarAccess.getTSTRINGRule()); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING551);
            ruleTSTRING();
            _fsp--;

             after(grammarAccess.getTSTRINGRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING558); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end entryRuleTSTRING


    // $ANTLR start ruleTSTRING
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:302:1: ruleTSTRING : ( ( rule__TSTRING__Group__0 ) ) ;
    public final void ruleTSTRING() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:306:2: ( ( ( rule__TSTRING__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:307:1: ( ( rule__TSTRING__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:307:1: ( ( rule__TSTRING__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:308:1: ( rule__TSTRING__Group__0 )
            {
             before(grammarAccess.getTSTRINGAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:1: ( rule__TSTRING__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:2: rule__TSTRING__Group__0
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0_in_ruleTSTRING584);
            rule__TSTRING__Group__0();
            _fsp--;


            }

             after(grammarAccess.getTSTRINGAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleTSTRING


    // $ANTLR start entryRulePATH
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:321:1: entryRulePATH : rulePATH EOF ;
    public final void entryRulePATH() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:322:1: ( rulePATH EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:323:1: rulePATH EOF
            {
             before(grammarAccess.getPATHRule()); 
            pushFollow(FOLLOW_rulePATH_in_entryRulePATH611);
            rulePATH();
            _fsp--;

             after(grammarAccess.getPATHRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePATH618); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end entryRulePATH


    // $ANTLR start rulePATH
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:330:1: rulePATH : ( ( rule__PATH__Group__0 ) ) ;
    public final void rulePATH() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:334:2: ( ( ( rule__PATH__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:335:1: ( ( rule__PATH__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:335:1: ( ( rule__PATH__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:336:1: ( rule__PATH__Group__0 )
            {
             before(grammarAccess.getPATHAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:1: ( rule__PATH__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:2: rule__PATH__Group__0
            {
            pushFollow(FOLLOW_rule__PATH__Group__0_in_rulePATH644);
            rule__PATH__Group__0();
            _fsp--;


            }

             after(grammarAccess.getPATHAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rulePATH


    // $ANTLR start entryRuleDIRECTION
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:349:1: entryRuleDIRECTION : ruleDIRECTION EOF ;
    public final void entryRuleDIRECTION() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:350:1: ( ruleDIRECTION EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:351:1: ruleDIRECTION EOF
            {
             before(grammarAccess.getDIRECTIONRule()); 
            pushFollow(FOLLOW_ruleDIRECTION_in_entryRuleDIRECTION671);
            ruleDIRECTION();
            _fsp--;

             after(grammarAccess.getDIRECTIONRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleDIRECTION678); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end entryRuleDIRECTION


    // $ANTLR start ruleDIRECTION
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:358:1: ruleDIRECTION : ( ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* ) ) ;
    public final void ruleDIRECTION() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:362:2: ( ( ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:363:1: ( ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:363:1: ( ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:364:1: ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:364:1: ( ( rule__DIRECTION__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:365:1: ( rule__DIRECTION__Group__0 )
            {
             before(grammarAccess.getDIRECTIONAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:366:1: ( rule__DIRECTION__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:366:2: rule__DIRECTION__Group__0
            {
            pushFollow(FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION706);
            rule__DIRECTION__Group__0();
            _fsp--;


            }

             after(grammarAccess.getDIRECTIONAccess().getGroup()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:369:1: ( ( rule__DIRECTION__Group__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:370:1: ( rule__DIRECTION__Group__0 )*
            {
             before(grammarAccess.getDIRECTIONAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:371:1: ( rule__DIRECTION__Group__0 )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( ((LA2_0>=17 && LA2_0<=24)) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:371:2: rule__DIRECTION__Group__0
            	    {
            	    pushFollow(FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION718);
            	    rule__DIRECTION__Group__0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);

             after(grammarAccess.getDIRECTIONAccess().getGroup()); 

            }


            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleDIRECTION


    // $ANTLR start entryRuleLIST
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:384:1: entryRuleLIST : ruleLIST EOF ;
    public final void entryRuleLIST() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:385:1: ( ruleLIST EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:386:1: ruleLIST EOF
            {
             before(grammarAccess.getLISTRule()); 
            pushFollow(FOLLOW_ruleLIST_in_entryRuleLIST748);
            ruleLIST();
            _fsp--;

             after(grammarAccess.getLISTRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleLIST755); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end entryRuleLIST


    // $ANTLR start ruleLIST
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:393:1: ruleLIST : ( ( rule__LIST__Group__0 ) ) ;
    public final void ruleLIST() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:397:2: ( ( ( rule__LIST__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:398:1: ( ( rule__LIST__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:398:1: ( ( rule__LIST__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:399:1: ( rule__LIST__Group__0 )
            {
             before(grammarAccess.getLISTAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:400:1: ( rule__LIST__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:400:2: rule__LIST__Group__0
            {
            pushFollow(FOLLOW_rule__LIST__Group__0_in_ruleLIST781);
            rule__LIST__Group__0();
            _fsp--;


            }

             after(grammarAccess.getLISTAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleLIST


    // $ANTLR start entryRulePROGRESSIVE
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:412:1: entryRulePROGRESSIVE : rulePROGRESSIVE EOF ;
    public final void entryRulePROGRESSIVE() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:413:1: ( rulePROGRESSIVE EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:414:1: rulePROGRESSIVE EOF
            {
             before(grammarAccess.getPROGRESSIVERule()); 
            pushFollow(FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE808);
            rulePROGRESSIVE();
            _fsp--;

             after(grammarAccess.getPROGRESSIVERule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePROGRESSIVE815); 

            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {
        }
        return ;
    }
    // $ANTLR end entryRulePROGRESSIVE


    // $ANTLR start rulePROGRESSIVE
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:421:1: rulePROGRESSIVE : ( ( rule__PROGRESSIVE__Group__0 ) ) ;
    public final void rulePROGRESSIVE() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:425:2: ( ( ( rule__PROGRESSIVE__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:426:1: ( ( rule__PROGRESSIVE__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:426:1: ( ( rule__PROGRESSIVE__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:427:1: ( rule__PROGRESSIVE__Group__0 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:428:1: ( rule__PROGRESSIVE__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:428:2: rule__PROGRESSIVE__Group__0
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__0_in_rulePROGRESSIVE841);
            rule__PROGRESSIVE__Group__0();
            _fsp--;


            }

             after(grammarAccess.getPROGRESSIVEAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rulePROGRESSIVE


    // $ANTLR start rule__WMLRoot__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:440:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__RtagsAssignment_0 ) ) | ( ( rule__WMLRoot__RmacrosAssignment_1 ) ) );
    public final void rule__WMLRoot__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:444:1: ( ( ( rule__WMLRoot__RtagsAssignment_0 ) ) | ( ( rule__WMLRoot__RmacrosAssignment_1 ) ) )
            int alt3=2;
            int LA3_0 = input.LA(1);

            if ( (LA3_0==27) ) {
                alt3=1;
            }
            else if ( (LA3_0==25) ) {
                alt3=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("440:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__RtagsAssignment_0 ) ) | ( ( rule__WMLRoot__RmacrosAssignment_1 ) ) );", 3, 0, input);

                throw nvae;
            }
            switch (alt3) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:445:1: ( ( rule__WMLRoot__RtagsAssignment_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:445:1: ( ( rule__WMLRoot__RtagsAssignment_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:446:1: ( rule__WMLRoot__RtagsAssignment_0 )
                    {
                     before(grammarAccess.getWMLRootAccess().getRtagsAssignment_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:447:1: ( rule__WMLRoot__RtagsAssignment_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:447:2: rule__WMLRoot__RtagsAssignment_0
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__RtagsAssignment_0_in_rule__WMLRoot__Alternatives877);
                    rule__WMLRoot__RtagsAssignment_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getRtagsAssignment_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:451:6: ( ( rule__WMLRoot__RmacrosAssignment_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:451:6: ( ( rule__WMLRoot__RmacrosAssignment_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:452:1: ( rule__WMLRoot__RmacrosAssignment_1 )
                    {
                     before(grammarAccess.getWMLRootAccess().getRmacrosAssignment_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:453:1: ( rule__WMLRoot__RmacrosAssignment_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:453:2: rule__WMLRoot__RmacrosAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__RmacrosAssignment_1_in_rule__WMLRoot__Alternatives895);
                    rule__WMLRoot__RmacrosAssignment_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getRmacrosAssignment_1()); 

                    }


                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLRoot__Alternatives


    // $ANTLR start rule__WMLMacro__TagcontentAlternatives_2_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:462:1: rule__WMLMacro__TagcontentAlternatives_2_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( ':' ) | ( '-' ) | ( '.' ) | ( '(' ) | ( ')' ) | ( '=' ) | ( '/' ) );
    public final void rule__WMLMacro__TagcontentAlternatives_2_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:466:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( ':' ) | ( '-' ) | ( '.' ) | ( '(' ) | ( ')' ) | ( '=' ) | ( '/' ) )
            int alt4=10;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt4=1;
                }
                break;
            case RULE_STRING:
                {
                alt4=2;
                }
                break;
            case 9:
                {
                alt4=3;
                }
                break;
            case 10:
                {
                alt4=4;
                }
                break;
            case 11:
                {
                alt4=5;
                }
                break;
            case 12:
                {
                alt4=6;
                }
                break;
            case 13:
                {
                alt4=7;
                }
                break;
            case 14:
                {
                alt4=8;
                }
                break;
            case 15:
                {
                alt4=9;
                }
                break;
            case 16:
                {
                alt4=10;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("462:1: rule__WMLMacro__TagcontentAlternatives_2_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( ':' ) | ( '-' ) | ( '.' ) | ( '(' ) | ( ')' ) | ( '=' ) | ( '/' ) );", 4, 0, input);

                throw nvae;
            }

            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:467:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:467:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:468:1: RULE_ID
                    {
                     before(grammarAccess.getWMLMacroAccess().getTagcontentIDTerminalRuleCall_2_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacro__TagcontentAlternatives_2_0928); 
                     after(grammarAccess.getWMLMacroAccess().getTagcontentIDTerminalRuleCall_2_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:473:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:473:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:474:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLMacroAccess().getTagcontentSTRINGTerminalRuleCall_2_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLMacro__TagcontentAlternatives_2_0945); 
                     after(grammarAccess.getWMLMacroAccess().getTagcontentSTRINGTerminalRuleCall_2_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:479:6: ( '_' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:479:6: ( '_' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:480:1: '_'
                    {
                     before(grammarAccess.getWMLMacroAccess().getTagcontent_Keyword_2_0_2()); 
                    match(input,9,FOLLOW_9_in_rule__WMLMacro__TagcontentAlternatives_2_0963); 
                     after(grammarAccess.getWMLMacroAccess().getTagcontent_Keyword_2_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:487:6: ( ':' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:487:6: ( ':' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:488:1: ':'
                    {
                     before(grammarAccess.getWMLMacroAccess().getTagcontentColonKeyword_2_0_3()); 
                    match(input,10,FOLLOW_10_in_rule__WMLMacro__TagcontentAlternatives_2_0983); 
                     after(grammarAccess.getWMLMacroAccess().getTagcontentColonKeyword_2_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:495:6: ( '-' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:495:6: ( '-' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:496:1: '-'
                    {
                     before(grammarAccess.getWMLMacroAccess().getTagcontentHyphenMinusKeyword_2_0_4()); 
                    match(input,11,FOLLOW_11_in_rule__WMLMacro__TagcontentAlternatives_2_01003); 
                     after(grammarAccess.getWMLMacroAccess().getTagcontentHyphenMinusKeyword_2_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:503:6: ( '.' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:503:6: ( '.' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:504:1: '.'
                    {
                     before(grammarAccess.getWMLMacroAccess().getTagcontentFullStopKeyword_2_0_5()); 
                    match(input,12,FOLLOW_12_in_rule__WMLMacro__TagcontentAlternatives_2_01023); 
                     after(grammarAccess.getWMLMacroAccess().getTagcontentFullStopKeyword_2_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:511:6: ( '(' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:511:6: ( '(' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:512:1: '('
                    {
                     before(grammarAccess.getWMLMacroAccess().getTagcontentLeftParenthesisKeyword_2_0_6()); 
                    match(input,13,FOLLOW_13_in_rule__WMLMacro__TagcontentAlternatives_2_01043); 
                     after(grammarAccess.getWMLMacroAccess().getTagcontentLeftParenthesisKeyword_2_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:519:6: ( ')' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:519:6: ( ')' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:520:1: ')'
                    {
                     before(grammarAccess.getWMLMacroAccess().getTagcontentRightParenthesisKeyword_2_0_7()); 
                    match(input,14,FOLLOW_14_in_rule__WMLMacro__TagcontentAlternatives_2_01063); 
                     after(grammarAccess.getWMLMacroAccess().getTagcontentRightParenthesisKeyword_2_0_7()); 

                    }


                    }
                    break;
                case 9 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:527:6: ( '=' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:527:6: ( '=' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:528:1: '='
                    {
                     before(grammarAccess.getWMLMacroAccess().getTagcontentEqualsSignKeyword_2_0_8()); 
                    match(input,15,FOLLOW_15_in_rule__WMLMacro__TagcontentAlternatives_2_01083); 
                     after(grammarAccess.getWMLMacroAccess().getTagcontentEqualsSignKeyword_2_0_8()); 

                    }


                    }
                    break;
                case 10 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:535:6: ( '/' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:535:6: ( '/' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:536:1: '/'
                    {
                     before(grammarAccess.getWMLMacroAccess().getTagcontentSolidusKeyword_2_0_9()); 
                    match(input,16,FOLLOW_16_in_rule__WMLMacro__TagcontentAlternatives_2_01103); 
                     after(grammarAccess.getWMLMacroAccess().getTagcontentSolidusKeyword_2_0_9()); 

                    }


                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacro__TagcontentAlternatives_2_0


    // $ANTLR start rule__WMLTag__Alternatives_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:548:1: rule__WMLTag__Alternatives_1 : ( ( ( rule__WMLTag__TtagsAssignment_1_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_1_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_1_2 ) ) );
    public final void rule__WMLTag__Alternatives_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:552:1: ( ( ( rule__WMLTag__TtagsAssignment_1_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_1_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_1_2 ) ) )
            int alt5=3;
            switch ( input.LA(1) ) {
            case 27:
                {
                alt5=1;
                }
                break;
            case RULE_ID:
                {
                alt5=2;
                }
                break;
            case 25:
                {
                alt5=3;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("548:1: rule__WMLTag__Alternatives_1 : ( ( ( rule__WMLTag__TtagsAssignment_1_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_1_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_1_2 ) ) );", 5, 0, input);

                throw nvae;
            }

            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:553:1: ( ( rule__WMLTag__TtagsAssignment_1_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:553:1: ( ( rule__WMLTag__TtagsAssignment_1_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:554:1: ( rule__WMLTag__TtagsAssignment_1_0 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTtagsAssignment_1_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:555:1: ( rule__WMLTag__TtagsAssignment_1_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:555:2: rule__WMLTag__TtagsAssignment_1_0
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TtagsAssignment_1_0_in_rule__WMLTag__Alternatives_11137);
                    rule__WMLTag__TtagsAssignment_1_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTtagsAssignment_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:559:6: ( ( rule__WMLTag__TkeysAssignment_1_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:559:6: ( ( rule__WMLTag__TkeysAssignment_1_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:560:1: ( rule__WMLTag__TkeysAssignment_1_1 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTkeysAssignment_1_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:561:1: ( rule__WMLTag__TkeysAssignment_1_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:561:2: rule__WMLTag__TkeysAssignment_1_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TkeysAssignment_1_1_in_rule__WMLTag__Alternatives_11155);
                    rule__WMLTag__TkeysAssignment_1_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTkeysAssignment_1_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:565:6: ( ( rule__WMLTag__TmacrosAssignment_1_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:565:6: ( ( rule__WMLTag__TmacrosAssignment_1_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:566:1: ( rule__WMLTag__TmacrosAssignment_1_2 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTmacrosAssignment_1_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:567:1: ( rule__WMLTag__TmacrosAssignment_1_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:567:2: rule__WMLTag__TmacrosAssignment_1_2
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TmacrosAssignment_1_2_in_rule__WMLTag__Alternatives_11173);
                    rule__WMLTag__TmacrosAssignment_1_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTmacrosAssignment_1_2()); 

                    }


                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__Alternatives_1


    // $ANTLR start rule__WMLKeyValue__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:576:1: rule__WMLKeyValue__Alternatives : ( ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) ) | ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) ) );
    public final void rule__WMLKeyValue__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:580:1: ( ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) ) | ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) ) )
            int alt6=2;
            int LA6_0 = input.LA(1);

            if ( ((LA6_0>=RULE_ID && LA6_0<=RULE_IINT)||LA6_0==9||(LA6_0>=17 && LA6_0<=24)||LA6_0==30) ) {
                alt6=1;
            }
            else if ( (LA6_0==25) ) {
                alt6=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("576:1: rule__WMLKeyValue__Alternatives : ( ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) ) | ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) ) );", 6, 0, input);

                throw nvae;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:581:1: ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:581:1: ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:582:1: ( rule__WMLKeyValue__Key1ValueAssignment_0 )
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueAssignment_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:583:1: ( rule__WMLKeyValue__Key1ValueAssignment_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:583:2: rule__WMLKeyValue__Key1ValueAssignment_0
                    {
                    pushFollow(FOLLOW_rule__WMLKeyValue__Key1ValueAssignment_0_in_rule__WMLKeyValue__Alternatives1206);
                    rule__WMLKeyValue__Key1ValueAssignment_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueAssignment_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:587:6: ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:587:6: ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:588:1: ( rule__WMLKeyValue__Key2ValueAssignment_1 )
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey2ValueAssignment_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:589:1: ( rule__WMLKeyValue__Key2ValueAssignment_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:589:2: rule__WMLKeyValue__Key2ValueAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLKeyValue__Key2ValueAssignment_1_in_rule__WMLKeyValue__Alternatives1224);
                    rule__WMLKeyValue__Key2ValueAssignment_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLKeyValueAccess().getKey2ValueAssignment_1()); 

                    }


                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKeyValue__Alternatives


    // $ANTLR start rule__WMLKeyValue__Key1ValueAlternatives_0_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:598:1: rule__WMLKeyValue__Key1ValueAlternatives_0_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFLOAT ) | ( RULE_IINT ) | ( rulePATH ) | ( ruleDIRECTION ) | ( ruleLIST ) | ( rulePROGRESSIVE ) );
    public final void rule__WMLKeyValue__Key1ValueAlternatives_0_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:602:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFLOAT ) | ( RULE_IINT ) | ( rulePATH ) | ( ruleDIRECTION ) | ( ruleLIST ) | ( rulePROGRESSIVE ) )
            int alt7=9;
            alt7 = dfa7.predict(input);
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:603:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:603:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:604:1: RULE_ID
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueIDTerminalRuleCall_0_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01257); 
                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueIDTerminalRuleCall_0_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:609:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:609:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:610:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueSTRINGTerminalRuleCall_0_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01274); 
                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueSTRINGTerminalRuleCall_0_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:615:6: ( ruleTSTRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:615:6: ( ruleTSTRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:616:1: ruleTSTRING
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueTSTRINGParserRuleCall_0_0_2()); 
                    pushFollow(FOLLOW_ruleTSTRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01291);
                    ruleTSTRING();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueTSTRINGParserRuleCall_0_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:621:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:621:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:622:1: ruleFLOAT
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueFLOATParserRuleCall_0_0_3()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01308);
                    ruleFLOAT();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueFLOATParserRuleCall_0_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:627:6: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:627:6: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:628:1: RULE_IINT
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueIINTTerminalRuleCall_0_0_4()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01325); 
                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueIINTTerminalRuleCall_0_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:633:6: ( rulePATH )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:633:6: ( rulePATH )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:634:1: rulePATH
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValuePATHParserRuleCall_0_0_5()); 
                    pushFollow(FOLLOW_rulePATH_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01342);
                    rulePATH();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValuePATHParserRuleCall_0_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:639:6: ( ruleDIRECTION )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:639:6: ( ruleDIRECTION )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:640:1: ruleDIRECTION
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueDIRECTIONParserRuleCall_0_0_6()); 
                    pushFollow(FOLLOW_ruleDIRECTION_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01359);
                    ruleDIRECTION();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueDIRECTIONParserRuleCall_0_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:645:6: ( ruleLIST )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:645:6: ( ruleLIST )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:646:1: ruleLIST
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueLISTParserRuleCall_0_0_7()); 
                    pushFollow(FOLLOW_ruleLIST_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01376);
                    ruleLIST();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueLISTParserRuleCall_0_0_7()); 

                    }


                    }
                    break;
                case 9 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:651:6: ( rulePROGRESSIVE )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:651:6: ( rulePROGRESSIVE )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:652:1: rulePROGRESSIVE
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValuePROGRESSIVEParserRuleCall_0_0_8()); 
                    pushFollow(FOLLOW_rulePROGRESSIVE_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01393);
                    rulePROGRESSIVE();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValuePROGRESSIVEParserRuleCall_0_0_8()); 

                    }


                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKeyValue__Key1ValueAlternatives_0_0


    // $ANTLR start rule__PATH__Alternatives_0_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:662:1: rule__PATH__Alternatives_0_1 : ( ( '-' ) | ( '/' ) );
    public final void rule__PATH__Alternatives_0_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:666:1: ( ( '-' ) | ( '/' ) )
            int alt8=2;
            int LA8_0 = input.LA(1);

            if ( (LA8_0==11) ) {
                alt8=1;
            }
            else if ( (LA8_0==16) ) {
                alt8=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("662:1: rule__PATH__Alternatives_0_1 : ( ( '-' ) | ( '/' ) );", 8, 0, input);

                throw nvae;
            }
            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:667:1: ( '-' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:667:1: ( '-' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:668:1: '-'
                    {
                     before(grammarAccess.getPATHAccess().getHyphenMinusKeyword_0_1_0()); 
                    match(input,11,FOLLOW_11_in_rule__PATH__Alternatives_0_11426); 
                     after(grammarAccess.getPATHAccess().getHyphenMinusKeyword_0_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:675:6: ( '/' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:675:6: ( '/' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:676:1: '/'
                    {
                     before(grammarAccess.getPATHAccess().getSolidusKeyword_0_1_1()); 
                    match(input,16,FOLLOW_16_in_rule__PATH__Alternatives_0_11446); 
                     after(grammarAccess.getPATHAccess().getSolidusKeyword_0_1_1()); 

                    }


                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Alternatives_0_1


    // $ANTLR start rule__DIRECTION__Alternatives_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:688:1: rule__DIRECTION__Alternatives_0 : ( ( 'n' ) | ( 's' ) | ( 'w' ) | ( 'e' ) | ( 'sw' ) | ( 'se' ) | ( 'ne' ) | ( 'nw' ) );
    public final void rule__DIRECTION__Alternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:692:1: ( ( 'n' ) | ( 's' ) | ( 'w' ) | ( 'e' ) | ( 'sw' ) | ( 'se' ) | ( 'ne' ) | ( 'nw' ) )
            int alt9=8;
            switch ( input.LA(1) ) {
            case 17:
                {
                alt9=1;
                }
                break;
            case 18:
                {
                alt9=2;
                }
                break;
            case 19:
                {
                alt9=3;
                }
                break;
            case 20:
                {
                alt9=4;
                }
                break;
            case 21:
                {
                alt9=5;
                }
                break;
            case 22:
                {
                alt9=6;
                }
                break;
            case 23:
                {
                alt9=7;
                }
                break;
            case 24:
                {
                alt9=8;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("688:1: rule__DIRECTION__Alternatives_0 : ( ( 'n' ) | ( 's' ) | ( 'w' ) | ( 'e' ) | ( 'sw' ) | ( 'se' ) | ( 'ne' ) | ( 'nw' ) );", 9, 0, input);

                throw nvae;
            }

            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:693:1: ( 'n' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:693:1: ( 'n' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:694:1: 'n'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getNKeyword_0_0()); 
                    match(input,17,FOLLOW_17_in_rule__DIRECTION__Alternatives_01481); 
                     after(grammarAccess.getDIRECTIONAccess().getNKeyword_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:701:6: ( 's' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:701:6: ( 's' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:702:1: 's'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getSKeyword_0_1()); 
                    match(input,18,FOLLOW_18_in_rule__DIRECTION__Alternatives_01501); 
                     after(grammarAccess.getDIRECTIONAccess().getSKeyword_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:709:6: ( 'w' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:709:6: ( 'w' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:710:1: 'w'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getWKeyword_0_2()); 
                    match(input,19,FOLLOW_19_in_rule__DIRECTION__Alternatives_01521); 
                     after(grammarAccess.getDIRECTIONAccess().getWKeyword_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:717:6: ( 'e' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:717:6: ( 'e' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:718:1: 'e'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getEKeyword_0_3()); 
                    match(input,20,FOLLOW_20_in_rule__DIRECTION__Alternatives_01541); 
                     after(grammarAccess.getDIRECTIONAccess().getEKeyword_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:725:6: ( 'sw' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:725:6: ( 'sw' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:726:1: 'sw'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getSwKeyword_0_4()); 
                    match(input,21,FOLLOW_21_in_rule__DIRECTION__Alternatives_01561); 
                     after(grammarAccess.getDIRECTIONAccess().getSwKeyword_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:733:6: ( 'se' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:733:6: ( 'se' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:734:1: 'se'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getSeKeyword_0_5()); 
                    match(input,22,FOLLOW_22_in_rule__DIRECTION__Alternatives_01581); 
                     after(grammarAccess.getDIRECTIONAccess().getSeKeyword_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:741:6: ( 'ne' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:741:6: ( 'ne' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:742:1: 'ne'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getNeKeyword_0_6()); 
                    match(input,23,FOLLOW_23_in_rule__DIRECTION__Alternatives_01601); 
                     after(grammarAccess.getDIRECTIONAccess().getNeKeyword_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:749:6: ( 'nw' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:749:6: ( 'nw' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:750:1: 'nw'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getNwKeyword_0_7()); 
                    match(input,24,FOLLOW_24_in_rule__DIRECTION__Alternatives_01621); 
                     after(grammarAccess.getDIRECTIONAccess().getNwKeyword_0_7()); 

                    }


                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__DIRECTION__Alternatives_0


    // $ANTLR start rule__PROGRESSIVE__Alternatives_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:762:1: rule__PROGRESSIVE__Alternatives_0 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:766:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
            int alt10=2;
            int LA10_0 = input.LA(1);

            if ( (LA10_0==RULE_IINT) ) {
                int LA10_1 = input.LA(2);

                if ( (LA10_1==10||(LA10_1>=31 && LA10_1<=32)) ) {
                    alt10=1;
                }
                else if ( (LA10_1==12) ) {
                    alt10=2;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("762:1: rule__PROGRESSIVE__Alternatives_0 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 10, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("762:1: rule__PROGRESSIVE__Alternatives_0 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 10, 0, input);

                throw nvae;
            }
            switch (alt10) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:767:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:767:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:768:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_0_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_01655); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:773:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:773:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:774:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_0_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_01672);
                    ruleFLOAT();
                    _fsp--;

                     after(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_0_1()); 

                    }


                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Alternatives_0


    // $ANTLR start rule__PROGRESSIVE__Alternatives_1_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:784:1: rule__PROGRESSIVE__Alternatives_1_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_1_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:788:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
            int alt11=2;
            int LA11_0 = input.LA(1);

            if ( (LA11_0==RULE_IINT) ) {
                int LA11_1 = input.LA(2);

                if ( (LA11_1==10||LA11_1==31) ) {
                    alt11=1;
                }
                else if ( (LA11_1==12) ) {
                    alt11=2;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("784:1: rule__PROGRESSIVE__Alternatives_1_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 11, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("784:1: rule__PROGRESSIVE__Alternatives_1_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 11, 0, input);

                throw nvae;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:789:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:789:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:790:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_1_1_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_1_11704); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_1_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:795:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:795:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:796:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_1_1_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_1_11721);
                    ruleFLOAT();
                    _fsp--;

                     after(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_1_1_1()); 

                    }


                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Alternatives_1_1


    // $ANTLR start rule__PROGRESSIVE__Alternatives_3_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:806:1: rule__PROGRESSIVE__Alternatives_3_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_3_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:810:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0==RULE_IINT) ) {
                int LA12_1 = input.LA(2);

                if ( (LA12_1==12) ) {
                    alt12=2;
                }
                else if ( (LA12_1==EOF||LA12_1==RULE_ID||LA12_1==10||LA12_1==25||LA12_1==27||LA12_1==29||(LA12_1>=31 && LA12_1<=32)) ) {
                    alt12=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("806:1: rule__PROGRESSIVE__Alternatives_3_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 12, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("806:1: rule__PROGRESSIVE__Alternatives_3_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 12, 0, input);

                throw nvae;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:811:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:811:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:812:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_1_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_11753); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:817:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:817:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:818:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_1_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_11770);
                    ruleFLOAT();
                    _fsp--;

                     after(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_1_1()); 

                    }


                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Alternatives_3_1


    // $ANTLR start rule__PROGRESSIVE__Alternatives_3_2_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:828:1: rule__PROGRESSIVE__Alternatives_3_2_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_3_2_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:832:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
            int alt13=2;
            int LA13_0 = input.LA(1);

            if ( (LA13_0==RULE_IINT) ) {
                int LA13_1 = input.LA(2);

                if ( (LA13_1==EOF||LA13_1==RULE_ID||LA13_1==10||LA13_1==25||LA13_1==27||LA13_1==29||LA13_1==31) ) {
                    alt13=1;
                }
                else if ( (LA13_1==12) ) {
                    alt13=2;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("828:1: rule__PROGRESSIVE__Alternatives_3_2_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 13, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("828:1: rule__PROGRESSIVE__Alternatives_3_2_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 13, 0, input);

                throw nvae;
            }
            switch (alt13) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:833:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:833:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:834:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_2_1_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_2_11802); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_2_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:839:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:839:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:840:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_2_1_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_2_11819);
                    ruleFLOAT();
                    _fsp--;

                     after(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_2_1_1()); 

                    }


                    }
                    break;

            }
        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Alternatives_3_2_1


    // $ANTLR start rule__WMLMacro__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:852:1: rule__WMLMacro__Group__0 : rule__WMLMacro__Group__0__Impl rule__WMLMacro__Group__1 ;
    public final void rule__WMLMacro__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:856:1: ( rule__WMLMacro__Group__0__Impl rule__WMLMacro__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:857:2: rule__WMLMacro__Group__0__Impl rule__WMLMacro__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__0__Impl_in_rule__WMLMacro__Group__01849);
            rule__WMLMacro__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacro__Group__1_in_rule__WMLMacro__Group__01852);
            rule__WMLMacro__Group__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacro__Group__0


    // $ANTLR start rule__WMLMacro__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:864:1: rule__WMLMacro__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacro__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:868:1: ( ( '{' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:869:1: ( '{' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:869:1: ( '{' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:870:1: '{'
            {
             before(grammarAccess.getWMLMacroAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,25,FOLLOW_25_in_rule__WMLMacro__Group__0__Impl1880); 
             after(grammarAccess.getWMLMacroAccess().getLeftCurlyBracketKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacro__Group__0__Impl


    // $ANTLR start rule__WMLMacro__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:883:1: rule__WMLMacro__Group__1 : rule__WMLMacro__Group__1__Impl rule__WMLMacro__Group__2 ;
    public final void rule__WMLMacro__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:887:1: ( rule__WMLMacro__Group__1__Impl rule__WMLMacro__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:888:2: rule__WMLMacro__Group__1__Impl rule__WMLMacro__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__1__Impl_in_rule__WMLMacro__Group__11911);
            rule__WMLMacro__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacro__Group__2_in_rule__WMLMacro__Group__11914);
            rule__WMLMacro__Group__2();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacro__Group__1


    // $ANTLR start rule__WMLMacro__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:895:1: rule__WMLMacro__Group__1__Impl : ( ( rule__WMLMacro__MacroNameAssignment_1 ) ) ;
    public final void rule__WMLMacro__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:899:1: ( ( ( rule__WMLMacro__MacroNameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:900:1: ( ( rule__WMLMacro__MacroNameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:900:1: ( ( rule__WMLMacro__MacroNameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:901:1: ( rule__WMLMacro__MacroNameAssignment_1 )
            {
             before(grammarAccess.getWMLMacroAccess().getMacroNameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:902:1: ( rule__WMLMacro__MacroNameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:902:2: rule__WMLMacro__MacroNameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLMacro__MacroNameAssignment_1_in_rule__WMLMacro__Group__1__Impl1941);
            rule__WMLMacro__MacroNameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroAccess().getMacroNameAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacro__Group__1__Impl


    // $ANTLR start rule__WMLMacro__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:912:1: rule__WMLMacro__Group__2 : rule__WMLMacro__Group__2__Impl rule__WMLMacro__Group__3 ;
    public final void rule__WMLMacro__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:916:1: ( rule__WMLMacro__Group__2__Impl rule__WMLMacro__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:917:2: rule__WMLMacro__Group__2__Impl rule__WMLMacro__Group__3
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__2__Impl_in_rule__WMLMacro__Group__21971);
            rule__WMLMacro__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacro__Group__3_in_rule__WMLMacro__Group__21974);
            rule__WMLMacro__Group__3();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacro__Group__2


    // $ANTLR start rule__WMLMacro__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:924:1: rule__WMLMacro__Group__2__Impl : ( ( rule__WMLMacro__TagcontentAssignment_2 )* ) ;
    public final void rule__WMLMacro__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:928:1: ( ( ( rule__WMLMacro__TagcontentAssignment_2 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:929:1: ( ( rule__WMLMacro__TagcontentAssignment_2 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:929:1: ( ( rule__WMLMacro__TagcontentAssignment_2 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:930:1: ( rule__WMLMacro__TagcontentAssignment_2 )*
            {
             before(grammarAccess.getWMLMacroAccess().getTagcontentAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:931:1: ( rule__WMLMacro__TagcontentAssignment_2 )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( ((LA14_0>=RULE_ID && LA14_0<=RULE_STRING)||(LA14_0>=9 && LA14_0<=16)) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:931:2: rule__WMLMacro__TagcontentAssignment_2
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacro__TagcontentAssignment_2_in_rule__WMLMacro__Group__2__Impl2001);
            	    rule__WMLMacro__TagcontentAssignment_2();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop14;
                }
            } while (true);

             after(grammarAccess.getWMLMacroAccess().getTagcontentAssignment_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacro__Group__2__Impl


    // $ANTLR start rule__WMLMacro__Group__3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:941:1: rule__WMLMacro__Group__3 : rule__WMLMacro__Group__3__Impl ;
    public final void rule__WMLMacro__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:945:1: ( rule__WMLMacro__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:946:2: rule__WMLMacro__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__3__Impl_in_rule__WMLMacro__Group__32032);
            rule__WMLMacro__Group__3__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacro__Group__3


    // $ANTLR start rule__WMLMacro__Group__3__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:952:1: rule__WMLMacro__Group__3__Impl : ( '}' ) ;
    public final void rule__WMLMacro__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:956:1: ( ( '}' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:957:1: ( '}' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:957:1: ( '}' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:958:1: '}'
            {
             before(grammarAccess.getWMLMacroAccess().getRightCurlyBracketKeyword_3()); 
            match(input,26,FOLLOW_26_in_rule__WMLMacro__Group__3__Impl2060); 
             after(grammarAccess.getWMLMacroAccess().getRightCurlyBracketKeyword_3()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacro__Group__3__Impl


    // $ANTLR start rule__WMLTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:979:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:983:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:984:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__02099);
            rule__WMLTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__02102);
            rule__WMLTag__Group__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__Group__0


    // $ANTLR start rule__WMLTag__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:991:1: rule__WMLTag__Group__0__Impl : ( ( rule__WMLTag__StartAssignment_0 ) ) ;
    public final void rule__WMLTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:995:1: ( ( ( rule__WMLTag__StartAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:996:1: ( ( rule__WMLTag__StartAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:996:1: ( ( rule__WMLTag__StartAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:997:1: ( rule__WMLTag__StartAssignment_0 )
            {
             before(grammarAccess.getWMLTagAccess().getStartAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:998:1: ( rule__WMLTag__StartAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:998:2: rule__WMLTag__StartAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLTag__StartAssignment_0_in_rule__WMLTag__Group__0__Impl2129);
            rule__WMLTag__StartAssignment_0();
            _fsp--;


            }

             after(grammarAccess.getWMLTagAccess().getStartAssignment_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__Group__0__Impl


    // $ANTLR start rule__WMLTag__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1008:1: rule__WMLTag__Group__1 : rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 ;
    public final void rule__WMLTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1012:1: ( rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1013:2: rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__12159);
            rule__WMLTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__12162);
            rule__WMLTag__Group__2();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__Group__1


    // $ANTLR start rule__WMLTag__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1020:1: rule__WMLTag__Group__1__Impl : ( ( rule__WMLTag__Alternatives_1 )* ) ;
    public final void rule__WMLTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1024:1: ( ( ( rule__WMLTag__Alternatives_1 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1025:1: ( ( rule__WMLTag__Alternatives_1 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1025:1: ( ( rule__WMLTag__Alternatives_1 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1026:1: ( rule__WMLTag__Alternatives_1 )*
            {
             before(grammarAccess.getWMLTagAccess().getAlternatives_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1027:1: ( rule__WMLTag__Alternatives_1 )*
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( (LA15_0==RULE_ID||LA15_0==25||LA15_0==27) ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1027:2: rule__WMLTag__Alternatives_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLTag__Alternatives_1_in_rule__WMLTag__Group__1__Impl2189);
            	    rule__WMLTag__Alternatives_1();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop15;
                }
            } while (true);

             after(grammarAccess.getWMLTagAccess().getAlternatives_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__Group__1__Impl


    // $ANTLR start rule__WMLTag__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1037:1: rule__WMLTag__Group__2 : rule__WMLTag__Group__2__Impl ;
    public final void rule__WMLTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1041:1: ( rule__WMLTag__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1042:2: rule__WMLTag__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__22220);
            rule__WMLTag__Group__2__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__Group__2


    // $ANTLR start rule__WMLTag__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1048:1: rule__WMLTag__Group__2__Impl : ( ( rule__WMLTag__EndAssignment_2 ) ) ;
    public final void rule__WMLTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1052:1: ( ( ( rule__WMLTag__EndAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1053:1: ( ( rule__WMLTag__EndAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1053:1: ( ( rule__WMLTag__EndAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1054:1: ( rule__WMLTag__EndAssignment_2 )
            {
             before(grammarAccess.getWMLTagAccess().getEndAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1055:1: ( rule__WMLTag__EndAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1055:2: rule__WMLTag__EndAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLTag__EndAssignment_2_in_rule__WMLTag__Group__2__Impl2247);
            rule__WMLTag__EndAssignment_2();
            _fsp--;


            }

             after(grammarAccess.getWMLTagAccess().getEndAssignment_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__Group__2__Impl


    // $ANTLR start rule__WMLStartTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1071:1: rule__WMLStartTag__Group__0 : rule__WMLStartTag__Group__0__Impl rule__WMLStartTag__Group__1 ;
    public final void rule__WMLStartTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1075:1: ( rule__WMLStartTag__Group__0__Impl rule__WMLStartTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1076:2: rule__WMLStartTag__Group__0__Impl rule__WMLStartTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLStartTag__Group__0__Impl_in_rule__WMLStartTag__Group__02283);
            rule__WMLStartTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLStartTag__Group__1_in_rule__WMLStartTag__Group__02286);
            rule__WMLStartTag__Group__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLStartTag__Group__0


    // $ANTLR start rule__WMLStartTag__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1083:1: rule__WMLStartTag__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLStartTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1087:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1088:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1088:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1089:1: '['
            {
             before(grammarAccess.getWMLStartTagAccess().getLeftSquareBracketKeyword_0()); 
            match(input,27,FOLLOW_27_in_rule__WMLStartTag__Group__0__Impl2314); 
             after(grammarAccess.getWMLStartTagAccess().getLeftSquareBracketKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLStartTag__Group__0__Impl


    // $ANTLR start rule__WMLStartTag__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1102:1: rule__WMLStartTag__Group__1 : rule__WMLStartTag__Group__1__Impl rule__WMLStartTag__Group__2 ;
    public final void rule__WMLStartTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1106:1: ( rule__WMLStartTag__Group__1__Impl rule__WMLStartTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1107:2: rule__WMLStartTag__Group__1__Impl rule__WMLStartTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLStartTag__Group__1__Impl_in_rule__WMLStartTag__Group__12345);
            rule__WMLStartTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLStartTag__Group__2_in_rule__WMLStartTag__Group__12348);
            rule__WMLStartTag__Group__2();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLStartTag__Group__1


    // $ANTLR start rule__WMLStartTag__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1114:1: rule__WMLStartTag__Group__1__Impl : ( ( rule__WMLStartTag__TagnameAssignment_1 ) ) ;
    public final void rule__WMLStartTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1118:1: ( ( ( rule__WMLStartTag__TagnameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1119:1: ( ( rule__WMLStartTag__TagnameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1119:1: ( ( rule__WMLStartTag__TagnameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1120:1: ( rule__WMLStartTag__TagnameAssignment_1 )
            {
             before(grammarAccess.getWMLStartTagAccess().getTagnameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1121:1: ( rule__WMLStartTag__TagnameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1121:2: rule__WMLStartTag__TagnameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLStartTag__TagnameAssignment_1_in_rule__WMLStartTag__Group__1__Impl2375);
            rule__WMLStartTag__TagnameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLStartTagAccess().getTagnameAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLStartTag__Group__1__Impl


    // $ANTLR start rule__WMLStartTag__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1131:1: rule__WMLStartTag__Group__2 : rule__WMLStartTag__Group__2__Impl ;
    public final void rule__WMLStartTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1135:1: ( rule__WMLStartTag__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1136:2: rule__WMLStartTag__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLStartTag__Group__2__Impl_in_rule__WMLStartTag__Group__22405);
            rule__WMLStartTag__Group__2__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLStartTag__Group__2


    // $ANTLR start rule__WMLStartTag__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1142:1: rule__WMLStartTag__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLStartTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1146:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1147:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1147:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1148:1: ']'
            {
             before(grammarAccess.getWMLStartTagAccess().getRightSquareBracketKeyword_2()); 
            match(input,28,FOLLOW_28_in_rule__WMLStartTag__Group__2__Impl2433); 
             after(grammarAccess.getWMLStartTagAccess().getRightSquareBracketKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLStartTag__Group__2__Impl


    // $ANTLR start rule__WMLEndTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1167:1: rule__WMLEndTag__Group__0 : rule__WMLEndTag__Group__0__Impl rule__WMLEndTag__Group__1 ;
    public final void rule__WMLEndTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1171:1: ( rule__WMLEndTag__Group__0__Impl rule__WMLEndTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1172:2: rule__WMLEndTag__Group__0__Impl rule__WMLEndTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLEndTag__Group__0__Impl_in_rule__WMLEndTag__Group__02470);
            rule__WMLEndTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLEndTag__Group__1_in_rule__WMLEndTag__Group__02473);
            rule__WMLEndTag__Group__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLEndTag__Group__0


    // $ANTLR start rule__WMLEndTag__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1179:1: rule__WMLEndTag__Group__0__Impl : ( '[/' ) ;
    public final void rule__WMLEndTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1183:1: ( ( '[/' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1184:1: ( '[/' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1184:1: ( '[/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1185:1: '[/'
            {
             before(grammarAccess.getWMLEndTagAccess().getLeftSquareBracketSolidusKeyword_0()); 
            match(input,29,FOLLOW_29_in_rule__WMLEndTag__Group__0__Impl2501); 
             after(grammarAccess.getWMLEndTagAccess().getLeftSquareBracketSolidusKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLEndTag__Group__0__Impl


    // $ANTLR start rule__WMLEndTag__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1198:1: rule__WMLEndTag__Group__1 : rule__WMLEndTag__Group__1__Impl rule__WMLEndTag__Group__2 ;
    public final void rule__WMLEndTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1202:1: ( rule__WMLEndTag__Group__1__Impl rule__WMLEndTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1203:2: rule__WMLEndTag__Group__1__Impl rule__WMLEndTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLEndTag__Group__1__Impl_in_rule__WMLEndTag__Group__12532);
            rule__WMLEndTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLEndTag__Group__2_in_rule__WMLEndTag__Group__12535);
            rule__WMLEndTag__Group__2();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLEndTag__Group__1


    // $ANTLR start rule__WMLEndTag__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1210:1: rule__WMLEndTag__Group__1__Impl : ( ( rule__WMLEndTag__TagnameAssignment_1 ) ) ;
    public final void rule__WMLEndTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1214:1: ( ( ( rule__WMLEndTag__TagnameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1215:1: ( ( rule__WMLEndTag__TagnameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1215:1: ( ( rule__WMLEndTag__TagnameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1216:1: ( rule__WMLEndTag__TagnameAssignment_1 )
            {
             before(grammarAccess.getWMLEndTagAccess().getTagnameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1217:1: ( rule__WMLEndTag__TagnameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1217:2: rule__WMLEndTag__TagnameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLEndTag__TagnameAssignment_1_in_rule__WMLEndTag__Group__1__Impl2562);
            rule__WMLEndTag__TagnameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLEndTagAccess().getTagnameAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLEndTag__Group__1__Impl


    // $ANTLR start rule__WMLEndTag__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1227:1: rule__WMLEndTag__Group__2 : rule__WMLEndTag__Group__2__Impl ;
    public final void rule__WMLEndTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1231:1: ( rule__WMLEndTag__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1232:2: rule__WMLEndTag__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLEndTag__Group__2__Impl_in_rule__WMLEndTag__Group__22592);
            rule__WMLEndTag__Group__2__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLEndTag__Group__2


    // $ANTLR start rule__WMLEndTag__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1238:1: rule__WMLEndTag__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLEndTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1242:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1243:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1243:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1244:1: ']'
            {
             before(grammarAccess.getWMLEndTagAccess().getRightSquareBracketKeyword_2()); 
            match(input,28,FOLLOW_28_in_rule__WMLEndTag__Group__2__Impl2620); 
             after(grammarAccess.getWMLEndTagAccess().getRightSquareBracketKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLEndTag__Group__2__Impl


    // $ANTLR start rule__WMLKey__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1263:1: rule__WMLKey__Group__0 : rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 ;
    public final void rule__WMLKey__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1267:1: ( rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1268:2: rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02657);
            rule__WMLKey__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02660);
            rule__WMLKey__Group__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__Group__0


    // $ANTLR start rule__WMLKey__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1275:1: rule__WMLKey__Group__0__Impl : ( ( rule__WMLKey__KeyNameAssignment_0 ) ) ;
    public final void rule__WMLKey__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1279:1: ( ( ( rule__WMLKey__KeyNameAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1280:1: ( ( rule__WMLKey__KeyNameAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1280:1: ( ( rule__WMLKey__KeyNameAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1281:1: ( rule__WMLKey__KeyNameAssignment_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getKeyNameAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1282:1: ( rule__WMLKey__KeyNameAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1282:2: rule__WMLKey__KeyNameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLKey__KeyNameAssignment_0_in_rule__WMLKey__Group__0__Impl2687);
            rule__WMLKey__KeyNameAssignment_0();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getKeyNameAssignment_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__Group__0__Impl


    // $ANTLR start rule__WMLKey__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1292:1: rule__WMLKey__Group__1 : rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 ;
    public final void rule__WMLKey__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1296:1: ( rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1297:2: rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12717);
            rule__WMLKey__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12720);
            rule__WMLKey__Group__2();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__Group__1


    // $ANTLR start rule__WMLKey__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1304:1: rule__WMLKey__Group__1__Impl : ( '=' ) ;
    public final void rule__WMLKey__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1308:1: ( ( '=' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1309:1: ( '=' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1309:1: ( '=' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1310:1: '='
            {
             before(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1()); 
            match(input,15,FOLLOW_15_in_rule__WMLKey__Group__1__Impl2748); 
             after(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__Group__1__Impl


    // $ANTLR start rule__WMLKey__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1323:1: rule__WMLKey__Group__2 : rule__WMLKey__Group__2__Impl ;
    public final void rule__WMLKey__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1327:1: ( rule__WMLKey__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1328:2: rule__WMLKey__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22779);
            rule__WMLKey__Group__2__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__Group__2


    // $ANTLR start rule__WMLKey__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1334:1: rule__WMLKey__Group__2__Impl : ( ( rule__WMLKey__ValueAssignment_2 ) ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1338:1: ( ( ( rule__WMLKey__ValueAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1339:1: ( ( rule__WMLKey__ValueAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1339:1: ( ( rule__WMLKey__ValueAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1340:1: ( rule__WMLKey__ValueAssignment_2 )
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1341:1: ( rule__WMLKey__ValueAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1341:2: rule__WMLKey__ValueAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2806);
            rule__WMLKey__ValueAssignment_2();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__Group__2__Impl


    // $ANTLR start rule__FLOAT__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1357:1: rule__FLOAT__Group__0 : rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1 ;
    public final void rule__FLOAT__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1361:1: ( rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1362:2: rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__0__Impl_in_rule__FLOAT__Group__02842);
            rule__FLOAT__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__FLOAT__Group__1_in_rule__FLOAT__Group__02845);
            rule__FLOAT__Group__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__FLOAT__Group__0


    // $ANTLR start rule__FLOAT__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1369:1: rule__FLOAT__Group__0__Impl : ( RULE_IINT ) ;
    public final void rule__FLOAT__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1373:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1374:1: ( RULE_IINT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1374:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1375:1: RULE_IINT
            {
             before(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_0()); 
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__FLOAT__Group__0__Impl2872); 
             after(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__FLOAT__Group__0__Impl


    // $ANTLR start rule__FLOAT__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1386:1: rule__FLOAT__Group__1 : rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2 ;
    public final void rule__FLOAT__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1390:1: ( rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1391:2: rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__1__Impl_in_rule__FLOAT__Group__12901);
            rule__FLOAT__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__FLOAT__Group__2_in_rule__FLOAT__Group__12904);
            rule__FLOAT__Group__2();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__FLOAT__Group__1


    // $ANTLR start rule__FLOAT__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1398:1: rule__FLOAT__Group__1__Impl : ( '.' ) ;
    public final void rule__FLOAT__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1402:1: ( ( '.' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1403:1: ( '.' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1403:1: ( '.' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1404:1: '.'
            {
             before(grammarAccess.getFLOATAccess().getFullStopKeyword_1()); 
            match(input,12,FOLLOW_12_in_rule__FLOAT__Group__1__Impl2932); 
             after(grammarAccess.getFLOATAccess().getFullStopKeyword_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__FLOAT__Group__1__Impl


    // $ANTLR start rule__FLOAT__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1417:1: rule__FLOAT__Group__2 : rule__FLOAT__Group__2__Impl ;
    public final void rule__FLOAT__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1421:1: ( rule__FLOAT__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1422:2: rule__FLOAT__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__2__Impl_in_rule__FLOAT__Group__22963);
            rule__FLOAT__Group__2__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__FLOAT__Group__2


    // $ANTLR start rule__FLOAT__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1428:1: rule__FLOAT__Group__2__Impl : ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) ) ;
    public final void rule__FLOAT__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1432:1: ( ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1433:1: ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1433:1: ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1434:1: ( ( RULE_IINT ) ) ( ( RULE_IINT )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1434:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1435:1: ( RULE_IINT )
            {
             before(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1436:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1436:3: RULE_IINT
            {
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2993); 

            }

             after(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1439:1: ( ( RULE_IINT )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1440:1: ( RULE_IINT )*
            {
             before(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1441:1: ( RULE_IINT )*
            loop16:
            do {
                int alt16=2;
                int LA16_0 = input.LA(1);

                if ( (LA16_0==RULE_IINT) ) {
                    alt16=1;
                }


                switch (alt16) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1441:3: RULE_IINT
            	    {
            	    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl3006); 

            	    }
            	    break;

            	default :
            	    break loop16;
                }
            } while (true);

             after(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 

            }


            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__FLOAT__Group__2__Impl


    // $ANTLR start rule__TSTRING__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1458:1: rule__TSTRING__Group__0 : rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 ;
    public final void rule__TSTRING__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1462:1: ( rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1463:2: rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__03045);
            rule__TSTRING__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__03048);
            rule__TSTRING__Group__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TSTRING__Group__0


    // $ANTLR start rule__TSTRING__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1470:1: rule__TSTRING__Group__0__Impl : ( ( rule__TSTRING__Group_0__0 ) ) ;
    public final void rule__TSTRING__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1474:1: ( ( ( rule__TSTRING__Group_0__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1475:1: ( ( rule__TSTRING__Group_0__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1475:1: ( ( rule__TSTRING__Group_0__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1476:1: ( rule__TSTRING__Group_0__0 )
            {
             before(grammarAccess.getTSTRINGAccess().getGroup_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1477:1: ( rule__TSTRING__Group_0__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1477:2: rule__TSTRING__Group_0__0
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__0_in_rule__TSTRING__Group__0__Impl3075);
            rule__TSTRING__Group_0__0();
            _fsp--;


            }

             after(grammarAccess.getTSTRINGAccess().getGroup_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TSTRING__Group__0__Impl


    // $ANTLR start rule__TSTRING__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1487:1: rule__TSTRING__Group__1 : rule__TSTRING__Group__1__Impl ;
    public final void rule__TSTRING__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1491:1: ( rule__TSTRING__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1492:2: rule__TSTRING__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__13105);
            rule__TSTRING__Group__1__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TSTRING__Group__1


    // $ANTLR start rule__TSTRING__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1498:1: rule__TSTRING__Group__1__Impl : ( RULE_STRING ) ;
    public final void rule__TSTRING__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1502:1: ( ( RULE_STRING ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1503:1: ( RULE_STRING )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1503:1: ( RULE_STRING )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1504:1: RULE_STRING
            {
             before(grammarAccess.getTSTRINGAccess().getSTRINGTerminalRuleCall_1()); 
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl3132); 
             after(grammarAccess.getTSTRINGAccess().getSTRINGTerminalRuleCall_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TSTRING__Group__1__Impl


    // $ANTLR start rule__TSTRING__Group_0__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1519:1: rule__TSTRING__Group_0__0 : rule__TSTRING__Group_0__0__Impl rule__TSTRING__Group_0__1 ;
    public final void rule__TSTRING__Group_0__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1523:1: ( rule__TSTRING__Group_0__0__Impl rule__TSTRING__Group_0__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1524:2: rule__TSTRING__Group_0__0__Impl rule__TSTRING__Group_0__1
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__0__Impl_in_rule__TSTRING__Group_0__03165);
            rule__TSTRING__Group_0__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group_0__1_in_rule__TSTRING__Group_0__03168);
            rule__TSTRING__Group_0__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TSTRING__Group_0__0


    // $ANTLR start rule__TSTRING__Group_0__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1531:1: rule__TSTRING__Group_0__0__Impl : ( ( ' ' )? ) ;
    public final void rule__TSTRING__Group_0__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1535:1: ( ( ( ' ' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1536:1: ( ( ' ' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1536:1: ( ( ' ' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1537:1: ( ' ' )?
            {
             before(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1538:1: ( ' ' )?
            int alt17=2;
            int LA17_0 = input.LA(1);

            if ( (LA17_0==30) ) {
                alt17=1;
            }
            switch (alt17) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1539:2: ' '
                    {
                    match(input,30,FOLLOW_30_in_rule__TSTRING__Group_0__0__Impl3197); 

                    }
                    break;

            }

             after(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TSTRING__Group_0__0__Impl


    // $ANTLR start rule__TSTRING__Group_0__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1550:1: rule__TSTRING__Group_0__1 : rule__TSTRING__Group_0__1__Impl rule__TSTRING__Group_0__2 ;
    public final void rule__TSTRING__Group_0__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1554:1: ( rule__TSTRING__Group_0__1__Impl rule__TSTRING__Group_0__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1555:2: rule__TSTRING__Group_0__1__Impl rule__TSTRING__Group_0__2
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__1__Impl_in_rule__TSTRING__Group_0__13230);
            rule__TSTRING__Group_0__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group_0__2_in_rule__TSTRING__Group_0__13233);
            rule__TSTRING__Group_0__2();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TSTRING__Group_0__1


    // $ANTLR start rule__TSTRING__Group_0__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1562:1: rule__TSTRING__Group_0__1__Impl : ( '_' ) ;
    public final void rule__TSTRING__Group_0__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1566:1: ( ( '_' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1567:1: ( '_' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1567:1: ( '_' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1568:1: '_'
            {
             before(grammarAccess.getTSTRINGAccess().get_Keyword_0_1()); 
            match(input,9,FOLLOW_9_in_rule__TSTRING__Group_0__1__Impl3261); 
             after(grammarAccess.getTSTRINGAccess().get_Keyword_0_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TSTRING__Group_0__1__Impl


    // $ANTLR start rule__TSTRING__Group_0__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1581:1: rule__TSTRING__Group_0__2 : rule__TSTRING__Group_0__2__Impl ;
    public final void rule__TSTRING__Group_0__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1585:1: ( rule__TSTRING__Group_0__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1586:2: rule__TSTRING__Group_0__2__Impl
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__2__Impl_in_rule__TSTRING__Group_0__23292);
            rule__TSTRING__Group_0__2__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TSTRING__Group_0__2


    // $ANTLR start rule__TSTRING__Group_0__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1592:1: rule__TSTRING__Group_0__2__Impl : ( ( ' ' )? ) ;
    public final void rule__TSTRING__Group_0__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1596:1: ( ( ( ' ' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1597:1: ( ( ' ' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1597:1: ( ( ' ' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1598:1: ( ' ' )?
            {
             before(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1599:1: ( ' ' )?
            int alt18=2;
            int LA18_0 = input.LA(1);

            if ( (LA18_0==30) ) {
                alt18=1;
            }
            switch (alt18) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1600:2: ' '
                    {
                    match(input,30,FOLLOW_30_in_rule__TSTRING__Group_0__2__Impl3321); 

                    }
                    break;

            }

             after(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TSTRING__Group_0__2__Impl


    // $ANTLR start rule__PATH__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1617:1: rule__PATH__Group__0 : rule__PATH__Group__0__Impl rule__PATH__Group__1 ;
    public final void rule__PATH__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1621:1: ( rule__PATH__Group__0__Impl rule__PATH__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1622:2: rule__PATH__Group__0__Impl rule__PATH__Group__1
            {
            pushFollow(FOLLOW_rule__PATH__Group__0__Impl_in_rule__PATH__Group__03360);
            rule__PATH__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group__1_in_rule__PATH__Group__03363);
            rule__PATH__Group__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Group__0


    // $ANTLR start rule__PATH__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1629:1: rule__PATH__Group__0__Impl : ( ( rule__PATH__Group_0__0 )* ) ;
    public final void rule__PATH__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1633:1: ( ( ( rule__PATH__Group_0__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1634:1: ( ( rule__PATH__Group_0__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1634:1: ( ( rule__PATH__Group_0__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1635:1: ( rule__PATH__Group_0__0 )*
            {
             before(grammarAccess.getPATHAccess().getGroup_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1636:1: ( rule__PATH__Group_0__0 )*
            loop19:
            do {
                int alt19=2;
                alt19 = dfa19.predict(input);
                switch (alt19) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1636:2: rule__PATH__Group_0__0
            	    {
            	    pushFollow(FOLLOW_rule__PATH__Group_0__0_in_rule__PATH__Group__0__Impl3390);
            	    rule__PATH__Group_0__0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop19;
                }
            } while (true);

             after(grammarAccess.getPATHAccess().getGroup_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Group__0__Impl


    // $ANTLR start rule__PATH__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1646:1: rule__PATH__Group__1 : rule__PATH__Group__1__Impl rule__PATH__Group__2 ;
    public final void rule__PATH__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1650:1: ( rule__PATH__Group__1__Impl rule__PATH__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1651:2: rule__PATH__Group__1__Impl rule__PATH__Group__2
            {
            pushFollow(FOLLOW_rule__PATH__Group__1__Impl_in_rule__PATH__Group__13421);
            rule__PATH__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group__2_in_rule__PATH__Group__13424);
            rule__PATH__Group__2();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Group__1


    // $ANTLR start rule__PATH__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1658:1: rule__PATH__Group__1__Impl : ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) ;
    public final void rule__PATH__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1662:1: ( ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1663:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1663:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1664:1: ( ( RULE_ID ) ) ( ( RULE_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1664:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1665:1: ( RULE_ID )
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1666:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1666:3: RULE_ID
            {
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl3454); 

            }

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1669:1: ( ( RULE_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1670:1: ( RULE_ID )*
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1671:1: ( RULE_ID )*
            loop20:
            do {
                int alt20=2;
                int LA20_0 = input.LA(1);

                if ( (LA20_0==RULE_ID) ) {
                    alt20=1;
                }


                switch (alt20) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1671:3: RULE_ID
            	    {
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl3467); 

            	    }
            	    break;

            	default :
            	    break loop20;
                }
            } while (true);

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 

            }


            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Group__1__Impl


    // $ANTLR start rule__PATH__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1682:1: rule__PATH__Group__2 : rule__PATH__Group__2__Impl rule__PATH__Group__3 ;
    public final void rule__PATH__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1686:1: ( rule__PATH__Group__2__Impl rule__PATH__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1687:2: rule__PATH__Group__2__Impl rule__PATH__Group__3
            {
            pushFollow(FOLLOW_rule__PATH__Group__2__Impl_in_rule__PATH__Group__23500);
            rule__PATH__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group__3_in_rule__PATH__Group__23503);
            rule__PATH__Group__3();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Group__2


    // $ANTLR start rule__PATH__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1694:1: rule__PATH__Group__2__Impl : ( '.' ) ;
    public final void rule__PATH__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1698:1: ( ( '.' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1699:1: ( '.' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1699:1: ( '.' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1700:1: '.'
            {
             before(grammarAccess.getPATHAccess().getFullStopKeyword_2()); 
            match(input,12,FOLLOW_12_in_rule__PATH__Group__2__Impl3531); 
             after(grammarAccess.getPATHAccess().getFullStopKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Group__2__Impl


    // $ANTLR start rule__PATH__Group__3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1713:1: rule__PATH__Group__3 : rule__PATH__Group__3__Impl ;
    public final void rule__PATH__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1717:1: ( rule__PATH__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1718:2: rule__PATH__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__PATH__Group__3__Impl_in_rule__PATH__Group__33562);
            rule__PATH__Group__3__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Group__3


    // $ANTLR start rule__PATH__Group__3__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1724:1: rule__PATH__Group__3__Impl : ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) ;
    public final void rule__PATH__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1728:1: ( ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1729:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1729:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1730:1: ( ( RULE_ID ) ) ( ( RULE_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1730:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1731:1: ( RULE_ID )
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1732:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1732:3: RULE_ID
            {
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl3592); 

            }

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1735:1: ( ( RULE_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1736:1: ( RULE_ID )*
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1737:1: ( RULE_ID )*
            loop21:
            do {
                int alt21=2;
                int LA21_0 = input.LA(1);

                if ( (LA21_0==RULE_ID) ) {
                    int LA21_2 = input.LA(2);

                    if ( (LA21_2==EOF||LA21_2==RULE_ID||LA21_2==25||LA21_2==27||LA21_2==29) ) {
                        alt21=1;
                    }


                }


                switch (alt21) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1737:3: RULE_ID
            	    {
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl3605); 

            	    }
            	    break;

            	default :
            	    break loop21;
                }
            } while (true);

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 

            }


            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Group__3__Impl


    // $ANTLR start rule__PATH__Group_0__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1756:1: rule__PATH__Group_0__0 : rule__PATH__Group_0__0__Impl rule__PATH__Group_0__1 ;
    public final void rule__PATH__Group_0__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1760:1: ( rule__PATH__Group_0__0__Impl rule__PATH__Group_0__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1761:2: rule__PATH__Group_0__0__Impl rule__PATH__Group_0__1
            {
            pushFollow(FOLLOW_rule__PATH__Group_0__0__Impl_in_rule__PATH__Group_0__03646);
            rule__PATH__Group_0__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group_0__1_in_rule__PATH__Group_0__03649);
            rule__PATH__Group_0__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Group_0__0


    // $ANTLR start rule__PATH__Group_0__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1768:1: rule__PATH__Group_0__0__Impl : ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) ;
    public final void rule__PATH__Group_0__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1772:1: ( ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1773:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1773:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1774:1: ( ( RULE_ID ) ) ( ( RULE_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1774:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1775:1: ( RULE_ID )
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1776:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1776:3: RULE_ID
            {
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl3679); 

            }

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1779:1: ( ( RULE_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1780:1: ( RULE_ID )*
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1781:1: ( RULE_ID )*
            loop22:
            do {
                int alt22=2;
                int LA22_0 = input.LA(1);

                if ( (LA22_0==RULE_ID) ) {
                    alt22=1;
                }


                switch (alt22) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1781:3: RULE_ID
            	    {
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl3692); 

            	    }
            	    break;

            	default :
            	    break loop22;
                }
            } while (true);

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 

            }


            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Group_0__0__Impl


    // $ANTLR start rule__PATH__Group_0__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1792:1: rule__PATH__Group_0__1 : rule__PATH__Group_0__1__Impl ;
    public final void rule__PATH__Group_0__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1796:1: ( rule__PATH__Group_0__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1797:2: rule__PATH__Group_0__1__Impl
            {
            pushFollow(FOLLOW_rule__PATH__Group_0__1__Impl_in_rule__PATH__Group_0__13725);
            rule__PATH__Group_0__1__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Group_0__1


    // $ANTLR start rule__PATH__Group_0__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1803:1: rule__PATH__Group_0__1__Impl : ( ( rule__PATH__Alternatives_0_1 ) ) ;
    public final void rule__PATH__Group_0__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1807:1: ( ( ( rule__PATH__Alternatives_0_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1808:1: ( ( rule__PATH__Alternatives_0_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1808:1: ( ( rule__PATH__Alternatives_0_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1809:1: ( rule__PATH__Alternatives_0_1 )
            {
             before(grammarAccess.getPATHAccess().getAlternatives_0_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1810:1: ( rule__PATH__Alternatives_0_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1810:2: rule__PATH__Alternatives_0_1
            {
            pushFollow(FOLLOW_rule__PATH__Alternatives_0_1_in_rule__PATH__Group_0__1__Impl3752);
            rule__PATH__Alternatives_0_1();
            _fsp--;


            }

             after(grammarAccess.getPATHAccess().getAlternatives_0_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PATH__Group_0__1__Impl


    // $ANTLR start rule__DIRECTION__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1824:1: rule__DIRECTION__Group__0 : rule__DIRECTION__Group__0__Impl rule__DIRECTION__Group__1 ;
    public final void rule__DIRECTION__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1828:1: ( rule__DIRECTION__Group__0__Impl rule__DIRECTION__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1829:2: rule__DIRECTION__Group__0__Impl rule__DIRECTION__Group__1
            {
            pushFollow(FOLLOW_rule__DIRECTION__Group__0__Impl_in_rule__DIRECTION__Group__03786);
            rule__DIRECTION__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__DIRECTION__Group__1_in_rule__DIRECTION__Group__03789);
            rule__DIRECTION__Group__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__DIRECTION__Group__0


    // $ANTLR start rule__DIRECTION__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1836:1: rule__DIRECTION__Group__0__Impl : ( ( rule__DIRECTION__Alternatives_0 ) ) ;
    public final void rule__DIRECTION__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1840:1: ( ( ( rule__DIRECTION__Alternatives_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1841:1: ( ( rule__DIRECTION__Alternatives_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1841:1: ( ( rule__DIRECTION__Alternatives_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1842:1: ( rule__DIRECTION__Alternatives_0 )
            {
             before(grammarAccess.getDIRECTIONAccess().getAlternatives_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1843:1: ( rule__DIRECTION__Alternatives_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1843:2: rule__DIRECTION__Alternatives_0
            {
            pushFollow(FOLLOW_rule__DIRECTION__Alternatives_0_in_rule__DIRECTION__Group__0__Impl3816);
            rule__DIRECTION__Alternatives_0();
            _fsp--;


            }

             after(grammarAccess.getDIRECTIONAccess().getAlternatives_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__DIRECTION__Group__0__Impl


    // $ANTLR start rule__DIRECTION__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1853:1: rule__DIRECTION__Group__1 : rule__DIRECTION__Group__1__Impl ;
    public final void rule__DIRECTION__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1857:1: ( rule__DIRECTION__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1858:2: rule__DIRECTION__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__DIRECTION__Group__1__Impl_in_rule__DIRECTION__Group__13846);
            rule__DIRECTION__Group__1__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__DIRECTION__Group__1


    // $ANTLR start rule__DIRECTION__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1864:1: rule__DIRECTION__Group__1__Impl : ( ( ',' )? ) ;
    public final void rule__DIRECTION__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1868:1: ( ( ( ',' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1869:1: ( ( ',' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1869:1: ( ( ',' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1870:1: ( ',' )?
            {
             before(grammarAccess.getDIRECTIONAccess().getCommaKeyword_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1871:1: ( ',' )?
            int alt23=2;
            int LA23_0 = input.LA(1);

            if ( (LA23_0==31) ) {
                alt23=1;
            }
            switch (alt23) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1872:2: ','
                    {
                    match(input,31,FOLLOW_31_in_rule__DIRECTION__Group__1__Impl3875); 

                    }
                    break;

            }

             after(grammarAccess.getDIRECTIONAccess().getCommaKeyword_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__DIRECTION__Group__1__Impl


    // $ANTLR start rule__LIST__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1887:1: rule__LIST__Group__0 : rule__LIST__Group__0__Impl rule__LIST__Group__1 ;
    public final void rule__LIST__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1891:1: ( rule__LIST__Group__0__Impl rule__LIST__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1892:2: rule__LIST__Group__0__Impl rule__LIST__Group__1
            {
            pushFollow(FOLLOW_rule__LIST__Group__0__Impl_in_rule__LIST__Group__03912);
            rule__LIST__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__LIST__Group__1_in_rule__LIST__Group__03915);
            rule__LIST__Group__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__LIST__Group__0


    // $ANTLR start rule__LIST__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1899:1: rule__LIST__Group__0__Impl : ( RULE_ID ) ;
    public final void rule__LIST__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1903:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1904:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1904:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1905:1: RULE_ID
            {
             before(grammarAccess.getLISTAccess().getIDTerminalRuleCall_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__LIST__Group__0__Impl3942); 
             after(grammarAccess.getLISTAccess().getIDTerminalRuleCall_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__LIST__Group__0__Impl


    // $ANTLR start rule__LIST__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1916:1: rule__LIST__Group__1 : rule__LIST__Group__1__Impl ;
    public final void rule__LIST__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1920:1: ( rule__LIST__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1921:2: rule__LIST__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__LIST__Group__1__Impl_in_rule__LIST__Group__13971);
            rule__LIST__Group__1__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__LIST__Group__1


    // $ANTLR start rule__LIST__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1927:1: rule__LIST__Group__1__Impl : ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) ) ;
    public final void rule__LIST__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1931:1: ( ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1932:1: ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1932:1: ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1933:1: ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1933:1: ( ( rule__LIST__Group_1__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1934:1: ( rule__LIST__Group_1__0 )
            {
             before(grammarAccess.getLISTAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1935:1: ( rule__LIST__Group_1__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1935:2: rule__LIST__Group_1__0
            {
            pushFollow(FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl4000);
            rule__LIST__Group_1__0();
            _fsp--;


            }

             after(grammarAccess.getLISTAccess().getGroup_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1938:1: ( ( rule__LIST__Group_1__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1939:1: ( rule__LIST__Group_1__0 )*
            {
             before(grammarAccess.getLISTAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1940:1: ( rule__LIST__Group_1__0 )*
            loop24:
            do {
                int alt24=2;
                int LA24_0 = input.LA(1);

                if ( (LA24_0==31) ) {
                    alt24=1;
                }


                switch (alt24) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1940:2: rule__LIST__Group_1__0
            	    {
            	    pushFollow(FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl4012);
            	    rule__LIST__Group_1__0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop24;
                }
            } while (true);

             after(grammarAccess.getLISTAccess().getGroup_1()); 

            }


            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__LIST__Group__1__Impl


    // $ANTLR start rule__LIST__Group_1__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1955:1: rule__LIST__Group_1__0 : rule__LIST__Group_1__0__Impl rule__LIST__Group_1__1 ;
    public final void rule__LIST__Group_1__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1959:1: ( rule__LIST__Group_1__0__Impl rule__LIST__Group_1__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1960:2: rule__LIST__Group_1__0__Impl rule__LIST__Group_1__1
            {
            pushFollow(FOLLOW_rule__LIST__Group_1__0__Impl_in_rule__LIST__Group_1__04049);
            rule__LIST__Group_1__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__LIST__Group_1__1_in_rule__LIST__Group_1__04052);
            rule__LIST__Group_1__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__LIST__Group_1__0


    // $ANTLR start rule__LIST__Group_1__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1967:1: rule__LIST__Group_1__0__Impl : ( ',' ) ;
    public final void rule__LIST__Group_1__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1971:1: ( ( ',' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1972:1: ( ',' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1972:1: ( ',' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1973:1: ','
            {
             before(grammarAccess.getLISTAccess().getCommaKeyword_1_0()); 
            match(input,31,FOLLOW_31_in_rule__LIST__Group_1__0__Impl4080); 
             after(grammarAccess.getLISTAccess().getCommaKeyword_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__LIST__Group_1__0__Impl


    // $ANTLR start rule__LIST__Group_1__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1986:1: rule__LIST__Group_1__1 : rule__LIST__Group_1__1__Impl ;
    public final void rule__LIST__Group_1__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1990:1: ( rule__LIST__Group_1__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1991:2: rule__LIST__Group_1__1__Impl
            {
            pushFollow(FOLLOW_rule__LIST__Group_1__1__Impl_in_rule__LIST__Group_1__14111);
            rule__LIST__Group_1__1__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__LIST__Group_1__1


    // $ANTLR start rule__LIST__Group_1__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1997:1: rule__LIST__Group_1__1__Impl : ( RULE_ID ) ;
    public final void rule__LIST__Group_1__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2001:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2002:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2002:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2003:1: RULE_ID
            {
             before(grammarAccess.getLISTAccess().getIDTerminalRuleCall_1_1()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__LIST__Group_1__1__Impl4138); 
             after(grammarAccess.getLISTAccess().getIDTerminalRuleCall_1_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__LIST__Group_1__1__Impl


    // $ANTLR start rule__PROGRESSIVE__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2018:1: rule__PROGRESSIVE__Group__0 : rule__PROGRESSIVE__Group__0__Impl rule__PROGRESSIVE__Group__1 ;
    public final void rule__PROGRESSIVE__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2022:1: ( rule__PROGRESSIVE__Group__0__Impl rule__PROGRESSIVE__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2023:2: rule__PROGRESSIVE__Group__0__Impl rule__PROGRESSIVE__Group__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__0__Impl_in_rule__PROGRESSIVE__Group__04171);
            rule__PROGRESSIVE__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__1_in_rule__PROGRESSIVE__Group__04174);
            rule__PROGRESSIVE__Group__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group__0


    // $ANTLR start rule__PROGRESSIVE__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2030:1: rule__PROGRESSIVE__Group__0__Impl : ( ( rule__PROGRESSIVE__Alternatives_0 ) ) ;
    public final void rule__PROGRESSIVE__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2034:1: ( ( ( rule__PROGRESSIVE__Alternatives_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2035:1: ( ( rule__PROGRESSIVE__Alternatives_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2035:1: ( ( rule__PROGRESSIVE__Alternatives_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2036:1: ( rule__PROGRESSIVE__Alternatives_0 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2037:1: ( rule__PROGRESSIVE__Alternatives_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2037:2: rule__PROGRESSIVE__Alternatives_0
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_0_in_rule__PROGRESSIVE__Group__0__Impl4201);
            rule__PROGRESSIVE__Alternatives_0();
            _fsp--;


            }

             after(grammarAccess.getPROGRESSIVEAccess().getAlternatives_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group__0__Impl


    // $ANTLR start rule__PROGRESSIVE__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2047:1: rule__PROGRESSIVE__Group__1 : rule__PROGRESSIVE__Group__1__Impl rule__PROGRESSIVE__Group__2 ;
    public final void rule__PROGRESSIVE__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2051:1: ( rule__PROGRESSIVE__Group__1__Impl rule__PROGRESSIVE__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2052:2: rule__PROGRESSIVE__Group__1__Impl rule__PROGRESSIVE__Group__2
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__1__Impl_in_rule__PROGRESSIVE__Group__14231);
            rule__PROGRESSIVE__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__2_in_rule__PROGRESSIVE__Group__14234);
            rule__PROGRESSIVE__Group__2();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group__1


    // $ANTLR start rule__PROGRESSIVE__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2059:1: rule__PROGRESSIVE__Group__1__Impl : ( ( rule__PROGRESSIVE__Group_1__0 )? ) ;
    public final void rule__PROGRESSIVE__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2063:1: ( ( ( rule__PROGRESSIVE__Group_1__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2064:1: ( ( rule__PROGRESSIVE__Group_1__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2064:1: ( ( rule__PROGRESSIVE__Group_1__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2065:1: ( rule__PROGRESSIVE__Group_1__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2066:1: ( rule__PROGRESSIVE__Group_1__0 )?
            int alt25=2;
            int LA25_0 = input.LA(1);

            if ( (LA25_0==32) ) {
                alt25=1;
            }
            switch (alt25) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2066:2: rule__PROGRESSIVE__Group_1__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__0_in_rule__PROGRESSIVE__Group__1__Impl4261);
                    rule__PROGRESSIVE__Group_1__0();
                    _fsp--;


                    }
                    break;

            }

             after(grammarAccess.getPROGRESSIVEAccess().getGroup_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group__1__Impl


    // $ANTLR start rule__PROGRESSIVE__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2076:1: rule__PROGRESSIVE__Group__2 : rule__PROGRESSIVE__Group__2__Impl rule__PROGRESSIVE__Group__3 ;
    public final void rule__PROGRESSIVE__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2080:1: ( rule__PROGRESSIVE__Group__2__Impl rule__PROGRESSIVE__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2081:2: rule__PROGRESSIVE__Group__2__Impl rule__PROGRESSIVE__Group__3
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__2__Impl_in_rule__PROGRESSIVE__Group__24292);
            rule__PROGRESSIVE__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__3_in_rule__PROGRESSIVE__Group__24295);
            rule__PROGRESSIVE__Group__3();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group__2


    // $ANTLR start rule__PROGRESSIVE__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2088:1: rule__PROGRESSIVE__Group__2__Impl : ( ( rule__PROGRESSIVE__Group_2__0 )? ) ;
    public final void rule__PROGRESSIVE__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2092:1: ( ( ( rule__PROGRESSIVE__Group_2__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2093:1: ( ( rule__PROGRESSIVE__Group_2__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2093:1: ( ( rule__PROGRESSIVE__Group_2__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2094:1: ( rule__PROGRESSIVE__Group_2__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2095:1: ( rule__PROGRESSIVE__Group_2__0 )?
            int alt26=2;
            int LA26_0 = input.LA(1);

            if ( (LA26_0==10) ) {
                alt26=1;
            }
            switch (alt26) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2095:2: rule__PROGRESSIVE__Group_2__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__0_in_rule__PROGRESSIVE__Group__2__Impl4322);
                    rule__PROGRESSIVE__Group_2__0();
                    _fsp--;


                    }
                    break;

            }

             after(grammarAccess.getPROGRESSIVEAccess().getGroup_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group__2__Impl


    // $ANTLR start rule__PROGRESSIVE__Group__3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2105:1: rule__PROGRESSIVE__Group__3 : rule__PROGRESSIVE__Group__3__Impl ;
    public final void rule__PROGRESSIVE__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2109:1: ( rule__PROGRESSIVE__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2110:2: rule__PROGRESSIVE__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__3__Impl_in_rule__PROGRESSIVE__Group__34353);
            rule__PROGRESSIVE__Group__3__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group__3


    // $ANTLR start rule__PROGRESSIVE__Group__3__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2116:1: rule__PROGRESSIVE__Group__3__Impl : ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) ) ;
    public final void rule__PROGRESSIVE__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2120:1: ( ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2121:1: ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2121:1: ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2122:1: ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2122:1: ( ( rule__PROGRESSIVE__Group_3__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2123:1: ( rule__PROGRESSIVE__Group_3__0 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2124:1: ( rule__PROGRESSIVE__Group_3__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2124:2: rule__PROGRESSIVE__Group_3__0
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl4382);
            rule__PROGRESSIVE__Group_3__0();
            _fsp--;


            }

             after(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2127:1: ( ( rule__PROGRESSIVE__Group_3__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2128:1: ( rule__PROGRESSIVE__Group_3__0 )*
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2129:1: ( rule__PROGRESSIVE__Group_3__0 )*
            loop27:
            do {
                int alt27=2;
                int LA27_0 = input.LA(1);

                if ( (LA27_0==31) ) {
                    alt27=1;
                }


                switch (alt27) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2129:2: rule__PROGRESSIVE__Group_3__0
            	    {
            	    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl4394);
            	    rule__PROGRESSIVE__Group_3__0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop27;
                }
            } while (true);

             after(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 

            }


            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group__3__Impl


    // $ANTLR start rule__PROGRESSIVE__Group_1__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2148:1: rule__PROGRESSIVE__Group_1__0 : rule__PROGRESSIVE__Group_1__0__Impl rule__PROGRESSIVE__Group_1__1 ;
    public final void rule__PROGRESSIVE__Group_1__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2152:1: ( rule__PROGRESSIVE__Group_1__0__Impl rule__PROGRESSIVE__Group_1__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2153:2: rule__PROGRESSIVE__Group_1__0__Impl rule__PROGRESSIVE__Group_1__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__0__Impl_in_rule__PROGRESSIVE__Group_1__04435);
            rule__PROGRESSIVE__Group_1__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__1_in_rule__PROGRESSIVE__Group_1__04438);
            rule__PROGRESSIVE__Group_1__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_1__0


    // $ANTLR start rule__PROGRESSIVE__Group_1__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2160:1: rule__PROGRESSIVE__Group_1__0__Impl : ( '~' ) ;
    public final void rule__PROGRESSIVE__Group_1__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2164:1: ( ( '~' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2165:1: ( '~' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2165:1: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2166:1: '~'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_1_0()); 
            match(input,32,FOLLOW_32_in_rule__PROGRESSIVE__Group_1__0__Impl4466); 
             after(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_1__0__Impl


    // $ANTLR start rule__PROGRESSIVE__Group_1__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2179:1: rule__PROGRESSIVE__Group_1__1 : rule__PROGRESSIVE__Group_1__1__Impl ;
    public final void rule__PROGRESSIVE__Group_1__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2183:1: ( rule__PROGRESSIVE__Group_1__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2184:2: rule__PROGRESSIVE__Group_1__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__1__Impl_in_rule__PROGRESSIVE__Group_1__14497);
            rule__PROGRESSIVE__Group_1__1__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_1__1


    // $ANTLR start rule__PROGRESSIVE__Group_1__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2190:1: rule__PROGRESSIVE__Group_1__1__Impl : ( ( rule__PROGRESSIVE__Alternatives_1_1 ) ) ;
    public final void rule__PROGRESSIVE__Group_1__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2194:1: ( ( ( rule__PROGRESSIVE__Alternatives_1_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2195:1: ( ( rule__PROGRESSIVE__Alternatives_1_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2195:1: ( ( rule__PROGRESSIVE__Alternatives_1_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2196:1: ( rule__PROGRESSIVE__Alternatives_1_1 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_1_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2197:1: ( rule__PROGRESSIVE__Alternatives_1_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2197:2: rule__PROGRESSIVE__Alternatives_1_1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_1_1_in_rule__PROGRESSIVE__Group_1__1__Impl4524);
            rule__PROGRESSIVE__Alternatives_1_1();
            _fsp--;


            }

             after(grammarAccess.getPROGRESSIVEAccess().getAlternatives_1_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_1__1__Impl


    // $ANTLR start rule__PROGRESSIVE__Group_2__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2211:1: rule__PROGRESSIVE__Group_2__0 : rule__PROGRESSIVE__Group_2__0__Impl rule__PROGRESSIVE__Group_2__1 ;
    public final void rule__PROGRESSIVE__Group_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2215:1: ( rule__PROGRESSIVE__Group_2__0__Impl rule__PROGRESSIVE__Group_2__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2216:2: rule__PROGRESSIVE__Group_2__0__Impl rule__PROGRESSIVE__Group_2__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__0__Impl_in_rule__PROGRESSIVE__Group_2__04558);
            rule__PROGRESSIVE__Group_2__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__1_in_rule__PROGRESSIVE__Group_2__04561);
            rule__PROGRESSIVE__Group_2__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_2__0


    // $ANTLR start rule__PROGRESSIVE__Group_2__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2223:1: rule__PROGRESSIVE__Group_2__0__Impl : ( ':' ) ;
    public final void rule__PROGRESSIVE__Group_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2227:1: ( ( ':' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2228:1: ( ':' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2228:1: ( ':' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2229:1: ':'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_2_0()); 
            match(input,10,FOLLOW_10_in_rule__PROGRESSIVE__Group_2__0__Impl4589); 
             after(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_2__0__Impl


    // $ANTLR start rule__PROGRESSIVE__Group_2__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2242:1: rule__PROGRESSIVE__Group_2__1 : rule__PROGRESSIVE__Group_2__1__Impl ;
    public final void rule__PROGRESSIVE__Group_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2246:1: ( rule__PROGRESSIVE__Group_2__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2247:2: rule__PROGRESSIVE__Group_2__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__1__Impl_in_rule__PROGRESSIVE__Group_2__14620);
            rule__PROGRESSIVE__Group_2__1__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_2__1


    // $ANTLR start rule__PROGRESSIVE__Group_2__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2253:1: rule__PROGRESSIVE__Group_2__1__Impl : ( RULE_IINT ) ;
    public final void rule__PROGRESSIVE__Group_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2257:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2258:1: ( RULE_IINT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2258:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2259:1: RULE_IINT
            {
             before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_2_1()); 
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_2__1__Impl4647); 
             after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_2_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_2__1__Impl


    // $ANTLR start rule__PROGRESSIVE__Group_3__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2274:1: rule__PROGRESSIVE__Group_3__0 : rule__PROGRESSIVE__Group_3__0__Impl rule__PROGRESSIVE__Group_3__1 ;
    public final void rule__PROGRESSIVE__Group_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2278:1: ( rule__PROGRESSIVE__Group_3__0__Impl rule__PROGRESSIVE__Group_3__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2279:2: rule__PROGRESSIVE__Group_3__0__Impl rule__PROGRESSIVE__Group_3__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__0__Impl_in_rule__PROGRESSIVE__Group_3__04680);
            rule__PROGRESSIVE__Group_3__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__1_in_rule__PROGRESSIVE__Group_3__04683);
            rule__PROGRESSIVE__Group_3__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3__0


    // $ANTLR start rule__PROGRESSIVE__Group_3__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2286:1: rule__PROGRESSIVE__Group_3__0__Impl : ( ',' ) ;
    public final void rule__PROGRESSIVE__Group_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2290:1: ( ( ',' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2291:1: ( ',' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2291:1: ( ',' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2292:1: ','
            {
             before(grammarAccess.getPROGRESSIVEAccess().getCommaKeyword_3_0()); 
            match(input,31,FOLLOW_31_in_rule__PROGRESSIVE__Group_3__0__Impl4711); 
             after(grammarAccess.getPROGRESSIVEAccess().getCommaKeyword_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3__0__Impl


    // $ANTLR start rule__PROGRESSIVE__Group_3__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2305:1: rule__PROGRESSIVE__Group_3__1 : rule__PROGRESSIVE__Group_3__1__Impl rule__PROGRESSIVE__Group_3__2 ;
    public final void rule__PROGRESSIVE__Group_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2309:1: ( rule__PROGRESSIVE__Group_3__1__Impl rule__PROGRESSIVE__Group_3__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2310:2: rule__PROGRESSIVE__Group_3__1__Impl rule__PROGRESSIVE__Group_3__2
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__1__Impl_in_rule__PROGRESSIVE__Group_3__14742);
            rule__PROGRESSIVE__Group_3__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__2_in_rule__PROGRESSIVE__Group_3__14745);
            rule__PROGRESSIVE__Group_3__2();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3__1


    // $ANTLR start rule__PROGRESSIVE__Group_3__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2317:1: rule__PROGRESSIVE__Group_3__1__Impl : ( ( rule__PROGRESSIVE__Alternatives_3_1 ) ) ;
    public final void rule__PROGRESSIVE__Group_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2321:1: ( ( ( rule__PROGRESSIVE__Alternatives_3_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2322:1: ( ( rule__PROGRESSIVE__Alternatives_3_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2322:1: ( ( rule__PROGRESSIVE__Alternatives_3_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2323:1: ( rule__PROGRESSIVE__Alternatives_3_1 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_3_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2324:1: ( rule__PROGRESSIVE__Alternatives_3_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2324:2: rule__PROGRESSIVE__Alternatives_3_1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_3_1_in_rule__PROGRESSIVE__Group_3__1__Impl4772);
            rule__PROGRESSIVE__Alternatives_3_1();
            _fsp--;


            }

             after(grammarAccess.getPROGRESSIVEAccess().getAlternatives_3_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3__1__Impl


    // $ANTLR start rule__PROGRESSIVE__Group_3__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2334:1: rule__PROGRESSIVE__Group_3__2 : rule__PROGRESSIVE__Group_3__2__Impl rule__PROGRESSIVE__Group_3__3 ;
    public final void rule__PROGRESSIVE__Group_3__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2338:1: ( rule__PROGRESSIVE__Group_3__2__Impl rule__PROGRESSIVE__Group_3__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2339:2: rule__PROGRESSIVE__Group_3__2__Impl rule__PROGRESSIVE__Group_3__3
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__2__Impl_in_rule__PROGRESSIVE__Group_3__24802);
            rule__PROGRESSIVE__Group_3__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__3_in_rule__PROGRESSIVE__Group_3__24805);
            rule__PROGRESSIVE__Group_3__3();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3__2


    // $ANTLR start rule__PROGRESSIVE__Group_3__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2346:1: rule__PROGRESSIVE__Group_3__2__Impl : ( ( rule__PROGRESSIVE__Group_3_2__0 )? ) ;
    public final void rule__PROGRESSIVE__Group_3__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2350:1: ( ( ( rule__PROGRESSIVE__Group_3_2__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2351:1: ( ( rule__PROGRESSIVE__Group_3_2__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2351:1: ( ( rule__PROGRESSIVE__Group_3_2__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2352:1: ( rule__PROGRESSIVE__Group_3_2__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2353:1: ( rule__PROGRESSIVE__Group_3_2__0 )?
            int alt28=2;
            int LA28_0 = input.LA(1);

            if ( (LA28_0==32) ) {
                alt28=1;
            }
            switch (alt28) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2353:2: rule__PROGRESSIVE__Group_3_2__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__0_in_rule__PROGRESSIVE__Group_3__2__Impl4832);
                    rule__PROGRESSIVE__Group_3_2__0();
                    _fsp--;


                    }
                    break;

            }

             after(grammarAccess.getPROGRESSIVEAccess().getGroup_3_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3__2__Impl


    // $ANTLR start rule__PROGRESSIVE__Group_3__3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2363:1: rule__PROGRESSIVE__Group_3__3 : rule__PROGRESSIVE__Group_3__3__Impl ;
    public final void rule__PROGRESSIVE__Group_3__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2367:1: ( rule__PROGRESSIVE__Group_3__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2368:2: rule__PROGRESSIVE__Group_3__3__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__3__Impl_in_rule__PROGRESSIVE__Group_3__34863);
            rule__PROGRESSIVE__Group_3__3__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3__3


    // $ANTLR start rule__PROGRESSIVE__Group_3__3__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2374:1: rule__PROGRESSIVE__Group_3__3__Impl : ( ( rule__PROGRESSIVE__Group_3_3__0 )? ) ;
    public final void rule__PROGRESSIVE__Group_3__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2378:1: ( ( ( rule__PROGRESSIVE__Group_3_3__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2379:1: ( ( rule__PROGRESSIVE__Group_3_3__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2379:1: ( ( rule__PROGRESSIVE__Group_3_3__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2380:1: ( rule__PROGRESSIVE__Group_3_3__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2381:1: ( rule__PROGRESSIVE__Group_3_3__0 )?
            int alt29=2;
            int LA29_0 = input.LA(1);

            if ( (LA29_0==10) ) {
                alt29=1;
            }
            switch (alt29) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2381:2: rule__PROGRESSIVE__Group_3_3__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__0_in_rule__PROGRESSIVE__Group_3__3__Impl4890);
                    rule__PROGRESSIVE__Group_3_3__0();
                    _fsp--;


                    }
                    break;

            }

             after(grammarAccess.getPROGRESSIVEAccess().getGroup_3_3()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3__3__Impl


    // $ANTLR start rule__PROGRESSIVE__Group_3_2__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2399:1: rule__PROGRESSIVE__Group_3_2__0 : rule__PROGRESSIVE__Group_3_2__0__Impl rule__PROGRESSIVE__Group_3_2__1 ;
    public final void rule__PROGRESSIVE__Group_3_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2403:1: ( rule__PROGRESSIVE__Group_3_2__0__Impl rule__PROGRESSIVE__Group_3_2__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2404:2: rule__PROGRESSIVE__Group_3_2__0__Impl rule__PROGRESSIVE__Group_3_2__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__0__Impl_in_rule__PROGRESSIVE__Group_3_2__04929);
            rule__PROGRESSIVE__Group_3_2__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__1_in_rule__PROGRESSIVE__Group_3_2__04932);
            rule__PROGRESSIVE__Group_3_2__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3_2__0


    // $ANTLR start rule__PROGRESSIVE__Group_3_2__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2411:1: rule__PROGRESSIVE__Group_3_2__0__Impl : ( '~' ) ;
    public final void rule__PROGRESSIVE__Group_3_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2415:1: ( ( '~' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2416:1: ( '~' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2416:1: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2417:1: '~'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_3_2_0()); 
            match(input,32,FOLLOW_32_in_rule__PROGRESSIVE__Group_3_2__0__Impl4960); 
             after(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_3_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3_2__0__Impl


    // $ANTLR start rule__PROGRESSIVE__Group_3_2__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2430:1: rule__PROGRESSIVE__Group_3_2__1 : rule__PROGRESSIVE__Group_3_2__1__Impl ;
    public final void rule__PROGRESSIVE__Group_3_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2434:1: ( rule__PROGRESSIVE__Group_3_2__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2435:2: rule__PROGRESSIVE__Group_3_2__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__1__Impl_in_rule__PROGRESSIVE__Group_3_2__14991);
            rule__PROGRESSIVE__Group_3_2__1__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3_2__1


    // $ANTLR start rule__PROGRESSIVE__Group_3_2__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2441:1: rule__PROGRESSIVE__Group_3_2__1__Impl : ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) ) ;
    public final void rule__PROGRESSIVE__Group_3_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2445:1: ( ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2446:1: ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2446:1: ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2447:1: ( rule__PROGRESSIVE__Alternatives_3_2_1 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_3_2_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2448:1: ( rule__PROGRESSIVE__Alternatives_3_2_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2448:2: rule__PROGRESSIVE__Alternatives_3_2_1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_3_2_1_in_rule__PROGRESSIVE__Group_3_2__1__Impl5018);
            rule__PROGRESSIVE__Alternatives_3_2_1();
            _fsp--;


            }

             after(grammarAccess.getPROGRESSIVEAccess().getAlternatives_3_2_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3_2__1__Impl


    // $ANTLR start rule__PROGRESSIVE__Group_3_3__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2462:1: rule__PROGRESSIVE__Group_3_3__0 : rule__PROGRESSIVE__Group_3_3__0__Impl rule__PROGRESSIVE__Group_3_3__1 ;
    public final void rule__PROGRESSIVE__Group_3_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2466:1: ( rule__PROGRESSIVE__Group_3_3__0__Impl rule__PROGRESSIVE__Group_3_3__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2467:2: rule__PROGRESSIVE__Group_3_3__0__Impl rule__PROGRESSIVE__Group_3_3__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__0__Impl_in_rule__PROGRESSIVE__Group_3_3__05052);
            rule__PROGRESSIVE__Group_3_3__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__1_in_rule__PROGRESSIVE__Group_3_3__05055);
            rule__PROGRESSIVE__Group_3_3__1();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3_3__0


    // $ANTLR start rule__PROGRESSIVE__Group_3_3__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2474:1: rule__PROGRESSIVE__Group_3_3__0__Impl : ( ':' ) ;
    public final void rule__PROGRESSIVE__Group_3_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2478:1: ( ( ':' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2479:1: ( ':' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2479:1: ( ':' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2480:1: ':'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_3_3_0()); 
            match(input,10,FOLLOW_10_in_rule__PROGRESSIVE__Group_3_3__0__Impl5083); 
             after(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_3_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3_3__0__Impl


    // $ANTLR start rule__PROGRESSIVE__Group_3_3__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2493:1: rule__PROGRESSIVE__Group_3_3__1 : rule__PROGRESSIVE__Group_3_3__1__Impl ;
    public final void rule__PROGRESSIVE__Group_3_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2497:1: ( rule__PROGRESSIVE__Group_3_3__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2498:2: rule__PROGRESSIVE__Group_3_3__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__1__Impl_in_rule__PROGRESSIVE__Group_3_3__15114);
            rule__PROGRESSIVE__Group_3_3__1__Impl();
            _fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3_3__1


    // $ANTLR start rule__PROGRESSIVE__Group_3_3__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2504:1: rule__PROGRESSIVE__Group_3_3__1__Impl : ( RULE_IINT ) ;
    public final void rule__PROGRESSIVE__Group_3_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2508:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2509:1: ( RULE_IINT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2509:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2510:1: RULE_IINT
            {
             before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_3_1()); 
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_3_3__1__Impl5141); 
             after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_3_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PROGRESSIVE__Group_3_3__1__Impl


    // $ANTLR start rule__WMLRoot__RtagsAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2526:1: rule__WMLRoot__RtagsAssignment_0 : ( ruleWMLTag ) ;
    public final void rule__WMLRoot__RtagsAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2530:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2531:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2531:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2532:1: ruleWMLTag
            {
             before(grammarAccess.getWMLRootAccess().getRtagsWMLTagParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRoot__RtagsAssignment_05179);
            ruleWMLTag();
            _fsp--;

             after(grammarAccess.getWMLRootAccess().getRtagsWMLTagParserRuleCall_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLRoot__RtagsAssignment_0


    // $ANTLR start rule__WMLRoot__RmacrosAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2541:1: rule__WMLRoot__RmacrosAssignment_1 : ( ruleWMLMacro ) ;
    public final void rule__WMLRoot__RmacrosAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2545:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2546:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2546:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2547:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLRootAccess().getRmacrosWMLMacroParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLRoot__RmacrosAssignment_15210);
            ruleWMLMacro();
            _fsp--;

             after(grammarAccess.getWMLRootAccess().getRmacrosWMLMacroParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLRoot__RmacrosAssignment_1


    // $ANTLR start rule__WMLMacro__MacroNameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2556:1: rule__WMLMacro__MacroNameAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLMacro__MacroNameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2560:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2561:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2561:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2562:1: RULE_ID
            {
             before(grammarAccess.getWMLMacroAccess().getMacroNameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacro__MacroNameAssignment_15241); 
             after(grammarAccess.getWMLMacroAccess().getMacroNameIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacro__MacroNameAssignment_1


    // $ANTLR start rule__WMLMacro__TagcontentAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2571:1: rule__WMLMacro__TagcontentAssignment_2 : ( ( rule__WMLMacro__TagcontentAlternatives_2_0 ) ) ;
    public final void rule__WMLMacro__TagcontentAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2575:1: ( ( ( rule__WMLMacro__TagcontentAlternatives_2_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2576:1: ( ( rule__WMLMacro__TagcontentAlternatives_2_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2576:1: ( ( rule__WMLMacro__TagcontentAlternatives_2_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2577:1: ( rule__WMLMacro__TagcontentAlternatives_2_0 )
            {
             before(grammarAccess.getWMLMacroAccess().getTagcontentAlternatives_2_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2578:1: ( rule__WMLMacro__TagcontentAlternatives_2_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2578:2: rule__WMLMacro__TagcontentAlternatives_2_0
            {
            pushFollow(FOLLOW_rule__WMLMacro__TagcontentAlternatives_2_0_in_rule__WMLMacro__TagcontentAssignment_25272);
            rule__WMLMacro__TagcontentAlternatives_2_0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroAccess().getTagcontentAlternatives_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacro__TagcontentAssignment_2


    // $ANTLR start rule__WMLTag__StartAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2587:1: rule__WMLTag__StartAssignment_0 : ( ruleWMLStartTag ) ;
    public final void rule__WMLTag__StartAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2591:1: ( ( ruleWMLStartTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2592:1: ( ruleWMLStartTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2592:1: ( ruleWMLStartTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2593:1: ruleWMLStartTag
            {
             before(grammarAccess.getWMLTagAccess().getStartWMLStartTagParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleWMLStartTag_in_rule__WMLTag__StartAssignment_05305);
            ruleWMLStartTag();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getStartWMLStartTagParserRuleCall_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__StartAssignment_0


    // $ANTLR start rule__WMLTag__TtagsAssignment_1_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2602:1: rule__WMLTag__TtagsAssignment_1_0 : ( ruleWMLTag ) ;
    public final void rule__WMLTag__TtagsAssignment_1_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2606:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2607:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2607:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2608:1: ruleWMLTag
            {
             before(grammarAccess.getWMLTagAccess().getTtagsWMLTagParserRuleCall_1_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLTag__TtagsAssignment_1_05336);
            ruleWMLTag();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getTtagsWMLTagParserRuleCall_1_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__TtagsAssignment_1_0


    // $ANTLR start rule__WMLTag__TkeysAssignment_1_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2617:1: rule__WMLTag__TkeysAssignment_1_1 : ( ruleWMLKey ) ;
    public final void rule__WMLTag__TkeysAssignment_1_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2621:1: ( ( ruleWMLKey ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2622:1: ( ruleWMLKey )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2622:1: ( ruleWMLKey )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2623:1: ruleWMLKey
            {
             before(grammarAccess.getWMLTagAccess().getTkeysWMLKeyParserRuleCall_1_1_0()); 
            pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLTag__TkeysAssignment_1_15367);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getTkeysWMLKeyParserRuleCall_1_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__TkeysAssignment_1_1


    // $ANTLR start rule__WMLTag__TmacrosAssignment_1_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2632:1: rule__WMLTag__TmacrosAssignment_1_2 : ( ruleWMLMacro ) ;
    public final void rule__WMLTag__TmacrosAssignment_1_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2636:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2637:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2637:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2638:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLTagAccess().getTmacrosWMLMacroParserRuleCall_1_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLTag__TmacrosAssignment_1_25398);
            ruleWMLMacro();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getTmacrosWMLMacroParserRuleCall_1_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__TmacrosAssignment_1_2


    // $ANTLR start rule__WMLTag__EndAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2647:1: rule__WMLTag__EndAssignment_2 : ( ruleWMLEndTag ) ;
    public final void rule__WMLTag__EndAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2651:1: ( ( ruleWMLEndTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2652:1: ( ruleWMLEndTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2652:1: ( ruleWMLEndTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2653:1: ruleWMLEndTag
            {
             before(grammarAccess.getWMLTagAccess().getEndWMLEndTagParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLEndTag_in_rule__WMLTag__EndAssignment_25429);
            ruleWMLEndTag();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getEndWMLEndTagParserRuleCall_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__EndAssignment_2


    // $ANTLR start rule__WMLStartTag__TagnameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2662:1: rule__WMLStartTag__TagnameAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLStartTag__TagnameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2666:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2667:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2667:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2668:1: RULE_ID
            {
             before(grammarAccess.getWMLStartTagAccess().getTagnameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLStartTag__TagnameAssignment_15460); 
             after(grammarAccess.getWMLStartTagAccess().getTagnameIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLStartTag__TagnameAssignment_1


    // $ANTLR start rule__WMLEndTag__TagnameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2677:1: rule__WMLEndTag__TagnameAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLEndTag__TagnameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2681:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2682:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2682:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2683:1: RULE_ID
            {
             before(grammarAccess.getWMLEndTagAccess().getTagnameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLEndTag__TagnameAssignment_15491); 
             after(grammarAccess.getWMLEndTagAccess().getTagnameIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLEndTag__TagnameAssignment_1


    // $ANTLR start rule__WMLKey__KeyNameAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2692:1: rule__WMLKey__KeyNameAssignment_0 : ( RULE_ID ) ;
    public final void rule__WMLKey__KeyNameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2696:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2697:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2697:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2698:1: RULE_ID
            {
             before(grammarAccess.getWMLKeyAccess().getKeyNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__KeyNameAssignment_05522); 
             after(grammarAccess.getWMLKeyAccess().getKeyNameIDTerminalRuleCall_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__KeyNameAssignment_0


    // $ANTLR start rule__WMLKey__ValueAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2707:1: rule__WMLKey__ValueAssignment_2 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2711:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2712:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2712:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2713:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_25553);
            ruleWMLKeyValue();
            _fsp--;

             after(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__ValueAssignment_2


    // $ANTLR start rule__WMLKeyValue__Key1ValueAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2722:1: rule__WMLKeyValue__Key1ValueAssignment_0 : ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) ) ;
    public final void rule__WMLKeyValue__Key1ValueAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2726:1: ( ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2727:1: ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2727:1: ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2728:1: ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 )
            {
             before(grammarAccess.getWMLKeyValueAccess().getKey1ValueAlternatives_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2729:1: ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2729:2: rule__WMLKeyValue__Key1ValueAlternatives_0_0
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Key1ValueAlternatives_0_0_in_rule__WMLKeyValue__Key1ValueAssignment_05584);
            rule__WMLKeyValue__Key1ValueAlternatives_0_0();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyValueAccess().getKey1ValueAlternatives_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKeyValue__Key1ValueAssignment_0


    // $ANTLR start rule__WMLKeyValue__Key2ValueAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2738:1: rule__WMLKeyValue__Key2ValueAssignment_1 : ( ruleWMLMacro ) ;
    public final void rule__WMLKeyValue__Key2ValueAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2742:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2743:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2743:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2744:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLKeyValueAccess().getKey2ValueWMLMacroParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLKeyValue__Key2ValueAssignment_15617);
            ruleWMLMacro();
            _fsp--;

             after(grammarAccess.getWMLKeyValueAccess().getKey2ValueWMLMacroParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKeyValue__Key2ValueAssignment_1


    protected DFA7 dfa7 = new DFA7(this);
    protected DFA19 dfa19 = new DFA19(this);
    static final String DFA7_eotS =
        "\20\uffff";
    static final String DFA7_eofS =
        "\1\uffff\1\10\2\uffff\1\14\10\uffff\2\17\1\uffff";
    static final String DFA7_minS =
        "\2\4\2\uffff\1\4\1\uffff\1\4\3\uffff\1\6\2\uffff\2\4\1\uffff";
    static final String DFA7_maxS =
        "\1\36\1\37\2\uffff\1\40\1\uffff\1\20\3\uffff\1\6\2\uffff\2\40\1"+
        "\uffff";
    static final String DFA7_acceptS =
        "\2\uffff\1\2\1\3\1\uffff\1\7\1\uffff\1\6\1\1\1\10\1\uffff\1\11\1"+
        "\5\2\uffff\1\4";
    static final String DFA7_specialS =
        "\20\uffff}>";
    static final String[] DFA7_transitionS = {
            "\1\1\1\2\1\4\2\uffff\1\3\7\uffff\10\5\5\uffff\1\3",
            "\1\6\6\uffff\2\7\3\uffff\1\7\10\uffff\1\10\1\uffff\1\10\1\uffff"+
            "\1\10\1\uffff\1\11",
            "",
            "",
            "\1\14\5\uffff\1\13\1\uffff\1\12\14\uffff\1\14\1\uffff\1\14\1"+
            "\uffff\1\14\1\uffff\2\13",
            "",
            "\1\7\6\uffff\2\7\2\uffff\1\10\1\7",
            "",
            "",
            "",
            "\1\15",
            "",
            "",
            "\1\17\1\uffff\1\16\3\uffff\1\13\16\uffff\1\17\1\uffff\1\17\1"+
            "\uffff\1\17\1\uffff\2\13",
            "\1\17\1\uffff\1\16\3\uffff\1\13\16\uffff\1\17\1\uffff\1\17\1"+
            "\uffff\1\17\1\uffff\2\13",
            ""
    };

    static final short[] DFA7_eot = DFA.unpackEncodedString(DFA7_eotS);
    static final short[] DFA7_eof = DFA.unpackEncodedString(DFA7_eofS);
    static final char[] DFA7_min = DFA.unpackEncodedStringToUnsignedChars(DFA7_minS);
    static final char[] DFA7_max = DFA.unpackEncodedStringToUnsignedChars(DFA7_maxS);
    static final short[] DFA7_accept = DFA.unpackEncodedString(DFA7_acceptS);
    static final short[] DFA7_special = DFA.unpackEncodedString(DFA7_specialS);
    static final short[][] DFA7_transition;

    static {
        int numStates = DFA7_transitionS.length;
        DFA7_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA7_transition[i] = DFA.unpackEncodedString(DFA7_transitionS[i]);
        }
    }

    class DFA7 extends DFA {

        public DFA7(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 7;
            this.eot = DFA7_eot;
            this.eof = DFA7_eof;
            this.min = DFA7_min;
            this.max = DFA7_max;
            this.accept = DFA7_accept;
            this.special = DFA7_special;
            this.transition = DFA7_transition;
        }
        public String getDescription() {
            return "598:1: rule__WMLKeyValue__Key1ValueAlternatives_0_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFLOAT ) | ( RULE_IINT ) | ( rulePATH ) | ( ruleDIRECTION ) | ( ruleLIST ) | ( rulePROGRESSIVE ) );";
        }
    }
    static final String DFA19_eotS =
        "\5\uffff";
    static final String DFA19_eofS =
        "\5\uffff";
    static final String DFA19_minS =
        "\3\4\2\uffff";
    static final String DFA19_maxS =
        "\1\4\2\20\2\uffff";
    static final String DFA19_acceptS =
        "\3\uffff\1\2\1\1";
    static final String DFA19_specialS =
        "\5\uffff}>";
    static final String[] DFA19_transitionS = {
            "\1\1",
            "\1\2\6\uffff\1\4\1\3\3\uffff\1\4",
            "\1\2\6\uffff\1\4\1\3\3\uffff\1\4",
            "",
            ""
    };

    static final short[] DFA19_eot = DFA.unpackEncodedString(DFA19_eotS);
    static final short[] DFA19_eof = DFA.unpackEncodedString(DFA19_eofS);
    static final char[] DFA19_min = DFA.unpackEncodedStringToUnsignedChars(DFA19_minS);
    static final char[] DFA19_max = DFA.unpackEncodedStringToUnsignedChars(DFA19_maxS);
    static final short[] DFA19_accept = DFA.unpackEncodedString(DFA19_acceptS);
    static final short[] DFA19_special = DFA.unpackEncodedString(DFA19_specialS);
    static final short[][] DFA19_transition;

    static {
        int numStates = DFA19_transitionS.length;
        DFA19_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA19_transition[i] = DFA.unpackEncodedString(DFA19_transitionS[i]);
        }
    }

    class DFA19 extends DFA {

        public DFA19(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 19;
            this.eot = DFA19_eot;
            this.eof = DFA19_eof;
            this.min = DFA19_min;
            this.max = DFA19_max;
            this.accept = DFA19_accept;
            this.special = DFA19_special;
            this.transition = DFA19_transition;
        }
        public String getDescription() {
            return "()* loopback of 1636:1: ( rule__PATH__Group_0__0 )*";
        }
    }
 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot61 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot68 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__Alternatives_in_ruleWMLRoot94 = new BitSet(new long[]{0x000000000A000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_entryRuleWMLMacro122 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacro129 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__0_in_ruleWMLMacro155 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag182 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag189 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0_in_ruleWMLTag215 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLStartTag_in_entryRuleWMLStartTag242 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLStartTag249 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLStartTag__Group__0_in_ruleWMLStartTag275 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLEndTag_in_entryRuleWMLEndTag302 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLEndTag309 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__0_in_ruleWMLEndTag335 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey362 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey369 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey395 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue422 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue429 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue455 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_entryRuleFLOAT487 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleFLOAT494 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__0_in_ruleFLOAT524 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING551 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING558 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0_in_ruleTSTRING584 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_entryRulePATH611 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePATH618 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group__0_in_rulePATH644 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_entryRuleDIRECTION671 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleDIRECTION678 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION706 = new BitSet(new long[]{0x0000000001FE0002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION718 = new BitSet(new long[]{0x0000000001FE0002L});
    public static final BitSet FOLLOW_ruleLIST_in_entryRuleLIST748 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleLIST755 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group__0_in_ruleLIST781 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE808 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePROGRESSIVE815 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__0_in_rulePROGRESSIVE841 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__RtagsAssignment_0_in_rule__WMLRoot__Alternatives877 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__RmacrosAssignment_1_in_rule__WMLRoot__Alternatives895 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacro__TagcontentAlternatives_2_0928 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLMacro__TagcontentAlternatives_2_0945 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_9_in_rule__WMLMacro__TagcontentAlternatives_2_0963 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_rule__WMLMacro__TagcontentAlternatives_2_0983 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_rule__WMLMacro__TagcontentAlternatives_2_01003 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__WMLMacro__TagcontentAlternatives_2_01023 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__WMLMacro__TagcontentAlternatives_2_01043 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__WMLMacro__TagcontentAlternatives_2_01063 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLMacro__TagcontentAlternatives_2_01083 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__WMLMacro__TagcontentAlternatives_2_01103 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TtagsAssignment_1_0_in_rule__WMLTag__Alternatives_11137 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TkeysAssignment_1_1_in_rule__WMLTag__Alternatives_11155 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TmacrosAssignment_1_2_in_rule__WMLTag__Alternatives_11173 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Key1ValueAssignment_0_in_rule__WMLKeyValue__Alternatives1206 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Key2ValueAssignment_1_in_rule__WMLKeyValue__Alternatives1224 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01257 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01274 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01291 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01308 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01325 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01342 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01359 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleLIST_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01376 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01393 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_rule__PATH__Alternatives_0_11426 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__PATH__Alternatives_0_11446 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_rule__DIRECTION__Alternatives_01481 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_rule__DIRECTION__Alternatives_01501 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_rule__DIRECTION__Alternatives_01521 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__DIRECTION__Alternatives_01541 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__DIRECTION__Alternatives_01561 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__DIRECTION__Alternatives_01581 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__DIRECTION__Alternatives_01601 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_rule__DIRECTION__Alternatives_01621 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_01655 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_01672 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_1_11704 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_1_11721 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_11753 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_11770 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_2_11802 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_2_11819 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__0__Impl_in_rule__WMLMacro__Group__01849 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__1_in_rule__WMLMacro__Group__01852 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__WMLMacro__Group__0__Impl1880 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__1__Impl_in_rule__WMLMacro__Group__11911 = new BitSet(new long[]{0x000000000401FE30L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__2_in_rule__WMLMacro__Group__11914 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__MacroNameAssignment_1_in_rule__WMLMacro__Group__1__Impl1941 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__2__Impl_in_rule__WMLMacro__Group__21971 = new BitSet(new long[]{0x0000000004000000L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__3_in_rule__WMLMacro__Group__21974 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__TagcontentAssignment_2_in_rule__WMLMacro__Group__2__Impl2001 = new BitSet(new long[]{0x000000000001FE32L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__3__Impl_in_rule__WMLMacro__Group__32032 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_rule__WMLMacro__Group__3__Impl2060 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__02099 = new BitSet(new long[]{0x000000002A000010L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__02102 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__StartAssignment_0_in_rule__WMLTag__Group__0__Impl2129 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__12159 = new BitSet(new long[]{0x0000000020000000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__12162 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Alternatives_1_in_rule__WMLTag__Group__1__Impl2189 = new BitSet(new long[]{0x000000000A000012L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__22220 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__EndAssignment_2_in_rule__WMLTag__Group__2__Impl2247 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLStartTag__Group__0__Impl_in_rule__WMLStartTag__Group__02283 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLStartTag__Group__1_in_rule__WMLStartTag__Group__02286 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rule__WMLStartTag__Group__0__Impl2314 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLStartTag__Group__1__Impl_in_rule__WMLStartTag__Group__12345 = new BitSet(new long[]{0x0000000010000000L});
    public static final BitSet FOLLOW_rule__WMLStartTag__Group__2_in_rule__WMLStartTag__Group__12348 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLStartTag__TagnameAssignment_1_in_rule__WMLStartTag__Group__1__Impl2375 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLStartTag__Group__2__Impl_in_rule__WMLStartTag__Group__22405 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__WMLStartTag__Group__2__Impl2433 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__0__Impl_in_rule__WMLEndTag__Group__02470 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__1_in_rule__WMLEndTag__Group__02473 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__WMLEndTag__Group__0__Impl2501 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__1__Impl_in_rule__WMLEndTag__Group__12532 = new BitSet(new long[]{0x0000000010000000L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__2_in_rule__WMLEndTag__Group__12535 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__TagnameAssignment_1_in_rule__WMLEndTag__Group__1__Impl2562 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__2__Impl_in_rule__WMLEndTag__Group__22592 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__WMLEndTag__Group__2__Impl2620 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02657 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02660 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__KeyNameAssignment_0_in_rule__WMLKey__Group__0__Impl2687 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12717 = new BitSet(new long[]{0x0000000043FE0270L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12720 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLKey__Group__1__Impl2748 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22779 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2806 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__0__Impl_in_rule__FLOAT__Group__02842 = new BitSet(new long[]{0x0000000000001000L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__1_in_rule__FLOAT__Group__02845 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__FLOAT__Group__0__Impl2872 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__1__Impl_in_rule__FLOAT__Group__12901 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__2_in_rule__FLOAT__Group__12904 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__FLOAT__Group__1__Impl2932 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__2__Impl_in_rule__FLOAT__Group__22963 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2993 = new BitSet(new long[]{0x0000000000000042L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl3006 = new BitSet(new long[]{0x0000000000000042L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__03045 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__03048 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__0_in_rule__TSTRING__Group__0__Impl3075 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__13105 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl3132 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__0__Impl_in_rule__TSTRING__Group_0__03165 = new BitSet(new long[]{0x0000000000000200L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__1_in_rule__TSTRING__Group_0__03168 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__TSTRING__Group_0__0__Impl3197 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__1__Impl_in_rule__TSTRING__Group_0__13230 = new BitSet(new long[]{0x0000000040000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__2_in_rule__TSTRING__Group_0__13233 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_9_in_rule__TSTRING__Group_0__1__Impl3261 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__2__Impl_in_rule__TSTRING__Group_0__23292 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__TSTRING__Group_0__2__Impl3321 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group__0__Impl_in_rule__PATH__Group__03360 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__PATH__Group__1_in_rule__PATH__Group__03363 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__0_in_rule__PATH__Group__0__Impl3390 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group__1__Impl_in_rule__PATH__Group__13421 = new BitSet(new long[]{0x0000000000001000L});
    public static final BitSet FOLLOW_rule__PATH__Group__2_in_rule__PATH__Group__13424 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl3454 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl3467 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group__2__Impl_in_rule__PATH__Group__23500 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__PATH__Group__3_in_rule__PATH__Group__23503 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__PATH__Group__2__Impl3531 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group__3__Impl_in_rule__PATH__Group__33562 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl3592 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl3605 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__0__Impl_in_rule__PATH__Group_0__03646 = new BitSet(new long[]{0x0000000000010800L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__1_in_rule__PATH__Group_0__03649 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl3679 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl3692 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__1__Impl_in_rule__PATH__Group_0__13725 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Alternatives_0_1_in_rule__PATH__Group_0__1__Impl3752 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__0__Impl_in_rule__DIRECTION__Group__03786 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__1_in_rule__DIRECTION__Group__03789 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Alternatives_0_in_rule__DIRECTION__Group__0__Impl3816 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__1__Impl_in_rule__DIRECTION__Group__13846 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__DIRECTION__Group__1__Impl3875 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group__0__Impl_in_rule__LIST__Group__03912 = new BitSet(new long[]{0x0000000080000000L});
    public static final BitSet FOLLOW_rule__LIST__Group__1_in_rule__LIST__Group__03915 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__LIST__Group__0__Impl3942 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group__1__Impl_in_rule__LIST__Group__13971 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl4000 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl4012 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__0__Impl_in_rule__LIST__Group_1__04049 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__1_in_rule__LIST__Group_1__04052 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__LIST__Group_1__0__Impl4080 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__1__Impl_in_rule__LIST__Group_1__14111 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__LIST__Group_1__1__Impl4138 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__0__Impl_in_rule__PROGRESSIVE__Group__04171 = new BitSet(new long[]{0x0000000180000400L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__1_in_rule__PROGRESSIVE__Group__04174 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_0_in_rule__PROGRESSIVE__Group__0__Impl4201 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__1__Impl_in_rule__PROGRESSIVE__Group__14231 = new BitSet(new long[]{0x0000000080000400L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__2_in_rule__PROGRESSIVE__Group__14234 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__0_in_rule__PROGRESSIVE__Group__1__Impl4261 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__2__Impl_in_rule__PROGRESSIVE__Group__24292 = new BitSet(new long[]{0x0000000080000000L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__3_in_rule__PROGRESSIVE__Group__24295 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__0_in_rule__PROGRESSIVE__Group__2__Impl4322 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__3__Impl_in_rule__PROGRESSIVE__Group__34353 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl4382 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl4394 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__0__Impl_in_rule__PROGRESSIVE__Group_1__04435 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__1_in_rule__PROGRESSIVE__Group_1__04438 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__PROGRESSIVE__Group_1__0__Impl4466 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__1__Impl_in_rule__PROGRESSIVE__Group_1__14497 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_1_1_in_rule__PROGRESSIVE__Group_1__1__Impl4524 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__0__Impl_in_rule__PROGRESSIVE__Group_2__04558 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__1_in_rule__PROGRESSIVE__Group_2__04561 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_rule__PROGRESSIVE__Group_2__0__Impl4589 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__1__Impl_in_rule__PROGRESSIVE__Group_2__14620 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_2__1__Impl4647 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__0__Impl_in_rule__PROGRESSIVE__Group_3__04680 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__1_in_rule__PROGRESSIVE__Group_3__04683 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__PROGRESSIVE__Group_3__0__Impl4711 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__1__Impl_in_rule__PROGRESSIVE__Group_3__14742 = new BitSet(new long[]{0x0000000100000402L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__2_in_rule__PROGRESSIVE__Group_3__14745 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_3_1_in_rule__PROGRESSIVE__Group_3__1__Impl4772 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__2__Impl_in_rule__PROGRESSIVE__Group_3__24802 = new BitSet(new long[]{0x0000000000000402L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__3_in_rule__PROGRESSIVE__Group_3__24805 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__0_in_rule__PROGRESSIVE__Group_3__2__Impl4832 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__3__Impl_in_rule__PROGRESSIVE__Group_3__34863 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__0_in_rule__PROGRESSIVE__Group_3__3__Impl4890 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__0__Impl_in_rule__PROGRESSIVE__Group_3_2__04929 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__1_in_rule__PROGRESSIVE__Group_3_2__04932 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__PROGRESSIVE__Group_3_2__0__Impl4960 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__1__Impl_in_rule__PROGRESSIVE__Group_3_2__14991 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_3_2_1_in_rule__PROGRESSIVE__Group_3_2__1__Impl5018 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__0__Impl_in_rule__PROGRESSIVE__Group_3_3__05052 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__1_in_rule__PROGRESSIVE__Group_3_3__05055 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_rule__PROGRESSIVE__Group_3_3__0__Impl5083 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__1__Impl_in_rule__PROGRESSIVE__Group_3_3__15114 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_3_3__1__Impl5141 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRoot__RtagsAssignment_05179 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLRoot__RmacrosAssignment_15210 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacro__MacroNameAssignment_15241 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__TagcontentAlternatives_2_0_in_rule__WMLMacro__TagcontentAssignment_25272 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLStartTag_in_rule__WMLTag__StartAssignment_05305 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLTag__TtagsAssignment_1_05336 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLTag__TkeysAssignment_1_15367 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLTag__TmacrosAssignment_1_25398 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLEndTag_in_rule__WMLTag__EndAssignment_25429 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLStartTag__TagnameAssignment_15460 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLEndTag__TagnameAssignment_15491 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__KeyNameAssignment_05522 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_25553 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Key1ValueAlternatives_0_0_in_rule__WMLKeyValue__Key1ValueAssignment_05584 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLKeyValue__Key2ValueAssignment_15617 = new BitSet(new long[]{0x0000000000000002L});

}