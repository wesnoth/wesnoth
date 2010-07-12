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


    // $ANTLR start entryRuleWMLEndTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:145:1: entryRuleWMLEndTag : ruleWMLEndTag EOF ;
    public final void entryRuleWMLEndTag() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:146:1: ( ruleWMLEndTag EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:147:1: ruleWMLEndTag EOF
            {
             before(grammarAccess.getWMLEndTagRule()); 
            pushFollow(FOLLOW_ruleWMLEndTag_in_entryRuleWMLEndTag242);
            ruleWMLEndTag();
            _fsp--;

             after(grammarAccess.getWMLEndTagRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLEndTag249); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:154:1: ruleWMLEndTag : ( ( rule__WMLEndTag__Group__0 ) ) ;
    public final void ruleWMLEndTag() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:158:2: ( ( ( rule__WMLEndTag__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:159:1: ( ( rule__WMLEndTag__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:159:1: ( ( rule__WMLEndTag__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:160:1: ( rule__WMLEndTag__Group__0 )
            {
             before(grammarAccess.getWMLEndTagAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:161:1: ( rule__WMLEndTag__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:161:2: rule__WMLEndTag__Group__0
            {
            pushFollow(FOLLOW_rule__WMLEndTag__Group__0_in_ruleWMLEndTag275);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:173:1: entryRuleWMLKey : ruleWMLKey EOF ;
    public final void entryRuleWMLKey() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:174:1: ( ruleWMLKey EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:175:1: ruleWMLKey EOF
            {
             before(grammarAccess.getWMLKeyRule()); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey302);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLKeyRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey309); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:182:1: ruleWMLKey : ( ( rule__WMLKey__Group__0 ) ) ;
    public final void ruleWMLKey() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:186:2: ( ( ( rule__WMLKey__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:187:1: ( ( rule__WMLKey__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:187:1: ( ( rule__WMLKey__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:188:1: ( rule__WMLKey__Group__0 )
            {
             before(grammarAccess.getWMLKeyAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:189:1: ( rule__WMLKey__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:189:2: rule__WMLKey__Group__0
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey335);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:201:1: entryRuleWMLKeyValue : ruleWMLKeyValue EOF ;
    public final void entryRuleWMLKeyValue() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:202:1: ( ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:203:1: ruleWMLKeyValue EOF
            {
             before(grammarAccess.getWMLKeyValueRule()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue362);
            ruleWMLKeyValue();
            _fsp--;

             after(grammarAccess.getWMLKeyValueRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue369); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:210:1: ruleWMLKeyValue : ( ( rule__WMLKeyValue__Alternatives ) ) ;
    public final void ruleWMLKeyValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:214:2: ( ( ( rule__WMLKeyValue__Alternatives ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:215:1: ( ( rule__WMLKeyValue__Alternatives ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:215:1: ( ( rule__WMLKeyValue__Alternatives ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:216:1: ( rule__WMLKeyValue__Alternatives )
            {
             before(grammarAccess.getWMLKeyValueAccess().getAlternatives()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:217:1: ( rule__WMLKeyValue__Alternatives )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:217:2: rule__WMLKeyValue__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue395);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:229:1: entryRuleFLOAT : ruleFLOAT EOF ;
    public final void entryRuleFLOAT() throws RecognitionException {

        	HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();

        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:233:1: ( ruleFLOAT EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:234:1: ruleFLOAT EOF
            {
             before(grammarAccess.getFLOATRule()); 
            pushFollow(FOLLOW_ruleFLOAT_in_entryRuleFLOAT427);
            ruleFLOAT();
            _fsp--;

             after(grammarAccess.getFLOATRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleFLOAT434); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:244:1: ruleFLOAT : ( ( rule__FLOAT__Group__0 ) ) ;
    public final void ruleFLOAT() throws RecognitionException {

        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();
        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:249:2: ( ( ( rule__FLOAT__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:250:1: ( ( rule__FLOAT__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:250:1: ( ( rule__FLOAT__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:251:1: ( rule__FLOAT__Group__0 )
            {
             before(grammarAccess.getFLOATAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:252:1: ( rule__FLOAT__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:252:2: rule__FLOAT__Group__0
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__0_in_ruleFLOAT464);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:265:1: entryRuleTSTRING : ruleTSTRING EOF ;
    public final void entryRuleTSTRING() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:266:1: ( ruleTSTRING EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:267:1: ruleTSTRING EOF
            {
             before(grammarAccess.getTSTRINGRule()); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING491);
            ruleTSTRING();
            _fsp--;

             after(grammarAccess.getTSTRINGRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING498); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:274:1: ruleTSTRING : ( ( rule__TSTRING__Group__0 ) ) ;
    public final void ruleTSTRING() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:278:2: ( ( ( rule__TSTRING__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:279:1: ( ( rule__TSTRING__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:279:1: ( ( rule__TSTRING__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:280:1: ( rule__TSTRING__Group__0 )
            {
             before(grammarAccess.getTSTRINGAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:1: ( rule__TSTRING__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:2: rule__TSTRING__Group__0
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0_in_ruleTSTRING524);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:293:1: entryRulePATH : rulePATH EOF ;
    public final void entryRulePATH() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:294:1: ( rulePATH EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:295:1: rulePATH EOF
            {
             before(grammarAccess.getPATHRule()); 
            pushFollow(FOLLOW_rulePATH_in_entryRulePATH551);
            rulePATH();
            _fsp--;

             after(grammarAccess.getPATHRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePATH558); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:302:1: rulePATH : ( ( rule__PATH__Group__0 ) ) ;
    public final void rulePATH() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:306:2: ( ( ( rule__PATH__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:307:1: ( ( rule__PATH__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:307:1: ( ( rule__PATH__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:308:1: ( rule__PATH__Group__0 )
            {
             before(grammarAccess.getPATHAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:1: ( rule__PATH__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:2: rule__PATH__Group__0
            {
            pushFollow(FOLLOW_rule__PATH__Group__0_in_rulePATH584);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:321:1: entryRuleDIRECTION : ruleDIRECTION EOF ;
    public final void entryRuleDIRECTION() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:322:1: ( ruleDIRECTION EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:323:1: ruleDIRECTION EOF
            {
             before(grammarAccess.getDIRECTIONRule()); 
            pushFollow(FOLLOW_ruleDIRECTION_in_entryRuleDIRECTION611);
            ruleDIRECTION();
            _fsp--;

             after(grammarAccess.getDIRECTIONRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleDIRECTION618); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:330:1: ruleDIRECTION : ( ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* ) ) ;
    public final void ruleDIRECTION() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:334:2: ( ( ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:335:1: ( ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:335:1: ( ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:336:1: ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:336:1: ( ( rule__DIRECTION__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:1: ( rule__DIRECTION__Group__0 )
            {
             before(grammarAccess.getDIRECTIONAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:338:1: ( rule__DIRECTION__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:338:2: rule__DIRECTION__Group__0
            {
            pushFollow(FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION646);
            rule__DIRECTION__Group__0();
            _fsp--;


            }

             after(grammarAccess.getDIRECTIONAccess().getGroup()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:341:1: ( ( rule__DIRECTION__Group__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:342:1: ( rule__DIRECTION__Group__0 )*
            {
             before(grammarAccess.getDIRECTIONAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:343:1: ( rule__DIRECTION__Group__0 )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( ((LA2_0>=17 && LA2_0<=24)) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:343:2: rule__DIRECTION__Group__0
            	    {
            	    pushFollow(FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION658);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:356:1: entryRuleLIST : ruleLIST EOF ;
    public final void entryRuleLIST() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:357:1: ( ruleLIST EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:358:1: ruleLIST EOF
            {
             before(grammarAccess.getLISTRule()); 
            pushFollow(FOLLOW_ruleLIST_in_entryRuleLIST688);
            ruleLIST();
            _fsp--;

             after(grammarAccess.getLISTRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleLIST695); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:365:1: ruleLIST : ( ( rule__LIST__Group__0 ) ) ;
    public final void ruleLIST() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:369:2: ( ( ( rule__LIST__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:370:1: ( ( rule__LIST__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:370:1: ( ( rule__LIST__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:371:1: ( rule__LIST__Group__0 )
            {
             before(grammarAccess.getLISTAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:372:1: ( rule__LIST__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:372:2: rule__LIST__Group__0
            {
            pushFollow(FOLLOW_rule__LIST__Group__0_in_ruleLIST721);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:384:1: entryRulePROGRESSIVE : rulePROGRESSIVE EOF ;
    public final void entryRulePROGRESSIVE() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:385:1: ( rulePROGRESSIVE EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:386:1: rulePROGRESSIVE EOF
            {
             before(grammarAccess.getPROGRESSIVERule()); 
            pushFollow(FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE748);
            rulePROGRESSIVE();
            _fsp--;

             after(grammarAccess.getPROGRESSIVERule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePROGRESSIVE755); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:393:1: rulePROGRESSIVE : ( ( rule__PROGRESSIVE__Group__0 ) ) ;
    public final void rulePROGRESSIVE() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:397:2: ( ( ( rule__PROGRESSIVE__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:398:1: ( ( rule__PROGRESSIVE__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:398:1: ( ( rule__PROGRESSIVE__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:399:1: ( rule__PROGRESSIVE__Group__0 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:400:1: ( rule__PROGRESSIVE__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:400:2: rule__PROGRESSIVE__Group__0
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__0_in_rulePROGRESSIVE781);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:412:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__RtagsAssignment_0 ) ) | ( ( rule__WMLRoot__RmacrosAssignment_1 ) ) );
    public final void rule__WMLRoot__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:416:1: ( ( ( rule__WMLRoot__RtagsAssignment_0 ) ) | ( ( rule__WMLRoot__RmacrosAssignment_1 ) ) )
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
                    new NoViableAltException("412:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__RtagsAssignment_0 ) ) | ( ( rule__WMLRoot__RmacrosAssignment_1 ) ) );", 3, 0, input);

                throw nvae;
            }
            switch (alt3) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:417:1: ( ( rule__WMLRoot__RtagsAssignment_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:417:1: ( ( rule__WMLRoot__RtagsAssignment_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:418:1: ( rule__WMLRoot__RtagsAssignment_0 )
                    {
                     before(grammarAccess.getWMLRootAccess().getRtagsAssignment_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:419:1: ( rule__WMLRoot__RtagsAssignment_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:419:2: rule__WMLRoot__RtagsAssignment_0
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__RtagsAssignment_0_in_rule__WMLRoot__Alternatives817);
                    rule__WMLRoot__RtagsAssignment_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getRtagsAssignment_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:423:6: ( ( rule__WMLRoot__RmacrosAssignment_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:423:6: ( ( rule__WMLRoot__RmacrosAssignment_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:424:1: ( rule__WMLRoot__RmacrosAssignment_1 )
                    {
                     before(grammarAccess.getWMLRootAccess().getRmacrosAssignment_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:425:1: ( rule__WMLRoot__RmacrosAssignment_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:425:2: rule__WMLRoot__RmacrosAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__RmacrosAssignment_1_in_rule__WMLRoot__Alternatives835);
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


    // $ANTLR start rule__WMLMacro__ValueAlternatives_2_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:434:1: rule__WMLMacro__ValueAlternatives_2_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( ':' ) | ( '-' ) | ( '.' ) | ( '(' ) | ( ')' ) | ( '=' ) | ( '/' ) );
    public final void rule__WMLMacro__ValueAlternatives_2_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:438:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( ':' ) | ( '-' ) | ( '.' ) | ( '(' ) | ( ')' ) | ( '=' ) | ( '/' ) )
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
                    new NoViableAltException("434:1: rule__WMLMacro__ValueAlternatives_2_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( ':' ) | ( '-' ) | ( '.' ) | ( '(' ) | ( ')' ) | ( '=' ) | ( '/' ) );", 4, 0, input);

                throw nvae;
            }

            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:439:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:439:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:440:1: RULE_ID
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueIDTerminalRuleCall_2_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacro__ValueAlternatives_2_0868); 
                     after(grammarAccess.getWMLMacroAccess().getValueIDTerminalRuleCall_2_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:445:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:445:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:446:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueSTRINGTerminalRuleCall_2_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLMacro__ValueAlternatives_2_0885); 
                     after(grammarAccess.getWMLMacroAccess().getValueSTRINGTerminalRuleCall_2_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:451:6: ( '_' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:451:6: ( '_' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:452:1: '_'
                    {
                     before(grammarAccess.getWMLMacroAccess().getValue_Keyword_2_0_2()); 
                    match(input,9,FOLLOW_9_in_rule__WMLMacro__ValueAlternatives_2_0903); 
                     after(grammarAccess.getWMLMacroAccess().getValue_Keyword_2_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:459:6: ( ':' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:459:6: ( ':' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:460:1: ':'
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueColonKeyword_2_0_3()); 
                    match(input,10,FOLLOW_10_in_rule__WMLMacro__ValueAlternatives_2_0923); 
                     after(grammarAccess.getWMLMacroAccess().getValueColonKeyword_2_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:467:6: ( '-' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:467:6: ( '-' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:468:1: '-'
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueHyphenMinusKeyword_2_0_4()); 
                    match(input,11,FOLLOW_11_in_rule__WMLMacro__ValueAlternatives_2_0943); 
                     after(grammarAccess.getWMLMacroAccess().getValueHyphenMinusKeyword_2_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:475:6: ( '.' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:475:6: ( '.' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:476:1: '.'
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueFullStopKeyword_2_0_5()); 
                    match(input,12,FOLLOW_12_in_rule__WMLMacro__ValueAlternatives_2_0963); 
                     after(grammarAccess.getWMLMacroAccess().getValueFullStopKeyword_2_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:483:6: ( '(' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:483:6: ( '(' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:484:1: '('
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueLeftParenthesisKeyword_2_0_6()); 
                    match(input,13,FOLLOW_13_in_rule__WMLMacro__ValueAlternatives_2_0983); 
                     after(grammarAccess.getWMLMacroAccess().getValueLeftParenthesisKeyword_2_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:491:6: ( ')' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:491:6: ( ')' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:492:1: ')'
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueRightParenthesisKeyword_2_0_7()); 
                    match(input,14,FOLLOW_14_in_rule__WMLMacro__ValueAlternatives_2_01003); 
                     after(grammarAccess.getWMLMacroAccess().getValueRightParenthesisKeyword_2_0_7()); 

                    }


                    }
                    break;
                case 9 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:499:6: ( '=' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:499:6: ( '=' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:500:1: '='
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueEqualsSignKeyword_2_0_8()); 
                    match(input,15,FOLLOW_15_in_rule__WMLMacro__ValueAlternatives_2_01023); 
                     after(grammarAccess.getWMLMacroAccess().getValueEqualsSignKeyword_2_0_8()); 

                    }


                    }
                    break;
                case 10 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:507:6: ( '/' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:507:6: ( '/' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:508:1: '/'
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueSolidusKeyword_2_0_9()); 
                    match(input,16,FOLLOW_16_in_rule__WMLMacro__ValueAlternatives_2_01043); 
                     after(grammarAccess.getWMLMacroAccess().getValueSolidusKeyword_2_0_9()); 

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
    // $ANTLR end rule__WMLMacro__ValueAlternatives_2_0


    // $ANTLR start rule__WMLTag__Alternatives_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:520:1: rule__WMLTag__Alternatives_3 : ( ( ( rule__WMLTag__TtagsAssignment_3_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_3_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_3_2 ) ) );
    public final void rule__WMLTag__Alternatives_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:524:1: ( ( ( rule__WMLTag__TtagsAssignment_3_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_3_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_3_2 ) ) )
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
                    new NoViableAltException("520:1: rule__WMLTag__Alternatives_3 : ( ( ( rule__WMLTag__TtagsAssignment_3_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_3_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_3_2 ) ) );", 5, 0, input);

                throw nvae;
            }

            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:525:1: ( ( rule__WMLTag__TtagsAssignment_3_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:525:1: ( ( rule__WMLTag__TtagsAssignment_3_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:526:1: ( rule__WMLTag__TtagsAssignment_3_0 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTtagsAssignment_3_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:527:1: ( rule__WMLTag__TtagsAssignment_3_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:527:2: rule__WMLTag__TtagsAssignment_3_0
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TtagsAssignment_3_0_in_rule__WMLTag__Alternatives_31077);
                    rule__WMLTag__TtagsAssignment_3_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTtagsAssignment_3_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:531:6: ( ( rule__WMLTag__TkeysAssignment_3_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:531:6: ( ( rule__WMLTag__TkeysAssignment_3_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:532:1: ( rule__WMLTag__TkeysAssignment_3_1 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTkeysAssignment_3_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:1: ( rule__WMLTag__TkeysAssignment_3_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:2: rule__WMLTag__TkeysAssignment_3_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TkeysAssignment_3_1_in_rule__WMLTag__Alternatives_31095);
                    rule__WMLTag__TkeysAssignment_3_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTkeysAssignment_3_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:537:6: ( ( rule__WMLTag__TmacrosAssignment_3_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:537:6: ( ( rule__WMLTag__TmacrosAssignment_3_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:538:1: ( rule__WMLTag__TmacrosAssignment_3_2 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTmacrosAssignment_3_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:539:1: ( rule__WMLTag__TmacrosAssignment_3_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:539:2: rule__WMLTag__TmacrosAssignment_3_2
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TmacrosAssignment_3_2_in_rule__WMLTag__Alternatives_31113);
                    rule__WMLTag__TmacrosAssignment_3_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTmacrosAssignment_3_2()); 

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
    // $ANTLR end rule__WMLTag__Alternatives_3


    // $ANTLR start rule__WMLKeyValue__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:548:1: rule__WMLKeyValue__Alternatives : ( ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) ) | ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) ) );
    public final void rule__WMLKeyValue__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:552:1: ( ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) ) | ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) ) )
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
                    new NoViableAltException("548:1: rule__WMLKeyValue__Alternatives : ( ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) ) | ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) ) );", 6, 0, input);

                throw nvae;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:553:1: ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:553:1: ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:554:1: ( rule__WMLKeyValue__Key1ValueAssignment_0 )
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueAssignment_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:555:1: ( rule__WMLKeyValue__Key1ValueAssignment_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:555:2: rule__WMLKeyValue__Key1ValueAssignment_0
                    {
                    pushFollow(FOLLOW_rule__WMLKeyValue__Key1ValueAssignment_0_in_rule__WMLKeyValue__Alternatives1146);
                    rule__WMLKeyValue__Key1ValueAssignment_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueAssignment_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:559:6: ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:559:6: ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:560:1: ( rule__WMLKeyValue__Key2ValueAssignment_1 )
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey2ValueAssignment_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:561:1: ( rule__WMLKeyValue__Key2ValueAssignment_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:561:2: rule__WMLKeyValue__Key2ValueAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLKeyValue__Key2ValueAssignment_1_in_rule__WMLKeyValue__Alternatives1164);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:570:1: rule__WMLKeyValue__Key1ValueAlternatives_0_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFLOAT ) | ( RULE_IINT ) | ( rulePATH ) | ( ruleDIRECTION ) | ( ruleLIST ) | ( rulePROGRESSIVE ) );
    public final void rule__WMLKeyValue__Key1ValueAlternatives_0_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:574:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFLOAT ) | ( RULE_IINT ) | ( rulePATH ) | ( ruleDIRECTION ) | ( ruleLIST ) | ( rulePROGRESSIVE ) )
            int alt7=9;
            alt7 = dfa7.predict(input);
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:575:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:575:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:576:1: RULE_ID
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueIDTerminalRuleCall_0_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01197); 
                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueIDTerminalRuleCall_0_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:581:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:581:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:582:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueSTRINGTerminalRuleCall_0_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01214); 
                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueSTRINGTerminalRuleCall_0_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:587:6: ( ruleTSTRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:587:6: ( ruleTSTRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:588:1: ruleTSTRING
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueTSTRINGParserRuleCall_0_0_2()); 
                    pushFollow(FOLLOW_ruleTSTRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01231);
                    ruleTSTRING();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueTSTRINGParserRuleCall_0_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:593:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:593:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:594:1: ruleFLOAT
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueFLOATParserRuleCall_0_0_3()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01248);
                    ruleFLOAT();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueFLOATParserRuleCall_0_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:599:6: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:599:6: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:600:1: RULE_IINT
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueIINTTerminalRuleCall_0_0_4()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01265); 
                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueIINTTerminalRuleCall_0_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:605:6: ( rulePATH )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:605:6: ( rulePATH )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:606:1: rulePATH
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValuePATHParserRuleCall_0_0_5()); 
                    pushFollow(FOLLOW_rulePATH_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01282);
                    rulePATH();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValuePATHParserRuleCall_0_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:611:6: ( ruleDIRECTION )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:611:6: ( ruleDIRECTION )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:612:1: ruleDIRECTION
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueDIRECTIONParserRuleCall_0_0_6()); 
                    pushFollow(FOLLOW_ruleDIRECTION_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01299);
                    ruleDIRECTION();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueDIRECTIONParserRuleCall_0_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:617:6: ( ruleLIST )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:617:6: ( ruleLIST )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:618:1: ruleLIST
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueLISTParserRuleCall_0_0_7()); 
                    pushFollow(FOLLOW_ruleLIST_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01316);
                    ruleLIST();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueLISTParserRuleCall_0_0_7()); 

                    }


                    }
                    break;
                case 9 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:623:6: ( rulePROGRESSIVE )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:623:6: ( rulePROGRESSIVE )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:624:1: rulePROGRESSIVE
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValuePROGRESSIVEParserRuleCall_0_0_8()); 
                    pushFollow(FOLLOW_rulePROGRESSIVE_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01333);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:634:1: rule__PATH__Alternatives_0_1 : ( ( '-' ) | ( '/' ) );
    public final void rule__PATH__Alternatives_0_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:638:1: ( ( '-' ) | ( '/' ) )
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
                    new NoViableAltException("634:1: rule__PATH__Alternatives_0_1 : ( ( '-' ) | ( '/' ) );", 8, 0, input);

                throw nvae;
            }
            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:639:1: ( '-' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:639:1: ( '-' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:640:1: '-'
                    {
                     before(grammarAccess.getPATHAccess().getHyphenMinusKeyword_0_1_0()); 
                    match(input,11,FOLLOW_11_in_rule__PATH__Alternatives_0_11366); 
                     after(grammarAccess.getPATHAccess().getHyphenMinusKeyword_0_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:647:6: ( '/' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:647:6: ( '/' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:648:1: '/'
                    {
                     before(grammarAccess.getPATHAccess().getSolidusKeyword_0_1_1()); 
                    match(input,16,FOLLOW_16_in_rule__PATH__Alternatives_0_11386); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:660:1: rule__DIRECTION__Alternatives_0 : ( ( 'n' ) | ( 's' ) | ( 'w' ) | ( 'e' ) | ( 'sw' ) | ( 'se' ) | ( 'ne' ) | ( 'nw' ) );
    public final void rule__DIRECTION__Alternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:664:1: ( ( 'n' ) | ( 's' ) | ( 'w' ) | ( 'e' ) | ( 'sw' ) | ( 'se' ) | ( 'ne' ) | ( 'nw' ) )
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
                    new NoViableAltException("660:1: rule__DIRECTION__Alternatives_0 : ( ( 'n' ) | ( 's' ) | ( 'w' ) | ( 'e' ) | ( 'sw' ) | ( 'se' ) | ( 'ne' ) | ( 'nw' ) );", 9, 0, input);

                throw nvae;
            }

            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:665:1: ( 'n' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:665:1: ( 'n' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:666:1: 'n'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getNKeyword_0_0()); 
                    match(input,17,FOLLOW_17_in_rule__DIRECTION__Alternatives_01421); 
                     after(grammarAccess.getDIRECTIONAccess().getNKeyword_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:673:6: ( 's' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:673:6: ( 's' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:674:1: 's'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getSKeyword_0_1()); 
                    match(input,18,FOLLOW_18_in_rule__DIRECTION__Alternatives_01441); 
                     after(grammarAccess.getDIRECTIONAccess().getSKeyword_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:681:6: ( 'w' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:681:6: ( 'w' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:682:1: 'w'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getWKeyword_0_2()); 
                    match(input,19,FOLLOW_19_in_rule__DIRECTION__Alternatives_01461); 
                     after(grammarAccess.getDIRECTIONAccess().getWKeyword_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:689:6: ( 'e' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:689:6: ( 'e' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:690:1: 'e'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getEKeyword_0_3()); 
                    match(input,20,FOLLOW_20_in_rule__DIRECTION__Alternatives_01481); 
                     after(grammarAccess.getDIRECTIONAccess().getEKeyword_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:697:6: ( 'sw' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:697:6: ( 'sw' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:698:1: 'sw'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getSwKeyword_0_4()); 
                    match(input,21,FOLLOW_21_in_rule__DIRECTION__Alternatives_01501); 
                     after(grammarAccess.getDIRECTIONAccess().getSwKeyword_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:705:6: ( 'se' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:705:6: ( 'se' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:706:1: 'se'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getSeKeyword_0_5()); 
                    match(input,22,FOLLOW_22_in_rule__DIRECTION__Alternatives_01521); 
                     after(grammarAccess.getDIRECTIONAccess().getSeKeyword_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:713:6: ( 'ne' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:713:6: ( 'ne' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:714:1: 'ne'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getNeKeyword_0_6()); 
                    match(input,23,FOLLOW_23_in_rule__DIRECTION__Alternatives_01541); 
                     after(grammarAccess.getDIRECTIONAccess().getNeKeyword_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:721:6: ( 'nw' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:721:6: ( 'nw' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:722:1: 'nw'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getNwKeyword_0_7()); 
                    match(input,24,FOLLOW_24_in_rule__DIRECTION__Alternatives_01561); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:734:1: rule__PROGRESSIVE__Alternatives_0 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:738:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
            int alt10=2;
            int LA10_0 = input.LA(1);

            if ( (LA10_0==RULE_IINT) ) {
                int LA10_1 = input.LA(2);

                if ( (LA10_1==12) ) {
                    alt10=2;
                }
                else if ( (LA10_1==10||(LA10_1>=31 && LA10_1<=32)) ) {
                    alt10=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("734:1: rule__PROGRESSIVE__Alternatives_0 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 10, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("734:1: rule__PROGRESSIVE__Alternatives_0 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 10, 0, input);

                throw nvae;
            }
            switch (alt10) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:739:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:739:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:740:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_0_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_01595); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:745:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:745:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:746:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_0_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_01612);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:756:1: rule__PROGRESSIVE__Alternatives_1_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_1_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:760:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
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
                        new NoViableAltException("756:1: rule__PROGRESSIVE__Alternatives_1_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 11, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("756:1: rule__PROGRESSIVE__Alternatives_1_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 11, 0, input);

                throw nvae;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:761:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:761:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:762:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_1_1_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_1_11644); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_1_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:767:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:767:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:768:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_1_1_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_1_11661);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:778:1: rule__PROGRESSIVE__Alternatives_3_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_3_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:782:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
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
                        new NoViableAltException("778:1: rule__PROGRESSIVE__Alternatives_3_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 12, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("778:1: rule__PROGRESSIVE__Alternatives_3_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 12, 0, input);

                throw nvae;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:783:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:783:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:784:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_1_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_11693); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:789:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:789:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:790:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_1_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_11710);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:800:1: rule__PROGRESSIVE__Alternatives_3_2_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_3_2_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:804:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
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
                        new NoViableAltException("800:1: rule__PROGRESSIVE__Alternatives_3_2_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 13, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("800:1: rule__PROGRESSIVE__Alternatives_3_2_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 13, 0, input);

                throw nvae;
            }
            switch (alt13) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:805:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:805:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:806:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_2_1_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_2_11742); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_2_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:811:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:811:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:812:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_2_1_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_2_11759);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:824:1: rule__WMLMacro__Group__0 : rule__WMLMacro__Group__0__Impl rule__WMLMacro__Group__1 ;
    public final void rule__WMLMacro__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:828:1: ( rule__WMLMacro__Group__0__Impl rule__WMLMacro__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:829:2: rule__WMLMacro__Group__0__Impl rule__WMLMacro__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__0__Impl_in_rule__WMLMacro__Group__01789);
            rule__WMLMacro__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacro__Group__1_in_rule__WMLMacro__Group__01792);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:836:1: rule__WMLMacro__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacro__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:840:1: ( ( '{' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:841:1: ( '{' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:841:1: ( '{' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:842:1: '{'
            {
             before(grammarAccess.getWMLMacroAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,25,FOLLOW_25_in_rule__WMLMacro__Group__0__Impl1820); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:855:1: rule__WMLMacro__Group__1 : rule__WMLMacro__Group__1__Impl rule__WMLMacro__Group__2 ;
    public final void rule__WMLMacro__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:859:1: ( rule__WMLMacro__Group__1__Impl rule__WMLMacro__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:860:2: rule__WMLMacro__Group__1__Impl rule__WMLMacro__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__1__Impl_in_rule__WMLMacro__Group__11851);
            rule__WMLMacro__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacro__Group__2_in_rule__WMLMacro__Group__11854);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:867:1: rule__WMLMacro__Group__1__Impl : ( ( rule__WMLMacro__NameAssignment_1 ) ) ;
    public final void rule__WMLMacro__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:871:1: ( ( ( rule__WMLMacro__NameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:872:1: ( ( rule__WMLMacro__NameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:872:1: ( ( rule__WMLMacro__NameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:873:1: ( rule__WMLMacro__NameAssignment_1 )
            {
             before(grammarAccess.getWMLMacroAccess().getNameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:874:1: ( rule__WMLMacro__NameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:874:2: rule__WMLMacro__NameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLMacro__NameAssignment_1_in_rule__WMLMacro__Group__1__Impl1881);
            rule__WMLMacro__NameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroAccess().getNameAssignment_1()); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:884:1: rule__WMLMacro__Group__2 : rule__WMLMacro__Group__2__Impl rule__WMLMacro__Group__3 ;
    public final void rule__WMLMacro__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:888:1: ( rule__WMLMacro__Group__2__Impl rule__WMLMacro__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:889:2: rule__WMLMacro__Group__2__Impl rule__WMLMacro__Group__3
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__2__Impl_in_rule__WMLMacro__Group__21911);
            rule__WMLMacro__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacro__Group__3_in_rule__WMLMacro__Group__21914);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:896:1: rule__WMLMacro__Group__2__Impl : ( ( rule__WMLMacro__ValueAssignment_2 )* ) ;
    public final void rule__WMLMacro__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:900:1: ( ( ( rule__WMLMacro__ValueAssignment_2 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:901:1: ( ( rule__WMLMacro__ValueAssignment_2 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:901:1: ( ( rule__WMLMacro__ValueAssignment_2 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:902:1: ( rule__WMLMacro__ValueAssignment_2 )*
            {
             before(grammarAccess.getWMLMacroAccess().getValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:903:1: ( rule__WMLMacro__ValueAssignment_2 )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( ((LA14_0>=RULE_ID && LA14_0<=RULE_STRING)||(LA14_0>=9 && LA14_0<=16)) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:903:2: rule__WMLMacro__ValueAssignment_2
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacro__ValueAssignment_2_in_rule__WMLMacro__Group__2__Impl1941);
            	    rule__WMLMacro__ValueAssignment_2();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop14;
                }
            } while (true);

             after(grammarAccess.getWMLMacroAccess().getValueAssignment_2()); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:913:1: rule__WMLMacro__Group__3 : rule__WMLMacro__Group__3__Impl ;
    public final void rule__WMLMacro__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:917:1: ( rule__WMLMacro__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:918:2: rule__WMLMacro__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__3__Impl_in_rule__WMLMacro__Group__31972);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:924:1: rule__WMLMacro__Group__3__Impl : ( '}' ) ;
    public final void rule__WMLMacro__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:928:1: ( ( '}' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:929:1: ( '}' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:929:1: ( '}' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:930:1: '}'
            {
             before(grammarAccess.getWMLMacroAccess().getRightCurlyBracketKeyword_3()); 
            match(input,26,FOLLOW_26_in_rule__WMLMacro__Group__3__Impl2000); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:951:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:955:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:956:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__02039);
            rule__WMLTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__02042);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:963:1: rule__WMLTag__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:967:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:968:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:968:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:969:1: '['
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0()); 
            match(input,27,FOLLOW_27_in_rule__WMLTag__Group__0__Impl2070); 
             after(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0()); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:982:1: rule__WMLTag__Group__1 : rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 ;
    public final void rule__WMLTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:986:1: ( rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:987:2: rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__12101);
            rule__WMLTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__12104);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:994:1: rule__WMLTag__Group__1__Impl : ( ( rule__WMLTag__NameAssignment_1 ) ) ;
    public final void rule__WMLTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:998:1: ( ( ( rule__WMLTag__NameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:999:1: ( ( rule__WMLTag__NameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:999:1: ( ( rule__WMLTag__NameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1000:1: ( rule__WMLTag__NameAssignment_1 )
            {
             before(grammarAccess.getWMLTagAccess().getNameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1001:1: ( rule__WMLTag__NameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1001:2: rule__WMLTag__NameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLTag__NameAssignment_1_in_rule__WMLTag__Group__1__Impl2131);
            rule__WMLTag__NameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLTagAccess().getNameAssignment_1()); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1011:1: rule__WMLTag__Group__2 : rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 ;
    public final void rule__WMLTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1015:1: ( rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1016:2: rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__22161);
            rule__WMLTag__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__22164);
            rule__WMLTag__Group__3();
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1023:1: rule__WMLTag__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1027:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1028:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1028:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1029:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_2()); 
            match(input,28,FOLLOW_28_in_rule__WMLTag__Group__2__Impl2192); 
             after(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_2()); 

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


    // $ANTLR start rule__WMLTag__Group__3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1042:1: rule__WMLTag__Group__3 : rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 ;
    public final void rule__WMLTag__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1046:1: ( rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1047:2: rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__32223);
            rule__WMLTag__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__32226);
            rule__WMLTag__Group__4();
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
    // $ANTLR end rule__WMLTag__Group__3


    // $ANTLR start rule__WMLTag__Group__3__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1054:1: rule__WMLTag__Group__3__Impl : ( ( rule__WMLTag__Alternatives_3 )* ) ;
    public final void rule__WMLTag__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1058:1: ( ( ( rule__WMLTag__Alternatives_3 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1059:1: ( ( rule__WMLTag__Alternatives_3 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1059:1: ( ( rule__WMLTag__Alternatives_3 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1060:1: ( rule__WMLTag__Alternatives_3 )*
            {
             before(grammarAccess.getWMLTagAccess().getAlternatives_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1061:1: ( rule__WMLTag__Alternatives_3 )*
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( (LA15_0==RULE_ID||LA15_0==25||LA15_0==27) ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1061:2: rule__WMLTag__Alternatives_3
            	    {
            	    pushFollow(FOLLOW_rule__WMLTag__Alternatives_3_in_rule__WMLTag__Group__3__Impl2253);
            	    rule__WMLTag__Alternatives_3();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop15;
                }
            } while (true);

             after(grammarAccess.getWMLTagAccess().getAlternatives_3()); 

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
    // $ANTLR end rule__WMLTag__Group__3__Impl


    // $ANTLR start rule__WMLTag__Group__4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1071:1: rule__WMLTag__Group__4 : rule__WMLTag__Group__4__Impl ;
    public final void rule__WMLTag__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1075:1: ( rule__WMLTag__Group__4__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1076:2: rule__WMLTag__Group__4__Impl
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__42284);
            rule__WMLTag__Group__4__Impl();
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
    // $ANTLR end rule__WMLTag__Group__4


    // $ANTLR start rule__WMLTag__Group__4__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1082:1: rule__WMLTag__Group__4__Impl : ( ( rule__WMLTag__EndAssignment_4 ) ) ;
    public final void rule__WMLTag__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1086:1: ( ( ( rule__WMLTag__EndAssignment_4 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1087:1: ( ( rule__WMLTag__EndAssignment_4 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1087:1: ( ( rule__WMLTag__EndAssignment_4 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1088:1: ( rule__WMLTag__EndAssignment_4 )
            {
             before(grammarAccess.getWMLTagAccess().getEndAssignment_4()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1089:1: ( rule__WMLTag__EndAssignment_4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1089:2: rule__WMLTag__EndAssignment_4
            {
            pushFollow(FOLLOW_rule__WMLTag__EndAssignment_4_in_rule__WMLTag__Group__4__Impl2311);
            rule__WMLTag__EndAssignment_4();
            _fsp--;


            }

             after(grammarAccess.getWMLTagAccess().getEndAssignment_4()); 

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
    // $ANTLR end rule__WMLTag__Group__4__Impl


    // $ANTLR start rule__WMLEndTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1109:1: rule__WMLEndTag__Group__0 : rule__WMLEndTag__Group__0__Impl rule__WMLEndTag__Group__1 ;
    public final void rule__WMLEndTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1113:1: ( rule__WMLEndTag__Group__0__Impl rule__WMLEndTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1114:2: rule__WMLEndTag__Group__0__Impl rule__WMLEndTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLEndTag__Group__0__Impl_in_rule__WMLEndTag__Group__02351);
            rule__WMLEndTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLEndTag__Group__1_in_rule__WMLEndTag__Group__02354);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1121:1: rule__WMLEndTag__Group__0__Impl : ( '[/' ) ;
    public final void rule__WMLEndTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1125:1: ( ( '[/' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1126:1: ( '[/' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1126:1: ( '[/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1127:1: '[/'
            {
             before(grammarAccess.getWMLEndTagAccess().getLeftSquareBracketSolidusKeyword_0()); 
            match(input,29,FOLLOW_29_in_rule__WMLEndTag__Group__0__Impl2382); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1140:1: rule__WMLEndTag__Group__1 : rule__WMLEndTag__Group__1__Impl rule__WMLEndTag__Group__2 ;
    public final void rule__WMLEndTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1144:1: ( rule__WMLEndTag__Group__1__Impl rule__WMLEndTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1145:2: rule__WMLEndTag__Group__1__Impl rule__WMLEndTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLEndTag__Group__1__Impl_in_rule__WMLEndTag__Group__12413);
            rule__WMLEndTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLEndTag__Group__2_in_rule__WMLEndTag__Group__12416);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1152:1: rule__WMLEndTag__Group__1__Impl : ( ( rule__WMLEndTag__TagnameAssignment_1 ) ) ;
    public final void rule__WMLEndTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1156:1: ( ( ( rule__WMLEndTag__TagnameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1157:1: ( ( rule__WMLEndTag__TagnameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1157:1: ( ( rule__WMLEndTag__TagnameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1158:1: ( rule__WMLEndTag__TagnameAssignment_1 )
            {
             before(grammarAccess.getWMLEndTagAccess().getTagnameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1159:1: ( rule__WMLEndTag__TagnameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1159:2: rule__WMLEndTag__TagnameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLEndTag__TagnameAssignment_1_in_rule__WMLEndTag__Group__1__Impl2443);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1169:1: rule__WMLEndTag__Group__2 : rule__WMLEndTag__Group__2__Impl ;
    public final void rule__WMLEndTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1173:1: ( rule__WMLEndTag__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1174:2: rule__WMLEndTag__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLEndTag__Group__2__Impl_in_rule__WMLEndTag__Group__22473);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1180:1: rule__WMLEndTag__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLEndTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1184:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1185:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1185:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1186:1: ']'
            {
             before(grammarAccess.getWMLEndTagAccess().getRightSquareBracketKeyword_2()); 
            match(input,28,FOLLOW_28_in_rule__WMLEndTag__Group__2__Impl2501); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1205:1: rule__WMLKey__Group__0 : rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 ;
    public final void rule__WMLKey__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1209:1: ( rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1210:2: rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02538);
            rule__WMLKey__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02541);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1217:1: rule__WMLKey__Group__0__Impl : ( ( rule__WMLKey__KeyNameAssignment_0 ) ) ;
    public final void rule__WMLKey__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1221:1: ( ( ( rule__WMLKey__KeyNameAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1222:1: ( ( rule__WMLKey__KeyNameAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1222:1: ( ( rule__WMLKey__KeyNameAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1223:1: ( rule__WMLKey__KeyNameAssignment_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getKeyNameAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1224:1: ( rule__WMLKey__KeyNameAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1224:2: rule__WMLKey__KeyNameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLKey__KeyNameAssignment_0_in_rule__WMLKey__Group__0__Impl2568);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1234:1: rule__WMLKey__Group__1 : rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 ;
    public final void rule__WMLKey__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1238:1: ( rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1239:2: rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12598);
            rule__WMLKey__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12601);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1246:1: rule__WMLKey__Group__1__Impl : ( '=' ) ;
    public final void rule__WMLKey__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1250:1: ( ( '=' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1251:1: ( '=' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1251:1: ( '=' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1252:1: '='
            {
             before(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1()); 
            match(input,15,FOLLOW_15_in_rule__WMLKey__Group__1__Impl2629); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1265:1: rule__WMLKey__Group__2 : rule__WMLKey__Group__2__Impl ;
    public final void rule__WMLKey__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1269:1: ( rule__WMLKey__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1270:2: rule__WMLKey__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22660);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1276:1: rule__WMLKey__Group__2__Impl : ( ( rule__WMLKey__ValueAssignment_2 ) ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1280:1: ( ( ( rule__WMLKey__ValueAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1281:1: ( ( rule__WMLKey__ValueAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1281:1: ( ( rule__WMLKey__ValueAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1282:1: ( rule__WMLKey__ValueAssignment_2 )
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1283:1: ( rule__WMLKey__ValueAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1283:2: rule__WMLKey__ValueAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2687);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1299:1: rule__FLOAT__Group__0 : rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1 ;
    public final void rule__FLOAT__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1303:1: ( rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1304:2: rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__0__Impl_in_rule__FLOAT__Group__02723);
            rule__FLOAT__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__FLOAT__Group__1_in_rule__FLOAT__Group__02726);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1311:1: rule__FLOAT__Group__0__Impl : ( RULE_IINT ) ;
    public final void rule__FLOAT__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1315:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1316:1: ( RULE_IINT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1316:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1317:1: RULE_IINT
            {
             before(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_0()); 
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__FLOAT__Group__0__Impl2753); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1328:1: rule__FLOAT__Group__1 : rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2 ;
    public final void rule__FLOAT__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1332:1: ( rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1333:2: rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__1__Impl_in_rule__FLOAT__Group__12782);
            rule__FLOAT__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__FLOAT__Group__2_in_rule__FLOAT__Group__12785);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1340:1: rule__FLOAT__Group__1__Impl : ( '.' ) ;
    public final void rule__FLOAT__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1344:1: ( ( '.' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1345:1: ( '.' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1345:1: ( '.' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1346:1: '.'
            {
             before(grammarAccess.getFLOATAccess().getFullStopKeyword_1()); 
            match(input,12,FOLLOW_12_in_rule__FLOAT__Group__1__Impl2813); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1359:1: rule__FLOAT__Group__2 : rule__FLOAT__Group__2__Impl ;
    public final void rule__FLOAT__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1363:1: ( rule__FLOAT__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1364:2: rule__FLOAT__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__2__Impl_in_rule__FLOAT__Group__22844);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1370:1: rule__FLOAT__Group__2__Impl : ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) ) ;
    public final void rule__FLOAT__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1374:1: ( ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1375:1: ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1375:1: ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1376:1: ( ( RULE_IINT ) ) ( ( RULE_IINT )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1376:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1377:1: ( RULE_IINT )
            {
             before(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1378:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1378:3: RULE_IINT
            {
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2874); 

            }

             after(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1381:1: ( ( RULE_IINT )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1382:1: ( RULE_IINT )*
            {
             before(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1383:1: ( RULE_IINT )*
            loop16:
            do {
                int alt16=2;
                int LA16_0 = input.LA(1);

                if ( (LA16_0==RULE_IINT) ) {
                    alt16=1;
                }


                switch (alt16) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1383:3: RULE_IINT
            	    {
            	    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2887); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1400:1: rule__TSTRING__Group__0 : rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 ;
    public final void rule__TSTRING__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1404:1: ( rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1405:2: rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__02926);
            rule__TSTRING__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__02929);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1412:1: rule__TSTRING__Group__0__Impl : ( ( rule__TSTRING__Group_0__0 ) ) ;
    public final void rule__TSTRING__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1416:1: ( ( ( rule__TSTRING__Group_0__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1417:1: ( ( rule__TSTRING__Group_0__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1417:1: ( ( rule__TSTRING__Group_0__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1418:1: ( rule__TSTRING__Group_0__0 )
            {
             before(grammarAccess.getTSTRINGAccess().getGroup_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1419:1: ( rule__TSTRING__Group_0__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1419:2: rule__TSTRING__Group_0__0
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__0_in_rule__TSTRING__Group__0__Impl2956);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1429:1: rule__TSTRING__Group__1 : rule__TSTRING__Group__1__Impl ;
    public final void rule__TSTRING__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1433:1: ( rule__TSTRING__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1434:2: rule__TSTRING__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__12986);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1440:1: rule__TSTRING__Group__1__Impl : ( RULE_STRING ) ;
    public final void rule__TSTRING__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1444:1: ( ( RULE_STRING ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1445:1: ( RULE_STRING )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1445:1: ( RULE_STRING )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1446:1: RULE_STRING
            {
             before(grammarAccess.getTSTRINGAccess().getSTRINGTerminalRuleCall_1()); 
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl3013); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1461:1: rule__TSTRING__Group_0__0 : rule__TSTRING__Group_0__0__Impl rule__TSTRING__Group_0__1 ;
    public final void rule__TSTRING__Group_0__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1465:1: ( rule__TSTRING__Group_0__0__Impl rule__TSTRING__Group_0__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1466:2: rule__TSTRING__Group_0__0__Impl rule__TSTRING__Group_0__1
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__0__Impl_in_rule__TSTRING__Group_0__03046);
            rule__TSTRING__Group_0__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group_0__1_in_rule__TSTRING__Group_0__03049);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1473:1: rule__TSTRING__Group_0__0__Impl : ( ( ' ' )? ) ;
    public final void rule__TSTRING__Group_0__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1477:1: ( ( ( ' ' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1478:1: ( ( ' ' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1478:1: ( ( ' ' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1479:1: ( ' ' )?
            {
             before(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1480:1: ( ' ' )?
            int alt17=2;
            int LA17_0 = input.LA(1);

            if ( (LA17_0==30) ) {
                alt17=1;
            }
            switch (alt17) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1481:2: ' '
                    {
                    match(input,30,FOLLOW_30_in_rule__TSTRING__Group_0__0__Impl3078); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1492:1: rule__TSTRING__Group_0__1 : rule__TSTRING__Group_0__1__Impl rule__TSTRING__Group_0__2 ;
    public final void rule__TSTRING__Group_0__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1496:1: ( rule__TSTRING__Group_0__1__Impl rule__TSTRING__Group_0__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1497:2: rule__TSTRING__Group_0__1__Impl rule__TSTRING__Group_0__2
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__1__Impl_in_rule__TSTRING__Group_0__13111);
            rule__TSTRING__Group_0__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group_0__2_in_rule__TSTRING__Group_0__13114);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1504:1: rule__TSTRING__Group_0__1__Impl : ( '_' ) ;
    public final void rule__TSTRING__Group_0__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1508:1: ( ( '_' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1509:1: ( '_' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1509:1: ( '_' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1510:1: '_'
            {
             before(grammarAccess.getTSTRINGAccess().get_Keyword_0_1()); 
            match(input,9,FOLLOW_9_in_rule__TSTRING__Group_0__1__Impl3142); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1523:1: rule__TSTRING__Group_0__2 : rule__TSTRING__Group_0__2__Impl ;
    public final void rule__TSTRING__Group_0__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1527:1: ( rule__TSTRING__Group_0__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1528:2: rule__TSTRING__Group_0__2__Impl
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__2__Impl_in_rule__TSTRING__Group_0__23173);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1534:1: rule__TSTRING__Group_0__2__Impl : ( ( ' ' )? ) ;
    public final void rule__TSTRING__Group_0__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1538:1: ( ( ( ' ' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1539:1: ( ( ' ' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1539:1: ( ( ' ' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1540:1: ( ' ' )?
            {
             before(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1541:1: ( ' ' )?
            int alt18=2;
            int LA18_0 = input.LA(1);

            if ( (LA18_0==30) ) {
                alt18=1;
            }
            switch (alt18) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1542:2: ' '
                    {
                    match(input,30,FOLLOW_30_in_rule__TSTRING__Group_0__2__Impl3202); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1559:1: rule__PATH__Group__0 : rule__PATH__Group__0__Impl rule__PATH__Group__1 ;
    public final void rule__PATH__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1563:1: ( rule__PATH__Group__0__Impl rule__PATH__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1564:2: rule__PATH__Group__0__Impl rule__PATH__Group__1
            {
            pushFollow(FOLLOW_rule__PATH__Group__0__Impl_in_rule__PATH__Group__03241);
            rule__PATH__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group__1_in_rule__PATH__Group__03244);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1571:1: rule__PATH__Group__0__Impl : ( ( rule__PATH__Group_0__0 )* ) ;
    public final void rule__PATH__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1575:1: ( ( ( rule__PATH__Group_0__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1576:1: ( ( rule__PATH__Group_0__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1576:1: ( ( rule__PATH__Group_0__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1577:1: ( rule__PATH__Group_0__0 )*
            {
             before(grammarAccess.getPATHAccess().getGroup_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1578:1: ( rule__PATH__Group_0__0 )*
            loop19:
            do {
                int alt19=2;
                alt19 = dfa19.predict(input);
                switch (alt19) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1578:2: rule__PATH__Group_0__0
            	    {
            	    pushFollow(FOLLOW_rule__PATH__Group_0__0_in_rule__PATH__Group__0__Impl3271);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1588:1: rule__PATH__Group__1 : rule__PATH__Group__1__Impl rule__PATH__Group__2 ;
    public final void rule__PATH__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1592:1: ( rule__PATH__Group__1__Impl rule__PATH__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1593:2: rule__PATH__Group__1__Impl rule__PATH__Group__2
            {
            pushFollow(FOLLOW_rule__PATH__Group__1__Impl_in_rule__PATH__Group__13302);
            rule__PATH__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group__2_in_rule__PATH__Group__13305);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1600:1: rule__PATH__Group__1__Impl : ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) ;
    public final void rule__PATH__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1604:1: ( ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1605:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1605:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1606:1: ( ( RULE_ID ) ) ( ( RULE_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1606:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1607:1: ( RULE_ID )
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1608:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1608:3: RULE_ID
            {
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl3335); 

            }

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1611:1: ( ( RULE_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1612:1: ( RULE_ID )*
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1613:1: ( RULE_ID )*
            loop20:
            do {
                int alt20=2;
                int LA20_0 = input.LA(1);

                if ( (LA20_0==RULE_ID) ) {
                    alt20=1;
                }


                switch (alt20) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1613:3: RULE_ID
            	    {
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl3348); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1624:1: rule__PATH__Group__2 : rule__PATH__Group__2__Impl rule__PATH__Group__3 ;
    public final void rule__PATH__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1628:1: ( rule__PATH__Group__2__Impl rule__PATH__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1629:2: rule__PATH__Group__2__Impl rule__PATH__Group__3
            {
            pushFollow(FOLLOW_rule__PATH__Group__2__Impl_in_rule__PATH__Group__23381);
            rule__PATH__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group__3_in_rule__PATH__Group__23384);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1636:1: rule__PATH__Group__2__Impl : ( '.' ) ;
    public final void rule__PATH__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1640:1: ( ( '.' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1641:1: ( '.' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1641:1: ( '.' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1642:1: '.'
            {
             before(grammarAccess.getPATHAccess().getFullStopKeyword_2()); 
            match(input,12,FOLLOW_12_in_rule__PATH__Group__2__Impl3412); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1655:1: rule__PATH__Group__3 : rule__PATH__Group__3__Impl ;
    public final void rule__PATH__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1659:1: ( rule__PATH__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1660:2: rule__PATH__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__PATH__Group__3__Impl_in_rule__PATH__Group__33443);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1666:1: rule__PATH__Group__3__Impl : ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) ;
    public final void rule__PATH__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1670:1: ( ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1671:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1671:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1672:1: ( ( RULE_ID ) ) ( ( RULE_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1672:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1673:1: ( RULE_ID )
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1674:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1674:3: RULE_ID
            {
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl3473); 

            }

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1677:1: ( ( RULE_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1678:1: ( RULE_ID )*
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1679:1: ( RULE_ID )*
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
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1679:3: RULE_ID
            	    {
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl3486); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1698:1: rule__PATH__Group_0__0 : rule__PATH__Group_0__0__Impl rule__PATH__Group_0__1 ;
    public final void rule__PATH__Group_0__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1702:1: ( rule__PATH__Group_0__0__Impl rule__PATH__Group_0__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1703:2: rule__PATH__Group_0__0__Impl rule__PATH__Group_0__1
            {
            pushFollow(FOLLOW_rule__PATH__Group_0__0__Impl_in_rule__PATH__Group_0__03527);
            rule__PATH__Group_0__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group_0__1_in_rule__PATH__Group_0__03530);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1710:1: rule__PATH__Group_0__0__Impl : ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) ;
    public final void rule__PATH__Group_0__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1714:1: ( ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1715:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1715:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1716:1: ( ( RULE_ID ) ) ( ( RULE_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1716:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1717:1: ( RULE_ID )
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1718:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1718:3: RULE_ID
            {
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl3560); 

            }

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1721:1: ( ( RULE_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1722:1: ( RULE_ID )*
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1723:1: ( RULE_ID )*
            loop22:
            do {
                int alt22=2;
                int LA22_0 = input.LA(1);

                if ( (LA22_0==RULE_ID) ) {
                    alt22=1;
                }


                switch (alt22) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1723:3: RULE_ID
            	    {
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl3573); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1734:1: rule__PATH__Group_0__1 : rule__PATH__Group_0__1__Impl ;
    public final void rule__PATH__Group_0__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1738:1: ( rule__PATH__Group_0__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1739:2: rule__PATH__Group_0__1__Impl
            {
            pushFollow(FOLLOW_rule__PATH__Group_0__1__Impl_in_rule__PATH__Group_0__13606);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1745:1: rule__PATH__Group_0__1__Impl : ( ( rule__PATH__Alternatives_0_1 ) ) ;
    public final void rule__PATH__Group_0__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1749:1: ( ( ( rule__PATH__Alternatives_0_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1750:1: ( ( rule__PATH__Alternatives_0_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1750:1: ( ( rule__PATH__Alternatives_0_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1751:1: ( rule__PATH__Alternatives_0_1 )
            {
             before(grammarAccess.getPATHAccess().getAlternatives_0_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1752:1: ( rule__PATH__Alternatives_0_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1752:2: rule__PATH__Alternatives_0_1
            {
            pushFollow(FOLLOW_rule__PATH__Alternatives_0_1_in_rule__PATH__Group_0__1__Impl3633);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1766:1: rule__DIRECTION__Group__0 : rule__DIRECTION__Group__0__Impl rule__DIRECTION__Group__1 ;
    public final void rule__DIRECTION__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1770:1: ( rule__DIRECTION__Group__0__Impl rule__DIRECTION__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1771:2: rule__DIRECTION__Group__0__Impl rule__DIRECTION__Group__1
            {
            pushFollow(FOLLOW_rule__DIRECTION__Group__0__Impl_in_rule__DIRECTION__Group__03667);
            rule__DIRECTION__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__DIRECTION__Group__1_in_rule__DIRECTION__Group__03670);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1778:1: rule__DIRECTION__Group__0__Impl : ( ( rule__DIRECTION__Alternatives_0 ) ) ;
    public final void rule__DIRECTION__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1782:1: ( ( ( rule__DIRECTION__Alternatives_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1783:1: ( ( rule__DIRECTION__Alternatives_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1783:1: ( ( rule__DIRECTION__Alternatives_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1784:1: ( rule__DIRECTION__Alternatives_0 )
            {
             before(grammarAccess.getDIRECTIONAccess().getAlternatives_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1785:1: ( rule__DIRECTION__Alternatives_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1785:2: rule__DIRECTION__Alternatives_0
            {
            pushFollow(FOLLOW_rule__DIRECTION__Alternatives_0_in_rule__DIRECTION__Group__0__Impl3697);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1795:1: rule__DIRECTION__Group__1 : rule__DIRECTION__Group__1__Impl ;
    public final void rule__DIRECTION__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1799:1: ( rule__DIRECTION__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1800:2: rule__DIRECTION__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__DIRECTION__Group__1__Impl_in_rule__DIRECTION__Group__13727);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1806:1: rule__DIRECTION__Group__1__Impl : ( ( ',' )? ) ;
    public final void rule__DIRECTION__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1810:1: ( ( ( ',' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1811:1: ( ( ',' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1811:1: ( ( ',' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1812:1: ( ',' )?
            {
             before(grammarAccess.getDIRECTIONAccess().getCommaKeyword_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1813:1: ( ',' )?
            int alt23=2;
            int LA23_0 = input.LA(1);

            if ( (LA23_0==31) ) {
                alt23=1;
            }
            switch (alt23) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1814:2: ','
                    {
                    match(input,31,FOLLOW_31_in_rule__DIRECTION__Group__1__Impl3756); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1829:1: rule__LIST__Group__0 : rule__LIST__Group__0__Impl rule__LIST__Group__1 ;
    public final void rule__LIST__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1833:1: ( rule__LIST__Group__0__Impl rule__LIST__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1834:2: rule__LIST__Group__0__Impl rule__LIST__Group__1
            {
            pushFollow(FOLLOW_rule__LIST__Group__0__Impl_in_rule__LIST__Group__03793);
            rule__LIST__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__LIST__Group__1_in_rule__LIST__Group__03796);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1841:1: rule__LIST__Group__0__Impl : ( RULE_ID ) ;
    public final void rule__LIST__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1845:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1846:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1846:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1847:1: RULE_ID
            {
             before(grammarAccess.getLISTAccess().getIDTerminalRuleCall_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__LIST__Group__0__Impl3823); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1858:1: rule__LIST__Group__1 : rule__LIST__Group__1__Impl ;
    public final void rule__LIST__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1862:1: ( rule__LIST__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1863:2: rule__LIST__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__LIST__Group__1__Impl_in_rule__LIST__Group__13852);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1869:1: rule__LIST__Group__1__Impl : ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) ) ;
    public final void rule__LIST__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1873:1: ( ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1874:1: ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1874:1: ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1875:1: ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1875:1: ( ( rule__LIST__Group_1__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1876:1: ( rule__LIST__Group_1__0 )
            {
             before(grammarAccess.getLISTAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1877:1: ( rule__LIST__Group_1__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1877:2: rule__LIST__Group_1__0
            {
            pushFollow(FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl3881);
            rule__LIST__Group_1__0();
            _fsp--;


            }

             after(grammarAccess.getLISTAccess().getGroup_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1880:1: ( ( rule__LIST__Group_1__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1881:1: ( rule__LIST__Group_1__0 )*
            {
             before(grammarAccess.getLISTAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1882:1: ( rule__LIST__Group_1__0 )*
            loop24:
            do {
                int alt24=2;
                int LA24_0 = input.LA(1);

                if ( (LA24_0==31) ) {
                    alt24=1;
                }


                switch (alt24) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1882:2: rule__LIST__Group_1__0
            	    {
            	    pushFollow(FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl3893);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1897:1: rule__LIST__Group_1__0 : rule__LIST__Group_1__0__Impl rule__LIST__Group_1__1 ;
    public final void rule__LIST__Group_1__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1901:1: ( rule__LIST__Group_1__0__Impl rule__LIST__Group_1__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1902:2: rule__LIST__Group_1__0__Impl rule__LIST__Group_1__1
            {
            pushFollow(FOLLOW_rule__LIST__Group_1__0__Impl_in_rule__LIST__Group_1__03930);
            rule__LIST__Group_1__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__LIST__Group_1__1_in_rule__LIST__Group_1__03933);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1909:1: rule__LIST__Group_1__0__Impl : ( ',' ) ;
    public final void rule__LIST__Group_1__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1913:1: ( ( ',' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1914:1: ( ',' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1914:1: ( ',' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1915:1: ','
            {
             before(grammarAccess.getLISTAccess().getCommaKeyword_1_0()); 
            match(input,31,FOLLOW_31_in_rule__LIST__Group_1__0__Impl3961); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1928:1: rule__LIST__Group_1__1 : rule__LIST__Group_1__1__Impl ;
    public final void rule__LIST__Group_1__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1932:1: ( rule__LIST__Group_1__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1933:2: rule__LIST__Group_1__1__Impl
            {
            pushFollow(FOLLOW_rule__LIST__Group_1__1__Impl_in_rule__LIST__Group_1__13992);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1939:1: rule__LIST__Group_1__1__Impl : ( RULE_ID ) ;
    public final void rule__LIST__Group_1__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1943:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1944:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1944:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1945:1: RULE_ID
            {
             before(grammarAccess.getLISTAccess().getIDTerminalRuleCall_1_1()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__LIST__Group_1__1__Impl4019); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1960:1: rule__PROGRESSIVE__Group__0 : rule__PROGRESSIVE__Group__0__Impl rule__PROGRESSIVE__Group__1 ;
    public final void rule__PROGRESSIVE__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1964:1: ( rule__PROGRESSIVE__Group__0__Impl rule__PROGRESSIVE__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1965:2: rule__PROGRESSIVE__Group__0__Impl rule__PROGRESSIVE__Group__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__0__Impl_in_rule__PROGRESSIVE__Group__04052);
            rule__PROGRESSIVE__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__1_in_rule__PROGRESSIVE__Group__04055);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1972:1: rule__PROGRESSIVE__Group__0__Impl : ( ( rule__PROGRESSIVE__Alternatives_0 ) ) ;
    public final void rule__PROGRESSIVE__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1976:1: ( ( ( rule__PROGRESSIVE__Alternatives_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1977:1: ( ( rule__PROGRESSIVE__Alternatives_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1977:1: ( ( rule__PROGRESSIVE__Alternatives_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1978:1: ( rule__PROGRESSIVE__Alternatives_0 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1979:1: ( rule__PROGRESSIVE__Alternatives_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1979:2: rule__PROGRESSIVE__Alternatives_0
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_0_in_rule__PROGRESSIVE__Group__0__Impl4082);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1989:1: rule__PROGRESSIVE__Group__1 : rule__PROGRESSIVE__Group__1__Impl rule__PROGRESSIVE__Group__2 ;
    public final void rule__PROGRESSIVE__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1993:1: ( rule__PROGRESSIVE__Group__1__Impl rule__PROGRESSIVE__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1994:2: rule__PROGRESSIVE__Group__1__Impl rule__PROGRESSIVE__Group__2
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__1__Impl_in_rule__PROGRESSIVE__Group__14112);
            rule__PROGRESSIVE__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__2_in_rule__PROGRESSIVE__Group__14115);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2001:1: rule__PROGRESSIVE__Group__1__Impl : ( ( rule__PROGRESSIVE__Group_1__0 )? ) ;
    public final void rule__PROGRESSIVE__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2005:1: ( ( ( rule__PROGRESSIVE__Group_1__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2006:1: ( ( rule__PROGRESSIVE__Group_1__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2006:1: ( ( rule__PROGRESSIVE__Group_1__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2007:1: ( rule__PROGRESSIVE__Group_1__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2008:1: ( rule__PROGRESSIVE__Group_1__0 )?
            int alt25=2;
            int LA25_0 = input.LA(1);

            if ( (LA25_0==32) ) {
                alt25=1;
            }
            switch (alt25) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2008:2: rule__PROGRESSIVE__Group_1__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__0_in_rule__PROGRESSIVE__Group__1__Impl4142);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2018:1: rule__PROGRESSIVE__Group__2 : rule__PROGRESSIVE__Group__2__Impl rule__PROGRESSIVE__Group__3 ;
    public final void rule__PROGRESSIVE__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2022:1: ( rule__PROGRESSIVE__Group__2__Impl rule__PROGRESSIVE__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2023:2: rule__PROGRESSIVE__Group__2__Impl rule__PROGRESSIVE__Group__3
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__2__Impl_in_rule__PROGRESSIVE__Group__24173);
            rule__PROGRESSIVE__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__3_in_rule__PROGRESSIVE__Group__24176);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2030:1: rule__PROGRESSIVE__Group__2__Impl : ( ( rule__PROGRESSIVE__Group_2__0 )? ) ;
    public final void rule__PROGRESSIVE__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2034:1: ( ( ( rule__PROGRESSIVE__Group_2__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2035:1: ( ( rule__PROGRESSIVE__Group_2__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2035:1: ( ( rule__PROGRESSIVE__Group_2__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2036:1: ( rule__PROGRESSIVE__Group_2__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2037:1: ( rule__PROGRESSIVE__Group_2__0 )?
            int alt26=2;
            int LA26_0 = input.LA(1);

            if ( (LA26_0==10) ) {
                alt26=1;
            }
            switch (alt26) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2037:2: rule__PROGRESSIVE__Group_2__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__0_in_rule__PROGRESSIVE__Group__2__Impl4203);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2047:1: rule__PROGRESSIVE__Group__3 : rule__PROGRESSIVE__Group__3__Impl ;
    public final void rule__PROGRESSIVE__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2051:1: ( rule__PROGRESSIVE__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2052:2: rule__PROGRESSIVE__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__3__Impl_in_rule__PROGRESSIVE__Group__34234);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2058:1: rule__PROGRESSIVE__Group__3__Impl : ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) ) ;
    public final void rule__PROGRESSIVE__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2062:1: ( ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2063:1: ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2063:1: ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2064:1: ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2064:1: ( ( rule__PROGRESSIVE__Group_3__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2065:1: ( rule__PROGRESSIVE__Group_3__0 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2066:1: ( rule__PROGRESSIVE__Group_3__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2066:2: rule__PROGRESSIVE__Group_3__0
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl4263);
            rule__PROGRESSIVE__Group_3__0();
            _fsp--;


            }

             after(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2069:1: ( ( rule__PROGRESSIVE__Group_3__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2070:1: ( rule__PROGRESSIVE__Group_3__0 )*
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2071:1: ( rule__PROGRESSIVE__Group_3__0 )*
            loop27:
            do {
                int alt27=2;
                int LA27_0 = input.LA(1);

                if ( (LA27_0==31) ) {
                    alt27=1;
                }


                switch (alt27) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2071:2: rule__PROGRESSIVE__Group_3__0
            	    {
            	    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl4275);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2090:1: rule__PROGRESSIVE__Group_1__0 : rule__PROGRESSIVE__Group_1__0__Impl rule__PROGRESSIVE__Group_1__1 ;
    public final void rule__PROGRESSIVE__Group_1__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2094:1: ( rule__PROGRESSIVE__Group_1__0__Impl rule__PROGRESSIVE__Group_1__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2095:2: rule__PROGRESSIVE__Group_1__0__Impl rule__PROGRESSIVE__Group_1__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__0__Impl_in_rule__PROGRESSIVE__Group_1__04316);
            rule__PROGRESSIVE__Group_1__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__1_in_rule__PROGRESSIVE__Group_1__04319);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2102:1: rule__PROGRESSIVE__Group_1__0__Impl : ( '~' ) ;
    public final void rule__PROGRESSIVE__Group_1__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2106:1: ( ( '~' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2107:1: ( '~' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2107:1: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2108:1: '~'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_1_0()); 
            match(input,32,FOLLOW_32_in_rule__PROGRESSIVE__Group_1__0__Impl4347); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2121:1: rule__PROGRESSIVE__Group_1__1 : rule__PROGRESSIVE__Group_1__1__Impl ;
    public final void rule__PROGRESSIVE__Group_1__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2125:1: ( rule__PROGRESSIVE__Group_1__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2126:2: rule__PROGRESSIVE__Group_1__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__1__Impl_in_rule__PROGRESSIVE__Group_1__14378);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2132:1: rule__PROGRESSIVE__Group_1__1__Impl : ( ( rule__PROGRESSIVE__Alternatives_1_1 ) ) ;
    public final void rule__PROGRESSIVE__Group_1__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2136:1: ( ( ( rule__PROGRESSIVE__Alternatives_1_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2137:1: ( ( rule__PROGRESSIVE__Alternatives_1_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2137:1: ( ( rule__PROGRESSIVE__Alternatives_1_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2138:1: ( rule__PROGRESSIVE__Alternatives_1_1 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_1_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2139:1: ( rule__PROGRESSIVE__Alternatives_1_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2139:2: rule__PROGRESSIVE__Alternatives_1_1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_1_1_in_rule__PROGRESSIVE__Group_1__1__Impl4405);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2153:1: rule__PROGRESSIVE__Group_2__0 : rule__PROGRESSIVE__Group_2__0__Impl rule__PROGRESSIVE__Group_2__1 ;
    public final void rule__PROGRESSIVE__Group_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2157:1: ( rule__PROGRESSIVE__Group_2__0__Impl rule__PROGRESSIVE__Group_2__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2158:2: rule__PROGRESSIVE__Group_2__0__Impl rule__PROGRESSIVE__Group_2__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__0__Impl_in_rule__PROGRESSIVE__Group_2__04439);
            rule__PROGRESSIVE__Group_2__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__1_in_rule__PROGRESSIVE__Group_2__04442);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2165:1: rule__PROGRESSIVE__Group_2__0__Impl : ( ':' ) ;
    public final void rule__PROGRESSIVE__Group_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2169:1: ( ( ':' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2170:1: ( ':' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2170:1: ( ':' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2171:1: ':'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_2_0()); 
            match(input,10,FOLLOW_10_in_rule__PROGRESSIVE__Group_2__0__Impl4470); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2184:1: rule__PROGRESSIVE__Group_2__1 : rule__PROGRESSIVE__Group_2__1__Impl ;
    public final void rule__PROGRESSIVE__Group_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2188:1: ( rule__PROGRESSIVE__Group_2__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2189:2: rule__PROGRESSIVE__Group_2__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__1__Impl_in_rule__PROGRESSIVE__Group_2__14501);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2195:1: rule__PROGRESSIVE__Group_2__1__Impl : ( RULE_IINT ) ;
    public final void rule__PROGRESSIVE__Group_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2199:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2200:1: ( RULE_IINT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2200:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2201:1: RULE_IINT
            {
             before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_2_1()); 
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_2__1__Impl4528); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2216:1: rule__PROGRESSIVE__Group_3__0 : rule__PROGRESSIVE__Group_3__0__Impl rule__PROGRESSIVE__Group_3__1 ;
    public final void rule__PROGRESSIVE__Group_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2220:1: ( rule__PROGRESSIVE__Group_3__0__Impl rule__PROGRESSIVE__Group_3__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2221:2: rule__PROGRESSIVE__Group_3__0__Impl rule__PROGRESSIVE__Group_3__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__0__Impl_in_rule__PROGRESSIVE__Group_3__04561);
            rule__PROGRESSIVE__Group_3__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__1_in_rule__PROGRESSIVE__Group_3__04564);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2228:1: rule__PROGRESSIVE__Group_3__0__Impl : ( ',' ) ;
    public final void rule__PROGRESSIVE__Group_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2232:1: ( ( ',' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2233:1: ( ',' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2233:1: ( ',' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2234:1: ','
            {
             before(grammarAccess.getPROGRESSIVEAccess().getCommaKeyword_3_0()); 
            match(input,31,FOLLOW_31_in_rule__PROGRESSIVE__Group_3__0__Impl4592); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2247:1: rule__PROGRESSIVE__Group_3__1 : rule__PROGRESSIVE__Group_3__1__Impl rule__PROGRESSIVE__Group_3__2 ;
    public final void rule__PROGRESSIVE__Group_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2251:1: ( rule__PROGRESSIVE__Group_3__1__Impl rule__PROGRESSIVE__Group_3__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2252:2: rule__PROGRESSIVE__Group_3__1__Impl rule__PROGRESSIVE__Group_3__2
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__1__Impl_in_rule__PROGRESSIVE__Group_3__14623);
            rule__PROGRESSIVE__Group_3__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__2_in_rule__PROGRESSIVE__Group_3__14626);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2259:1: rule__PROGRESSIVE__Group_3__1__Impl : ( ( rule__PROGRESSIVE__Alternatives_3_1 ) ) ;
    public final void rule__PROGRESSIVE__Group_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2263:1: ( ( ( rule__PROGRESSIVE__Alternatives_3_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2264:1: ( ( rule__PROGRESSIVE__Alternatives_3_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2264:1: ( ( rule__PROGRESSIVE__Alternatives_3_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2265:1: ( rule__PROGRESSIVE__Alternatives_3_1 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_3_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2266:1: ( rule__PROGRESSIVE__Alternatives_3_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2266:2: rule__PROGRESSIVE__Alternatives_3_1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_3_1_in_rule__PROGRESSIVE__Group_3__1__Impl4653);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2276:1: rule__PROGRESSIVE__Group_3__2 : rule__PROGRESSIVE__Group_3__2__Impl rule__PROGRESSIVE__Group_3__3 ;
    public final void rule__PROGRESSIVE__Group_3__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2280:1: ( rule__PROGRESSIVE__Group_3__2__Impl rule__PROGRESSIVE__Group_3__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2281:2: rule__PROGRESSIVE__Group_3__2__Impl rule__PROGRESSIVE__Group_3__3
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__2__Impl_in_rule__PROGRESSIVE__Group_3__24683);
            rule__PROGRESSIVE__Group_3__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__3_in_rule__PROGRESSIVE__Group_3__24686);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2288:1: rule__PROGRESSIVE__Group_3__2__Impl : ( ( rule__PROGRESSIVE__Group_3_2__0 )? ) ;
    public final void rule__PROGRESSIVE__Group_3__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2292:1: ( ( ( rule__PROGRESSIVE__Group_3_2__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2293:1: ( ( rule__PROGRESSIVE__Group_3_2__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2293:1: ( ( rule__PROGRESSIVE__Group_3_2__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2294:1: ( rule__PROGRESSIVE__Group_3_2__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2295:1: ( rule__PROGRESSIVE__Group_3_2__0 )?
            int alt28=2;
            int LA28_0 = input.LA(1);

            if ( (LA28_0==32) ) {
                alt28=1;
            }
            switch (alt28) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2295:2: rule__PROGRESSIVE__Group_3_2__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__0_in_rule__PROGRESSIVE__Group_3__2__Impl4713);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2305:1: rule__PROGRESSIVE__Group_3__3 : rule__PROGRESSIVE__Group_3__3__Impl ;
    public final void rule__PROGRESSIVE__Group_3__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2309:1: ( rule__PROGRESSIVE__Group_3__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2310:2: rule__PROGRESSIVE__Group_3__3__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__3__Impl_in_rule__PROGRESSIVE__Group_3__34744);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2316:1: rule__PROGRESSIVE__Group_3__3__Impl : ( ( rule__PROGRESSIVE__Group_3_3__0 )? ) ;
    public final void rule__PROGRESSIVE__Group_3__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2320:1: ( ( ( rule__PROGRESSIVE__Group_3_3__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2321:1: ( ( rule__PROGRESSIVE__Group_3_3__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2321:1: ( ( rule__PROGRESSIVE__Group_3_3__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2322:1: ( rule__PROGRESSIVE__Group_3_3__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2323:1: ( rule__PROGRESSIVE__Group_3_3__0 )?
            int alt29=2;
            int LA29_0 = input.LA(1);

            if ( (LA29_0==10) ) {
                alt29=1;
            }
            switch (alt29) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2323:2: rule__PROGRESSIVE__Group_3_3__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__0_in_rule__PROGRESSIVE__Group_3__3__Impl4771);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2341:1: rule__PROGRESSIVE__Group_3_2__0 : rule__PROGRESSIVE__Group_3_2__0__Impl rule__PROGRESSIVE__Group_3_2__1 ;
    public final void rule__PROGRESSIVE__Group_3_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2345:1: ( rule__PROGRESSIVE__Group_3_2__0__Impl rule__PROGRESSIVE__Group_3_2__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2346:2: rule__PROGRESSIVE__Group_3_2__0__Impl rule__PROGRESSIVE__Group_3_2__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__0__Impl_in_rule__PROGRESSIVE__Group_3_2__04810);
            rule__PROGRESSIVE__Group_3_2__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__1_in_rule__PROGRESSIVE__Group_3_2__04813);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2353:1: rule__PROGRESSIVE__Group_3_2__0__Impl : ( '~' ) ;
    public final void rule__PROGRESSIVE__Group_3_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2357:1: ( ( '~' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2358:1: ( '~' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2358:1: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2359:1: '~'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_3_2_0()); 
            match(input,32,FOLLOW_32_in_rule__PROGRESSIVE__Group_3_2__0__Impl4841); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2372:1: rule__PROGRESSIVE__Group_3_2__1 : rule__PROGRESSIVE__Group_3_2__1__Impl ;
    public final void rule__PROGRESSIVE__Group_3_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2376:1: ( rule__PROGRESSIVE__Group_3_2__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2377:2: rule__PROGRESSIVE__Group_3_2__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__1__Impl_in_rule__PROGRESSIVE__Group_3_2__14872);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2383:1: rule__PROGRESSIVE__Group_3_2__1__Impl : ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) ) ;
    public final void rule__PROGRESSIVE__Group_3_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2387:1: ( ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2388:1: ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2388:1: ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2389:1: ( rule__PROGRESSIVE__Alternatives_3_2_1 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_3_2_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2390:1: ( rule__PROGRESSIVE__Alternatives_3_2_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2390:2: rule__PROGRESSIVE__Alternatives_3_2_1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_3_2_1_in_rule__PROGRESSIVE__Group_3_2__1__Impl4899);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2404:1: rule__PROGRESSIVE__Group_3_3__0 : rule__PROGRESSIVE__Group_3_3__0__Impl rule__PROGRESSIVE__Group_3_3__1 ;
    public final void rule__PROGRESSIVE__Group_3_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2408:1: ( rule__PROGRESSIVE__Group_3_3__0__Impl rule__PROGRESSIVE__Group_3_3__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2409:2: rule__PROGRESSIVE__Group_3_3__0__Impl rule__PROGRESSIVE__Group_3_3__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__0__Impl_in_rule__PROGRESSIVE__Group_3_3__04933);
            rule__PROGRESSIVE__Group_3_3__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__1_in_rule__PROGRESSIVE__Group_3_3__04936);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2416:1: rule__PROGRESSIVE__Group_3_3__0__Impl : ( ':' ) ;
    public final void rule__PROGRESSIVE__Group_3_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2420:1: ( ( ':' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2421:1: ( ':' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2421:1: ( ':' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2422:1: ':'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_3_3_0()); 
            match(input,10,FOLLOW_10_in_rule__PROGRESSIVE__Group_3_3__0__Impl4964); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2435:1: rule__PROGRESSIVE__Group_3_3__1 : rule__PROGRESSIVE__Group_3_3__1__Impl ;
    public final void rule__PROGRESSIVE__Group_3_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2439:1: ( rule__PROGRESSIVE__Group_3_3__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2440:2: rule__PROGRESSIVE__Group_3_3__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__1__Impl_in_rule__PROGRESSIVE__Group_3_3__14995);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2446:1: rule__PROGRESSIVE__Group_3_3__1__Impl : ( RULE_IINT ) ;
    public final void rule__PROGRESSIVE__Group_3_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2450:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2451:1: ( RULE_IINT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2451:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2452:1: RULE_IINT
            {
             before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_3_1()); 
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_3_3__1__Impl5022); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2468:1: rule__WMLRoot__RtagsAssignment_0 : ( ruleWMLTag ) ;
    public final void rule__WMLRoot__RtagsAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2472:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2473:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2473:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2474:1: ruleWMLTag
            {
             before(grammarAccess.getWMLRootAccess().getRtagsWMLTagParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRoot__RtagsAssignment_05060);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2483:1: rule__WMLRoot__RmacrosAssignment_1 : ( ruleWMLMacro ) ;
    public final void rule__WMLRoot__RmacrosAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2487:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2488:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2488:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2489:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLRootAccess().getRmacrosWMLMacroParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLRoot__RmacrosAssignment_15091);
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


    // $ANTLR start rule__WMLMacro__NameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2498:1: rule__WMLMacro__NameAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLMacro__NameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2502:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2503:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2503:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2504:1: RULE_ID
            {
             before(grammarAccess.getWMLMacroAccess().getNameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacro__NameAssignment_15122); 
             after(grammarAccess.getWMLMacroAccess().getNameIDTerminalRuleCall_1_0()); 

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
    // $ANTLR end rule__WMLMacro__NameAssignment_1


    // $ANTLR start rule__WMLMacro__ValueAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2513:1: rule__WMLMacro__ValueAssignment_2 : ( ( rule__WMLMacro__ValueAlternatives_2_0 ) ) ;
    public final void rule__WMLMacro__ValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2517:1: ( ( ( rule__WMLMacro__ValueAlternatives_2_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2518:1: ( ( rule__WMLMacro__ValueAlternatives_2_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2518:1: ( ( rule__WMLMacro__ValueAlternatives_2_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2519:1: ( rule__WMLMacro__ValueAlternatives_2_0 )
            {
             before(grammarAccess.getWMLMacroAccess().getValueAlternatives_2_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2520:1: ( rule__WMLMacro__ValueAlternatives_2_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2520:2: rule__WMLMacro__ValueAlternatives_2_0
            {
            pushFollow(FOLLOW_rule__WMLMacro__ValueAlternatives_2_0_in_rule__WMLMacro__ValueAssignment_25153);
            rule__WMLMacro__ValueAlternatives_2_0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroAccess().getValueAlternatives_2_0()); 

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
    // $ANTLR end rule__WMLMacro__ValueAssignment_2


    // $ANTLR start rule__WMLTag__NameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2529:1: rule__WMLTag__NameAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLTag__NameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2533:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2534:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2534:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2535:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_15186); 
             after(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_1_0()); 

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
    // $ANTLR end rule__WMLTag__NameAssignment_1


    // $ANTLR start rule__WMLTag__TtagsAssignment_3_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2544:1: rule__WMLTag__TtagsAssignment_3_0 : ( ruleWMLTag ) ;
    public final void rule__WMLTag__TtagsAssignment_3_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2548:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2549:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2549:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2550:1: ruleWMLTag
            {
             before(grammarAccess.getWMLTagAccess().getTtagsWMLTagParserRuleCall_3_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLTag__TtagsAssignment_3_05217);
            ruleWMLTag();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getTtagsWMLTagParserRuleCall_3_0_0()); 

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
    // $ANTLR end rule__WMLTag__TtagsAssignment_3_0


    // $ANTLR start rule__WMLTag__TkeysAssignment_3_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2559:1: rule__WMLTag__TkeysAssignment_3_1 : ( ruleWMLKey ) ;
    public final void rule__WMLTag__TkeysAssignment_3_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2563:1: ( ( ruleWMLKey ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2564:1: ( ruleWMLKey )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2564:1: ( ruleWMLKey )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2565:1: ruleWMLKey
            {
             before(grammarAccess.getWMLTagAccess().getTkeysWMLKeyParserRuleCall_3_1_0()); 
            pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLTag__TkeysAssignment_3_15248);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getTkeysWMLKeyParserRuleCall_3_1_0()); 

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
    // $ANTLR end rule__WMLTag__TkeysAssignment_3_1


    // $ANTLR start rule__WMLTag__TmacrosAssignment_3_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2574:1: rule__WMLTag__TmacrosAssignment_3_2 : ( ruleWMLMacro ) ;
    public final void rule__WMLTag__TmacrosAssignment_3_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2578:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2579:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2579:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2580:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLTagAccess().getTmacrosWMLMacroParserRuleCall_3_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLTag__TmacrosAssignment_3_25279);
            ruleWMLMacro();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getTmacrosWMLMacroParserRuleCall_3_2_0()); 

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
    // $ANTLR end rule__WMLTag__TmacrosAssignment_3_2


    // $ANTLR start rule__WMLTag__EndAssignment_4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2589:1: rule__WMLTag__EndAssignment_4 : ( ruleWMLEndTag ) ;
    public final void rule__WMLTag__EndAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2593:1: ( ( ruleWMLEndTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2594:1: ( ruleWMLEndTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2594:1: ( ruleWMLEndTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2595:1: ruleWMLEndTag
            {
             before(grammarAccess.getWMLTagAccess().getEndWMLEndTagParserRuleCall_4_0()); 
            pushFollow(FOLLOW_ruleWMLEndTag_in_rule__WMLTag__EndAssignment_45310);
            ruleWMLEndTag();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getEndWMLEndTagParserRuleCall_4_0()); 

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
    // $ANTLR end rule__WMLTag__EndAssignment_4


    // $ANTLR start rule__WMLEndTag__TagnameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2604:1: rule__WMLEndTag__TagnameAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLEndTag__TagnameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2608:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2609:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2609:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2610:1: RULE_ID
            {
             before(grammarAccess.getWMLEndTagAccess().getTagnameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLEndTag__TagnameAssignment_15341); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2619:1: rule__WMLKey__KeyNameAssignment_0 : ( RULE_ID ) ;
    public final void rule__WMLKey__KeyNameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2623:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2624:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2624:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2625:1: RULE_ID
            {
             before(grammarAccess.getWMLKeyAccess().getKeyNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__KeyNameAssignment_05372); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2634:1: rule__WMLKey__ValueAssignment_2 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2638:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2639:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2639:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2640:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_25403);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2649:1: rule__WMLKeyValue__Key1ValueAssignment_0 : ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) ) ;
    public final void rule__WMLKeyValue__Key1ValueAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2653:1: ( ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2654:1: ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2654:1: ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2655:1: ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 )
            {
             before(grammarAccess.getWMLKeyValueAccess().getKey1ValueAlternatives_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2656:1: ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2656:2: rule__WMLKeyValue__Key1ValueAlternatives_0_0
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Key1ValueAlternatives_0_0_in_rule__WMLKeyValue__Key1ValueAssignment_05434);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2665:1: rule__WMLKeyValue__Key2ValueAssignment_1 : ( ruleWMLMacro ) ;
    public final void rule__WMLKeyValue__Key2ValueAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2669:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2670:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2670:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2671:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLKeyValueAccess().getKey2ValueWMLMacroParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLKeyValue__Key2ValueAssignment_15467);
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
        "\1\uffff\1\7\2\uffff\1\12\10\uffff\2\17\1\uffff";
    static final String DFA7_minS =
        "\2\4\2\uffff\1\4\3\uffff\1\4\2\uffff\1\6\1\uffff\2\4\1\uffff";
    static final String DFA7_maxS =
        "\1\36\1\37\2\uffff\1\40\3\uffff\1\20\2\uffff\1\6\1\uffff\2\40\1"+
        "\uffff";
    static final String DFA7_acceptS =
        "\2\uffff\1\2\1\3\1\uffff\1\7\1\10\1\1\1\uffff\1\6\1\5\1\uffff\1"+
        "\11\2\uffff\1\4";
    static final String DFA7_specialS =
        "\20\uffff}>";
    static final String[] DFA7_transitionS = {
            "\1\1\1\2\1\4\2\uffff\1\3\7\uffff\10\5\5\uffff\1\3",
            "\1\10\6\uffff\2\11\3\uffff\1\11\10\uffff\1\7\1\uffff\1\7\1\uffff"+
            "\1\7\1\uffff\1\6",
            "",
            "",
            "\1\12\5\uffff\1\14\1\uffff\1\13\14\uffff\1\12\1\uffff\1\12\1"+
            "\uffff\1\12\1\uffff\2\14",
            "",
            "",
            "",
            "\1\11\6\uffff\2\11\2\uffff\1\7\1\11",
            "",
            "",
            "\1\15",
            "",
            "\1\17\1\uffff\1\16\3\uffff\1\14\16\uffff\1\17\1\uffff\1\17\1"+
            "\uffff\1\17\1\uffff\2\14",
            "\1\17\1\uffff\1\16\3\uffff\1\14\16\uffff\1\17\1\uffff\1\17\1"+
            "\uffff\1\17\1\uffff\2\14",
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
            return "570:1: rule__WMLKeyValue__Key1ValueAlternatives_0_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFLOAT ) | ( RULE_IINT ) | ( rulePATH ) | ( ruleDIRECTION ) | ( ruleLIST ) | ( rulePROGRESSIVE ) );";
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
            return "()* loopback of 1578:1: ( rule__PATH__Group_0__0 )*";
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
    public static final BitSet FOLLOW_ruleWMLEndTag_in_entryRuleWMLEndTag242 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLEndTag249 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__0_in_ruleWMLEndTag275 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey302 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey309 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey335 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue362 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue369 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue395 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_entryRuleFLOAT427 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleFLOAT434 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__0_in_ruleFLOAT464 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING491 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING498 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0_in_ruleTSTRING524 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_entryRulePATH551 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePATH558 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group__0_in_rulePATH584 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_entryRuleDIRECTION611 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleDIRECTION618 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION646 = new BitSet(new long[]{0x0000000001FE0002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION658 = new BitSet(new long[]{0x0000000001FE0002L});
    public static final BitSet FOLLOW_ruleLIST_in_entryRuleLIST688 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleLIST695 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group__0_in_ruleLIST721 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE748 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePROGRESSIVE755 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__0_in_rulePROGRESSIVE781 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__RtagsAssignment_0_in_rule__WMLRoot__Alternatives817 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__RmacrosAssignment_1_in_rule__WMLRoot__Alternatives835 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacro__ValueAlternatives_2_0868 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLMacro__ValueAlternatives_2_0885 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_9_in_rule__WMLMacro__ValueAlternatives_2_0903 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_rule__WMLMacro__ValueAlternatives_2_0923 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_rule__WMLMacro__ValueAlternatives_2_0943 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__WMLMacro__ValueAlternatives_2_0963 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__WMLMacro__ValueAlternatives_2_0983 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__WMLMacro__ValueAlternatives_2_01003 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLMacro__ValueAlternatives_2_01023 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__WMLMacro__ValueAlternatives_2_01043 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TtagsAssignment_3_0_in_rule__WMLTag__Alternatives_31077 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TkeysAssignment_3_1_in_rule__WMLTag__Alternatives_31095 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TmacrosAssignment_3_2_in_rule__WMLTag__Alternatives_31113 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Key1ValueAssignment_0_in_rule__WMLKeyValue__Alternatives1146 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Key2ValueAssignment_1_in_rule__WMLKeyValue__Alternatives1164 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01197 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01214 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01231 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01248 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01265 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01282 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01299 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleLIST_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01316 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01333 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_rule__PATH__Alternatives_0_11366 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__PATH__Alternatives_0_11386 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_rule__DIRECTION__Alternatives_01421 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_rule__DIRECTION__Alternatives_01441 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_rule__DIRECTION__Alternatives_01461 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__DIRECTION__Alternatives_01481 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__DIRECTION__Alternatives_01501 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__DIRECTION__Alternatives_01521 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__DIRECTION__Alternatives_01541 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_rule__DIRECTION__Alternatives_01561 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_01595 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_01612 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_1_11644 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_1_11661 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_11693 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_11710 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_2_11742 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_2_11759 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__0__Impl_in_rule__WMLMacro__Group__01789 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__1_in_rule__WMLMacro__Group__01792 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__WMLMacro__Group__0__Impl1820 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__1__Impl_in_rule__WMLMacro__Group__11851 = new BitSet(new long[]{0x000000000401FE30L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__2_in_rule__WMLMacro__Group__11854 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__NameAssignment_1_in_rule__WMLMacro__Group__1__Impl1881 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__2__Impl_in_rule__WMLMacro__Group__21911 = new BitSet(new long[]{0x0000000004000000L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__3_in_rule__WMLMacro__Group__21914 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__ValueAssignment_2_in_rule__WMLMacro__Group__2__Impl1941 = new BitSet(new long[]{0x000000000001FE32L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__3__Impl_in_rule__WMLMacro__Group__31972 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_rule__WMLMacro__Group__3__Impl2000 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__02039 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__02042 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rule__WMLTag__Group__0__Impl2070 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__12101 = new BitSet(new long[]{0x0000000010000000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__12104 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__NameAssignment_1_in_rule__WMLTag__Group__1__Impl2131 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__22161 = new BitSet(new long[]{0x000000002A000010L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__22164 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__WMLTag__Group__2__Impl2192 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__32223 = new BitSet(new long[]{0x0000000020000000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__32226 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Alternatives_3_in_rule__WMLTag__Group__3__Impl2253 = new BitSet(new long[]{0x000000000A000012L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__42284 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__EndAssignment_4_in_rule__WMLTag__Group__4__Impl2311 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__0__Impl_in_rule__WMLEndTag__Group__02351 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__1_in_rule__WMLEndTag__Group__02354 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__WMLEndTag__Group__0__Impl2382 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__1__Impl_in_rule__WMLEndTag__Group__12413 = new BitSet(new long[]{0x0000000010000000L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__2_in_rule__WMLEndTag__Group__12416 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__TagnameAssignment_1_in_rule__WMLEndTag__Group__1__Impl2443 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__2__Impl_in_rule__WMLEndTag__Group__22473 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__WMLEndTag__Group__2__Impl2501 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02538 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02541 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__KeyNameAssignment_0_in_rule__WMLKey__Group__0__Impl2568 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12598 = new BitSet(new long[]{0x0000000043FE0270L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12601 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLKey__Group__1__Impl2629 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22660 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2687 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__0__Impl_in_rule__FLOAT__Group__02723 = new BitSet(new long[]{0x0000000000001000L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__1_in_rule__FLOAT__Group__02726 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__FLOAT__Group__0__Impl2753 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__1__Impl_in_rule__FLOAT__Group__12782 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__2_in_rule__FLOAT__Group__12785 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__FLOAT__Group__1__Impl2813 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__2__Impl_in_rule__FLOAT__Group__22844 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2874 = new BitSet(new long[]{0x0000000000000042L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2887 = new BitSet(new long[]{0x0000000000000042L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__02926 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__02929 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__0_in_rule__TSTRING__Group__0__Impl2956 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__12986 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl3013 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__0__Impl_in_rule__TSTRING__Group_0__03046 = new BitSet(new long[]{0x0000000000000200L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__1_in_rule__TSTRING__Group_0__03049 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__TSTRING__Group_0__0__Impl3078 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__1__Impl_in_rule__TSTRING__Group_0__13111 = new BitSet(new long[]{0x0000000040000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__2_in_rule__TSTRING__Group_0__13114 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_9_in_rule__TSTRING__Group_0__1__Impl3142 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__2__Impl_in_rule__TSTRING__Group_0__23173 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__TSTRING__Group_0__2__Impl3202 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group__0__Impl_in_rule__PATH__Group__03241 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__PATH__Group__1_in_rule__PATH__Group__03244 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__0_in_rule__PATH__Group__0__Impl3271 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group__1__Impl_in_rule__PATH__Group__13302 = new BitSet(new long[]{0x0000000000001000L});
    public static final BitSet FOLLOW_rule__PATH__Group__2_in_rule__PATH__Group__13305 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl3335 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl3348 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group__2__Impl_in_rule__PATH__Group__23381 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__PATH__Group__3_in_rule__PATH__Group__23384 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__PATH__Group__2__Impl3412 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group__3__Impl_in_rule__PATH__Group__33443 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl3473 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl3486 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__0__Impl_in_rule__PATH__Group_0__03527 = new BitSet(new long[]{0x0000000000010800L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__1_in_rule__PATH__Group_0__03530 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl3560 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl3573 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__1__Impl_in_rule__PATH__Group_0__13606 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Alternatives_0_1_in_rule__PATH__Group_0__1__Impl3633 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__0__Impl_in_rule__DIRECTION__Group__03667 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__1_in_rule__DIRECTION__Group__03670 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Alternatives_0_in_rule__DIRECTION__Group__0__Impl3697 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__1__Impl_in_rule__DIRECTION__Group__13727 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__DIRECTION__Group__1__Impl3756 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group__0__Impl_in_rule__LIST__Group__03793 = new BitSet(new long[]{0x0000000080000000L});
    public static final BitSet FOLLOW_rule__LIST__Group__1_in_rule__LIST__Group__03796 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__LIST__Group__0__Impl3823 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group__1__Impl_in_rule__LIST__Group__13852 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl3881 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl3893 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__0__Impl_in_rule__LIST__Group_1__03930 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__1_in_rule__LIST__Group_1__03933 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__LIST__Group_1__0__Impl3961 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__1__Impl_in_rule__LIST__Group_1__13992 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__LIST__Group_1__1__Impl4019 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__0__Impl_in_rule__PROGRESSIVE__Group__04052 = new BitSet(new long[]{0x0000000180000400L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__1_in_rule__PROGRESSIVE__Group__04055 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_0_in_rule__PROGRESSIVE__Group__0__Impl4082 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__1__Impl_in_rule__PROGRESSIVE__Group__14112 = new BitSet(new long[]{0x0000000080000400L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__2_in_rule__PROGRESSIVE__Group__14115 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__0_in_rule__PROGRESSIVE__Group__1__Impl4142 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__2__Impl_in_rule__PROGRESSIVE__Group__24173 = new BitSet(new long[]{0x0000000080000000L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__3_in_rule__PROGRESSIVE__Group__24176 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__0_in_rule__PROGRESSIVE__Group__2__Impl4203 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__3__Impl_in_rule__PROGRESSIVE__Group__34234 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl4263 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl4275 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__0__Impl_in_rule__PROGRESSIVE__Group_1__04316 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__1_in_rule__PROGRESSIVE__Group_1__04319 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__PROGRESSIVE__Group_1__0__Impl4347 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__1__Impl_in_rule__PROGRESSIVE__Group_1__14378 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_1_1_in_rule__PROGRESSIVE__Group_1__1__Impl4405 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__0__Impl_in_rule__PROGRESSIVE__Group_2__04439 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__1_in_rule__PROGRESSIVE__Group_2__04442 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_rule__PROGRESSIVE__Group_2__0__Impl4470 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__1__Impl_in_rule__PROGRESSIVE__Group_2__14501 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_2__1__Impl4528 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__0__Impl_in_rule__PROGRESSIVE__Group_3__04561 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__1_in_rule__PROGRESSIVE__Group_3__04564 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__PROGRESSIVE__Group_3__0__Impl4592 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__1__Impl_in_rule__PROGRESSIVE__Group_3__14623 = new BitSet(new long[]{0x0000000100000402L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__2_in_rule__PROGRESSIVE__Group_3__14626 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_3_1_in_rule__PROGRESSIVE__Group_3__1__Impl4653 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__2__Impl_in_rule__PROGRESSIVE__Group_3__24683 = new BitSet(new long[]{0x0000000000000402L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__3_in_rule__PROGRESSIVE__Group_3__24686 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__0_in_rule__PROGRESSIVE__Group_3__2__Impl4713 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__3__Impl_in_rule__PROGRESSIVE__Group_3__34744 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__0_in_rule__PROGRESSIVE__Group_3__3__Impl4771 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__0__Impl_in_rule__PROGRESSIVE__Group_3_2__04810 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__1_in_rule__PROGRESSIVE__Group_3_2__04813 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__PROGRESSIVE__Group_3_2__0__Impl4841 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__1__Impl_in_rule__PROGRESSIVE__Group_3_2__14872 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_3_2_1_in_rule__PROGRESSIVE__Group_3_2__1__Impl4899 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__0__Impl_in_rule__PROGRESSIVE__Group_3_3__04933 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__1_in_rule__PROGRESSIVE__Group_3_3__04936 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_rule__PROGRESSIVE__Group_3_3__0__Impl4964 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__1__Impl_in_rule__PROGRESSIVE__Group_3_3__14995 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_3_3__1__Impl5022 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRoot__RtagsAssignment_05060 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLRoot__RmacrosAssignment_15091 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacro__NameAssignment_15122 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__ValueAlternatives_2_0_in_rule__WMLMacro__ValueAssignment_25153 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_15186 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLTag__TtagsAssignment_3_05217 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLTag__TkeysAssignment_3_15248 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLTag__TmacrosAssignment_3_25279 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLEndTag_in_rule__WMLTag__EndAssignment_45310 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLEndTag__TagnameAssignment_15341 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__KeyNameAssignment_05372 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_25403 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Key1ValueAlternatives_0_0_in_rule__WMLKeyValue__Key1ValueAssignment_05434 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLKeyValue__Key2ValueAssignment_15467 = new BitSet(new long[]{0x0000000000000002L});

}