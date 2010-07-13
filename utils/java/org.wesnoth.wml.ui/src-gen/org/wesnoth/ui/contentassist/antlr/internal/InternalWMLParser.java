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


    // $ANTLR start entryRuleWMLKey
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:145:1: entryRuleWMLKey : ruleWMLKey EOF ;
    public final void entryRuleWMLKey() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:146:1: ( ruleWMLKey EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:147:1: ruleWMLKey EOF
            {
             before(grammarAccess.getWMLKeyRule()); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey242);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLKeyRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey249); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:154:1: ruleWMLKey : ( ( rule__WMLKey__Group__0 ) ) ;
    public final void ruleWMLKey() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:158:2: ( ( ( rule__WMLKey__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:159:1: ( ( rule__WMLKey__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:159:1: ( ( rule__WMLKey__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:160:1: ( rule__WMLKey__Group__0 )
            {
             before(grammarAccess.getWMLKeyAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:161:1: ( rule__WMLKey__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:161:2: rule__WMLKey__Group__0
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey275);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:173:1: entryRuleWMLKeyValue : ruleWMLKeyValue EOF ;
    public final void entryRuleWMLKeyValue() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:174:1: ( ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:175:1: ruleWMLKeyValue EOF
            {
             before(grammarAccess.getWMLKeyValueRule()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue302);
            ruleWMLKeyValue();
            _fsp--;

             after(grammarAccess.getWMLKeyValueRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue309); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:182:1: ruleWMLKeyValue : ( ( rule__WMLKeyValue__Alternatives ) ) ;
    public final void ruleWMLKeyValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:186:2: ( ( ( rule__WMLKeyValue__Alternatives ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:187:1: ( ( rule__WMLKeyValue__Alternatives ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:187:1: ( ( rule__WMLKeyValue__Alternatives ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:188:1: ( rule__WMLKeyValue__Alternatives )
            {
             before(grammarAccess.getWMLKeyValueAccess().getAlternatives()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:189:1: ( rule__WMLKeyValue__Alternatives )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:189:2: rule__WMLKeyValue__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue335);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:201:1: entryRuleFLOAT : ruleFLOAT EOF ;
    public final void entryRuleFLOAT() throws RecognitionException {

        	HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();

        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:205:1: ( ruleFLOAT EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:206:1: ruleFLOAT EOF
            {
             before(grammarAccess.getFLOATRule()); 
            pushFollow(FOLLOW_ruleFLOAT_in_entryRuleFLOAT367);
            ruleFLOAT();
            _fsp--;

             after(grammarAccess.getFLOATRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleFLOAT374); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:216:1: ruleFLOAT : ( ( rule__FLOAT__Group__0 ) ) ;
    public final void ruleFLOAT() throws RecognitionException {

        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();
        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:221:2: ( ( ( rule__FLOAT__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:222:1: ( ( rule__FLOAT__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:222:1: ( ( rule__FLOAT__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:223:1: ( rule__FLOAT__Group__0 )
            {
             before(grammarAccess.getFLOATAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:224:1: ( rule__FLOAT__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:224:2: rule__FLOAT__Group__0
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__0_in_ruleFLOAT404);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:237:1: entryRuleTSTRING : ruleTSTRING EOF ;
    public final void entryRuleTSTRING() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:238:1: ( ruleTSTRING EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:239:1: ruleTSTRING EOF
            {
             before(grammarAccess.getTSTRINGRule()); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING431);
            ruleTSTRING();
            _fsp--;

             after(grammarAccess.getTSTRINGRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING438); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:246:1: ruleTSTRING : ( ( rule__TSTRING__Group__0 ) ) ;
    public final void ruleTSTRING() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:250:2: ( ( ( rule__TSTRING__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:251:1: ( ( rule__TSTRING__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:251:1: ( ( rule__TSTRING__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:252:1: ( rule__TSTRING__Group__0 )
            {
             before(grammarAccess.getTSTRINGAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:253:1: ( rule__TSTRING__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:253:2: rule__TSTRING__Group__0
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0_in_ruleTSTRING464);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:265:1: entryRulePATH : rulePATH EOF ;
    public final void entryRulePATH() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:266:1: ( rulePATH EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:267:1: rulePATH EOF
            {
             before(grammarAccess.getPATHRule()); 
            pushFollow(FOLLOW_rulePATH_in_entryRulePATH491);
            rulePATH();
            _fsp--;

             after(grammarAccess.getPATHRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePATH498); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:274:1: rulePATH : ( ( rule__PATH__Group__0 ) ) ;
    public final void rulePATH() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:278:2: ( ( ( rule__PATH__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:279:1: ( ( rule__PATH__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:279:1: ( ( rule__PATH__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:280:1: ( rule__PATH__Group__0 )
            {
             before(grammarAccess.getPATHAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:1: ( rule__PATH__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:2: rule__PATH__Group__0
            {
            pushFollow(FOLLOW_rule__PATH__Group__0_in_rulePATH524);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:293:1: entryRuleDIRECTION : ruleDIRECTION EOF ;
    public final void entryRuleDIRECTION() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:294:1: ( ruleDIRECTION EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:295:1: ruleDIRECTION EOF
            {
             before(grammarAccess.getDIRECTIONRule()); 
            pushFollow(FOLLOW_ruleDIRECTION_in_entryRuleDIRECTION551);
            ruleDIRECTION();
            _fsp--;

             after(grammarAccess.getDIRECTIONRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleDIRECTION558); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:302:1: ruleDIRECTION : ( ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* ) ) ;
    public final void ruleDIRECTION() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:306:2: ( ( ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:307:1: ( ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:307:1: ( ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:308:1: ( ( rule__DIRECTION__Group__0 ) ) ( ( rule__DIRECTION__Group__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:308:1: ( ( rule__DIRECTION__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:1: ( rule__DIRECTION__Group__0 )
            {
             before(grammarAccess.getDIRECTIONAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:310:1: ( rule__DIRECTION__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:310:2: rule__DIRECTION__Group__0
            {
            pushFollow(FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION586);
            rule__DIRECTION__Group__0();
            _fsp--;


            }

             after(grammarAccess.getDIRECTIONAccess().getGroup()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:313:1: ( ( rule__DIRECTION__Group__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:314:1: ( rule__DIRECTION__Group__0 )*
            {
             before(grammarAccess.getDIRECTIONAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:315:1: ( rule__DIRECTION__Group__0 )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( ((LA2_0>=17 && LA2_0<=24)) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:315:2: rule__DIRECTION__Group__0
            	    {
            	    pushFollow(FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION598);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:328:1: entryRuleLIST : ruleLIST EOF ;
    public final void entryRuleLIST() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:329:1: ( ruleLIST EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:330:1: ruleLIST EOF
            {
             before(grammarAccess.getLISTRule()); 
            pushFollow(FOLLOW_ruleLIST_in_entryRuleLIST628);
            ruleLIST();
            _fsp--;

             after(grammarAccess.getLISTRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleLIST635); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:1: ruleLIST : ( ( rule__LIST__Group__0 ) ) ;
    public final void ruleLIST() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:341:2: ( ( ( rule__LIST__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:342:1: ( ( rule__LIST__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:342:1: ( ( rule__LIST__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:343:1: ( rule__LIST__Group__0 )
            {
             before(grammarAccess.getLISTAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:344:1: ( rule__LIST__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:344:2: rule__LIST__Group__0
            {
            pushFollow(FOLLOW_rule__LIST__Group__0_in_ruleLIST661);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:356:1: entryRulePROGRESSIVE : rulePROGRESSIVE EOF ;
    public final void entryRulePROGRESSIVE() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:357:1: ( rulePROGRESSIVE EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:358:1: rulePROGRESSIVE EOF
            {
             before(grammarAccess.getPROGRESSIVERule()); 
            pushFollow(FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE688);
            rulePROGRESSIVE();
            _fsp--;

             after(grammarAccess.getPROGRESSIVERule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePROGRESSIVE695); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:365:1: rulePROGRESSIVE : ( ( rule__PROGRESSIVE__Group__0 ) ) ;
    public final void rulePROGRESSIVE() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:369:2: ( ( ( rule__PROGRESSIVE__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:370:1: ( ( rule__PROGRESSIVE__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:370:1: ( ( rule__PROGRESSIVE__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:371:1: ( rule__PROGRESSIVE__Group__0 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:372:1: ( rule__PROGRESSIVE__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:372:2: rule__PROGRESSIVE__Group__0
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__0_in_rulePROGRESSIVE721);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:384:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__RtagsAssignment_0 ) ) | ( ( rule__WMLRoot__RmacrosAssignment_1 ) ) );
    public final void rule__WMLRoot__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:388:1: ( ( ( rule__WMLRoot__RtagsAssignment_0 ) ) | ( ( rule__WMLRoot__RmacrosAssignment_1 ) ) )
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
                    new NoViableAltException("384:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__RtagsAssignment_0 ) ) | ( ( rule__WMLRoot__RmacrosAssignment_1 ) ) );", 3, 0, input);

                throw nvae;
            }
            switch (alt3) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:389:1: ( ( rule__WMLRoot__RtagsAssignment_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:389:1: ( ( rule__WMLRoot__RtagsAssignment_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:390:1: ( rule__WMLRoot__RtagsAssignment_0 )
                    {
                     before(grammarAccess.getWMLRootAccess().getRtagsAssignment_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:391:1: ( rule__WMLRoot__RtagsAssignment_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:391:2: rule__WMLRoot__RtagsAssignment_0
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__RtagsAssignment_0_in_rule__WMLRoot__Alternatives757);
                    rule__WMLRoot__RtagsAssignment_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getRtagsAssignment_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:395:6: ( ( rule__WMLRoot__RmacrosAssignment_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:395:6: ( ( rule__WMLRoot__RmacrosAssignment_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:396:1: ( rule__WMLRoot__RmacrosAssignment_1 )
                    {
                     before(grammarAccess.getWMLRootAccess().getRmacrosAssignment_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:397:1: ( rule__WMLRoot__RmacrosAssignment_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:397:2: rule__WMLRoot__RmacrosAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__RmacrosAssignment_1_in_rule__WMLRoot__Alternatives775);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:406:1: rule__WMLMacro__ValueAlternatives_2_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( ':' ) | ( '-' ) | ( '.' ) | ( '(' ) | ( ')' ) | ( '=' ) | ( '/' ) );
    public final void rule__WMLMacro__ValueAlternatives_2_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:410:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( ':' ) | ( '-' ) | ( '.' ) | ( '(' ) | ( ')' ) | ( '=' ) | ( '/' ) )
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
                    new NoViableAltException("406:1: rule__WMLMacro__ValueAlternatives_2_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( ':' ) | ( '-' ) | ( '.' ) | ( '(' ) | ( ')' ) | ( '=' ) | ( '/' ) );", 4, 0, input);

                throw nvae;
            }

            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:411:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:411:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:412:1: RULE_ID
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueIDTerminalRuleCall_2_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacro__ValueAlternatives_2_0808); 
                     after(grammarAccess.getWMLMacroAccess().getValueIDTerminalRuleCall_2_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:417:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:417:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:418:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueSTRINGTerminalRuleCall_2_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLMacro__ValueAlternatives_2_0825); 
                     after(grammarAccess.getWMLMacroAccess().getValueSTRINGTerminalRuleCall_2_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:423:6: ( '_' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:423:6: ( '_' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:424:1: '_'
                    {
                     before(grammarAccess.getWMLMacroAccess().getValue_Keyword_2_0_2()); 
                    match(input,9,FOLLOW_9_in_rule__WMLMacro__ValueAlternatives_2_0843); 
                     after(grammarAccess.getWMLMacroAccess().getValue_Keyword_2_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:431:6: ( ':' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:431:6: ( ':' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:432:1: ':'
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueColonKeyword_2_0_3()); 
                    match(input,10,FOLLOW_10_in_rule__WMLMacro__ValueAlternatives_2_0863); 
                     after(grammarAccess.getWMLMacroAccess().getValueColonKeyword_2_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:439:6: ( '-' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:439:6: ( '-' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:440:1: '-'
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueHyphenMinusKeyword_2_0_4()); 
                    match(input,11,FOLLOW_11_in_rule__WMLMacro__ValueAlternatives_2_0883); 
                     after(grammarAccess.getWMLMacroAccess().getValueHyphenMinusKeyword_2_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:447:6: ( '.' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:447:6: ( '.' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:448:1: '.'
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueFullStopKeyword_2_0_5()); 
                    match(input,12,FOLLOW_12_in_rule__WMLMacro__ValueAlternatives_2_0903); 
                     after(grammarAccess.getWMLMacroAccess().getValueFullStopKeyword_2_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:455:6: ( '(' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:455:6: ( '(' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:456:1: '('
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueLeftParenthesisKeyword_2_0_6()); 
                    match(input,13,FOLLOW_13_in_rule__WMLMacro__ValueAlternatives_2_0923); 
                     after(grammarAccess.getWMLMacroAccess().getValueLeftParenthesisKeyword_2_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:463:6: ( ')' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:463:6: ( ')' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:464:1: ')'
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueRightParenthesisKeyword_2_0_7()); 
                    match(input,14,FOLLOW_14_in_rule__WMLMacro__ValueAlternatives_2_0943); 
                     after(grammarAccess.getWMLMacroAccess().getValueRightParenthesisKeyword_2_0_7()); 

                    }


                    }
                    break;
                case 9 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:471:6: ( '=' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:471:6: ( '=' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:472:1: '='
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueEqualsSignKeyword_2_0_8()); 
                    match(input,15,FOLLOW_15_in_rule__WMLMacro__ValueAlternatives_2_0963); 
                     after(grammarAccess.getWMLMacroAccess().getValueEqualsSignKeyword_2_0_8()); 

                    }


                    }
                    break;
                case 10 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:479:6: ( '/' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:479:6: ( '/' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:480:1: '/'
                    {
                     before(grammarAccess.getWMLMacroAccess().getValueSolidusKeyword_2_0_9()); 
                    match(input,16,FOLLOW_16_in_rule__WMLMacro__ValueAlternatives_2_0983); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:492:1: rule__WMLTag__Alternatives_3 : ( ( ( rule__WMLTag__TtagsAssignment_3_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_3_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_3_2 ) ) );
    public final void rule__WMLTag__Alternatives_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:496:1: ( ( ( rule__WMLTag__TtagsAssignment_3_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_3_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_3_2 ) ) )
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
                    new NoViableAltException("492:1: rule__WMLTag__Alternatives_3 : ( ( ( rule__WMLTag__TtagsAssignment_3_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_3_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_3_2 ) ) );", 5, 0, input);

                throw nvae;
            }

            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:497:1: ( ( rule__WMLTag__TtagsAssignment_3_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:497:1: ( ( rule__WMLTag__TtagsAssignment_3_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:498:1: ( rule__WMLTag__TtagsAssignment_3_0 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTtagsAssignment_3_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:499:1: ( rule__WMLTag__TtagsAssignment_3_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:499:2: rule__WMLTag__TtagsAssignment_3_0
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TtagsAssignment_3_0_in_rule__WMLTag__Alternatives_31017);
                    rule__WMLTag__TtagsAssignment_3_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTtagsAssignment_3_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:503:6: ( ( rule__WMLTag__TkeysAssignment_3_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:503:6: ( ( rule__WMLTag__TkeysAssignment_3_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:504:1: ( rule__WMLTag__TkeysAssignment_3_1 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTkeysAssignment_3_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:505:1: ( rule__WMLTag__TkeysAssignment_3_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:505:2: rule__WMLTag__TkeysAssignment_3_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TkeysAssignment_3_1_in_rule__WMLTag__Alternatives_31035);
                    rule__WMLTag__TkeysAssignment_3_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTkeysAssignment_3_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:509:6: ( ( rule__WMLTag__TmacrosAssignment_3_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:509:6: ( ( rule__WMLTag__TmacrosAssignment_3_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:510:1: ( rule__WMLTag__TmacrosAssignment_3_2 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTmacrosAssignment_3_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:511:1: ( rule__WMLTag__TmacrosAssignment_3_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:511:2: rule__WMLTag__TmacrosAssignment_3_2
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TmacrosAssignment_3_2_in_rule__WMLTag__Alternatives_31053);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:520:1: rule__WMLKeyValue__Alternatives : ( ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) ) | ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) ) );
    public final void rule__WMLKeyValue__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:524:1: ( ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) ) | ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) ) )
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
                    new NoViableAltException("520:1: rule__WMLKeyValue__Alternatives : ( ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) ) | ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) ) );", 6, 0, input);

                throw nvae;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:525:1: ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:525:1: ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:526:1: ( rule__WMLKeyValue__Key1ValueAssignment_0 )
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueAssignment_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:527:1: ( rule__WMLKeyValue__Key1ValueAssignment_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:527:2: rule__WMLKeyValue__Key1ValueAssignment_0
                    {
                    pushFollow(FOLLOW_rule__WMLKeyValue__Key1ValueAssignment_0_in_rule__WMLKeyValue__Alternatives1086);
                    rule__WMLKeyValue__Key1ValueAssignment_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueAssignment_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:531:6: ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:531:6: ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:532:1: ( rule__WMLKeyValue__Key2ValueAssignment_1 )
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey2ValueAssignment_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:1: ( rule__WMLKeyValue__Key2ValueAssignment_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:2: rule__WMLKeyValue__Key2ValueAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLKeyValue__Key2ValueAssignment_1_in_rule__WMLKeyValue__Alternatives1104);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:542:1: rule__WMLKeyValue__Key1ValueAlternatives_0_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFLOAT ) | ( RULE_IINT ) | ( rulePATH ) | ( ruleDIRECTION ) | ( ruleLIST ) | ( rulePROGRESSIVE ) );
    public final void rule__WMLKeyValue__Key1ValueAlternatives_0_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:546:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFLOAT ) | ( RULE_IINT ) | ( rulePATH ) | ( ruleDIRECTION ) | ( ruleLIST ) | ( rulePROGRESSIVE ) )
            int alt7=9;
            alt7 = dfa7.predict(input);
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:547:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:547:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:548:1: RULE_ID
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueIDTerminalRuleCall_0_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01137); 
                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueIDTerminalRuleCall_0_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:553:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:553:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:554:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueSTRINGTerminalRuleCall_0_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01154); 
                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueSTRINGTerminalRuleCall_0_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:559:6: ( ruleTSTRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:559:6: ( ruleTSTRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:560:1: ruleTSTRING
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueTSTRINGParserRuleCall_0_0_2()); 
                    pushFollow(FOLLOW_ruleTSTRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01171);
                    ruleTSTRING();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueTSTRINGParserRuleCall_0_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:565:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:565:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:566:1: ruleFLOAT
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueFLOATParserRuleCall_0_0_3()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01188);
                    ruleFLOAT();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueFLOATParserRuleCall_0_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:571:6: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:571:6: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:572:1: RULE_IINT
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueIINTTerminalRuleCall_0_0_4()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01205); 
                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueIINTTerminalRuleCall_0_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:577:6: ( rulePATH )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:577:6: ( rulePATH )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:578:1: rulePATH
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValuePATHParserRuleCall_0_0_5()); 
                    pushFollow(FOLLOW_rulePATH_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01222);
                    rulePATH();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValuePATHParserRuleCall_0_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:583:6: ( ruleDIRECTION )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:583:6: ( ruleDIRECTION )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:584:1: ruleDIRECTION
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueDIRECTIONParserRuleCall_0_0_6()); 
                    pushFollow(FOLLOW_ruleDIRECTION_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01239);
                    ruleDIRECTION();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueDIRECTIONParserRuleCall_0_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:589:6: ( ruleLIST )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:589:6: ( ruleLIST )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:590:1: ruleLIST
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueLISTParserRuleCall_0_0_7()); 
                    pushFollow(FOLLOW_ruleLIST_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01256);
                    ruleLIST();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueLISTParserRuleCall_0_0_7()); 

                    }


                    }
                    break;
                case 9 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:595:6: ( rulePROGRESSIVE )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:595:6: ( rulePROGRESSIVE )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:596:1: rulePROGRESSIVE
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValuePROGRESSIVEParserRuleCall_0_0_8()); 
                    pushFollow(FOLLOW_rulePROGRESSIVE_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01273);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:606:1: rule__PATH__Alternatives_0_1 : ( ( '-' ) | ( '/' ) );
    public final void rule__PATH__Alternatives_0_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:610:1: ( ( '-' ) | ( '/' ) )
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
                    new NoViableAltException("606:1: rule__PATH__Alternatives_0_1 : ( ( '-' ) | ( '/' ) );", 8, 0, input);

                throw nvae;
            }
            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:611:1: ( '-' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:611:1: ( '-' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:612:1: '-'
                    {
                     before(grammarAccess.getPATHAccess().getHyphenMinusKeyword_0_1_0()); 
                    match(input,11,FOLLOW_11_in_rule__PATH__Alternatives_0_11306); 
                     after(grammarAccess.getPATHAccess().getHyphenMinusKeyword_0_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:619:6: ( '/' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:619:6: ( '/' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:620:1: '/'
                    {
                     before(grammarAccess.getPATHAccess().getSolidusKeyword_0_1_1()); 
                    match(input,16,FOLLOW_16_in_rule__PATH__Alternatives_0_11326); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:632:1: rule__DIRECTION__Alternatives_0 : ( ( 'n' ) | ( 's' ) | ( 'w' ) | ( 'e' ) | ( 'sw' ) | ( 'se' ) | ( 'ne' ) | ( 'nw' ) );
    public final void rule__DIRECTION__Alternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:636:1: ( ( 'n' ) | ( 's' ) | ( 'w' ) | ( 'e' ) | ( 'sw' ) | ( 'se' ) | ( 'ne' ) | ( 'nw' ) )
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
                    new NoViableAltException("632:1: rule__DIRECTION__Alternatives_0 : ( ( 'n' ) | ( 's' ) | ( 'w' ) | ( 'e' ) | ( 'sw' ) | ( 'se' ) | ( 'ne' ) | ( 'nw' ) );", 9, 0, input);

                throw nvae;
            }

            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:637:1: ( 'n' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:637:1: ( 'n' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:638:1: 'n'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getNKeyword_0_0()); 
                    match(input,17,FOLLOW_17_in_rule__DIRECTION__Alternatives_01361); 
                     after(grammarAccess.getDIRECTIONAccess().getNKeyword_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:645:6: ( 's' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:645:6: ( 's' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:646:1: 's'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getSKeyword_0_1()); 
                    match(input,18,FOLLOW_18_in_rule__DIRECTION__Alternatives_01381); 
                     after(grammarAccess.getDIRECTIONAccess().getSKeyword_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:653:6: ( 'w' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:653:6: ( 'w' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:654:1: 'w'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getWKeyword_0_2()); 
                    match(input,19,FOLLOW_19_in_rule__DIRECTION__Alternatives_01401); 
                     after(grammarAccess.getDIRECTIONAccess().getWKeyword_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:661:6: ( 'e' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:661:6: ( 'e' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:662:1: 'e'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getEKeyword_0_3()); 
                    match(input,20,FOLLOW_20_in_rule__DIRECTION__Alternatives_01421); 
                     after(grammarAccess.getDIRECTIONAccess().getEKeyword_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:669:6: ( 'sw' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:669:6: ( 'sw' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:670:1: 'sw'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getSwKeyword_0_4()); 
                    match(input,21,FOLLOW_21_in_rule__DIRECTION__Alternatives_01441); 
                     after(grammarAccess.getDIRECTIONAccess().getSwKeyword_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:677:6: ( 'se' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:677:6: ( 'se' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:678:1: 'se'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getSeKeyword_0_5()); 
                    match(input,22,FOLLOW_22_in_rule__DIRECTION__Alternatives_01461); 
                     after(grammarAccess.getDIRECTIONAccess().getSeKeyword_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:685:6: ( 'ne' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:685:6: ( 'ne' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:686:1: 'ne'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getNeKeyword_0_6()); 
                    match(input,23,FOLLOW_23_in_rule__DIRECTION__Alternatives_01481); 
                     after(grammarAccess.getDIRECTIONAccess().getNeKeyword_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:693:6: ( 'nw' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:693:6: ( 'nw' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:694:1: 'nw'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getNwKeyword_0_7()); 
                    match(input,24,FOLLOW_24_in_rule__DIRECTION__Alternatives_01501); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:706:1: rule__PROGRESSIVE__Alternatives_0 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:710:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
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
                        new NoViableAltException("706:1: rule__PROGRESSIVE__Alternatives_0 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 10, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("706:1: rule__PROGRESSIVE__Alternatives_0 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 10, 0, input);

                throw nvae;
            }
            switch (alt10) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:711:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:711:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:712:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_0_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_01535); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:717:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:717:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:718:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_0_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_01552);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:728:1: rule__PROGRESSIVE__Alternatives_1_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_1_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:732:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
            int alt11=2;
            int LA11_0 = input.LA(1);

            if ( (LA11_0==RULE_IINT) ) {
                int LA11_1 = input.LA(2);

                if ( (LA11_1==12) ) {
                    alt11=2;
                }
                else if ( (LA11_1==10||LA11_1==31) ) {
                    alt11=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("728:1: rule__PROGRESSIVE__Alternatives_1_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 11, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("728:1: rule__PROGRESSIVE__Alternatives_1_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 11, 0, input);

                throw nvae;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:733:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:733:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:734:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_1_1_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_1_11584); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_1_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:739:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:739:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:740:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_1_1_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_1_11601);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:750:1: rule__PROGRESSIVE__Alternatives_3_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_3_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:754:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0==RULE_IINT) ) {
                int LA12_1 = input.LA(2);

                if ( (LA12_1==EOF||LA12_1==RULE_ID||LA12_1==10||LA12_1==25||LA12_1==27||LA12_1==29||(LA12_1>=31 && LA12_1<=32)) ) {
                    alt12=1;
                }
                else if ( (LA12_1==12) ) {
                    alt12=2;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("750:1: rule__PROGRESSIVE__Alternatives_3_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 12, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("750:1: rule__PROGRESSIVE__Alternatives_3_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 12, 0, input);

                throw nvae;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:755:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:755:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:756:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_1_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_11633); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:761:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:761:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:762:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_1_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_11650);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:772:1: rule__PROGRESSIVE__Alternatives_3_2_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_3_2_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:776:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
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
                        new NoViableAltException("772:1: rule__PROGRESSIVE__Alternatives_3_2_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 13, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("772:1: rule__PROGRESSIVE__Alternatives_3_2_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 13, 0, input);

                throw nvae;
            }
            switch (alt13) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:777:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:777:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:778:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_2_1_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_2_11682); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_2_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:783:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:783:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:784:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_2_1_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_2_11699);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:796:1: rule__WMLMacro__Group__0 : rule__WMLMacro__Group__0__Impl rule__WMLMacro__Group__1 ;
    public final void rule__WMLMacro__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:800:1: ( rule__WMLMacro__Group__0__Impl rule__WMLMacro__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:801:2: rule__WMLMacro__Group__0__Impl rule__WMLMacro__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__0__Impl_in_rule__WMLMacro__Group__01729);
            rule__WMLMacro__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacro__Group__1_in_rule__WMLMacro__Group__01732);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:808:1: rule__WMLMacro__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacro__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:812:1: ( ( '{' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:813:1: ( '{' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:813:1: ( '{' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:814:1: '{'
            {
             before(grammarAccess.getWMLMacroAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,25,FOLLOW_25_in_rule__WMLMacro__Group__0__Impl1760); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:827:1: rule__WMLMacro__Group__1 : rule__WMLMacro__Group__1__Impl rule__WMLMacro__Group__2 ;
    public final void rule__WMLMacro__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:831:1: ( rule__WMLMacro__Group__1__Impl rule__WMLMacro__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:832:2: rule__WMLMacro__Group__1__Impl rule__WMLMacro__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__1__Impl_in_rule__WMLMacro__Group__11791);
            rule__WMLMacro__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacro__Group__2_in_rule__WMLMacro__Group__11794);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:839:1: rule__WMLMacro__Group__1__Impl : ( ( rule__WMLMacro__NameAssignment_1 ) ) ;
    public final void rule__WMLMacro__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:843:1: ( ( ( rule__WMLMacro__NameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:844:1: ( ( rule__WMLMacro__NameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:844:1: ( ( rule__WMLMacro__NameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:845:1: ( rule__WMLMacro__NameAssignment_1 )
            {
             before(grammarAccess.getWMLMacroAccess().getNameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:846:1: ( rule__WMLMacro__NameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:846:2: rule__WMLMacro__NameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLMacro__NameAssignment_1_in_rule__WMLMacro__Group__1__Impl1821);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:856:1: rule__WMLMacro__Group__2 : rule__WMLMacro__Group__2__Impl rule__WMLMacro__Group__3 ;
    public final void rule__WMLMacro__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:860:1: ( rule__WMLMacro__Group__2__Impl rule__WMLMacro__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:861:2: rule__WMLMacro__Group__2__Impl rule__WMLMacro__Group__3
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__2__Impl_in_rule__WMLMacro__Group__21851);
            rule__WMLMacro__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacro__Group__3_in_rule__WMLMacro__Group__21854);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:868:1: rule__WMLMacro__Group__2__Impl : ( ( rule__WMLMacro__ValueAssignment_2 )* ) ;
    public final void rule__WMLMacro__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:872:1: ( ( ( rule__WMLMacro__ValueAssignment_2 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:873:1: ( ( rule__WMLMacro__ValueAssignment_2 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:873:1: ( ( rule__WMLMacro__ValueAssignment_2 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:874:1: ( rule__WMLMacro__ValueAssignment_2 )*
            {
             before(grammarAccess.getWMLMacroAccess().getValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:875:1: ( rule__WMLMacro__ValueAssignment_2 )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( ((LA14_0>=RULE_ID && LA14_0<=RULE_STRING)||(LA14_0>=9 && LA14_0<=16)) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:875:2: rule__WMLMacro__ValueAssignment_2
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacro__ValueAssignment_2_in_rule__WMLMacro__Group__2__Impl1881);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:885:1: rule__WMLMacro__Group__3 : rule__WMLMacro__Group__3__Impl ;
    public final void rule__WMLMacro__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:889:1: ( rule__WMLMacro__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:890:2: rule__WMLMacro__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__3__Impl_in_rule__WMLMacro__Group__31912);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:896:1: rule__WMLMacro__Group__3__Impl : ( '}' ) ;
    public final void rule__WMLMacro__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:900:1: ( ( '}' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:901:1: ( '}' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:901:1: ( '}' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:902:1: '}'
            {
             before(grammarAccess.getWMLMacroAccess().getRightCurlyBracketKeyword_3()); 
            match(input,26,FOLLOW_26_in_rule__WMLMacro__Group__3__Impl1940); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:923:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:927:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:928:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01979);
            rule__WMLTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01982);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:935:1: rule__WMLTag__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:939:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:940:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:940:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:941:1: '['
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0()); 
            match(input,27,FOLLOW_27_in_rule__WMLTag__Group__0__Impl2010); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:954:1: rule__WMLTag__Group__1 : rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 ;
    public final void rule__WMLTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:958:1: ( rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:959:2: rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__12041);
            rule__WMLTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__12044);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:966:1: rule__WMLTag__Group__1__Impl : ( ( rule__WMLTag__NameAssignment_1 ) ) ;
    public final void rule__WMLTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:970:1: ( ( ( rule__WMLTag__NameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:971:1: ( ( rule__WMLTag__NameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:971:1: ( ( rule__WMLTag__NameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:972:1: ( rule__WMLTag__NameAssignment_1 )
            {
             before(grammarAccess.getWMLTagAccess().getNameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:973:1: ( rule__WMLTag__NameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:973:2: rule__WMLTag__NameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLTag__NameAssignment_1_in_rule__WMLTag__Group__1__Impl2071);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:983:1: rule__WMLTag__Group__2 : rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 ;
    public final void rule__WMLTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:987:1: ( rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:988:2: rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__22101);
            rule__WMLTag__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__22104);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:995:1: rule__WMLTag__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:999:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1000:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1000:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1001:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_2()); 
            match(input,28,FOLLOW_28_in_rule__WMLTag__Group__2__Impl2132); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1014:1: rule__WMLTag__Group__3 : rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 ;
    public final void rule__WMLTag__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1018:1: ( rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1019:2: rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__32163);
            rule__WMLTag__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__32166);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1026:1: rule__WMLTag__Group__3__Impl : ( ( rule__WMLTag__Alternatives_3 )* ) ;
    public final void rule__WMLTag__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1030:1: ( ( ( rule__WMLTag__Alternatives_3 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1031:1: ( ( rule__WMLTag__Alternatives_3 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1031:1: ( ( rule__WMLTag__Alternatives_3 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1032:1: ( rule__WMLTag__Alternatives_3 )*
            {
             before(grammarAccess.getWMLTagAccess().getAlternatives_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1033:1: ( rule__WMLTag__Alternatives_3 )*
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( (LA15_0==RULE_ID||LA15_0==25||LA15_0==27) ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1033:2: rule__WMLTag__Alternatives_3
            	    {
            	    pushFollow(FOLLOW_rule__WMLTag__Alternatives_3_in_rule__WMLTag__Group__3__Impl2193);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1043:1: rule__WMLTag__Group__4 : rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 ;
    public final void rule__WMLTag__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1047:1: ( rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1048:2: rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__42224);
            rule__WMLTag__Group__4__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__42227);
            rule__WMLTag__Group__5();
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1055:1: rule__WMLTag__Group__4__Impl : ( '[/' ) ;
    public final void rule__WMLTag__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1059:1: ( ( '[/' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1060:1: ( '[/' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1060:1: ( '[/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1061:1: '[/'
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_4()); 
            match(input,29,FOLLOW_29_in_rule__WMLTag__Group__4__Impl2255); 
             after(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_4()); 

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


    // $ANTLR start rule__WMLTag__Group__5
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1074:1: rule__WMLTag__Group__5 : rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 ;
    public final void rule__WMLTag__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1078:1: ( rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1079:2: rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__52286);
            rule__WMLTag__Group__5__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__52289);
            rule__WMLTag__Group__6();
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
    // $ANTLR end rule__WMLTag__Group__5


    // $ANTLR start rule__WMLTag__Group__5__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1086:1: rule__WMLTag__Group__5__Impl : ( ( rule__WMLTag__EndNameAssignment_5 ) ) ;
    public final void rule__WMLTag__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1090:1: ( ( ( rule__WMLTag__EndNameAssignment_5 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1091:1: ( ( rule__WMLTag__EndNameAssignment_5 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1091:1: ( ( rule__WMLTag__EndNameAssignment_5 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1092:1: ( rule__WMLTag__EndNameAssignment_5 )
            {
             before(grammarAccess.getWMLTagAccess().getEndNameAssignment_5()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1093:1: ( rule__WMLTag__EndNameAssignment_5 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1093:2: rule__WMLTag__EndNameAssignment_5
            {
            pushFollow(FOLLOW_rule__WMLTag__EndNameAssignment_5_in_rule__WMLTag__Group__5__Impl2316);
            rule__WMLTag__EndNameAssignment_5();
            _fsp--;


            }

             after(grammarAccess.getWMLTagAccess().getEndNameAssignment_5()); 

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
    // $ANTLR end rule__WMLTag__Group__5__Impl


    // $ANTLR start rule__WMLTag__Group__6
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1103:1: rule__WMLTag__Group__6 : rule__WMLTag__Group__6__Impl ;
    public final void rule__WMLTag__Group__6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1107:1: ( rule__WMLTag__Group__6__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1108:2: rule__WMLTag__Group__6__Impl
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__62346);
            rule__WMLTag__Group__6__Impl();
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
    // $ANTLR end rule__WMLTag__Group__6


    // $ANTLR start rule__WMLTag__Group__6__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1114:1: rule__WMLTag__Group__6__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__6__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1118:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1119:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1119:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1120:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_6()); 
            match(input,28,FOLLOW_28_in_rule__WMLTag__Group__6__Impl2374); 
             after(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_6()); 

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
    // $ANTLR end rule__WMLTag__Group__6__Impl


    // $ANTLR start rule__WMLKey__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1147:1: rule__WMLKey__Group__0 : rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 ;
    public final void rule__WMLKey__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1151:1: ( rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1152:2: rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02419);
            rule__WMLKey__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02422);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1159:1: rule__WMLKey__Group__0__Impl : ( ( rule__WMLKey__KeyNameAssignment_0 ) ) ;
    public final void rule__WMLKey__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1163:1: ( ( ( rule__WMLKey__KeyNameAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1164:1: ( ( rule__WMLKey__KeyNameAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1164:1: ( ( rule__WMLKey__KeyNameAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1165:1: ( rule__WMLKey__KeyNameAssignment_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getKeyNameAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1166:1: ( rule__WMLKey__KeyNameAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1166:2: rule__WMLKey__KeyNameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLKey__KeyNameAssignment_0_in_rule__WMLKey__Group__0__Impl2449);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1176:1: rule__WMLKey__Group__1 : rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 ;
    public final void rule__WMLKey__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1180:1: ( rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1181:2: rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12479);
            rule__WMLKey__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12482);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1188:1: rule__WMLKey__Group__1__Impl : ( '=' ) ;
    public final void rule__WMLKey__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1192:1: ( ( '=' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1193:1: ( '=' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1193:1: ( '=' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1194:1: '='
            {
             before(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1()); 
            match(input,15,FOLLOW_15_in_rule__WMLKey__Group__1__Impl2510); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1207:1: rule__WMLKey__Group__2 : rule__WMLKey__Group__2__Impl ;
    public final void rule__WMLKey__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1211:1: ( rule__WMLKey__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1212:2: rule__WMLKey__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22541);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1218:1: rule__WMLKey__Group__2__Impl : ( ( rule__WMLKey__ValueAssignment_2 ) ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1222:1: ( ( ( rule__WMLKey__ValueAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1223:1: ( ( rule__WMLKey__ValueAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1223:1: ( ( rule__WMLKey__ValueAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1224:1: ( rule__WMLKey__ValueAssignment_2 )
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1225:1: ( rule__WMLKey__ValueAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1225:2: rule__WMLKey__ValueAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2568);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1241:1: rule__FLOAT__Group__0 : rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1 ;
    public final void rule__FLOAT__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1245:1: ( rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1246:2: rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__0__Impl_in_rule__FLOAT__Group__02604);
            rule__FLOAT__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__FLOAT__Group__1_in_rule__FLOAT__Group__02607);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1253:1: rule__FLOAT__Group__0__Impl : ( RULE_IINT ) ;
    public final void rule__FLOAT__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1257:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1258:1: ( RULE_IINT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1258:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1259:1: RULE_IINT
            {
             before(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_0()); 
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__FLOAT__Group__0__Impl2634); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1270:1: rule__FLOAT__Group__1 : rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2 ;
    public final void rule__FLOAT__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1274:1: ( rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1275:2: rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__1__Impl_in_rule__FLOAT__Group__12663);
            rule__FLOAT__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__FLOAT__Group__2_in_rule__FLOAT__Group__12666);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1282:1: rule__FLOAT__Group__1__Impl : ( '.' ) ;
    public final void rule__FLOAT__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1286:1: ( ( '.' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1287:1: ( '.' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1287:1: ( '.' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1288:1: '.'
            {
             before(grammarAccess.getFLOATAccess().getFullStopKeyword_1()); 
            match(input,12,FOLLOW_12_in_rule__FLOAT__Group__1__Impl2694); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1301:1: rule__FLOAT__Group__2 : rule__FLOAT__Group__2__Impl ;
    public final void rule__FLOAT__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1305:1: ( rule__FLOAT__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1306:2: rule__FLOAT__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__2__Impl_in_rule__FLOAT__Group__22725);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1312:1: rule__FLOAT__Group__2__Impl : ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) ) ;
    public final void rule__FLOAT__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1316:1: ( ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1317:1: ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1317:1: ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1318:1: ( ( RULE_IINT ) ) ( ( RULE_IINT )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1318:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1319:1: ( RULE_IINT )
            {
             before(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1320:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1320:3: RULE_IINT
            {
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2755); 

            }

             after(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1323:1: ( ( RULE_IINT )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1324:1: ( RULE_IINT )*
            {
             before(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1325:1: ( RULE_IINT )*
            loop16:
            do {
                int alt16=2;
                int LA16_0 = input.LA(1);

                if ( (LA16_0==RULE_IINT) ) {
                    alt16=1;
                }


                switch (alt16) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1325:3: RULE_IINT
            	    {
            	    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2768); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1342:1: rule__TSTRING__Group__0 : rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 ;
    public final void rule__TSTRING__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1346:1: ( rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1347:2: rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__02807);
            rule__TSTRING__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__02810);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1354:1: rule__TSTRING__Group__0__Impl : ( ( rule__TSTRING__Group_0__0 ) ) ;
    public final void rule__TSTRING__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1358:1: ( ( ( rule__TSTRING__Group_0__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1359:1: ( ( rule__TSTRING__Group_0__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1359:1: ( ( rule__TSTRING__Group_0__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1360:1: ( rule__TSTRING__Group_0__0 )
            {
             before(grammarAccess.getTSTRINGAccess().getGroup_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1361:1: ( rule__TSTRING__Group_0__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1361:2: rule__TSTRING__Group_0__0
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__0_in_rule__TSTRING__Group__0__Impl2837);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1371:1: rule__TSTRING__Group__1 : rule__TSTRING__Group__1__Impl ;
    public final void rule__TSTRING__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1375:1: ( rule__TSTRING__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1376:2: rule__TSTRING__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__12867);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1382:1: rule__TSTRING__Group__1__Impl : ( RULE_STRING ) ;
    public final void rule__TSTRING__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1386:1: ( ( RULE_STRING ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1387:1: ( RULE_STRING )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1387:1: ( RULE_STRING )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1388:1: RULE_STRING
            {
             before(grammarAccess.getTSTRINGAccess().getSTRINGTerminalRuleCall_1()); 
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl2894); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1403:1: rule__TSTRING__Group_0__0 : rule__TSTRING__Group_0__0__Impl rule__TSTRING__Group_0__1 ;
    public final void rule__TSTRING__Group_0__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1407:1: ( rule__TSTRING__Group_0__0__Impl rule__TSTRING__Group_0__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1408:2: rule__TSTRING__Group_0__0__Impl rule__TSTRING__Group_0__1
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__0__Impl_in_rule__TSTRING__Group_0__02927);
            rule__TSTRING__Group_0__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group_0__1_in_rule__TSTRING__Group_0__02930);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1415:1: rule__TSTRING__Group_0__0__Impl : ( ( ' ' )? ) ;
    public final void rule__TSTRING__Group_0__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1419:1: ( ( ( ' ' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1420:1: ( ( ' ' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1420:1: ( ( ' ' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1421:1: ( ' ' )?
            {
             before(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1422:1: ( ' ' )?
            int alt17=2;
            int LA17_0 = input.LA(1);

            if ( (LA17_0==30) ) {
                alt17=1;
            }
            switch (alt17) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1423:2: ' '
                    {
                    match(input,30,FOLLOW_30_in_rule__TSTRING__Group_0__0__Impl2959); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1434:1: rule__TSTRING__Group_0__1 : rule__TSTRING__Group_0__1__Impl rule__TSTRING__Group_0__2 ;
    public final void rule__TSTRING__Group_0__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1438:1: ( rule__TSTRING__Group_0__1__Impl rule__TSTRING__Group_0__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1439:2: rule__TSTRING__Group_0__1__Impl rule__TSTRING__Group_0__2
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__1__Impl_in_rule__TSTRING__Group_0__12992);
            rule__TSTRING__Group_0__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group_0__2_in_rule__TSTRING__Group_0__12995);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1446:1: rule__TSTRING__Group_0__1__Impl : ( '_' ) ;
    public final void rule__TSTRING__Group_0__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1450:1: ( ( '_' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1451:1: ( '_' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1451:1: ( '_' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1452:1: '_'
            {
             before(grammarAccess.getTSTRINGAccess().get_Keyword_0_1()); 
            match(input,9,FOLLOW_9_in_rule__TSTRING__Group_0__1__Impl3023); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1465:1: rule__TSTRING__Group_0__2 : rule__TSTRING__Group_0__2__Impl ;
    public final void rule__TSTRING__Group_0__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1469:1: ( rule__TSTRING__Group_0__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1470:2: rule__TSTRING__Group_0__2__Impl
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__2__Impl_in_rule__TSTRING__Group_0__23054);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1476:1: rule__TSTRING__Group_0__2__Impl : ( ( ' ' )? ) ;
    public final void rule__TSTRING__Group_0__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1480:1: ( ( ( ' ' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1481:1: ( ( ' ' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1481:1: ( ( ' ' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1482:1: ( ' ' )?
            {
             before(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1483:1: ( ' ' )?
            int alt18=2;
            int LA18_0 = input.LA(1);

            if ( (LA18_0==30) ) {
                alt18=1;
            }
            switch (alt18) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1484:2: ' '
                    {
                    match(input,30,FOLLOW_30_in_rule__TSTRING__Group_0__2__Impl3083); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1501:1: rule__PATH__Group__0 : rule__PATH__Group__0__Impl rule__PATH__Group__1 ;
    public final void rule__PATH__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1505:1: ( rule__PATH__Group__0__Impl rule__PATH__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1506:2: rule__PATH__Group__0__Impl rule__PATH__Group__1
            {
            pushFollow(FOLLOW_rule__PATH__Group__0__Impl_in_rule__PATH__Group__03122);
            rule__PATH__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group__1_in_rule__PATH__Group__03125);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1513:1: rule__PATH__Group__0__Impl : ( ( rule__PATH__Group_0__0 )* ) ;
    public final void rule__PATH__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1517:1: ( ( ( rule__PATH__Group_0__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1518:1: ( ( rule__PATH__Group_0__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1518:1: ( ( rule__PATH__Group_0__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1519:1: ( rule__PATH__Group_0__0 )*
            {
             before(grammarAccess.getPATHAccess().getGroup_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1520:1: ( rule__PATH__Group_0__0 )*
            loop19:
            do {
                int alt19=2;
                alt19 = dfa19.predict(input);
                switch (alt19) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1520:2: rule__PATH__Group_0__0
            	    {
            	    pushFollow(FOLLOW_rule__PATH__Group_0__0_in_rule__PATH__Group__0__Impl3152);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1530:1: rule__PATH__Group__1 : rule__PATH__Group__1__Impl rule__PATH__Group__2 ;
    public final void rule__PATH__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1534:1: ( rule__PATH__Group__1__Impl rule__PATH__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1535:2: rule__PATH__Group__1__Impl rule__PATH__Group__2
            {
            pushFollow(FOLLOW_rule__PATH__Group__1__Impl_in_rule__PATH__Group__13183);
            rule__PATH__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group__2_in_rule__PATH__Group__13186);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1542:1: rule__PATH__Group__1__Impl : ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) ;
    public final void rule__PATH__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1546:1: ( ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1547:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1547:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1548:1: ( ( RULE_ID ) ) ( ( RULE_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1548:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1549:1: ( RULE_ID )
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1550:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1550:3: RULE_ID
            {
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl3216); 

            }

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1553:1: ( ( RULE_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1554:1: ( RULE_ID )*
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1555:1: ( RULE_ID )*
            loop20:
            do {
                int alt20=2;
                int LA20_0 = input.LA(1);

                if ( (LA20_0==RULE_ID) ) {
                    alt20=1;
                }


                switch (alt20) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1555:3: RULE_ID
            	    {
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl3229); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1566:1: rule__PATH__Group__2 : rule__PATH__Group__2__Impl rule__PATH__Group__3 ;
    public final void rule__PATH__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1570:1: ( rule__PATH__Group__2__Impl rule__PATH__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1571:2: rule__PATH__Group__2__Impl rule__PATH__Group__3
            {
            pushFollow(FOLLOW_rule__PATH__Group__2__Impl_in_rule__PATH__Group__23262);
            rule__PATH__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group__3_in_rule__PATH__Group__23265);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1578:1: rule__PATH__Group__2__Impl : ( '.' ) ;
    public final void rule__PATH__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1582:1: ( ( '.' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1583:1: ( '.' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1583:1: ( '.' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1584:1: '.'
            {
             before(grammarAccess.getPATHAccess().getFullStopKeyword_2()); 
            match(input,12,FOLLOW_12_in_rule__PATH__Group__2__Impl3293); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1597:1: rule__PATH__Group__3 : rule__PATH__Group__3__Impl ;
    public final void rule__PATH__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1601:1: ( rule__PATH__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1602:2: rule__PATH__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__PATH__Group__3__Impl_in_rule__PATH__Group__33324);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1608:1: rule__PATH__Group__3__Impl : ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) ;
    public final void rule__PATH__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1612:1: ( ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1613:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1613:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1614:1: ( ( RULE_ID ) ) ( ( RULE_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1614:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1615:1: ( RULE_ID )
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1616:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1616:3: RULE_ID
            {
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl3354); 

            }

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1619:1: ( ( RULE_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1620:1: ( RULE_ID )*
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1621:1: ( RULE_ID )*
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
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1621:3: RULE_ID
            	    {
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl3367); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1640:1: rule__PATH__Group_0__0 : rule__PATH__Group_0__0__Impl rule__PATH__Group_0__1 ;
    public final void rule__PATH__Group_0__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1644:1: ( rule__PATH__Group_0__0__Impl rule__PATH__Group_0__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1645:2: rule__PATH__Group_0__0__Impl rule__PATH__Group_0__1
            {
            pushFollow(FOLLOW_rule__PATH__Group_0__0__Impl_in_rule__PATH__Group_0__03408);
            rule__PATH__Group_0__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group_0__1_in_rule__PATH__Group_0__03411);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1652:1: rule__PATH__Group_0__0__Impl : ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) ;
    public final void rule__PATH__Group_0__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1656:1: ( ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1657:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1657:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1658:1: ( ( RULE_ID ) ) ( ( RULE_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1658:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1659:1: ( RULE_ID )
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1660:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1660:3: RULE_ID
            {
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl3441); 

            }

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1663:1: ( ( RULE_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1664:1: ( RULE_ID )*
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1665:1: ( RULE_ID )*
            loop22:
            do {
                int alt22=2;
                int LA22_0 = input.LA(1);

                if ( (LA22_0==RULE_ID) ) {
                    alt22=1;
                }


                switch (alt22) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1665:3: RULE_ID
            	    {
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl3454); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1676:1: rule__PATH__Group_0__1 : rule__PATH__Group_0__1__Impl ;
    public final void rule__PATH__Group_0__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1680:1: ( rule__PATH__Group_0__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1681:2: rule__PATH__Group_0__1__Impl
            {
            pushFollow(FOLLOW_rule__PATH__Group_0__1__Impl_in_rule__PATH__Group_0__13487);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1687:1: rule__PATH__Group_0__1__Impl : ( ( rule__PATH__Alternatives_0_1 ) ) ;
    public final void rule__PATH__Group_0__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1691:1: ( ( ( rule__PATH__Alternatives_0_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1692:1: ( ( rule__PATH__Alternatives_0_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1692:1: ( ( rule__PATH__Alternatives_0_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1693:1: ( rule__PATH__Alternatives_0_1 )
            {
             before(grammarAccess.getPATHAccess().getAlternatives_0_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1694:1: ( rule__PATH__Alternatives_0_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1694:2: rule__PATH__Alternatives_0_1
            {
            pushFollow(FOLLOW_rule__PATH__Alternatives_0_1_in_rule__PATH__Group_0__1__Impl3514);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1708:1: rule__DIRECTION__Group__0 : rule__DIRECTION__Group__0__Impl rule__DIRECTION__Group__1 ;
    public final void rule__DIRECTION__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1712:1: ( rule__DIRECTION__Group__0__Impl rule__DIRECTION__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1713:2: rule__DIRECTION__Group__0__Impl rule__DIRECTION__Group__1
            {
            pushFollow(FOLLOW_rule__DIRECTION__Group__0__Impl_in_rule__DIRECTION__Group__03548);
            rule__DIRECTION__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__DIRECTION__Group__1_in_rule__DIRECTION__Group__03551);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1720:1: rule__DIRECTION__Group__0__Impl : ( ( rule__DIRECTION__Alternatives_0 ) ) ;
    public final void rule__DIRECTION__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1724:1: ( ( ( rule__DIRECTION__Alternatives_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1725:1: ( ( rule__DIRECTION__Alternatives_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1725:1: ( ( rule__DIRECTION__Alternatives_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1726:1: ( rule__DIRECTION__Alternatives_0 )
            {
             before(grammarAccess.getDIRECTIONAccess().getAlternatives_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1727:1: ( rule__DIRECTION__Alternatives_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1727:2: rule__DIRECTION__Alternatives_0
            {
            pushFollow(FOLLOW_rule__DIRECTION__Alternatives_0_in_rule__DIRECTION__Group__0__Impl3578);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1737:1: rule__DIRECTION__Group__1 : rule__DIRECTION__Group__1__Impl ;
    public final void rule__DIRECTION__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1741:1: ( rule__DIRECTION__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1742:2: rule__DIRECTION__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__DIRECTION__Group__1__Impl_in_rule__DIRECTION__Group__13608);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1748:1: rule__DIRECTION__Group__1__Impl : ( ( ',' )? ) ;
    public final void rule__DIRECTION__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1752:1: ( ( ( ',' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1753:1: ( ( ',' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1753:1: ( ( ',' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1754:1: ( ',' )?
            {
             before(grammarAccess.getDIRECTIONAccess().getCommaKeyword_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1755:1: ( ',' )?
            int alt23=2;
            int LA23_0 = input.LA(1);

            if ( (LA23_0==31) ) {
                alt23=1;
            }
            switch (alt23) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1756:2: ','
                    {
                    match(input,31,FOLLOW_31_in_rule__DIRECTION__Group__1__Impl3637); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1771:1: rule__LIST__Group__0 : rule__LIST__Group__0__Impl rule__LIST__Group__1 ;
    public final void rule__LIST__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1775:1: ( rule__LIST__Group__0__Impl rule__LIST__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1776:2: rule__LIST__Group__0__Impl rule__LIST__Group__1
            {
            pushFollow(FOLLOW_rule__LIST__Group__0__Impl_in_rule__LIST__Group__03674);
            rule__LIST__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__LIST__Group__1_in_rule__LIST__Group__03677);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1783:1: rule__LIST__Group__0__Impl : ( RULE_ID ) ;
    public final void rule__LIST__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1787:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1788:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1788:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1789:1: RULE_ID
            {
             before(grammarAccess.getLISTAccess().getIDTerminalRuleCall_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__LIST__Group__0__Impl3704); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1800:1: rule__LIST__Group__1 : rule__LIST__Group__1__Impl ;
    public final void rule__LIST__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1804:1: ( rule__LIST__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1805:2: rule__LIST__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__LIST__Group__1__Impl_in_rule__LIST__Group__13733);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1811:1: rule__LIST__Group__1__Impl : ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) ) ;
    public final void rule__LIST__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1815:1: ( ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1816:1: ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1816:1: ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1817:1: ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1817:1: ( ( rule__LIST__Group_1__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1818:1: ( rule__LIST__Group_1__0 )
            {
             before(grammarAccess.getLISTAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1819:1: ( rule__LIST__Group_1__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1819:2: rule__LIST__Group_1__0
            {
            pushFollow(FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl3762);
            rule__LIST__Group_1__0();
            _fsp--;


            }

             after(grammarAccess.getLISTAccess().getGroup_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1822:1: ( ( rule__LIST__Group_1__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1823:1: ( rule__LIST__Group_1__0 )*
            {
             before(grammarAccess.getLISTAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1824:1: ( rule__LIST__Group_1__0 )*
            loop24:
            do {
                int alt24=2;
                int LA24_0 = input.LA(1);

                if ( (LA24_0==31) ) {
                    alt24=1;
                }


                switch (alt24) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1824:2: rule__LIST__Group_1__0
            	    {
            	    pushFollow(FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl3774);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1839:1: rule__LIST__Group_1__0 : rule__LIST__Group_1__0__Impl rule__LIST__Group_1__1 ;
    public final void rule__LIST__Group_1__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1843:1: ( rule__LIST__Group_1__0__Impl rule__LIST__Group_1__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1844:2: rule__LIST__Group_1__0__Impl rule__LIST__Group_1__1
            {
            pushFollow(FOLLOW_rule__LIST__Group_1__0__Impl_in_rule__LIST__Group_1__03811);
            rule__LIST__Group_1__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__LIST__Group_1__1_in_rule__LIST__Group_1__03814);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1851:1: rule__LIST__Group_1__0__Impl : ( ',' ) ;
    public final void rule__LIST__Group_1__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1855:1: ( ( ',' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1856:1: ( ',' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1856:1: ( ',' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1857:1: ','
            {
             before(grammarAccess.getLISTAccess().getCommaKeyword_1_0()); 
            match(input,31,FOLLOW_31_in_rule__LIST__Group_1__0__Impl3842); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1870:1: rule__LIST__Group_1__1 : rule__LIST__Group_1__1__Impl ;
    public final void rule__LIST__Group_1__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1874:1: ( rule__LIST__Group_1__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1875:2: rule__LIST__Group_1__1__Impl
            {
            pushFollow(FOLLOW_rule__LIST__Group_1__1__Impl_in_rule__LIST__Group_1__13873);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1881:1: rule__LIST__Group_1__1__Impl : ( RULE_ID ) ;
    public final void rule__LIST__Group_1__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1885:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1886:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1886:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1887:1: RULE_ID
            {
             before(grammarAccess.getLISTAccess().getIDTerminalRuleCall_1_1()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__LIST__Group_1__1__Impl3900); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1902:1: rule__PROGRESSIVE__Group__0 : rule__PROGRESSIVE__Group__0__Impl rule__PROGRESSIVE__Group__1 ;
    public final void rule__PROGRESSIVE__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1906:1: ( rule__PROGRESSIVE__Group__0__Impl rule__PROGRESSIVE__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1907:2: rule__PROGRESSIVE__Group__0__Impl rule__PROGRESSIVE__Group__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__0__Impl_in_rule__PROGRESSIVE__Group__03933);
            rule__PROGRESSIVE__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__1_in_rule__PROGRESSIVE__Group__03936);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1914:1: rule__PROGRESSIVE__Group__0__Impl : ( ( rule__PROGRESSIVE__Alternatives_0 ) ) ;
    public final void rule__PROGRESSIVE__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1918:1: ( ( ( rule__PROGRESSIVE__Alternatives_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1919:1: ( ( rule__PROGRESSIVE__Alternatives_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1919:1: ( ( rule__PROGRESSIVE__Alternatives_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1920:1: ( rule__PROGRESSIVE__Alternatives_0 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1921:1: ( rule__PROGRESSIVE__Alternatives_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1921:2: rule__PROGRESSIVE__Alternatives_0
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_0_in_rule__PROGRESSIVE__Group__0__Impl3963);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1931:1: rule__PROGRESSIVE__Group__1 : rule__PROGRESSIVE__Group__1__Impl rule__PROGRESSIVE__Group__2 ;
    public final void rule__PROGRESSIVE__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1935:1: ( rule__PROGRESSIVE__Group__1__Impl rule__PROGRESSIVE__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1936:2: rule__PROGRESSIVE__Group__1__Impl rule__PROGRESSIVE__Group__2
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__1__Impl_in_rule__PROGRESSIVE__Group__13993);
            rule__PROGRESSIVE__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__2_in_rule__PROGRESSIVE__Group__13996);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1943:1: rule__PROGRESSIVE__Group__1__Impl : ( ( rule__PROGRESSIVE__Group_1__0 )? ) ;
    public final void rule__PROGRESSIVE__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1947:1: ( ( ( rule__PROGRESSIVE__Group_1__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1948:1: ( ( rule__PROGRESSIVE__Group_1__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1948:1: ( ( rule__PROGRESSIVE__Group_1__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1949:1: ( rule__PROGRESSIVE__Group_1__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1950:1: ( rule__PROGRESSIVE__Group_1__0 )?
            int alt25=2;
            int LA25_0 = input.LA(1);

            if ( (LA25_0==32) ) {
                alt25=1;
            }
            switch (alt25) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1950:2: rule__PROGRESSIVE__Group_1__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__0_in_rule__PROGRESSIVE__Group__1__Impl4023);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1960:1: rule__PROGRESSIVE__Group__2 : rule__PROGRESSIVE__Group__2__Impl rule__PROGRESSIVE__Group__3 ;
    public final void rule__PROGRESSIVE__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1964:1: ( rule__PROGRESSIVE__Group__2__Impl rule__PROGRESSIVE__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1965:2: rule__PROGRESSIVE__Group__2__Impl rule__PROGRESSIVE__Group__3
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__2__Impl_in_rule__PROGRESSIVE__Group__24054);
            rule__PROGRESSIVE__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__3_in_rule__PROGRESSIVE__Group__24057);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1972:1: rule__PROGRESSIVE__Group__2__Impl : ( ( rule__PROGRESSIVE__Group_2__0 )? ) ;
    public final void rule__PROGRESSIVE__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1976:1: ( ( ( rule__PROGRESSIVE__Group_2__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1977:1: ( ( rule__PROGRESSIVE__Group_2__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1977:1: ( ( rule__PROGRESSIVE__Group_2__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1978:1: ( rule__PROGRESSIVE__Group_2__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1979:1: ( rule__PROGRESSIVE__Group_2__0 )?
            int alt26=2;
            int LA26_0 = input.LA(1);

            if ( (LA26_0==10) ) {
                alt26=1;
            }
            switch (alt26) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1979:2: rule__PROGRESSIVE__Group_2__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__0_in_rule__PROGRESSIVE__Group__2__Impl4084);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1989:1: rule__PROGRESSIVE__Group__3 : rule__PROGRESSIVE__Group__3__Impl ;
    public final void rule__PROGRESSIVE__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1993:1: ( rule__PROGRESSIVE__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1994:2: rule__PROGRESSIVE__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__3__Impl_in_rule__PROGRESSIVE__Group__34115);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2000:1: rule__PROGRESSIVE__Group__3__Impl : ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) ) ;
    public final void rule__PROGRESSIVE__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2004:1: ( ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2005:1: ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2005:1: ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2006:1: ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2006:1: ( ( rule__PROGRESSIVE__Group_3__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2007:1: ( rule__PROGRESSIVE__Group_3__0 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2008:1: ( rule__PROGRESSIVE__Group_3__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2008:2: rule__PROGRESSIVE__Group_3__0
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl4144);
            rule__PROGRESSIVE__Group_3__0();
            _fsp--;


            }

             after(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2011:1: ( ( rule__PROGRESSIVE__Group_3__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2012:1: ( rule__PROGRESSIVE__Group_3__0 )*
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2013:1: ( rule__PROGRESSIVE__Group_3__0 )*
            loop27:
            do {
                int alt27=2;
                int LA27_0 = input.LA(1);

                if ( (LA27_0==31) ) {
                    alt27=1;
                }


                switch (alt27) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2013:2: rule__PROGRESSIVE__Group_3__0
            	    {
            	    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl4156);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2032:1: rule__PROGRESSIVE__Group_1__0 : rule__PROGRESSIVE__Group_1__0__Impl rule__PROGRESSIVE__Group_1__1 ;
    public final void rule__PROGRESSIVE__Group_1__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2036:1: ( rule__PROGRESSIVE__Group_1__0__Impl rule__PROGRESSIVE__Group_1__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2037:2: rule__PROGRESSIVE__Group_1__0__Impl rule__PROGRESSIVE__Group_1__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__0__Impl_in_rule__PROGRESSIVE__Group_1__04197);
            rule__PROGRESSIVE__Group_1__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__1_in_rule__PROGRESSIVE__Group_1__04200);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2044:1: rule__PROGRESSIVE__Group_1__0__Impl : ( '~' ) ;
    public final void rule__PROGRESSIVE__Group_1__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2048:1: ( ( '~' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2049:1: ( '~' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2049:1: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2050:1: '~'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_1_0()); 
            match(input,32,FOLLOW_32_in_rule__PROGRESSIVE__Group_1__0__Impl4228); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2063:1: rule__PROGRESSIVE__Group_1__1 : rule__PROGRESSIVE__Group_1__1__Impl ;
    public final void rule__PROGRESSIVE__Group_1__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2067:1: ( rule__PROGRESSIVE__Group_1__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2068:2: rule__PROGRESSIVE__Group_1__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__1__Impl_in_rule__PROGRESSIVE__Group_1__14259);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2074:1: rule__PROGRESSIVE__Group_1__1__Impl : ( ( rule__PROGRESSIVE__Alternatives_1_1 ) ) ;
    public final void rule__PROGRESSIVE__Group_1__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2078:1: ( ( ( rule__PROGRESSIVE__Alternatives_1_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2079:1: ( ( rule__PROGRESSIVE__Alternatives_1_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2079:1: ( ( rule__PROGRESSIVE__Alternatives_1_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2080:1: ( rule__PROGRESSIVE__Alternatives_1_1 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_1_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2081:1: ( rule__PROGRESSIVE__Alternatives_1_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2081:2: rule__PROGRESSIVE__Alternatives_1_1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_1_1_in_rule__PROGRESSIVE__Group_1__1__Impl4286);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2095:1: rule__PROGRESSIVE__Group_2__0 : rule__PROGRESSIVE__Group_2__0__Impl rule__PROGRESSIVE__Group_2__1 ;
    public final void rule__PROGRESSIVE__Group_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2099:1: ( rule__PROGRESSIVE__Group_2__0__Impl rule__PROGRESSIVE__Group_2__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2100:2: rule__PROGRESSIVE__Group_2__0__Impl rule__PROGRESSIVE__Group_2__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__0__Impl_in_rule__PROGRESSIVE__Group_2__04320);
            rule__PROGRESSIVE__Group_2__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__1_in_rule__PROGRESSIVE__Group_2__04323);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2107:1: rule__PROGRESSIVE__Group_2__0__Impl : ( ':' ) ;
    public final void rule__PROGRESSIVE__Group_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2111:1: ( ( ':' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2112:1: ( ':' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2112:1: ( ':' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2113:1: ':'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_2_0()); 
            match(input,10,FOLLOW_10_in_rule__PROGRESSIVE__Group_2__0__Impl4351); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2126:1: rule__PROGRESSIVE__Group_2__1 : rule__PROGRESSIVE__Group_2__1__Impl ;
    public final void rule__PROGRESSIVE__Group_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2130:1: ( rule__PROGRESSIVE__Group_2__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2131:2: rule__PROGRESSIVE__Group_2__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__1__Impl_in_rule__PROGRESSIVE__Group_2__14382);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2137:1: rule__PROGRESSIVE__Group_2__1__Impl : ( RULE_IINT ) ;
    public final void rule__PROGRESSIVE__Group_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2141:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2142:1: ( RULE_IINT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2142:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2143:1: RULE_IINT
            {
             before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_2_1()); 
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_2__1__Impl4409); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2158:1: rule__PROGRESSIVE__Group_3__0 : rule__PROGRESSIVE__Group_3__0__Impl rule__PROGRESSIVE__Group_3__1 ;
    public final void rule__PROGRESSIVE__Group_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2162:1: ( rule__PROGRESSIVE__Group_3__0__Impl rule__PROGRESSIVE__Group_3__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2163:2: rule__PROGRESSIVE__Group_3__0__Impl rule__PROGRESSIVE__Group_3__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__0__Impl_in_rule__PROGRESSIVE__Group_3__04442);
            rule__PROGRESSIVE__Group_3__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__1_in_rule__PROGRESSIVE__Group_3__04445);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2170:1: rule__PROGRESSIVE__Group_3__0__Impl : ( ',' ) ;
    public final void rule__PROGRESSIVE__Group_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2174:1: ( ( ',' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2175:1: ( ',' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2175:1: ( ',' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2176:1: ','
            {
             before(grammarAccess.getPROGRESSIVEAccess().getCommaKeyword_3_0()); 
            match(input,31,FOLLOW_31_in_rule__PROGRESSIVE__Group_3__0__Impl4473); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2189:1: rule__PROGRESSIVE__Group_3__1 : rule__PROGRESSIVE__Group_3__1__Impl rule__PROGRESSIVE__Group_3__2 ;
    public final void rule__PROGRESSIVE__Group_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2193:1: ( rule__PROGRESSIVE__Group_3__1__Impl rule__PROGRESSIVE__Group_3__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2194:2: rule__PROGRESSIVE__Group_3__1__Impl rule__PROGRESSIVE__Group_3__2
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__1__Impl_in_rule__PROGRESSIVE__Group_3__14504);
            rule__PROGRESSIVE__Group_3__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__2_in_rule__PROGRESSIVE__Group_3__14507);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2201:1: rule__PROGRESSIVE__Group_3__1__Impl : ( ( rule__PROGRESSIVE__Alternatives_3_1 ) ) ;
    public final void rule__PROGRESSIVE__Group_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2205:1: ( ( ( rule__PROGRESSIVE__Alternatives_3_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2206:1: ( ( rule__PROGRESSIVE__Alternatives_3_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2206:1: ( ( rule__PROGRESSIVE__Alternatives_3_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2207:1: ( rule__PROGRESSIVE__Alternatives_3_1 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_3_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2208:1: ( rule__PROGRESSIVE__Alternatives_3_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2208:2: rule__PROGRESSIVE__Alternatives_3_1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_3_1_in_rule__PROGRESSIVE__Group_3__1__Impl4534);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2218:1: rule__PROGRESSIVE__Group_3__2 : rule__PROGRESSIVE__Group_3__2__Impl rule__PROGRESSIVE__Group_3__3 ;
    public final void rule__PROGRESSIVE__Group_3__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2222:1: ( rule__PROGRESSIVE__Group_3__2__Impl rule__PROGRESSIVE__Group_3__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2223:2: rule__PROGRESSIVE__Group_3__2__Impl rule__PROGRESSIVE__Group_3__3
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__2__Impl_in_rule__PROGRESSIVE__Group_3__24564);
            rule__PROGRESSIVE__Group_3__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__3_in_rule__PROGRESSIVE__Group_3__24567);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2230:1: rule__PROGRESSIVE__Group_3__2__Impl : ( ( rule__PROGRESSIVE__Group_3_2__0 )? ) ;
    public final void rule__PROGRESSIVE__Group_3__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2234:1: ( ( ( rule__PROGRESSIVE__Group_3_2__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2235:1: ( ( rule__PROGRESSIVE__Group_3_2__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2235:1: ( ( rule__PROGRESSIVE__Group_3_2__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2236:1: ( rule__PROGRESSIVE__Group_3_2__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2237:1: ( rule__PROGRESSIVE__Group_3_2__0 )?
            int alt28=2;
            int LA28_0 = input.LA(1);

            if ( (LA28_0==32) ) {
                alt28=1;
            }
            switch (alt28) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2237:2: rule__PROGRESSIVE__Group_3_2__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__0_in_rule__PROGRESSIVE__Group_3__2__Impl4594);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2247:1: rule__PROGRESSIVE__Group_3__3 : rule__PROGRESSIVE__Group_3__3__Impl ;
    public final void rule__PROGRESSIVE__Group_3__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2251:1: ( rule__PROGRESSIVE__Group_3__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2252:2: rule__PROGRESSIVE__Group_3__3__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__3__Impl_in_rule__PROGRESSIVE__Group_3__34625);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2258:1: rule__PROGRESSIVE__Group_3__3__Impl : ( ( rule__PROGRESSIVE__Group_3_3__0 )? ) ;
    public final void rule__PROGRESSIVE__Group_3__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2262:1: ( ( ( rule__PROGRESSIVE__Group_3_3__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2263:1: ( ( rule__PROGRESSIVE__Group_3_3__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2263:1: ( ( rule__PROGRESSIVE__Group_3_3__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2264:1: ( rule__PROGRESSIVE__Group_3_3__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2265:1: ( rule__PROGRESSIVE__Group_3_3__0 )?
            int alt29=2;
            int LA29_0 = input.LA(1);

            if ( (LA29_0==10) ) {
                alt29=1;
            }
            switch (alt29) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2265:2: rule__PROGRESSIVE__Group_3_3__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__0_in_rule__PROGRESSIVE__Group_3__3__Impl4652);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2283:1: rule__PROGRESSIVE__Group_3_2__0 : rule__PROGRESSIVE__Group_3_2__0__Impl rule__PROGRESSIVE__Group_3_2__1 ;
    public final void rule__PROGRESSIVE__Group_3_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2287:1: ( rule__PROGRESSIVE__Group_3_2__0__Impl rule__PROGRESSIVE__Group_3_2__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2288:2: rule__PROGRESSIVE__Group_3_2__0__Impl rule__PROGRESSIVE__Group_3_2__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__0__Impl_in_rule__PROGRESSIVE__Group_3_2__04691);
            rule__PROGRESSIVE__Group_3_2__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__1_in_rule__PROGRESSIVE__Group_3_2__04694);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2295:1: rule__PROGRESSIVE__Group_3_2__0__Impl : ( '~' ) ;
    public final void rule__PROGRESSIVE__Group_3_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2299:1: ( ( '~' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2300:1: ( '~' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2300:1: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2301:1: '~'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_3_2_0()); 
            match(input,32,FOLLOW_32_in_rule__PROGRESSIVE__Group_3_2__0__Impl4722); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2314:1: rule__PROGRESSIVE__Group_3_2__1 : rule__PROGRESSIVE__Group_3_2__1__Impl ;
    public final void rule__PROGRESSIVE__Group_3_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2318:1: ( rule__PROGRESSIVE__Group_3_2__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2319:2: rule__PROGRESSIVE__Group_3_2__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__1__Impl_in_rule__PROGRESSIVE__Group_3_2__14753);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2325:1: rule__PROGRESSIVE__Group_3_2__1__Impl : ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) ) ;
    public final void rule__PROGRESSIVE__Group_3_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2329:1: ( ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2330:1: ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2330:1: ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2331:1: ( rule__PROGRESSIVE__Alternatives_3_2_1 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_3_2_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2332:1: ( rule__PROGRESSIVE__Alternatives_3_2_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2332:2: rule__PROGRESSIVE__Alternatives_3_2_1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_3_2_1_in_rule__PROGRESSIVE__Group_3_2__1__Impl4780);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2346:1: rule__PROGRESSIVE__Group_3_3__0 : rule__PROGRESSIVE__Group_3_3__0__Impl rule__PROGRESSIVE__Group_3_3__1 ;
    public final void rule__PROGRESSIVE__Group_3_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2350:1: ( rule__PROGRESSIVE__Group_3_3__0__Impl rule__PROGRESSIVE__Group_3_3__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2351:2: rule__PROGRESSIVE__Group_3_3__0__Impl rule__PROGRESSIVE__Group_3_3__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__0__Impl_in_rule__PROGRESSIVE__Group_3_3__04814);
            rule__PROGRESSIVE__Group_3_3__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__1_in_rule__PROGRESSIVE__Group_3_3__04817);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2358:1: rule__PROGRESSIVE__Group_3_3__0__Impl : ( ':' ) ;
    public final void rule__PROGRESSIVE__Group_3_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2362:1: ( ( ':' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2363:1: ( ':' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2363:1: ( ':' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2364:1: ':'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_3_3_0()); 
            match(input,10,FOLLOW_10_in_rule__PROGRESSIVE__Group_3_3__0__Impl4845); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2377:1: rule__PROGRESSIVE__Group_3_3__1 : rule__PROGRESSIVE__Group_3_3__1__Impl ;
    public final void rule__PROGRESSIVE__Group_3_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2381:1: ( rule__PROGRESSIVE__Group_3_3__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2382:2: rule__PROGRESSIVE__Group_3_3__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__1__Impl_in_rule__PROGRESSIVE__Group_3_3__14876);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2388:1: rule__PROGRESSIVE__Group_3_3__1__Impl : ( RULE_IINT ) ;
    public final void rule__PROGRESSIVE__Group_3_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2392:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2393:1: ( RULE_IINT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2393:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2394:1: RULE_IINT
            {
             before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_3_1()); 
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_3_3__1__Impl4903); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2410:1: rule__WMLRoot__RtagsAssignment_0 : ( ruleWMLTag ) ;
    public final void rule__WMLRoot__RtagsAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2414:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2415:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2415:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2416:1: ruleWMLTag
            {
             before(grammarAccess.getWMLRootAccess().getRtagsWMLTagParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRoot__RtagsAssignment_04941);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2425:1: rule__WMLRoot__RmacrosAssignment_1 : ( ruleWMLMacro ) ;
    public final void rule__WMLRoot__RmacrosAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2429:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2430:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2430:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2431:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLRootAccess().getRmacrosWMLMacroParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLRoot__RmacrosAssignment_14972);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2440:1: rule__WMLMacro__NameAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLMacro__NameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2444:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2445:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2445:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2446:1: RULE_ID
            {
             before(grammarAccess.getWMLMacroAccess().getNameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacro__NameAssignment_15003); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2455:1: rule__WMLMacro__ValueAssignment_2 : ( ( rule__WMLMacro__ValueAlternatives_2_0 ) ) ;
    public final void rule__WMLMacro__ValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2459:1: ( ( ( rule__WMLMacro__ValueAlternatives_2_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2460:1: ( ( rule__WMLMacro__ValueAlternatives_2_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2460:1: ( ( rule__WMLMacro__ValueAlternatives_2_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2461:1: ( rule__WMLMacro__ValueAlternatives_2_0 )
            {
             before(grammarAccess.getWMLMacroAccess().getValueAlternatives_2_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2462:1: ( rule__WMLMacro__ValueAlternatives_2_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2462:2: rule__WMLMacro__ValueAlternatives_2_0
            {
            pushFollow(FOLLOW_rule__WMLMacro__ValueAlternatives_2_0_in_rule__WMLMacro__ValueAssignment_25034);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2471:1: rule__WMLTag__NameAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLTag__NameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2475:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2476:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2476:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2477:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_15067); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2486:1: rule__WMLTag__TtagsAssignment_3_0 : ( ruleWMLTag ) ;
    public final void rule__WMLTag__TtagsAssignment_3_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2490:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2491:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2491:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2492:1: ruleWMLTag
            {
             before(grammarAccess.getWMLTagAccess().getTtagsWMLTagParserRuleCall_3_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLTag__TtagsAssignment_3_05098);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2501:1: rule__WMLTag__TkeysAssignment_3_1 : ( ruleWMLKey ) ;
    public final void rule__WMLTag__TkeysAssignment_3_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2505:1: ( ( ruleWMLKey ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2506:1: ( ruleWMLKey )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2506:1: ( ruleWMLKey )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2507:1: ruleWMLKey
            {
             before(grammarAccess.getWMLTagAccess().getTkeysWMLKeyParserRuleCall_3_1_0()); 
            pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLTag__TkeysAssignment_3_15129);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2516:1: rule__WMLTag__TmacrosAssignment_3_2 : ( ruleWMLMacro ) ;
    public final void rule__WMLTag__TmacrosAssignment_3_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2520:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2521:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2521:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2522:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLTagAccess().getTmacrosWMLMacroParserRuleCall_3_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLTag__TmacrosAssignment_3_25160);
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


    // $ANTLR start rule__WMLTag__EndNameAssignment_5
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2531:1: rule__WMLTag__EndNameAssignment_5 : ( RULE_ID ) ;
    public final void rule__WMLTag__EndNameAssignment_5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2535:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2536:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2536:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2537:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_5_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_55191); 
             after(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_5_0()); 

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
    // $ANTLR end rule__WMLTag__EndNameAssignment_5


    // $ANTLR start rule__WMLKey__KeyNameAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2546:1: rule__WMLKey__KeyNameAssignment_0 : ( RULE_ID ) ;
    public final void rule__WMLKey__KeyNameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2550:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2551:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2551:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2552:1: RULE_ID
            {
             before(grammarAccess.getWMLKeyAccess().getKeyNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__KeyNameAssignment_05222); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2561:1: rule__WMLKey__ValueAssignment_2 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2565:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2566:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2566:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2567:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_25253);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2576:1: rule__WMLKeyValue__Key1ValueAssignment_0 : ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) ) ;
    public final void rule__WMLKeyValue__Key1ValueAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2580:1: ( ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2581:1: ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2581:1: ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2582:1: ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 )
            {
             before(grammarAccess.getWMLKeyValueAccess().getKey1ValueAlternatives_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2583:1: ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2583:2: rule__WMLKeyValue__Key1ValueAlternatives_0_0
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Key1ValueAlternatives_0_0_in_rule__WMLKeyValue__Key1ValueAssignment_05284);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2592:1: rule__WMLKeyValue__Key2ValueAssignment_1 : ( ruleWMLMacro ) ;
    public final void rule__WMLKeyValue__Key2ValueAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2596:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2597:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2597:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2598:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLKeyValueAccess().getKey2ValueWMLMacroParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLKeyValue__Key2ValueAssignment_15317);
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
        "\1\uffff\1\6\2\uffff\1\12\10\uffff\2\17\1\uffff";
    static final String DFA7_minS =
        "\2\4\2\uffff\1\4\2\uffff\1\4\3\uffff\1\6\1\uffff\2\4\1\uffff";
    static final String DFA7_maxS =
        "\1\36\1\37\2\uffff\1\40\2\uffff\1\20\3\uffff\1\6\1\uffff\2\40\1"+
        "\uffff";
    static final String DFA7_acceptS =
        "\2\uffff\1\2\1\3\1\uffff\1\7\1\1\1\uffff\1\6\1\10\1\5\1\uffff\1"+
        "\11\2\uffff\1\4";
    static final String DFA7_specialS =
        "\20\uffff}>";
    static final String[] DFA7_transitionS = {
            "\1\1\1\2\1\4\2\uffff\1\3\7\uffff\10\5\5\uffff\1\3",
            "\1\7\6\uffff\2\10\3\uffff\1\10\10\uffff\1\6\1\uffff\1\6\1\uffff"+
            "\1\6\1\uffff\1\11",
            "",
            "",
            "\1\12\5\uffff\1\14\1\uffff\1\13\14\uffff\1\12\1\uffff\1\12\1"+
            "\uffff\1\12\1\uffff\2\14",
            "",
            "",
            "\1\10\6\uffff\2\10\2\uffff\1\6\1\10",
            "",
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
            return "542:1: rule__WMLKeyValue__Key1ValueAlternatives_0_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFLOAT ) | ( RULE_IINT ) | ( rulePATH ) | ( ruleDIRECTION ) | ( ruleLIST ) | ( rulePROGRESSIVE ) );";
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
        "\3\uffff\1\1\1\2";
    static final String DFA19_specialS =
        "\5\uffff}>";
    static final String[] DFA19_transitionS = {
            "\1\1",
            "\1\2\6\uffff\1\3\1\4\3\uffff\1\3",
            "\1\2\6\uffff\1\3\1\4\3\uffff\1\3",
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
            return "()* loopback of 1520:1: ( rule__PATH__Group_0__0 )*";
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
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey242 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey249 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey275 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue302 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue309 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue335 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_entryRuleFLOAT367 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleFLOAT374 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__0_in_ruleFLOAT404 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING431 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING438 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0_in_ruleTSTRING464 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_entryRulePATH491 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePATH498 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group__0_in_rulePATH524 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_entryRuleDIRECTION551 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleDIRECTION558 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION586 = new BitSet(new long[]{0x0000000001FE0002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION598 = new BitSet(new long[]{0x0000000001FE0002L});
    public static final BitSet FOLLOW_ruleLIST_in_entryRuleLIST628 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleLIST635 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group__0_in_ruleLIST661 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE688 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePROGRESSIVE695 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__0_in_rulePROGRESSIVE721 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__RtagsAssignment_0_in_rule__WMLRoot__Alternatives757 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__RmacrosAssignment_1_in_rule__WMLRoot__Alternatives775 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacro__ValueAlternatives_2_0808 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLMacro__ValueAlternatives_2_0825 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_9_in_rule__WMLMacro__ValueAlternatives_2_0843 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_rule__WMLMacro__ValueAlternatives_2_0863 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_rule__WMLMacro__ValueAlternatives_2_0883 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__WMLMacro__ValueAlternatives_2_0903 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__WMLMacro__ValueAlternatives_2_0923 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__WMLMacro__ValueAlternatives_2_0943 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLMacro__ValueAlternatives_2_0963 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__WMLMacro__ValueAlternatives_2_0983 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TtagsAssignment_3_0_in_rule__WMLTag__Alternatives_31017 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TkeysAssignment_3_1_in_rule__WMLTag__Alternatives_31035 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TmacrosAssignment_3_2_in_rule__WMLTag__Alternatives_31053 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Key1ValueAssignment_0_in_rule__WMLKeyValue__Alternatives1086 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Key2ValueAssignment_1_in_rule__WMLKeyValue__Alternatives1104 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01137 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01154 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01171 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01188 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01205 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01222 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01239 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleLIST_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01256 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01273 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_rule__PATH__Alternatives_0_11306 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__PATH__Alternatives_0_11326 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_rule__DIRECTION__Alternatives_01361 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_rule__DIRECTION__Alternatives_01381 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_rule__DIRECTION__Alternatives_01401 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__DIRECTION__Alternatives_01421 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__DIRECTION__Alternatives_01441 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__DIRECTION__Alternatives_01461 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__DIRECTION__Alternatives_01481 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_rule__DIRECTION__Alternatives_01501 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_01535 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_01552 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_1_11584 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_1_11601 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_11633 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_11650 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_2_11682 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_2_11699 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__0__Impl_in_rule__WMLMacro__Group__01729 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__1_in_rule__WMLMacro__Group__01732 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__WMLMacro__Group__0__Impl1760 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__1__Impl_in_rule__WMLMacro__Group__11791 = new BitSet(new long[]{0x000000000401FE30L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__2_in_rule__WMLMacro__Group__11794 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__NameAssignment_1_in_rule__WMLMacro__Group__1__Impl1821 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__2__Impl_in_rule__WMLMacro__Group__21851 = new BitSet(new long[]{0x0000000004000000L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__3_in_rule__WMLMacro__Group__21854 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__ValueAssignment_2_in_rule__WMLMacro__Group__2__Impl1881 = new BitSet(new long[]{0x000000000001FE32L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__3__Impl_in_rule__WMLMacro__Group__31912 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_rule__WMLMacro__Group__3__Impl1940 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01979 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01982 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rule__WMLTag__Group__0__Impl2010 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__12041 = new BitSet(new long[]{0x0000000010000000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__12044 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__NameAssignment_1_in_rule__WMLTag__Group__1__Impl2071 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__22101 = new BitSet(new long[]{0x000000002A000010L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__22104 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__WMLTag__Group__2__Impl2132 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__32163 = new BitSet(new long[]{0x0000000020000000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__32166 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Alternatives_3_in_rule__WMLTag__Group__3__Impl2193 = new BitSet(new long[]{0x000000000A000012L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__42224 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__42227 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__WMLTag__Group__4__Impl2255 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__52286 = new BitSet(new long[]{0x0000000010000000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__52289 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__EndNameAssignment_5_in_rule__WMLTag__Group__5__Impl2316 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__62346 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__WMLTag__Group__6__Impl2374 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02419 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02422 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__KeyNameAssignment_0_in_rule__WMLKey__Group__0__Impl2449 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12479 = new BitSet(new long[]{0x0000000043FE0270L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12482 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLKey__Group__1__Impl2510 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22541 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2568 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__0__Impl_in_rule__FLOAT__Group__02604 = new BitSet(new long[]{0x0000000000001000L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__1_in_rule__FLOAT__Group__02607 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__FLOAT__Group__0__Impl2634 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__1__Impl_in_rule__FLOAT__Group__12663 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__2_in_rule__FLOAT__Group__12666 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__FLOAT__Group__1__Impl2694 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__2__Impl_in_rule__FLOAT__Group__22725 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2755 = new BitSet(new long[]{0x0000000000000042L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2768 = new BitSet(new long[]{0x0000000000000042L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__02807 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__02810 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__0_in_rule__TSTRING__Group__0__Impl2837 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__12867 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl2894 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__0__Impl_in_rule__TSTRING__Group_0__02927 = new BitSet(new long[]{0x0000000000000200L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__1_in_rule__TSTRING__Group_0__02930 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__TSTRING__Group_0__0__Impl2959 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__1__Impl_in_rule__TSTRING__Group_0__12992 = new BitSet(new long[]{0x0000000040000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__2_in_rule__TSTRING__Group_0__12995 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_9_in_rule__TSTRING__Group_0__1__Impl3023 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__2__Impl_in_rule__TSTRING__Group_0__23054 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__TSTRING__Group_0__2__Impl3083 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group__0__Impl_in_rule__PATH__Group__03122 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__PATH__Group__1_in_rule__PATH__Group__03125 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__0_in_rule__PATH__Group__0__Impl3152 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group__1__Impl_in_rule__PATH__Group__13183 = new BitSet(new long[]{0x0000000000001000L});
    public static final BitSet FOLLOW_rule__PATH__Group__2_in_rule__PATH__Group__13186 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl3216 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl3229 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group__2__Impl_in_rule__PATH__Group__23262 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__PATH__Group__3_in_rule__PATH__Group__23265 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__PATH__Group__2__Impl3293 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group__3__Impl_in_rule__PATH__Group__33324 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl3354 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl3367 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__0__Impl_in_rule__PATH__Group_0__03408 = new BitSet(new long[]{0x0000000000010800L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__1_in_rule__PATH__Group_0__03411 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl3441 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl3454 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__1__Impl_in_rule__PATH__Group_0__13487 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Alternatives_0_1_in_rule__PATH__Group_0__1__Impl3514 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__0__Impl_in_rule__DIRECTION__Group__03548 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__1_in_rule__DIRECTION__Group__03551 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Alternatives_0_in_rule__DIRECTION__Group__0__Impl3578 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__1__Impl_in_rule__DIRECTION__Group__13608 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__DIRECTION__Group__1__Impl3637 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group__0__Impl_in_rule__LIST__Group__03674 = new BitSet(new long[]{0x0000000080000000L});
    public static final BitSet FOLLOW_rule__LIST__Group__1_in_rule__LIST__Group__03677 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__LIST__Group__0__Impl3704 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group__1__Impl_in_rule__LIST__Group__13733 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl3762 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl3774 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__0__Impl_in_rule__LIST__Group_1__03811 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__1_in_rule__LIST__Group_1__03814 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__LIST__Group_1__0__Impl3842 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__1__Impl_in_rule__LIST__Group_1__13873 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__LIST__Group_1__1__Impl3900 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__0__Impl_in_rule__PROGRESSIVE__Group__03933 = new BitSet(new long[]{0x0000000180000400L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__1_in_rule__PROGRESSIVE__Group__03936 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_0_in_rule__PROGRESSIVE__Group__0__Impl3963 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__1__Impl_in_rule__PROGRESSIVE__Group__13993 = new BitSet(new long[]{0x0000000080000400L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__2_in_rule__PROGRESSIVE__Group__13996 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__0_in_rule__PROGRESSIVE__Group__1__Impl4023 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__2__Impl_in_rule__PROGRESSIVE__Group__24054 = new BitSet(new long[]{0x0000000080000000L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__3_in_rule__PROGRESSIVE__Group__24057 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__0_in_rule__PROGRESSIVE__Group__2__Impl4084 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__3__Impl_in_rule__PROGRESSIVE__Group__34115 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl4144 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl4156 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__0__Impl_in_rule__PROGRESSIVE__Group_1__04197 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__1_in_rule__PROGRESSIVE__Group_1__04200 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__PROGRESSIVE__Group_1__0__Impl4228 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__1__Impl_in_rule__PROGRESSIVE__Group_1__14259 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_1_1_in_rule__PROGRESSIVE__Group_1__1__Impl4286 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__0__Impl_in_rule__PROGRESSIVE__Group_2__04320 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__1_in_rule__PROGRESSIVE__Group_2__04323 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_rule__PROGRESSIVE__Group_2__0__Impl4351 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__1__Impl_in_rule__PROGRESSIVE__Group_2__14382 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_2__1__Impl4409 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__0__Impl_in_rule__PROGRESSIVE__Group_3__04442 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__1_in_rule__PROGRESSIVE__Group_3__04445 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__PROGRESSIVE__Group_3__0__Impl4473 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__1__Impl_in_rule__PROGRESSIVE__Group_3__14504 = new BitSet(new long[]{0x0000000100000402L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__2_in_rule__PROGRESSIVE__Group_3__14507 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_3_1_in_rule__PROGRESSIVE__Group_3__1__Impl4534 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__2__Impl_in_rule__PROGRESSIVE__Group_3__24564 = new BitSet(new long[]{0x0000000000000402L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__3_in_rule__PROGRESSIVE__Group_3__24567 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__0_in_rule__PROGRESSIVE__Group_3__2__Impl4594 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__3__Impl_in_rule__PROGRESSIVE__Group_3__34625 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__0_in_rule__PROGRESSIVE__Group_3__3__Impl4652 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__0__Impl_in_rule__PROGRESSIVE__Group_3_2__04691 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__1_in_rule__PROGRESSIVE__Group_3_2__04694 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__PROGRESSIVE__Group_3_2__0__Impl4722 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__1__Impl_in_rule__PROGRESSIVE__Group_3_2__14753 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_3_2_1_in_rule__PROGRESSIVE__Group_3_2__1__Impl4780 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__0__Impl_in_rule__PROGRESSIVE__Group_3_3__04814 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__1_in_rule__PROGRESSIVE__Group_3_3__04817 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_rule__PROGRESSIVE__Group_3_3__0__Impl4845 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__1__Impl_in_rule__PROGRESSIVE__Group_3_3__14876 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_3_3__1__Impl4903 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRoot__RtagsAssignment_04941 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLRoot__RmacrosAssignment_14972 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacro__NameAssignment_15003 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__ValueAlternatives_2_0_in_rule__WMLMacro__ValueAssignment_25034 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_15067 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLTag__TtagsAssignment_3_05098 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLTag__TkeysAssignment_3_15129 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLTag__TmacrosAssignment_3_25160 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_55191 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__KeyNameAssignment_05222 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_25253 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Key1ValueAlternatives_0_0_in_rule__WMLKeyValue__Key1ValueAssignment_05284 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLKeyValue__Key2ValueAssignment_15317 = new BitSet(new long[]{0x0000000000000002L});

}