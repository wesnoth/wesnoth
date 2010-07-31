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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_EOL", "RULE_SL_COMMENT", "RULE_ID", "RULE_STRING", "RULE_ANY_OTHER", "RULE_LUA_CODE", "RULE_DEFINE", "RULE_TEXTDOMAIN", "RULE_WS", "'+'", "'['", "']'", "'[/'", "'='", "'{'", "'}'", "'~'"
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

                if ( ((LA1_0>=RULE_DEFINE && LA1_0<=RULE_TEXTDOMAIN)||LA1_0==14||LA1_0==18) ) {
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:211:1: entryRuleWMLLuaCode : ruleWMLLuaCode EOF ;
    public final void entryRuleWMLLuaCode() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:212:1: ( ruleWMLLuaCode EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:213:1: ruleWMLLuaCode EOF
            {
             before(grammarAccess.getWMLLuaCodeRule()); 
            pushFollow(FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode373);
            ruleWMLLuaCode();
            _fsp--;

             after(grammarAccess.getWMLLuaCodeRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLLuaCode380); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:220:1: ruleWMLLuaCode : ( ( rule__WMLLuaCode__ValueAssignment ) ) ;
    public final void ruleWMLLuaCode() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:224:2: ( ( ( rule__WMLLuaCode__ValueAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:225:1: ( ( rule__WMLLuaCode__ValueAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:225:1: ( ( rule__WMLLuaCode__ValueAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:226:1: ( rule__WMLLuaCode__ValueAssignment )
            {
             before(grammarAccess.getWMLLuaCodeAccess().getValueAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:227:1: ( rule__WMLLuaCode__ValueAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:227:2: rule__WMLLuaCode__ValueAssignment
            {
            pushFollow(FOLLOW_rule__WMLLuaCode__ValueAssignment_in_ruleWMLLuaCode406);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:239:1: entryRuleWMLArrayCall : ruleWMLArrayCall EOF ;
    public final void entryRuleWMLArrayCall() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:240:1: ( ruleWMLArrayCall EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:241:1: ruleWMLArrayCall EOF
            {
             before(grammarAccess.getWMLArrayCallRule()); 
            pushFollow(FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall433);
            ruleWMLArrayCall();
            _fsp--;

             after(grammarAccess.getWMLArrayCallRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLArrayCall440); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:248:1: ruleWMLArrayCall : ( ( rule__WMLArrayCall__Group__0 ) ) ;
    public final void ruleWMLArrayCall() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:252:2: ( ( ( rule__WMLArrayCall__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:253:1: ( ( rule__WMLArrayCall__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:253:1: ( ( rule__WMLArrayCall__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:254:1: ( rule__WMLArrayCall__Group__0 )
            {
             before(grammarAccess.getWMLArrayCallAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:255:1: ( rule__WMLArrayCall__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:255:2: rule__WMLArrayCall__Group__0
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__0_in_ruleWMLArrayCall466);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:267:1: entryRuleWMLMacroDefine : ruleWMLMacroDefine EOF ;
    public final void entryRuleWMLMacroDefine() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:268:1: ( ruleWMLMacroDefine EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:269:1: ruleWMLMacroDefine EOF
            {
             before(grammarAccess.getWMLMacroDefineRule()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine493);
            ruleWMLMacroDefine();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroDefine500); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:276:1: ruleWMLMacroDefine : ( ( rule__WMLMacroDefine__NameAssignment ) ) ;
    public final void ruleWMLMacroDefine() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:280:2: ( ( ( rule__WMLMacroDefine__NameAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:1: ( ( rule__WMLMacroDefine__NameAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:1: ( ( rule__WMLMacroDefine__NameAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:282:1: ( rule__WMLMacroDefine__NameAssignment )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:283:1: ( rule__WMLMacroDefine__NameAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:283:2: rule__WMLMacroDefine__NameAssignment
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__NameAssignment_in_ruleWMLMacroDefine526);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:295:1: entryRuleWMLTextdomain : ruleWMLTextdomain EOF ;
    public final void entryRuleWMLTextdomain() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:296:1: ( ruleWMLTextdomain EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:297:1: ruleWMLTextdomain EOF
            {
             before(grammarAccess.getWMLTextdomainRule()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain553);
            ruleWMLTextdomain();
            _fsp--;

             after(grammarAccess.getWMLTextdomainRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTextdomain560); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:304:1: ruleWMLTextdomain : ( ( rule__WMLTextdomain__NameAssignment ) ) ;
    public final void ruleWMLTextdomain() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:308:2: ( ( ( rule__WMLTextdomain__NameAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:1: ( ( rule__WMLTextdomain__NameAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:1: ( ( rule__WMLTextdomain__NameAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:310:1: ( rule__WMLTextdomain__NameAssignment )
            {
             before(grammarAccess.getWMLTextdomainAccess().getNameAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:311:1: ( rule__WMLTextdomain__NameAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:311:2: rule__WMLTextdomain__NameAssignment
            {
            pushFollow(FOLLOW_rule__WMLTextdomain__NameAssignment_in_ruleWMLTextdomain586);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:323:1: entryRuleWMLValue : ruleWMLValue EOF ;
    public final void entryRuleWMLValue() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:324:1: ( ruleWMLValue EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:325:1: ruleWMLValue EOF
            {
             before(grammarAccess.getWMLValueRule()); 
            pushFollow(FOLLOW_ruleWMLValue_in_entryRuleWMLValue613);
            ruleWMLValue();
            _fsp--;

             after(grammarAccess.getWMLValueRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValue620); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:332:1: ruleWMLValue : ( ( rule__WMLValue__ValueAssignment ) ) ;
    public final void ruleWMLValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:336:2: ( ( ( rule__WMLValue__ValueAssignment ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:1: ( ( rule__WMLValue__ValueAssignment ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:1: ( ( rule__WMLValue__ValueAssignment ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:338:1: ( rule__WMLValue__ValueAssignment )
            {
             before(grammarAccess.getWMLValueAccess().getValueAssignment()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:339:1: ( rule__WMLValue__ValueAssignment )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:339:2: rule__WMLValue__ValueAssignment
            {
            pushFollow(FOLLOW_rule__WMLValue__ValueAssignment_in_ruleWMLValue646);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:351:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__TagsAssignment_0 ) ) | ( ( rule__WMLRoot__MacroCallsAssignment_1 ) ) | ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) ) | ( ( rule__WMLRoot__TextdomainsAssignment_3 ) ) );
    public final void rule__WMLRoot__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:355:1: ( ( ( rule__WMLRoot__TagsAssignment_0 ) ) | ( ( rule__WMLRoot__MacroCallsAssignment_1 ) ) | ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) ) | ( ( rule__WMLRoot__TextdomainsAssignment_3 ) ) )
            int alt2=4;
            switch ( input.LA(1) ) {
            case 14:
                {
                alt2=1;
                }
                break;
            case 18:
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
                    new NoViableAltException("351:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__TagsAssignment_0 ) ) | ( ( rule__WMLRoot__MacroCallsAssignment_1 ) ) | ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) ) | ( ( rule__WMLRoot__TextdomainsAssignment_3 ) ) );", 2, 0, input);

                throw nvae;
            }

            switch (alt2) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:356:1: ( ( rule__WMLRoot__TagsAssignment_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:356:1: ( ( rule__WMLRoot__TagsAssignment_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:357:1: ( rule__WMLRoot__TagsAssignment_0 )
                    {
                     before(grammarAccess.getWMLRootAccess().getTagsAssignment_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:358:1: ( rule__WMLRoot__TagsAssignment_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:358:2: rule__WMLRoot__TagsAssignment_0
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__TagsAssignment_0_in_rule__WMLRoot__Alternatives682);
                    rule__WMLRoot__TagsAssignment_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getTagsAssignment_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:362:6: ( ( rule__WMLRoot__MacroCallsAssignment_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:362:6: ( ( rule__WMLRoot__MacroCallsAssignment_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:363:1: ( rule__WMLRoot__MacroCallsAssignment_1 )
                    {
                     before(grammarAccess.getWMLRootAccess().getMacroCallsAssignment_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:364:1: ( rule__WMLRoot__MacroCallsAssignment_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:364:2: rule__WMLRoot__MacroCallsAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__MacroCallsAssignment_1_in_rule__WMLRoot__Alternatives700);
                    rule__WMLRoot__MacroCallsAssignment_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getMacroCallsAssignment_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:368:6: ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:368:6: ( ( rule__WMLRoot__MacroDefinesAssignment_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:369:1: ( rule__WMLRoot__MacroDefinesAssignment_2 )
                    {
                     before(grammarAccess.getWMLRootAccess().getMacroDefinesAssignment_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:370:1: ( rule__WMLRoot__MacroDefinesAssignment_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:370:2: rule__WMLRoot__MacroDefinesAssignment_2
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__MacroDefinesAssignment_2_in_rule__WMLRoot__Alternatives718);
                    rule__WMLRoot__MacroDefinesAssignment_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getMacroDefinesAssignment_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:374:6: ( ( rule__WMLRoot__TextdomainsAssignment_3 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:374:6: ( ( rule__WMLRoot__TextdomainsAssignment_3 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:375:1: ( rule__WMLRoot__TextdomainsAssignment_3 )
                    {
                     before(grammarAccess.getWMLRootAccess().getTextdomainsAssignment_3()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:376:1: ( rule__WMLRoot__TextdomainsAssignment_3 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:376:2: rule__WMLRoot__TextdomainsAssignment_3
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__TextdomainsAssignment_3_in_rule__WMLRoot__Alternatives736);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:385:1: rule__WMLTag__Alternatives_4 : ( ( ( rule__WMLTag__TagsAssignment_4_0 ) ) | ( ( rule__WMLTag__KeysAssignment_4_1 ) ) | ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) ) | ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) ) | ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) ) );
    public final void rule__WMLTag__Alternatives_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:389:1: ( ( ( rule__WMLTag__TagsAssignment_4_0 ) ) | ( ( rule__WMLTag__KeysAssignment_4_1 ) ) | ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) ) | ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) ) | ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) ) )
            int alt3=5;
            switch ( input.LA(1) ) {
            case 14:
                {
                alt3=1;
                }
                break;
            case RULE_ID:
                {
                alt3=2;
                }
                break;
            case 18:
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
                    new NoViableAltException("385:1: rule__WMLTag__Alternatives_4 : ( ( ( rule__WMLTag__TagsAssignment_4_0 ) ) | ( ( rule__WMLTag__KeysAssignment_4_1 ) ) | ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) ) | ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) ) | ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) ) );", 3, 0, input);

                throw nvae;
            }

            switch (alt3) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:390:1: ( ( rule__WMLTag__TagsAssignment_4_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:390:1: ( ( rule__WMLTag__TagsAssignment_4_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:391:1: ( rule__WMLTag__TagsAssignment_4_0 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTagsAssignment_4_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:392:1: ( rule__WMLTag__TagsAssignment_4_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:392:2: rule__WMLTag__TagsAssignment_4_0
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TagsAssignment_4_0_in_rule__WMLTag__Alternatives_4769);
                    rule__WMLTag__TagsAssignment_4_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTagsAssignment_4_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:396:6: ( ( rule__WMLTag__KeysAssignment_4_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:396:6: ( ( rule__WMLTag__KeysAssignment_4_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:397:1: ( rule__WMLTag__KeysAssignment_4_1 )
                    {
                     before(grammarAccess.getWMLTagAccess().getKeysAssignment_4_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:398:1: ( rule__WMLTag__KeysAssignment_4_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:398:2: rule__WMLTag__KeysAssignment_4_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__KeysAssignment_4_1_in_rule__WMLTag__Alternatives_4787);
                    rule__WMLTag__KeysAssignment_4_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getKeysAssignment_4_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:402:6: ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:402:6: ( ( rule__WMLTag__MacroCallsAssignment_4_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:403:1: ( rule__WMLTag__MacroCallsAssignment_4_2 )
                    {
                     before(grammarAccess.getWMLTagAccess().getMacroCallsAssignment_4_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:404:1: ( rule__WMLTag__MacroCallsAssignment_4_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:404:2: rule__WMLTag__MacroCallsAssignment_4_2
                    {
                    pushFollow(FOLLOW_rule__WMLTag__MacroCallsAssignment_4_2_in_rule__WMLTag__Alternatives_4805);
                    rule__WMLTag__MacroCallsAssignment_4_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getMacroCallsAssignment_4_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:408:6: ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:408:6: ( ( rule__WMLTag__MacroDefinesAssignment_4_3 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:409:1: ( rule__WMLTag__MacroDefinesAssignment_4_3 )
                    {
                     before(grammarAccess.getWMLTagAccess().getMacroDefinesAssignment_4_3()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:410:1: ( rule__WMLTag__MacroDefinesAssignment_4_3 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:410:2: rule__WMLTag__MacroDefinesAssignment_4_3
                    {
                    pushFollow(FOLLOW_rule__WMLTag__MacroDefinesAssignment_4_3_in_rule__WMLTag__Alternatives_4823);
                    rule__WMLTag__MacroDefinesAssignment_4_3();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getMacroDefinesAssignment_4_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:414:6: ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:414:6: ( ( rule__WMLTag__TextdomainsAssignment_4_4 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:415:1: ( rule__WMLTag__TextdomainsAssignment_4_4 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTextdomainsAssignment_4_4()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:416:1: ( rule__WMLTag__TextdomainsAssignment_4_4 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:416:2: rule__WMLTag__TextdomainsAssignment_4_4
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TextdomainsAssignment_4_4_in_rule__WMLTag__Alternatives_4841);
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


    // $ANTLR start rule__WMLKey__Alternatives_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:425:1: rule__WMLKey__Alternatives_3 : ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) );
    public final void rule__WMLKey__Alternatives_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:429:1: ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) )
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
                    new NoViableAltException("425:1: rule__WMLKey__Alternatives_3 : ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) );", 4, 0, input);

                throw nvae;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:430:1: ( RULE_EOL )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:430:1: ( RULE_EOL )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:431:1: RULE_EOL
                    {
                     before(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_0()); 
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__Alternatives_3874); 
                     after(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:436:6: ( RULE_SL_COMMENT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:436:6: ( RULE_SL_COMMENT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:437:1: RULE_SL_COMMENT
                    {
                     before(grammarAccess.getWMLKeyAccess().getSL_COMMENTTerminalRuleCall_3_1()); 
                    match(input,RULE_SL_COMMENT,FOLLOW_RULE_SL_COMMENT_in_rule__WMLKey__Alternatives_3891); 
                     after(grammarAccess.getWMLKeyAccess().getSL_COMMENTTerminalRuleCall_3_1()); 

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
    // $ANTLR end rule__WMLKey__Alternatives_3


    // $ANTLR start rule__WMLKeyValue__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:447:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) );
    public final void rule__WMLKeyValue__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:451:1: ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) )
            int alt5=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
            case RULE_STRING:
            case RULE_ANY_OTHER:
            case 13:
                {
                alt5=1;
                }
                break;
            case 18:
                {
                alt5=2;
                }
                break;
            case RULE_LUA_CODE:
                {
                alt5=3;
                }
                break;
            case 14:
                {
                alt5=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("447:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) );", 5, 0, input);

                throw nvae;
            }

            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:452:1: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:452:1: ( ruleWMLValue )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:453:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLKeyValue__Alternatives923);
                    ruleWMLValue();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:458:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:458:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:459:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives940);
                    ruleWMLMacroCall();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:464:6: ( ruleWMLLuaCode )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:464:6: ( ruleWMLLuaCode )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:465:1: ruleWMLLuaCode
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2()); 
                    pushFollow(FOLLOW_ruleWMLLuaCode_in_rule__WMLKeyValue__Alternatives957);
                    ruleWMLLuaCode();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:470:6: ( ruleWMLArrayCall )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:470:6: ( ruleWMLArrayCall )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:471:1: ruleWMLArrayCall
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLArrayCallParserRuleCall_3()); 
                    pushFollow(FOLLOW_ruleWMLArrayCall_in_rule__WMLKeyValue__Alternatives974);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:481:1: rule__WMLMacroCall__Alternatives_3 : ( ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) ) | ( ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 ) ) );
    public final void rule__WMLMacroCall__Alternatives_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:485:1: ( ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) ) | ( ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 ) ) )
            int alt6=2;
            int LA6_0 = input.LA(1);

            if ( ((LA6_0>=RULE_ID && LA6_0<=RULE_ANY_OTHER)||LA6_0==13) ) {
                alt6=1;
            }
            else if ( (LA6_0==18) ) {
                alt6=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("481:1: rule__WMLMacroCall__Alternatives_3 : ( ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) ) | ( ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 ) ) );", 6, 0, input);

                throw nvae;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:486:1: ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:486:1: ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:487:1: ( rule__WMLMacroCall__ParamsAssignment_3_0 )
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getParamsAssignment_3_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:488:1: ( rule__WMLMacroCall__ParamsAssignment_3_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:488:2: rule__WMLMacroCall__ParamsAssignment_3_0
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__ParamsAssignment_3_0_in_rule__WMLMacroCall__Alternatives_31006);
                    rule__WMLMacroCall__ParamsAssignment_3_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroCallAccess().getParamsAssignment_3_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:492:6: ( ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:492:6: ( ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:493:1: ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 )
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getExtraMacrosAssignment_3_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:494:1: ( rule__WMLMacroCall__ExtraMacrosAssignment_3_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:494:2: rule__WMLMacroCall__ExtraMacrosAssignment_3_1
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__ExtraMacrosAssignment_3_1_in_rule__WMLMacroCall__Alternatives_31024);
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


    // $ANTLR start rule__WMLValue__ValueAlternatives_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:504:1: rule__WMLValue__ValueAlternatives_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( RULE_ANY_OTHER ) | ( '+' ) );
    public final void rule__WMLValue__ValueAlternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:508:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( RULE_ANY_OTHER ) | ( '+' ) )
            int alt7=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt7=1;
                }
                break;
            case RULE_STRING:
                {
                alt7=2;
                }
                break;
            case RULE_ANY_OTHER:
                {
                alt7=3;
                }
                break;
            case 13:
                {
                alt7=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("504:1: rule__WMLValue__ValueAlternatives_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( RULE_ANY_OTHER ) | ( '+' ) );", 7, 0, input);

                throw nvae;
            }

            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:509:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:509:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:510:1: RULE_ID
                    {
                     before(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLValue__ValueAlternatives_01058); 
                     after(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:515:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:515:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:516:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLValue__ValueAlternatives_01075); 
                     after(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:521:6: ( RULE_ANY_OTHER )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:521:6: ( RULE_ANY_OTHER )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:522:1: RULE_ANY_OTHER
                    {
                     before(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_2()); 
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__ValueAlternatives_01092); 
                     after(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:527:6: ( '+' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:527:6: ( '+' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:528:1: '+'
                    {
                     before(grammarAccess.getWMLValueAccess().getValuePlusSignKeyword_0_3()); 
                    match(input,13,FOLLOW_13_in_rule__WMLValue__ValueAlternatives_01110); 
                     after(grammarAccess.getWMLValueAccess().getValuePlusSignKeyword_0_3()); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:542:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:546:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:547:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01142);
            rule__WMLTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01145);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:554:1: rule__WMLTag__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:558:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:559:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:559:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:560:1: '['
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0()); 
            match(input,14,FOLLOW_14_in_rule__WMLTag__Group__0__Impl1173); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:573:1: rule__WMLTag__Group__1 : rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 ;
    public final void rule__WMLTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:577:1: ( rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:578:2: rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__11204);
            rule__WMLTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__11207);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:585:1: rule__WMLTag__Group__1__Impl : ( ( rule__WMLTag__PlusAssignment_1 )? ) ;
    public final void rule__WMLTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:589:1: ( ( ( rule__WMLTag__PlusAssignment_1 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:590:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:590:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:591:1: ( rule__WMLTag__PlusAssignment_1 )?
            {
             before(grammarAccess.getWMLTagAccess().getPlusAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:592:1: ( rule__WMLTag__PlusAssignment_1 )?
            int alt8=2;
            int LA8_0 = input.LA(1);

            if ( (LA8_0==13) ) {
                alt8=1;
            }
            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:592:2: rule__WMLTag__PlusAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl1234);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:602:1: rule__WMLTag__Group__2 : rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 ;
    public final void rule__WMLTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:606:1: ( rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:607:2: rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__21265);
            rule__WMLTag__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__21268);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:614:1: rule__WMLTag__Group__2__Impl : ( ( rule__WMLTag__NameAssignment_2 ) ) ;
    public final void rule__WMLTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:618:1: ( ( ( rule__WMLTag__NameAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:619:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:619:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:620:1: ( rule__WMLTag__NameAssignment_2 )
            {
             before(grammarAccess.getWMLTagAccess().getNameAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:621:1: ( rule__WMLTag__NameAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:621:2: rule__WMLTag__NameAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl1295);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:631:1: rule__WMLTag__Group__3 : rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 ;
    public final void rule__WMLTag__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:635:1: ( rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:636:2: rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__31325);
            rule__WMLTag__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__31328);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:643:1: rule__WMLTag__Group__3__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:647:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:648:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:648:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:649:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3()); 
            match(input,15,FOLLOW_15_in_rule__WMLTag__Group__3__Impl1356); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:662:1: rule__WMLTag__Group__4 : rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 ;
    public final void rule__WMLTag__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:666:1: ( rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:667:2: rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__41387);
            rule__WMLTag__Group__4__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__41390);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:674:1: rule__WMLTag__Group__4__Impl : ( ( rule__WMLTag__Alternatives_4 )* ) ;
    public final void rule__WMLTag__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:678:1: ( ( ( rule__WMLTag__Alternatives_4 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:679:1: ( ( rule__WMLTag__Alternatives_4 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:679:1: ( ( rule__WMLTag__Alternatives_4 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:680:1: ( rule__WMLTag__Alternatives_4 )*
            {
             before(grammarAccess.getWMLTagAccess().getAlternatives_4()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:681:1: ( rule__WMLTag__Alternatives_4 )*
            loop9:
            do {
                int alt9=2;
                int LA9_0 = input.LA(1);

                if ( (LA9_0==RULE_ID||(LA9_0>=RULE_DEFINE && LA9_0<=RULE_TEXTDOMAIN)||LA9_0==14||LA9_0==18) ) {
                    alt9=1;
                }


                switch (alt9) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:681:2: rule__WMLTag__Alternatives_4
            	    {
            	    pushFollow(FOLLOW_rule__WMLTag__Alternatives_4_in_rule__WMLTag__Group__4__Impl1417);
            	    rule__WMLTag__Alternatives_4();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop9;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:691:1: rule__WMLTag__Group__5 : rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 ;
    public final void rule__WMLTag__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:695:1: ( rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:696:2: rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__51448);
            rule__WMLTag__Group__5__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__51451);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:703:1: rule__WMLTag__Group__5__Impl : ( '[/' ) ;
    public final void rule__WMLTag__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:707:1: ( ( '[/' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:708:1: ( '[/' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:708:1: ( '[/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:709:1: '[/'
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5()); 
            match(input,16,FOLLOW_16_in_rule__WMLTag__Group__5__Impl1479); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:722:1: rule__WMLTag__Group__6 : rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 ;
    public final void rule__WMLTag__Group__6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:726:1: ( rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:727:2: rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__61510);
            rule__WMLTag__Group__6__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__61513);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:734:1: rule__WMLTag__Group__6__Impl : ( ( rule__WMLTag__EndNameAssignment_6 ) ) ;
    public final void rule__WMLTag__Group__6__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:738:1: ( ( ( rule__WMLTag__EndNameAssignment_6 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:739:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:739:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:740:1: ( rule__WMLTag__EndNameAssignment_6 )
            {
             before(grammarAccess.getWMLTagAccess().getEndNameAssignment_6()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:741:1: ( rule__WMLTag__EndNameAssignment_6 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:741:2: rule__WMLTag__EndNameAssignment_6
            {
            pushFollow(FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl1540);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:751:1: rule__WMLTag__Group__7 : rule__WMLTag__Group__7__Impl ;
    public final void rule__WMLTag__Group__7() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:755:1: ( rule__WMLTag__Group__7__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:756:2: rule__WMLTag__Group__7__Impl
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__71570);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:762:1: rule__WMLTag__Group__7__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__7__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:766:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:767:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:767:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:768:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_7()); 
            match(input,15,FOLLOW_15_in_rule__WMLTag__Group__7__Impl1598); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:797:1: rule__WMLKey__Group__0 : rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 ;
    public final void rule__WMLKey__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:801:1: ( rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:802:2: rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__01645);
            rule__WMLKey__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__01648);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:809:1: rule__WMLKey__Group__0__Impl : ( ( rule__WMLKey__NameAssignment_0 ) ) ;
    public final void rule__WMLKey__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:813:1: ( ( ( rule__WMLKey__NameAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:814:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:814:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:815:1: ( rule__WMLKey__NameAssignment_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getNameAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:816:1: ( rule__WMLKey__NameAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:816:2: rule__WMLKey__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl1675);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:826:1: rule__WMLKey__Group__1 : rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 ;
    public final void rule__WMLKey__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:830:1: ( rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:831:2: rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__11705);
            rule__WMLKey__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__11708);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:838:1: rule__WMLKey__Group__1__Impl : ( '=' ) ;
    public final void rule__WMLKey__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:842:1: ( ( '=' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:843:1: ( '=' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:843:1: ( '=' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:844:1: '='
            {
             before(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1()); 
            match(input,17,FOLLOW_17_in_rule__WMLKey__Group__1__Impl1736); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:857:1: rule__WMLKey__Group__2 : rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 ;
    public final void rule__WMLKey__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:861:1: ( rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:862:2: rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__21767);
            rule__WMLKey__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__21770);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:869:1: rule__WMLKey__Group__2__Impl : ( ( ( rule__WMLKey__ValueAssignment_2 ) ) ( ( rule__WMLKey__ValueAssignment_2 )* ) ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:873:1: ( ( ( ( rule__WMLKey__ValueAssignment_2 ) ) ( ( rule__WMLKey__ValueAssignment_2 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:874:1: ( ( ( rule__WMLKey__ValueAssignment_2 ) ) ( ( rule__WMLKey__ValueAssignment_2 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:874:1: ( ( ( rule__WMLKey__ValueAssignment_2 ) ) ( ( rule__WMLKey__ValueAssignment_2 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:875:1: ( ( rule__WMLKey__ValueAssignment_2 ) ) ( ( rule__WMLKey__ValueAssignment_2 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:875:1: ( ( rule__WMLKey__ValueAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:876:1: ( rule__WMLKey__ValueAssignment_2 )
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:877:1: ( rule__WMLKey__ValueAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:877:2: rule__WMLKey__ValueAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl1799);
            rule__WMLKey__ValueAssignment_2();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:880:1: ( ( rule__WMLKey__ValueAssignment_2 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:881:1: ( rule__WMLKey__ValueAssignment_2 )*
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:882:1: ( rule__WMLKey__ValueAssignment_2 )*
            loop10:
            do {
                int alt10=2;
                int LA10_0 = input.LA(1);

                if ( ((LA10_0>=RULE_ID && LA10_0<=RULE_LUA_CODE)||(LA10_0>=13 && LA10_0<=14)||LA10_0==18) ) {
                    alt10=1;
                }


                switch (alt10) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:882:2: rule__WMLKey__ValueAssignment_2
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl1811);
            	    rule__WMLKey__ValueAssignment_2();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop10;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:893:1: rule__WMLKey__Group__3 : rule__WMLKey__Group__3__Impl ;
    public final void rule__WMLKey__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:897:1: ( rule__WMLKey__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:898:2: rule__WMLKey__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__31844);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:904:1: rule__WMLKey__Group__3__Impl : ( ( rule__WMLKey__Alternatives_3 ) ) ;
    public final void rule__WMLKey__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:908:1: ( ( ( rule__WMLKey__Alternatives_3 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:909:1: ( ( rule__WMLKey__Alternatives_3 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:909:1: ( ( rule__WMLKey__Alternatives_3 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:910:1: ( rule__WMLKey__Alternatives_3 )
            {
             before(grammarAccess.getWMLKeyAccess().getAlternatives_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:911:1: ( rule__WMLKey__Alternatives_3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:911:2: rule__WMLKey__Alternatives_3
            {
            pushFollow(FOLLOW_rule__WMLKey__Alternatives_3_in_rule__WMLKey__Group__3__Impl1871);
            rule__WMLKey__Alternatives_3();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getAlternatives_3()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:929:1: rule__WMLMacroCall__Group__0 : rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 ;
    public final void rule__WMLMacroCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:933:1: ( rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:934:2: rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__01909);
            rule__WMLMacroCall__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__01912);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:941:1: rule__WMLMacroCall__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacroCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:945:1: ( ( '{' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:946:1: ( '{' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:946:1: ( '{' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:947:1: '{'
            {
             before(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,18,FOLLOW_18_in_rule__WMLMacroCall__Group__0__Impl1940); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:960:1: rule__WMLMacroCall__Group__1 : rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 ;
    public final void rule__WMLMacroCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:964:1: ( rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:965:2: rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__11971);
            rule__WMLMacroCall__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__11974);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:972:1: rule__WMLMacroCall__Group__1__Impl : ( ( rule__WMLMacroCall__RelativeAssignment_1 )? ) ;
    public final void rule__WMLMacroCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:976:1: ( ( ( rule__WMLMacroCall__RelativeAssignment_1 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:977:1: ( ( rule__WMLMacroCall__RelativeAssignment_1 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:977:1: ( ( rule__WMLMacroCall__RelativeAssignment_1 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:978:1: ( rule__WMLMacroCall__RelativeAssignment_1 )?
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:979:1: ( rule__WMLMacroCall__RelativeAssignment_1 )?
            int alt11=2;
            int LA11_0 = input.LA(1);

            if ( (LA11_0==20) ) {
                alt11=1;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:979:2: rule__WMLMacroCall__RelativeAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__RelativeAssignment_1_in_rule__WMLMacroCall__Group__1__Impl2001);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:989:1: rule__WMLMacroCall__Group__2 : rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 ;
    public final void rule__WMLMacroCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:993:1: ( rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:994:2: rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__22032);
            rule__WMLMacroCall__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__22035);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1001:1: rule__WMLMacroCall__Group__2__Impl : ( ( rule__WMLMacroCall__NameAssignment_2 ) ) ;
    public final void rule__WMLMacroCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1005:1: ( ( ( rule__WMLMacroCall__NameAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1006:1: ( ( rule__WMLMacroCall__NameAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1006:1: ( ( rule__WMLMacroCall__NameAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1007:1: ( rule__WMLMacroCall__NameAssignment_2 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1008:1: ( rule__WMLMacroCall__NameAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1008:2: rule__WMLMacroCall__NameAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__NameAssignment_2_in_rule__WMLMacroCall__Group__2__Impl2062);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1018:1: rule__WMLMacroCall__Group__3 : rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 ;
    public final void rule__WMLMacroCall__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1022:1: ( rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1023:2: rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__32092);
            rule__WMLMacroCall__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__32095);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1030:1: rule__WMLMacroCall__Group__3__Impl : ( ( rule__WMLMacroCall__Alternatives_3 )* ) ;
    public final void rule__WMLMacroCall__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1034:1: ( ( ( rule__WMLMacroCall__Alternatives_3 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1035:1: ( ( rule__WMLMacroCall__Alternatives_3 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1035:1: ( ( rule__WMLMacroCall__Alternatives_3 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1036:1: ( rule__WMLMacroCall__Alternatives_3 )*
            {
             before(grammarAccess.getWMLMacroCallAccess().getAlternatives_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1037:1: ( rule__WMLMacroCall__Alternatives_3 )*
            loop12:
            do {
                int alt12=2;
                int LA12_0 = input.LA(1);

                if ( ((LA12_0>=RULE_ID && LA12_0<=RULE_ANY_OTHER)||LA12_0==13||LA12_0==18) ) {
                    alt12=1;
                }


                switch (alt12) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1037:2: rule__WMLMacroCall__Alternatives_3
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroCall__Alternatives_3_in_rule__WMLMacroCall__Group__3__Impl2122);
            	    rule__WMLMacroCall__Alternatives_3();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop12;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1047:1: rule__WMLMacroCall__Group__4 : rule__WMLMacroCall__Group__4__Impl ;
    public final void rule__WMLMacroCall__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1051:1: ( rule__WMLMacroCall__Group__4__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1052:2: rule__WMLMacroCall__Group__4__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__42153);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1058:1: rule__WMLMacroCall__Group__4__Impl : ( '}' ) ;
    public final void rule__WMLMacroCall__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1062:1: ( ( '}' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1063:1: ( '}' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1063:1: ( '}' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1064:1: '}'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_4()); 
            match(input,19,FOLLOW_19_in_rule__WMLMacroCall__Group__4__Impl2181); 
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


    // $ANTLR start rule__WMLArrayCall__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1088:1: rule__WMLArrayCall__Group__0 : rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 ;
    public final void rule__WMLArrayCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1092:1: ( rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1093:2: rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__02223);
            rule__WMLArrayCall__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__02226);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1100:1: rule__WMLArrayCall__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLArrayCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1104:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1105:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1105:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1106:1: '['
            {
             before(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0()); 
            match(input,14,FOLLOW_14_in_rule__WMLArrayCall__Group__0__Impl2254); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1119:1: rule__WMLArrayCall__Group__1 : rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 ;
    public final void rule__WMLArrayCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1123:1: ( rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1124:2: rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__12285);
            rule__WMLArrayCall__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__12288);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1131:1: rule__WMLArrayCall__Group__1__Impl : ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) ;
    public final void rule__WMLArrayCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1135:1: ( ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1136:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1136:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1137:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1137:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1138:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1139:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1139:2: rule__WMLArrayCall__ValueAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl2317);
            rule__WMLArrayCall__ValueAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1142:1: ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1143:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1144:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            loop13:
            do {
                int alt13=2;
                int LA13_0 = input.LA(1);

                if ( ((LA13_0>=RULE_ID && LA13_0<=RULE_ANY_OTHER)||LA13_0==13) ) {
                    alt13=1;
                }


                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1144:2: rule__WMLArrayCall__ValueAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl2329);
            	    rule__WMLArrayCall__ValueAssignment_1();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop13;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1155:1: rule__WMLArrayCall__Group__2 : rule__WMLArrayCall__Group__2__Impl ;
    public final void rule__WMLArrayCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1159:1: ( rule__WMLArrayCall__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1160:2: rule__WMLArrayCall__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__22362);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1166:1: rule__WMLArrayCall__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLArrayCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1170:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1171:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1171:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1172:1: ']'
            {
             before(grammarAccess.getWMLArrayCallAccess().getRightSquareBracketKeyword_2()); 
            match(input,15,FOLLOW_15_in_rule__WMLArrayCall__Group__2__Impl2390); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1192:1: rule__WMLRoot__TagsAssignment_0 : ( ruleWMLTag ) ;
    public final void rule__WMLRoot__TagsAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1196:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1197:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1197:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1198:1: ruleWMLTag
            {
             before(grammarAccess.getWMLRootAccess().getTagsWMLTagParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRoot__TagsAssignment_02432);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1207:1: rule__WMLRoot__MacroCallsAssignment_1 : ( ruleWMLMacroCall ) ;
    public final void rule__WMLRoot__MacroCallsAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1211:1: ( ( ruleWMLMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1212:1: ( ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1212:1: ( ruleWMLMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1213:1: ruleWMLMacroCall
            {
             before(grammarAccess.getWMLRootAccess().getMacroCallsWMLMacroCallParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLRoot__MacroCallsAssignment_12463);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1222:1: rule__WMLRoot__MacroDefinesAssignment_2 : ( ruleWMLMacroDefine ) ;
    public final void rule__WMLRoot__MacroDefinesAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1226:1: ( ( ruleWMLMacroDefine ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1227:1: ( ruleWMLMacroDefine )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1227:1: ( ruleWMLMacroDefine )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1228:1: ruleWMLMacroDefine
            {
             before(grammarAccess.getWMLRootAccess().getMacroDefinesWMLMacroDefineParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLRoot__MacroDefinesAssignment_22494);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1237:1: rule__WMLRoot__TextdomainsAssignment_3 : ( ruleWMLTextdomain ) ;
    public final void rule__WMLRoot__TextdomainsAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1241:1: ( ( ruleWMLTextdomain ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1242:1: ( ruleWMLTextdomain )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1242:1: ( ruleWMLTextdomain )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1243:1: ruleWMLTextdomain
            {
             before(grammarAccess.getWMLRootAccess().getTextdomainsWMLTextdomainParserRuleCall_3_0()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_rule__WMLRoot__TextdomainsAssignment_32525);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1252:1: rule__WMLTag__PlusAssignment_1 : ( ( '+' ) ) ;
    public final void rule__WMLTag__PlusAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1256:1: ( ( ( '+' ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1257:1: ( ( '+' ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1257:1: ( ( '+' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1258:1: ( '+' )
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1259:1: ( '+' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1260:1: '+'
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            match(input,13,FOLLOW_13_in_rule__WMLTag__PlusAssignment_12561); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1275:1: rule__WMLTag__NameAssignment_2 : ( RULE_ID ) ;
    public final void rule__WMLTag__NameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1279:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1280:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1280:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1281:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_2_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_22600); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1290:1: rule__WMLTag__TagsAssignment_4_0 : ( ruleWMLTag ) ;
    public final void rule__WMLTag__TagsAssignment_4_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1294:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1295:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1295:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1296:1: ruleWMLTag
            {
             before(grammarAccess.getWMLTagAccess().getTagsWMLTagParserRuleCall_4_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLTag__TagsAssignment_4_02631);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1305:1: rule__WMLTag__KeysAssignment_4_1 : ( ruleWMLKey ) ;
    public final void rule__WMLTag__KeysAssignment_4_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1309:1: ( ( ruleWMLKey ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1310:1: ( ruleWMLKey )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1310:1: ( ruleWMLKey )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1311:1: ruleWMLKey
            {
             before(grammarAccess.getWMLTagAccess().getKeysWMLKeyParserRuleCall_4_1_0()); 
            pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLTag__KeysAssignment_4_12662);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1320:1: rule__WMLTag__MacroCallsAssignment_4_2 : ( ruleWMLMacroCall ) ;
    public final void rule__WMLTag__MacroCallsAssignment_4_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1324:1: ( ( ruleWMLMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1325:1: ( ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1325:1: ( ruleWMLMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1326:1: ruleWMLMacroCall
            {
             before(grammarAccess.getWMLTagAccess().getMacroCallsWMLMacroCallParserRuleCall_4_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLTag__MacroCallsAssignment_4_22693);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1335:1: rule__WMLTag__MacroDefinesAssignment_4_3 : ( ruleWMLMacroDefine ) ;
    public final void rule__WMLTag__MacroDefinesAssignment_4_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1339:1: ( ( ruleWMLMacroDefine ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1340:1: ( ruleWMLMacroDefine )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1340:1: ( ruleWMLMacroDefine )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1341:1: ruleWMLMacroDefine
            {
             before(grammarAccess.getWMLTagAccess().getMacroDefinesWMLMacroDefineParserRuleCall_4_3_0()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLTag__MacroDefinesAssignment_4_32724);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1350:1: rule__WMLTag__TextdomainsAssignment_4_4 : ( ruleWMLTextdomain ) ;
    public final void rule__WMLTag__TextdomainsAssignment_4_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1354:1: ( ( ruleWMLTextdomain ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1355:1: ( ruleWMLTextdomain )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1355:1: ( ruleWMLTextdomain )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1356:1: ruleWMLTextdomain
            {
             before(grammarAccess.getWMLTagAccess().getTextdomainsWMLTextdomainParserRuleCall_4_4_0()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_rule__WMLTag__TextdomainsAssignment_4_42755);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1365:1: rule__WMLTag__EndNameAssignment_6 : ( RULE_ID ) ;
    public final void rule__WMLTag__EndNameAssignment_6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1369:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1370:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1370:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1371:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_62786); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1380:1: rule__WMLKey__NameAssignment_0 : ( RULE_ID ) ;
    public final void rule__WMLKey__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1384:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1385:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1385:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1386:1: RULE_ID
            {
             before(grammarAccess.getWMLKeyAccess().getNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_02817); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1395:1: rule__WMLKey__ValueAssignment_2 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1399:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1400:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1400:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1401:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_22848);
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


    // $ANTLR start rule__WMLMacroCall__RelativeAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1410:1: rule__WMLMacroCall__RelativeAssignment_1 : ( ( '~' ) ) ;
    public final void rule__WMLMacroCall__RelativeAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1414:1: ( ( ( '~' ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1415:1: ( ( '~' ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1415:1: ( ( '~' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1416:1: ( '~' )
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_1_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1417:1: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1418:1: '~'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_1_0()); 
            match(input,20,FOLLOW_20_in_rule__WMLMacroCall__RelativeAssignment_12884); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1433:1: rule__WMLMacroCall__NameAssignment_2 : ( RULE_ID ) ;
    public final void rule__WMLMacroCall__NameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1437:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1438:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1438:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1439:1: RULE_ID
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_2_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_22923); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1448:1: rule__WMLMacroCall__ParamsAssignment_3_0 : ( ruleWMLValue ) ;
    public final void rule__WMLMacroCall__ParamsAssignment_3_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1452:1: ( ( ruleWMLValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1453:1: ( ruleWMLValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1453:1: ( ruleWMLValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1454:1: ruleWMLValue
            {
             before(grammarAccess.getWMLMacroCallAccess().getParamsWMLValueParserRuleCall_3_0_0()); 
            pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLMacroCall__ParamsAssignment_3_02954);
            ruleWMLValue();
            _fsp--;

             after(grammarAccess.getWMLMacroCallAccess().getParamsWMLValueParserRuleCall_3_0_0()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1463:1: rule__WMLMacroCall__ExtraMacrosAssignment_3_1 : ( ruleWMLMacroCall ) ;
    public final void rule__WMLMacroCall__ExtraMacrosAssignment_3_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1467:1: ( ( ruleWMLMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1468:1: ( ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1468:1: ( ruleWMLMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1469:1: ruleWMLMacroCall
            {
             before(grammarAccess.getWMLMacroCallAccess().getExtraMacrosWMLMacroCallParserRuleCall_3_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCall__ExtraMacrosAssignment_3_12985);
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


    // $ANTLR start rule__WMLLuaCode__ValueAssignment
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1479:1: rule__WMLLuaCode__ValueAssignment : ( RULE_LUA_CODE ) ;
    public final void rule__WMLLuaCode__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1483:1: ( ( RULE_LUA_CODE ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1484:1: ( RULE_LUA_CODE )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1484:1: ( RULE_LUA_CODE )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1485:1: RULE_LUA_CODE
            {
             before(grammarAccess.getWMLLuaCodeAccess().getValueLUA_CODETerminalRuleCall_0()); 
            match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment3017); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1494:1: rule__WMLArrayCall__ValueAssignment_1 : ( ruleWMLValue ) ;
    public final void rule__WMLArrayCall__ValueAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1498:1: ( ( ruleWMLValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1499:1: ( ruleWMLValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1499:1: ( ruleWMLValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1500:1: ruleWMLValue
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_13048);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1509:1: rule__WMLMacroDefine__NameAssignment : ( RULE_DEFINE ) ;
    public final void rule__WMLMacroDefine__NameAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1513:1: ( ( RULE_DEFINE ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1514:1: ( RULE_DEFINE )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1514:1: ( RULE_DEFINE )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1515:1: RULE_DEFINE
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0()); 
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment3079); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1524:1: rule__WMLTextdomain__NameAssignment : ( RULE_TEXTDOMAIN ) ;
    public final void rule__WMLTextdomain__NameAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1528:1: ( ( RULE_TEXTDOMAIN ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1529:1: ( RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1529:1: ( RULE_TEXTDOMAIN )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1530:1: RULE_TEXTDOMAIN
            {
             before(grammarAccess.getWMLTextdomainAccess().getNameTEXTDOMAINTerminalRuleCall_0()); 
            match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment3110); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1539:1: rule__WMLValue__ValueAssignment : ( ( rule__WMLValue__ValueAlternatives_0 ) ) ;
    public final void rule__WMLValue__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1543:1: ( ( ( rule__WMLValue__ValueAlternatives_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1544:1: ( ( rule__WMLValue__ValueAlternatives_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1544:1: ( ( rule__WMLValue__ValueAlternatives_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1545:1: ( rule__WMLValue__ValueAlternatives_0 )
            {
             before(grammarAccess.getWMLValueAccess().getValueAlternatives_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1546:1: ( rule__WMLValue__ValueAlternatives_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1546:2: rule__WMLValue__ValueAlternatives_0
            {
            pushFollow(FOLLOW_rule__WMLValue__ValueAlternatives_0_in_rule__WMLValue__ValueAssignment3141);
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
    public static final BitSet FOLLOW_rule__WMLRoot__Alternatives_in_ruleWMLRoot94 = new BitSet(new long[]{0x0000000000044C02L});
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
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode373 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLLuaCode380 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLLuaCode__ValueAssignment_in_ruleWMLLuaCode406 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall433 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLArrayCall440 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__0_in_ruleWMLArrayCall466 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine493 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine500 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__NameAssignment_in_ruleWMLMacroDefine526 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain553 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTextdomain560 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTextdomain__NameAssignment_in_ruleWMLTextdomain586 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_entryRuleWMLValue613 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValue620 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValue__ValueAssignment_in_ruleWMLValue646 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__TagsAssignment_0_in_rule__WMLRoot__Alternatives682 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__MacroCallsAssignment_1_in_rule__WMLRoot__Alternatives700 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__MacroDefinesAssignment_2_in_rule__WMLRoot__Alternatives718 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__TextdomainsAssignment_3_in_rule__WMLRoot__Alternatives736 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TagsAssignment_4_0_in_rule__WMLTag__Alternatives_4769 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__KeysAssignment_4_1_in_rule__WMLTag__Alternatives_4787 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__MacroCallsAssignment_4_2_in_rule__WMLTag__Alternatives_4805 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__MacroDefinesAssignment_4_3_in_rule__WMLTag__Alternatives_4823 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TextdomainsAssignment_4_4_in_rule__WMLTag__Alternatives_4841 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__Alternatives_3874 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_SL_COMMENT_in_rule__WMLKey__Alternatives_3891 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLKeyValue__Alternatives923 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives940 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_rule__WMLKeyValue__Alternatives957 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_rule__WMLKeyValue__Alternatives974 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ParamsAssignment_3_0_in_rule__WMLMacroCall__Alternatives_31006 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ExtraMacrosAssignment_3_1_in_rule__WMLMacroCall__Alternatives_31024 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLValue__ValueAlternatives_01058 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLValue__ValueAlternatives_01075 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__ValueAlternatives_01092 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__WMLValue__ValueAlternatives_01110 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01142 = new BitSet(new long[]{0x0000000000002040L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01145 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__WMLTag__Group__0__Impl1173 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__11204 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__11207 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl1234 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__21265 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__21268 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl1295 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__31325 = new BitSet(new long[]{0x0000000000054C40L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__31328 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLTag__Group__3__Impl1356 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__41387 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__41390 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Alternatives_4_in_rule__WMLTag__Group__4__Impl1417 = new BitSet(new long[]{0x0000000000044C42L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__51448 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__51451 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__WMLTag__Group__5__Impl1479 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__61510 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__61513 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl1540 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__71570 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLTag__Group__7__Impl1598 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__01645 = new BitSet(new long[]{0x0000000000020000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__01648 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl1675 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__11705 = new BitSet(new long[]{0x00000000000463C0L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__11708 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_rule__WMLKey__Group__1__Impl1736 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__21767 = new BitSet(new long[]{0x0000000000000030L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__21770 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl1799 = new BitSet(new long[]{0x00000000000463C2L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl1811 = new BitSet(new long[]{0x00000000000463C2L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__31844 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Alternatives_3_in_rule__WMLKey__Group__3__Impl1871 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__01909 = new BitSet(new long[]{0x0000000000100040L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__01912 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_rule__WMLMacroCall__Group__0__Impl1940 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__11971 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__11974 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__RelativeAssignment_1_in_rule__WMLMacroCall__Group__1__Impl2001 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__22032 = new BitSet(new long[]{0x00000000000C21C0L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__22035 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__NameAssignment_2_in_rule__WMLMacroCall__Group__2__Impl2062 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__32092 = new BitSet(new long[]{0x0000000000080000L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__32095 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Alternatives_3_in_rule__WMLMacroCall__Group__3__Impl2122 = new BitSet(new long[]{0x00000000000421C2L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__42153 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_rule__WMLMacroCall__Group__4__Impl2181 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__02223 = new BitSet(new long[]{0x00000000000021C0L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__02226 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__WMLArrayCall__Group__0__Impl2254 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__12285 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__12288 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl2317 = new BitSet(new long[]{0x00000000000021C2L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl2329 = new BitSet(new long[]{0x00000000000021C2L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__22362 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLArrayCall__Group__2__Impl2390 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRoot__TagsAssignment_02432 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLRoot__MacroCallsAssignment_12463 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLRoot__MacroDefinesAssignment_22494 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_rule__WMLRoot__TextdomainsAssignment_32525 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__WMLTag__PlusAssignment_12561 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_22600 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLTag__TagsAssignment_4_02631 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLTag__KeysAssignment_4_12662 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLTag__MacroCallsAssignment_4_22693 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLTag__MacroDefinesAssignment_4_32724 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_rule__WMLTag__TextdomainsAssignment_4_42755 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_62786 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_02817 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_22848 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__WMLMacroCall__RelativeAssignment_12884 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_22923 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLMacroCall__ParamsAssignment_3_02954 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCall__ExtraMacrosAssignment_3_12985 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment3017 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_13048 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment3079 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment3110 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValue__ValueAlternatives_0_in_rule__WMLValue__ValueAssignment3141 = new BitSet(new long[]{0x0000000000000002L});

}