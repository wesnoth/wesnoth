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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_STRING", "RULE_IINT", "RULE_MACRO", "RULE_SL_COMMENT", "RULE_WS", "'-'", "'/'", "'n'", "'s'", "'w'", "'e'", "'sw'", "'se'", "'ne'", "'nw'", "'['", "']'", "'[/'", "'='", "'.'", "' '", "'_'", "','", "'~'", "':'"
    };
    public static final int RULE_ID=4;
    public static final int RULE_STRING=5;
    public static final int RULE_IINT=6;
    public static final int RULE_WS=9;
    public static final int RULE_SL_COMMENT=8;
    public static final int EOF=-1;
    public static final int RULE_MACRO=7;

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

                if ( (LA1_0==RULE_MACRO||LA1_0==20) ) {
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:98:1: ruleWMLMacro : ( ( rule__WMLMacro__NameAssignment ) ) ;
    public final void ruleWMLMacro() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:102:2: ( ( ( rule__WMLMacro__NameAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:103:1: ( ( rule__WMLMacro__NameAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:103:1: ( ( rule__WMLMacro__NameAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:104:1: ( rule__WMLMacro__NameAssignment )
            {
             before(grammarAccess.getWMLMacroAccess().getNameAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:105:1: ( rule__WMLMacro__NameAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:105:2: rule__WMLMacro__NameAssignment
            {
            pushFollow(FOLLOW_rule__WMLMacro__NameAssignment_in_ruleWMLMacro155);
            rule__WMLMacro__NameAssignment();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroAccess().getNameAssignment()); 

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

                if ( ((LA2_0>=12 && LA2_0<=19)) ) {
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

            if ( (LA3_0==20) ) {
                alt3=1;
            }
            else if ( (LA3_0==RULE_MACRO) ) {
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


    // $ANTLR start rule__WMLTag__Alternatives_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:406:1: rule__WMLTag__Alternatives_3 : ( ( ( rule__WMLTag__TtagsAssignment_3_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_3_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_3_2 ) ) );
    public final void rule__WMLTag__Alternatives_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:410:1: ( ( ( rule__WMLTag__TtagsAssignment_3_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_3_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_3_2 ) ) )
            int alt4=3;
            switch ( input.LA(1) ) {
            case 20:
                {
                alt4=1;
                }
                break;
            case RULE_ID:
                {
                alt4=2;
                }
                break;
            case RULE_MACRO:
                {
                alt4=3;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("406:1: rule__WMLTag__Alternatives_3 : ( ( ( rule__WMLTag__TtagsAssignment_3_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_3_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_3_2 ) ) );", 4, 0, input);

                throw nvae;
            }

            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:411:1: ( ( rule__WMLTag__TtagsAssignment_3_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:411:1: ( ( rule__WMLTag__TtagsAssignment_3_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:412:1: ( rule__WMLTag__TtagsAssignment_3_0 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTtagsAssignment_3_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:413:1: ( rule__WMLTag__TtagsAssignment_3_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:413:2: rule__WMLTag__TtagsAssignment_3_0
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TtagsAssignment_3_0_in_rule__WMLTag__Alternatives_3808);
                    rule__WMLTag__TtagsAssignment_3_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTtagsAssignment_3_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:417:6: ( ( rule__WMLTag__TkeysAssignment_3_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:417:6: ( ( rule__WMLTag__TkeysAssignment_3_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:418:1: ( rule__WMLTag__TkeysAssignment_3_1 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTkeysAssignment_3_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:419:1: ( rule__WMLTag__TkeysAssignment_3_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:419:2: rule__WMLTag__TkeysAssignment_3_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TkeysAssignment_3_1_in_rule__WMLTag__Alternatives_3826);
                    rule__WMLTag__TkeysAssignment_3_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTkeysAssignment_3_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:423:6: ( ( rule__WMLTag__TmacrosAssignment_3_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:423:6: ( ( rule__WMLTag__TmacrosAssignment_3_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:424:1: ( rule__WMLTag__TmacrosAssignment_3_2 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTmacrosAssignment_3_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:425:1: ( rule__WMLTag__TmacrosAssignment_3_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:425:2: rule__WMLTag__TmacrosAssignment_3_2
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TmacrosAssignment_3_2_in_rule__WMLTag__Alternatives_3844);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:434:1: rule__WMLKeyValue__Alternatives : ( ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) ) | ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) ) );
    public final void rule__WMLKeyValue__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:438:1: ( ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) ) | ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) ) )
            int alt5=2;
            int LA5_0 = input.LA(1);

            if ( ((LA5_0>=RULE_ID && LA5_0<=RULE_IINT)||(LA5_0>=12 && LA5_0<=19)||(LA5_0>=25 && LA5_0<=26)) ) {
                alt5=1;
            }
            else if ( (LA5_0==RULE_MACRO) ) {
                alt5=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("434:1: rule__WMLKeyValue__Alternatives : ( ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) ) | ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) ) );", 5, 0, input);

                throw nvae;
            }
            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:439:1: ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:439:1: ( ( rule__WMLKeyValue__Key1ValueAssignment_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:440:1: ( rule__WMLKeyValue__Key1ValueAssignment_0 )
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueAssignment_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:441:1: ( rule__WMLKeyValue__Key1ValueAssignment_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:441:2: rule__WMLKeyValue__Key1ValueAssignment_0
                    {
                    pushFollow(FOLLOW_rule__WMLKeyValue__Key1ValueAssignment_0_in_rule__WMLKeyValue__Alternatives877);
                    rule__WMLKeyValue__Key1ValueAssignment_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueAssignment_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:445:6: ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:445:6: ( ( rule__WMLKeyValue__Key2ValueAssignment_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:446:1: ( rule__WMLKeyValue__Key2ValueAssignment_1 )
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey2ValueAssignment_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:447:1: ( rule__WMLKeyValue__Key2ValueAssignment_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:447:2: rule__WMLKeyValue__Key2ValueAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLKeyValue__Key2ValueAssignment_1_in_rule__WMLKeyValue__Alternatives895);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:456:1: rule__WMLKeyValue__Key1ValueAlternatives_0_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFLOAT ) | ( RULE_IINT ) | ( rulePATH ) | ( ruleDIRECTION ) | ( ruleLIST ) | ( rulePROGRESSIVE ) );
    public final void rule__WMLKeyValue__Key1ValueAlternatives_0_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:460:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFLOAT ) | ( RULE_IINT ) | ( rulePATH ) | ( ruleDIRECTION ) | ( ruleLIST ) | ( rulePROGRESSIVE ) )
            int alt6=9;
            alt6 = dfa6.predict(input);
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:461:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:461:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:462:1: RULE_ID
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueIDTerminalRuleCall_0_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKeyValue__Key1ValueAlternatives_0_0928); 
                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueIDTerminalRuleCall_0_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:467:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:467:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:468:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueSTRINGTerminalRuleCall_0_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_0945); 
                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueSTRINGTerminalRuleCall_0_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:473:6: ( ruleTSTRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:473:6: ( ruleTSTRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:474:1: ruleTSTRING
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueTSTRINGParserRuleCall_0_0_2()); 
                    pushFollow(FOLLOW_ruleTSTRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_0962);
                    ruleTSTRING();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueTSTRINGParserRuleCall_0_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:479:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:479:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:480:1: ruleFLOAT
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueFLOATParserRuleCall_0_0_3()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_0979);
                    ruleFLOAT();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueFLOATParserRuleCall_0_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:485:6: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:485:6: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:486:1: RULE_IINT
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueIINTTerminalRuleCall_0_0_4()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_0996); 
                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueIINTTerminalRuleCall_0_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:491:6: ( rulePATH )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:491:6: ( rulePATH )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:492:1: rulePATH
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValuePATHParserRuleCall_0_0_5()); 
                    pushFollow(FOLLOW_rulePATH_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01013);
                    rulePATH();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValuePATHParserRuleCall_0_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:497:6: ( ruleDIRECTION )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:497:6: ( ruleDIRECTION )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:498:1: ruleDIRECTION
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueDIRECTIONParserRuleCall_0_0_6()); 
                    pushFollow(FOLLOW_ruleDIRECTION_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01030);
                    ruleDIRECTION();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueDIRECTIONParserRuleCall_0_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:503:6: ( ruleLIST )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:503:6: ( ruleLIST )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:504:1: ruleLIST
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValueLISTParserRuleCall_0_0_7()); 
                    pushFollow(FOLLOW_ruleLIST_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01047);
                    ruleLIST();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getKey1ValueLISTParserRuleCall_0_0_7()); 

                    }


                    }
                    break;
                case 9 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:509:6: ( rulePROGRESSIVE )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:509:6: ( rulePROGRESSIVE )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:510:1: rulePROGRESSIVE
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getKey1ValuePROGRESSIVEParserRuleCall_0_0_8()); 
                    pushFollow(FOLLOW_rulePROGRESSIVE_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01064);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:520:1: rule__PATH__Alternatives_0_1 : ( ( '-' ) | ( '/' ) );
    public final void rule__PATH__Alternatives_0_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:524:1: ( ( '-' ) | ( '/' ) )
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( (LA7_0==10) ) {
                alt7=1;
            }
            else if ( (LA7_0==11) ) {
                alt7=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("520:1: rule__PATH__Alternatives_0_1 : ( ( '-' ) | ( '/' ) );", 7, 0, input);

                throw nvae;
            }
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:525:1: ( '-' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:525:1: ( '-' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:526:1: '-'
                    {
                     before(grammarAccess.getPATHAccess().getHyphenMinusKeyword_0_1_0()); 
                    match(input,10,FOLLOW_10_in_rule__PATH__Alternatives_0_11097); 
                     after(grammarAccess.getPATHAccess().getHyphenMinusKeyword_0_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:6: ( '/' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:6: ( '/' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:534:1: '/'
                    {
                     before(grammarAccess.getPATHAccess().getSolidusKeyword_0_1_1()); 
                    match(input,11,FOLLOW_11_in_rule__PATH__Alternatives_0_11117); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:546:1: rule__DIRECTION__Alternatives_0 : ( ( 'n' ) | ( 's' ) | ( 'w' ) | ( 'e' ) | ( 'sw' ) | ( 'se' ) | ( 'ne' ) | ( 'nw' ) );
    public final void rule__DIRECTION__Alternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:550:1: ( ( 'n' ) | ( 's' ) | ( 'w' ) | ( 'e' ) | ( 'sw' ) | ( 'se' ) | ( 'ne' ) | ( 'nw' ) )
            int alt8=8;
            switch ( input.LA(1) ) {
            case 12:
                {
                alt8=1;
                }
                break;
            case 13:
                {
                alt8=2;
                }
                break;
            case 14:
                {
                alt8=3;
                }
                break;
            case 15:
                {
                alt8=4;
                }
                break;
            case 16:
                {
                alt8=5;
                }
                break;
            case 17:
                {
                alt8=6;
                }
                break;
            case 18:
                {
                alt8=7;
                }
                break;
            case 19:
                {
                alt8=8;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("546:1: rule__DIRECTION__Alternatives_0 : ( ( 'n' ) | ( 's' ) | ( 'w' ) | ( 'e' ) | ( 'sw' ) | ( 'se' ) | ( 'ne' ) | ( 'nw' ) );", 8, 0, input);

                throw nvae;
            }

            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:551:1: ( 'n' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:551:1: ( 'n' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:552:1: 'n'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getNKeyword_0_0()); 
                    match(input,12,FOLLOW_12_in_rule__DIRECTION__Alternatives_01152); 
                     after(grammarAccess.getDIRECTIONAccess().getNKeyword_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:559:6: ( 's' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:559:6: ( 's' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:560:1: 's'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getSKeyword_0_1()); 
                    match(input,13,FOLLOW_13_in_rule__DIRECTION__Alternatives_01172); 
                     after(grammarAccess.getDIRECTIONAccess().getSKeyword_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:567:6: ( 'w' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:567:6: ( 'w' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:568:1: 'w'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getWKeyword_0_2()); 
                    match(input,14,FOLLOW_14_in_rule__DIRECTION__Alternatives_01192); 
                     after(grammarAccess.getDIRECTIONAccess().getWKeyword_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:575:6: ( 'e' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:575:6: ( 'e' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:576:1: 'e'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getEKeyword_0_3()); 
                    match(input,15,FOLLOW_15_in_rule__DIRECTION__Alternatives_01212); 
                     after(grammarAccess.getDIRECTIONAccess().getEKeyword_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:583:6: ( 'sw' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:583:6: ( 'sw' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:584:1: 'sw'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getSwKeyword_0_4()); 
                    match(input,16,FOLLOW_16_in_rule__DIRECTION__Alternatives_01232); 
                     after(grammarAccess.getDIRECTIONAccess().getSwKeyword_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:591:6: ( 'se' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:591:6: ( 'se' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:592:1: 'se'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getSeKeyword_0_5()); 
                    match(input,17,FOLLOW_17_in_rule__DIRECTION__Alternatives_01252); 
                     after(grammarAccess.getDIRECTIONAccess().getSeKeyword_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:599:6: ( 'ne' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:599:6: ( 'ne' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:600:1: 'ne'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getNeKeyword_0_6()); 
                    match(input,18,FOLLOW_18_in_rule__DIRECTION__Alternatives_01272); 
                     after(grammarAccess.getDIRECTIONAccess().getNeKeyword_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:607:6: ( 'nw' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:607:6: ( 'nw' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:608:1: 'nw'
                    {
                     before(grammarAccess.getDIRECTIONAccess().getNwKeyword_0_7()); 
                    match(input,19,FOLLOW_19_in_rule__DIRECTION__Alternatives_01292); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:620:1: rule__PROGRESSIVE__Alternatives_0 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:624:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
            int alt9=2;
            int LA9_0 = input.LA(1);

            if ( (LA9_0==RULE_IINT) ) {
                int LA9_1 = input.LA(2);

                if ( (LA9_1==24) ) {
                    alt9=2;
                }
                else if ( ((LA9_1>=27 && LA9_1<=29)) ) {
                    alt9=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("620:1: rule__PROGRESSIVE__Alternatives_0 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 9, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("620:1: rule__PROGRESSIVE__Alternatives_0 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 9, 0, input);

                throw nvae;
            }
            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:625:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:625:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:626:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_0_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_01326); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:631:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:631:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:632:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_0_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_01343);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:642:1: rule__PROGRESSIVE__Alternatives_1_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_1_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:646:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
            int alt10=2;
            int LA10_0 = input.LA(1);

            if ( (LA10_0==RULE_IINT) ) {
                int LA10_1 = input.LA(2);

                if ( (LA10_1==27||LA10_1==29) ) {
                    alt10=1;
                }
                else if ( (LA10_1==24) ) {
                    alt10=2;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("642:1: rule__PROGRESSIVE__Alternatives_1_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 10, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("642:1: rule__PROGRESSIVE__Alternatives_1_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 10, 0, input);

                throw nvae;
            }
            switch (alt10) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:647:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:647:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:648:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_1_1_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_1_11375); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_1_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:653:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:653:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:654:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_1_1_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_1_11392);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:664:1: rule__PROGRESSIVE__Alternatives_3_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_3_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:668:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
            int alt11=2;
            int LA11_0 = input.LA(1);

            if ( (LA11_0==RULE_IINT) ) {
                int LA11_1 = input.LA(2);

                if ( (LA11_1==24) ) {
                    alt11=2;
                }
                else if ( (LA11_1==EOF||LA11_1==RULE_ID||LA11_1==RULE_MACRO||LA11_1==20||LA11_1==22||(LA11_1>=27 && LA11_1<=29)) ) {
                    alt11=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("664:1: rule__PROGRESSIVE__Alternatives_3_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 11, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("664:1: rule__PROGRESSIVE__Alternatives_3_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 11, 0, input);

                throw nvae;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:669:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:669:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:670:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_1_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_11424); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:675:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:675:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:676:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_1_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_11441);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:686:1: rule__PROGRESSIVE__Alternatives_3_2_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );
    public final void rule__PROGRESSIVE__Alternatives_3_2_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:690:1: ( ( RULE_IINT ) | ( ruleFLOAT ) )
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0==RULE_IINT) ) {
                int LA12_1 = input.LA(2);

                if ( (LA12_1==EOF||LA12_1==RULE_ID||LA12_1==RULE_MACRO||LA12_1==20||LA12_1==22||LA12_1==27||LA12_1==29) ) {
                    alt12=1;
                }
                else if ( (LA12_1==24) ) {
                    alt12=2;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("686:1: rule__PROGRESSIVE__Alternatives_3_2_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 12, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("686:1: rule__PROGRESSIVE__Alternatives_3_2_1 : ( ( RULE_IINT ) | ( ruleFLOAT ) );", 12, 0, input);

                throw nvae;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:691:1: ( RULE_IINT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:691:1: ( RULE_IINT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:692:1: RULE_IINT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_2_1_0()); 
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_2_11473); 
                     after(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_2_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:697:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:697:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:698:1: ruleFLOAT
                    {
                     before(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_2_1_1()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_2_11490);
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


    // $ANTLR start rule__WMLTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:710:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:714:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:715:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01520);
            rule__WMLTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01523);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:722:1: rule__WMLTag__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:726:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:727:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:727:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:728:1: '['
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0()); 
            match(input,20,FOLLOW_20_in_rule__WMLTag__Group__0__Impl1551); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:741:1: rule__WMLTag__Group__1 : rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 ;
    public final void rule__WMLTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:745:1: ( rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:746:2: rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__11582);
            rule__WMLTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__11585);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:753:1: rule__WMLTag__Group__1__Impl : ( ( rule__WMLTag__NameAssignment_1 ) ) ;
    public final void rule__WMLTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:757:1: ( ( ( rule__WMLTag__NameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:758:1: ( ( rule__WMLTag__NameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:758:1: ( ( rule__WMLTag__NameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:759:1: ( rule__WMLTag__NameAssignment_1 )
            {
             before(grammarAccess.getWMLTagAccess().getNameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:760:1: ( rule__WMLTag__NameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:760:2: rule__WMLTag__NameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLTag__NameAssignment_1_in_rule__WMLTag__Group__1__Impl1612);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:770:1: rule__WMLTag__Group__2 : rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 ;
    public final void rule__WMLTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:774:1: ( rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:775:2: rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__21642);
            rule__WMLTag__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__21645);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:782:1: rule__WMLTag__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:786:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:787:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:787:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:788:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_2()); 
            match(input,21,FOLLOW_21_in_rule__WMLTag__Group__2__Impl1673); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:801:1: rule__WMLTag__Group__3 : rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 ;
    public final void rule__WMLTag__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:805:1: ( rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:806:2: rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__31704);
            rule__WMLTag__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__31707);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:813:1: rule__WMLTag__Group__3__Impl : ( ( rule__WMLTag__Alternatives_3 )* ) ;
    public final void rule__WMLTag__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:817:1: ( ( ( rule__WMLTag__Alternatives_3 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:818:1: ( ( rule__WMLTag__Alternatives_3 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:818:1: ( ( rule__WMLTag__Alternatives_3 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:819:1: ( rule__WMLTag__Alternatives_3 )*
            {
             before(grammarAccess.getWMLTagAccess().getAlternatives_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:820:1: ( rule__WMLTag__Alternatives_3 )*
            loop13:
            do {
                int alt13=2;
                int LA13_0 = input.LA(1);

                if ( (LA13_0==RULE_ID||LA13_0==RULE_MACRO||LA13_0==20) ) {
                    alt13=1;
                }


                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:820:2: rule__WMLTag__Alternatives_3
            	    {
            	    pushFollow(FOLLOW_rule__WMLTag__Alternatives_3_in_rule__WMLTag__Group__3__Impl1734);
            	    rule__WMLTag__Alternatives_3();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop13;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:830:1: rule__WMLTag__Group__4 : rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 ;
    public final void rule__WMLTag__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:834:1: ( rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:835:2: rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__41765);
            rule__WMLTag__Group__4__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__41768);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:842:1: rule__WMLTag__Group__4__Impl : ( '[/' ) ;
    public final void rule__WMLTag__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:846:1: ( ( '[/' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:847:1: ( '[/' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:847:1: ( '[/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:848:1: '[/'
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_4()); 
            match(input,22,FOLLOW_22_in_rule__WMLTag__Group__4__Impl1796); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:861:1: rule__WMLTag__Group__5 : rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 ;
    public final void rule__WMLTag__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:865:1: ( rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:866:2: rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__51827);
            rule__WMLTag__Group__5__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__51830);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:873:1: rule__WMLTag__Group__5__Impl : ( ( rule__WMLTag__EndNameAssignment_5 ) ) ;
    public final void rule__WMLTag__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:877:1: ( ( ( rule__WMLTag__EndNameAssignment_5 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:878:1: ( ( rule__WMLTag__EndNameAssignment_5 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:878:1: ( ( rule__WMLTag__EndNameAssignment_5 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:879:1: ( rule__WMLTag__EndNameAssignment_5 )
            {
             before(grammarAccess.getWMLTagAccess().getEndNameAssignment_5()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:880:1: ( rule__WMLTag__EndNameAssignment_5 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:880:2: rule__WMLTag__EndNameAssignment_5
            {
            pushFollow(FOLLOW_rule__WMLTag__EndNameAssignment_5_in_rule__WMLTag__Group__5__Impl1857);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:890:1: rule__WMLTag__Group__6 : rule__WMLTag__Group__6__Impl ;
    public final void rule__WMLTag__Group__6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:894:1: ( rule__WMLTag__Group__6__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:895:2: rule__WMLTag__Group__6__Impl
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__61887);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:901:1: rule__WMLTag__Group__6__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__6__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:905:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:906:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:906:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:907:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_6()); 
            match(input,21,FOLLOW_21_in_rule__WMLTag__Group__6__Impl1915); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:934:1: rule__WMLKey__Group__0 : rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 ;
    public final void rule__WMLKey__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:938:1: ( rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:939:2: rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__01960);
            rule__WMLKey__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__01963);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:946:1: rule__WMLKey__Group__0__Impl : ( ( rule__WMLKey__KeyNameAssignment_0 ) ) ;
    public final void rule__WMLKey__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:950:1: ( ( ( rule__WMLKey__KeyNameAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:951:1: ( ( rule__WMLKey__KeyNameAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:951:1: ( ( rule__WMLKey__KeyNameAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:952:1: ( rule__WMLKey__KeyNameAssignment_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getKeyNameAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:953:1: ( rule__WMLKey__KeyNameAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:953:2: rule__WMLKey__KeyNameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLKey__KeyNameAssignment_0_in_rule__WMLKey__Group__0__Impl1990);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:963:1: rule__WMLKey__Group__1 : rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 ;
    public final void rule__WMLKey__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:967:1: ( rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:968:2: rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12020);
            rule__WMLKey__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12023);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:975:1: rule__WMLKey__Group__1__Impl : ( '=' ) ;
    public final void rule__WMLKey__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:979:1: ( ( '=' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:980:1: ( '=' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:980:1: ( '=' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:981:1: '='
            {
             before(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1()); 
            match(input,23,FOLLOW_23_in_rule__WMLKey__Group__1__Impl2051); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:994:1: rule__WMLKey__Group__2 : rule__WMLKey__Group__2__Impl ;
    public final void rule__WMLKey__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:998:1: ( rule__WMLKey__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:999:2: rule__WMLKey__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22082);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1005:1: rule__WMLKey__Group__2__Impl : ( ( rule__WMLKey__ValueAssignment_2 ) ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1009:1: ( ( ( rule__WMLKey__ValueAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1010:1: ( ( rule__WMLKey__ValueAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1010:1: ( ( rule__WMLKey__ValueAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1011:1: ( rule__WMLKey__ValueAssignment_2 )
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1012:1: ( rule__WMLKey__ValueAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1012:2: rule__WMLKey__ValueAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2109);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1028:1: rule__FLOAT__Group__0 : rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1 ;
    public final void rule__FLOAT__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1032:1: ( rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1033:2: rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__0__Impl_in_rule__FLOAT__Group__02145);
            rule__FLOAT__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__FLOAT__Group__1_in_rule__FLOAT__Group__02148);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1040:1: rule__FLOAT__Group__0__Impl : ( RULE_IINT ) ;
    public final void rule__FLOAT__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1044:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1045:1: ( RULE_IINT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1045:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1046:1: RULE_IINT
            {
             before(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_0()); 
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__FLOAT__Group__0__Impl2175); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1057:1: rule__FLOAT__Group__1 : rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2 ;
    public final void rule__FLOAT__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1061:1: ( rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1062:2: rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__1__Impl_in_rule__FLOAT__Group__12204);
            rule__FLOAT__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__FLOAT__Group__2_in_rule__FLOAT__Group__12207);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1069:1: rule__FLOAT__Group__1__Impl : ( '.' ) ;
    public final void rule__FLOAT__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1073:1: ( ( '.' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1074:1: ( '.' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1074:1: ( '.' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1075:1: '.'
            {
             before(grammarAccess.getFLOATAccess().getFullStopKeyword_1()); 
            match(input,24,FOLLOW_24_in_rule__FLOAT__Group__1__Impl2235); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1088:1: rule__FLOAT__Group__2 : rule__FLOAT__Group__2__Impl ;
    public final void rule__FLOAT__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1092:1: ( rule__FLOAT__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1093:2: rule__FLOAT__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__2__Impl_in_rule__FLOAT__Group__22266);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1099:1: rule__FLOAT__Group__2__Impl : ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) ) ;
    public final void rule__FLOAT__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1103:1: ( ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1104:1: ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1104:1: ( ( ( RULE_IINT ) ) ( ( RULE_IINT )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1105:1: ( ( RULE_IINT ) ) ( ( RULE_IINT )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1105:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1106:1: ( RULE_IINT )
            {
             before(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1107:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1107:3: RULE_IINT
            {
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2296); 

            }

             after(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1110:1: ( ( RULE_IINT )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1111:1: ( RULE_IINT )*
            {
             before(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1112:1: ( RULE_IINT )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( (LA14_0==RULE_IINT) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1112:3: RULE_IINT
            	    {
            	    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2309); 

            	    }
            	    break;

            	default :
            	    break loop14;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1129:1: rule__TSTRING__Group__0 : rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 ;
    public final void rule__TSTRING__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1133:1: ( rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1134:2: rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__02348);
            rule__TSTRING__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__02351);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1141:1: rule__TSTRING__Group__0__Impl : ( ( rule__TSTRING__Group_0__0 ) ) ;
    public final void rule__TSTRING__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1145:1: ( ( ( rule__TSTRING__Group_0__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1146:1: ( ( rule__TSTRING__Group_0__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1146:1: ( ( rule__TSTRING__Group_0__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1147:1: ( rule__TSTRING__Group_0__0 )
            {
             before(grammarAccess.getTSTRINGAccess().getGroup_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1148:1: ( rule__TSTRING__Group_0__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1148:2: rule__TSTRING__Group_0__0
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__0_in_rule__TSTRING__Group__0__Impl2378);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1158:1: rule__TSTRING__Group__1 : rule__TSTRING__Group__1__Impl ;
    public final void rule__TSTRING__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1162:1: ( rule__TSTRING__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1163:2: rule__TSTRING__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__12408);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1169:1: rule__TSTRING__Group__1__Impl : ( RULE_STRING ) ;
    public final void rule__TSTRING__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1173:1: ( ( RULE_STRING ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1174:1: ( RULE_STRING )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1174:1: ( RULE_STRING )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1175:1: RULE_STRING
            {
             before(grammarAccess.getTSTRINGAccess().getSTRINGTerminalRuleCall_1()); 
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl2435); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1190:1: rule__TSTRING__Group_0__0 : rule__TSTRING__Group_0__0__Impl rule__TSTRING__Group_0__1 ;
    public final void rule__TSTRING__Group_0__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1194:1: ( rule__TSTRING__Group_0__0__Impl rule__TSTRING__Group_0__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1195:2: rule__TSTRING__Group_0__0__Impl rule__TSTRING__Group_0__1
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__0__Impl_in_rule__TSTRING__Group_0__02468);
            rule__TSTRING__Group_0__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group_0__1_in_rule__TSTRING__Group_0__02471);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1202:1: rule__TSTRING__Group_0__0__Impl : ( ( ' ' )? ) ;
    public final void rule__TSTRING__Group_0__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1206:1: ( ( ( ' ' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1207:1: ( ( ' ' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1207:1: ( ( ' ' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1208:1: ( ' ' )?
            {
             before(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1209:1: ( ' ' )?
            int alt15=2;
            int LA15_0 = input.LA(1);

            if ( (LA15_0==25) ) {
                alt15=1;
            }
            switch (alt15) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1210:2: ' '
                    {
                    match(input,25,FOLLOW_25_in_rule__TSTRING__Group_0__0__Impl2500); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1221:1: rule__TSTRING__Group_0__1 : rule__TSTRING__Group_0__1__Impl rule__TSTRING__Group_0__2 ;
    public final void rule__TSTRING__Group_0__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1225:1: ( rule__TSTRING__Group_0__1__Impl rule__TSTRING__Group_0__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1226:2: rule__TSTRING__Group_0__1__Impl rule__TSTRING__Group_0__2
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__1__Impl_in_rule__TSTRING__Group_0__12533);
            rule__TSTRING__Group_0__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group_0__2_in_rule__TSTRING__Group_0__12536);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1233:1: rule__TSTRING__Group_0__1__Impl : ( '_' ) ;
    public final void rule__TSTRING__Group_0__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1237:1: ( ( '_' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1238:1: ( '_' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1238:1: ( '_' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1239:1: '_'
            {
             before(grammarAccess.getTSTRINGAccess().get_Keyword_0_1()); 
            match(input,26,FOLLOW_26_in_rule__TSTRING__Group_0__1__Impl2564); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1252:1: rule__TSTRING__Group_0__2 : rule__TSTRING__Group_0__2__Impl ;
    public final void rule__TSTRING__Group_0__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1256:1: ( rule__TSTRING__Group_0__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1257:2: rule__TSTRING__Group_0__2__Impl
            {
            pushFollow(FOLLOW_rule__TSTRING__Group_0__2__Impl_in_rule__TSTRING__Group_0__22595);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1263:1: rule__TSTRING__Group_0__2__Impl : ( ( ' ' )? ) ;
    public final void rule__TSTRING__Group_0__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1267:1: ( ( ( ' ' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1268:1: ( ( ' ' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1268:1: ( ( ' ' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1269:1: ( ' ' )?
            {
             before(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1270:1: ( ' ' )?
            int alt16=2;
            int LA16_0 = input.LA(1);

            if ( (LA16_0==25) ) {
                alt16=1;
            }
            switch (alt16) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1271:2: ' '
                    {
                    match(input,25,FOLLOW_25_in_rule__TSTRING__Group_0__2__Impl2624); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1288:1: rule__PATH__Group__0 : rule__PATH__Group__0__Impl rule__PATH__Group__1 ;
    public final void rule__PATH__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1292:1: ( rule__PATH__Group__0__Impl rule__PATH__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1293:2: rule__PATH__Group__0__Impl rule__PATH__Group__1
            {
            pushFollow(FOLLOW_rule__PATH__Group__0__Impl_in_rule__PATH__Group__02663);
            rule__PATH__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group__1_in_rule__PATH__Group__02666);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1300:1: rule__PATH__Group__0__Impl : ( ( rule__PATH__Group_0__0 )* ) ;
    public final void rule__PATH__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1304:1: ( ( ( rule__PATH__Group_0__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1305:1: ( ( rule__PATH__Group_0__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1305:1: ( ( rule__PATH__Group_0__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1306:1: ( rule__PATH__Group_0__0 )*
            {
             before(grammarAccess.getPATHAccess().getGroup_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1307:1: ( rule__PATH__Group_0__0 )*
            loop17:
            do {
                int alt17=2;
                alt17 = dfa17.predict(input);
                switch (alt17) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1307:2: rule__PATH__Group_0__0
            	    {
            	    pushFollow(FOLLOW_rule__PATH__Group_0__0_in_rule__PATH__Group__0__Impl2693);
            	    rule__PATH__Group_0__0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop17;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1317:1: rule__PATH__Group__1 : rule__PATH__Group__1__Impl rule__PATH__Group__2 ;
    public final void rule__PATH__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1321:1: ( rule__PATH__Group__1__Impl rule__PATH__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1322:2: rule__PATH__Group__1__Impl rule__PATH__Group__2
            {
            pushFollow(FOLLOW_rule__PATH__Group__1__Impl_in_rule__PATH__Group__12724);
            rule__PATH__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group__2_in_rule__PATH__Group__12727);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1329:1: rule__PATH__Group__1__Impl : ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) ;
    public final void rule__PATH__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1333:1: ( ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1334:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1334:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1335:1: ( ( RULE_ID ) ) ( ( RULE_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1335:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1336:1: ( RULE_ID )
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1337:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1337:3: RULE_ID
            {
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl2757); 

            }

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1340:1: ( ( RULE_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1341:1: ( RULE_ID )*
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1342:1: ( RULE_ID )*
            loop18:
            do {
                int alt18=2;
                int LA18_0 = input.LA(1);

                if ( (LA18_0==RULE_ID) ) {
                    alt18=1;
                }


                switch (alt18) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1342:3: RULE_ID
            	    {
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl2770); 

            	    }
            	    break;

            	default :
            	    break loop18;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1353:1: rule__PATH__Group__2 : rule__PATH__Group__2__Impl rule__PATH__Group__3 ;
    public final void rule__PATH__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1357:1: ( rule__PATH__Group__2__Impl rule__PATH__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1358:2: rule__PATH__Group__2__Impl rule__PATH__Group__3
            {
            pushFollow(FOLLOW_rule__PATH__Group__2__Impl_in_rule__PATH__Group__22803);
            rule__PATH__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group__3_in_rule__PATH__Group__22806);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1365:1: rule__PATH__Group__2__Impl : ( '.' ) ;
    public final void rule__PATH__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1369:1: ( ( '.' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1370:1: ( '.' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1370:1: ( '.' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1371:1: '.'
            {
             before(grammarAccess.getPATHAccess().getFullStopKeyword_2()); 
            match(input,24,FOLLOW_24_in_rule__PATH__Group__2__Impl2834); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1384:1: rule__PATH__Group__3 : rule__PATH__Group__3__Impl ;
    public final void rule__PATH__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1388:1: ( rule__PATH__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1389:2: rule__PATH__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__PATH__Group__3__Impl_in_rule__PATH__Group__32865);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1395:1: rule__PATH__Group__3__Impl : ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) ;
    public final void rule__PATH__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1399:1: ( ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1400:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1400:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1401:1: ( ( RULE_ID ) ) ( ( RULE_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1401:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1402:1: ( RULE_ID )
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1403:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1403:3: RULE_ID
            {
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl2895); 

            }

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1406:1: ( ( RULE_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1407:1: ( RULE_ID )*
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1408:1: ( RULE_ID )*
            loop19:
            do {
                int alt19=2;
                int LA19_0 = input.LA(1);

                if ( (LA19_0==RULE_ID) ) {
                    int LA19_2 = input.LA(2);

                    if ( (LA19_2==EOF||LA19_2==RULE_ID||LA19_2==RULE_MACRO||LA19_2==20||LA19_2==22) ) {
                        alt19=1;
                    }


                }


                switch (alt19) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1408:3: RULE_ID
            	    {
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl2908); 

            	    }
            	    break;

            	default :
            	    break loop19;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1427:1: rule__PATH__Group_0__0 : rule__PATH__Group_0__0__Impl rule__PATH__Group_0__1 ;
    public final void rule__PATH__Group_0__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1431:1: ( rule__PATH__Group_0__0__Impl rule__PATH__Group_0__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1432:2: rule__PATH__Group_0__0__Impl rule__PATH__Group_0__1
            {
            pushFollow(FOLLOW_rule__PATH__Group_0__0__Impl_in_rule__PATH__Group_0__02949);
            rule__PATH__Group_0__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PATH__Group_0__1_in_rule__PATH__Group_0__02952);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1439:1: rule__PATH__Group_0__0__Impl : ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) ;
    public final void rule__PATH__Group_0__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1443:1: ( ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1444:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1444:1: ( ( ( RULE_ID ) ) ( ( RULE_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1445:1: ( ( RULE_ID ) ) ( ( RULE_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1445:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1446:1: ( RULE_ID )
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1447:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1447:3: RULE_ID
            {
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl2982); 

            }

             after(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1450:1: ( ( RULE_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1451:1: ( RULE_ID )*
            {
             before(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1452:1: ( RULE_ID )*
            loop20:
            do {
                int alt20=2;
                int LA20_0 = input.LA(1);

                if ( (LA20_0==RULE_ID) ) {
                    alt20=1;
                }


                switch (alt20) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1452:3: RULE_ID
            	    {
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl2995); 

            	    }
            	    break;

            	default :
            	    break loop20;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1463:1: rule__PATH__Group_0__1 : rule__PATH__Group_0__1__Impl ;
    public final void rule__PATH__Group_0__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1467:1: ( rule__PATH__Group_0__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1468:2: rule__PATH__Group_0__1__Impl
            {
            pushFollow(FOLLOW_rule__PATH__Group_0__1__Impl_in_rule__PATH__Group_0__13028);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1474:1: rule__PATH__Group_0__1__Impl : ( ( rule__PATH__Alternatives_0_1 ) ) ;
    public final void rule__PATH__Group_0__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1478:1: ( ( ( rule__PATH__Alternatives_0_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1479:1: ( ( rule__PATH__Alternatives_0_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1479:1: ( ( rule__PATH__Alternatives_0_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1480:1: ( rule__PATH__Alternatives_0_1 )
            {
             before(grammarAccess.getPATHAccess().getAlternatives_0_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1481:1: ( rule__PATH__Alternatives_0_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1481:2: rule__PATH__Alternatives_0_1
            {
            pushFollow(FOLLOW_rule__PATH__Alternatives_0_1_in_rule__PATH__Group_0__1__Impl3055);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1495:1: rule__DIRECTION__Group__0 : rule__DIRECTION__Group__0__Impl rule__DIRECTION__Group__1 ;
    public final void rule__DIRECTION__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1499:1: ( rule__DIRECTION__Group__0__Impl rule__DIRECTION__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1500:2: rule__DIRECTION__Group__0__Impl rule__DIRECTION__Group__1
            {
            pushFollow(FOLLOW_rule__DIRECTION__Group__0__Impl_in_rule__DIRECTION__Group__03089);
            rule__DIRECTION__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__DIRECTION__Group__1_in_rule__DIRECTION__Group__03092);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1507:1: rule__DIRECTION__Group__0__Impl : ( ( rule__DIRECTION__Alternatives_0 ) ) ;
    public final void rule__DIRECTION__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1511:1: ( ( ( rule__DIRECTION__Alternatives_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1512:1: ( ( rule__DIRECTION__Alternatives_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1512:1: ( ( rule__DIRECTION__Alternatives_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1513:1: ( rule__DIRECTION__Alternatives_0 )
            {
             before(grammarAccess.getDIRECTIONAccess().getAlternatives_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1514:1: ( rule__DIRECTION__Alternatives_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1514:2: rule__DIRECTION__Alternatives_0
            {
            pushFollow(FOLLOW_rule__DIRECTION__Alternatives_0_in_rule__DIRECTION__Group__0__Impl3119);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1524:1: rule__DIRECTION__Group__1 : rule__DIRECTION__Group__1__Impl ;
    public final void rule__DIRECTION__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1528:1: ( rule__DIRECTION__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1529:2: rule__DIRECTION__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__DIRECTION__Group__1__Impl_in_rule__DIRECTION__Group__13149);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1535:1: rule__DIRECTION__Group__1__Impl : ( ( ',' )? ) ;
    public final void rule__DIRECTION__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1539:1: ( ( ( ',' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1540:1: ( ( ',' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1540:1: ( ( ',' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1541:1: ( ',' )?
            {
             before(grammarAccess.getDIRECTIONAccess().getCommaKeyword_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1542:1: ( ',' )?
            int alt21=2;
            int LA21_0 = input.LA(1);

            if ( (LA21_0==27) ) {
                alt21=1;
            }
            switch (alt21) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1543:2: ','
                    {
                    match(input,27,FOLLOW_27_in_rule__DIRECTION__Group__1__Impl3178); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1558:1: rule__LIST__Group__0 : rule__LIST__Group__0__Impl rule__LIST__Group__1 ;
    public final void rule__LIST__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1562:1: ( rule__LIST__Group__0__Impl rule__LIST__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1563:2: rule__LIST__Group__0__Impl rule__LIST__Group__1
            {
            pushFollow(FOLLOW_rule__LIST__Group__0__Impl_in_rule__LIST__Group__03215);
            rule__LIST__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__LIST__Group__1_in_rule__LIST__Group__03218);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1570:1: rule__LIST__Group__0__Impl : ( RULE_ID ) ;
    public final void rule__LIST__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1574:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1575:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1575:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1576:1: RULE_ID
            {
             before(grammarAccess.getLISTAccess().getIDTerminalRuleCall_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__LIST__Group__0__Impl3245); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1587:1: rule__LIST__Group__1 : rule__LIST__Group__1__Impl ;
    public final void rule__LIST__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1591:1: ( rule__LIST__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1592:2: rule__LIST__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__LIST__Group__1__Impl_in_rule__LIST__Group__13274);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1598:1: rule__LIST__Group__1__Impl : ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) ) ;
    public final void rule__LIST__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1602:1: ( ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1603:1: ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1603:1: ( ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1604:1: ( ( rule__LIST__Group_1__0 ) ) ( ( rule__LIST__Group_1__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1604:1: ( ( rule__LIST__Group_1__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1605:1: ( rule__LIST__Group_1__0 )
            {
             before(grammarAccess.getLISTAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1606:1: ( rule__LIST__Group_1__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1606:2: rule__LIST__Group_1__0
            {
            pushFollow(FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl3303);
            rule__LIST__Group_1__0();
            _fsp--;


            }

             after(grammarAccess.getLISTAccess().getGroup_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1609:1: ( ( rule__LIST__Group_1__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1610:1: ( rule__LIST__Group_1__0 )*
            {
             before(grammarAccess.getLISTAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1611:1: ( rule__LIST__Group_1__0 )*
            loop22:
            do {
                int alt22=2;
                int LA22_0 = input.LA(1);

                if ( (LA22_0==27) ) {
                    alt22=1;
                }


                switch (alt22) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1611:2: rule__LIST__Group_1__0
            	    {
            	    pushFollow(FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl3315);
            	    rule__LIST__Group_1__0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop22;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1626:1: rule__LIST__Group_1__0 : rule__LIST__Group_1__0__Impl rule__LIST__Group_1__1 ;
    public final void rule__LIST__Group_1__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1630:1: ( rule__LIST__Group_1__0__Impl rule__LIST__Group_1__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1631:2: rule__LIST__Group_1__0__Impl rule__LIST__Group_1__1
            {
            pushFollow(FOLLOW_rule__LIST__Group_1__0__Impl_in_rule__LIST__Group_1__03352);
            rule__LIST__Group_1__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__LIST__Group_1__1_in_rule__LIST__Group_1__03355);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1638:1: rule__LIST__Group_1__0__Impl : ( ',' ) ;
    public final void rule__LIST__Group_1__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1642:1: ( ( ',' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1643:1: ( ',' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1643:1: ( ',' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1644:1: ','
            {
             before(grammarAccess.getLISTAccess().getCommaKeyword_1_0()); 
            match(input,27,FOLLOW_27_in_rule__LIST__Group_1__0__Impl3383); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1657:1: rule__LIST__Group_1__1 : rule__LIST__Group_1__1__Impl ;
    public final void rule__LIST__Group_1__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1661:1: ( rule__LIST__Group_1__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1662:2: rule__LIST__Group_1__1__Impl
            {
            pushFollow(FOLLOW_rule__LIST__Group_1__1__Impl_in_rule__LIST__Group_1__13414);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1668:1: rule__LIST__Group_1__1__Impl : ( RULE_ID ) ;
    public final void rule__LIST__Group_1__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1672:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1673:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1673:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1674:1: RULE_ID
            {
             before(grammarAccess.getLISTAccess().getIDTerminalRuleCall_1_1()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__LIST__Group_1__1__Impl3441); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1689:1: rule__PROGRESSIVE__Group__0 : rule__PROGRESSIVE__Group__0__Impl rule__PROGRESSIVE__Group__1 ;
    public final void rule__PROGRESSIVE__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1693:1: ( rule__PROGRESSIVE__Group__0__Impl rule__PROGRESSIVE__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1694:2: rule__PROGRESSIVE__Group__0__Impl rule__PROGRESSIVE__Group__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__0__Impl_in_rule__PROGRESSIVE__Group__03474);
            rule__PROGRESSIVE__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__1_in_rule__PROGRESSIVE__Group__03477);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1701:1: rule__PROGRESSIVE__Group__0__Impl : ( ( rule__PROGRESSIVE__Alternatives_0 ) ) ;
    public final void rule__PROGRESSIVE__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1705:1: ( ( ( rule__PROGRESSIVE__Alternatives_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1706:1: ( ( rule__PROGRESSIVE__Alternatives_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1706:1: ( ( rule__PROGRESSIVE__Alternatives_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1707:1: ( rule__PROGRESSIVE__Alternatives_0 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1708:1: ( rule__PROGRESSIVE__Alternatives_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1708:2: rule__PROGRESSIVE__Alternatives_0
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_0_in_rule__PROGRESSIVE__Group__0__Impl3504);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1718:1: rule__PROGRESSIVE__Group__1 : rule__PROGRESSIVE__Group__1__Impl rule__PROGRESSIVE__Group__2 ;
    public final void rule__PROGRESSIVE__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1722:1: ( rule__PROGRESSIVE__Group__1__Impl rule__PROGRESSIVE__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1723:2: rule__PROGRESSIVE__Group__1__Impl rule__PROGRESSIVE__Group__2
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__1__Impl_in_rule__PROGRESSIVE__Group__13534);
            rule__PROGRESSIVE__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__2_in_rule__PROGRESSIVE__Group__13537);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1730:1: rule__PROGRESSIVE__Group__1__Impl : ( ( rule__PROGRESSIVE__Group_1__0 )? ) ;
    public final void rule__PROGRESSIVE__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1734:1: ( ( ( rule__PROGRESSIVE__Group_1__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1735:1: ( ( rule__PROGRESSIVE__Group_1__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1735:1: ( ( rule__PROGRESSIVE__Group_1__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1736:1: ( rule__PROGRESSIVE__Group_1__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1737:1: ( rule__PROGRESSIVE__Group_1__0 )?
            int alt23=2;
            int LA23_0 = input.LA(1);

            if ( (LA23_0==28) ) {
                alt23=1;
            }
            switch (alt23) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1737:2: rule__PROGRESSIVE__Group_1__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__0_in_rule__PROGRESSIVE__Group__1__Impl3564);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1747:1: rule__PROGRESSIVE__Group__2 : rule__PROGRESSIVE__Group__2__Impl rule__PROGRESSIVE__Group__3 ;
    public final void rule__PROGRESSIVE__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1751:1: ( rule__PROGRESSIVE__Group__2__Impl rule__PROGRESSIVE__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1752:2: rule__PROGRESSIVE__Group__2__Impl rule__PROGRESSIVE__Group__3
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__2__Impl_in_rule__PROGRESSIVE__Group__23595);
            rule__PROGRESSIVE__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__3_in_rule__PROGRESSIVE__Group__23598);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1759:1: rule__PROGRESSIVE__Group__2__Impl : ( ( rule__PROGRESSIVE__Group_2__0 )? ) ;
    public final void rule__PROGRESSIVE__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1763:1: ( ( ( rule__PROGRESSIVE__Group_2__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1764:1: ( ( rule__PROGRESSIVE__Group_2__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1764:1: ( ( rule__PROGRESSIVE__Group_2__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1765:1: ( rule__PROGRESSIVE__Group_2__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1766:1: ( rule__PROGRESSIVE__Group_2__0 )?
            int alt24=2;
            int LA24_0 = input.LA(1);

            if ( (LA24_0==29) ) {
                alt24=1;
            }
            switch (alt24) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1766:2: rule__PROGRESSIVE__Group_2__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__0_in_rule__PROGRESSIVE__Group__2__Impl3625);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1776:1: rule__PROGRESSIVE__Group__3 : rule__PROGRESSIVE__Group__3__Impl ;
    public final void rule__PROGRESSIVE__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1780:1: ( rule__PROGRESSIVE__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1781:2: rule__PROGRESSIVE__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group__3__Impl_in_rule__PROGRESSIVE__Group__33656);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1787:1: rule__PROGRESSIVE__Group__3__Impl : ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) ) ;
    public final void rule__PROGRESSIVE__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1791:1: ( ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1792:1: ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1792:1: ( ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1793:1: ( ( rule__PROGRESSIVE__Group_3__0 ) ) ( ( rule__PROGRESSIVE__Group_3__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1793:1: ( ( rule__PROGRESSIVE__Group_3__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1794:1: ( rule__PROGRESSIVE__Group_3__0 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1795:1: ( rule__PROGRESSIVE__Group_3__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1795:2: rule__PROGRESSIVE__Group_3__0
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl3685);
            rule__PROGRESSIVE__Group_3__0();
            _fsp--;


            }

             after(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1798:1: ( ( rule__PROGRESSIVE__Group_3__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1799:1: ( rule__PROGRESSIVE__Group_3__0 )*
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1800:1: ( rule__PROGRESSIVE__Group_3__0 )*
            loop25:
            do {
                int alt25=2;
                int LA25_0 = input.LA(1);

                if ( (LA25_0==27) ) {
                    alt25=1;
                }


                switch (alt25) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1800:2: rule__PROGRESSIVE__Group_3__0
            	    {
            	    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl3697);
            	    rule__PROGRESSIVE__Group_3__0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop25;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1819:1: rule__PROGRESSIVE__Group_1__0 : rule__PROGRESSIVE__Group_1__0__Impl rule__PROGRESSIVE__Group_1__1 ;
    public final void rule__PROGRESSIVE__Group_1__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1823:1: ( rule__PROGRESSIVE__Group_1__0__Impl rule__PROGRESSIVE__Group_1__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1824:2: rule__PROGRESSIVE__Group_1__0__Impl rule__PROGRESSIVE__Group_1__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__0__Impl_in_rule__PROGRESSIVE__Group_1__03738);
            rule__PROGRESSIVE__Group_1__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__1_in_rule__PROGRESSIVE__Group_1__03741);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1831:1: rule__PROGRESSIVE__Group_1__0__Impl : ( '~' ) ;
    public final void rule__PROGRESSIVE__Group_1__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1835:1: ( ( '~' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1836:1: ( '~' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1836:1: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1837:1: '~'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_1_0()); 
            match(input,28,FOLLOW_28_in_rule__PROGRESSIVE__Group_1__0__Impl3769); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1850:1: rule__PROGRESSIVE__Group_1__1 : rule__PROGRESSIVE__Group_1__1__Impl ;
    public final void rule__PROGRESSIVE__Group_1__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1854:1: ( rule__PROGRESSIVE__Group_1__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1855:2: rule__PROGRESSIVE__Group_1__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_1__1__Impl_in_rule__PROGRESSIVE__Group_1__13800);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1861:1: rule__PROGRESSIVE__Group_1__1__Impl : ( ( rule__PROGRESSIVE__Alternatives_1_1 ) ) ;
    public final void rule__PROGRESSIVE__Group_1__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1865:1: ( ( ( rule__PROGRESSIVE__Alternatives_1_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1866:1: ( ( rule__PROGRESSIVE__Alternatives_1_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1866:1: ( ( rule__PROGRESSIVE__Alternatives_1_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1867:1: ( rule__PROGRESSIVE__Alternatives_1_1 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_1_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1868:1: ( rule__PROGRESSIVE__Alternatives_1_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1868:2: rule__PROGRESSIVE__Alternatives_1_1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_1_1_in_rule__PROGRESSIVE__Group_1__1__Impl3827);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1882:1: rule__PROGRESSIVE__Group_2__0 : rule__PROGRESSIVE__Group_2__0__Impl rule__PROGRESSIVE__Group_2__1 ;
    public final void rule__PROGRESSIVE__Group_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1886:1: ( rule__PROGRESSIVE__Group_2__0__Impl rule__PROGRESSIVE__Group_2__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1887:2: rule__PROGRESSIVE__Group_2__0__Impl rule__PROGRESSIVE__Group_2__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__0__Impl_in_rule__PROGRESSIVE__Group_2__03861);
            rule__PROGRESSIVE__Group_2__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__1_in_rule__PROGRESSIVE__Group_2__03864);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1894:1: rule__PROGRESSIVE__Group_2__0__Impl : ( ':' ) ;
    public final void rule__PROGRESSIVE__Group_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1898:1: ( ( ':' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1899:1: ( ':' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1899:1: ( ':' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1900:1: ':'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_2_0()); 
            match(input,29,FOLLOW_29_in_rule__PROGRESSIVE__Group_2__0__Impl3892); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1913:1: rule__PROGRESSIVE__Group_2__1 : rule__PROGRESSIVE__Group_2__1__Impl ;
    public final void rule__PROGRESSIVE__Group_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1917:1: ( rule__PROGRESSIVE__Group_2__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1918:2: rule__PROGRESSIVE__Group_2__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_2__1__Impl_in_rule__PROGRESSIVE__Group_2__13923);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1924:1: rule__PROGRESSIVE__Group_2__1__Impl : ( RULE_IINT ) ;
    public final void rule__PROGRESSIVE__Group_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1928:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1929:1: ( RULE_IINT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1929:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1930:1: RULE_IINT
            {
             before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_2_1()); 
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_2__1__Impl3950); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1945:1: rule__PROGRESSIVE__Group_3__0 : rule__PROGRESSIVE__Group_3__0__Impl rule__PROGRESSIVE__Group_3__1 ;
    public final void rule__PROGRESSIVE__Group_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1949:1: ( rule__PROGRESSIVE__Group_3__0__Impl rule__PROGRESSIVE__Group_3__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1950:2: rule__PROGRESSIVE__Group_3__0__Impl rule__PROGRESSIVE__Group_3__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__0__Impl_in_rule__PROGRESSIVE__Group_3__03983);
            rule__PROGRESSIVE__Group_3__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__1_in_rule__PROGRESSIVE__Group_3__03986);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1957:1: rule__PROGRESSIVE__Group_3__0__Impl : ( ',' ) ;
    public final void rule__PROGRESSIVE__Group_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1961:1: ( ( ',' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1962:1: ( ',' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1962:1: ( ',' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1963:1: ','
            {
             before(grammarAccess.getPROGRESSIVEAccess().getCommaKeyword_3_0()); 
            match(input,27,FOLLOW_27_in_rule__PROGRESSIVE__Group_3__0__Impl4014); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1976:1: rule__PROGRESSIVE__Group_3__1 : rule__PROGRESSIVE__Group_3__1__Impl rule__PROGRESSIVE__Group_3__2 ;
    public final void rule__PROGRESSIVE__Group_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1980:1: ( rule__PROGRESSIVE__Group_3__1__Impl rule__PROGRESSIVE__Group_3__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1981:2: rule__PROGRESSIVE__Group_3__1__Impl rule__PROGRESSIVE__Group_3__2
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__1__Impl_in_rule__PROGRESSIVE__Group_3__14045);
            rule__PROGRESSIVE__Group_3__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__2_in_rule__PROGRESSIVE__Group_3__14048);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1988:1: rule__PROGRESSIVE__Group_3__1__Impl : ( ( rule__PROGRESSIVE__Alternatives_3_1 ) ) ;
    public final void rule__PROGRESSIVE__Group_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1992:1: ( ( ( rule__PROGRESSIVE__Alternatives_3_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1993:1: ( ( rule__PROGRESSIVE__Alternatives_3_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1993:1: ( ( rule__PROGRESSIVE__Alternatives_3_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1994:1: ( rule__PROGRESSIVE__Alternatives_3_1 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_3_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1995:1: ( rule__PROGRESSIVE__Alternatives_3_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1995:2: rule__PROGRESSIVE__Alternatives_3_1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_3_1_in_rule__PROGRESSIVE__Group_3__1__Impl4075);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2005:1: rule__PROGRESSIVE__Group_3__2 : rule__PROGRESSIVE__Group_3__2__Impl rule__PROGRESSIVE__Group_3__3 ;
    public final void rule__PROGRESSIVE__Group_3__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2009:1: ( rule__PROGRESSIVE__Group_3__2__Impl rule__PROGRESSIVE__Group_3__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2010:2: rule__PROGRESSIVE__Group_3__2__Impl rule__PROGRESSIVE__Group_3__3
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__2__Impl_in_rule__PROGRESSIVE__Group_3__24105);
            rule__PROGRESSIVE__Group_3__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__3_in_rule__PROGRESSIVE__Group_3__24108);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2017:1: rule__PROGRESSIVE__Group_3__2__Impl : ( ( rule__PROGRESSIVE__Group_3_2__0 )? ) ;
    public final void rule__PROGRESSIVE__Group_3__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2021:1: ( ( ( rule__PROGRESSIVE__Group_3_2__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2022:1: ( ( rule__PROGRESSIVE__Group_3_2__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2022:1: ( ( rule__PROGRESSIVE__Group_3_2__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2023:1: ( rule__PROGRESSIVE__Group_3_2__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2024:1: ( rule__PROGRESSIVE__Group_3_2__0 )?
            int alt26=2;
            int LA26_0 = input.LA(1);

            if ( (LA26_0==28) ) {
                alt26=1;
            }
            switch (alt26) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2024:2: rule__PROGRESSIVE__Group_3_2__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__0_in_rule__PROGRESSIVE__Group_3__2__Impl4135);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2034:1: rule__PROGRESSIVE__Group_3__3 : rule__PROGRESSIVE__Group_3__3__Impl ;
    public final void rule__PROGRESSIVE__Group_3__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2038:1: ( rule__PROGRESSIVE__Group_3__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2039:2: rule__PROGRESSIVE__Group_3__3__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3__3__Impl_in_rule__PROGRESSIVE__Group_3__34166);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2045:1: rule__PROGRESSIVE__Group_3__3__Impl : ( ( rule__PROGRESSIVE__Group_3_3__0 )? ) ;
    public final void rule__PROGRESSIVE__Group_3__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2049:1: ( ( ( rule__PROGRESSIVE__Group_3_3__0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2050:1: ( ( rule__PROGRESSIVE__Group_3_3__0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2050:1: ( ( rule__PROGRESSIVE__Group_3_3__0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2051:1: ( rule__PROGRESSIVE__Group_3_3__0 )?
            {
             before(grammarAccess.getPROGRESSIVEAccess().getGroup_3_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2052:1: ( rule__PROGRESSIVE__Group_3_3__0 )?
            int alt27=2;
            int LA27_0 = input.LA(1);

            if ( (LA27_0==29) ) {
                alt27=1;
            }
            switch (alt27) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2052:2: rule__PROGRESSIVE__Group_3_3__0
                    {
                    pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__0_in_rule__PROGRESSIVE__Group_3__3__Impl4193);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2070:1: rule__PROGRESSIVE__Group_3_2__0 : rule__PROGRESSIVE__Group_3_2__0__Impl rule__PROGRESSIVE__Group_3_2__1 ;
    public final void rule__PROGRESSIVE__Group_3_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2074:1: ( rule__PROGRESSIVE__Group_3_2__0__Impl rule__PROGRESSIVE__Group_3_2__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2075:2: rule__PROGRESSIVE__Group_3_2__0__Impl rule__PROGRESSIVE__Group_3_2__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__0__Impl_in_rule__PROGRESSIVE__Group_3_2__04232);
            rule__PROGRESSIVE__Group_3_2__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__1_in_rule__PROGRESSIVE__Group_3_2__04235);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2082:1: rule__PROGRESSIVE__Group_3_2__0__Impl : ( '~' ) ;
    public final void rule__PROGRESSIVE__Group_3_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2086:1: ( ( '~' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2087:1: ( '~' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2087:1: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2088:1: '~'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_3_2_0()); 
            match(input,28,FOLLOW_28_in_rule__PROGRESSIVE__Group_3_2__0__Impl4263); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2101:1: rule__PROGRESSIVE__Group_3_2__1 : rule__PROGRESSIVE__Group_3_2__1__Impl ;
    public final void rule__PROGRESSIVE__Group_3_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2105:1: ( rule__PROGRESSIVE__Group_3_2__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2106:2: rule__PROGRESSIVE__Group_3_2__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_2__1__Impl_in_rule__PROGRESSIVE__Group_3_2__14294);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2112:1: rule__PROGRESSIVE__Group_3_2__1__Impl : ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) ) ;
    public final void rule__PROGRESSIVE__Group_3_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2116:1: ( ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2117:1: ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2117:1: ( ( rule__PROGRESSIVE__Alternatives_3_2_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2118:1: ( rule__PROGRESSIVE__Alternatives_3_2_1 )
            {
             before(grammarAccess.getPROGRESSIVEAccess().getAlternatives_3_2_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2119:1: ( rule__PROGRESSIVE__Alternatives_3_2_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2119:2: rule__PROGRESSIVE__Alternatives_3_2_1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Alternatives_3_2_1_in_rule__PROGRESSIVE__Group_3_2__1__Impl4321);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2133:1: rule__PROGRESSIVE__Group_3_3__0 : rule__PROGRESSIVE__Group_3_3__0__Impl rule__PROGRESSIVE__Group_3_3__1 ;
    public final void rule__PROGRESSIVE__Group_3_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2137:1: ( rule__PROGRESSIVE__Group_3_3__0__Impl rule__PROGRESSIVE__Group_3_3__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2138:2: rule__PROGRESSIVE__Group_3_3__0__Impl rule__PROGRESSIVE__Group_3_3__1
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__0__Impl_in_rule__PROGRESSIVE__Group_3_3__04355);
            rule__PROGRESSIVE__Group_3_3__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__1_in_rule__PROGRESSIVE__Group_3_3__04358);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2145:1: rule__PROGRESSIVE__Group_3_3__0__Impl : ( ':' ) ;
    public final void rule__PROGRESSIVE__Group_3_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2149:1: ( ( ':' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2150:1: ( ':' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2150:1: ( ':' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2151:1: ':'
            {
             before(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_3_3_0()); 
            match(input,29,FOLLOW_29_in_rule__PROGRESSIVE__Group_3_3__0__Impl4386); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2164:1: rule__PROGRESSIVE__Group_3_3__1 : rule__PROGRESSIVE__Group_3_3__1__Impl ;
    public final void rule__PROGRESSIVE__Group_3_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2168:1: ( rule__PROGRESSIVE__Group_3_3__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2169:2: rule__PROGRESSIVE__Group_3_3__1__Impl
            {
            pushFollow(FOLLOW_rule__PROGRESSIVE__Group_3_3__1__Impl_in_rule__PROGRESSIVE__Group_3_3__14417);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2175:1: rule__PROGRESSIVE__Group_3_3__1__Impl : ( RULE_IINT ) ;
    public final void rule__PROGRESSIVE__Group_3_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2179:1: ( ( RULE_IINT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2180:1: ( RULE_IINT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2180:1: ( RULE_IINT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2181:1: RULE_IINT
            {
             before(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_3_1()); 
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_3_3__1__Impl4444); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2197:1: rule__WMLRoot__RtagsAssignment_0 : ( ruleWMLTag ) ;
    public final void rule__WMLRoot__RtagsAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2201:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2202:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2202:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2203:1: ruleWMLTag
            {
             before(grammarAccess.getWMLRootAccess().getRtagsWMLTagParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRoot__RtagsAssignment_04482);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2212:1: rule__WMLRoot__RmacrosAssignment_1 : ( ruleWMLMacro ) ;
    public final void rule__WMLRoot__RmacrosAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2216:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2217:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2217:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2218:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLRootAccess().getRmacrosWMLMacroParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLRoot__RmacrosAssignment_14513);
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


    // $ANTLR start rule__WMLMacro__NameAssignment
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2227:1: rule__WMLMacro__NameAssignment : ( RULE_MACRO ) ;
    public final void rule__WMLMacro__NameAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2231:1: ( ( RULE_MACRO ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2232:1: ( RULE_MACRO )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2232:1: ( RULE_MACRO )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2233:1: RULE_MACRO
            {
             before(grammarAccess.getWMLMacroAccess().getNameMACROTerminalRuleCall_0()); 
            match(input,RULE_MACRO,FOLLOW_RULE_MACRO_in_rule__WMLMacro__NameAssignment4544); 
             after(grammarAccess.getWMLMacroAccess().getNameMACROTerminalRuleCall_0()); 

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
    // $ANTLR end rule__WMLMacro__NameAssignment


    // $ANTLR start rule__WMLTag__NameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2242:1: rule__WMLTag__NameAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLTag__NameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2246:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2247:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2247:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2248:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_14575); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2257:1: rule__WMLTag__TtagsAssignment_3_0 : ( ruleWMLTag ) ;
    public final void rule__WMLTag__TtagsAssignment_3_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2261:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2262:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2262:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2263:1: ruleWMLTag
            {
             before(grammarAccess.getWMLTagAccess().getTtagsWMLTagParserRuleCall_3_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLTag__TtagsAssignment_3_04606);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2272:1: rule__WMLTag__TkeysAssignment_3_1 : ( ruleWMLKey ) ;
    public final void rule__WMLTag__TkeysAssignment_3_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2276:1: ( ( ruleWMLKey ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2277:1: ( ruleWMLKey )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2277:1: ( ruleWMLKey )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2278:1: ruleWMLKey
            {
             before(grammarAccess.getWMLTagAccess().getTkeysWMLKeyParserRuleCall_3_1_0()); 
            pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLTag__TkeysAssignment_3_14637);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2287:1: rule__WMLTag__TmacrosAssignment_3_2 : ( ruleWMLMacro ) ;
    public final void rule__WMLTag__TmacrosAssignment_3_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2291:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2292:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2292:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2293:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLTagAccess().getTmacrosWMLMacroParserRuleCall_3_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLTag__TmacrosAssignment_3_24668);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2302:1: rule__WMLTag__EndNameAssignment_5 : ( RULE_ID ) ;
    public final void rule__WMLTag__EndNameAssignment_5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2306:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2307:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2307:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2308:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_5_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_54699); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2317:1: rule__WMLKey__KeyNameAssignment_0 : ( RULE_ID ) ;
    public final void rule__WMLKey__KeyNameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2321:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2322:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2322:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2323:1: RULE_ID
            {
             before(grammarAccess.getWMLKeyAccess().getKeyNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__KeyNameAssignment_04730); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2332:1: rule__WMLKey__ValueAssignment_2 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2336:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2337:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2337:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2338:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_24761);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2347:1: rule__WMLKeyValue__Key1ValueAssignment_0 : ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) ) ;
    public final void rule__WMLKeyValue__Key1ValueAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2351:1: ( ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2352:1: ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2352:1: ( ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2353:1: ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 )
            {
             before(grammarAccess.getWMLKeyValueAccess().getKey1ValueAlternatives_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2354:1: ( rule__WMLKeyValue__Key1ValueAlternatives_0_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2354:2: rule__WMLKeyValue__Key1ValueAlternatives_0_0
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Key1ValueAlternatives_0_0_in_rule__WMLKeyValue__Key1ValueAssignment_04792);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2363:1: rule__WMLKeyValue__Key2ValueAssignment_1 : ( ruleWMLMacro ) ;
    public final void rule__WMLKeyValue__Key2ValueAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2367:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2368:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2368:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2369:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLKeyValueAccess().getKey2ValueWMLMacroParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLKeyValue__Key2ValueAssignment_14825);
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


    protected DFA6 dfa6 = new DFA6(this);
    protected DFA17 dfa17 = new DFA17(this);
    static final String DFA6_eotS =
        "\20\uffff";
    static final String DFA6_eofS =
        "\1\uffff\1\11\2\uffff\1\14\10\uffff\2\17\1\uffff";
    static final String DFA6_minS =
        "\2\4\2\uffff\1\4\2\uffff\1\4\3\uffff\1\6\1\uffff\2\4\1\uffff";
    static final String DFA6_maxS =
        "\1\32\1\33\2\uffff\1\35\2\uffff\1\30\3\uffff\1\6\1\uffff\2\35\1"+
        "\uffff";
    static final String DFA6_acceptS =
        "\2\uffff\1\2\1\3\1\uffff\1\7\1\10\1\uffff\1\6\1\1\1\11\1\uffff\1"+
        "\5\2\uffff\1\4";
    static final String DFA6_specialS =
        "\20\uffff}>";
    static final String[] DFA6_transitionS = {
            "\1\1\1\2\1\4\5\uffff\10\5\5\uffff\2\3",
            "\1\7\2\uffff\1\11\2\uffff\2\10\10\uffff\1\11\1\uffff\1\11\1"+
            "\uffff\1\10\2\uffff\1\6",
            "",
            "",
            "\1\14\2\uffff\1\14\14\uffff\1\14\1\uffff\1\14\1\uffff\1\13\2"+
            "\uffff\3\12",
            "",
            "",
            "\1\10\5\uffff\2\10\13\uffff\1\11\1\10",
            "",
            "",
            "",
            "\1\15",
            "",
            "\1\17\1\uffff\1\16\1\17\14\uffff\1\17\1\uffff\1\17\4\uffff\3"+
            "\12",
            "\1\17\1\uffff\1\16\1\17\14\uffff\1\17\1\uffff\1\17\4\uffff\3"+
            "\12",
            ""
    };

    static final short[] DFA6_eot = DFA.unpackEncodedString(DFA6_eotS);
    static final short[] DFA6_eof = DFA.unpackEncodedString(DFA6_eofS);
    static final char[] DFA6_min = DFA.unpackEncodedStringToUnsignedChars(DFA6_minS);
    static final char[] DFA6_max = DFA.unpackEncodedStringToUnsignedChars(DFA6_maxS);
    static final short[] DFA6_accept = DFA.unpackEncodedString(DFA6_acceptS);
    static final short[] DFA6_special = DFA.unpackEncodedString(DFA6_specialS);
    static final short[][] DFA6_transition;

    static {
        int numStates = DFA6_transitionS.length;
        DFA6_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA6_transition[i] = DFA.unpackEncodedString(DFA6_transitionS[i]);
        }
    }

    class DFA6 extends DFA {

        public DFA6(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 6;
            this.eot = DFA6_eot;
            this.eof = DFA6_eof;
            this.min = DFA6_min;
            this.max = DFA6_max;
            this.accept = DFA6_accept;
            this.special = DFA6_special;
            this.transition = DFA6_transition;
        }
        public String getDescription() {
            return "456:1: rule__WMLKeyValue__Key1ValueAlternatives_0_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFLOAT ) | ( RULE_IINT ) | ( rulePATH ) | ( ruleDIRECTION ) | ( ruleLIST ) | ( rulePROGRESSIVE ) );";
        }
    }
    static final String DFA17_eotS =
        "\5\uffff";
    static final String DFA17_eofS =
        "\5\uffff";
    static final String DFA17_minS =
        "\3\4\2\uffff";
    static final String DFA17_maxS =
        "\1\4\2\30\2\uffff";
    static final String DFA17_acceptS =
        "\3\uffff\1\2\1\1";
    static final String DFA17_specialS =
        "\5\uffff}>";
    static final String[] DFA17_transitionS = {
            "\1\1",
            "\1\2\5\uffff\2\4\14\uffff\1\3",
            "\1\2\5\uffff\2\4\14\uffff\1\3",
            "",
            ""
    };

    static final short[] DFA17_eot = DFA.unpackEncodedString(DFA17_eotS);
    static final short[] DFA17_eof = DFA.unpackEncodedString(DFA17_eofS);
    static final char[] DFA17_min = DFA.unpackEncodedStringToUnsignedChars(DFA17_minS);
    static final char[] DFA17_max = DFA.unpackEncodedStringToUnsignedChars(DFA17_maxS);
    static final short[] DFA17_accept = DFA.unpackEncodedString(DFA17_acceptS);
    static final short[] DFA17_special = DFA.unpackEncodedString(DFA17_specialS);
    static final short[][] DFA17_transition;

    static {
        int numStates = DFA17_transitionS.length;
        DFA17_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA17_transition[i] = DFA.unpackEncodedString(DFA17_transitionS[i]);
        }
    }

    class DFA17 extends DFA {

        public DFA17(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 17;
            this.eot = DFA17_eot;
            this.eof = DFA17_eof;
            this.min = DFA17_min;
            this.max = DFA17_max;
            this.accept = DFA17_accept;
            this.special = DFA17_special;
            this.transition = DFA17_transition;
        }
        public String getDescription() {
            return "()* loopback of 1307:1: ( rule__PATH__Group_0__0 )*";
        }
    }
 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot61 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot68 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__Alternatives_in_ruleWMLRoot94 = new BitSet(new long[]{0x0000000000100082L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_entryRuleWMLMacro122 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacro129 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__NameAssignment_in_ruleWMLMacro155 = new BitSet(new long[]{0x0000000000000002L});
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
    public static final BitSet FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION586 = new BitSet(new long[]{0x00000000000FF002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__0_in_ruleDIRECTION598 = new BitSet(new long[]{0x00000000000FF002L});
    public static final BitSet FOLLOW_ruleLIST_in_entryRuleLIST628 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleLIST635 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group__0_in_ruleLIST661 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE688 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePROGRESSIVE695 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__0_in_rulePROGRESSIVE721 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__RtagsAssignment_0_in_rule__WMLRoot__Alternatives757 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__RmacrosAssignment_1_in_rule__WMLRoot__Alternatives775 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TtagsAssignment_3_0_in_rule__WMLTag__Alternatives_3808 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TkeysAssignment_3_1_in_rule__WMLTag__Alternatives_3826 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TmacrosAssignment_3_2_in_rule__WMLTag__Alternatives_3844 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Key1ValueAssignment_0_in_rule__WMLKeyValue__Alternatives877 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Key2ValueAssignment_1_in_rule__WMLKeyValue__Alternatives895 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKeyValue__Key1ValueAlternatives_0_0928 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_0945 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_rule__WMLKeyValue__Key1ValueAlternatives_0_0962 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_0979 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__WMLKeyValue__Key1ValueAlternatives_0_0996 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01013 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01030 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleLIST_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01047 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_rule__WMLKeyValue__Key1ValueAlternatives_0_01064 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_rule__PATH__Alternatives_0_11097 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_rule__PATH__Alternatives_0_11117 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__DIRECTION__Alternatives_01152 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__DIRECTION__Alternatives_01172 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__DIRECTION__Alternatives_01192 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__DIRECTION__Alternatives_01212 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__DIRECTION__Alternatives_01232 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_rule__DIRECTION__Alternatives_01252 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_rule__DIRECTION__Alternatives_01272 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_rule__DIRECTION__Alternatives_01292 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_01326 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_01343 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_1_11375 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_1_11392 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_11424 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_11441 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Alternatives_3_2_11473 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__PROGRESSIVE__Alternatives_3_2_11490 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01520 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01523 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__WMLTag__Group__0__Impl1551 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__11582 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__11585 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__NameAssignment_1_in_rule__WMLTag__Group__1__Impl1612 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__21642 = new BitSet(new long[]{0x0000000000500090L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__21645 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLTag__Group__2__Impl1673 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__31704 = new BitSet(new long[]{0x0000000000400000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__31707 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Alternatives_3_in_rule__WMLTag__Group__3__Impl1734 = new BitSet(new long[]{0x0000000000100092L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__41765 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__41768 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__WMLTag__Group__4__Impl1796 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__51827 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__51830 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__EndNameAssignment_5_in_rule__WMLTag__Group__5__Impl1857 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__61887 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLTag__Group__6__Impl1915 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__01960 = new BitSet(new long[]{0x0000000000800000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__01963 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__KeyNameAssignment_0_in_rule__WMLKey__Group__0__Impl1990 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12020 = new BitSet(new long[]{0x00000000060FF0F0L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12023 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__WMLKey__Group__1__Impl2051 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22082 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2109 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__0__Impl_in_rule__FLOAT__Group__02145 = new BitSet(new long[]{0x0000000001000000L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__1_in_rule__FLOAT__Group__02148 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__FLOAT__Group__0__Impl2175 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__1__Impl_in_rule__FLOAT__Group__12204 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__2_in_rule__FLOAT__Group__12207 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_rule__FLOAT__Group__1__Impl2235 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__2__Impl_in_rule__FLOAT__Group__22266 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2296 = new BitSet(new long[]{0x0000000000000042L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__FLOAT__Group__2__Impl2309 = new BitSet(new long[]{0x0000000000000042L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__02348 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__02351 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__0_in_rule__TSTRING__Group__0__Impl2378 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__12408 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl2435 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__0__Impl_in_rule__TSTRING__Group_0__02468 = new BitSet(new long[]{0x0000000004000000L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__1_in_rule__TSTRING__Group_0__02471 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__TSTRING__Group_0__0__Impl2500 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__1__Impl_in_rule__TSTRING__Group_0__12533 = new BitSet(new long[]{0x0000000002000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__2_in_rule__TSTRING__Group_0__12536 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_rule__TSTRING__Group_0__1__Impl2564 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group_0__2__Impl_in_rule__TSTRING__Group_0__22595 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__TSTRING__Group_0__2__Impl2624 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group__0__Impl_in_rule__PATH__Group__02663 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__PATH__Group__1_in_rule__PATH__Group__02666 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__0_in_rule__PATH__Group__0__Impl2693 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group__1__Impl_in_rule__PATH__Group__12724 = new BitSet(new long[]{0x0000000001000000L});
    public static final BitSet FOLLOW_rule__PATH__Group__2_in_rule__PATH__Group__12727 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl2757 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__1__Impl2770 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group__2__Impl_in_rule__PATH__Group__22803 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__PATH__Group__3_in_rule__PATH__Group__22806 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_rule__PATH__Group__2__Impl2834 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Group__3__Impl_in_rule__PATH__Group__32865 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl2895 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group__3__Impl2908 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__0__Impl_in_rule__PATH__Group_0__02949 = new BitSet(new long[]{0x0000000000000C00L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__1_in_rule__PATH__Group_0__02952 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl2982 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH__Group_0__0__Impl2995 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__PATH__Group_0__1__Impl_in_rule__PATH__Group_0__13028 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH__Alternatives_0_1_in_rule__PATH__Group_0__1__Impl3055 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__0__Impl_in_rule__DIRECTION__Group__03089 = new BitSet(new long[]{0x0000000008000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__1_in_rule__DIRECTION__Group__03092 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Alternatives_0_in_rule__DIRECTION__Group__0__Impl3119 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__DIRECTION__Group__1__Impl_in_rule__DIRECTION__Group__13149 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rule__DIRECTION__Group__1__Impl3178 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group__0__Impl_in_rule__LIST__Group__03215 = new BitSet(new long[]{0x0000000008000000L});
    public static final BitSet FOLLOW_rule__LIST__Group__1_in_rule__LIST__Group__03218 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__LIST__Group__0__Impl3245 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group__1__Impl_in_rule__LIST__Group__13274 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl3303 = new BitSet(new long[]{0x0000000008000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__0_in_rule__LIST__Group__1__Impl3315 = new BitSet(new long[]{0x0000000008000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__0__Impl_in_rule__LIST__Group_1__03352 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__1_in_rule__LIST__Group_1__03355 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rule__LIST__Group_1__0__Impl3383 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__LIST__Group_1__1__Impl_in_rule__LIST__Group_1__13414 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__LIST__Group_1__1__Impl3441 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__0__Impl_in_rule__PROGRESSIVE__Group__03474 = new BitSet(new long[]{0x0000000038000000L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__1_in_rule__PROGRESSIVE__Group__03477 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_0_in_rule__PROGRESSIVE__Group__0__Impl3504 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__1__Impl_in_rule__PROGRESSIVE__Group__13534 = new BitSet(new long[]{0x0000000028000000L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__2_in_rule__PROGRESSIVE__Group__13537 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__0_in_rule__PROGRESSIVE__Group__1__Impl3564 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__2__Impl_in_rule__PROGRESSIVE__Group__23595 = new BitSet(new long[]{0x0000000008000000L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__3_in_rule__PROGRESSIVE__Group__23598 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__0_in_rule__PROGRESSIVE__Group__2__Impl3625 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group__3__Impl_in_rule__PROGRESSIVE__Group__33656 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl3685 = new BitSet(new long[]{0x0000000008000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__0_in_rule__PROGRESSIVE__Group__3__Impl3697 = new BitSet(new long[]{0x0000000008000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__0__Impl_in_rule__PROGRESSIVE__Group_1__03738 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__1_in_rule__PROGRESSIVE__Group_1__03741 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__PROGRESSIVE__Group_1__0__Impl3769 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_1__1__Impl_in_rule__PROGRESSIVE__Group_1__13800 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_1_1_in_rule__PROGRESSIVE__Group_1__1__Impl3827 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__0__Impl_in_rule__PROGRESSIVE__Group_2__03861 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__1_in_rule__PROGRESSIVE__Group_2__03864 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__PROGRESSIVE__Group_2__0__Impl3892 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_2__1__Impl_in_rule__PROGRESSIVE__Group_2__13923 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_2__1__Impl3950 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__0__Impl_in_rule__PROGRESSIVE__Group_3__03983 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__1_in_rule__PROGRESSIVE__Group_3__03986 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rule__PROGRESSIVE__Group_3__0__Impl4014 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__1__Impl_in_rule__PROGRESSIVE__Group_3__14045 = new BitSet(new long[]{0x0000000030000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__2_in_rule__PROGRESSIVE__Group_3__14048 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_3_1_in_rule__PROGRESSIVE__Group_3__1__Impl4075 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__2__Impl_in_rule__PROGRESSIVE__Group_3__24105 = new BitSet(new long[]{0x0000000020000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__3_in_rule__PROGRESSIVE__Group_3__24108 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__0_in_rule__PROGRESSIVE__Group_3__2__Impl4135 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3__3__Impl_in_rule__PROGRESSIVE__Group_3__34166 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__0_in_rule__PROGRESSIVE__Group_3__3__Impl4193 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__0__Impl_in_rule__PROGRESSIVE__Group_3_2__04232 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__1_in_rule__PROGRESSIVE__Group_3_2__04235 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__PROGRESSIVE__Group_3_2__0__Impl4263 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_2__1__Impl_in_rule__PROGRESSIVE__Group_3_2__14294 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Alternatives_3_2_1_in_rule__PROGRESSIVE__Group_3_2__1__Impl4321 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__0__Impl_in_rule__PROGRESSIVE__Group_3_3__04355 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__1_in_rule__PROGRESSIVE__Group_3_3__04358 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__PROGRESSIVE__Group_3_3__0__Impl4386 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PROGRESSIVE__Group_3_3__1__Impl_in_rule__PROGRESSIVE__Group_3_3__14417 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rule__PROGRESSIVE__Group_3_3__1__Impl4444 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRoot__RtagsAssignment_04482 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLRoot__RmacrosAssignment_14513 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_MACRO_in_rule__WMLMacro__NameAssignment4544 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_14575 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLTag__TtagsAssignment_3_04606 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLTag__TkeysAssignment_3_14637 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLTag__TmacrosAssignment_3_24668 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_54699 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__KeyNameAssignment_04730 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_24761 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Key1ValueAlternatives_0_0_in_rule__WMLKeyValue__Key1ValueAssignment_04792 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLKeyValue__Key2ValueAssignment_14825 = new BitSet(new long[]{0x0000000000000002L});

}