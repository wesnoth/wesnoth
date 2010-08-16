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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_EOL", "RULE_SL_COMMENT", "RULE_ID", "RULE_STRING", "RULE_ANY_OTHER", "RULE_ENDDEF", "RULE_LUA_CODE", "RULE_DEFINE", "RULE_TEXTDOMAIN", "RULE_WS", "'_$'", "'~'", "'.'", "'('", "')'", "'='", "'['", "']'", "'+'", "'[/'", "'{'", "'./'", "'}'", "'_'"
    };
    public static final int RULE_LUA_CODE=10;
    public static final int RULE_ID=6;
    public static final int RULE_STRING=7;
    public static final int RULE_DEFINE=11;
    public static final int RULE_ANY_OTHER=8;
    public static final int RULE_ENDDEF=9;
    public static final int RULE_EOL=4;
    public static final int RULE_TEXTDOMAIN=12;
    public static final int RULE_WS=13;
    public static final int EOF=-1;
    public static final int RULE_SL_COMMENT=5;

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

                if ( ((LA1_0>=RULE_DEFINE && LA1_0<=RULE_TEXTDOMAIN)||LA1_0==20||LA1_0==24) ) {
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


    // $ANTLR start entryRuleWMLTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:89:1: entryRuleWMLTag : ruleWMLTag EOF ;
    public final void entryRuleWMLTag() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:90:1: ( ruleWMLTag EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:91:1: ruleWMLTag EOF
            {
             before(grammarAccess.getWMLTagRule()); 
            pushFollow(FOLLOW_ruleWMLTag_in_entryRuleWMLTag122);
            ruleWMLTag();
            _fsp--;

             after(grammarAccess.getWMLTagRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTag129); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:98:1: ruleWMLTag : ( ( rule__WMLTag__Group__0 ) ) ;
    public final void ruleWMLTag() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:102:2: ( ( ( rule__WMLTag__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:103:1: ( ( rule__WMLTag__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:103:1: ( ( rule__WMLTag__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:104:1: ( rule__WMLTag__Group__0 )
            {
             before(grammarAccess.getWMLTagAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:105:1: ( rule__WMLTag__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:105:2: rule__WMLTag__Group__0
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0_in_ruleWMLTag155);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:117:1: entryRuleWMLKey : ruleWMLKey EOF ;
    public final void entryRuleWMLKey() throws RecognitionException {

        	HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");

        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:121:1: ( ruleWMLKey EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:122:1: ruleWMLKey EOF
            {
             before(grammarAccess.getWMLKeyRule()); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey187);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLKeyRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey194); 

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
    // $ANTLR end entryRuleWMLKey


    // $ANTLR start ruleWMLKey
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:132:1: ruleWMLKey : ( ( rule__WMLKey__Group__0 ) ) ;
    public final void ruleWMLKey() throws RecognitionException {

        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:137:2: ( ( ( rule__WMLKey__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:138:1: ( ( rule__WMLKey__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:138:1: ( ( rule__WMLKey__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:139:1: ( rule__WMLKey__Group__0 )
            {
             before(grammarAccess.getWMLKeyAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:140:1: ( rule__WMLKey__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:140:2: rule__WMLKey__Group__0
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey224);
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
            	myHiddenTokenState.restore();

        }
        return ;
    }
    // $ANTLR end ruleWMLKey


    // $ANTLR start entryRuleWMLKeyValue
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:153:1: entryRuleWMLKeyValue : ruleWMLKeyValue EOF ;
    public final void entryRuleWMLKeyValue() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:154:1: ( ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:155:1: ruleWMLKeyValue EOF
            {
             before(grammarAccess.getWMLKeyValueRule()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue251);
            ruleWMLKeyValue();
            _fsp--;

             after(grammarAccess.getWMLKeyValueRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue258); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:162:1: ruleWMLKeyValue : ( ( rule__WMLKeyValue__Alternatives ) ) ;
    public final void ruleWMLKeyValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:166:2: ( ( ( rule__WMLKeyValue__Alternatives ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:167:1: ( ( rule__WMLKeyValue__Alternatives ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:167:1: ( ( rule__WMLKeyValue__Alternatives ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:168:1: ( rule__WMLKeyValue__Alternatives )
            {
             before(grammarAccess.getWMLKeyValueAccess().getAlternatives()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:169:1: ( rule__WMLKeyValue__Alternatives )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:169:2: rule__WMLKeyValue__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue284);
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


    // $ANTLR start entryRuleWMLMacroCall
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:181:1: entryRuleWMLMacroCall : ruleWMLMacroCall EOF ;
    public final void entryRuleWMLMacroCall() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:182:1: ( ruleWMLMacroCall EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:183:1: ruleWMLMacroCall EOF
            {
             before(grammarAccess.getWMLMacroCallRule()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall311);
            ruleWMLMacroCall();
            _fsp--;

             after(grammarAccess.getWMLMacroCallRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCall318); 

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
    // $ANTLR end entryRuleWMLMacroCall


    // $ANTLR start ruleWMLMacroCall
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:190:1: ruleWMLMacroCall : ( ( rule__WMLMacroCall__Group__0 ) ) ;
    public final void ruleWMLMacroCall() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:194:2: ( ( ( rule__WMLMacroCall__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:195:1: ( ( rule__WMLMacroCall__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:195:1: ( ( rule__WMLMacroCall__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:196:1: ( rule__WMLMacroCall__Group__0 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:197:1: ( rule__WMLMacroCall__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:197:2: rule__WMLMacroCall__Group__0
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__0_in_ruleWMLMacroCall344);
            rule__WMLMacroCall__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroCallAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLMacroCall


    // $ANTLR start entryRuleWMLLuaCode
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:209:1: entryRuleWMLLuaCode : ruleWMLLuaCode EOF ;
    public final void entryRuleWMLLuaCode() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:210:1: ( ruleWMLLuaCode EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:211:1: ruleWMLLuaCode EOF
            {
             before(grammarAccess.getWMLLuaCodeRule()); 
            pushFollow(FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode371);
            ruleWMLLuaCode();
            _fsp--;

             after(grammarAccess.getWMLLuaCodeRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLLuaCode378); 

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
    // $ANTLR end entryRuleWMLLuaCode


    // $ANTLR start ruleWMLLuaCode
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:218:1: ruleWMLLuaCode : ( ( rule__WMLLuaCode__ValueAssignment ) ) ;
    public final void ruleWMLLuaCode() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:222:2: ( ( ( rule__WMLLuaCode__ValueAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:223:1: ( ( rule__WMLLuaCode__ValueAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:223:1: ( ( rule__WMLLuaCode__ValueAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:224:1: ( rule__WMLLuaCode__ValueAssignment )
            {
             before(grammarAccess.getWMLLuaCodeAccess().getValueAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:225:1: ( rule__WMLLuaCode__ValueAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:225:2: rule__WMLLuaCode__ValueAssignment
            {
            pushFollow(FOLLOW_rule__WMLLuaCode__ValueAssignment_in_ruleWMLLuaCode404);
            rule__WMLLuaCode__ValueAssignment();
            _fsp--;


            }

             after(grammarAccess.getWMLLuaCodeAccess().getValueAssignment()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLLuaCode


    // $ANTLR start entryRuleWMLArrayCall
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:237:1: entryRuleWMLArrayCall : ruleWMLArrayCall EOF ;
    public final void entryRuleWMLArrayCall() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:238:1: ( ruleWMLArrayCall EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:239:1: ruleWMLArrayCall EOF
            {
             before(grammarAccess.getWMLArrayCallRule()); 
            pushFollow(FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall431);
            ruleWMLArrayCall();
            _fsp--;

             after(grammarAccess.getWMLArrayCallRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLArrayCall438); 

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
    // $ANTLR end entryRuleWMLArrayCall


    // $ANTLR start ruleWMLArrayCall
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:246:1: ruleWMLArrayCall : ( ( rule__WMLArrayCall__Group__0 ) ) ;
    public final void ruleWMLArrayCall() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:250:2: ( ( ( rule__WMLArrayCall__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:251:1: ( ( rule__WMLArrayCall__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:251:1: ( ( rule__WMLArrayCall__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:252:1: ( rule__WMLArrayCall__Group__0 )
            {
             before(grammarAccess.getWMLArrayCallAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:253:1: ( rule__WMLArrayCall__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:253:2: rule__WMLArrayCall__Group__0
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__0_in_ruleWMLArrayCall464);
            rule__WMLArrayCall__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLArrayCallAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLArrayCall


    // $ANTLR start entryRuleWMLMacroDefine
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:265:1: entryRuleWMLMacroDefine : ruleWMLMacroDefine EOF ;
    public final void entryRuleWMLMacroDefine() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:266:1: ( ruleWMLMacroDefine EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:267:1: ruleWMLMacroDefine EOF
            {
             before(grammarAccess.getWMLMacroDefineRule()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine491);
            ruleWMLMacroDefine();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroDefine498); 

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
    // $ANTLR end entryRuleWMLMacroDefine


    // $ANTLR start ruleWMLMacroDefine
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:274:1: ruleWMLMacroDefine : ( ( rule__WMLMacroDefine__Group__0 ) ) ;
    public final void ruleWMLMacroDefine() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:278:2: ( ( ( rule__WMLMacroDefine__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:279:1: ( ( rule__WMLMacroDefine__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:279:1: ( ( rule__WMLMacroDefine__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:280:1: ( rule__WMLMacroDefine__Group__0 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:1: ( rule__WMLMacroDefine__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:2: rule__WMLMacroDefine__Group__0
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__0_in_ruleWMLMacroDefine524);
            rule__WMLMacroDefine__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroDefineAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLMacroDefine


    // $ANTLR start entryRuleWMLTextdomain
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:293:1: entryRuleWMLTextdomain : ruleWMLTextdomain EOF ;
    public final void entryRuleWMLTextdomain() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:294:1: ( ruleWMLTextdomain EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:295:1: ruleWMLTextdomain EOF
            {
             before(grammarAccess.getWMLTextdomainRule()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain551);
            ruleWMLTextdomain();
            _fsp--;

             after(grammarAccess.getWMLTextdomainRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTextdomain558); 

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
    // $ANTLR end entryRuleWMLTextdomain


    // $ANTLR start ruleWMLTextdomain
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:302:1: ruleWMLTextdomain : ( ( rule__WMLTextdomain__NameAssignment ) ) ;
    public final void ruleWMLTextdomain() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:306:2: ( ( ( rule__WMLTextdomain__NameAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:307:1: ( ( rule__WMLTextdomain__NameAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:307:1: ( ( rule__WMLTextdomain__NameAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:308:1: ( rule__WMLTextdomain__NameAssignment )
            {
             before(grammarAccess.getWMLTextdomainAccess().getNameAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:1: ( rule__WMLTextdomain__NameAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:2: rule__WMLTextdomain__NameAssignment
            {
            pushFollow(FOLLOW_rule__WMLTextdomain__NameAssignment_in_ruleWMLTextdomain584);
            rule__WMLTextdomain__NameAssignment();
            _fsp--;


            }

             after(grammarAccess.getWMLTextdomainAccess().getNameAssignment()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLTextdomain


    // $ANTLR start entryRuleWMLValue
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:321:1: entryRuleWMLValue : ruleWMLValue EOF ;
    public final void entryRuleWMLValue() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:322:1: ( ruleWMLValue EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:323:1: ruleWMLValue EOF
            {
             before(grammarAccess.getWMLValueRule()); 
            pushFollow(FOLLOW_ruleWMLValue_in_entryRuleWMLValue611);
            ruleWMLValue();
            _fsp--;

             after(grammarAccess.getWMLValueRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValue618); 

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
    // $ANTLR end entryRuleWMLValue


    // $ANTLR start ruleWMLValue
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:330:1: ruleWMLValue : ( ( rule__WMLValue__ValueAssignment ) ) ;
    public final void ruleWMLValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:334:2: ( ( ( rule__WMLValue__ValueAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:335:1: ( ( rule__WMLValue__ValueAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:335:1: ( ( rule__WMLValue__ValueAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:336:1: ( rule__WMLValue__ValueAssignment )
            {
             before(grammarAccess.getWMLValueAccess().getValueAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:1: ( rule__WMLValue__ValueAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:2: rule__WMLValue__ValueAssignment
            {
            pushFollow(FOLLOW_rule__WMLValue__ValueAssignment_in_ruleWMLValue644);
            rule__WMLValue__ValueAssignment();
            _fsp--;


            }

             after(grammarAccess.getWMLValueAccess().getValueAssignment()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLValue


    // $ANTLR start entryRuleTSTRING
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:349:1: entryRuleTSTRING : ruleTSTRING EOF ;
    public final void entryRuleTSTRING() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:350:1: ( ruleTSTRING EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:351:1: ruleTSTRING EOF
            {
             before(grammarAccess.getTSTRINGRule()); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING671);
            ruleTSTRING();
            _fsp--;

             after(grammarAccess.getTSTRINGRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING678); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:358:1: ruleTSTRING : ( ( rule__TSTRING__Group__0 ) ) ;
    public final void ruleTSTRING() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:362:2: ( ( ( rule__TSTRING__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:363:1: ( ( rule__TSTRING__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:363:1: ( ( rule__TSTRING__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:364:1: ( rule__TSTRING__Group__0 )
            {
             before(grammarAccess.getTSTRINGAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:365:1: ( rule__TSTRING__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:365:2: rule__TSTRING__Group__0
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0_in_ruleTSTRING704);
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


    // $ANTLR start entryRuleTVAR
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:377:1: entryRuleTVAR : ruleTVAR EOF ;
    public final void entryRuleTVAR() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:378:1: ( ruleTVAR EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:379:1: ruleTVAR EOF
            {
             before(grammarAccess.getTVARRule()); 
            pushFollow(FOLLOW_ruleTVAR_in_entryRuleTVAR731);
            ruleTVAR();
            _fsp--;

             after(grammarAccess.getTVARRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTVAR738); 

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
    // $ANTLR end entryRuleTVAR


    // $ANTLR start ruleTVAR
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:386:1: ruleTVAR : ( '_$' ) ;
    public final void ruleTVAR() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:390:2: ( ( '_$' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:391:1: ( '_$' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:391:1: ( '_$' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:392:1: '_$'
            {
             before(grammarAccess.getTVARAccess().get_Keyword()); 
            match(input,14,FOLLOW_14_in_ruleTVAR765); 
             after(grammarAccess.getTVARAccess().get_Keyword()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleTVAR


    // $ANTLR start entryRuleMacroTokens
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:407:1: entryRuleMacroTokens : ruleMacroTokens EOF ;
    public final void entryRuleMacroTokens() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:408:1: ( ruleMacroTokens EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:409:1: ruleMacroTokens EOF
            {
             before(grammarAccess.getMacroTokensRule()); 
            pushFollow(FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens793);
            ruleMacroTokens();
            _fsp--;

             after(grammarAccess.getMacroTokensRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleMacroTokens800); 

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
    // $ANTLR end entryRuleMacroTokens


    // $ANTLR start ruleMacroTokens
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:416:1: ruleMacroTokens : ( ( rule__MacroTokens__ValAssignment ) ) ;
    public final void ruleMacroTokens() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:420:2: ( ( ( rule__MacroTokens__ValAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:421:1: ( ( rule__MacroTokens__ValAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:421:1: ( ( rule__MacroTokens__ValAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:422:1: ( rule__MacroTokens__ValAssignment )
            {
             before(grammarAccess.getMacroTokensAccess().getValAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:423:1: ( rule__MacroTokens__ValAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:423:2: rule__MacroTokens__ValAssignment
            {
            pushFollow(FOLLOW_rule__MacroTokens__ValAssignment_in_ruleMacroTokens826);
            rule__MacroTokens__ValAssignment();
            _fsp--;


            }

             after(grammarAccess.getMacroTokensAccess().getValAssignment()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleMacroTokens


    // $ANTLR start rule__WMLRoot__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:435:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__TagsAssignment_0 ) ) | ( ( rule__WMLRoot__MacroCallsAssignment_1 ) ) | ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) ) | ( ( rule__WMLRoot__TextdomainsAssignment_3 ) ) );
    public final void rule__WMLRoot__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:439:1: ( ( ( rule__WMLRoot__TagsAssignment_0 ) ) | ( ( rule__WMLRoot__MacroCallsAssignment_1 ) ) | ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) ) | ( ( rule__WMLRoot__TextdomainsAssignment_3 ) ) )
            int alt2=4;
            switch ( input.LA(1) ) {
            case 20:
                {
                alt2=1;
                }
                break;
            case 24:
                {
                alt2=2;
                }
                break;
            case RULE_DEFINE:
                {
                alt2=3;
                }
                break;
            case RULE_TEXTDOMAIN:
                {
                alt2=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("435:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__TagsAssignment_0 ) ) | ( ( rule__WMLRoot__MacroCallsAssignment_1 ) ) | ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) ) | ( ( rule__WMLRoot__TextdomainsAssignment_3 ) ) );", 2, 0, input);

                throw nvae;
            }

            switch (alt2) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:440:1: ( ( rule__WMLRoot__TagsAssignment_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:440:1: ( ( rule__WMLRoot__TagsAssignment_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:441:1: ( rule__WMLRoot__TagsAssignment_0 )
                    {
                     before(grammarAccess.getWMLRootAccess().getTagsAssignment_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:442:1: ( rule__WMLRoot__TagsAssignment_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:442:2: rule__WMLRoot__TagsAssignment_0
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__TagsAssignment_0_in_rule__WMLRoot__Alternatives862);
                    rule__WMLRoot__TagsAssignment_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getTagsAssignment_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:446:6: ( ( rule__WMLRoot__MacroCallsAssignment_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:446:6: ( ( rule__WMLRoot__MacroCallsAssignment_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:447:1: ( rule__WMLRoot__MacroCallsAssignment_1 )
                    {
                     before(grammarAccess.getWMLRootAccess().getMacroCallsAssignment_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:448:1: ( rule__WMLRoot__MacroCallsAssignment_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:448:2: rule__WMLRoot__MacroCallsAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__MacroCallsAssignment_1_in_rule__WMLRoot__Alternatives880);
                    rule__WMLRoot__MacroCallsAssignment_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getMacroCallsAssignment_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:452:6: ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:452:6: ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:453:1: ( rule__WMLRoot__MacroDefinesAssignment_2 )
                    {
                     before(grammarAccess.getWMLRootAccess().getMacroDefinesAssignment_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:454:1: ( rule__WMLRoot__MacroDefinesAssignment_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:454:2: rule__WMLRoot__MacroDefinesAssignment_2
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__MacroDefinesAssignment_2_in_rule__WMLRoot__Alternatives898);
                    rule__WMLRoot__MacroDefinesAssignment_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getMacroDefinesAssignment_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:458:6: ( ( rule__WMLRoot__TextdomainsAssignment_3 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:458:6: ( ( rule__WMLRoot__TextdomainsAssignment_3 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:459:1: ( rule__WMLRoot__TextdomainsAssignment_3 )
                    {
                     before(grammarAccess.getWMLRootAccess().getTextdomainsAssignment_3()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:460:1: ( rule__WMLRoot__TextdomainsAssignment_3 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:460:2: rule__WMLRoot__TextdomainsAssignment_3
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__TextdomainsAssignment_3_in_rule__WMLRoot__Alternatives916);
                    rule__WMLRoot__TextdomainsAssignment_3();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getTextdomainsAssignment_3()); 

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


    // $ANTLR start rule__WMLTag__Alternatives_4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:469:1: rule__WMLTag__Alternatives_4 : ( ( ( rule__WMLTag__TagsAssignment_4_0 ) ) | ( ( rule__WMLTag__KeysAssignment_4_1 ) ) | ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) ) | ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) ) | ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) ) );
    public final void rule__WMLTag__Alternatives_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:473:1: ( ( ( rule__WMLTag__TagsAssignment_4_0 ) ) | ( ( rule__WMLTag__KeysAssignment_4_1 ) ) | ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) ) | ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) ) | ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) ) )
            int alt3=5;
            switch ( input.LA(1) ) {
            case 20:
                {
                alt3=1;
                }
                break;
            case RULE_ID:
                {
                alt3=2;
                }
                break;
            case 24:
                {
                alt3=3;
                }
                break;
            case RULE_DEFINE:
                {
                alt3=4;
                }
                break;
            case RULE_TEXTDOMAIN:
                {
                alt3=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("469:1: rule__WMLTag__Alternatives_4 : ( ( ( rule__WMLTag__TagsAssignment_4_0 ) ) | ( ( rule__WMLTag__KeysAssignment_4_1 ) ) | ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) ) | ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) ) | ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) ) );", 3, 0, input);

                throw nvae;
            }

            switch (alt3) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:474:1: ( ( rule__WMLTag__TagsAssignment_4_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:474:1: ( ( rule__WMLTag__TagsAssignment_4_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:475:1: ( rule__WMLTag__TagsAssignment_4_0 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTagsAssignment_4_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:476:1: ( rule__WMLTag__TagsAssignment_4_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:476:2: rule__WMLTag__TagsAssignment_4_0
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TagsAssignment_4_0_in_rule__WMLTag__Alternatives_4949);
                    rule__WMLTag__TagsAssignment_4_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTagsAssignment_4_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:480:6: ( ( rule__WMLTag__KeysAssignment_4_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:480:6: ( ( rule__WMLTag__KeysAssignment_4_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:481:1: ( rule__WMLTag__KeysAssignment_4_1 )
                    {
                     before(grammarAccess.getWMLTagAccess().getKeysAssignment_4_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:482:1: ( rule__WMLTag__KeysAssignment_4_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:482:2: rule__WMLTag__KeysAssignment_4_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__KeysAssignment_4_1_in_rule__WMLTag__Alternatives_4967);
                    rule__WMLTag__KeysAssignment_4_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getKeysAssignment_4_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:486:6: ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:486:6: ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:487:1: ( rule__WMLTag__MacroCallsAssignment_4_2 )
                    {
                     before(grammarAccess.getWMLTagAccess().getMacroCallsAssignment_4_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:488:1: ( rule__WMLTag__MacroCallsAssignment_4_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:488:2: rule__WMLTag__MacroCallsAssignment_4_2
                    {
                    pushFollow(FOLLOW_rule__WMLTag__MacroCallsAssignment_4_2_in_rule__WMLTag__Alternatives_4985);
                    rule__WMLTag__MacroCallsAssignment_4_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getMacroCallsAssignment_4_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:492:6: ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:492:6: ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:493:1: ( rule__WMLTag__MacroDefinesAssignment_4_3 )
                    {
                     before(grammarAccess.getWMLTagAccess().getMacroDefinesAssignment_4_3()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:494:1: ( rule__WMLTag__MacroDefinesAssignment_4_3 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:494:2: rule__WMLTag__MacroDefinesAssignment_4_3
                    {
                    pushFollow(FOLLOW_rule__WMLTag__MacroDefinesAssignment_4_3_in_rule__WMLTag__Alternatives_41003);
                    rule__WMLTag__MacroDefinesAssignment_4_3();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getMacroDefinesAssignment_4_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:498:6: ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:498:6: ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:499:1: ( rule__WMLTag__TextdomainsAssignment_4_4 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTextdomainsAssignment_4_4()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:500:1: ( rule__WMLTag__TextdomainsAssignment_4_4 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:500:2: rule__WMLTag__TextdomainsAssignment_4_4
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TextdomainsAssignment_4_4_in_rule__WMLTag__Alternatives_41021);
                    rule__WMLTag__TextdomainsAssignment_4_4();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTextdomainsAssignment_4_4()); 

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
    // $ANTLR end rule__WMLTag__Alternatives_4


    // $ANTLR start rule__WMLKey__Alternatives_4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:509:1: rule__WMLKey__Alternatives_4 : ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) );
    public final void rule__WMLKey__Alternatives_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:513:1: ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) )
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( (LA4_0==RULE_EOL) ) {
                alt4=1;
            }
            else if ( (LA4_0==RULE_SL_COMMENT) ) {
                alt4=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("509:1: rule__WMLKey__Alternatives_4 : ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) );", 4, 0, input);

                throw nvae;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:514:1: ( RULE_EOL )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:514:1: ( RULE_EOL )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:515:1: RULE_EOL
                    {
                     before(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_4_0()); 
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__Alternatives_41054); 
                     after(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_4_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:520:6: ( RULE_SL_COMMENT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:520:6: ( RULE_SL_COMMENT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:521:1: RULE_SL_COMMENT
                    {
                     before(grammarAccess.getWMLKeyAccess().getSL_COMMENTTerminalRuleCall_4_1()); 
                    match(input,RULE_SL_COMMENT,FOLLOW_RULE_SL_COMMENT_in_rule__WMLKey__Alternatives_41071); 
                     after(grammarAccess.getWMLKeyAccess().getSL_COMMENTTerminalRuleCall_4_1()); 

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
    // $ANTLR end rule__WMLKey__Alternatives_4


    // $ANTLR start rule__WMLKeyValue__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:531:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) );
    public final void rule__WMLKeyValue__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:535:1: ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) )
            int alt5=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
            case RULE_STRING:
            case RULE_ANY_OTHER:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 27:
                {
                alt5=1;
                }
                break;
            case 24:
                {
                alt5=2;
                }
                break;
            case RULE_LUA_CODE:
                {
                alt5=3;
                }
                break;
            case 20:
                {
                alt5=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("531:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) );", 5, 0, input);

                throw nvae;
            }

            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:536:1: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:536:1: ( ruleWMLValue )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:537:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLKeyValue__Alternatives1103);
                    ruleWMLValue();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:542:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:542:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:543:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives1120);
                    ruleWMLMacroCall();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:548:6: ( ruleWMLLuaCode )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:548:6: ( ruleWMLLuaCode )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:549:1: ruleWMLLuaCode
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2()); 
                    pushFollow(FOLLOW_ruleWMLLuaCode_in_rule__WMLKeyValue__Alternatives1137);
                    ruleWMLLuaCode();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:554:6: ( ruleWMLArrayCall )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:554:6: ( ruleWMLArrayCall )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:555:1: ruleWMLArrayCall
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLArrayCallParserRuleCall_3()); 
                    pushFollow(FOLLOW_ruleWMLArrayCall_in_rule__WMLKeyValue__Alternatives1154);
                    ruleWMLArrayCall();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLArrayCallParserRuleCall_3()); 

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


    // $ANTLR start rule__WMLMacroCall__Alternatives_4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:565:1: rule__WMLMacroCall__Alternatives_4 : ( ( ( rule__WMLMacroCall__ParamsAssignment_4_0 ) ) | ( ( rule__WMLMacroCall__ExtraMacrosAssignment_4_1 ) ) );
    public final void rule__WMLMacroCall__Alternatives_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:569:1: ( ( ( rule__WMLMacroCall__ParamsAssignment_4_0 ) ) | ( ( rule__WMLMacroCall__ExtraMacrosAssignment_4_1 ) ) )
            int alt6=2;
            int LA6_0 = input.LA(1);

            if ( ((LA6_0>=RULE_ID && LA6_0<=RULE_ANY_OTHER)||(LA6_0>=14 && LA6_0<=23)||LA6_0==27) ) {
                alt6=1;
            }
            else if ( (LA6_0==24) ) {
                alt6=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("565:1: rule__WMLMacroCall__Alternatives_4 : ( ( ( rule__WMLMacroCall__ParamsAssignment_4_0 ) ) | ( ( rule__WMLMacroCall__ExtraMacrosAssignment_4_1 ) ) );", 6, 0, input);

                throw nvae;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:570:1: ( ( rule__WMLMacroCall__ParamsAssignment_4_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:570:1: ( ( rule__WMLMacroCall__ParamsAssignment_4_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:571:1: ( rule__WMLMacroCall__ParamsAssignment_4_0 )
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getParamsAssignment_4_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:572:1: ( rule__WMLMacroCall__ParamsAssignment_4_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:572:2: rule__WMLMacroCall__ParamsAssignment_4_0
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__ParamsAssignment_4_0_in_rule__WMLMacroCall__Alternatives_41186);
                    rule__WMLMacroCall__ParamsAssignment_4_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroCallAccess().getParamsAssignment_4_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:576:6: ( ( rule__WMLMacroCall__ExtraMacrosAssignment_4_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:576:6: ( ( rule__WMLMacroCall__ExtraMacrosAssignment_4_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:577:1: ( rule__WMLMacroCall__ExtraMacrosAssignment_4_1 )
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getExtraMacrosAssignment_4_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:578:1: ( rule__WMLMacroCall__ExtraMacrosAssignment_4_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:578:2: rule__WMLMacroCall__ExtraMacrosAssignment_4_1
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__ExtraMacrosAssignment_4_1_in_rule__WMLMacroCall__Alternatives_41204);
                    rule__WMLMacroCall__ExtraMacrosAssignment_4_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroCallAccess().getExtraMacrosAssignment_4_1()); 

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
    // $ANTLR end rule__WMLMacroCall__Alternatives_4


    // $ANTLR start rule__WMLMacroCall__ParamsAlternatives_4_0_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:587:1: rule__WMLMacroCall__ParamsAlternatives_4_0_0 : ( ( ruleWMLValue ) | ( ruleMacroTokens ) );
    public final void rule__WMLMacroCall__ParamsAlternatives_4_0_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:591:1: ( ( ruleWMLValue ) | ( ruleMacroTokens ) )
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( ((LA7_0>=RULE_ID && LA7_0<=RULE_ANY_OTHER)||(LA7_0>=14 && LA7_0<=18)||LA7_0==27) ) {
                alt7=1;
            }
            else if ( ((LA7_0>=19 && LA7_0<=23)) ) {
                alt7=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("587:1: rule__WMLMacroCall__ParamsAlternatives_4_0_0 : ( ( ruleWMLValue ) | ( ruleMacroTokens ) );", 7, 0, input);

                throw nvae;
            }
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:592:1: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:592:1: ( ruleWMLValue )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:593:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getParamsWMLValueParserRuleCall_4_0_0_0()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLMacroCall__ParamsAlternatives_4_0_01237);
                    ruleWMLValue();
                    _fsp--;

                     after(grammarAccess.getWMLMacroCallAccess().getParamsWMLValueParserRuleCall_4_0_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:598:6: ( ruleMacroTokens )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:598:6: ( ruleMacroTokens )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:599:1: ruleMacroTokens
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getParamsMacroTokensParserRuleCall_4_0_0_1()); 
                    pushFollow(FOLLOW_ruleMacroTokens_in_rule__WMLMacroCall__ParamsAlternatives_4_0_01254);
                    ruleMacroTokens();
                    _fsp--;

                     after(grammarAccess.getWMLMacroCallAccess().getParamsMacroTokensParserRuleCall_4_0_0_1()); 

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
    // $ANTLR end rule__WMLMacroCall__ParamsAlternatives_4_0_0


    // $ANTLR start rule__WMLMacroDefine__Alternatives_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:609:1: rule__WMLMacroDefine__Alternatives_1 : ( ( ( rule__WMLMacroDefine__TagsAssignment_1_0 ) ) | ( ( rule__WMLMacroDefine__KeysAssignment_1_1 ) ) | ( ( rule__WMLMacroDefine__MacroCallsAssignment_1_2 ) ) | ( ( rule__WMLMacroDefine__MacroDefinesAssignment_1_3 ) ) | ( ( rule__WMLMacroDefine__TextdomainsAssignment_1_4 ) ) | ( ( rule__WMLMacroDefine__ValuesAssignment_1_5 ) ) );
    public final void rule__WMLMacroDefine__Alternatives_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:613:1: ( ( ( rule__WMLMacroDefine__TagsAssignment_1_0 ) ) | ( ( rule__WMLMacroDefine__KeysAssignment_1_1 ) ) | ( ( rule__WMLMacroDefine__MacroCallsAssignment_1_2 ) ) | ( ( rule__WMLMacroDefine__MacroDefinesAssignment_1_3 ) ) | ( ( rule__WMLMacroDefine__TextdomainsAssignment_1_4 ) ) | ( ( rule__WMLMacroDefine__ValuesAssignment_1_5 ) ) )
            int alt8=6;
            switch ( input.LA(1) ) {
            case 20:
                {
                alt8=1;
                }
                break;
            case RULE_ID:
                {
                int LA8_2 = input.LA(2);

                if ( ((LA8_2>=RULE_ID && LA8_2<=RULE_ENDDEF)||(LA8_2>=RULE_DEFINE && LA8_2<=RULE_TEXTDOMAIN)||(LA8_2>=14 && LA8_2<=18)||LA8_2==20||LA8_2==24||LA8_2==27) ) {
                    alt8=6;
                }
                else if ( (LA8_2==19) ) {
                    alt8=2;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("609:1: rule__WMLMacroDefine__Alternatives_1 : ( ( ( rule__WMLMacroDefine__TagsAssignment_1_0 ) ) | ( ( rule__WMLMacroDefine__KeysAssignment_1_1 ) ) | ( ( rule__WMLMacroDefine__MacroCallsAssignment_1_2 ) ) | ( ( rule__WMLMacroDefine__MacroDefinesAssignment_1_3 ) ) | ( ( rule__WMLMacroDefine__TextdomainsAssignment_1_4 ) ) | ( ( rule__WMLMacroDefine__ValuesAssignment_1_5 ) ) );", 8, 2, input);

                    throw nvae;
                }
                }
                break;
            case 24:
                {
                alt8=3;
                }
                break;
            case RULE_DEFINE:
                {
                alt8=4;
                }
                break;
            case RULE_TEXTDOMAIN:
                {
                alt8=5;
                }
                break;
            case RULE_STRING:
            case RULE_ANY_OTHER:
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
            case 27:
                {
                alt8=6;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("609:1: rule__WMLMacroDefine__Alternatives_1 : ( ( ( rule__WMLMacroDefine__TagsAssignment_1_0 ) ) | ( ( rule__WMLMacroDefine__KeysAssignment_1_1 ) ) | ( ( rule__WMLMacroDefine__MacroCallsAssignment_1_2 ) ) | ( ( rule__WMLMacroDefine__MacroDefinesAssignment_1_3 ) ) | ( ( rule__WMLMacroDefine__TextdomainsAssignment_1_4 ) ) | ( ( rule__WMLMacroDefine__ValuesAssignment_1_5 ) ) );", 8, 0, input);

                throw nvae;
            }

            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:614:1: ( ( rule__WMLMacroDefine__TagsAssignment_1_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:614:1: ( ( rule__WMLMacroDefine__TagsAssignment_1_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:615:1: ( rule__WMLMacroDefine__TagsAssignment_1_0 )
                    {
                     before(grammarAccess.getWMLMacroDefineAccess().getTagsAssignment_1_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:616:1: ( rule__WMLMacroDefine__TagsAssignment_1_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:616:2: rule__WMLMacroDefine__TagsAssignment_1_0
                    {
                    pushFollow(FOLLOW_rule__WMLMacroDefine__TagsAssignment_1_0_in_rule__WMLMacroDefine__Alternatives_11286);
                    rule__WMLMacroDefine__TagsAssignment_1_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroDefineAccess().getTagsAssignment_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:620:6: ( ( rule__WMLMacroDefine__KeysAssignment_1_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:620:6: ( ( rule__WMLMacroDefine__KeysAssignment_1_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:621:1: ( rule__WMLMacroDefine__KeysAssignment_1_1 )
                    {
                     before(grammarAccess.getWMLMacroDefineAccess().getKeysAssignment_1_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:622:1: ( rule__WMLMacroDefine__KeysAssignment_1_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:622:2: rule__WMLMacroDefine__KeysAssignment_1_1
                    {
                    pushFollow(FOLLOW_rule__WMLMacroDefine__KeysAssignment_1_1_in_rule__WMLMacroDefine__Alternatives_11304);
                    rule__WMLMacroDefine__KeysAssignment_1_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroDefineAccess().getKeysAssignment_1_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:626:6: ( ( rule__WMLMacroDefine__MacroCallsAssignment_1_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:626:6: ( ( rule__WMLMacroDefine__MacroCallsAssignment_1_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:627:1: ( rule__WMLMacroDefine__MacroCallsAssignment_1_2 )
                    {
                     before(grammarAccess.getWMLMacroDefineAccess().getMacroCallsAssignment_1_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:628:1: ( rule__WMLMacroDefine__MacroCallsAssignment_1_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:628:2: rule__WMLMacroDefine__MacroCallsAssignment_1_2
                    {
                    pushFollow(FOLLOW_rule__WMLMacroDefine__MacroCallsAssignment_1_2_in_rule__WMLMacroDefine__Alternatives_11322);
                    rule__WMLMacroDefine__MacroCallsAssignment_1_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroDefineAccess().getMacroCallsAssignment_1_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:632:6: ( ( rule__WMLMacroDefine__MacroDefinesAssignment_1_3 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:632:6: ( ( rule__WMLMacroDefine__MacroDefinesAssignment_1_3 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:633:1: ( rule__WMLMacroDefine__MacroDefinesAssignment_1_3 )
                    {
                     before(grammarAccess.getWMLMacroDefineAccess().getMacroDefinesAssignment_1_3()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:634:1: ( rule__WMLMacroDefine__MacroDefinesAssignment_1_3 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:634:2: rule__WMLMacroDefine__MacroDefinesAssignment_1_3
                    {
                    pushFollow(FOLLOW_rule__WMLMacroDefine__MacroDefinesAssignment_1_3_in_rule__WMLMacroDefine__Alternatives_11340);
                    rule__WMLMacroDefine__MacroDefinesAssignment_1_3();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroDefineAccess().getMacroDefinesAssignment_1_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:638:6: ( ( rule__WMLMacroDefine__TextdomainsAssignment_1_4 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:638:6: ( ( rule__WMLMacroDefine__TextdomainsAssignment_1_4 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:639:1: ( rule__WMLMacroDefine__TextdomainsAssignment_1_4 )
                    {
                     before(grammarAccess.getWMLMacroDefineAccess().getTextdomainsAssignment_1_4()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:640:1: ( rule__WMLMacroDefine__TextdomainsAssignment_1_4 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:640:2: rule__WMLMacroDefine__TextdomainsAssignment_1_4
                    {
                    pushFollow(FOLLOW_rule__WMLMacroDefine__TextdomainsAssignment_1_4_in_rule__WMLMacroDefine__Alternatives_11358);
                    rule__WMLMacroDefine__TextdomainsAssignment_1_4();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroDefineAccess().getTextdomainsAssignment_1_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:644:6: ( ( rule__WMLMacroDefine__ValuesAssignment_1_5 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:644:6: ( ( rule__WMLMacroDefine__ValuesAssignment_1_5 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:645:1: ( rule__WMLMacroDefine__ValuesAssignment_1_5 )
                    {
                     before(grammarAccess.getWMLMacroDefineAccess().getValuesAssignment_1_5()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:646:1: ( rule__WMLMacroDefine__ValuesAssignment_1_5 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:646:2: rule__WMLMacroDefine__ValuesAssignment_1_5
                    {
                    pushFollow(FOLLOW_rule__WMLMacroDefine__ValuesAssignment_1_5_in_rule__WMLMacroDefine__Alternatives_11376);
                    rule__WMLMacroDefine__ValuesAssignment_1_5();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroDefineAccess().getValuesAssignment_1_5()); 

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
    // $ANTLR end rule__WMLMacroDefine__Alternatives_1


    // $ANTLR start rule__WMLValue__ValueAlternatives_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:655:1: rule__WMLValue__ValueAlternatives_0 : ( ( RULE_ID ) | ( ruleTSTRING ) | ( ruleTVAR ) | ( RULE_STRING ) | ( '~' ) | ( '.' ) | ( '(' ) | ( ')' ) | ( RULE_ANY_OTHER ) );
    public final void rule__WMLValue__ValueAlternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:659:1: ( ( RULE_ID ) | ( ruleTSTRING ) | ( ruleTVAR ) | ( RULE_STRING ) | ( '~' ) | ( '.' ) | ( '(' ) | ( ')' ) | ( RULE_ANY_OTHER ) )
            int alt9=9;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt9=1;
                }
                break;
            case 27:
                {
                alt9=2;
                }
                break;
            case 14:
                {
                alt9=3;
                }
                break;
            case RULE_STRING:
                {
                alt9=4;
                }
                break;
            case 15:
                {
                alt9=5;
                }
                break;
            case 16:
                {
                alt9=6;
                }
                break;
            case 17:
                {
                alt9=7;
                }
                break;
            case 18:
                {
                alt9=8;
                }
                break;
            case RULE_ANY_OTHER:
                {
                alt9=9;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("655:1: rule__WMLValue__ValueAlternatives_0 : ( ( RULE_ID ) | ( ruleTSTRING ) | ( ruleTVAR ) | ( RULE_STRING ) | ( '~' ) | ( '.' ) | ( '(' ) | ( ')' ) | ( RULE_ANY_OTHER ) );", 9, 0, input);

                throw nvae;
            }

            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:660:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:660:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:661:1: RULE_ID
                    {
                     before(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLValue__ValueAlternatives_01409); 
                     after(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:666:6: ( ruleTSTRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:666:6: ( ruleTSTRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:667:1: ruleTSTRING
                    {
                     before(grammarAccess.getWMLValueAccess().getValueTSTRINGParserRuleCall_0_1()); 
                    pushFollow(FOLLOW_ruleTSTRING_in_rule__WMLValue__ValueAlternatives_01426);
                    ruleTSTRING();
                    _fsp--;

                     after(grammarAccess.getWMLValueAccess().getValueTSTRINGParserRuleCall_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:672:6: ( ruleTVAR )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:672:6: ( ruleTVAR )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:673:1: ruleTVAR
                    {
                     before(grammarAccess.getWMLValueAccess().getValueTVARParserRuleCall_0_2()); 
                    pushFollow(FOLLOW_ruleTVAR_in_rule__WMLValue__ValueAlternatives_01443);
                    ruleTVAR();
                    _fsp--;

                     after(grammarAccess.getWMLValueAccess().getValueTVARParserRuleCall_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:678:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:678:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:679:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_3()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLValue__ValueAlternatives_01460); 
                     after(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:684:6: ( '~' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:684:6: ( '~' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:685:1: '~'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_4()); 
                    match(input,15,FOLLOW_15_in_rule__WMLValue__ValueAlternatives_01478); 
                     after(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:692:6: ( '.' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:692:6: ( '.' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:693:1: '.'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueFullStopKeyword_0_5()); 
                    match(input,16,FOLLOW_16_in_rule__WMLValue__ValueAlternatives_01498); 
                     after(grammarAccess.getWMLValueAccess().getValueFullStopKeyword_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:700:6: ( '(' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:700:6: ( '(' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:701:1: '('
                    {
                     before(grammarAccess.getWMLValueAccess().getValueLeftParenthesisKeyword_0_6()); 
                    match(input,17,FOLLOW_17_in_rule__WMLValue__ValueAlternatives_01518); 
                     after(grammarAccess.getWMLValueAccess().getValueLeftParenthesisKeyword_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:708:6: ( ')' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:708:6: ( ')' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:709:1: ')'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueRightParenthesisKeyword_0_7()); 
                    match(input,18,FOLLOW_18_in_rule__WMLValue__ValueAlternatives_01538); 
                     after(grammarAccess.getWMLValueAccess().getValueRightParenthesisKeyword_0_7()); 

                    }


                    }
                    break;
                case 9 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:716:6: ( RULE_ANY_OTHER )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:716:6: ( RULE_ANY_OTHER )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:717:1: RULE_ANY_OTHER
                    {
                     before(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_8()); 
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__ValueAlternatives_01557); 
                     after(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_8()); 

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
    // $ANTLR end rule__WMLValue__ValueAlternatives_0


    // $ANTLR start rule__MacroTokens__ValAlternatives_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:727:1: rule__MacroTokens__ValAlternatives_0 : ( ( '=' ) | ( '[' ) | ( ']' ) | ( '+' ) | ( '[/' ) );
    public final void rule__MacroTokens__ValAlternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:731:1: ( ( '=' ) | ( '[' ) | ( ']' ) | ( '+' ) | ( '[/' ) )
            int alt10=5;
            switch ( input.LA(1) ) {
            case 19:
                {
                alt10=1;
                }
                break;
            case 20:
                {
                alt10=2;
                }
                break;
            case 21:
                {
                alt10=3;
                }
                break;
            case 22:
                {
                alt10=4;
                }
                break;
            case 23:
                {
                alt10=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("727:1: rule__MacroTokens__ValAlternatives_0 : ( ( '=' ) | ( '[' ) | ( ']' ) | ( '+' ) | ( '[/' ) );", 10, 0, input);

                throw nvae;
            }

            switch (alt10) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:732:1: ( '=' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:732:1: ( '=' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:733:1: '='
                    {
                     before(grammarAccess.getMacroTokensAccess().getValEqualsSignKeyword_0_0()); 
                    match(input,19,FOLLOW_19_in_rule__MacroTokens__ValAlternatives_01590); 
                     after(grammarAccess.getMacroTokensAccess().getValEqualsSignKeyword_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:740:6: ( '[' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:740:6: ( '[' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:741:1: '['
                    {
                     before(grammarAccess.getMacroTokensAccess().getValLeftSquareBracketKeyword_0_1()); 
                    match(input,20,FOLLOW_20_in_rule__MacroTokens__ValAlternatives_01610); 
                     after(grammarAccess.getMacroTokensAccess().getValLeftSquareBracketKeyword_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:748:6: ( ']' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:748:6: ( ']' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:749:1: ']'
                    {
                     before(grammarAccess.getMacroTokensAccess().getValRightSquareBracketKeyword_0_2()); 
                    match(input,21,FOLLOW_21_in_rule__MacroTokens__ValAlternatives_01630); 
                     after(grammarAccess.getMacroTokensAccess().getValRightSquareBracketKeyword_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:756:6: ( '+' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:756:6: ( '+' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:757:1: '+'
                    {
                     before(grammarAccess.getMacroTokensAccess().getValPlusSignKeyword_0_3()); 
                    match(input,22,FOLLOW_22_in_rule__MacroTokens__ValAlternatives_01650); 
                     after(grammarAccess.getMacroTokensAccess().getValPlusSignKeyword_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:764:6: ( '[/' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:764:6: ( '[/' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:765:1: '[/'
                    {
                     before(grammarAccess.getMacroTokensAccess().getValLeftSquareBracketSolidusKeyword_0_4()); 
                    match(input,23,FOLLOW_23_in_rule__MacroTokens__ValAlternatives_01670); 
                     after(grammarAccess.getMacroTokensAccess().getValLeftSquareBracketSolidusKeyword_0_4()); 

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
    // $ANTLR end rule__MacroTokens__ValAlternatives_0


    // $ANTLR start rule__WMLTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:779:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:783:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:784:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01702);
            rule__WMLTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01705);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:791:1: rule__WMLTag__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:795:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:796:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:796:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:797:1: '['
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0()); 
            match(input,20,FOLLOW_20_in_rule__WMLTag__Group__0__Impl1733); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:810:1: rule__WMLTag__Group__1 : rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 ;
    public final void rule__WMLTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:814:1: ( rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:815:2: rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__11764);
            rule__WMLTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__11767);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:822:1: rule__WMLTag__Group__1__Impl : ( ( rule__WMLTag__PlusAssignment_1 )? ) ;
    public final void rule__WMLTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:826:1: ( ( ( rule__WMLTag__PlusAssignment_1 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:827:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:827:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:828:1: ( rule__WMLTag__PlusAssignment_1 )?
            {
             before(grammarAccess.getWMLTagAccess().getPlusAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:829:1: ( rule__WMLTag__PlusAssignment_1 )?
            int alt11=2;
            int LA11_0 = input.LA(1);

            if ( (LA11_0==22) ) {
                alt11=1;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:829:2: rule__WMLTag__PlusAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl1794);
                    rule__WMLTag__PlusAssignment_1();
                    _fsp--;


                    }
                    break;

            }

             after(grammarAccess.getWMLTagAccess().getPlusAssignment_1()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:839:1: rule__WMLTag__Group__2 : rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 ;
    public final void rule__WMLTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:843:1: ( rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:844:2: rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__21825);
            rule__WMLTag__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__21828);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:851:1: rule__WMLTag__Group__2__Impl : ( ( rule__WMLTag__NameAssignment_2 ) ) ;
    public final void rule__WMLTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:855:1: ( ( ( rule__WMLTag__NameAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:856:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:856:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:857:1: ( rule__WMLTag__NameAssignment_2 )
            {
             before(grammarAccess.getWMLTagAccess().getNameAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:858:1: ( rule__WMLTag__NameAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:858:2: rule__WMLTag__NameAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl1855);
            rule__WMLTag__NameAssignment_2();
            _fsp--;


            }

             after(grammarAccess.getWMLTagAccess().getNameAssignment_2()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:868:1: rule__WMLTag__Group__3 : rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 ;
    public final void rule__WMLTag__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:872:1: ( rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:873:2: rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__31885);
            rule__WMLTag__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__31888);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:880:1: rule__WMLTag__Group__3__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:884:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:885:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:885:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:886:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3()); 
            match(input,21,FOLLOW_21_in_rule__WMLTag__Group__3__Impl1916); 
             after(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:899:1: rule__WMLTag__Group__4 : rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 ;
    public final void rule__WMLTag__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:903:1: ( rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:904:2: rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__41947);
            rule__WMLTag__Group__4__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__41950);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:911:1: rule__WMLTag__Group__4__Impl : ( ( rule__WMLTag__Alternatives_4 )* ) ;
    public final void rule__WMLTag__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:915:1: ( ( ( rule__WMLTag__Alternatives_4 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:916:1: ( ( rule__WMLTag__Alternatives_4 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:916:1: ( ( rule__WMLTag__Alternatives_4 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:917:1: ( rule__WMLTag__Alternatives_4 )*
            {
             before(grammarAccess.getWMLTagAccess().getAlternatives_4()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:918:1: ( rule__WMLTag__Alternatives_4 )*
            loop12:
            do {
                int alt12=2;
                int LA12_0 = input.LA(1);

                if ( (LA12_0==RULE_ID||(LA12_0>=RULE_DEFINE && LA12_0<=RULE_TEXTDOMAIN)||LA12_0==20||LA12_0==24) ) {
                    alt12=1;
                }


                switch (alt12) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:918:2: rule__WMLTag__Alternatives_4
            	    {
            	    pushFollow(FOLLOW_rule__WMLTag__Alternatives_4_in_rule__WMLTag__Group__4__Impl1977);
            	    rule__WMLTag__Alternatives_4();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop12;
                }
            } while (true);

             after(grammarAccess.getWMLTagAccess().getAlternatives_4()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:928:1: rule__WMLTag__Group__5 : rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 ;
    public final void rule__WMLTag__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:932:1: ( rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:933:2: rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__52008);
            rule__WMLTag__Group__5__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__52011);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:940:1: rule__WMLTag__Group__5__Impl : ( '[/' ) ;
    public final void rule__WMLTag__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:944:1: ( ( '[/' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:945:1: ( '[/' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:945:1: ( '[/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:946:1: '[/'
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5()); 
            match(input,23,FOLLOW_23_in_rule__WMLTag__Group__5__Impl2039); 
             after(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:959:1: rule__WMLTag__Group__6 : rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 ;
    public final void rule__WMLTag__Group__6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:963:1: ( rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:964:2: rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__62070);
            rule__WMLTag__Group__6__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__62073);
            rule__WMLTag__Group__7();
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:971:1: rule__WMLTag__Group__6__Impl : ( ( rule__WMLTag__EndNameAssignment_6 ) ) ;
    public final void rule__WMLTag__Group__6__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:975:1: ( ( ( rule__WMLTag__EndNameAssignment_6 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:976:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:976:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:977:1: ( rule__WMLTag__EndNameAssignment_6 )
            {
             before(grammarAccess.getWMLTagAccess().getEndNameAssignment_6()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:978:1: ( rule__WMLTag__EndNameAssignment_6 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:978:2: rule__WMLTag__EndNameAssignment_6
            {
            pushFollow(FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl2100);
            rule__WMLTag__EndNameAssignment_6();
            _fsp--;


            }

             after(grammarAccess.getWMLTagAccess().getEndNameAssignment_6()); 

            }


            }

        }
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


    // $ANTLR start rule__WMLTag__Group__7
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:988:1: rule__WMLTag__Group__7 : rule__WMLTag__Group__7__Impl ;
    public final void rule__WMLTag__Group__7() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:992:1: ( rule__WMLTag__Group__7__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:993:2: rule__WMLTag__Group__7__Impl
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__72130);
            rule__WMLTag__Group__7__Impl();
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
    // $ANTLR end rule__WMLTag__Group__7


    // $ANTLR start rule__WMLTag__Group__7__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:999:1: rule__WMLTag__Group__7__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__7__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1003:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1004:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1004:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1005:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_7()); 
            match(input,21,FOLLOW_21_in_rule__WMLTag__Group__7__Impl2158); 
             after(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_7()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__Group__7__Impl


    // $ANTLR start rule__WMLKey__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1034:1: rule__WMLKey__Group__0 : rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 ;
    public final void rule__WMLKey__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1038:1: ( rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1039:2: rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02205);
            rule__WMLKey__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02208);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1046:1: rule__WMLKey__Group__0__Impl : ( ( rule__WMLKey__NameAssignment_0 ) ) ;
    public final void rule__WMLKey__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1050:1: ( ( ( rule__WMLKey__NameAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1051:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1051:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1052:1: ( rule__WMLKey__NameAssignment_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getNameAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1053:1: ( rule__WMLKey__NameAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1053:2: rule__WMLKey__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl2235);
            rule__WMLKey__NameAssignment_0();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getNameAssignment_0()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1063:1: rule__WMLKey__Group__1 : rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 ;
    public final void rule__WMLKey__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1067:1: ( rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1068:2: rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12265);
            rule__WMLKey__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12268);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1075:1: rule__WMLKey__Group__1__Impl : ( '=' ) ;
    public final void rule__WMLKey__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1079:1: ( ( '=' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1080:1: ( '=' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1080:1: ( '=' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1081:1: '='
            {
             before(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1()); 
            match(input,19,FOLLOW_19_in_rule__WMLKey__Group__1__Impl2296); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1094:1: rule__WMLKey__Group__2 : rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 ;
    public final void rule__WMLKey__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1098:1: ( rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1099:2: rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22327);
            rule__WMLKey__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__22330);
            rule__WMLKey__Group__3();
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1106:1: rule__WMLKey__Group__2__Impl : ( ( rule__WMLKey__ValueAssignment_2 )* ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1110:1: ( ( ( rule__WMLKey__ValueAssignment_2 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1111:1: ( ( rule__WMLKey__ValueAssignment_2 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1111:1: ( ( rule__WMLKey__ValueAssignment_2 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1112:1: ( rule__WMLKey__ValueAssignment_2 )*
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1113:1: ( rule__WMLKey__ValueAssignment_2 )*
            loop13:
            do {
                int alt13=2;
                int LA13_0 = input.LA(1);

                if ( ((LA13_0>=RULE_ID && LA13_0<=RULE_ANY_OTHER)||LA13_0==RULE_LUA_CODE||(LA13_0>=14 && LA13_0<=18)||LA13_0==20||LA13_0==24||LA13_0==27) ) {
                    alt13=1;
                }


                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1113:2: rule__WMLKey__ValueAssignment_2
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2357);
            	    rule__WMLKey__ValueAssignment_2();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop13;
                }
            } while (true);

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


    // $ANTLR start rule__WMLKey__Group__3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1123:1: rule__WMLKey__Group__3 : rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4 ;
    public final void rule__WMLKey__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1127:1: ( rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1128:2: rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__32388);
            rule__WMLKey__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__4_in_rule__WMLKey__Group__32391);
            rule__WMLKey__Group__4();
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
    // $ANTLR end rule__WMLKey__Group__3


    // $ANTLR start rule__WMLKey__Group__3__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1135:1: rule__WMLKey__Group__3__Impl : ( ( rule__WMLKey__Group_3__0 )* ) ;
    public final void rule__WMLKey__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1139:1: ( ( ( rule__WMLKey__Group_3__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1140:1: ( ( rule__WMLKey__Group_3__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1140:1: ( ( rule__WMLKey__Group_3__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1141:1: ( rule__WMLKey__Group_3__0 )*
            {
             before(grammarAccess.getWMLKeyAccess().getGroup_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1142:1: ( rule__WMLKey__Group_3__0 )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( (LA14_0==RULE_EOL) ) {
                    int LA14_1 = input.LA(2);

                    if ( (LA14_1==22) ) {
                        alt14=1;
                    }


                }
                else if ( (LA14_0==22) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1142:2: rule__WMLKey__Group_3__0
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__Group_3__0_in_rule__WMLKey__Group__3__Impl2418);
            	    rule__WMLKey__Group_3__0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop14;
                }
            } while (true);

             after(grammarAccess.getWMLKeyAccess().getGroup_3()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__Group__3__Impl


    // $ANTLR start rule__WMLKey__Group__4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1152:1: rule__WMLKey__Group__4 : rule__WMLKey__Group__4__Impl ;
    public final void rule__WMLKey__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1156:1: ( rule__WMLKey__Group__4__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1157:2: rule__WMLKey__Group__4__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__4__Impl_in_rule__WMLKey__Group__42449);
            rule__WMLKey__Group__4__Impl();
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
    // $ANTLR end rule__WMLKey__Group__4


    // $ANTLR start rule__WMLKey__Group__4__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1163:1: rule__WMLKey__Group__4__Impl : ( ( rule__WMLKey__Alternatives_4 ) ) ;
    public final void rule__WMLKey__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1167:1: ( ( ( rule__WMLKey__Alternatives_4 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1168:1: ( ( rule__WMLKey__Alternatives_4 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1168:1: ( ( rule__WMLKey__Alternatives_4 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1169:1: ( rule__WMLKey__Alternatives_4 )
            {
             before(grammarAccess.getWMLKeyAccess().getAlternatives_4()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1170:1: ( rule__WMLKey__Alternatives_4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1170:2: rule__WMLKey__Alternatives_4
            {
            pushFollow(FOLLOW_rule__WMLKey__Alternatives_4_in_rule__WMLKey__Group__4__Impl2476);
            rule__WMLKey__Alternatives_4();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getAlternatives_4()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__Group__4__Impl


    // $ANTLR start rule__WMLKey__Group_3__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1190:1: rule__WMLKey__Group_3__0 : rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1 ;
    public final void rule__WMLKey__Group_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1194:1: ( rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1195:2: rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__0__Impl_in_rule__WMLKey__Group_3__02516);
            rule__WMLKey__Group_3__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__1_in_rule__WMLKey__Group_3__02519);
            rule__WMLKey__Group_3__1();
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
    // $ANTLR end rule__WMLKey__Group_3__0


    // $ANTLR start rule__WMLKey__Group_3__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1202:1: rule__WMLKey__Group_3__0__Impl : ( ( RULE_EOL )? ) ;
    public final void rule__WMLKey__Group_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1206:1: ( ( ( RULE_EOL )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1207:1: ( ( RULE_EOL )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1207:1: ( ( RULE_EOL )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1208:1: ( RULE_EOL )?
            {
             before(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1209:1: ( RULE_EOL )?
            int alt15=2;
            int LA15_0 = input.LA(1);

            if ( (LA15_0==RULE_EOL) ) {
                alt15=1;
            }
            switch (alt15) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1209:3: RULE_EOL
                    {
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__0__Impl2547); 

                    }
                    break;

            }

             after(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__Group_3__0__Impl


    // $ANTLR start rule__WMLKey__Group_3__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1219:1: rule__WMLKey__Group_3__1 : rule__WMLKey__Group_3__1__Impl rule__WMLKey__Group_3__2 ;
    public final void rule__WMLKey__Group_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1223:1: ( rule__WMLKey__Group_3__1__Impl rule__WMLKey__Group_3__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1224:2: rule__WMLKey__Group_3__1__Impl rule__WMLKey__Group_3__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__1__Impl_in_rule__WMLKey__Group_3__12578);
            rule__WMLKey__Group_3__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__2_in_rule__WMLKey__Group_3__12581);
            rule__WMLKey__Group_3__2();
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
    // $ANTLR end rule__WMLKey__Group_3__1


    // $ANTLR start rule__WMLKey__Group_3__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1231:1: rule__WMLKey__Group_3__1__Impl : ( '+' ) ;
    public final void rule__WMLKey__Group_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1235:1: ( ( '+' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1236:1: ( '+' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1236:1: ( '+' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1237:1: '+'
            {
             before(grammarAccess.getWMLKeyAccess().getPlusSignKeyword_3_1()); 
            match(input,22,FOLLOW_22_in_rule__WMLKey__Group_3__1__Impl2609); 
             after(grammarAccess.getWMLKeyAccess().getPlusSignKeyword_3_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__Group_3__1__Impl


    // $ANTLR start rule__WMLKey__Group_3__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1250:1: rule__WMLKey__Group_3__2 : rule__WMLKey__Group_3__2__Impl rule__WMLKey__Group_3__3 ;
    public final void rule__WMLKey__Group_3__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1254:1: ( rule__WMLKey__Group_3__2__Impl rule__WMLKey__Group_3__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1255:2: rule__WMLKey__Group_3__2__Impl rule__WMLKey__Group_3__3
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__2__Impl_in_rule__WMLKey__Group_3__22640);
            rule__WMLKey__Group_3__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__3_in_rule__WMLKey__Group_3__22643);
            rule__WMLKey__Group_3__3();
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
    // $ANTLR end rule__WMLKey__Group_3__2


    // $ANTLR start rule__WMLKey__Group_3__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1262:1: rule__WMLKey__Group_3__2__Impl : ( ( RULE_EOL )? ) ;
    public final void rule__WMLKey__Group_3__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1266:1: ( ( ( RULE_EOL )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1267:1: ( ( RULE_EOL )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1267:1: ( ( RULE_EOL )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1268:1: ( RULE_EOL )?
            {
             before(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1269:1: ( RULE_EOL )?
            int alt16=2;
            int LA16_0 = input.LA(1);

            if ( (LA16_0==RULE_EOL) ) {
                alt16=1;
            }
            switch (alt16) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1269:3: RULE_EOL
                    {
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__2__Impl2671); 

                    }
                    break;

            }

             after(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__Group_3__2__Impl


    // $ANTLR start rule__WMLKey__Group_3__3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1279:1: rule__WMLKey__Group_3__3 : rule__WMLKey__Group_3__3__Impl ;
    public final void rule__WMLKey__Group_3__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1283:1: ( rule__WMLKey__Group_3__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1284:2: rule__WMLKey__Group_3__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__3__Impl_in_rule__WMLKey__Group_3__32702);
            rule__WMLKey__Group_3__3__Impl();
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
    // $ANTLR end rule__WMLKey__Group_3__3


    // $ANTLR start rule__WMLKey__Group_3__3__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1290:1: rule__WMLKey__Group_3__3__Impl : ( ( rule__WMLKey__ValueAssignment_3_3 ) ) ;
    public final void rule__WMLKey__Group_3__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1294:1: ( ( ( rule__WMLKey__ValueAssignment_3_3 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1295:1: ( ( rule__WMLKey__ValueAssignment_3_3 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1295:1: ( ( rule__WMLKey__ValueAssignment_3_3 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1296:1: ( rule__WMLKey__ValueAssignment_3_3 )
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_3_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1297:1: ( rule__WMLKey__ValueAssignment_3_3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1297:2: rule__WMLKey__ValueAssignment_3_3
            {
            pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2729);
            rule__WMLKey__ValueAssignment_3_3();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getValueAssignment_3_3()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__Group_3__3__Impl


    // $ANTLR start rule__WMLMacroCall__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1315:1: rule__WMLMacroCall__Group__0 : rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 ;
    public final void rule__WMLMacroCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1319:1: ( rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1320:2: rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__02767);
            rule__WMLMacroCall__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__02770);
            rule__WMLMacroCall__Group__1();
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
    // $ANTLR end rule__WMLMacroCall__Group__0


    // $ANTLR start rule__WMLMacroCall__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1327:1: rule__WMLMacroCall__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacroCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1331:1: ( ( '{' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1332:1: ( '{' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1332:1: ( '{' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1333:1: '{'
            {
             before(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,24,FOLLOW_24_in_rule__WMLMacroCall__Group__0__Impl2798); 
             after(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__Group__0__Impl


    // $ANTLR start rule__WMLMacroCall__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1346:1: rule__WMLMacroCall__Group__1 : rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 ;
    public final void rule__WMLMacroCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1350:1: ( rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1351:2: rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__12829);
            rule__WMLMacroCall__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__12832);
            rule__WMLMacroCall__Group__2();
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
    // $ANTLR end rule__WMLMacroCall__Group__1


    // $ANTLR start rule__WMLMacroCall__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1358:1: rule__WMLMacroCall__Group__1__Impl : ( ( './' )? ) ;
    public final void rule__WMLMacroCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1362:1: ( ( ( './' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1363:1: ( ( './' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1363:1: ( ( './' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1364:1: ( './' )?
            {
             before(grammarAccess.getWMLMacroCallAccess().getFullStopSolidusKeyword_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1365:1: ( './' )?
            int alt17=2;
            int LA17_0 = input.LA(1);

            if ( (LA17_0==25) ) {
                alt17=1;
            }
            switch (alt17) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1366:2: './'
                    {
                    match(input,25,FOLLOW_25_in_rule__WMLMacroCall__Group__1__Impl2861); 

                    }
                    break;

            }

             after(grammarAccess.getWMLMacroCallAccess().getFullStopSolidusKeyword_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__Group__1__Impl


    // $ANTLR start rule__WMLMacroCall__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1377:1: rule__WMLMacroCall__Group__2 : rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 ;
    public final void rule__WMLMacroCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1381:1: ( rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1382:2: rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__22894);
            rule__WMLMacroCall__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__22897);
            rule__WMLMacroCall__Group__3();
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
    // $ANTLR end rule__WMLMacroCall__Group__2


    // $ANTLR start rule__WMLMacroCall__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1389:1: rule__WMLMacroCall__Group__2__Impl : ( ( rule__WMLMacroCall__RelativeAssignment_2 )? ) ;
    public final void rule__WMLMacroCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1393:1: ( ( ( rule__WMLMacroCall__RelativeAssignment_2 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1394:1: ( ( rule__WMLMacroCall__RelativeAssignment_2 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1394:1: ( ( rule__WMLMacroCall__RelativeAssignment_2 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1395:1: ( rule__WMLMacroCall__RelativeAssignment_2 )?
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1396:1: ( rule__WMLMacroCall__RelativeAssignment_2 )?
            int alt18=2;
            int LA18_0 = input.LA(1);

            if ( (LA18_0==15) ) {
                alt18=1;
            }
            switch (alt18) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1396:2: rule__WMLMacroCall__RelativeAssignment_2
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__RelativeAssignment_2_in_rule__WMLMacroCall__Group__2__Impl2924);
                    rule__WMLMacroCall__RelativeAssignment_2();
                    _fsp--;


                    }
                    break;

            }

             after(grammarAccess.getWMLMacroCallAccess().getRelativeAssignment_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__Group__2__Impl


    // $ANTLR start rule__WMLMacroCall__Group__3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1406:1: rule__WMLMacroCall__Group__3 : rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 ;
    public final void rule__WMLMacroCall__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1410:1: ( rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1411:2: rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__32955);
            rule__WMLMacroCall__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__32958);
            rule__WMLMacroCall__Group__4();
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
    // $ANTLR end rule__WMLMacroCall__Group__3


    // $ANTLR start rule__WMLMacroCall__Group__3__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1418:1: rule__WMLMacroCall__Group__3__Impl : ( ( rule__WMLMacroCall__NameAssignment_3 ) ) ;
    public final void rule__WMLMacroCall__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1422:1: ( ( ( rule__WMLMacroCall__NameAssignment_3 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1423:1: ( ( rule__WMLMacroCall__NameAssignment_3 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1423:1: ( ( rule__WMLMacroCall__NameAssignment_3 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1424:1: ( rule__WMLMacroCall__NameAssignment_3 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameAssignment_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1425:1: ( rule__WMLMacroCall__NameAssignment_3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1425:2: rule__WMLMacroCall__NameAssignment_3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__NameAssignment_3_in_rule__WMLMacroCall__Group__3__Impl2985);
            rule__WMLMacroCall__NameAssignment_3();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroCallAccess().getNameAssignment_3()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__Group__3__Impl


    // $ANTLR start rule__WMLMacroCall__Group__4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1435:1: rule__WMLMacroCall__Group__4 : rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5 ;
    public final void rule__WMLMacroCall__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1439:1: ( rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1440:2: rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__43015);
            rule__WMLMacroCall__Group__4__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__5_in_rule__WMLMacroCall__Group__43018);
            rule__WMLMacroCall__Group__5();
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
    // $ANTLR end rule__WMLMacroCall__Group__4


    // $ANTLR start rule__WMLMacroCall__Group__4__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1447:1: rule__WMLMacroCall__Group__4__Impl : ( ( rule__WMLMacroCall__Alternatives_4 )* ) ;
    public final void rule__WMLMacroCall__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1451:1: ( ( ( rule__WMLMacroCall__Alternatives_4 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1452:1: ( ( rule__WMLMacroCall__Alternatives_4 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1452:1: ( ( rule__WMLMacroCall__Alternatives_4 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1453:1: ( rule__WMLMacroCall__Alternatives_4 )*
            {
             before(grammarAccess.getWMLMacroCallAccess().getAlternatives_4()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1454:1: ( rule__WMLMacroCall__Alternatives_4 )*
            loop19:
            do {
                int alt19=2;
                int LA19_0 = input.LA(1);

                if ( ((LA19_0>=RULE_ID && LA19_0<=RULE_ANY_OTHER)||(LA19_0>=14 && LA19_0<=24)||LA19_0==27) ) {
                    alt19=1;
                }


                switch (alt19) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1454:2: rule__WMLMacroCall__Alternatives_4
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroCall__Alternatives_4_in_rule__WMLMacroCall__Group__4__Impl3045);
            	    rule__WMLMacroCall__Alternatives_4();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop19;
                }
            } while (true);

             after(grammarAccess.getWMLMacroCallAccess().getAlternatives_4()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__Group__4__Impl


    // $ANTLR start rule__WMLMacroCall__Group__5
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1464:1: rule__WMLMacroCall__Group__5 : rule__WMLMacroCall__Group__5__Impl ;
    public final void rule__WMLMacroCall__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1468:1: ( rule__WMLMacroCall__Group__5__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1469:2: rule__WMLMacroCall__Group__5__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__5__Impl_in_rule__WMLMacroCall__Group__53076);
            rule__WMLMacroCall__Group__5__Impl();
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
    // $ANTLR end rule__WMLMacroCall__Group__5


    // $ANTLR start rule__WMLMacroCall__Group__5__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1475:1: rule__WMLMacroCall__Group__5__Impl : ( '}' ) ;
    public final void rule__WMLMacroCall__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1479:1: ( ( '}' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1480:1: ( '}' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1480:1: ( '}' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1481:1: '}'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_5()); 
            match(input,26,FOLLOW_26_in_rule__WMLMacroCall__Group__5__Impl3104); 
             after(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_5()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__Group__5__Impl


    // $ANTLR start rule__WMLArrayCall__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1506:1: rule__WMLArrayCall__Group__0 : rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 ;
    public final void rule__WMLArrayCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1510:1: ( rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1511:2: rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__03147);
            rule__WMLArrayCall__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__03150);
            rule__WMLArrayCall__Group__1();
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
    // $ANTLR end rule__WMLArrayCall__Group__0


    // $ANTLR start rule__WMLArrayCall__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1518:1: rule__WMLArrayCall__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLArrayCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1522:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1523:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1523:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1524:1: '['
            {
             before(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0()); 
            match(input,20,FOLLOW_20_in_rule__WMLArrayCall__Group__0__Impl3178); 
             after(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLArrayCall__Group__0__Impl


    // $ANTLR start rule__WMLArrayCall__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1537:1: rule__WMLArrayCall__Group__1 : rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 ;
    public final void rule__WMLArrayCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1541:1: ( rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1542:2: rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__13209);
            rule__WMLArrayCall__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__13212);
            rule__WMLArrayCall__Group__2();
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
    // $ANTLR end rule__WMLArrayCall__Group__1


    // $ANTLR start rule__WMLArrayCall__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1549:1: rule__WMLArrayCall__Group__1__Impl : ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) ;
    public final void rule__WMLArrayCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1553:1: ( ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1554:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1554:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1555:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1555:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1556:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1557:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1557:2: rule__WMLArrayCall__ValueAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3241);
            rule__WMLArrayCall__ValueAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1560:1: ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1561:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1562:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            loop20:
            do {
                int alt20=2;
                int LA20_0 = input.LA(1);

                if ( ((LA20_0>=RULE_ID && LA20_0<=RULE_ANY_OTHER)||(LA20_0>=14 && LA20_0<=18)||LA20_0==27) ) {
                    alt20=1;
                }


                switch (alt20) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1562:2: rule__WMLArrayCall__ValueAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3253);
            	    rule__WMLArrayCall__ValueAssignment_1();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop20;
                }
            } while (true);

             after(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 

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
    // $ANTLR end rule__WMLArrayCall__Group__1__Impl


    // $ANTLR start rule__WMLArrayCall__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1573:1: rule__WMLArrayCall__Group__2 : rule__WMLArrayCall__Group__2__Impl ;
    public final void rule__WMLArrayCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1577:1: ( rule__WMLArrayCall__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1578:2: rule__WMLArrayCall__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__23286);
            rule__WMLArrayCall__Group__2__Impl();
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
    // $ANTLR end rule__WMLArrayCall__Group__2


    // $ANTLR start rule__WMLArrayCall__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1584:1: rule__WMLArrayCall__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLArrayCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1588:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1589:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1589:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1590:1: ']'
            {
             before(grammarAccess.getWMLArrayCallAccess().getRightSquareBracketKeyword_2()); 
            match(input,21,FOLLOW_21_in_rule__WMLArrayCall__Group__2__Impl3314); 
             after(grammarAccess.getWMLArrayCallAccess().getRightSquareBracketKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLArrayCall__Group__2__Impl


    // $ANTLR start rule__WMLMacroDefine__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1609:1: rule__WMLMacroDefine__Group__0 : rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1 ;
    public final void rule__WMLMacroDefine__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1613:1: ( rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1614:2: rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__0__Impl_in_rule__WMLMacroDefine__Group__03351);
            rule__WMLMacroDefine__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__1_in_rule__WMLMacroDefine__Group__03354);
            rule__WMLMacroDefine__Group__1();
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
    // $ANTLR end rule__WMLMacroDefine__Group__0


    // $ANTLR start rule__WMLMacroDefine__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1621:1: rule__WMLMacroDefine__Group__0__Impl : ( ( rule__WMLMacroDefine__NameAssignment_0 ) ) ;
    public final void rule__WMLMacroDefine__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1625:1: ( ( ( rule__WMLMacroDefine__NameAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1626:1: ( ( rule__WMLMacroDefine__NameAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1626:1: ( ( rule__WMLMacroDefine__NameAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1627:1: ( rule__WMLMacroDefine__NameAssignment_0 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1628:1: ( rule__WMLMacroDefine__NameAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1628:2: rule__WMLMacroDefine__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__NameAssignment_0_in_rule__WMLMacroDefine__Group__0__Impl3381);
            rule__WMLMacroDefine__NameAssignment_0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroDefineAccess().getNameAssignment_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__Group__0__Impl


    // $ANTLR start rule__WMLMacroDefine__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1638:1: rule__WMLMacroDefine__Group__1 : rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2 ;
    public final void rule__WMLMacroDefine__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1642:1: ( rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1643:2: rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__1__Impl_in_rule__WMLMacroDefine__Group__13411);
            rule__WMLMacroDefine__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__2_in_rule__WMLMacroDefine__Group__13414);
            rule__WMLMacroDefine__Group__2();
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
    // $ANTLR end rule__WMLMacroDefine__Group__1


    // $ANTLR start rule__WMLMacroDefine__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1650:1: rule__WMLMacroDefine__Group__1__Impl : ( ( rule__WMLMacroDefine__Alternatives_1 )* ) ;
    public final void rule__WMLMacroDefine__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1654:1: ( ( ( rule__WMLMacroDefine__Alternatives_1 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1655:1: ( ( rule__WMLMacroDefine__Alternatives_1 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1655:1: ( ( rule__WMLMacroDefine__Alternatives_1 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1656:1: ( rule__WMLMacroDefine__Alternatives_1 )*
            {
             before(grammarAccess.getWMLMacroDefineAccess().getAlternatives_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1657:1: ( rule__WMLMacroDefine__Alternatives_1 )*
            loop21:
            do {
                int alt21=2;
                int LA21_0 = input.LA(1);

                if ( ((LA21_0>=RULE_ID && LA21_0<=RULE_ANY_OTHER)||(LA21_0>=RULE_DEFINE && LA21_0<=RULE_TEXTDOMAIN)||(LA21_0>=14 && LA21_0<=18)||LA21_0==20||LA21_0==24||LA21_0==27) ) {
                    alt21=1;
                }


                switch (alt21) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1657:2: rule__WMLMacroDefine__Alternatives_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroDefine__Alternatives_1_in_rule__WMLMacroDefine__Group__1__Impl3441);
            	    rule__WMLMacroDefine__Alternatives_1();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop21;
                }
            } while (true);

             after(grammarAccess.getWMLMacroDefineAccess().getAlternatives_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__Group__1__Impl


    // $ANTLR start rule__WMLMacroDefine__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1667:1: rule__WMLMacroDefine__Group__2 : rule__WMLMacroDefine__Group__2__Impl ;
    public final void rule__WMLMacroDefine__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1671:1: ( rule__WMLMacroDefine__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1672:2: rule__WMLMacroDefine__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__2__Impl_in_rule__WMLMacroDefine__Group__23472);
            rule__WMLMacroDefine__Group__2__Impl();
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
    // $ANTLR end rule__WMLMacroDefine__Group__2


    // $ANTLR start rule__WMLMacroDefine__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1678:1: rule__WMLMacroDefine__Group__2__Impl : ( RULE_ENDDEF ) ;
    public final void rule__WMLMacroDefine__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1682:1: ( ( RULE_ENDDEF ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1683:1: ( RULE_ENDDEF )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1683:1: ( RULE_ENDDEF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1684:1: RULE_ENDDEF
            {
             before(grammarAccess.getWMLMacroDefineAccess().getENDDEFTerminalRuleCall_2()); 
            match(input,RULE_ENDDEF,FOLLOW_RULE_ENDDEF_in_rule__WMLMacroDefine__Group__2__Impl3499); 
             after(grammarAccess.getWMLMacroDefineAccess().getENDDEFTerminalRuleCall_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__Group__2__Impl


    // $ANTLR start rule__TSTRING__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1701:1: rule__TSTRING__Group__0 : rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 ;
    public final void rule__TSTRING__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1705:1: ( rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1706:2: rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__03534);
            rule__TSTRING__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__03537);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1713:1: rule__TSTRING__Group__0__Impl : ( '_' ) ;
    public final void rule__TSTRING__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1717:1: ( ( '_' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1718:1: ( '_' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1718:1: ( '_' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1719:1: '_'
            {
             before(grammarAccess.getTSTRINGAccess().get_Keyword_0()); 
            match(input,27,FOLLOW_27_in_rule__TSTRING__Group__0__Impl3565); 
             after(grammarAccess.getTSTRINGAccess().get_Keyword_0()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1732:1: rule__TSTRING__Group__1 : rule__TSTRING__Group__1__Impl ;
    public final void rule__TSTRING__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1736:1: ( rule__TSTRING__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1737:2: rule__TSTRING__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__13596);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1743:1: rule__TSTRING__Group__1__Impl : ( RULE_STRING ) ;
    public final void rule__TSTRING__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1747:1: ( ( RULE_STRING ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1748:1: ( RULE_STRING )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1748:1: ( RULE_STRING )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1749:1: RULE_STRING
            {
             before(grammarAccess.getTSTRINGAccess().getSTRINGTerminalRuleCall_1()); 
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl3623); 
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


    // $ANTLR start rule__WMLRoot__TagsAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1765:1: rule__WMLRoot__TagsAssignment_0 : ( ruleWMLTag ) ;
    public final void rule__WMLRoot__TagsAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1769:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1770:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1770:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1771:1: ruleWMLTag
            {
             before(grammarAccess.getWMLRootAccess().getTagsWMLTagParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRoot__TagsAssignment_03661);
            ruleWMLTag();
            _fsp--;

             after(grammarAccess.getWMLRootAccess().getTagsWMLTagParserRuleCall_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLRoot__TagsAssignment_0


    // $ANTLR start rule__WMLRoot__MacroCallsAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1780:1: rule__WMLRoot__MacroCallsAssignment_1 : ( ruleWMLMacroCall ) ;
    public final void rule__WMLRoot__MacroCallsAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1784:1: ( ( ruleWMLMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1785:1: ( ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1785:1: ( ruleWMLMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1786:1: ruleWMLMacroCall
            {
             before(grammarAccess.getWMLRootAccess().getMacroCallsWMLMacroCallParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLRoot__MacroCallsAssignment_13692);
            ruleWMLMacroCall();
            _fsp--;

             after(grammarAccess.getWMLRootAccess().getMacroCallsWMLMacroCallParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLRoot__MacroCallsAssignment_1


    // $ANTLR start rule__WMLRoot__MacroDefinesAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1795:1: rule__WMLRoot__MacroDefinesAssignment_2 : ( ruleWMLMacroDefine ) ;
    public final void rule__WMLRoot__MacroDefinesAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1799:1: ( ( ruleWMLMacroDefine ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1800:1: ( ruleWMLMacroDefine )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1800:1: ( ruleWMLMacroDefine )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1801:1: ruleWMLMacroDefine
            {
             before(grammarAccess.getWMLRootAccess().getMacroDefinesWMLMacroDefineParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLRoot__MacroDefinesAssignment_23723);
            ruleWMLMacroDefine();
            _fsp--;

             after(grammarAccess.getWMLRootAccess().getMacroDefinesWMLMacroDefineParserRuleCall_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLRoot__MacroDefinesAssignment_2


    // $ANTLR start rule__WMLRoot__TextdomainsAssignment_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1810:1: rule__WMLRoot__TextdomainsAssignment_3 : ( ruleWMLTextdomain ) ;
    public final void rule__WMLRoot__TextdomainsAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1814:1: ( ( ruleWMLTextdomain ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1815:1: ( ruleWMLTextdomain )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1815:1: ( ruleWMLTextdomain )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1816:1: ruleWMLTextdomain
            {
             before(grammarAccess.getWMLRootAccess().getTextdomainsWMLTextdomainParserRuleCall_3_0()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_rule__WMLRoot__TextdomainsAssignment_33754);
            ruleWMLTextdomain();
            _fsp--;

             after(grammarAccess.getWMLRootAccess().getTextdomainsWMLTextdomainParserRuleCall_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLRoot__TextdomainsAssignment_3


    // $ANTLR start rule__WMLTag__PlusAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1825:1: rule__WMLTag__PlusAssignment_1 : ( ( '+' ) ) ;
    public final void rule__WMLTag__PlusAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1829:1: ( ( ( '+' ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1830:1: ( ( '+' ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1830:1: ( ( '+' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1831:1: ( '+' )
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1832:1: ( '+' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1833:1: '+'
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            match(input,22,FOLLOW_22_in_rule__WMLTag__PlusAssignment_13790); 
             after(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 

            }

             after(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__PlusAssignment_1


    // $ANTLR start rule__WMLTag__NameAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1848:1: rule__WMLTag__NameAssignment_2 : ( RULE_ID ) ;
    public final void rule__WMLTag__NameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1852:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1853:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1853:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1854:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_2_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_23829); 
             after(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__NameAssignment_2


    // $ANTLR start rule__WMLTag__TagsAssignment_4_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1863:1: rule__WMLTag__TagsAssignment_4_0 : ( ruleWMLTag ) ;
    public final void rule__WMLTag__TagsAssignment_4_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1867:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1868:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1868:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1869:1: ruleWMLTag
            {
             before(grammarAccess.getWMLTagAccess().getTagsWMLTagParserRuleCall_4_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLTag__TagsAssignment_4_03860);
            ruleWMLTag();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getTagsWMLTagParserRuleCall_4_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__TagsAssignment_4_0


    // $ANTLR start rule__WMLTag__KeysAssignment_4_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1878:1: rule__WMLTag__KeysAssignment_4_1 : ( ruleWMLKey ) ;
    public final void rule__WMLTag__KeysAssignment_4_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1882:1: ( ( ruleWMLKey ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1883:1: ( ruleWMLKey )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1883:1: ( ruleWMLKey )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1884:1: ruleWMLKey
            {
             before(grammarAccess.getWMLTagAccess().getKeysWMLKeyParserRuleCall_4_1_0()); 
            pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLTag__KeysAssignment_4_13891);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getKeysWMLKeyParserRuleCall_4_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__KeysAssignment_4_1


    // $ANTLR start rule__WMLTag__MacroCallsAssignment_4_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1893:1: rule__WMLTag__MacroCallsAssignment_4_2 : ( ruleWMLMacroCall ) ;
    public final void rule__WMLTag__MacroCallsAssignment_4_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1897:1: ( ( ruleWMLMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1898:1: ( ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1898:1: ( ruleWMLMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1899:1: ruleWMLMacroCall
            {
             before(grammarAccess.getWMLTagAccess().getMacroCallsWMLMacroCallParserRuleCall_4_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLTag__MacroCallsAssignment_4_23922);
            ruleWMLMacroCall();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getMacroCallsWMLMacroCallParserRuleCall_4_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__MacroCallsAssignment_4_2


    // $ANTLR start rule__WMLTag__MacroDefinesAssignment_4_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1908:1: rule__WMLTag__MacroDefinesAssignment_4_3 : ( ruleWMLMacroDefine ) ;
    public final void rule__WMLTag__MacroDefinesAssignment_4_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1912:1: ( ( ruleWMLMacroDefine ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1913:1: ( ruleWMLMacroDefine )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1913:1: ( ruleWMLMacroDefine )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1914:1: ruleWMLMacroDefine
            {
             before(grammarAccess.getWMLTagAccess().getMacroDefinesWMLMacroDefineParserRuleCall_4_3_0()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLTag__MacroDefinesAssignment_4_33953);
            ruleWMLMacroDefine();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getMacroDefinesWMLMacroDefineParserRuleCall_4_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__MacroDefinesAssignment_4_3


    // $ANTLR start rule__WMLTag__TextdomainsAssignment_4_4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1923:1: rule__WMLTag__TextdomainsAssignment_4_4 : ( ruleWMLTextdomain ) ;
    public final void rule__WMLTag__TextdomainsAssignment_4_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1927:1: ( ( ruleWMLTextdomain ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1928:1: ( ruleWMLTextdomain )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1928:1: ( ruleWMLTextdomain )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1929:1: ruleWMLTextdomain
            {
             before(grammarAccess.getWMLTagAccess().getTextdomainsWMLTextdomainParserRuleCall_4_4_0()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_rule__WMLTag__TextdomainsAssignment_4_43984);
            ruleWMLTextdomain();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getTextdomainsWMLTextdomainParserRuleCall_4_4_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__TextdomainsAssignment_4_4


    // $ANTLR start rule__WMLTag__EndNameAssignment_6
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1938:1: rule__WMLTag__EndNameAssignment_6 : ( RULE_ID ) ;
    public final void rule__WMLTag__EndNameAssignment_6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1942:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1943:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1943:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1944:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_64015); 
             after(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__EndNameAssignment_6


    // $ANTLR start rule__WMLKey__NameAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1953:1: rule__WMLKey__NameAssignment_0 : ( RULE_ID ) ;
    public final void rule__WMLKey__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1957:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1958:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1958:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1959:1: RULE_ID
            {
             before(grammarAccess.getWMLKeyAccess().getNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_04046); 
             after(grammarAccess.getWMLKeyAccess().getNameIDTerminalRuleCall_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__NameAssignment_0


    // $ANTLR start rule__WMLKey__ValueAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1968:1: rule__WMLKey__ValueAssignment_2 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1972:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1973:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1973:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1974:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_24077);
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


    // $ANTLR start rule__WMLKey__ValueAssignment_3_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1983:1: rule__WMLKey__ValueAssignment_3_3 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValueAssignment_3_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1987:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1988:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1988:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1989:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_3_3_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_3_34108);
            ruleWMLKeyValue();
            _fsp--;

             after(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_3_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__ValueAssignment_3_3


    // $ANTLR start rule__WMLMacroCall__RelativeAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1998:1: rule__WMLMacroCall__RelativeAssignment_2 : ( ( '~' ) ) ;
    public final void rule__WMLMacroCall__RelativeAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2002:1: ( ( ( '~' ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2003:1: ( ( '~' ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2003:1: ( ( '~' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2004:1: ( '~' )
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2005:1: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2006:1: '~'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0()); 
            match(input,15,FOLLOW_15_in_rule__WMLMacroCall__RelativeAssignment_24144); 
             after(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0()); 

            }

             after(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__RelativeAssignment_2


    // $ANTLR start rule__WMLMacroCall__NameAssignment_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2021:1: rule__WMLMacroCall__NameAssignment_3 : ( RULE_ID ) ;
    public final void rule__WMLMacroCall__NameAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2025:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2026:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2026:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2027:1: RULE_ID
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_3_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_34183); 
             after(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__NameAssignment_3


    // $ANTLR start rule__WMLMacroCall__ParamsAssignment_4_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2036:1: rule__WMLMacroCall__ParamsAssignment_4_0 : ( ( rule__WMLMacroCall__ParamsAlternatives_4_0_0 ) ) ;
    public final void rule__WMLMacroCall__ParamsAssignment_4_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2040:1: ( ( ( rule__WMLMacroCall__ParamsAlternatives_4_0_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2041:1: ( ( rule__WMLMacroCall__ParamsAlternatives_4_0_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2041:1: ( ( rule__WMLMacroCall__ParamsAlternatives_4_0_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2042:1: ( rule__WMLMacroCall__ParamsAlternatives_4_0_0 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getParamsAlternatives_4_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2043:1: ( rule__WMLMacroCall__ParamsAlternatives_4_0_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2043:2: rule__WMLMacroCall__ParamsAlternatives_4_0_0
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__ParamsAlternatives_4_0_0_in_rule__WMLMacroCall__ParamsAssignment_4_04214);
            rule__WMLMacroCall__ParamsAlternatives_4_0_0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroCallAccess().getParamsAlternatives_4_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__ParamsAssignment_4_0


    // $ANTLR start rule__WMLMacroCall__ExtraMacrosAssignment_4_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2052:1: rule__WMLMacroCall__ExtraMacrosAssignment_4_1 : ( ruleWMLMacroCall ) ;
    public final void rule__WMLMacroCall__ExtraMacrosAssignment_4_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2056:1: ( ( ruleWMLMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2057:1: ( ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2057:1: ( ruleWMLMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2058:1: ruleWMLMacroCall
            {
             before(grammarAccess.getWMLMacroCallAccess().getExtraMacrosWMLMacroCallParserRuleCall_4_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCall__ExtraMacrosAssignment_4_14247);
            ruleWMLMacroCall();
            _fsp--;

             after(grammarAccess.getWMLMacroCallAccess().getExtraMacrosWMLMacroCallParserRuleCall_4_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__ExtraMacrosAssignment_4_1


    // $ANTLR start rule__WMLLuaCode__ValueAssignment
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2067:1: rule__WMLLuaCode__ValueAssignment : ( RULE_LUA_CODE ) ;
    public final void rule__WMLLuaCode__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2071:1: ( ( RULE_LUA_CODE ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2072:1: ( RULE_LUA_CODE )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2072:1: ( RULE_LUA_CODE )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2073:1: RULE_LUA_CODE
            {
             before(grammarAccess.getWMLLuaCodeAccess().getValueLUA_CODETerminalRuleCall_0()); 
            match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment4278); 
             after(grammarAccess.getWMLLuaCodeAccess().getValueLUA_CODETerminalRuleCall_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLLuaCode__ValueAssignment


    // $ANTLR start rule__WMLArrayCall__ValueAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2082:1: rule__WMLArrayCall__ValueAssignment_1 : ( ruleWMLValue ) ;
    public final void rule__WMLArrayCall__ValueAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2086:1: ( ( ruleWMLValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2087:1: ( ruleWMLValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2087:1: ( ruleWMLValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2088:1: ruleWMLValue
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_14309);
            ruleWMLValue();
            _fsp--;

             after(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLArrayCall__ValueAssignment_1


    // $ANTLR start rule__WMLMacroDefine__NameAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2097:1: rule__WMLMacroDefine__NameAssignment_0 : ( RULE_DEFINE ) ;
    public final void rule__WMLMacroDefine__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2101:1: ( ( RULE_DEFINE ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2102:1: ( RULE_DEFINE )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2102:1: ( RULE_DEFINE )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2103:1: RULE_DEFINE
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0_0()); 
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment_04340); 
             after(grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__NameAssignment_0


    // $ANTLR start rule__WMLMacroDefine__TagsAssignment_1_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2112:1: rule__WMLMacroDefine__TagsAssignment_1_0 : ( ruleWMLTag ) ;
    public final void rule__WMLMacroDefine__TagsAssignment_1_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2116:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2117:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2117:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2118:1: ruleWMLTag
            {
             before(grammarAccess.getWMLMacroDefineAccess().getTagsWMLTagParserRuleCall_1_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLMacroDefine__TagsAssignment_1_04371);
            ruleWMLTag();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineAccess().getTagsWMLTagParserRuleCall_1_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__TagsAssignment_1_0


    // $ANTLR start rule__WMLMacroDefine__KeysAssignment_1_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2127:1: rule__WMLMacroDefine__KeysAssignment_1_1 : ( ruleWMLKey ) ;
    public final void rule__WMLMacroDefine__KeysAssignment_1_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2131:1: ( ( ruleWMLKey ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2132:1: ( ruleWMLKey )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2132:1: ( ruleWMLKey )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2133:1: ruleWMLKey
            {
             before(grammarAccess.getWMLMacroDefineAccess().getKeysWMLKeyParserRuleCall_1_1_0()); 
            pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLMacroDefine__KeysAssignment_1_14402);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineAccess().getKeysWMLKeyParserRuleCall_1_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__KeysAssignment_1_1


    // $ANTLR start rule__WMLMacroDefine__MacroCallsAssignment_1_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2142:1: rule__WMLMacroDefine__MacroCallsAssignment_1_2 : ( ruleWMLMacroCall ) ;
    public final void rule__WMLMacroDefine__MacroCallsAssignment_1_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2146:1: ( ( ruleWMLMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2147:1: ( ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2147:1: ( ruleWMLMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2148:1: ruleWMLMacroCall
            {
             before(grammarAccess.getWMLMacroDefineAccess().getMacroCallsWMLMacroCallParserRuleCall_1_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroDefine__MacroCallsAssignment_1_24433);
            ruleWMLMacroCall();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineAccess().getMacroCallsWMLMacroCallParserRuleCall_1_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__MacroCallsAssignment_1_2


    // $ANTLR start rule__WMLMacroDefine__MacroDefinesAssignment_1_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2157:1: rule__WMLMacroDefine__MacroDefinesAssignment_1_3 : ( ruleWMLMacroDefine ) ;
    public final void rule__WMLMacroDefine__MacroDefinesAssignment_1_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2161:1: ( ( ruleWMLMacroDefine ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2162:1: ( ruleWMLMacroDefine )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2162:1: ( ruleWMLMacroDefine )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2163:1: ruleWMLMacroDefine
            {
             before(grammarAccess.getWMLMacroDefineAccess().getMacroDefinesWMLMacroDefineParserRuleCall_1_3_0()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLMacroDefine__MacroDefinesAssignment_1_34464);
            ruleWMLMacroDefine();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineAccess().getMacroDefinesWMLMacroDefineParserRuleCall_1_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__MacroDefinesAssignment_1_3


    // $ANTLR start rule__WMLMacroDefine__TextdomainsAssignment_1_4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2172:1: rule__WMLMacroDefine__TextdomainsAssignment_1_4 : ( ruleWMLTextdomain ) ;
    public final void rule__WMLMacroDefine__TextdomainsAssignment_1_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2176:1: ( ( ruleWMLTextdomain ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2177:1: ( ruleWMLTextdomain )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2177:1: ( ruleWMLTextdomain )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2178:1: ruleWMLTextdomain
            {
             before(grammarAccess.getWMLMacroDefineAccess().getTextdomainsWMLTextdomainParserRuleCall_1_4_0()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_rule__WMLMacroDefine__TextdomainsAssignment_1_44495);
            ruleWMLTextdomain();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineAccess().getTextdomainsWMLTextdomainParserRuleCall_1_4_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__TextdomainsAssignment_1_4


    // $ANTLR start rule__WMLMacroDefine__ValuesAssignment_1_5
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2187:1: rule__WMLMacroDefine__ValuesAssignment_1_5 : ( ruleWMLValue ) ;
    public final void rule__WMLMacroDefine__ValuesAssignment_1_5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2191:1: ( ( ruleWMLValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2192:1: ( ruleWMLValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2192:1: ( ruleWMLValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2193:1: ruleWMLValue
            {
             before(grammarAccess.getWMLMacroDefineAccess().getValuesWMLValueParserRuleCall_1_5_0()); 
            pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLMacroDefine__ValuesAssignment_1_54526);
            ruleWMLValue();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineAccess().getValuesWMLValueParserRuleCall_1_5_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__ValuesAssignment_1_5


    // $ANTLR start rule__WMLTextdomain__NameAssignment
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2202:1: rule__WMLTextdomain__NameAssignment : ( RULE_TEXTDOMAIN ) ;
    public final void rule__WMLTextdomain__NameAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2206:1: ( ( RULE_TEXTDOMAIN ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2207:1: ( RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2207:1: ( RULE_TEXTDOMAIN )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2208:1: RULE_TEXTDOMAIN
            {
             before(grammarAccess.getWMLTextdomainAccess().getNameTEXTDOMAINTerminalRuleCall_0()); 
            match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment4557); 
             after(grammarAccess.getWMLTextdomainAccess().getNameTEXTDOMAINTerminalRuleCall_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTextdomain__NameAssignment


    // $ANTLR start rule__WMLValue__ValueAssignment
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2217:1: rule__WMLValue__ValueAssignment : ( ( rule__WMLValue__ValueAlternatives_0 ) ) ;
    public final void rule__WMLValue__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2221:1: ( ( ( rule__WMLValue__ValueAlternatives_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2222:1: ( ( rule__WMLValue__ValueAlternatives_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2222:1: ( ( rule__WMLValue__ValueAlternatives_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2223:1: ( rule__WMLValue__ValueAlternatives_0 )
            {
             before(grammarAccess.getWMLValueAccess().getValueAlternatives_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2224:1: ( rule__WMLValue__ValueAlternatives_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2224:2: rule__WMLValue__ValueAlternatives_0
            {
            pushFollow(FOLLOW_rule__WMLValue__ValueAlternatives_0_in_rule__WMLValue__ValueAssignment4588);
            rule__WMLValue__ValueAlternatives_0();
            _fsp--;


            }

             after(grammarAccess.getWMLValueAccess().getValueAlternatives_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLValue__ValueAssignment


    // $ANTLR start rule__MacroTokens__ValAssignment
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2233:1: rule__MacroTokens__ValAssignment : ( ( rule__MacroTokens__ValAlternatives_0 ) ) ;
    public final void rule__MacroTokens__ValAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2237:1: ( ( ( rule__MacroTokens__ValAlternatives_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2238:1: ( ( rule__MacroTokens__ValAlternatives_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2238:1: ( ( rule__MacroTokens__ValAlternatives_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2239:1: ( rule__MacroTokens__ValAlternatives_0 )
            {
             before(grammarAccess.getMacroTokensAccess().getValAlternatives_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2240:1: ( rule__MacroTokens__ValAlternatives_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2240:2: rule__MacroTokens__ValAlternatives_0
            {
            pushFollow(FOLLOW_rule__MacroTokens__ValAlternatives_0_in_rule__MacroTokens__ValAssignment4621);
            rule__MacroTokens__ValAlternatives_0();
            _fsp--;


            }

             after(grammarAccess.getMacroTokensAccess().getValAlternatives_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__MacroTokens__ValAssignment


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot61 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot68 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__Alternatives_in_ruleWMLRoot94 = new BitSet(new long[]{0x0000000001101802L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag122 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag129 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0_in_ruleWMLTag155 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey187 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey194 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey224 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue251 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue258 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue284 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall311 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCall318 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__0_in_ruleWMLMacroCall344 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode371 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLLuaCode378 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLLuaCode__ValueAssignment_in_ruleWMLLuaCode404 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall431 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLArrayCall438 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__0_in_ruleWMLArrayCall464 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine491 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine498 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__0_in_ruleWMLMacroDefine524 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain551 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTextdomain558 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTextdomain__NameAssignment_in_ruleWMLTextdomain584 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_entryRuleWMLValue611 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValue618 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValue__ValueAssignment_in_ruleWMLValue644 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING671 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING678 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0_in_ruleTSTRING704 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTVAR_in_entryRuleTVAR731 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTVAR738 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_ruleTVAR765 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens793 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleMacroTokens800 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__MacroTokens__ValAssignment_in_ruleMacroTokens826 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__TagsAssignment_0_in_rule__WMLRoot__Alternatives862 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__MacroCallsAssignment_1_in_rule__WMLRoot__Alternatives880 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__MacroDefinesAssignment_2_in_rule__WMLRoot__Alternatives898 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__TextdomainsAssignment_3_in_rule__WMLRoot__Alternatives916 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TagsAssignment_4_0_in_rule__WMLTag__Alternatives_4949 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__KeysAssignment_4_1_in_rule__WMLTag__Alternatives_4967 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__MacroCallsAssignment_4_2_in_rule__WMLTag__Alternatives_4985 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__MacroDefinesAssignment_4_3_in_rule__WMLTag__Alternatives_41003 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TextdomainsAssignment_4_4_in_rule__WMLTag__Alternatives_41021 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__Alternatives_41054 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_SL_COMMENT_in_rule__WMLKey__Alternatives_41071 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLKeyValue__Alternatives1103 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives1120 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_rule__WMLKeyValue__Alternatives1137 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_rule__WMLKeyValue__Alternatives1154 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ParamsAssignment_4_0_in_rule__WMLMacroCall__Alternatives_41186 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ExtraMacrosAssignment_4_1_in_rule__WMLMacroCall__Alternatives_41204 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLMacroCall__ParamsAlternatives_4_0_01237 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_rule__WMLMacroCall__ParamsAlternatives_4_0_01254 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__TagsAssignment_1_0_in_rule__WMLMacroDefine__Alternatives_11286 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__KeysAssignment_1_1_in_rule__WMLMacroDefine__Alternatives_11304 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__MacroCallsAssignment_1_2_in_rule__WMLMacroDefine__Alternatives_11322 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__MacroDefinesAssignment_1_3_in_rule__WMLMacroDefine__Alternatives_11340 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__TextdomainsAssignment_1_4_in_rule__WMLMacroDefine__Alternatives_11358 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__ValuesAssignment_1_5_in_rule__WMLMacroDefine__Alternatives_11376 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLValue__ValueAlternatives_01409 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_rule__WMLValue__ValueAlternatives_01426 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTVAR_in_rule__WMLValue__ValueAlternatives_01443 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLValue__ValueAlternatives_01460 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLValue__ValueAlternatives_01478 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__WMLValue__ValueAlternatives_01498 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_rule__WMLValue__ValueAlternatives_01518 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_rule__WMLValue__ValueAlternatives_01538 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__ValueAlternatives_01557 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_rule__MacroTokens__ValAlternatives_01590 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__MacroTokens__ValAlternatives_01610 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__MacroTokens__ValAlternatives_01630 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__MacroTokens__ValAlternatives_01650 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__MacroTokens__ValAlternatives_01670 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01702 = new BitSet(new long[]{0x0000000000400040L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01705 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__WMLTag__Group__0__Impl1733 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__11764 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__11767 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl1794 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__21825 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__21828 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl1855 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__31885 = new BitSet(new long[]{0x0000000001901840L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__31888 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLTag__Group__3__Impl1916 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__41947 = new BitSet(new long[]{0x0000000000800000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__41950 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Alternatives_4_in_rule__WMLTag__Group__4__Impl1977 = new BitSet(new long[]{0x0000000001101842L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__52008 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__52011 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__WMLTag__Group__5__Impl2039 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__62070 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__62073 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl2100 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__72130 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLTag__Group__7__Impl2158 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02205 = new BitSet(new long[]{0x0000000000080000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02208 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl2235 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12265 = new BitSet(new long[]{0x000000000957C5F0L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12268 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_rule__WMLKey__Group__1__Impl2296 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22327 = new BitSet(new long[]{0x0000000000400030L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__22330 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2357 = new BitSet(new long[]{0x000000000917C5C2L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__32388 = new BitSet(new long[]{0x0000000000000030L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__4_in_rule__WMLKey__Group__32391 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__0_in_rule__WMLKey__Group__3__Impl2418 = new BitSet(new long[]{0x0000000000400012L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__4__Impl_in_rule__WMLKey__Group__42449 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Alternatives_4_in_rule__WMLKey__Group__4__Impl2476 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__0__Impl_in_rule__WMLKey__Group_3__02516 = new BitSet(new long[]{0x0000000000400000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__1_in_rule__WMLKey__Group_3__02519 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__0__Impl2547 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__1__Impl_in_rule__WMLKey__Group_3__12578 = new BitSet(new long[]{0x000000000917C5D0L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__2_in_rule__WMLKey__Group_3__12581 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__WMLKey__Group_3__1__Impl2609 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__2__Impl_in_rule__WMLKey__Group_3__22640 = new BitSet(new long[]{0x000000000917C5C0L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__3_in_rule__WMLKey__Group_3__22643 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__2__Impl2671 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__3__Impl_in_rule__WMLKey__Group_3__32702 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2729 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__02767 = new BitSet(new long[]{0x0000000002008040L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__02770 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_rule__WMLMacroCall__Group__0__Impl2798 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__12829 = new BitSet(new long[]{0x0000000000008040L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__12832 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__WMLMacroCall__Group__1__Impl2861 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__22894 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__22897 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__RelativeAssignment_2_in_rule__WMLMacroCall__Group__2__Impl2924 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__32955 = new BitSet(new long[]{0x000000000DFFC1C0L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__32958 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__NameAssignment_3_in_rule__WMLMacroCall__Group__3__Impl2985 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__43015 = new BitSet(new long[]{0x0000000004000000L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__5_in_rule__WMLMacroCall__Group__43018 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Alternatives_4_in_rule__WMLMacroCall__Group__4__Impl3045 = new BitSet(new long[]{0x0000000009FFC1C2L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__5__Impl_in_rule__WMLMacroCall__Group__53076 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_rule__WMLMacroCall__Group__5__Impl3104 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__03147 = new BitSet(new long[]{0x000000000807C1C0L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__03150 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__WMLArrayCall__Group__0__Impl3178 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__13209 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__13212 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3241 = new BitSet(new long[]{0x000000000807C1C2L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3253 = new BitSet(new long[]{0x000000000807C1C2L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__23286 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLArrayCall__Group__2__Impl3314 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__0__Impl_in_rule__WMLMacroDefine__Group__03351 = new BitSet(new long[]{0x000000000917DBC0L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__1_in_rule__WMLMacroDefine__Group__03354 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__NameAssignment_0_in_rule__WMLMacroDefine__Group__0__Impl3381 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__1__Impl_in_rule__WMLMacroDefine__Group__13411 = new BitSet(new long[]{0x0000000000000200L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__2_in_rule__WMLMacroDefine__Group__13414 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Alternatives_1_in_rule__WMLMacroDefine__Group__1__Impl3441 = new BitSet(new long[]{0x000000000917D9C2L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__2__Impl_in_rule__WMLMacroDefine__Group__23472 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ENDDEF_in_rule__WMLMacroDefine__Group__2__Impl3499 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__03534 = new BitSet(new long[]{0x0000000000000080L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__03537 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rule__TSTRING__Group__0__Impl3565 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__13596 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl3623 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRoot__TagsAssignment_03661 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLRoot__MacroCallsAssignment_13692 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLRoot__MacroDefinesAssignment_23723 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_rule__WMLRoot__TextdomainsAssignment_33754 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__WMLTag__PlusAssignment_13790 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_23829 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLTag__TagsAssignment_4_03860 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLTag__KeysAssignment_4_13891 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLTag__MacroCallsAssignment_4_23922 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLTag__MacroDefinesAssignment_4_33953 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_rule__WMLTag__TextdomainsAssignment_4_43984 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_64015 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_04046 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_24077 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_3_34108 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLMacroCall__RelativeAssignment_24144 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_34183 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ParamsAlternatives_4_0_0_in_rule__WMLMacroCall__ParamsAssignment_4_04214 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCall__ExtraMacrosAssignment_4_14247 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment4278 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_14309 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment_04340 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLMacroDefine__TagsAssignment_1_04371 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLMacroDefine__KeysAssignment_1_14402 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroDefine__MacroCallsAssignment_1_24433 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLMacroDefine__MacroDefinesAssignment_1_34464 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_rule__WMLMacroDefine__TextdomainsAssignment_1_44495 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLMacroDefine__ValuesAssignment_1_54526 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment4557 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValue__ValueAlternatives_0_in_rule__WMLValue__ValueAssignment4588 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__MacroTokens__ValAlternatives_0_in_rule__MacroTokens__ValAssignment4621 = new BitSet(new long[]{0x0000000000000002L});

}