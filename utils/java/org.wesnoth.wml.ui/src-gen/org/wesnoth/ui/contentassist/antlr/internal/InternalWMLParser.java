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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_EOL", "RULE_SL_COMMENT", "RULE_ID", "RULE_STRING", "RULE_ANY_OTHER", "RULE_LUA_CODE", "RULE_DEFINE", "RULE_TEXTDOMAIN", "RULE_WS", "'+'", "'~'", "'['", "']'", "'[/'", "'='", "'{'", "'}'", "'('", "')'"
    };
    public static final int RULE_LUA_CODE=9;
    public static final int RULE_ID=6;
    public static final int RULE_STRING=7;
    public static final int RULE_DEFINE=10;
    public static final int RULE_ANY_OTHER=8;
    public static final int RULE_EOL=4;
    public static final int RULE_TEXTDOMAIN=11;
    public static final int RULE_WS=12;
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

                if ( ((LA1_0>=RULE_DEFINE && LA1_0<=RULE_TEXTDOMAIN)||LA1_0==15||LA1_0==19) ) {
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


    // $ANTLR start entryRuleWMLMacroParameter
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:209:1: entryRuleWMLMacroParameter : ruleWMLMacroParameter EOF ;
    public final void entryRuleWMLMacroParameter() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:210:1: ( ruleWMLMacroParameter EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:211:1: ruleWMLMacroParameter EOF
            {
             before(grammarAccess.getWMLMacroParameterRule()); 
            pushFollow(FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter371);
            ruleWMLMacroParameter();
            _fsp--;

             after(grammarAccess.getWMLMacroParameterRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroParameter378); 

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
    // $ANTLR end entryRuleWMLMacroParameter


    // $ANTLR start ruleWMLMacroParameter
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:218:1: ruleWMLMacroParameter : ( ( rule__WMLMacroParameter__Group__0 ) ) ;
    public final void ruleWMLMacroParameter() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:222:2: ( ( ( rule__WMLMacroParameter__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:223:1: ( ( rule__WMLMacroParameter__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:223:1: ( ( rule__WMLMacroParameter__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:224:1: ( rule__WMLMacroParameter__Group__0 )
            {
             before(grammarAccess.getWMLMacroParameterAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:225:1: ( rule__WMLMacroParameter__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:225:2: rule__WMLMacroParameter__Group__0
            {
            pushFollow(FOLLOW_rule__WMLMacroParameter__Group__0_in_ruleWMLMacroParameter404);
            rule__WMLMacroParameter__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroParameterAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLMacroParameter


    // $ANTLR start entryRuleWMLLuaCode
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:237:1: entryRuleWMLLuaCode : ruleWMLLuaCode EOF ;
    public final void entryRuleWMLLuaCode() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:238:1: ( ruleWMLLuaCode EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:239:1: ruleWMLLuaCode EOF
            {
             before(grammarAccess.getWMLLuaCodeRule()); 
            pushFollow(FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode431);
            ruleWMLLuaCode();
            _fsp--;

             after(grammarAccess.getWMLLuaCodeRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLLuaCode438); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:246:1: ruleWMLLuaCode : ( ( rule__WMLLuaCode__ValueAssignment ) ) ;
    public final void ruleWMLLuaCode() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:250:2: ( ( ( rule__WMLLuaCode__ValueAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:251:1: ( ( rule__WMLLuaCode__ValueAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:251:1: ( ( rule__WMLLuaCode__ValueAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:252:1: ( rule__WMLLuaCode__ValueAssignment )
            {
             before(grammarAccess.getWMLLuaCodeAccess().getValueAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:253:1: ( rule__WMLLuaCode__ValueAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:253:2: rule__WMLLuaCode__ValueAssignment
            {
            pushFollow(FOLLOW_rule__WMLLuaCode__ValueAssignment_in_ruleWMLLuaCode464);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:265:1: entryRuleWMLArrayCall : ruleWMLArrayCall EOF ;
    public final void entryRuleWMLArrayCall() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:266:1: ( ruleWMLArrayCall EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:267:1: ruleWMLArrayCall EOF
            {
             before(grammarAccess.getWMLArrayCallRule()); 
            pushFollow(FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall491);
            ruleWMLArrayCall();
            _fsp--;

             after(grammarAccess.getWMLArrayCallRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLArrayCall498); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:274:1: ruleWMLArrayCall : ( ( rule__WMLArrayCall__Group__0 ) ) ;
    public final void ruleWMLArrayCall() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:278:2: ( ( ( rule__WMLArrayCall__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:279:1: ( ( rule__WMLArrayCall__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:279:1: ( ( rule__WMLArrayCall__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:280:1: ( rule__WMLArrayCall__Group__0 )
            {
             before(grammarAccess.getWMLArrayCallAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:1: ( rule__WMLArrayCall__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:2: rule__WMLArrayCall__Group__0
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__0_in_ruleWMLArrayCall524);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:293:1: entryRuleWMLMacroDefine : ruleWMLMacroDefine EOF ;
    public final void entryRuleWMLMacroDefine() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:294:1: ( ruleWMLMacroDefine EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:295:1: ruleWMLMacroDefine EOF
            {
             before(grammarAccess.getWMLMacroDefineRule()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine551);
            ruleWMLMacroDefine();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroDefine558); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:302:1: ruleWMLMacroDefine : ( ( rule__WMLMacroDefine__NameAssignment ) ) ;
    public final void ruleWMLMacroDefine() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:306:2: ( ( ( rule__WMLMacroDefine__NameAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:307:1: ( ( rule__WMLMacroDefine__NameAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:307:1: ( ( rule__WMLMacroDefine__NameAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:308:1: ( rule__WMLMacroDefine__NameAssignment )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:1: ( rule__WMLMacroDefine__NameAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:2: rule__WMLMacroDefine__NameAssignment
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__NameAssignment_in_ruleWMLMacroDefine584);
            rule__WMLMacroDefine__NameAssignment();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroDefineAccess().getNameAssignment()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:321:1: entryRuleWMLTextdomain : ruleWMLTextdomain EOF ;
    public final void entryRuleWMLTextdomain() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:322:1: ( ruleWMLTextdomain EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:323:1: ruleWMLTextdomain EOF
            {
             before(grammarAccess.getWMLTextdomainRule()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain611);
            ruleWMLTextdomain();
            _fsp--;

             after(grammarAccess.getWMLTextdomainRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTextdomain618); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:330:1: ruleWMLTextdomain : ( ( rule__WMLTextdomain__NameAssignment ) ) ;
    public final void ruleWMLTextdomain() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:334:2: ( ( ( rule__WMLTextdomain__NameAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:335:1: ( ( rule__WMLTextdomain__NameAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:335:1: ( ( rule__WMLTextdomain__NameAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:336:1: ( rule__WMLTextdomain__NameAssignment )
            {
             before(grammarAccess.getWMLTextdomainAccess().getNameAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:1: ( rule__WMLTextdomain__NameAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:2: rule__WMLTextdomain__NameAssignment
            {
            pushFollow(FOLLOW_rule__WMLTextdomain__NameAssignment_in_ruleWMLTextdomain644);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:349:1: entryRuleWMLValue : ruleWMLValue EOF ;
    public final void entryRuleWMLValue() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:350:1: ( ruleWMLValue EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:351:1: ruleWMLValue EOF
            {
             before(grammarAccess.getWMLValueRule()); 
            pushFollow(FOLLOW_ruleWMLValue_in_entryRuleWMLValue671);
            ruleWMLValue();
            _fsp--;

             after(grammarAccess.getWMLValueRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValue678); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:358:1: ruleWMLValue : ( ( rule__WMLValue__ValueAssignment ) ) ;
    public final void ruleWMLValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:362:2: ( ( ( rule__WMLValue__ValueAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:363:1: ( ( rule__WMLValue__ValueAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:363:1: ( ( rule__WMLValue__ValueAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:364:1: ( rule__WMLValue__ValueAssignment )
            {
             before(grammarAccess.getWMLValueAccess().getValueAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:365:1: ( rule__WMLValue__ValueAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:365:2: rule__WMLValue__ValueAssignment
            {
            pushFollow(FOLLOW_rule__WMLValue__ValueAssignment_in_ruleWMLValue704);
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


    // $ANTLR start rule__WMLRoot__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:377:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__TagsAssignment_0 ) ) | ( ( rule__WMLRoot__MacroCallsAssignment_1 ) ) | ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) ) | ( ( rule__WMLRoot__TextdomainsAssignment_3 ) ) );
    public final void rule__WMLRoot__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:381:1: ( ( ( rule__WMLRoot__TagsAssignment_0 ) ) | ( ( rule__WMLRoot__MacroCallsAssignment_1 ) ) | ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) ) | ( ( rule__WMLRoot__TextdomainsAssignment_3 ) ) )
            int alt2=4;
            switch ( input.LA(1) ) {
            case 15:
                {
                alt2=1;
                }
                break;
            case 19:
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
                    new NoViableAltException("377:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__TagsAssignment_0 ) ) | ( ( rule__WMLRoot__MacroCallsAssignment_1 ) ) | ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) ) | ( ( rule__WMLRoot__TextdomainsAssignment_3 ) ) );", 2, 0, input);

                throw nvae;
            }

            switch (alt2) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:382:1: ( ( rule__WMLRoot__TagsAssignment_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:382:1: ( ( rule__WMLRoot__TagsAssignment_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:383:1: ( rule__WMLRoot__TagsAssignment_0 )
                    {
                     before(grammarAccess.getWMLRootAccess().getTagsAssignment_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:384:1: ( rule__WMLRoot__TagsAssignment_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:384:2: rule__WMLRoot__TagsAssignment_0
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__TagsAssignment_0_in_rule__WMLRoot__Alternatives740);
                    rule__WMLRoot__TagsAssignment_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getTagsAssignment_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:388:6: ( ( rule__WMLRoot__MacroCallsAssignment_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:388:6: ( ( rule__WMLRoot__MacroCallsAssignment_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:389:1: ( rule__WMLRoot__MacroCallsAssignment_1 )
                    {
                     before(grammarAccess.getWMLRootAccess().getMacroCallsAssignment_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:390:1: ( rule__WMLRoot__MacroCallsAssignment_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:390:2: rule__WMLRoot__MacroCallsAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__MacroCallsAssignment_1_in_rule__WMLRoot__Alternatives758);
                    rule__WMLRoot__MacroCallsAssignment_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getMacroCallsAssignment_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:394:6: ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:394:6: ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:395:1: ( rule__WMLRoot__MacroDefinesAssignment_2 )
                    {
                     before(grammarAccess.getWMLRootAccess().getMacroDefinesAssignment_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:396:1: ( rule__WMLRoot__MacroDefinesAssignment_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:396:2: rule__WMLRoot__MacroDefinesAssignment_2
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__MacroDefinesAssignment_2_in_rule__WMLRoot__Alternatives776);
                    rule__WMLRoot__MacroDefinesAssignment_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getMacroDefinesAssignment_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:400:6: ( ( rule__WMLRoot__TextdomainsAssignment_3 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:400:6: ( ( rule__WMLRoot__TextdomainsAssignment_3 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:401:1: ( rule__WMLRoot__TextdomainsAssignment_3 )
                    {
                     before(grammarAccess.getWMLRootAccess().getTextdomainsAssignment_3()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:402:1: ( rule__WMLRoot__TextdomainsAssignment_3 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:402:2: rule__WMLRoot__TextdomainsAssignment_3
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__TextdomainsAssignment_3_in_rule__WMLRoot__Alternatives794);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:411:1: rule__WMLTag__Alternatives_4 : ( ( ( rule__WMLTag__TagsAssignment_4_0 ) ) | ( ( rule__WMLTag__KeysAssignment_4_1 ) ) | ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) ) | ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) ) | ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) ) );
    public final void rule__WMLTag__Alternatives_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:415:1: ( ( ( rule__WMLTag__TagsAssignment_4_0 ) ) | ( ( rule__WMLTag__KeysAssignment_4_1 ) ) | ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) ) | ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) ) | ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) ) )
            int alt3=5;
            switch ( input.LA(1) ) {
            case 15:
                {
                alt3=1;
                }
                break;
            case RULE_ID:
                {
                alt3=2;
                }
                break;
            case 19:
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
                    new NoViableAltException("411:1: rule__WMLTag__Alternatives_4 : ( ( ( rule__WMLTag__TagsAssignment_4_0 ) ) | ( ( rule__WMLTag__KeysAssignment_4_1 ) ) | ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) ) | ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) ) | ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) ) );", 3, 0, input);

                throw nvae;
            }

            switch (alt3) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:416:1: ( ( rule__WMLTag__TagsAssignment_4_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:416:1: ( ( rule__WMLTag__TagsAssignment_4_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:417:1: ( rule__WMLTag__TagsAssignment_4_0 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTagsAssignment_4_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:418:1: ( rule__WMLTag__TagsAssignment_4_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:418:2: rule__WMLTag__TagsAssignment_4_0
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TagsAssignment_4_0_in_rule__WMLTag__Alternatives_4827);
                    rule__WMLTag__TagsAssignment_4_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTagsAssignment_4_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:422:6: ( ( rule__WMLTag__KeysAssignment_4_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:422:6: ( ( rule__WMLTag__KeysAssignment_4_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:423:1: ( rule__WMLTag__KeysAssignment_4_1 )
                    {
                     before(grammarAccess.getWMLTagAccess().getKeysAssignment_4_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:424:1: ( rule__WMLTag__KeysAssignment_4_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:424:2: rule__WMLTag__KeysAssignment_4_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__KeysAssignment_4_1_in_rule__WMLTag__Alternatives_4845);
                    rule__WMLTag__KeysAssignment_4_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getKeysAssignment_4_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:428:6: ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:428:6: ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:429:1: ( rule__WMLTag__MacroCallsAssignment_4_2 )
                    {
                     before(grammarAccess.getWMLTagAccess().getMacroCallsAssignment_4_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:430:1: ( rule__WMLTag__MacroCallsAssignment_4_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:430:2: rule__WMLTag__MacroCallsAssignment_4_2
                    {
                    pushFollow(FOLLOW_rule__WMLTag__MacroCallsAssignment_4_2_in_rule__WMLTag__Alternatives_4863);
                    rule__WMLTag__MacroCallsAssignment_4_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getMacroCallsAssignment_4_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:434:6: ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:434:6: ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:435:1: ( rule__WMLTag__MacroDefinesAssignment_4_3 )
                    {
                     before(grammarAccess.getWMLTagAccess().getMacroDefinesAssignment_4_3()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:436:1: ( rule__WMLTag__MacroDefinesAssignment_4_3 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:436:2: rule__WMLTag__MacroDefinesAssignment_4_3
                    {
                    pushFollow(FOLLOW_rule__WMLTag__MacroDefinesAssignment_4_3_in_rule__WMLTag__Alternatives_4881);
                    rule__WMLTag__MacroDefinesAssignment_4_3();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getMacroDefinesAssignment_4_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:440:6: ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:440:6: ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:441:1: ( rule__WMLTag__TextdomainsAssignment_4_4 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTextdomainsAssignment_4_4()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:442:1: ( rule__WMLTag__TextdomainsAssignment_4_4 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:442:2: rule__WMLTag__TextdomainsAssignment_4_4
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TextdomainsAssignment_4_4_in_rule__WMLTag__Alternatives_4899);
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


    // $ANTLR start rule__WMLKey__EolAlternatives_3_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:451:1: rule__WMLKey__EolAlternatives_3_0 : ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) );
    public final void rule__WMLKey__EolAlternatives_3_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:455:1: ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) )
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
                    new NoViableAltException("451:1: rule__WMLKey__EolAlternatives_3_0 : ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) );", 4, 0, input);

                throw nvae;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:456:1: ( RULE_EOL )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:456:1: ( RULE_EOL )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:457:1: RULE_EOL
                    {
                     before(grammarAccess.getWMLKeyAccess().getEolEOLTerminalRuleCall_3_0_0()); 
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__EolAlternatives_3_0932); 
                     after(grammarAccess.getWMLKeyAccess().getEolEOLTerminalRuleCall_3_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:462:6: ( RULE_SL_COMMENT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:462:6: ( RULE_SL_COMMENT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:463:1: RULE_SL_COMMENT
                    {
                     before(grammarAccess.getWMLKeyAccess().getEolSL_COMMENTTerminalRuleCall_3_0_1()); 
                    match(input,RULE_SL_COMMENT,FOLLOW_RULE_SL_COMMENT_in_rule__WMLKey__EolAlternatives_3_0949); 
                     after(grammarAccess.getWMLKeyAccess().getEolSL_COMMENTTerminalRuleCall_3_0_1()); 

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
    // $ANTLR end rule__WMLKey__EolAlternatives_3_0


    // $ANTLR start rule__WMLKeyValue__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:473:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) );
    public final void rule__WMLKeyValue__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:477:1: ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) )
            int alt5=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
            case RULE_STRING:
            case RULE_ANY_OTHER:
            case 13:
            case 14:
                {
                alt5=1;
                }
                break;
            case 19:
                {
                alt5=2;
                }
                break;
            case RULE_LUA_CODE:
                {
                alt5=3;
                }
                break;
            case 15:
                {
                alt5=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("473:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) );", 5, 0, input);

                throw nvae;
            }

            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:478:1: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:478:1: ( ruleWMLValue )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:479:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLKeyValue__Alternatives981);
                    ruleWMLValue();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:484:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:484:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:485:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives998);
                    ruleWMLMacroCall();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:490:6: ( ruleWMLLuaCode )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:490:6: ( ruleWMLLuaCode )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:491:1: ruleWMLLuaCode
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2()); 
                    pushFollow(FOLLOW_ruleWMLLuaCode_in_rule__WMLKeyValue__Alternatives1015);
                    ruleWMLLuaCode();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:496:6: ( ruleWMLArrayCall )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:496:6: ( ruleWMLArrayCall )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:497:1: ruleWMLArrayCall
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLArrayCallParserRuleCall_3()); 
                    pushFollow(FOLLOW_ruleWMLArrayCall_in_rule__WMLKeyValue__Alternatives1032);
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


    // $ANTLR start rule__WMLMacroCall__Alternatives_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:507:1: rule__WMLMacroCall__Alternatives_3 : ( ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) ) | ( ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 ) ) );
    public final void rule__WMLMacroCall__Alternatives_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:511:1: ( ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) ) | ( ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 ) ) )
            int alt6=2;
            int LA6_0 = input.LA(1);

            if ( ((LA6_0>=RULE_ID && LA6_0<=RULE_ANY_OTHER)||(LA6_0>=13 && LA6_0<=14)||LA6_0==21) ) {
                alt6=1;
            }
            else if ( (LA6_0==19) ) {
                alt6=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("507:1: rule__WMLMacroCall__Alternatives_3 : ( ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) ) | ( ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 ) ) );", 6, 0, input);

                throw nvae;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:512:1: ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:512:1: ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:513:1: ( rule__WMLMacroCall__ParamsAssignment_3_0 )
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getParamsAssignment_3_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:514:1: ( rule__WMLMacroCall__ParamsAssignment_3_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:514:2: rule__WMLMacroCall__ParamsAssignment_3_0
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__ParamsAssignment_3_0_in_rule__WMLMacroCall__Alternatives_31064);
                    rule__WMLMacroCall__ParamsAssignment_3_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroCallAccess().getParamsAssignment_3_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:518:6: ( ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:518:6: ( ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:519:1: ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 )
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getExtraMacrosAssignment_3_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:520:1: ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:520:2: rule__WMLMacroCall__ExtraMacrosAssignment_3_1
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__ExtraMacrosAssignment_3_1_in_rule__WMLMacroCall__Alternatives_31082);
                    rule__WMLMacroCall__ExtraMacrosAssignment_3_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroCallAccess().getExtraMacrosAssignment_3_1()); 

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
    // $ANTLR end rule__WMLMacroCall__Alternatives_3


    // $ANTLR start rule__WMLMacroCall__ParamsAlternatives_3_0_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:529:1: rule__WMLMacroCall__ParamsAlternatives_3_0_0 : ( ( ruleWMLValue ) | ( ruleWMLMacroParameter ) );
    public final void rule__WMLMacroCall__ParamsAlternatives_3_0_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:1: ( ( ruleWMLValue ) | ( ruleWMLMacroParameter ) )
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( ((LA7_0>=RULE_ID && LA7_0<=RULE_ANY_OTHER)||(LA7_0>=13 && LA7_0<=14)) ) {
                alt7=1;
            }
            else if ( (LA7_0==21) ) {
                alt7=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("529:1: rule__WMLMacroCall__ParamsAlternatives_3_0_0 : ( ( ruleWMLValue ) | ( ruleWMLMacroParameter ) );", 7, 0, input);

                throw nvae;
            }
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:534:1: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:534:1: ( ruleWMLValue )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:535:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getParamsWMLValueParserRuleCall_3_0_0_0()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLMacroCall__ParamsAlternatives_3_0_01115);
                    ruleWMLValue();
                    _fsp--;

                     after(grammarAccess.getWMLMacroCallAccess().getParamsWMLValueParserRuleCall_3_0_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:540:6: ( ruleWMLMacroParameter )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:540:6: ( ruleWMLMacroParameter )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:541:1: ruleWMLMacroParameter
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getParamsWMLMacroParameterParserRuleCall_3_0_0_1()); 
                    pushFollow(FOLLOW_ruleWMLMacroParameter_in_rule__WMLMacroCall__ParamsAlternatives_3_0_01132);
                    ruleWMLMacroParameter();
                    _fsp--;

                     after(grammarAccess.getWMLMacroCallAccess().getParamsWMLMacroParameterParserRuleCall_3_0_0_1()); 

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
    // $ANTLR end rule__WMLMacroCall__ParamsAlternatives_3_0_0


    // $ANTLR start rule__WMLMacroParameter__ParamAlternatives_1_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:551:1: rule__WMLMacroParameter__ParamAlternatives_1_0 : ( ( ruleWMLValue ) | ( ruleWMLTag ) | ( ruleWMLMacroCall ) | ( ruleWMLKey ) );
    public final void rule__WMLMacroParameter__ParamAlternatives_1_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:555:1: ( ( ruleWMLValue ) | ( ruleWMLTag ) | ( ruleWMLMacroCall ) | ( ruleWMLKey ) )
            int alt8=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                int LA8_1 = input.LA(2);

                if ( ((LA8_1>=RULE_ID && LA8_1<=RULE_ANY_OTHER)||(LA8_1>=13 && LA8_1<=15)||LA8_1==19||LA8_1==22) ) {
                    alt8=1;
                }
                else if ( (LA8_1==18) ) {
                    alt8=4;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("551:1: rule__WMLMacroParameter__ParamAlternatives_1_0 : ( ( ruleWMLValue ) | ( ruleWMLTag ) | ( ruleWMLMacroCall ) | ( ruleWMLKey ) );", 8, 1, input);

                    throw nvae;
                }
                }
                break;
            case RULE_STRING:
            case RULE_ANY_OTHER:
            case 13:
            case 14:
                {
                alt8=1;
                }
                break;
            case 15:
                {
                alt8=2;
                }
                break;
            case 19:
                {
                alt8=3;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("551:1: rule__WMLMacroParameter__ParamAlternatives_1_0 : ( ( ruleWMLValue ) | ( ruleWMLTag ) | ( ruleWMLMacroCall ) | ( ruleWMLKey ) );", 8, 0, input);

                throw nvae;
            }

            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:556:1: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:556:1: ( ruleWMLValue )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:557:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLMacroParameterAccess().getParamWMLValueParserRuleCall_1_0_0()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLMacroParameter__ParamAlternatives_1_01164);
                    ruleWMLValue();
                    _fsp--;

                     after(grammarAccess.getWMLMacroParameterAccess().getParamWMLValueParserRuleCall_1_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:562:6: ( ruleWMLTag )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:562:6: ( ruleWMLTag )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:563:1: ruleWMLTag
                    {
                     before(grammarAccess.getWMLMacroParameterAccess().getParamWMLTagParserRuleCall_1_0_1()); 
                    pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLMacroParameter__ParamAlternatives_1_01181);
                    ruleWMLTag();
                    _fsp--;

                     after(grammarAccess.getWMLMacroParameterAccess().getParamWMLTagParserRuleCall_1_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:568:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:568:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:569:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLMacroParameterAccess().getParamWMLMacroCallParserRuleCall_1_0_2()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroParameter__ParamAlternatives_1_01198);
                    ruleWMLMacroCall();
                    _fsp--;

                     after(grammarAccess.getWMLMacroParameterAccess().getParamWMLMacroCallParserRuleCall_1_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:574:6: ( ruleWMLKey )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:574:6: ( ruleWMLKey )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:575:1: ruleWMLKey
                    {
                     before(grammarAccess.getWMLMacroParameterAccess().getParamWMLKeyParserRuleCall_1_0_3()); 
                    pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLMacroParameter__ParamAlternatives_1_01215);
                    ruleWMLKey();
                    _fsp--;

                     after(grammarAccess.getWMLMacroParameterAccess().getParamWMLKeyParserRuleCall_1_0_3()); 

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
    // $ANTLR end rule__WMLMacroParameter__ParamAlternatives_1_0


    // $ANTLR start rule__WMLValue__ValueAlternatives_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:585:1: rule__WMLValue__ValueAlternatives_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( '+' ) | ( '~' ) | ( RULE_ANY_OTHER ) );
    public final void rule__WMLValue__ValueAlternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:589:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( '+' ) | ( '~' ) | ( RULE_ANY_OTHER ) )
            int alt9=5;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt9=1;
                }
                break;
            case RULE_STRING:
                {
                alt9=2;
                }
                break;
            case 13:
                {
                alt9=3;
                }
                break;
            case 14:
                {
                alt9=4;
                }
                break;
            case RULE_ANY_OTHER:
                {
                alt9=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("585:1: rule__WMLValue__ValueAlternatives_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( '+' ) | ( '~' ) | ( RULE_ANY_OTHER ) );", 9, 0, input);

                throw nvae;
            }

            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:590:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:590:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:591:1: RULE_ID
                    {
                     before(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLValue__ValueAlternatives_01247); 
                     after(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:596:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:596:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:597:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLValue__ValueAlternatives_01264); 
                     after(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:602:6: ( '+' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:602:6: ( '+' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:603:1: '+'
                    {
                     before(grammarAccess.getWMLValueAccess().getValuePlusSignKeyword_0_2()); 
                    match(input,13,FOLLOW_13_in_rule__WMLValue__ValueAlternatives_01282); 
                     after(grammarAccess.getWMLValueAccess().getValuePlusSignKeyword_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:610:6: ( '~' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:610:6: ( '~' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:611:1: '~'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_3()); 
                    match(input,14,FOLLOW_14_in_rule__WMLValue__ValueAlternatives_01302); 
                     after(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:618:6: ( RULE_ANY_OTHER )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:618:6: ( RULE_ANY_OTHER )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:619:1: RULE_ANY_OTHER
                    {
                     before(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_4()); 
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__ValueAlternatives_01321); 
                     after(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_4()); 

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


    // $ANTLR start rule__WMLTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:631:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:635:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:636:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01351);
            rule__WMLTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01354);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:643:1: rule__WMLTag__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:647:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:648:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:648:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:649:1: '['
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0()); 
            match(input,15,FOLLOW_15_in_rule__WMLTag__Group__0__Impl1382); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:662:1: rule__WMLTag__Group__1 : rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 ;
    public final void rule__WMLTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:666:1: ( rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:667:2: rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__11413);
            rule__WMLTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__11416);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:674:1: rule__WMLTag__Group__1__Impl : ( ( rule__WMLTag__PlusAssignment_1 )? ) ;
    public final void rule__WMLTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:678:1: ( ( ( rule__WMLTag__PlusAssignment_1 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:679:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:679:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:680:1: ( rule__WMLTag__PlusAssignment_1 )?
            {
             before(grammarAccess.getWMLTagAccess().getPlusAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:681:1: ( rule__WMLTag__PlusAssignment_1 )?
            int alt10=2;
            int LA10_0 = input.LA(1);

            if ( (LA10_0==13) ) {
                alt10=1;
            }
            switch (alt10) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:681:2: rule__WMLTag__PlusAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl1443);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:691:1: rule__WMLTag__Group__2 : rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 ;
    public final void rule__WMLTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:695:1: ( rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:696:2: rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__21474);
            rule__WMLTag__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__21477);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:703:1: rule__WMLTag__Group__2__Impl : ( ( rule__WMLTag__NameAssignment_2 ) ) ;
    public final void rule__WMLTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:707:1: ( ( ( rule__WMLTag__NameAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:708:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:708:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:709:1: ( rule__WMLTag__NameAssignment_2 )
            {
             before(grammarAccess.getWMLTagAccess().getNameAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:710:1: ( rule__WMLTag__NameAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:710:2: rule__WMLTag__NameAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl1504);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:720:1: rule__WMLTag__Group__3 : rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 ;
    public final void rule__WMLTag__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:724:1: ( rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:725:2: rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__31534);
            rule__WMLTag__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__31537);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:732:1: rule__WMLTag__Group__3__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:736:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:737:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:737:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:738:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3()); 
            match(input,16,FOLLOW_16_in_rule__WMLTag__Group__3__Impl1565); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:751:1: rule__WMLTag__Group__4 : rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 ;
    public final void rule__WMLTag__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:755:1: ( rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:756:2: rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__41596);
            rule__WMLTag__Group__4__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__41599);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:763:1: rule__WMLTag__Group__4__Impl : ( ( rule__WMLTag__Alternatives_4 )* ) ;
    public final void rule__WMLTag__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:767:1: ( ( ( rule__WMLTag__Alternatives_4 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:768:1: ( ( rule__WMLTag__Alternatives_4 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:768:1: ( ( rule__WMLTag__Alternatives_4 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:769:1: ( rule__WMLTag__Alternatives_4 )*
            {
             before(grammarAccess.getWMLTagAccess().getAlternatives_4()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:770:1: ( rule__WMLTag__Alternatives_4 )*
            loop11:
            do {
                int alt11=2;
                int LA11_0 = input.LA(1);

                if ( (LA11_0==RULE_ID||(LA11_0>=RULE_DEFINE && LA11_0<=RULE_TEXTDOMAIN)||LA11_0==15||LA11_0==19) ) {
                    alt11=1;
                }


                switch (alt11) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:770:2: rule__WMLTag__Alternatives_4
            	    {
            	    pushFollow(FOLLOW_rule__WMLTag__Alternatives_4_in_rule__WMLTag__Group__4__Impl1626);
            	    rule__WMLTag__Alternatives_4();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop11;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:780:1: rule__WMLTag__Group__5 : rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 ;
    public final void rule__WMLTag__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:784:1: ( rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:785:2: rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__51657);
            rule__WMLTag__Group__5__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__51660);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:792:1: rule__WMLTag__Group__5__Impl : ( '[/' ) ;
    public final void rule__WMLTag__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:796:1: ( ( '[/' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:797:1: ( '[/' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:797:1: ( '[/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:798:1: '[/'
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5()); 
            match(input,17,FOLLOW_17_in_rule__WMLTag__Group__5__Impl1688); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:811:1: rule__WMLTag__Group__6 : rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 ;
    public final void rule__WMLTag__Group__6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:815:1: ( rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:816:2: rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__61719);
            rule__WMLTag__Group__6__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__61722);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:823:1: rule__WMLTag__Group__6__Impl : ( ( rule__WMLTag__EndNameAssignment_6 ) ) ;
    public final void rule__WMLTag__Group__6__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:827:1: ( ( ( rule__WMLTag__EndNameAssignment_6 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:828:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:828:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:829:1: ( rule__WMLTag__EndNameAssignment_6 )
            {
             before(grammarAccess.getWMLTagAccess().getEndNameAssignment_6()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:830:1: ( rule__WMLTag__EndNameAssignment_6 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:830:2: rule__WMLTag__EndNameAssignment_6
            {
            pushFollow(FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl1749);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:840:1: rule__WMLTag__Group__7 : rule__WMLTag__Group__7__Impl ;
    public final void rule__WMLTag__Group__7() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:844:1: ( rule__WMLTag__Group__7__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:845:2: rule__WMLTag__Group__7__Impl
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__71779);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:851:1: rule__WMLTag__Group__7__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__7__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:855:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:856:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:856:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:857:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_7()); 
            match(input,16,FOLLOW_16_in_rule__WMLTag__Group__7__Impl1807); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:886:1: rule__WMLKey__Group__0 : rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 ;
    public final void rule__WMLKey__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:890:1: ( rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:891:2: rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__01854);
            rule__WMLKey__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__01857);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:898:1: rule__WMLKey__Group__0__Impl : ( ( rule__WMLKey__NameAssignment_0 ) ) ;
    public final void rule__WMLKey__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:902:1: ( ( ( rule__WMLKey__NameAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:903:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:903:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:904:1: ( rule__WMLKey__NameAssignment_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getNameAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:905:1: ( rule__WMLKey__NameAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:905:2: rule__WMLKey__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl1884);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:915:1: rule__WMLKey__Group__1 : rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 ;
    public final void rule__WMLKey__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:919:1: ( rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:920:2: rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__11914);
            rule__WMLKey__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__11917);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:927:1: rule__WMLKey__Group__1__Impl : ( '=' ) ;
    public final void rule__WMLKey__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:931:1: ( ( '=' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:932:1: ( '=' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:932:1: ( '=' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:933:1: '='
            {
             before(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1()); 
            match(input,18,FOLLOW_18_in_rule__WMLKey__Group__1__Impl1945); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:946:1: rule__WMLKey__Group__2 : rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 ;
    public final void rule__WMLKey__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:950:1: ( rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:951:2: rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__21976);
            rule__WMLKey__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__21979);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:958:1: rule__WMLKey__Group__2__Impl : ( ( ( rule__WMLKey__ValueAssignment_2 ) ) ( ( rule__WMLKey__ValueAssignment_2 )* ) ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:962:1: ( ( ( ( rule__WMLKey__ValueAssignment_2 ) ) ( ( rule__WMLKey__ValueAssignment_2 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:963:1: ( ( ( rule__WMLKey__ValueAssignment_2 ) ) ( ( rule__WMLKey__ValueAssignment_2 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:963:1: ( ( ( rule__WMLKey__ValueAssignment_2 ) ) ( ( rule__WMLKey__ValueAssignment_2 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:964:1: ( ( rule__WMLKey__ValueAssignment_2 ) ) ( ( rule__WMLKey__ValueAssignment_2 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:964:1: ( ( rule__WMLKey__ValueAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:965:1: ( rule__WMLKey__ValueAssignment_2 )
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:966:1: ( rule__WMLKey__ValueAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:966:2: rule__WMLKey__ValueAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2008);
            rule__WMLKey__ValueAssignment_2();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:969:1: ( ( rule__WMLKey__ValueAssignment_2 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:970:1: ( rule__WMLKey__ValueAssignment_2 )*
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:971:1: ( rule__WMLKey__ValueAssignment_2 )*
            loop12:
            do {
                int alt12=2;
                int LA12_0 = input.LA(1);

                if ( ((LA12_0>=RULE_ID && LA12_0<=RULE_LUA_CODE)||(LA12_0>=13 && LA12_0<=15)||LA12_0==19) ) {
                    alt12=1;
                }


                switch (alt12) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:971:2: rule__WMLKey__ValueAssignment_2
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2020);
            	    rule__WMLKey__ValueAssignment_2();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop12;
                }
            } while (true);

             after(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 

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
    // $ANTLR end rule__WMLKey__Group__2__Impl


    // $ANTLR start rule__WMLKey__Group__3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:982:1: rule__WMLKey__Group__3 : rule__WMLKey__Group__3__Impl ;
    public final void rule__WMLKey__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:986:1: ( rule__WMLKey__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:987:2: rule__WMLKey__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__32053);
            rule__WMLKey__Group__3__Impl();
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:993:1: rule__WMLKey__Group__3__Impl : ( ( rule__WMLKey__EolAssignment_3 ) ) ;
    public final void rule__WMLKey__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:997:1: ( ( ( rule__WMLKey__EolAssignment_3 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:998:1: ( ( rule__WMLKey__EolAssignment_3 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:998:1: ( ( rule__WMLKey__EolAssignment_3 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:999:1: ( rule__WMLKey__EolAssignment_3 )
            {
             before(grammarAccess.getWMLKeyAccess().getEolAssignment_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1000:1: ( rule__WMLKey__EolAssignment_3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1000:2: rule__WMLKey__EolAssignment_3
            {
            pushFollow(FOLLOW_rule__WMLKey__EolAssignment_3_in_rule__WMLKey__Group__3__Impl2080);
            rule__WMLKey__EolAssignment_3();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getEolAssignment_3()); 

            }


            }

        }
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


    // $ANTLR start rule__WMLMacroCall__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1018:1: rule__WMLMacroCall__Group__0 : rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 ;
    public final void rule__WMLMacroCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1022:1: ( rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1023:2: rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__02118);
            rule__WMLMacroCall__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__02121);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1030:1: rule__WMLMacroCall__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacroCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1034:1: ( ( '{' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1035:1: ( '{' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1035:1: ( '{' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1036:1: '{'
            {
             before(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,19,FOLLOW_19_in_rule__WMLMacroCall__Group__0__Impl2149); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1049:1: rule__WMLMacroCall__Group__1 : rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 ;
    public final void rule__WMLMacroCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1053:1: ( rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1054:2: rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__12180);
            rule__WMLMacroCall__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__12183);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1061:1: rule__WMLMacroCall__Group__1__Impl : ( ( rule__WMLMacroCall__RelativeAssignment_1 )? ) ;
    public final void rule__WMLMacroCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1065:1: ( ( ( rule__WMLMacroCall__RelativeAssignment_1 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1066:1: ( ( rule__WMLMacroCall__RelativeAssignment_1 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1066:1: ( ( rule__WMLMacroCall__RelativeAssignment_1 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1067:1: ( rule__WMLMacroCall__RelativeAssignment_1 )?
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1068:1: ( rule__WMLMacroCall__RelativeAssignment_1 )?
            int alt13=2;
            int LA13_0 = input.LA(1);

            if ( (LA13_0==14) ) {
                alt13=1;
            }
            switch (alt13) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1068:2: rule__WMLMacroCall__RelativeAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__RelativeAssignment_1_in_rule__WMLMacroCall__Group__1__Impl2210);
                    rule__WMLMacroCall__RelativeAssignment_1();
                    _fsp--;


                    }
                    break;

            }

             after(grammarAccess.getWMLMacroCallAccess().getRelativeAssignment_1()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1078:1: rule__WMLMacroCall__Group__2 : rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 ;
    public final void rule__WMLMacroCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1082:1: ( rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1083:2: rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__22241);
            rule__WMLMacroCall__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__22244);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1090:1: rule__WMLMacroCall__Group__2__Impl : ( ( rule__WMLMacroCall__NameAssignment_2 ) ) ;
    public final void rule__WMLMacroCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1094:1: ( ( ( rule__WMLMacroCall__NameAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1095:1: ( ( rule__WMLMacroCall__NameAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1095:1: ( ( rule__WMLMacroCall__NameAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1096:1: ( rule__WMLMacroCall__NameAssignment_2 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1097:1: ( rule__WMLMacroCall__NameAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1097:2: rule__WMLMacroCall__NameAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__NameAssignment_2_in_rule__WMLMacroCall__Group__2__Impl2271);
            rule__WMLMacroCall__NameAssignment_2();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroCallAccess().getNameAssignment_2()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1107:1: rule__WMLMacroCall__Group__3 : rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 ;
    public final void rule__WMLMacroCall__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1111:1: ( rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1112:2: rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__32301);
            rule__WMLMacroCall__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__32304);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1119:1: rule__WMLMacroCall__Group__3__Impl : ( ( rule__WMLMacroCall__Alternatives_3 )* ) ;
    public final void rule__WMLMacroCall__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1123:1: ( ( ( rule__WMLMacroCall__Alternatives_3 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1124:1: ( ( rule__WMLMacroCall__Alternatives_3 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1124:1: ( ( rule__WMLMacroCall__Alternatives_3 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1125:1: ( rule__WMLMacroCall__Alternatives_3 )*
            {
             before(grammarAccess.getWMLMacroCallAccess().getAlternatives_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1126:1: ( rule__WMLMacroCall__Alternatives_3 )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( ((LA14_0>=RULE_ID && LA14_0<=RULE_ANY_OTHER)||(LA14_0>=13 && LA14_0<=14)||LA14_0==19||LA14_0==21) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1126:2: rule__WMLMacroCall__Alternatives_3
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroCall__Alternatives_3_in_rule__WMLMacroCall__Group__3__Impl2331);
            	    rule__WMLMacroCall__Alternatives_3();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop14;
                }
            } while (true);

             after(grammarAccess.getWMLMacroCallAccess().getAlternatives_3()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1136:1: rule__WMLMacroCall__Group__4 : rule__WMLMacroCall__Group__4__Impl ;
    public final void rule__WMLMacroCall__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1140:1: ( rule__WMLMacroCall__Group__4__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1141:2: rule__WMLMacroCall__Group__4__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__42362);
            rule__WMLMacroCall__Group__4__Impl();
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1147:1: rule__WMLMacroCall__Group__4__Impl : ( '}' ) ;
    public final void rule__WMLMacroCall__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1151:1: ( ( '}' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1152:1: ( '}' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1152:1: ( '}' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1153:1: '}'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_4()); 
            match(input,20,FOLLOW_20_in_rule__WMLMacroCall__Group__4__Impl2390); 
             after(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_4()); 

            }


            }

        }
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


    // $ANTLR start rule__WMLMacroParameter__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1176:1: rule__WMLMacroParameter__Group__0 : rule__WMLMacroParameter__Group__0__Impl rule__WMLMacroParameter__Group__1 ;
    public final void rule__WMLMacroParameter__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1180:1: ( rule__WMLMacroParameter__Group__0__Impl rule__WMLMacroParameter__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1181:2: rule__WMLMacroParameter__Group__0__Impl rule__WMLMacroParameter__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroParameter__Group__0__Impl_in_rule__WMLMacroParameter__Group__02431);
            rule__WMLMacroParameter__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroParameter__Group__1_in_rule__WMLMacroParameter__Group__02434);
            rule__WMLMacroParameter__Group__1();
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
    // $ANTLR end rule__WMLMacroParameter__Group__0


    // $ANTLR start rule__WMLMacroParameter__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1188:1: rule__WMLMacroParameter__Group__0__Impl : ( '(' ) ;
    public final void rule__WMLMacroParameter__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1192:1: ( ( '(' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1193:1: ( '(' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1193:1: ( '(' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1194:1: '('
            {
             before(grammarAccess.getWMLMacroParameterAccess().getLeftParenthesisKeyword_0()); 
            match(input,21,FOLLOW_21_in_rule__WMLMacroParameter__Group__0__Impl2462); 
             after(grammarAccess.getWMLMacroParameterAccess().getLeftParenthesisKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroParameter__Group__0__Impl


    // $ANTLR start rule__WMLMacroParameter__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1207:1: rule__WMLMacroParameter__Group__1 : rule__WMLMacroParameter__Group__1__Impl rule__WMLMacroParameter__Group__2 ;
    public final void rule__WMLMacroParameter__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1211:1: ( rule__WMLMacroParameter__Group__1__Impl rule__WMLMacroParameter__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1212:2: rule__WMLMacroParameter__Group__1__Impl rule__WMLMacroParameter__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroParameter__Group__1__Impl_in_rule__WMLMacroParameter__Group__12493);
            rule__WMLMacroParameter__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroParameter__Group__2_in_rule__WMLMacroParameter__Group__12496);
            rule__WMLMacroParameter__Group__2();
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
    // $ANTLR end rule__WMLMacroParameter__Group__1


    // $ANTLR start rule__WMLMacroParameter__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1219:1: rule__WMLMacroParameter__Group__1__Impl : ( ( ( rule__WMLMacroParameter__ParamAssignment_1 ) ) ( ( rule__WMLMacroParameter__ParamAssignment_1 )* ) ) ;
    public final void rule__WMLMacroParameter__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1223:1: ( ( ( ( rule__WMLMacroParameter__ParamAssignment_1 ) ) ( ( rule__WMLMacroParameter__ParamAssignment_1 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1224:1: ( ( ( rule__WMLMacroParameter__ParamAssignment_1 ) ) ( ( rule__WMLMacroParameter__ParamAssignment_1 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1224:1: ( ( ( rule__WMLMacroParameter__ParamAssignment_1 ) ) ( ( rule__WMLMacroParameter__ParamAssignment_1 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1225:1: ( ( rule__WMLMacroParameter__ParamAssignment_1 ) ) ( ( rule__WMLMacroParameter__ParamAssignment_1 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1225:1: ( ( rule__WMLMacroParameter__ParamAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1226:1: ( rule__WMLMacroParameter__ParamAssignment_1 )
            {
             before(grammarAccess.getWMLMacroParameterAccess().getParamAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1227:1: ( rule__WMLMacroParameter__ParamAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1227:2: rule__WMLMacroParameter__ParamAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLMacroParameter__ParamAssignment_1_in_rule__WMLMacroParameter__Group__1__Impl2525);
            rule__WMLMacroParameter__ParamAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroParameterAccess().getParamAssignment_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1230:1: ( ( rule__WMLMacroParameter__ParamAssignment_1 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1231:1: ( rule__WMLMacroParameter__ParamAssignment_1 )*
            {
             before(grammarAccess.getWMLMacroParameterAccess().getParamAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1232:1: ( rule__WMLMacroParameter__ParamAssignment_1 )*
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( ((LA15_0>=RULE_ID && LA15_0<=RULE_ANY_OTHER)||(LA15_0>=13 && LA15_0<=15)||LA15_0==19) ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1232:2: rule__WMLMacroParameter__ParamAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroParameter__ParamAssignment_1_in_rule__WMLMacroParameter__Group__1__Impl2537);
            	    rule__WMLMacroParameter__ParamAssignment_1();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop15;
                }
            } while (true);

             after(grammarAccess.getWMLMacroParameterAccess().getParamAssignment_1()); 

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
    // $ANTLR end rule__WMLMacroParameter__Group__1__Impl


    // $ANTLR start rule__WMLMacroParameter__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1243:1: rule__WMLMacroParameter__Group__2 : rule__WMLMacroParameter__Group__2__Impl ;
    public final void rule__WMLMacroParameter__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1247:1: ( rule__WMLMacroParameter__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1248:2: rule__WMLMacroParameter__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroParameter__Group__2__Impl_in_rule__WMLMacroParameter__Group__22570);
            rule__WMLMacroParameter__Group__2__Impl();
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
    // $ANTLR end rule__WMLMacroParameter__Group__2


    // $ANTLR start rule__WMLMacroParameter__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1254:1: rule__WMLMacroParameter__Group__2__Impl : ( ')' ) ;
    public final void rule__WMLMacroParameter__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1258:1: ( ( ')' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1259:1: ( ')' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1259:1: ( ')' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1260:1: ')'
            {
             before(grammarAccess.getWMLMacroParameterAccess().getRightParenthesisKeyword_2()); 
            match(input,22,FOLLOW_22_in_rule__WMLMacroParameter__Group__2__Impl2598); 
             after(grammarAccess.getWMLMacroParameterAccess().getRightParenthesisKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroParameter__Group__2__Impl


    // $ANTLR start rule__WMLArrayCall__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1279:1: rule__WMLArrayCall__Group__0 : rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 ;
    public final void rule__WMLArrayCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1283:1: ( rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1284:2: rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__02635);
            rule__WMLArrayCall__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__02638);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1291:1: rule__WMLArrayCall__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLArrayCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1295:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1296:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1296:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1297:1: '['
            {
             before(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0()); 
            match(input,15,FOLLOW_15_in_rule__WMLArrayCall__Group__0__Impl2666); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1310:1: rule__WMLArrayCall__Group__1 : rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 ;
    public final void rule__WMLArrayCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1314:1: ( rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1315:2: rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__12697);
            rule__WMLArrayCall__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__12700);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1322:1: rule__WMLArrayCall__Group__1__Impl : ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) ;
    public final void rule__WMLArrayCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1326:1: ( ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1327:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1327:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1328:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1328:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1329:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1330:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1330:2: rule__WMLArrayCall__ValueAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl2729);
            rule__WMLArrayCall__ValueAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1333:1: ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1334:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1335:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            loop16:
            do {
                int alt16=2;
                int LA16_0 = input.LA(1);

                if ( ((LA16_0>=RULE_ID && LA16_0<=RULE_ANY_OTHER)||(LA16_0>=13 && LA16_0<=14)) ) {
                    alt16=1;
                }


                switch (alt16) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1335:2: rule__WMLArrayCall__ValueAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl2741);
            	    rule__WMLArrayCall__ValueAssignment_1();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop16;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1346:1: rule__WMLArrayCall__Group__2 : rule__WMLArrayCall__Group__2__Impl ;
    public final void rule__WMLArrayCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1350:1: ( rule__WMLArrayCall__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1351:2: rule__WMLArrayCall__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__22774);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1357:1: rule__WMLArrayCall__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLArrayCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1361:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1362:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1362:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1363:1: ']'
            {
             before(grammarAccess.getWMLArrayCallAccess().getRightSquareBracketKeyword_2()); 
            match(input,16,FOLLOW_16_in_rule__WMLArrayCall__Group__2__Impl2802); 
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


    // $ANTLR start rule__WMLRoot__TagsAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1383:1: rule__WMLRoot__TagsAssignment_0 : ( ruleWMLTag ) ;
    public final void rule__WMLRoot__TagsAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1387:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1388:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1388:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1389:1: ruleWMLTag
            {
             before(grammarAccess.getWMLRootAccess().getTagsWMLTagParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRoot__TagsAssignment_02844);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1398:1: rule__WMLRoot__MacroCallsAssignment_1 : ( ruleWMLMacroCall ) ;
    public final void rule__WMLRoot__MacroCallsAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1402:1: ( ( ruleWMLMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1403:1: ( ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1403:1: ( ruleWMLMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1404:1: ruleWMLMacroCall
            {
             before(grammarAccess.getWMLRootAccess().getMacroCallsWMLMacroCallParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLRoot__MacroCallsAssignment_12875);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1413:1: rule__WMLRoot__MacroDefinesAssignment_2 : ( ruleWMLMacroDefine ) ;
    public final void rule__WMLRoot__MacroDefinesAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1417:1: ( ( ruleWMLMacroDefine ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1418:1: ( ruleWMLMacroDefine )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1418:1: ( ruleWMLMacroDefine )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1419:1: ruleWMLMacroDefine
            {
             before(grammarAccess.getWMLRootAccess().getMacroDefinesWMLMacroDefineParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLRoot__MacroDefinesAssignment_22906);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1428:1: rule__WMLRoot__TextdomainsAssignment_3 : ( ruleWMLTextdomain ) ;
    public final void rule__WMLRoot__TextdomainsAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1432:1: ( ( ruleWMLTextdomain ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1433:1: ( ruleWMLTextdomain )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1433:1: ( ruleWMLTextdomain )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1434:1: ruleWMLTextdomain
            {
             before(grammarAccess.getWMLRootAccess().getTextdomainsWMLTextdomainParserRuleCall_3_0()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_rule__WMLRoot__TextdomainsAssignment_32937);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1443:1: rule__WMLTag__PlusAssignment_1 : ( ( '+' ) ) ;
    public final void rule__WMLTag__PlusAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1447:1: ( ( ( '+' ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1448:1: ( ( '+' ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1448:1: ( ( '+' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1449:1: ( '+' )
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1450:1: ( '+' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1451:1: '+'
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            match(input,13,FOLLOW_13_in_rule__WMLTag__PlusAssignment_12973); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1466:1: rule__WMLTag__NameAssignment_2 : ( RULE_ID ) ;
    public final void rule__WMLTag__NameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1470:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1471:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1471:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1472:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_2_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_23012); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1481:1: rule__WMLTag__TagsAssignment_4_0 : ( ruleWMLTag ) ;
    public final void rule__WMLTag__TagsAssignment_4_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1485:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1486:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1486:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1487:1: ruleWMLTag
            {
             before(grammarAccess.getWMLTagAccess().getTagsWMLTagParserRuleCall_4_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLTag__TagsAssignment_4_03043);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1496:1: rule__WMLTag__KeysAssignment_4_1 : ( ruleWMLKey ) ;
    public final void rule__WMLTag__KeysAssignment_4_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1500:1: ( ( ruleWMLKey ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1501:1: ( ruleWMLKey )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1501:1: ( ruleWMLKey )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1502:1: ruleWMLKey
            {
             before(grammarAccess.getWMLTagAccess().getKeysWMLKeyParserRuleCall_4_1_0()); 
            pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLTag__KeysAssignment_4_13074);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1511:1: rule__WMLTag__MacroCallsAssignment_4_2 : ( ruleWMLMacroCall ) ;
    public final void rule__WMLTag__MacroCallsAssignment_4_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1515:1: ( ( ruleWMLMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1516:1: ( ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1516:1: ( ruleWMLMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1517:1: ruleWMLMacroCall
            {
             before(grammarAccess.getWMLTagAccess().getMacroCallsWMLMacroCallParserRuleCall_4_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLTag__MacroCallsAssignment_4_23105);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1526:1: rule__WMLTag__MacroDefinesAssignment_4_3 : ( ruleWMLMacroDefine ) ;
    public final void rule__WMLTag__MacroDefinesAssignment_4_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1530:1: ( ( ruleWMLMacroDefine ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1531:1: ( ruleWMLMacroDefine )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1531:1: ( ruleWMLMacroDefine )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1532:1: ruleWMLMacroDefine
            {
             before(grammarAccess.getWMLTagAccess().getMacroDefinesWMLMacroDefineParserRuleCall_4_3_0()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLTag__MacroDefinesAssignment_4_33136);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1541:1: rule__WMLTag__TextdomainsAssignment_4_4 : ( ruleWMLTextdomain ) ;
    public final void rule__WMLTag__TextdomainsAssignment_4_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1545:1: ( ( ruleWMLTextdomain ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1546:1: ( ruleWMLTextdomain )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1546:1: ( ruleWMLTextdomain )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1547:1: ruleWMLTextdomain
            {
             before(grammarAccess.getWMLTagAccess().getTextdomainsWMLTextdomainParserRuleCall_4_4_0()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_rule__WMLTag__TextdomainsAssignment_4_43167);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1556:1: rule__WMLTag__EndNameAssignment_6 : ( RULE_ID ) ;
    public final void rule__WMLTag__EndNameAssignment_6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1560:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1561:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1561:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1562:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_63198); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1571:1: rule__WMLKey__NameAssignment_0 : ( RULE_ID ) ;
    public final void rule__WMLKey__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1575:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1576:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1576:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1577:1: RULE_ID
            {
             before(grammarAccess.getWMLKeyAccess().getNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_03229); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1586:1: rule__WMLKey__ValueAssignment_2 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1590:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1591:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1591:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1592:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_23260);
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


    // $ANTLR start rule__WMLKey__EolAssignment_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1601:1: rule__WMLKey__EolAssignment_3 : ( ( rule__WMLKey__EolAlternatives_3_0 ) ) ;
    public final void rule__WMLKey__EolAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1605:1: ( ( ( rule__WMLKey__EolAlternatives_3_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1606:1: ( ( rule__WMLKey__EolAlternatives_3_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1606:1: ( ( rule__WMLKey__EolAlternatives_3_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1607:1: ( rule__WMLKey__EolAlternatives_3_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getEolAlternatives_3_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1608:1: ( rule__WMLKey__EolAlternatives_3_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1608:2: rule__WMLKey__EolAlternatives_3_0
            {
            pushFollow(FOLLOW_rule__WMLKey__EolAlternatives_3_0_in_rule__WMLKey__EolAssignment_33291);
            rule__WMLKey__EolAlternatives_3_0();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getEolAlternatives_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__EolAssignment_3


    // $ANTLR start rule__WMLMacroCall__RelativeAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1617:1: rule__WMLMacroCall__RelativeAssignment_1 : ( ( '~' ) ) ;
    public final void rule__WMLMacroCall__RelativeAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1621:1: ( ( ( '~' ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1622:1: ( ( '~' ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1622:1: ( ( '~' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1623:1: ( '~' )
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_1_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1624:1: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1625:1: '~'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_1_0()); 
            match(input,14,FOLLOW_14_in_rule__WMLMacroCall__RelativeAssignment_13329); 
             after(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_1_0()); 

            }

             after(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__RelativeAssignment_1


    // $ANTLR start rule__WMLMacroCall__NameAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1640:1: rule__WMLMacroCall__NameAssignment_2 : ( RULE_ID ) ;
    public final void rule__WMLMacroCall__NameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1644:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1645:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1645:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1646:1: RULE_ID
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_2_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_23368); 
             after(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__NameAssignment_2


    // $ANTLR start rule__WMLMacroCall__ParamsAssignment_3_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1655:1: rule__WMLMacroCall__ParamsAssignment_3_0 : ( ( rule__WMLMacroCall__ParamsAlternatives_3_0_0 ) ) ;
    public final void rule__WMLMacroCall__ParamsAssignment_3_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1659:1: ( ( ( rule__WMLMacroCall__ParamsAlternatives_3_0_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1660:1: ( ( rule__WMLMacroCall__ParamsAlternatives_3_0_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1660:1: ( ( rule__WMLMacroCall__ParamsAlternatives_3_0_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1661:1: ( rule__WMLMacroCall__ParamsAlternatives_3_0_0 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getParamsAlternatives_3_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1662:1: ( rule__WMLMacroCall__ParamsAlternatives_3_0_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1662:2: rule__WMLMacroCall__ParamsAlternatives_3_0_0
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__ParamsAlternatives_3_0_0_in_rule__WMLMacroCall__ParamsAssignment_3_03399);
            rule__WMLMacroCall__ParamsAlternatives_3_0_0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroCallAccess().getParamsAlternatives_3_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__ParamsAssignment_3_0


    // $ANTLR start rule__WMLMacroCall__ExtraMacrosAssignment_3_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1671:1: rule__WMLMacroCall__ExtraMacrosAssignment_3_1 : ( ruleWMLMacroCall ) ;
    public final void rule__WMLMacroCall__ExtraMacrosAssignment_3_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1675:1: ( ( ruleWMLMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1676:1: ( ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1676:1: ( ruleWMLMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1677:1: ruleWMLMacroCall
            {
             before(grammarAccess.getWMLMacroCallAccess().getExtraMacrosWMLMacroCallParserRuleCall_3_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCall__ExtraMacrosAssignment_3_13432);
            ruleWMLMacroCall();
            _fsp--;

             after(grammarAccess.getWMLMacroCallAccess().getExtraMacrosWMLMacroCallParserRuleCall_3_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__ExtraMacrosAssignment_3_1


    // $ANTLR start rule__WMLMacroParameter__ParamAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1686:1: rule__WMLMacroParameter__ParamAssignment_1 : ( ( rule__WMLMacroParameter__ParamAlternatives_1_0 ) ) ;
    public final void rule__WMLMacroParameter__ParamAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1690:1: ( ( ( rule__WMLMacroParameter__ParamAlternatives_1_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1691:1: ( ( rule__WMLMacroParameter__ParamAlternatives_1_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1691:1: ( ( rule__WMLMacroParameter__ParamAlternatives_1_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1692:1: ( rule__WMLMacroParameter__ParamAlternatives_1_0 )
            {
             before(grammarAccess.getWMLMacroParameterAccess().getParamAlternatives_1_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1693:1: ( rule__WMLMacroParameter__ParamAlternatives_1_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1693:2: rule__WMLMacroParameter__ParamAlternatives_1_0
            {
            pushFollow(FOLLOW_rule__WMLMacroParameter__ParamAlternatives_1_0_in_rule__WMLMacroParameter__ParamAssignment_13463);
            rule__WMLMacroParameter__ParamAlternatives_1_0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroParameterAccess().getParamAlternatives_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroParameter__ParamAssignment_1


    // $ANTLR start rule__WMLLuaCode__ValueAssignment
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1702:1: rule__WMLLuaCode__ValueAssignment : ( RULE_LUA_CODE ) ;
    public final void rule__WMLLuaCode__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1706:1: ( ( RULE_LUA_CODE ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1707:1: ( RULE_LUA_CODE )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1707:1: ( RULE_LUA_CODE )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1708:1: RULE_LUA_CODE
            {
             before(grammarAccess.getWMLLuaCodeAccess().getValueLUA_CODETerminalRuleCall_0()); 
            match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment3496); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1717:1: rule__WMLArrayCall__ValueAssignment_1 : ( ruleWMLValue ) ;
    public final void rule__WMLArrayCall__ValueAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1721:1: ( ( ruleWMLValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1722:1: ( ruleWMLValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1722:1: ( ruleWMLValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1723:1: ruleWMLValue
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_13527);
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


    // $ANTLR start rule__WMLMacroDefine__NameAssignment
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1732:1: rule__WMLMacroDefine__NameAssignment : ( RULE_DEFINE ) ;
    public final void rule__WMLMacroDefine__NameAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1736:1: ( ( RULE_DEFINE ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1737:1: ( RULE_DEFINE )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1737:1: ( RULE_DEFINE )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1738:1: RULE_DEFINE
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0()); 
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment3558); 
             after(grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__NameAssignment


    // $ANTLR start rule__WMLTextdomain__NameAssignment
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1747:1: rule__WMLTextdomain__NameAssignment : ( RULE_TEXTDOMAIN ) ;
    public final void rule__WMLTextdomain__NameAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1751:1: ( ( RULE_TEXTDOMAIN ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1752:1: ( RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1752:1: ( RULE_TEXTDOMAIN )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1753:1: RULE_TEXTDOMAIN
            {
             before(grammarAccess.getWMLTextdomainAccess().getNameTEXTDOMAINTerminalRuleCall_0()); 
            match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment3589); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1762:1: rule__WMLValue__ValueAssignment : ( ( rule__WMLValue__ValueAlternatives_0 ) ) ;
    public final void rule__WMLValue__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1766:1: ( ( ( rule__WMLValue__ValueAlternatives_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1767:1: ( ( rule__WMLValue__ValueAlternatives_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1767:1: ( ( rule__WMLValue__ValueAlternatives_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1768:1: ( rule__WMLValue__ValueAlternatives_0 )
            {
             before(grammarAccess.getWMLValueAccess().getValueAlternatives_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1769:1: ( rule__WMLValue__ValueAlternatives_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1769:2: rule__WMLValue__ValueAlternatives_0
            {
            pushFollow(FOLLOW_rule__WMLValue__ValueAlternatives_0_in_rule__WMLValue__ValueAssignment3620);
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


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot61 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot68 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__Alternatives_in_ruleWMLRoot94 = new BitSet(new long[]{0x0000000000088C02L});
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
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter371 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroParameter378 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroParameter__Group__0_in_ruleWMLMacroParameter404 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode431 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLLuaCode438 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLLuaCode__ValueAssignment_in_ruleWMLLuaCode464 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall491 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLArrayCall498 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__0_in_ruleWMLArrayCall524 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine551 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine558 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__NameAssignment_in_ruleWMLMacroDefine584 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain611 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTextdomain618 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTextdomain__NameAssignment_in_ruleWMLTextdomain644 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_entryRuleWMLValue671 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValue678 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValue__ValueAssignment_in_ruleWMLValue704 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__TagsAssignment_0_in_rule__WMLRoot__Alternatives740 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__MacroCallsAssignment_1_in_rule__WMLRoot__Alternatives758 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__MacroDefinesAssignment_2_in_rule__WMLRoot__Alternatives776 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__TextdomainsAssignment_3_in_rule__WMLRoot__Alternatives794 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TagsAssignment_4_0_in_rule__WMLTag__Alternatives_4827 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__KeysAssignment_4_1_in_rule__WMLTag__Alternatives_4845 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__MacroCallsAssignment_4_2_in_rule__WMLTag__Alternatives_4863 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__MacroDefinesAssignment_4_3_in_rule__WMLTag__Alternatives_4881 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TextdomainsAssignment_4_4_in_rule__WMLTag__Alternatives_4899 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__EolAlternatives_3_0932 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_SL_COMMENT_in_rule__WMLKey__EolAlternatives_3_0949 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLKeyValue__Alternatives981 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives998 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_rule__WMLKeyValue__Alternatives1015 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_rule__WMLKeyValue__Alternatives1032 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ParamsAssignment_3_0_in_rule__WMLMacroCall__Alternatives_31064 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ExtraMacrosAssignment_3_1_in_rule__WMLMacroCall__Alternatives_31082 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLMacroCall__ParamsAlternatives_3_0_01115 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_rule__WMLMacroCall__ParamsAlternatives_3_0_01132 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLMacroParameter__ParamAlternatives_1_01164 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLMacroParameter__ParamAlternatives_1_01181 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroParameter__ParamAlternatives_1_01198 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLMacroParameter__ParamAlternatives_1_01215 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLValue__ValueAlternatives_01247 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLValue__ValueAlternatives_01264 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__WMLValue__ValueAlternatives_01282 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__WMLValue__ValueAlternatives_01302 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__ValueAlternatives_01321 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01351 = new BitSet(new long[]{0x0000000000002040L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01354 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLTag__Group__0__Impl1382 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__11413 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__11416 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl1443 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__21474 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__21477 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl1504 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__31534 = new BitSet(new long[]{0x00000000000A8C40L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__31537 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__WMLTag__Group__3__Impl1565 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__41596 = new BitSet(new long[]{0x0000000000020000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__41599 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Alternatives_4_in_rule__WMLTag__Group__4__Impl1626 = new BitSet(new long[]{0x0000000000088C42L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__51657 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__51660 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_rule__WMLTag__Group__5__Impl1688 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__61719 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__61722 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl1749 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__71779 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__WMLTag__Group__7__Impl1807 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__01854 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__01857 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl1884 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__11914 = new BitSet(new long[]{0x000000000008E3C0L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__11917 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_rule__WMLKey__Group__1__Impl1945 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__21976 = new BitSet(new long[]{0x0000000000000030L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__21979 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2008 = new BitSet(new long[]{0x000000000008E3C2L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2020 = new BitSet(new long[]{0x000000000008E3C2L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__32053 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__EolAssignment_3_in_rule__WMLKey__Group__3__Impl2080 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__02118 = new BitSet(new long[]{0x0000000000004040L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__02121 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_rule__WMLMacroCall__Group__0__Impl2149 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__12180 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__12183 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__RelativeAssignment_1_in_rule__WMLMacroCall__Group__1__Impl2210 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__22241 = new BitSet(new long[]{0x00000000003861C0L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__22244 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__NameAssignment_2_in_rule__WMLMacroCall__Group__2__Impl2271 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__32301 = new BitSet(new long[]{0x0000000000100000L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__32304 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Alternatives_3_in_rule__WMLMacroCall__Group__3__Impl2331 = new BitSet(new long[]{0x00000000002861C2L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__42362 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__WMLMacroCall__Group__4__Impl2390 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroParameter__Group__0__Impl_in_rule__WMLMacroParameter__Group__02431 = new BitSet(new long[]{0x000000000008E1C0L});
    public static final BitSet FOLLOW_rule__WMLMacroParameter__Group__1_in_rule__WMLMacroParameter__Group__02434 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLMacroParameter__Group__0__Impl2462 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroParameter__Group__1__Impl_in_rule__WMLMacroParameter__Group__12493 = new BitSet(new long[]{0x0000000000400000L});
    public static final BitSet FOLLOW_rule__WMLMacroParameter__Group__2_in_rule__WMLMacroParameter__Group__12496 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroParameter__ParamAssignment_1_in_rule__WMLMacroParameter__Group__1__Impl2525 = new BitSet(new long[]{0x000000000008E1C2L});
    public static final BitSet FOLLOW_rule__WMLMacroParameter__ParamAssignment_1_in_rule__WMLMacroParameter__Group__1__Impl2537 = new BitSet(new long[]{0x000000000008E1C2L});
    public static final BitSet FOLLOW_rule__WMLMacroParameter__Group__2__Impl_in_rule__WMLMacroParameter__Group__22570 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__WMLMacroParameter__Group__2__Impl2598 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__02635 = new BitSet(new long[]{0x00000000000061C0L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__02638 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLArrayCall__Group__0__Impl2666 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__12697 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__12700 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl2729 = new BitSet(new long[]{0x00000000000061C2L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl2741 = new BitSet(new long[]{0x00000000000061C2L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__22774 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__WMLArrayCall__Group__2__Impl2802 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRoot__TagsAssignment_02844 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLRoot__MacroCallsAssignment_12875 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLRoot__MacroDefinesAssignment_22906 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_rule__WMLRoot__TextdomainsAssignment_32937 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__WMLTag__PlusAssignment_12973 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_23012 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLTag__TagsAssignment_4_03043 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLTag__KeysAssignment_4_13074 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLTag__MacroCallsAssignment_4_23105 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLTag__MacroDefinesAssignment_4_33136 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_rule__WMLTag__TextdomainsAssignment_4_43167 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_63198 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_03229 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_23260 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__EolAlternatives_3_0_in_rule__WMLKey__EolAssignment_33291 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__WMLMacroCall__RelativeAssignment_13329 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_23368 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ParamsAlternatives_3_0_0_in_rule__WMLMacroCall__ParamsAssignment_3_03399 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCall__ExtraMacrosAssignment_3_13432 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroParameter__ParamAlternatives_1_0_in_rule__WMLMacroParameter__ParamAssignment_13463 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment3496 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_13527 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment3558 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment3589 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValue__ValueAlternatives_0_in_rule__WMLValue__ValueAssignment3620 = new BitSet(new long[]{0x0000000000000002L});

}