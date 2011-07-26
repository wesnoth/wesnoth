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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_EOL", "RULE_SL_COMMENT", "RULE_IFDEF", "RULE_IFNDEF", "RULE_IFHAVE", "RULE_IFNHAVE", "RULE_ID", "RULE_STRING", "RULE_ANY_OTHER", "RULE_DEFINE", "RULE_ENDDEF", "RULE_ELSE", "RULE_ENDIF", "RULE_TEXTDOMAIN", "RULE_LUA_CODE", "RULE_WS", "'_'", "'~'", "'.'", "'./'", "'$'", "'/'", "'('", "')'", "'='", "'['", "']'", "'+'", "'[/'", "'{'", "'}'"
    };
    public static final int RULE_LUA_CODE=18;
    public static final int RULE_ID=10;
    public static final int RULE_IFDEF=6;
    public static final int T__29=29;
    public static final int T__28=28;
    public static final int T__27=27;
    public static final int T__26=26;
    public static final int T__25=25;
    public static final int T__24=24;
    public static final int T__23=23;
    public static final int T__22=22;
    public static final int RULE_ANY_OTHER=12;
    public static final int T__21=21;
    public static final int T__20=20;
    public static final int RULE_IFNDEF=7;
    public static final int RULE_EOL=4;
    public static final int RULE_TEXTDOMAIN=17;
    public static final int RULE_IFNHAVE=9;
    public static final int RULE_SL_COMMENT=5;
    public static final int EOF=-1;
    public static final int T__30=30;
    public static final int T__31=31;
    public static final int RULE_STRING=11;
    public static final int T__32=32;
    public static final int T__33=33;
    public static final int RULE_ENDIF=16;
    public static final int T__34=34;
    public static final int RULE_DEFINE=13;
    public static final int RULE_ENDDEF=14;
    public static final int RULE_IFHAVE=8;
    public static final int RULE_WS=19;
    public static final int RULE_ELSE=15;

    // delegates
    // delegators


        public InternalWMLParser(TokenStream input) {
            this(input, new RecognizerSharedState());
        }
        public InternalWMLParser(TokenStream input, RecognizerSharedState state) {
            super(input, state);
             
        }
        

    public String[] getTokenNames() { return InternalWMLParser.tokenNames; }
    public String getGrammarFileName() { return "../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g"; }


     
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




    // $ANTLR start "entryRuleWMLRoot"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:61:1: entryRuleWMLRoot : ruleWMLRoot EOF ;
    public final void entryRuleWMLRoot() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:62:1: ( ruleWMLRoot EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:63:1: ruleWMLRoot EOF
            {
             before(grammarAccess.getWMLRootRule()); 
            pushFollow(FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot61);
            ruleWMLRoot();

            state._fsp--;

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
    // $ANTLR end "entryRuleWMLRoot"


    // $ANTLR start "ruleWMLRoot"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:70:1: ruleWMLRoot : ( ( rule__WMLRoot__ExpressionsAssignment )* ) ;
    public final void ruleWMLRoot() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:74:2: ( ( ( rule__WMLRoot__ExpressionsAssignment )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:75:1: ( ( rule__WMLRoot__ExpressionsAssignment )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:75:1: ( ( rule__WMLRoot__ExpressionsAssignment )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:76:1: ( rule__WMLRoot__ExpressionsAssignment )*
            {
             before(grammarAccess.getWMLRootAccess().getExpressionsAssignment()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:77:1: ( rule__WMLRoot__ExpressionsAssignment )*
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( ((LA1_0>=RULE_IFDEF && LA1_0<=RULE_IFNHAVE)||LA1_0==RULE_DEFINE||LA1_0==RULE_TEXTDOMAIN||LA1_0==29||LA1_0==33) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:77:2: rule__WMLRoot__ExpressionsAssignment
            	    {
            	    pushFollow(FOLLOW_rule__WMLRoot__ExpressionsAssignment_in_ruleWMLRoot94);
            	    rule__WMLRoot__ExpressionsAssignment();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    break loop1;
                }
            } while (true);

             after(grammarAccess.getWMLRootAccess().getExpressionsAssignment()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "ruleWMLRoot"


    // $ANTLR start "entryRuleWMLTag"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:89:1: entryRuleWMLTag : ruleWMLTag EOF ;
    public final void entryRuleWMLTag() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:90:1: ( ruleWMLTag EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:91:1: ruleWMLTag EOF
            {
             before(grammarAccess.getWMLTagRule()); 
            pushFollow(FOLLOW_ruleWMLTag_in_entryRuleWMLTag122);
            ruleWMLTag();

            state._fsp--;

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
    // $ANTLR end "entryRuleWMLTag"


    // $ANTLR start "ruleWMLTag"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:98:1: ruleWMLTag : ( ( rule__WMLTag__Group__0 ) ) ;
    public final void ruleWMLTag() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:102:2: ( ( ( rule__WMLTag__Group__0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:103:1: ( ( rule__WMLTag__Group__0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:103:1: ( ( rule__WMLTag__Group__0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:104:1: ( rule__WMLTag__Group__0 )
            {
             before(grammarAccess.getWMLTagAccess().getGroup()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:105:1: ( rule__WMLTag__Group__0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:105:2: rule__WMLTag__Group__0
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0_in_ruleWMLTag155);
            rule__WMLTag__Group__0();

            state._fsp--;


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
    // $ANTLR end "ruleWMLTag"


    // $ANTLR start "entryRuleWMLKey"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:117:1: entryRuleWMLKey : ruleWMLKey EOF ;
    public final void entryRuleWMLKey() throws RecognitionException {

        	HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");

        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:121:1: ( ruleWMLKey EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:122:1: ruleWMLKey EOF
            {
             before(grammarAccess.getWMLKeyRule()); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey187);
            ruleWMLKey();

            state._fsp--;

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
    // $ANTLR end "entryRuleWMLKey"


    // $ANTLR start "ruleWMLKey"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:132:1: ruleWMLKey : ( ( rule__WMLKey__Group__0 ) ) ;
    public final void ruleWMLKey() throws RecognitionException {

        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:137:2: ( ( ( rule__WMLKey__Group__0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:138:1: ( ( rule__WMLKey__Group__0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:138:1: ( ( rule__WMLKey__Group__0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:139:1: ( rule__WMLKey__Group__0 )
            {
             before(grammarAccess.getWMLKeyAccess().getGroup()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:140:1: ( rule__WMLKey__Group__0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:140:2: rule__WMLKey__Group__0
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey224);
            rule__WMLKey__Group__0();

            state._fsp--;


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
    // $ANTLR end "ruleWMLKey"


    // $ANTLR start "entryRuleWMLKeyValue"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:153:1: entryRuleWMLKeyValue : ruleWMLKeyValue EOF ;
    public final void entryRuleWMLKeyValue() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:154:1: ( ruleWMLKeyValue EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:155:1: ruleWMLKeyValue EOF
            {
             before(grammarAccess.getWMLKeyValueRule()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue251);
            ruleWMLKeyValue();

            state._fsp--;

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
    // $ANTLR end "entryRuleWMLKeyValue"


    // $ANTLR start "ruleWMLKeyValue"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:162:1: ruleWMLKeyValue : ( ( rule__WMLKeyValue__Alternatives ) ) ;
    public final void ruleWMLKeyValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:166:2: ( ( ( rule__WMLKeyValue__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:167:1: ( ( rule__WMLKeyValue__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:167:1: ( ( rule__WMLKeyValue__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:168:1: ( rule__WMLKeyValue__Alternatives )
            {
             before(grammarAccess.getWMLKeyValueAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:169:1: ( rule__WMLKeyValue__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:169:2: rule__WMLKeyValue__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue284);
            rule__WMLKeyValue__Alternatives();

            state._fsp--;


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
    // $ANTLR end "ruleWMLKeyValue"


    // $ANTLR start "entryRuleWMLMacroCall"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:181:1: entryRuleWMLMacroCall : ruleWMLMacroCall EOF ;
    public final void entryRuleWMLMacroCall() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:182:1: ( ruleWMLMacroCall EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:183:1: ruleWMLMacroCall EOF
            {
             before(grammarAccess.getWMLMacroCallRule()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall311);
            ruleWMLMacroCall();

            state._fsp--;

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
    // $ANTLR end "entryRuleWMLMacroCall"


    // $ANTLR start "ruleWMLMacroCall"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:190:1: ruleWMLMacroCall : ( ( rule__WMLMacroCall__Group__0 ) ) ;
    public final void ruleWMLMacroCall() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:194:2: ( ( ( rule__WMLMacroCall__Group__0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:195:1: ( ( rule__WMLMacroCall__Group__0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:195:1: ( ( rule__WMLMacroCall__Group__0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:196:1: ( rule__WMLMacroCall__Group__0 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getGroup()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:197:1: ( rule__WMLMacroCall__Group__0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:197:2: rule__WMLMacroCall__Group__0
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__0_in_ruleWMLMacroCall344);
            rule__WMLMacroCall__Group__0();

            state._fsp--;


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
    // $ANTLR end "ruleWMLMacroCall"


    // $ANTLR start "entryRuleWMLMacroCallParameter"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:209:1: entryRuleWMLMacroCallParameter : ruleWMLMacroCallParameter EOF ;
    public final void entryRuleWMLMacroCallParameter() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:210:1: ( ruleWMLMacroCallParameter EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:211:1: ruleWMLMacroCallParameter EOF
            {
             before(grammarAccess.getWMLMacroCallParameterRule()); 
            pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_entryRuleWMLMacroCallParameter371);
            ruleWMLMacroCallParameter();

            state._fsp--;

             after(grammarAccess.getWMLMacroCallParameterRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCallParameter378); 

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
    // $ANTLR end "entryRuleWMLMacroCallParameter"


    // $ANTLR start "ruleWMLMacroCallParameter"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:218:1: ruleWMLMacroCallParameter : ( ( rule__WMLMacroCallParameter__Alternatives ) ) ;
    public final void ruleWMLMacroCallParameter() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:222:2: ( ( ( rule__WMLMacroCallParameter__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:223:1: ( ( rule__WMLMacroCallParameter__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:223:1: ( ( rule__WMLMacroCallParameter__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:224:1: ( rule__WMLMacroCallParameter__Alternatives )
            {
             before(grammarAccess.getWMLMacroCallParameterAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:225:1: ( rule__WMLMacroCallParameter__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:225:2: rule__WMLMacroCallParameter__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLMacroCallParameter__Alternatives_in_ruleWMLMacroCallParameter404);
            rule__WMLMacroCallParameter__Alternatives();

            state._fsp--;


            }

             after(grammarAccess.getWMLMacroCallParameterAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "ruleWMLMacroCallParameter"


    // $ANTLR start "entryRuleWMLArrayCall"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:237:1: entryRuleWMLArrayCall : ruleWMLArrayCall EOF ;
    public final void entryRuleWMLArrayCall() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:238:1: ( ruleWMLArrayCall EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:239:1: ruleWMLArrayCall EOF
            {
             before(grammarAccess.getWMLArrayCallRule()); 
            pushFollow(FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall431);
            ruleWMLArrayCall();

            state._fsp--;

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
    // $ANTLR end "entryRuleWMLArrayCall"


    // $ANTLR start "ruleWMLArrayCall"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:246:1: ruleWMLArrayCall : ( ( rule__WMLArrayCall__Group__0 ) ) ;
    public final void ruleWMLArrayCall() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:250:2: ( ( ( rule__WMLArrayCall__Group__0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:251:1: ( ( rule__WMLArrayCall__Group__0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:251:1: ( ( rule__WMLArrayCall__Group__0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:252:1: ( rule__WMLArrayCall__Group__0 )
            {
             before(grammarAccess.getWMLArrayCallAccess().getGroup()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:253:1: ( rule__WMLArrayCall__Group__0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:253:2: rule__WMLArrayCall__Group__0
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__0_in_ruleWMLArrayCall464);
            rule__WMLArrayCall__Group__0();

            state._fsp--;


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
    // $ANTLR end "ruleWMLArrayCall"


    // $ANTLR start "entryRuleWMLMacroDefine"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:265:1: entryRuleWMLMacroDefine : ruleWMLMacroDefine EOF ;
    public final void entryRuleWMLMacroDefine() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:266:1: ( ruleWMLMacroDefine EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:267:1: ruleWMLMacroDefine EOF
            {
             before(grammarAccess.getWMLMacroDefineRule()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine491);
            ruleWMLMacroDefine();

            state._fsp--;

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
    // $ANTLR end "entryRuleWMLMacroDefine"


    // $ANTLR start "ruleWMLMacroDefine"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:274:1: ruleWMLMacroDefine : ( ( rule__WMLMacroDefine__Group__0 ) ) ;
    public final void ruleWMLMacroDefine() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:278:2: ( ( ( rule__WMLMacroDefine__Group__0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:279:1: ( ( rule__WMLMacroDefine__Group__0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:279:1: ( ( rule__WMLMacroDefine__Group__0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:280:1: ( rule__WMLMacroDefine__Group__0 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getGroup()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:1: ( rule__WMLMacroDefine__Group__0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:2: rule__WMLMacroDefine__Group__0
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__0_in_ruleWMLMacroDefine524);
            rule__WMLMacroDefine__Group__0();

            state._fsp--;


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
    // $ANTLR end "ruleWMLMacroDefine"


    // $ANTLR start "entryRuleWMLPreprocIF"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:293:1: entryRuleWMLPreprocIF : ruleWMLPreprocIF EOF ;
    public final void entryRuleWMLPreprocIF() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:294:1: ( ruleWMLPreprocIF EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:295:1: ruleWMLPreprocIF EOF
            {
             before(grammarAccess.getWMLPreprocIFRule()); 
            pushFollow(FOLLOW_ruleWMLPreprocIF_in_entryRuleWMLPreprocIF551);
            ruleWMLPreprocIF();

            state._fsp--;

             after(grammarAccess.getWMLPreprocIFRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLPreprocIF558); 

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
    // $ANTLR end "entryRuleWMLPreprocIF"


    // $ANTLR start "ruleWMLPreprocIF"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:302:1: ruleWMLPreprocIF : ( ( rule__WMLPreprocIF__Group__0 ) ) ;
    public final void ruleWMLPreprocIF() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:306:2: ( ( ( rule__WMLPreprocIF__Group__0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:307:1: ( ( rule__WMLPreprocIF__Group__0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:307:1: ( ( rule__WMLPreprocIF__Group__0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:308:1: ( rule__WMLPreprocIF__Group__0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getGroup()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:1: ( rule__WMLPreprocIF__Group__0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:2: rule__WMLPreprocIF__Group__0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__0_in_ruleWMLPreprocIF584);
            rule__WMLPreprocIF__Group__0();

            state._fsp--;


            }

             after(grammarAccess.getWMLPreprocIFAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "ruleWMLPreprocIF"


    // $ANTLR start "entryRuleWMLRootExpression"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:321:1: entryRuleWMLRootExpression : ruleWMLRootExpression EOF ;
    public final void entryRuleWMLRootExpression() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:322:1: ( ruleWMLRootExpression EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:323:1: ruleWMLRootExpression EOF
            {
             before(grammarAccess.getWMLRootExpressionRule()); 
            pushFollow(FOLLOW_ruleWMLRootExpression_in_entryRuleWMLRootExpression611);
            ruleWMLRootExpression();

            state._fsp--;

             after(grammarAccess.getWMLRootExpressionRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLRootExpression618); 

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
    // $ANTLR end "entryRuleWMLRootExpression"


    // $ANTLR start "ruleWMLRootExpression"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:330:1: ruleWMLRootExpression : ( ( rule__WMLRootExpression__Alternatives ) ) ;
    public final void ruleWMLRootExpression() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:334:2: ( ( ( rule__WMLRootExpression__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:335:1: ( ( rule__WMLRootExpression__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:335:1: ( ( rule__WMLRootExpression__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:336:1: ( rule__WMLRootExpression__Alternatives )
            {
             before(grammarAccess.getWMLRootExpressionAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:1: ( rule__WMLRootExpression__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:2: rule__WMLRootExpression__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLRootExpression__Alternatives_in_ruleWMLRootExpression644);
            rule__WMLRootExpression__Alternatives();

            state._fsp--;


            }

             after(grammarAccess.getWMLRootExpressionAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "ruleWMLRootExpression"


    // $ANTLR start "entryRuleWMLExpression"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:349:1: entryRuleWMLExpression : ruleWMLExpression EOF ;
    public final void entryRuleWMLExpression() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:350:1: ( ruleWMLExpression EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:351:1: ruleWMLExpression EOF
            {
             before(grammarAccess.getWMLExpressionRule()); 
            pushFollow(FOLLOW_ruleWMLExpression_in_entryRuleWMLExpression671);
            ruleWMLExpression();

            state._fsp--;

             after(grammarAccess.getWMLExpressionRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLExpression678); 

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
    // $ANTLR end "entryRuleWMLExpression"


    // $ANTLR start "ruleWMLExpression"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:358:1: ruleWMLExpression : ( ( rule__WMLExpression__Alternatives ) ) ;
    public final void ruleWMLExpression() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:362:2: ( ( ( rule__WMLExpression__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:363:1: ( ( rule__WMLExpression__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:363:1: ( ( rule__WMLExpression__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:364:1: ( rule__WMLExpression__Alternatives )
            {
             before(grammarAccess.getWMLExpressionAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:365:1: ( rule__WMLExpression__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:365:2: rule__WMLExpression__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLExpression__Alternatives_in_ruleWMLExpression704);
            rule__WMLExpression__Alternatives();

            state._fsp--;


            }

             after(grammarAccess.getWMLExpressionAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "ruleWMLExpression"


    // $ANTLR start "entryRuleWMLValuedExpression"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:377:1: entryRuleWMLValuedExpression : ruleWMLValuedExpression EOF ;
    public final void entryRuleWMLValuedExpression() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:378:1: ( ruleWMLValuedExpression EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:379:1: ruleWMLValuedExpression EOF
            {
             before(grammarAccess.getWMLValuedExpressionRule()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_entryRuleWMLValuedExpression731);
            ruleWMLValuedExpression();

            state._fsp--;

             after(grammarAccess.getWMLValuedExpressionRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValuedExpression738); 

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
    // $ANTLR end "entryRuleWMLValuedExpression"


    // $ANTLR start "ruleWMLValuedExpression"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:386:1: ruleWMLValuedExpression : ( ( rule__WMLValuedExpression__Alternatives ) ) ;
    public final void ruleWMLValuedExpression() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:390:2: ( ( ( rule__WMLValuedExpression__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:391:1: ( ( rule__WMLValuedExpression__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:391:1: ( ( rule__WMLValuedExpression__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:392:1: ( rule__WMLValuedExpression__Alternatives )
            {
             before(grammarAccess.getWMLValuedExpressionAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:393:1: ( rule__WMLValuedExpression__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:393:2: rule__WMLValuedExpression__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLValuedExpression__Alternatives_in_ruleWMLValuedExpression764);
            rule__WMLValuedExpression__Alternatives();

            state._fsp--;


            }

             after(grammarAccess.getWMLValuedExpressionAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "ruleWMLValuedExpression"


    // $ANTLR start "entryRuleWMLTextdomain"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:405:1: entryRuleWMLTextdomain : ruleWMLTextdomain EOF ;
    public final void entryRuleWMLTextdomain() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:406:1: ( ruleWMLTextdomain EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:407:1: ruleWMLTextdomain EOF
            {
             before(grammarAccess.getWMLTextdomainRule()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain791);
            ruleWMLTextdomain();

            state._fsp--;

             after(grammarAccess.getWMLTextdomainRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTextdomain798); 

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
    // $ANTLR end "entryRuleWMLTextdomain"


    // $ANTLR start "ruleWMLTextdomain"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:414:1: ruleWMLTextdomain : ( ( rule__WMLTextdomain__NameAssignment ) ) ;
    public final void ruleWMLTextdomain() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:418:2: ( ( ( rule__WMLTextdomain__NameAssignment ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:419:1: ( ( rule__WMLTextdomain__NameAssignment ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:419:1: ( ( rule__WMLTextdomain__NameAssignment ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:420:1: ( rule__WMLTextdomain__NameAssignment )
            {
             before(grammarAccess.getWMLTextdomainAccess().getNameAssignment()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:421:1: ( rule__WMLTextdomain__NameAssignment )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:421:2: rule__WMLTextdomain__NameAssignment
            {
            pushFollow(FOLLOW_rule__WMLTextdomain__NameAssignment_in_ruleWMLTextdomain824);
            rule__WMLTextdomain__NameAssignment();

            state._fsp--;


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
    // $ANTLR end "ruleWMLTextdomain"


    // $ANTLR start "entryRuleWMLLuaCode"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:433:1: entryRuleWMLLuaCode : ruleWMLLuaCode EOF ;
    public final void entryRuleWMLLuaCode() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:434:1: ( ruleWMLLuaCode EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:435:1: ruleWMLLuaCode EOF
            {
             before(grammarAccess.getWMLLuaCodeRule()); 
            pushFollow(FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode851);
            ruleWMLLuaCode();

            state._fsp--;

             after(grammarAccess.getWMLLuaCodeRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLLuaCode858); 

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
    // $ANTLR end "entryRuleWMLLuaCode"


    // $ANTLR start "ruleWMLLuaCode"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:442:1: ruleWMLLuaCode : ( ( rule__WMLLuaCode__ValueAssignment ) ) ;
    public final void ruleWMLLuaCode() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:446:2: ( ( ( rule__WMLLuaCode__ValueAssignment ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:447:1: ( ( rule__WMLLuaCode__ValueAssignment ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:447:1: ( ( rule__WMLLuaCode__ValueAssignment ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:448:1: ( rule__WMLLuaCode__ValueAssignment )
            {
             before(grammarAccess.getWMLLuaCodeAccess().getValueAssignment()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:449:1: ( rule__WMLLuaCode__ValueAssignment )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:449:2: rule__WMLLuaCode__ValueAssignment
            {
            pushFollow(FOLLOW_rule__WMLLuaCode__ValueAssignment_in_ruleWMLLuaCode884);
            rule__WMLLuaCode__ValueAssignment();

            state._fsp--;


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
    // $ANTLR end "ruleWMLLuaCode"


    // $ANTLR start "entryRuleWMLMacroParameter"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:461:1: entryRuleWMLMacroParameter : ruleWMLMacroParameter EOF ;
    public final void entryRuleWMLMacroParameter() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:462:1: ( ruleWMLMacroParameter EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:463:1: ruleWMLMacroParameter EOF
            {
             before(grammarAccess.getWMLMacroParameterRule()); 
            pushFollow(FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter911);
            ruleWMLMacroParameter();

            state._fsp--;

             after(grammarAccess.getWMLMacroParameterRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroParameter918); 

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
    // $ANTLR end "entryRuleWMLMacroParameter"


    // $ANTLR start "ruleWMLMacroParameter"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:470:1: ruleWMLMacroParameter : ( ( rule__WMLMacroParameter__Alternatives ) ) ;
    public final void ruleWMLMacroParameter() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:474:2: ( ( ( rule__WMLMacroParameter__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:475:1: ( ( rule__WMLMacroParameter__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:475:1: ( ( rule__WMLMacroParameter__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:476:1: ( rule__WMLMacroParameter__Alternatives )
            {
             before(grammarAccess.getWMLMacroParameterAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:477:1: ( rule__WMLMacroParameter__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:477:2: rule__WMLMacroParameter__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLMacroParameter__Alternatives_in_ruleWMLMacroParameter944);
            rule__WMLMacroParameter__Alternatives();

            state._fsp--;


            }

             after(grammarAccess.getWMLMacroParameterAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "ruleWMLMacroParameter"


    // $ANTLR start "entryRuleWMLValue"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:489:1: entryRuleWMLValue : ruleWMLValue EOF ;
    public final void entryRuleWMLValue() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:490:1: ( ruleWMLValue EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:491:1: ruleWMLValue EOF
            {
             before(grammarAccess.getWMLValueRule()); 
            pushFollow(FOLLOW_ruleWMLValue_in_entryRuleWMLValue971);
            ruleWMLValue();

            state._fsp--;

             after(grammarAccess.getWMLValueRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValue978); 

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
    // $ANTLR end "entryRuleWMLValue"


    // $ANTLR start "ruleWMLValue"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:498:1: ruleWMLValue : ( ( rule__WMLValue__ValueAssignment ) ) ;
    public final void ruleWMLValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:502:2: ( ( ( rule__WMLValue__ValueAssignment ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:503:1: ( ( rule__WMLValue__ValueAssignment ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:503:1: ( ( rule__WMLValue__ValueAssignment ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:504:1: ( rule__WMLValue__ValueAssignment )
            {
             before(grammarAccess.getWMLValueAccess().getValueAssignment()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:505:1: ( rule__WMLValue__ValueAssignment )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:505:2: rule__WMLValue__ValueAssignment
            {
            pushFollow(FOLLOW_rule__WMLValue__ValueAssignment_in_ruleWMLValue1004);
            rule__WMLValue__ValueAssignment();

            state._fsp--;


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
    // $ANTLR end "ruleWMLValue"


    // $ANTLR start "entryRuleMacroTokens"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:517:1: entryRuleMacroTokens : ruleMacroTokens EOF ;
    public final void entryRuleMacroTokens() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:518:1: ( ruleMacroTokens EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:519:1: ruleMacroTokens EOF
            {
             before(grammarAccess.getMacroTokensRule()); 
            pushFollow(FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens1031);
            ruleMacroTokens();

            state._fsp--;

             after(grammarAccess.getMacroTokensRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleMacroTokens1038); 

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
    // $ANTLR end "entryRuleMacroTokens"


    // $ANTLR start "ruleMacroTokens"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:526:1: ruleMacroTokens : ( ( rule__MacroTokens__ValueAssignment ) ) ;
    public final void ruleMacroTokens() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:530:2: ( ( ( rule__MacroTokens__ValueAssignment ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:531:1: ( ( rule__MacroTokens__ValueAssignment ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:531:1: ( ( rule__MacroTokens__ValueAssignment ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:532:1: ( rule__MacroTokens__ValueAssignment )
            {
             before(grammarAccess.getMacroTokensAccess().getValueAssignment()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:1: ( rule__MacroTokens__ValueAssignment )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:2: rule__MacroTokens__ValueAssignment
            {
            pushFollow(FOLLOW_rule__MacroTokens__ValueAssignment_in_ruleMacroTokens1064);
            rule__MacroTokens__ValueAssignment();

            state._fsp--;


            }

             after(grammarAccess.getMacroTokensAccess().getValueAssignment()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "ruleMacroTokens"


    // $ANTLR start "rule__WMLKey__EolAlternatives_4_0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:545:1: rule__WMLKey__EolAlternatives_4_0 : ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) );
    public final void rule__WMLKey__EolAlternatives_4_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:549:1: ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) )
            int alt2=2;
            int LA2_0 = input.LA(1);

            if ( (LA2_0==RULE_EOL) ) {
                alt2=1;
            }
            else if ( (LA2_0==RULE_SL_COMMENT) ) {
                alt2=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("", 2, 0, input);

                throw nvae;
            }
            switch (alt2) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:550:1: ( RULE_EOL )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:550:1: ( RULE_EOL )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:551:1: RULE_EOL
                    {
                     before(grammarAccess.getWMLKeyAccess().getEolEOLTerminalRuleCall_4_0_0()); 
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__EolAlternatives_4_01100); 
                     after(grammarAccess.getWMLKeyAccess().getEolEOLTerminalRuleCall_4_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:556:6: ( RULE_SL_COMMENT )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:556:6: ( RULE_SL_COMMENT )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:557:1: RULE_SL_COMMENT
                    {
                     before(grammarAccess.getWMLKeyAccess().getEolSL_COMMENTTerminalRuleCall_4_0_1()); 
                    match(input,RULE_SL_COMMENT,FOLLOW_RULE_SL_COMMENT_in_rule__WMLKey__EolAlternatives_4_01117); 
                     after(grammarAccess.getWMLKeyAccess().getEolSL_COMMENTTerminalRuleCall_4_0_1()); 

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
    // $ANTLR end "rule__WMLKey__EolAlternatives_4_0"


    // $ANTLR start "rule__WMLKeyValue__Alternatives"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:567:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) );
    public final void rule__WMLKeyValue__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:571:1: ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) )
            int alt3=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
            case RULE_STRING:
            case RULE_ANY_OTHER:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
                {
                alt3=1;
                }
                break;
            case 33:
                {
                alt3=2;
                }
                break;
            case RULE_LUA_CODE:
                {
                alt3=3;
                }
                break;
            case 29:
                {
                alt3=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("", 3, 0, input);

                throw nvae;
            }

            switch (alt3) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:572:1: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:572:1: ( ruleWMLValue )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:573:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLKeyValue__Alternatives1149);
                    ruleWMLValue();

                    state._fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:578:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:578:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:579:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives1166);
                    ruleWMLMacroCall();

                    state._fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:584:6: ( ruleWMLLuaCode )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:584:6: ( ruleWMLLuaCode )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:585:1: ruleWMLLuaCode
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2()); 
                    pushFollow(FOLLOW_ruleWMLLuaCode_in_rule__WMLKeyValue__Alternatives1183);
                    ruleWMLLuaCode();

                    state._fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:590:6: ( ruleWMLArrayCall )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:590:6: ( ruleWMLArrayCall )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:591:1: ruleWMLArrayCall
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLArrayCallParserRuleCall_3()); 
                    pushFollow(FOLLOW_ruleWMLArrayCall_in_rule__WMLKeyValue__Alternatives1200);
                    ruleWMLArrayCall();

                    state._fsp--;

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
    // $ANTLR end "rule__WMLKeyValue__Alternatives"


    // $ANTLR start "rule__WMLMacroCallParameter__Alternatives"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:601:1: rule__WMLMacroCallParameter__Alternatives : ( ( ruleWMLMacroParameter ) | ( ruleWMLMacroCall ) );
    public final void rule__WMLMacroCallParameter__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:605:1: ( ( ruleWMLMacroParameter ) | ( ruleWMLMacroCall ) )
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( ((LA4_0>=RULE_ID && LA4_0<=RULE_ANY_OTHER)||(LA4_0>=20 && LA4_0<=32)) ) {
                alt4=1;
            }
            else if ( (LA4_0==33) ) {
                alt4=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("", 4, 0, input);

                throw nvae;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:606:1: ( ruleWMLMacroParameter )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:606:1: ( ruleWMLMacroParameter )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:607:1: ruleWMLMacroParameter
                    {
                     before(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroParameterParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLMacroParameter_in_rule__WMLMacroCallParameter__Alternatives1232);
                    ruleWMLMacroParameter();

                    state._fsp--;

                     after(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroParameterParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:612:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:612:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:613:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroCallParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCallParameter__Alternatives1249);
                    ruleWMLMacroCall();

                    state._fsp--;

                     after(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroCallParserRuleCall_1()); 

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
    // $ANTLR end "rule__WMLMacroCallParameter__Alternatives"


    // $ANTLR start "rule__WMLPreprocIF__NameAlternatives_0_0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:623:1: rule__WMLPreprocIF__NameAlternatives_0_0 : ( ( RULE_IFDEF ) | ( RULE_IFNDEF ) | ( RULE_IFHAVE ) | ( RULE_IFNHAVE ) );
    public final void rule__WMLPreprocIF__NameAlternatives_0_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:627:1: ( ( RULE_IFDEF ) | ( RULE_IFNDEF ) | ( RULE_IFHAVE ) | ( RULE_IFNHAVE ) )
            int alt5=4;
            switch ( input.LA(1) ) {
            case RULE_IFDEF:
                {
                alt5=1;
                }
                break;
            case RULE_IFNDEF:
                {
                alt5=2;
                }
                break;
            case RULE_IFHAVE:
                {
                alt5=3;
                }
                break;
            case RULE_IFNHAVE:
                {
                alt5=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("", 5, 0, input);

                throw nvae;
            }

            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:628:1: ( RULE_IFDEF )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:628:1: ( RULE_IFDEF )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:629:1: RULE_IFDEF
                    {
                     before(grammarAccess.getWMLPreprocIFAccess().getNameIFDEFTerminalRuleCall_0_0_0()); 
                    match(input,RULE_IFDEF,FOLLOW_RULE_IFDEF_in_rule__WMLPreprocIF__NameAlternatives_0_01281); 
                     after(grammarAccess.getWMLPreprocIFAccess().getNameIFDEFTerminalRuleCall_0_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:634:6: ( RULE_IFNDEF )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:634:6: ( RULE_IFNDEF )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:635:1: RULE_IFNDEF
                    {
                     before(grammarAccess.getWMLPreprocIFAccess().getNameIFNDEFTerminalRuleCall_0_0_1()); 
                    match(input,RULE_IFNDEF,FOLLOW_RULE_IFNDEF_in_rule__WMLPreprocIF__NameAlternatives_0_01298); 
                     after(grammarAccess.getWMLPreprocIFAccess().getNameIFNDEFTerminalRuleCall_0_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:640:6: ( RULE_IFHAVE )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:640:6: ( RULE_IFHAVE )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:641:1: RULE_IFHAVE
                    {
                     before(grammarAccess.getWMLPreprocIFAccess().getNameIFHAVETerminalRuleCall_0_0_2()); 
                    match(input,RULE_IFHAVE,FOLLOW_RULE_IFHAVE_in_rule__WMLPreprocIF__NameAlternatives_0_01315); 
                     after(grammarAccess.getWMLPreprocIFAccess().getNameIFHAVETerminalRuleCall_0_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:646:6: ( RULE_IFNHAVE )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:646:6: ( RULE_IFNHAVE )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:647:1: RULE_IFNHAVE
                    {
                     before(grammarAccess.getWMLPreprocIFAccess().getNameIFNHAVETerminalRuleCall_0_0_3()); 
                    match(input,RULE_IFNHAVE,FOLLOW_RULE_IFNHAVE_in_rule__WMLPreprocIF__NameAlternatives_0_01332); 
                     after(grammarAccess.getWMLPreprocIFAccess().getNameIFNHAVETerminalRuleCall_0_0_3()); 

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
    // $ANTLR end "rule__WMLPreprocIF__NameAlternatives_0_0"


    // $ANTLR start "rule__WMLRootExpression__Alternatives"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:657:1: rule__WMLRootExpression__Alternatives : ( ( ruleWMLTag ) | ( ruleWMLMacroCall ) | ( ruleWMLMacroDefine ) | ( ruleWMLTextdomain ) | ( ruleWMLPreprocIF ) );
    public final void rule__WMLRootExpression__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:661:1: ( ( ruleWMLTag ) | ( ruleWMLMacroCall ) | ( ruleWMLMacroDefine ) | ( ruleWMLTextdomain ) | ( ruleWMLPreprocIF ) )
            int alt6=5;
            switch ( input.LA(1) ) {
            case 29:
                {
                alt6=1;
                }
                break;
            case 33:
                {
                alt6=2;
                }
                break;
            case RULE_DEFINE:
                {
                alt6=3;
                }
                break;
            case RULE_TEXTDOMAIN:
                {
                alt6=4;
                }
                break;
            case RULE_IFDEF:
            case RULE_IFNDEF:
            case RULE_IFHAVE:
            case RULE_IFNHAVE:
                {
                alt6=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("", 6, 0, input);

                throw nvae;
            }

            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:662:1: ( ruleWMLTag )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:662:1: ( ruleWMLTag )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:663:1: ruleWMLTag
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLTagParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRootExpression__Alternatives1364);
                    ruleWMLTag();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLTagParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:668:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:668:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:669:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLMacroCallParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLRootExpression__Alternatives1381);
                    ruleWMLMacroCall();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLMacroCallParserRuleCall_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:674:6: ( ruleWMLMacroDefine )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:674:6: ( ruleWMLMacroDefine )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:675:1: ruleWMLMacroDefine
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLMacroDefineParserRuleCall_2()); 
                    pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLRootExpression__Alternatives1398);
                    ruleWMLMacroDefine();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLMacroDefineParserRuleCall_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:680:6: ( ruleWMLTextdomain )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:680:6: ( ruleWMLTextdomain )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:681:1: ruleWMLTextdomain
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLTextdomainParserRuleCall_3()); 
                    pushFollow(FOLLOW_ruleWMLTextdomain_in_rule__WMLRootExpression__Alternatives1415);
                    ruleWMLTextdomain();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLTextdomainParserRuleCall_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:686:6: ( ruleWMLPreprocIF )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:686:6: ( ruleWMLPreprocIF )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:687:1: ruleWMLPreprocIF
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLPreprocIFParserRuleCall_4()); 
                    pushFollow(FOLLOW_ruleWMLPreprocIF_in_rule__WMLRootExpression__Alternatives1432);
                    ruleWMLPreprocIF();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLPreprocIFParserRuleCall_4()); 

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
    // $ANTLR end "rule__WMLRootExpression__Alternatives"


    // $ANTLR start "rule__WMLExpression__Alternatives"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:697:1: rule__WMLExpression__Alternatives : ( ( ruleWMLRootExpression ) | ( ruleWMLKey ) );
    public final void rule__WMLExpression__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:701:1: ( ( ruleWMLRootExpression ) | ( ruleWMLKey ) )
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( ((LA7_0>=RULE_IFDEF && LA7_0<=RULE_IFNHAVE)||LA7_0==RULE_DEFINE||LA7_0==RULE_TEXTDOMAIN||LA7_0==29||LA7_0==33) ) {
                alt7=1;
            }
            else if ( (LA7_0==RULE_ID) ) {
                alt7=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("", 7, 0, input);

                throw nvae;
            }
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:702:1: ( ruleWMLRootExpression )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:702:1: ( ruleWMLRootExpression )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:703:1: ruleWMLRootExpression
                    {
                     before(grammarAccess.getWMLExpressionAccess().getWMLRootExpressionParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLRootExpression_in_rule__WMLExpression__Alternatives1464);
                    ruleWMLRootExpression();

                    state._fsp--;

                     after(grammarAccess.getWMLExpressionAccess().getWMLRootExpressionParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:708:6: ( ruleWMLKey )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:708:6: ( ruleWMLKey )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:709:1: ruleWMLKey
                    {
                     before(grammarAccess.getWMLExpressionAccess().getWMLKeyParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLExpression__Alternatives1481);
                    ruleWMLKey();

                    state._fsp--;

                     after(grammarAccess.getWMLExpressionAccess().getWMLKeyParserRuleCall_1()); 

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
    // $ANTLR end "rule__WMLExpression__Alternatives"


    // $ANTLR start "rule__WMLValuedExpression__Alternatives"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:719:1: rule__WMLValuedExpression__Alternatives : ( ( ruleWMLExpression ) | ( ruleWMLValue ) );
    public final void rule__WMLValuedExpression__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:723:1: ( ( ruleWMLExpression ) | ( ruleWMLValue ) )
            int alt8=2;
            switch ( input.LA(1) ) {
            case RULE_IFDEF:
            case RULE_IFNDEF:
            case RULE_IFHAVE:
            case RULE_IFNHAVE:
            case RULE_DEFINE:
            case RULE_TEXTDOMAIN:
            case 29:
            case 33:
                {
                alt8=1;
                }
                break;
            case RULE_ID:
                {
                int LA8_2 = input.LA(2);

                if ( (LA8_2==EOF||(LA8_2>=RULE_IFDEF && LA8_2<=RULE_TEXTDOMAIN)||(LA8_2>=20 && LA8_2<=27)||LA8_2==29||LA8_2==33) ) {
                    alt8=2;
                }
                else if ( (LA8_2==28) ) {
                    alt8=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("", 8, 2, input);

                    throw nvae;
                }
                }
                break;
            case RULE_STRING:
            case RULE_ANY_OTHER:
            case 20:
            case 21:
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
                {
                alt8=2;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("", 8, 0, input);

                throw nvae;
            }

            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:724:1: ( ruleWMLExpression )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:724:1: ( ruleWMLExpression )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:725:1: ruleWMLExpression
                    {
                     before(grammarAccess.getWMLValuedExpressionAccess().getWMLExpressionParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLExpression_in_rule__WMLValuedExpression__Alternatives1513);
                    ruleWMLExpression();

                    state._fsp--;

                     after(grammarAccess.getWMLValuedExpressionAccess().getWMLExpressionParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:730:6: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:730:6: ( ruleWMLValue )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:731:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLValuedExpressionAccess().getWMLValueParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLValuedExpression__Alternatives1530);
                    ruleWMLValue();

                    state._fsp--;

                     after(grammarAccess.getWMLValuedExpressionAccess().getWMLValueParserRuleCall_1()); 

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
    // $ANTLR end "rule__WMLValuedExpression__Alternatives"


    // $ANTLR start "rule__WMLMacroParameter__Alternatives"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:741:1: rule__WMLMacroParameter__Alternatives : ( ( ruleWMLValue ) | ( ruleMacroTokens ) );
    public final void rule__WMLMacroParameter__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:745:1: ( ( ruleWMLValue ) | ( ruleMacroTokens ) )
            int alt9=2;
            int LA9_0 = input.LA(1);

            if ( ((LA9_0>=RULE_ID && LA9_0<=RULE_ANY_OTHER)||(LA9_0>=20 && LA9_0<=27)) ) {
                alt9=1;
            }
            else if ( ((LA9_0>=28 && LA9_0<=32)) ) {
                alt9=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("", 9, 0, input);

                throw nvae;
            }
            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:746:1: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:746:1: ( ruleWMLValue )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:747:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLMacroParameterAccess().getWMLValueParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLMacroParameter__Alternatives1562);
                    ruleWMLValue();

                    state._fsp--;

                     after(grammarAccess.getWMLMacroParameterAccess().getWMLValueParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:752:6: ( ruleMacroTokens )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:752:6: ( ruleMacroTokens )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:753:1: ruleMacroTokens
                    {
                     before(grammarAccess.getWMLMacroParameterAccess().getMacroTokensParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleMacroTokens_in_rule__WMLMacroParameter__Alternatives1579);
                    ruleMacroTokens();

                    state._fsp--;

                     after(grammarAccess.getWMLMacroParameterAccess().getMacroTokensParserRuleCall_1()); 

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
    // $ANTLR end "rule__WMLMacroParameter__Alternatives"


    // $ANTLR start "rule__WMLValue__ValueAlternatives_0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:763:1: rule__WMLValue__ValueAlternatives_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( '~' ) | ( '.' ) | ( './' ) | ( '$' ) | ( '/' ) | ( '(' ) | ( ')' ) | ( RULE_ANY_OTHER ) );
    public final void rule__WMLValue__ValueAlternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:767:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( '~' ) | ( '.' ) | ( './' ) | ( '$' ) | ( '/' ) | ( '(' ) | ( ')' ) | ( RULE_ANY_OTHER ) )
            int alt10=11;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt10=1;
                }
                break;
            case RULE_STRING:
                {
                alt10=2;
                }
                break;
            case 20:
                {
                alt10=3;
                }
                break;
            case 21:
                {
                alt10=4;
                }
                break;
            case 22:
                {
                alt10=5;
                }
                break;
            case 23:
                {
                alt10=6;
                }
                break;
            case 24:
                {
                alt10=7;
                }
                break;
            case 25:
                {
                alt10=8;
                }
                break;
            case 26:
                {
                alt10=9;
                }
                break;
            case 27:
                {
                alt10=10;
                }
                break;
            case RULE_ANY_OTHER:
                {
                alt10=11;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("", 10, 0, input);

                throw nvae;
            }

            switch (alt10) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:768:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:768:1: ( RULE_ID )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:769:1: RULE_ID
                    {
                     before(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLValue__ValueAlternatives_01611); 
                     after(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:774:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:774:6: ( RULE_STRING )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:775:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLValue__ValueAlternatives_01628); 
                     after(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:780:6: ( '_' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:780:6: ( '_' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:781:1: '_'
                    {
                     before(grammarAccess.getWMLValueAccess().getValue_Keyword_0_2()); 
                    match(input,20,FOLLOW_20_in_rule__WMLValue__ValueAlternatives_01646); 
                     after(grammarAccess.getWMLValueAccess().getValue_Keyword_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:788:6: ( '~' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:788:6: ( '~' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:789:1: '~'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_3()); 
                    match(input,21,FOLLOW_21_in_rule__WMLValue__ValueAlternatives_01666); 
                     after(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:796:6: ( '.' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:796:6: ( '.' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:797:1: '.'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueFullStopKeyword_0_4()); 
                    match(input,22,FOLLOW_22_in_rule__WMLValue__ValueAlternatives_01686); 
                     after(grammarAccess.getWMLValueAccess().getValueFullStopKeyword_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:804:6: ( './' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:804:6: ( './' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:805:1: './'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueFullStopSolidusKeyword_0_5()); 
                    match(input,23,FOLLOW_23_in_rule__WMLValue__ValueAlternatives_01706); 
                     after(grammarAccess.getWMLValueAccess().getValueFullStopSolidusKeyword_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:812:6: ( '$' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:812:6: ( '$' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:813:1: '$'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueDollarSignKeyword_0_6()); 
                    match(input,24,FOLLOW_24_in_rule__WMLValue__ValueAlternatives_01726); 
                     after(grammarAccess.getWMLValueAccess().getValueDollarSignKeyword_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:820:6: ( '/' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:820:6: ( '/' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:821:1: '/'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueSolidusKeyword_0_7()); 
                    match(input,25,FOLLOW_25_in_rule__WMLValue__ValueAlternatives_01746); 
                     after(grammarAccess.getWMLValueAccess().getValueSolidusKeyword_0_7()); 

                    }


                    }
                    break;
                case 9 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:828:6: ( '(' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:828:6: ( '(' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:829:1: '('
                    {
                     before(grammarAccess.getWMLValueAccess().getValueLeftParenthesisKeyword_0_8()); 
                    match(input,26,FOLLOW_26_in_rule__WMLValue__ValueAlternatives_01766); 
                     after(grammarAccess.getWMLValueAccess().getValueLeftParenthesisKeyword_0_8()); 

                    }


                    }
                    break;
                case 10 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:836:6: ( ')' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:836:6: ( ')' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:837:1: ')'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueRightParenthesisKeyword_0_9()); 
                    match(input,27,FOLLOW_27_in_rule__WMLValue__ValueAlternatives_01786); 
                     after(grammarAccess.getWMLValueAccess().getValueRightParenthesisKeyword_0_9()); 

                    }


                    }
                    break;
                case 11 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:844:6: ( RULE_ANY_OTHER )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:844:6: ( RULE_ANY_OTHER )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:845:1: RULE_ANY_OTHER
                    {
                     before(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_10()); 
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__ValueAlternatives_01805); 
                     after(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_10()); 

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
    // $ANTLR end "rule__WMLValue__ValueAlternatives_0"


    // $ANTLR start "rule__MacroTokens__ValueAlternatives_0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:855:1: rule__MacroTokens__ValueAlternatives_0 : ( ( '=' ) | ( '[' ) | ( ']' ) | ( '+' ) | ( '[/' ) );
    public final void rule__MacroTokens__ValueAlternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:859:1: ( ( '=' ) | ( '[' ) | ( ']' ) | ( '+' ) | ( '[/' ) )
            int alt11=5;
            switch ( input.LA(1) ) {
            case 28:
                {
                alt11=1;
                }
                break;
            case 29:
                {
                alt11=2;
                }
                break;
            case 30:
                {
                alt11=3;
                }
                break;
            case 31:
                {
                alt11=4;
                }
                break;
            case 32:
                {
                alt11=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("", 11, 0, input);

                throw nvae;
            }

            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:860:1: ( '=' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:860:1: ( '=' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:861:1: '='
                    {
                     before(grammarAccess.getMacroTokensAccess().getValueEqualsSignKeyword_0_0()); 
                    match(input,28,FOLLOW_28_in_rule__MacroTokens__ValueAlternatives_01838); 
                     after(grammarAccess.getMacroTokensAccess().getValueEqualsSignKeyword_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:868:6: ( '[' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:868:6: ( '[' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:869:1: '['
                    {
                     before(grammarAccess.getMacroTokensAccess().getValueLeftSquareBracketKeyword_0_1()); 
                    match(input,29,FOLLOW_29_in_rule__MacroTokens__ValueAlternatives_01858); 
                     after(grammarAccess.getMacroTokensAccess().getValueLeftSquareBracketKeyword_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:876:6: ( ']' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:876:6: ( ']' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:877:1: ']'
                    {
                     before(grammarAccess.getMacroTokensAccess().getValueRightSquareBracketKeyword_0_2()); 
                    match(input,30,FOLLOW_30_in_rule__MacroTokens__ValueAlternatives_01878); 
                     after(grammarAccess.getMacroTokensAccess().getValueRightSquareBracketKeyword_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:884:6: ( '+' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:884:6: ( '+' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:885:1: '+'
                    {
                     before(grammarAccess.getMacroTokensAccess().getValuePlusSignKeyword_0_3()); 
                    match(input,31,FOLLOW_31_in_rule__MacroTokens__ValueAlternatives_01898); 
                     after(grammarAccess.getMacroTokensAccess().getValuePlusSignKeyword_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:892:6: ( '[/' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:892:6: ( '[/' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:893:1: '[/'
                    {
                     before(grammarAccess.getMacroTokensAccess().getValueLeftSquareBracketSolidusKeyword_0_4()); 
                    match(input,32,FOLLOW_32_in_rule__MacroTokens__ValueAlternatives_01918); 
                     after(grammarAccess.getMacroTokensAccess().getValueLeftSquareBracketSolidusKeyword_0_4()); 

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
    // $ANTLR end "rule__MacroTokens__ValueAlternatives_0"


    // $ANTLR start "rule__WMLTag__Group__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:907:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:911:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:912:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01950);
            rule__WMLTag__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01953);
            rule__WMLTag__Group__1();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLTag__Group__0"


    // $ANTLR start "rule__WMLTag__Group__0__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:919:1: rule__WMLTag__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:923:1: ( ( '[' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:924:1: ( '[' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:924:1: ( '[' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:925:1: '['
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0()); 
            match(input,29,FOLLOW_29_in_rule__WMLTag__Group__0__Impl1981); 
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
    // $ANTLR end "rule__WMLTag__Group__0__Impl"


    // $ANTLR start "rule__WMLTag__Group__1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:938:1: rule__WMLTag__Group__1 : rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 ;
    public final void rule__WMLTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:942:1: ( rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:943:2: rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__12012);
            rule__WMLTag__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__12015);
            rule__WMLTag__Group__2();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLTag__Group__1"


    // $ANTLR start "rule__WMLTag__Group__1__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:950:1: rule__WMLTag__Group__1__Impl : ( ( rule__WMLTag__PlusAssignment_1 )? ) ;
    public final void rule__WMLTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:954:1: ( ( ( rule__WMLTag__PlusAssignment_1 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:955:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:955:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:956:1: ( rule__WMLTag__PlusAssignment_1 )?
            {
             before(grammarAccess.getWMLTagAccess().getPlusAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:957:1: ( rule__WMLTag__PlusAssignment_1 )?
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0==31) ) {
                alt12=1;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:957:2: rule__WMLTag__PlusAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl2042);
                    rule__WMLTag__PlusAssignment_1();

                    state._fsp--;


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
    // $ANTLR end "rule__WMLTag__Group__1__Impl"


    // $ANTLR start "rule__WMLTag__Group__2"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:967:1: rule__WMLTag__Group__2 : rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 ;
    public final void rule__WMLTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:971:1: ( rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:972:2: rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__22073);
            rule__WMLTag__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__22076);
            rule__WMLTag__Group__3();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLTag__Group__2"


    // $ANTLR start "rule__WMLTag__Group__2__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:979:1: rule__WMLTag__Group__2__Impl : ( ( rule__WMLTag__NameAssignment_2 ) ) ;
    public final void rule__WMLTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:983:1: ( ( ( rule__WMLTag__NameAssignment_2 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:984:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:984:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:985:1: ( rule__WMLTag__NameAssignment_2 )
            {
             before(grammarAccess.getWMLTagAccess().getNameAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:986:1: ( rule__WMLTag__NameAssignment_2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:986:2: rule__WMLTag__NameAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl2103);
            rule__WMLTag__NameAssignment_2();

            state._fsp--;


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
    // $ANTLR end "rule__WMLTag__Group__2__Impl"


    // $ANTLR start "rule__WMLTag__Group__3"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:996:1: rule__WMLTag__Group__3 : rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 ;
    public final void rule__WMLTag__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1000:1: ( rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1001:2: rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__32133);
            rule__WMLTag__Group__3__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__32136);
            rule__WMLTag__Group__4();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLTag__Group__3"


    // $ANTLR start "rule__WMLTag__Group__3__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1008:1: rule__WMLTag__Group__3__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1012:1: ( ( ']' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1013:1: ( ']' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1013:1: ( ']' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1014:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3()); 
            match(input,30,FOLLOW_30_in_rule__WMLTag__Group__3__Impl2164); 
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
    // $ANTLR end "rule__WMLTag__Group__3__Impl"


    // $ANTLR start "rule__WMLTag__Group__4"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1027:1: rule__WMLTag__Group__4 : rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 ;
    public final void rule__WMLTag__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1031:1: ( rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1032:2: rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__42195);
            rule__WMLTag__Group__4__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__42198);
            rule__WMLTag__Group__5();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLTag__Group__4"


    // $ANTLR start "rule__WMLTag__Group__4__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1039:1: rule__WMLTag__Group__4__Impl : ( ( rule__WMLTag__ExpressionsAssignment_4 )* ) ;
    public final void rule__WMLTag__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1043:1: ( ( ( rule__WMLTag__ExpressionsAssignment_4 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1044:1: ( ( rule__WMLTag__ExpressionsAssignment_4 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1044:1: ( ( rule__WMLTag__ExpressionsAssignment_4 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1045:1: ( rule__WMLTag__ExpressionsAssignment_4 )*
            {
             before(grammarAccess.getWMLTagAccess().getExpressionsAssignment_4()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1046:1: ( rule__WMLTag__ExpressionsAssignment_4 )*
            loop13:
            do {
                int alt13=2;
                int LA13_0 = input.LA(1);

                if ( ((LA13_0>=RULE_IFDEF && LA13_0<=RULE_ID)||LA13_0==RULE_DEFINE||LA13_0==RULE_TEXTDOMAIN||LA13_0==29||LA13_0==33) ) {
                    alt13=1;
                }


                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1046:2: rule__WMLTag__ExpressionsAssignment_4
            	    {
            	    pushFollow(FOLLOW_rule__WMLTag__ExpressionsAssignment_4_in_rule__WMLTag__Group__4__Impl2225);
            	    rule__WMLTag__ExpressionsAssignment_4();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    break loop13;
                }
            } while (true);

             after(grammarAccess.getWMLTagAccess().getExpressionsAssignment_4()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLTag__Group__4__Impl"


    // $ANTLR start "rule__WMLTag__Group__5"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1056:1: rule__WMLTag__Group__5 : rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 ;
    public final void rule__WMLTag__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1060:1: ( rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1061:2: rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__52256);
            rule__WMLTag__Group__5__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__52259);
            rule__WMLTag__Group__6();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLTag__Group__5"


    // $ANTLR start "rule__WMLTag__Group__5__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1068:1: rule__WMLTag__Group__5__Impl : ( '[/' ) ;
    public final void rule__WMLTag__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1072:1: ( ( '[/' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1073:1: ( '[/' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1073:1: ( '[/' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1074:1: '[/'
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5()); 
            match(input,32,FOLLOW_32_in_rule__WMLTag__Group__5__Impl2287); 
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
    // $ANTLR end "rule__WMLTag__Group__5__Impl"


    // $ANTLR start "rule__WMLTag__Group__6"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1087:1: rule__WMLTag__Group__6 : rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 ;
    public final void rule__WMLTag__Group__6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1091:1: ( rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1092:2: rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__62318);
            rule__WMLTag__Group__6__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__62321);
            rule__WMLTag__Group__7();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLTag__Group__6"


    // $ANTLR start "rule__WMLTag__Group__6__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1099:1: rule__WMLTag__Group__6__Impl : ( ( rule__WMLTag__EndNameAssignment_6 ) ) ;
    public final void rule__WMLTag__Group__6__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1103:1: ( ( ( rule__WMLTag__EndNameAssignment_6 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1104:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1104:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1105:1: ( rule__WMLTag__EndNameAssignment_6 )
            {
             before(grammarAccess.getWMLTagAccess().getEndNameAssignment_6()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1106:1: ( rule__WMLTag__EndNameAssignment_6 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1106:2: rule__WMLTag__EndNameAssignment_6
            {
            pushFollow(FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl2348);
            rule__WMLTag__EndNameAssignment_6();

            state._fsp--;


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
    // $ANTLR end "rule__WMLTag__Group__6__Impl"


    // $ANTLR start "rule__WMLTag__Group__7"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1116:1: rule__WMLTag__Group__7 : rule__WMLTag__Group__7__Impl ;
    public final void rule__WMLTag__Group__7() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1120:1: ( rule__WMLTag__Group__7__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1121:2: rule__WMLTag__Group__7__Impl
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__72378);
            rule__WMLTag__Group__7__Impl();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLTag__Group__7"


    // $ANTLR start "rule__WMLTag__Group__7__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1127:1: rule__WMLTag__Group__7__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__7__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1131:1: ( ( ']' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1132:1: ( ']' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1132:1: ( ']' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1133:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_7()); 
            match(input,30,FOLLOW_30_in_rule__WMLTag__Group__7__Impl2406); 
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
    // $ANTLR end "rule__WMLTag__Group__7__Impl"


    // $ANTLR start "rule__WMLKey__Group__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1162:1: rule__WMLKey__Group__0 : rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 ;
    public final void rule__WMLKey__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1166:1: ( rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1167:2: rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02453);
            rule__WMLKey__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02456);
            rule__WMLKey__Group__1();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__Group__0"


    // $ANTLR start "rule__WMLKey__Group__0__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1174:1: rule__WMLKey__Group__0__Impl : ( ( rule__WMLKey__NameAssignment_0 ) ) ;
    public final void rule__WMLKey__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1178:1: ( ( ( rule__WMLKey__NameAssignment_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1179:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1179:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1180:1: ( rule__WMLKey__NameAssignment_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getNameAssignment_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1181:1: ( rule__WMLKey__NameAssignment_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1181:2: rule__WMLKey__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl2483);
            rule__WMLKey__NameAssignment_0();

            state._fsp--;


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
    // $ANTLR end "rule__WMLKey__Group__0__Impl"


    // $ANTLR start "rule__WMLKey__Group__1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1191:1: rule__WMLKey__Group__1 : rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 ;
    public final void rule__WMLKey__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1195:1: ( rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1196:2: rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12513);
            rule__WMLKey__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12516);
            rule__WMLKey__Group__2();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__Group__1"


    // $ANTLR start "rule__WMLKey__Group__1__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1203:1: rule__WMLKey__Group__1__Impl : ( '=' ) ;
    public final void rule__WMLKey__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1207:1: ( ( '=' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1208:1: ( '=' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1208:1: ( '=' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1209:1: '='
            {
             before(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1()); 
            match(input,28,FOLLOW_28_in_rule__WMLKey__Group__1__Impl2544); 
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
    // $ANTLR end "rule__WMLKey__Group__1__Impl"


    // $ANTLR start "rule__WMLKey__Group__2"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1222:1: rule__WMLKey__Group__2 : rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 ;
    public final void rule__WMLKey__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1226:1: ( rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1227:2: rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22575);
            rule__WMLKey__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__22578);
            rule__WMLKey__Group__3();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__Group__2"


    // $ANTLR start "rule__WMLKey__Group__2__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1234:1: rule__WMLKey__Group__2__Impl : ( ( rule__WMLKey__ValuesAssignment_2 )* ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1238:1: ( ( ( rule__WMLKey__ValuesAssignment_2 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1239:1: ( ( rule__WMLKey__ValuesAssignment_2 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1239:1: ( ( rule__WMLKey__ValuesAssignment_2 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1240:1: ( rule__WMLKey__ValuesAssignment_2 )*
            {
             before(grammarAccess.getWMLKeyAccess().getValuesAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1241:1: ( rule__WMLKey__ValuesAssignment_2 )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( ((LA14_0>=RULE_ID && LA14_0<=RULE_ANY_OTHER)||LA14_0==RULE_LUA_CODE||(LA14_0>=20 && LA14_0<=27)||LA14_0==29||LA14_0==33) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1241:2: rule__WMLKey__ValuesAssignment_2
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__ValuesAssignment_2_in_rule__WMLKey__Group__2__Impl2605);
            	    rule__WMLKey__ValuesAssignment_2();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    break loop14;
                }
            } while (true);

             after(grammarAccess.getWMLKeyAccess().getValuesAssignment_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__Group__2__Impl"


    // $ANTLR start "rule__WMLKey__Group__3"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1251:1: rule__WMLKey__Group__3 : rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4 ;
    public final void rule__WMLKey__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1255:1: ( rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1256:2: rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__32636);
            rule__WMLKey__Group__3__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__4_in_rule__WMLKey__Group__32639);
            rule__WMLKey__Group__4();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__Group__3"


    // $ANTLR start "rule__WMLKey__Group__3__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1263:1: rule__WMLKey__Group__3__Impl : ( ( rule__WMLKey__Group_3__0 )* ) ;
    public final void rule__WMLKey__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1267:1: ( ( ( rule__WMLKey__Group_3__0 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1268:1: ( ( rule__WMLKey__Group_3__0 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1268:1: ( ( rule__WMLKey__Group_3__0 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1269:1: ( rule__WMLKey__Group_3__0 )*
            {
             before(grammarAccess.getWMLKeyAccess().getGroup_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1270:1: ( rule__WMLKey__Group_3__0 )*
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( (LA15_0==RULE_EOL) ) {
                    int LA15_1 = input.LA(2);

                    if ( (LA15_1==31) ) {
                        alt15=1;
                    }


                }
                else if ( (LA15_0==31) ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1270:2: rule__WMLKey__Group_3__0
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__Group_3__0_in_rule__WMLKey__Group__3__Impl2666);
            	    rule__WMLKey__Group_3__0();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    break loop15;
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
    // $ANTLR end "rule__WMLKey__Group__3__Impl"


    // $ANTLR start "rule__WMLKey__Group__4"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1280:1: rule__WMLKey__Group__4 : rule__WMLKey__Group__4__Impl ;
    public final void rule__WMLKey__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1284:1: ( rule__WMLKey__Group__4__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1285:2: rule__WMLKey__Group__4__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__4__Impl_in_rule__WMLKey__Group__42697);
            rule__WMLKey__Group__4__Impl();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__Group__4"


    // $ANTLR start "rule__WMLKey__Group__4__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1291:1: rule__WMLKey__Group__4__Impl : ( ( rule__WMLKey__EolAssignment_4 ) ) ;
    public final void rule__WMLKey__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1295:1: ( ( ( rule__WMLKey__EolAssignment_4 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1296:1: ( ( rule__WMLKey__EolAssignment_4 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1296:1: ( ( rule__WMLKey__EolAssignment_4 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1297:1: ( rule__WMLKey__EolAssignment_4 )
            {
             before(grammarAccess.getWMLKeyAccess().getEolAssignment_4()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1298:1: ( rule__WMLKey__EolAssignment_4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1298:2: rule__WMLKey__EolAssignment_4
            {
            pushFollow(FOLLOW_rule__WMLKey__EolAssignment_4_in_rule__WMLKey__Group__4__Impl2724);
            rule__WMLKey__EolAssignment_4();

            state._fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getEolAssignment_4()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__Group__4__Impl"


    // $ANTLR start "rule__WMLKey__Group_3__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1318:1: rule__WMLKey__Group_3__0 : rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1 ;
    public final void rule__WMLKey__Group_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1322:1: ( rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1323:2: rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__0__Impl_in_rule__WMLKey__Group_3__02764);
            rule__WMLKey__Group_3__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__1_in_rule__WMLKey__Group_3__02767);
            rule__WMLKey__Group_3__1();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__Group_3__0"


    // $ANTLR start "rule__WMLKey__Group_3__0__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1330:1: rule__WMLKey__Group_3__0__Impl : ( ( RULE_EOL )? ) ;
    public final void rule__WMLKey__Group_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1334:1: ( ( ( RULE_EOL )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1335:1: ( ( RULE_EOL )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1335:1: ( ( RULE_EOL )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1336:1: ( RULE_EOL )?
            {
             before(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1337:1: ( RULE_EOL )?
            int alt16=2;
            int LA16_0 = input.LA(1);

            if ( (LA16_0==RULE_EOL) ) {
                alt16=1;
            }
            switch (alt16) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1337:3: RULE_EOL
                    {
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__0__Impl2795); 

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
    // $ANTLR end "rule__WMLKey__Group_3__0__Impl"


    // $ANTLR start "rule__WMLKey__Group_3__1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1347:1: rule__WMLKey__Group_3__1 : rule__WMLKey__Group_3__1__Impl rule__WMLKey__Group_3__2 ;
    public final void rule__WMLKey__Group_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1351:1: ( rule__WMLKey__Group_3__1__Impl rule__WMLKey__Group_3__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1352:2: rule__WMLKey__Group_3__1__Impl rule__WMLKey__Group_3__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__1__Impl_in_rule__WMLKey__Group_3__12826);
            rule__WMLKey__Group_3__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__2_in_rule__WMLKey__Group_3__12829);
            rule__WMLKey__Group_3__2();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__Group_3__1"


    // $ANTLR start "rule__WMLKey__Group_3__1__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1359:1: rule__WMLKey__Group_3__1__Impl : ( '+' ) ;
    public final void rule__WMLKey__Group_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1363:1: ( ( '+' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1364:1: ( '+' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1364:1: ( '+' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1365:1: '+'
            {
             before(grammarAccess.getWMLKeyAccess().getPlusSignKeyword_3_1()); 
            match(input,31,FOLLOW_31_in_rule__WMLKey__Group_3__1__Impl2857); 
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
    // $ANTLR end "rule__WMLKey__Group_3__1__Impl"


    // $ANTLR start "rule__WMLKey__Group_3__2"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1378:1: rule__WMLKey__Group_3__2 : rule__WMLKey__Group_3__2__Impl rule__WMLKey__Group_3__3 ;
    public final void rule__WMLKey__Group_3__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1382:1: ( rule__WMLKey__Group_3__2__Impl rule__WMLKey__Group_3__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1383:2: rule__WMLKey__Group_3__2__Impl rule__WMLKey__Group_3__3
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__2__Impl_in_rule__WMLKey__Group_3__22888);
            rule__WMLKey__Group_3__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__3_in_rule__WMLKey__Group_3__22891);
            rule__WMLKey__Group_3__3();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__Group_3__2"


    // $ANTLR start "rule__WMLKey__Group_3__2__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1390:1: rule__WMLKey__Group_3__2__Impl : ( ( RULE_EOL )? ) ;
    public final void rule__WMLKey__Group_3__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1394:1: ( ( ( RULE_EOL )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1395:1: ( ( RULE_EOL )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1395:1: ( ( RULE_EOL )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1396:1: ( RULE_EOL )?
            {
             before(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1397:1: ( RULE_EOL )?
            int alt17=2;
            int LA17_0 = input.LA(1);

            if ( (LA17_0==RULE_EOL) ) {
                alt17=1;
            }
            switch (alt17) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1397:3: RULE_EOL
                    {
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__2__Impl2919); 

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
    // $ANTLR end "rule__WMLKey__Group_3__2__Impl"


    // $ANTLR start "rule__WMLKey__Group_3__3"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1407:1: rule__WMLKey__Group_3__3 : rule__WMLKey__Group_3__3__Impl ;
    public final void rule__WMLKey__Group_3__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1411:1: ( rule__WMLKey__Group_3__3__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1412:2: rule__WMLKey__Group_3__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__3__Impl_in_rule__WMLKey__Group_3__32950);
            rule__WMLKey__Group_3__3__Impl();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__Group_3__3"


    // $ANTLR start "rule__WMLKey__Group_3__3__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1418:1: rule__WMLKey__Group_3__3__Impl : ( ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* ) ) ;
    public final void rule__WMLKey__Group_3__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1422:1: ( ( ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1423:1: ( ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1423:1: ( ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1424:1: ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1424:1: ( ( rule__WMLKey__ValuesAssignment_3_3 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1425:1: ( rule__WMLKey__ValuesAssignment_3_3 )
            {
             before(grammarAccess.getWMLKeyAccess().getValuesAssignment_3_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1426:1: ( rule__WMLKey__ValuesAssignment_3_3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1426:2: rule__WMLKey__ValuesAssignment_3_3
            {
            pushFollow(FOLLOW_rule__WMLKey__ValuesAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2979);
            rule__WMLKey__ValuesAssignment_3_3();

            state._fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getValuesAssignment_3_3()); 

            }

            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1429:1: ( ( rule__WMLKey__ValuesAssignment_3_3 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1430:1: ( rule__WMLKey__ValuesAssignment_3_3 )*
            {
             before(grammarAccess.getWMLKeyAccess().getValuesAssignment_3_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1431:1: ( rule__WMLKey__ValuesAssignment_3_3 )*
            loop18:
            do {
                int alt18=2;
                int LA18_0 = input.LA(1);

                if ( ((LA18_0>=RULE_ID && LA18_0<=RULE_ANY_OTHER)||LA18_0==RULE_LUA_CODE||(LA18_0>=20 && LA18_0<=27)||LA18_0==29||LA18_0==33) ) {
                    alt18=1;
                }


                switch (alt18) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1431:2: rule__WMLKey__ValuesAssignment_3_3
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__ValuesAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2991);
            	    rule__WMLKey__ValuesAssignment_3_3();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    break loop18;
                }
            } while (true);

             after(grammarAccess.getWMLKeyAccess().getValuesAssignment_3_3()); 

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
    // $ANTLR end "rule__WMLKey__Group_3__3__Impl"


    // $ANTLR start "rule__WMLMacroCall__Group__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1450:1: rule__WMLMacroCall__Group__0 : rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 ;
    public final void rule__WMLMacroCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1454:1: ( rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1455:2: rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__03032);
            rule__WMLMacroCall__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__03035);
            rule__WMLMacroCall__Group__1();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroCall__Group__0"


    // $ANTLR start "rule__WMLMacroCall__Group__0__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1462:1: rule__WMLMacroCall__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacroCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1466:1: ( ( '{' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1467:1: ( '{' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1467:1: ( '{' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1468:1: '{'
            {
             before(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,33,FOLLOW_33_in_rule__WMLMacroCall__Group__0__Impl3063); 
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
    // $ANTLR end "rule__WMLMacroCall__Group__0__Impl"


    // $ANTLR start "rule__WMLMacroCall__Group__1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1481:1: rule__WMLMacroCall__Group__1 : rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 ;
    public final void rule__WMLMacroCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1485:1: ( rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1486:2: rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__13094);
            rule__WMLMacroCall__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__13097);
            rule__WMLMacroCall__Group__2();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroCall__Group__1"


    // $ANTLR start "rule__WMLMacroCall__Group__1__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1493:1: rule__WMLMacroCall__Group__1__Impl : ( ( rule__WMLMacroCall__PointAssignment_1 )? ) ;
    public final void rule__WMLMacroCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1497:1: ( ( ( rule__WMLMacroCall__PointAssignment_1 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1498:1: ( ( rule__WMLMacroCall__PointAssignment_1 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1498:1: ( ( rule__WMLMacroCall__PointAssignment_1 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1499:1: ( rule__WMLMacroCall__PointAssignment_1 )?
            {
             before(grammarAccess.getWMLMacroCallAccess().getPointAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1500:1: ( rule__WMLMacroCall__PointAssignment_1 )?
            int alt19=2;
            int LA19_0 = input.LA(1);

            if ( (LA19_0==23) ) {
                alt19=1;
            }
            switch (alt19) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1500:2: rule__WMLMacroCall__PointAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__PointAssignment_1_in_rule__WMLMacroCall__Group__1__Impl3124);
                    rule__WMLMacroCall__PointAssignment_1();

                    state._fsp--;


                    }
                    break;

            }

             after(grammarAccess.getWMLMacroCallAccess().getPointAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroCall__Group__1__Impl"


    // $ANTLR start "rule__WMLMacroCall__Group__2"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1510:1: rule__WMLMacroCall__Group__2 : rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 ;
    public final void rule__WMLMacroCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1514:1: ( rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1515:2: rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__23155);
            rule__WMLMacroCall__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__23158);
            rule__WMLMacroCall__Group__3();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroCall__Group__2"


    // $ANTLR start "rule__WMLMacroCall__Group__2__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1522:1: rule__WMLMacroCall__Group__2__Impl : ( ( rule__WMLMacroCall__RelativeAssignment_2 )? ) ;
    public final void rule__WMLMacroCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1526:1: ( ( ( rule__WMLMacroCall__RelativeAssignment_2 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1527:1: ( ( rule__WMLMacroCall__RelativeAssignment_2 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1527:1: ( ( rule__WMLMacroCall__RelativeAssignment_2 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1528:1: ( rule__WMLMacroCall__RelativeAssignment_2 )?
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1529:1: ( rule__WMLMacroCall__RelativeAssignment_2 )?
            int alt20=2;
            int LA20_0 = input.LA(1);

            if ( (LA20_0==21) ) {
                alt20=1;
            }
            switch (alt20) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1529:2: rule__WMLMacroCall__RelativeAssignment_2
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__RelativeAssignment_2_in_rule__WMLMacroCall__Group__2__Impl3185);
                    rule__WMLMacroCall__RelativeAssignment_2();

                    state._fsp--;


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
    // $ANTLR end "rule__WMLMacroCall__Group__2__Impl"


    // $ANTLR start "rule__WMLMacroCall__Group__3"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1539:1: rule__WMLMacroCall__Group__3 : rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 ;
    public final void rule__WMLMacroCall__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1543:1: ( rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1544:2: rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__33216);
            rule__WMLMacroCall__Group__3__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__33219);
            rule__WMLMacroCall__Group__4();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroCall__Group__3"


    // $ANTLR start "rule__WMLMacroCall__Group__3__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1551:1: rule__WMLMacroCall__Group__3__Impl : ( ( rule__WMLMacroCall__NameAssignment_3 ) ) ;
    public final void rule__WMLMacroCall__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1555:1: ( ( ( rule__WMLMacroCall__NameAssignment_3 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1556:1: ( ( rule__WMLMacroCall__NameAssignment_3 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1556:1: ( ( rule__WMLMacroCall__NameAssignment_3 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1557:1: ( rule__WMLMacroCall__NameAssignment_3 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameAssignment_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1558:1: ( rule__WMLMacroCall__NameAssignment_3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1558:2: rule__WMLMacroCall__NameAssignment_3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__NameAssignment_3_in_rule__WMLMacroCall__Group__3__Impl3246);
            rule__WMLMacroCall__NameAssignment_3();

            state._fsp--;


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
    // $ANTLR end "rule__WMLMacroCall__Group__3__Impl"


    // $ANTLR start "rule__WMLMacroCall__Group__4"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1568:1: rule__WMLMacroCall__Group__4 : rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5 ;
    public final void rule__WMLMacroCall__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1572:1: ( rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1573:2: rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__43276);
            rule__WMLMacroCall__Group__4__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__5_in_rule__WMLMacroCall__Group__43279);
            rule__WMLMacroCall__Group__5();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroCall__Group__4"


    // $ANTLR start "rule__WMLMacroCall__Group__4__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1580:1: rule__WMLMacroCall__Group__4__Impl : ( ( rule__WMLMacroCall__ParametersAssignment_4 )* ) ;
    public final void rule__WMLMacroCall__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1584:1: ( ( ( rule__WMLMacroCall__ParametersAssignment_4 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1585:1: ( ( rule__WMLMacroCall__ParametersAssignment_4 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1585:1: ( ( rule__WMLMacroCall__ParametersAssignment_4 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1586:1: ( rule__WMLMacroCall__ParametersAssignment_4 )*
            {
             before(grammarAccess.getWMLMacroCallAccess().getParametersAssignment_4()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1587:1: ( rule__WMLMacroCall__ParametersAssignment_4 )*
            loop21:
            do {
                int alt21=2;
                int LA21_0 = input.LA(1);

                if ( ((LA21_0>=RULE_ID && LA21_0<=RULE_ANY_OTHER)||(LA21_0>=20 && LA21_0<=33)) ) {
                    alt21=1;
                }


                switch (alt21) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1587:2: rule__WMLMacroCall__ParametersAssignment_4
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroCall__ParametersAssignment_4_in_rule__WMLMacroCall__Group__4__Impl3306);
            	    rule__WMLMacroCall__ParametersAssignment_4();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    break loop21;
                }
            } while (true);

             after(grammarAccess.getWMLMacroCallAccess().getParametersAssignment_4()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroCall__Group__4__Impl"


    // $ANTLR start "rule__WMLMacroCall__Group__5"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1597:1: rule__WMLMacroCall__Group__5 : rule__WMLMacroCall__Group__5__Impl ;
    public final void rule__WMLMacroCall__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1601:1: ( rule__WMLMacroCall__Group__5__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1602:2: rule__WMLMacroCall__Group__5__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__5__Impl_in_rule__WMLMacroCall__Group__53337);
            rule__WMLMacroCall__Group__5__Impl();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroCall__Group__5"


    // $ANTLR start "rule__WMLMacroCall__Group__5__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1608:1: rule__WMLMacroCall__Group__5__Impl : ( '}' ) ;
    public final void rule__WMLMacroCall__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1612:1: ( ( '}' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1613:1: ( '}' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1613:1: ( '}' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1614:1: '}'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_5()); 
            match(input,34,FOLLOW_34_in_rule__WMLMacroCall__Group__5__Impl3365); 
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
    // $ANTLR end "rule__WMLMacroCall__Group__5__Impl"


    // $ANTLR start "rule__WMLArrayCall__Group__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1639:1: rule__WMLArrayCall__Group__0 : rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 ;
    public final void rule__WMLArrayCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1643:1: ( rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1644:2: rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__03408);
            rule__WMLArrayCall__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__03411);
            rule__WMLArrayCall__Group__1();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLArrayCall__Group__0"


    // $ANTLR start "rule__WMLArrayCall__Group__0__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1651:1: rule__WMLArrayCall__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLArrayCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1655:1: ( ( '[' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1656:1: ( '[' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1656:1: ( '[' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1657:1: '['
            {
             before(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0()); 
            match(input,29,FOLLOW_29_in_rule__WMLArrayCall__Group__0__Impl3439); 
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
    // $ANTLR end "rule__WMLArrayCall__Group__0__Impl"


    // $ANTLR start "rule__WMLArrayCall__Group__1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1670:1: rule__WMLArrayCall__Group__1 : rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 ;
    public final void rule__WMLArrayCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1674:1: ( rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1675:2: rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__13470);
            rule__WMLArrayCall__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__13473);
            rule__WMLArrayCall__Group__2();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLArrayCall__Group__1"


    // $ANTLR start "rule__WMLArrayCall__Group__1__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1682:1: rule__WMLArrayCall__Group__1__Impl : ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) ;
    public final void rule__WMLArrayCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1686:1: ( ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1687:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1687:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1688:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1688:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1689:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1690:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1690:2: rule__WMLArrayCall__ValueAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3502);
            rule__WMLArrayCall__ValueAssignment_1();

            state._fsp--;


            }

             after(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 

            }

            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1693:1: ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1694:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1695:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            loop22:
            do {
                int alt22=2;
                int LA22_0 = input.LA(1);

                if ( ((LA22_0>=RULE_ID && LA22_0<=RULE_ANY_OTHER)||(LA22_0>=20 && LA22_0<=27)) ) {
                    alt22=1;
                }


                switch (alt22) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1695:2: rule__WMLArrayCall__ValueAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3514);
            	    rule__WMLArrayCall__ValueAssignment_1();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    break loop22;
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
    // $ANTLR end "rule__WMLArrayCall__Group__1__Impl"


    // $ANTLR start "rule__WMLArrayCall__Group__2"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1706:1: rule__WMLArrayCall__Group__2 : rule__WMLArrayCall__Group__2__Impl ;
    public final void rule__WMLArrayCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1710:1: ( rule__WMLArrayCall__Group__2__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1711:2: rule__WMLArrayCall__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__23547);
            rule__WMLArrayCall__Group__2__Impl();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLArrayCall__Group__2"


    // $ANTLR start "rule__WMLArrayCall__Group__2__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1717:1: rule__WMLArrayCall__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLArrayCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1721:1: ( ( ']' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1722:1: ( ']' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1722:1: ( ']' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1723:1: ']'
            {
             before(grammarAccess.getWMLArrayCallAccess().getRightSquareBracketKeyword_2()); 
            match(input,30,FOLLOW_30_in_rule__WMLArrayCall__Group__2__Impl3575); 
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
    // $ANTLR end "rule__WMLArrayCall__Group__2__Impl"


    // $ANTLR start "rule__WMLMacroDefine__Group__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1742:1: rule__WMLMacroDefine__Group__0 : rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1 ;
    public final void rule__WMLMacroDefine__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1746:1: ( rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1747:2: rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__0__Impl_in_rule__WMLMacroDefine__Group__03612);
            rule__WMLMacroDefine__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__1_in_rule__WMLMacroDefine__Group__03615);
            rule__WMLMacroDefine__Group__1();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroDefine__Group__0"


    // $ANTLR start "rule__WMLMacroDefine__Group__0__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1754:1: rule__WMLMacroDefine__Group__0__Impl : ( ( rule__WMLMacroDefine__NameAssignment_0 ) ) ;
    public final void rule__WMLMacroDefine__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1758:1: ( ( ( rule__WMLMacroDefine__NameAssignment_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1759:1: ( ( rule__WMLMacroDefine__NameAssignment_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1759:1: ( ( rule__WMLMacroDefine__NameAssignment_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1760:1: ( rule__WMLMacroDefine__NameAssignment_0 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameAssignment_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1761:1: ( rule__WMLMacroDefine__NameAssignment_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1761:2: rule__WMLMacroDefine__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__NameAssignment_0_in_rule__WMLMacroDefine__Group__0__Impl3642);
            rule__WMLMacroDefine__NameAssignment_0();

            state._fsp--;


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
    // $ANTLR end "rule__WMLMacroDefine__Group__0__Impl"


    // $ANTLR start "rule__WMLMacroDefine__Group__1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1771:1: rule__WMLMacroDefine__Group__1 : rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2 ;
    public final void rule__WMLMacroDefine__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1775:1: ( rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1776:2: rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__1__Impl_in_rule__WMLMacroDefine__Group__13672);
            rule__WMLMacroDefine__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__2_in_rule__WMLMacroDefine__Group__13675);
            rule__WMLMacroDefine__Group__2();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroDefine__Group__1"


    // $ANTLR start "rule__WMLMacroDefine__Group__1__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1783:1: rule__WMLMacroDefine__Group__1__Impl : ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* ) ;
    public final void rule__WMLMacroDefine__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1787:1: ( ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1788:1: ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1788:1: ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1789:1: ( rule__WMLMacroDefine__ExpressionsAssignment_1 )*
            {
             before(grammarAccess.getWMLMacroDefineAccess().getExpressionsAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1790:1: ( rule__WMLMacroDefine__ExpressionsAssignment_1 )*
            loop23:
            do {
                int alt23=2;
                int LA23_0 = input.LA(1);

                if ( ((LA23_0>=RULE_IFDEF && LA23_0<=RULE_DEFINE)||LA23_0==RULE_TEXTDOMAIN||(LA23_0>=20 && LA23_0<=27)||LA23_0==29||LA23_0==33) ) {
                    alt23=1;
                }


                switch (alt23) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1790:2: rule__WMLMacroDefine__ExpressionsAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroDefine__ExpressionsAssignment_1_in_rule__WMLMacroDefine__Group__1__Impl3702);
            	    rule__WMLMacroDefine__ExpressionsAssignment_1();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    break loop23;
                }
            } while (true);

             after(grammarAccess.getWMLMacroDefineAccess().getExpressionsAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroDefine__Group__1__Impl"


    // $ANTLR start "rule__WMLMacroDefine__Group__2"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1800:1: rule__WMLMacroDefine__Group__2 : rule__WMLMacroDefine__Group__2__Impl ;
    public final void rule__WMLMacroDefine__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1804:1: ( rule__WMLMacroDefine__Group__2__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1805:2: rule__WMLMacroDefine__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__2__Impl_in_rule__WMLMacroDefine__Group__23733);
            rule__WMLMacroDefine__Group__2__Impl();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroDefine__Group__2"


    // $ANTLR start "rule__WMLMacroDefine__Group__2__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1811:1: rule__WMLMacroDefine__Group__2__Impl : ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) ) ;
    public final void rule__WMLMacroDefine__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1815:1: ( ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1816:1: ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1816:1: ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1817:1: ( rule__WMLMacroDefine__EndNameAssignment_2 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getEndNameAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1818:1: ( rule__WMLMacroDefine__EndNameAssignment_2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1818:2: rule__WMLMacroDefine__EndNameAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__EndNameAssignment_2_in_rule__WMLMacroDefine__Group__2__Impl3760);
            rule__WMLMacroDefine__EndNameAssignment_2();

            state._fsp--;


            }

             after(grammarAccess.getWMLMacroDefineAccess().getEndNameAssignment_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroDefine__Group__2__Impl"


    // $ANTLR start "rule__WMLPreprocIF__Group__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1834:1: rule__WMLPreprocIF__Group__0 : rule__WMLPreprocIF__Group__0__Impl rule__WMLPreprocIF__Group__1 ;
    public final void rule__WMLPreprocIF__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1838:1: ( rule__WMLPreprocIF__Group__0__Impl rule__WMLPreprocIF__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1839:2: rule__WMLPreprocIF__Group__0__Impl rule__WMLPreprocIF__Group__1
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__0__Impl_in_rule__WMLPreprocIF__Group__03796);
            rule__WMLPreprocIF__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__1_in_rule__WMLPreprocIF__Group__03799);
            rule__WMLPreprocIF__Group__1();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__Group__0"


    // $ANTLR start "rule__WMLPreprocIF__Group__0__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1846:1: rule__WMLPreprocIF__Group__0__Impl : ( ( rule__WMLPreprocIF__NameAssignment_0 ) ) ;
    public final void rule__WMLPreprocIF__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1850:1: ( ( ( rule__WMLPreprocIF__NameAssignment_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1851:1: ( ( rule__WMLPreprocIF__NameAssignment_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1851:1: ( ( rule__WMLPreprocIF__NameAssignment_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1852:1: ( rule__WMLPreprocIF__NameAssignment_0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getNameAssignment_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1853:1: ( rule__WMLPreprocIF__NameAssignment_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1853:2: rule__WMLPreprocIF__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__NameAssignment_0_in_rule__WMLPreprocIF__Group__0__Impl3826);
            rule__WMLPreprocIF__NameAssignment_0();

            state._fsp--;


            }

             after(grammarAccess.getWMLPreprocIFAccess().getNameAssignment_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__Group__0__Impl"


    // $ANTLR start "rule__WMLPreprocIF__Group__1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1863:1: rule__WMLPreprocIF__Group__1 : rule__WMLPreprocIF__Group__1__Impl rule__WMLPreprocIF__Group__2 ;
    public final void rule__WMLPreprocIF__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1867:1: ( rule__WMLPreprocIF__Group__1__Impl rule__WMLPreprocIF__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1868:2: rule__WMLPreprocIF__Group__1__Impl rule__WMLPreprocIF__Group__2
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__1__Impl_in_rule__WMLPreprocIF__Group__13856);
            rule__WMLPreprocIF__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__2_in_rule__WMLPreprocIF__Group__13859);
            rule__WMLPreprocIF__Group__2();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__Group__1"


    // $ANTLR start "rule__WMLPreprocIF__Group__1__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1875:1: rule__WMLPreprocIF__Group__1__Impl : ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* ) ;
    public final void rule__WMLPreprocIF__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1879:1: ( ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1880:1: ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1880:1: ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1881:1: ( rule__WMLPreprocIF__ExpressionsAssignment_1 )*
            {
             before(grammarAccess.getWMLPreprocIFAccess().getExpressionsAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1882:1: ( rule__WMLPreprocIF__ExpressionsAssignment_1 )*
            loop24:
            do {
                int alt24=2;
                int LA24_0 = input.LA(1);

                if ( ((LA24_0>=RULE_IFDEF && LA24_0<=RULE_DEFINE)||LA24_0==RULE_TEXTDOMAIN||(LA24_0>=20 && LA24_0<=27)||LA24_0==29||LA24_0==33) ) {
                    alt24=1;
                }


                switch (alt24) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1882:2: rule__WMLPreprocIF__ExpressionsAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLPreprocIF__ExpressionsAssignment_1_in_rule__WMLPreprocIF__Group__1__Impl3886);
            	    rule__WMLPreprocIF__ExpressionsAssignment_1();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    break loop24;
                }
            } while (true);

             after(grammarAccess.getWMLPreprocIFAccess().getExpressionsAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__Group__1__Impl"


    // $ANTLR start "rule__WMLPreprocIF__Group__2"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1892:1: rule__WMLPreprocIF__Group__2 : rule__WMLPreprocIF__Group__2__Impl rule__WMLPreprocIF__Group__3 ;
    public final void rule__WMLPreprocIF__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1896:1: ( rule__WMLPreprocIF__Group__2__Impl rule__WMLPreprocIF__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1897:2: rule__WMLPreprocIF__Group__2__Impl rule__WMLPreprocIF__Group__3
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__2__Impl_in_rule__WMLPreprocIF__Group__23917);
            rule__WMLPreprocIF__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__3_in_rule__WMLPreprocIF__Group__23920);
            rule__WMLPreprocIF__Group__3();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__Group__2"


    // $ANTLR start "rule__WMLPreprocIF__Group__2__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1904:1: rule__WMLPreprocIF__Group__2__Impl : ( ( rule__WMLPreprocIF__Group_2__0 )? ) ;
    public final void rule__WMLPreprocIF__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1908:1: ( ( ( rule__WMLPreprocIF__Group_2__0 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1909:1: ( ( rule__WMLPreprocIF__Group_2__0 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1909:1: ( ( rule__WMLPreprocIF__Group_2__0 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1910:1: ( rule__WMLPreprocIF__Group_2__0 )?
            {
             before(grammarAccess.getWMLPreprocIFAccess().getGroup_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1911:1: ( rule__WMLPreprocIF__Group_2__0 )?
            int alt25=2;
            int LA25_0 = input.LA(1);

            if ( (LA25_0==RULE_ELSE) ) {
                alt25=1;
            }
            switch (alt25) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1911:2: rule__WMLPreprocIF__Group_2__0
                    {
                    pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__0_in_rule__WMLPreprocIF__Group__2__Impl3947);
                    rule__WMLPreprocIF__Group_2__0();

                    state._fsp--;


                    }
                    break;

            }

             after(grammarAccess.getWMLPreprocIFAccess().getGroup_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__Group__2__Impl"


    // $ANTLR start "rule__WMLPreprocIF__Group__3"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1921:1: rule__WMLPreprocIF__Group__3 : rule__WMLPreprocIF__Group__3__Impl ;
    public final void rule__WMLPreprocIF__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1925:1: ( rule__WMLPreprocIF__Group__3__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1926:2: rule__WMLPreprocIF__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__3__Impl_in_rule__WMLPreprocIF__Group__33978);
            rule__WMLPreprocIF__Group__3__Impl();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__Group__3"


    // $ANTLR start "rule__WMLPreprocIF__Group__3__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1932:1: rule__WMLPreprocIF__Group__3__Impl : ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) ) ;
    public final void rule__WMLPreprocIF__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1936:1: ( ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1937:1: ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1937:1: ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1938:1: ( rule__WMLPreprocIF__EndNameAssignment_3 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getEndNameAssignment_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1939:1: ( rule__WMLPreprocIF__EndNameAssignment_3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1939:2: rule__WMLPreprocIF__EndNameAssignment_3
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__EndNameAssignment_3_in_rule__WMLPreprocIF__Group__3__Impl4005);
            rule__WMLPreprocIF__EndNameAssignment_3();

            state._fsp--;


            }

             after(grammarAccess.getWMLPreprocIFAccess().getEndNameAssignment_3()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__Group__3__Impl"


    // $ANTLR start "rule__WMLPreprocIF__Group_2__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1957:1: rule__WMLPreprocIF__Group_2__0 : rule__WMLPreprocIF__Group_2__0__Impl rule__WMLPreprocIF__Group_2__1 ;
    public final void rule__WMLPreprocIF__Group_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1961:1: ( rule__WMLPreprocIF__Group_2__0__Impl rule__WMLPreprocIF__Group_2__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1962:2: rule__WMLPreprocIF__Group_2__0__Impl rule__WMLPreprocIF__Group_2__1
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__0__Impl_in_rule__WMLPreprocIF__Group_2__04043);
            rule__WMLPreprocIF__Group_2__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__1_in_rule__WMLPreprocIF__Group_2__04046);
            rule__WMLPreprocIF__Group_2__1();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__Group_2__0"


    // $ANTLR start "rule__WMLPreprocIF__Group_2__0__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1969:1: rule__WMLPreprocIF__Group_2__0__Impl : ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) ) ;
    public final void rule__WMLPreprocIF__Group_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1973:1: ( ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1974:1: ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1974:1: ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1975:1: ( rule__WMLPreprocIF__ElsesAssignment_2_0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElsesAssignment_2_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1976:1: ( rule__WMLPreprocIF__ElsesAssignment_2_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1976:2: rule__WMLPreprocIF__ElsesAssignment_2_0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__ElsesAssignment_2_0_in_rule__WMLPreprocIF__Group_2__0__Impl4073);
            rule__WMLPreprocIF__ElsesAssignment_2_0();

            state._fsp--;


            }

             after(grammarAccess.getWMLPreprocIFAccess().getElsesAssignment_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__Group_2__0__Impl"


    // $ANTLR start "rule__WMLPreprocIF__Group_2__1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1986:1: rule__WMLPreprocIF__Group_2__1 : rule__WMLPreprocIF__Group_2__1__Impl ;
    public final void rule__WMLPreprocIF__Group_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1990:1: ( rule__WMLPreprocIF__Group_2__1__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1991:2: rule__WMLPreprocIF__Group_2__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__1__Impl_in_rule__WMLPreprocIF__Group_2__14103);
            rule__WMLPreprocIF__Group_2__1__Impl();

            state._fsp--;


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__Group_2__1"


    // $ANTLR start "rule__WMLPreprocIF__Group_2__1__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1997:1: rule__WMLPreprocIF__Group_2__1__Impl : ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) ) ;
    public final void rule__WMLPreprocIF__Group_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2001:1: ( ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2002:1: ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2002:1: ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2003:1: ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2003:1: ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2004:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2005:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2005:2: rule__WMLPreprocIF__ElseExpressionsAssignment_2_1
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4132);
            rule__WMLPreprocIF__ElseExpressionsAssignment_2_1();

            state._fsp--;


            }

             after(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 

            }

            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2008:1: ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2009:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )*
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2010:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )*
            loop26:
            do {
                int alt26=2;
                int LA26_0 = input.LA(1);

                if ( ((LA26_0>=RULE_IFDEF && LA26_0<=RULE_DEFINE)||LA26_0==RULE_TEXTDOMAIN||(LA26_0>=20 && LA26_0<=27)||LA26_0==29||LA26_0==33) ) {
                    alt26=1;
                }


                switch (alt26) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2010:2: rule__WMLPreprocIF__ElseExpressionsAssignment_2_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4144);
            	    rule__WMLPreprocIF__ElseExpressionsAssignment_2_1();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    break loop26;
                }
            } while (true);

             after(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 

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
    // $ANTLR end "rule__WMLPreprocIF__Group_2__1__Impl"


    // $ANTLR start "rule__WMLRoot__ExpressionsAssignment"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2026:1: rule__WMLRoot__ExpressionsAssignment : ( ruleWMLRootExpression ) ;
    public final void rule__WMLRoot__ExpressionsAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2030:1: ( ( ruleWMLRootExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2031:1: ( ruleWMLRootExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2031:1: ( ruleWMLRootExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2032:1: ruleWMLRootExpression
            {
             before(grammarAccess.getWMLRootAccess().getExpressionsWMLRootExpressionParserRuleCall_0()); 
            pushFollow(FOLLOW_ruleWMLRootExpression_in_rule__WMLRoot__ExpressionsAssignment4186);
            ruleWMLRootExpression();

            state._fsp--;

             after(grammarAccess.getWMLRootAccess().getExpressionsWMLRootExpressionParserRuleCall_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLRoot__ExpressionsAssignment"


    // $ANTLR start "rule__WMLTag__PlusAssignment_1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2041:1: rule__WMLTag__PlusAssignment_1 : ( ( '+' ) ) ;
    public final void rule__WMLTag__PlusAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2045:1: ( ( ( '+' ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2046:1: ( ( '+' ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2046:1: ( ( '+' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2047:1: ( '+' )
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2048:1: ( '+' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2049:1: '+'
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            match(input,31,FOLLOW_31_in_rule__WMLTag__PlusAssignment_14222); 
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
    // $ANTLR end "rule__WMLTag__PlusAssignment_1"


    // $ANTLR start "rule__WMLTag__NameAssignment_2"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2064:1: rule__WMLTag__NameAssignment_2 : ( RULE_ID ) ;
    public final void rule__WMLTag__NameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2068:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2069:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2069:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2070:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_2_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_24261); 
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
    // $ANTLR end "rule__WMLTag__NameAssignment_2"


    // $ANTLR start "rule__WMLTag__ExpressionsAssignment_4"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2079:1: rule__WMLTag__ExpressionsAssignment_4 : ( ruleWMLExpression ) ;
    public final void rule__WMLTag__ExpressionsAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2083:1: ( ( ruleWMLExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2084:1: ( ruleWMLExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2084:1: ( ruleWMLExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2085:1: ruleWMLExpression
            {
             before(grammarAccess.getWMLTagAccess().getExpressionsWMLExpressionParserRuleCall_4_0()); 
            pushFollow(FOLLOW_ruleWMLExpression_in_rule__WMLTag__ExpressionsAssignment_44292);
            ruleWMLExpression();

            state._fsp--;

             after(grammarAccess.getWMLTagAccess().getExpressionsWMLExpressionParserRuleCall_4_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLTag__ExpressionsAssignment_4"


    // $ANTLR start "rule__WMLTag__EndNameAssignment_6"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2094:1: rule__WMLTag__EndNameAssignment_6 : ( RULE_ID ) ;
    public final void rule__WMLTag__EndNameAssignment_6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2098:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2099:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2099:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2100:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_64323); 
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
    // $ANTLR end "rule__WMLTag__EndNameAssignment_6"


    // $ANTLR start "rule__WMLKey__NameAssignment_0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2109:1: rule__WMLKey__NameAssignment_0 : ( RULE_ID ) ;
    public final void rule__WMLKey__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2113:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2114:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2114:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2115:1: RULE_ID
            {
             before(grammarAccess.getWMLKeyAccess().getNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_04354); 
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
    // $ANTLR end "rule__WMLKey__NameAssignment_0"


    // $ANTLR start "rule__WMLKey__ValuesAssignment_2"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2124:1: rule__WMLKey__ValuesAssignment_2 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValuesAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2128:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2129:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2129:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2130:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValuesWMLKeyValueParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValuesAssignment_24385);
            ruleWMLKeyValue();

            state._fsp--;

             after(grammarAccess.getWMLKeyAccess().getValuesWMLKeyValueParserRuleCall_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__ValuesAssignment_2"


    // $ANTLR start "rule__WMLKey__ValuesAssignment_3_3"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2139:1: rule__WMLKey__ValuesAssignment_3_3 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValuesAssignment_3_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2143:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2144:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2144:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2145:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValuesWMLKeyValueParserRuleCall_3_3_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValuesAssignment_3_34416);
            ruleWMLKeyValue();

            state._fsp--;

             after(grammarAccess.getWMLKeyAccess().getValuesWMLKeyValueParserRuleCall_3_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__ValuesAssignment_3_3"


    // $ANTLR start "rule__WMLKey__EolAssignment_4"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2154:1: rule__WMLKey__EolAssignment_4 : ( ( rule__WMLKey__EolAlternatives_4_0 ) ) ;
    public final void rule__WMLKey__EolAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2158:1: ( ( ( rule__WMLKey__EolAlternatives_4_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2159:1: ( ( rule__WMLKey__EolAlternatives_4_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2159:1: ( ( rule__WMLKey__EolAlternatives_4_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2160:1: ( rule__WMLKey__EolAlternatives_4_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getEolAlternatives_4_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2161:1: ( rule__WMLKey__EolAlternatives_4_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2161:2: rule__WMLKey__EolAlternatives_4_0
            {
            pushFollow(FOLLOW_rule__WMLKey__EolAlternatives_4_0_in_rule__WMLKey__EolAssignment_44447);
            rule__WMLKey__EolAlternatives_4_0();

            state._fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getEolAlternatives_4_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKey__EolAssignment_4"


    // $ANTLR start "rule__WMLMacroCall__PointAssignment_1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2170:1: rule__WMLMacroCall__PointAssignment_1 : ( ( './' ) ) ;
    public final void rule__WMLMacroCall__PointAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2174:1: ( ( ( './' ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2175:1: ( ( './' ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2175:1: ( ( './' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2176:1: ( './' )
            {
             before(grammarAccess.getWMLMacroCallAccess().getPointFullStopSolidusKeyword_1_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2177:1: ( './' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2178:1: './'
            {
             before(grammarAccess.getWMLMacroCallAccess().getPointFullStopSolidusKeyword_1_0()); 
            match(input,23,FOLLOW_23_in_rule__WMLMacroCall__PointAssignment_14485); 
             after(grammarAccess.getWMLMacroCallAccess().getPointFullStopSolidusKeyword_1_0()); 

            }

             after(grammarAccess.getWMLMacroCallAccess().getPointFullStopSolidusKeyword_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroCall__PointAssignment_1"


    // $ANTLR start "rule__WMLMacroCall__RelativeAssignment_2"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2193:1: rule__WMLMacroCall__RelativeAssignment_2 : ( ( '~' ) ) ;
    public final void rule__WMLMacroCall__RelativeAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2197:1: ( ( ( '~' ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2198:1: ( ( '~' ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2198:1: ( ( '~' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2199:1: ( '~' )
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2200:1: ( '~' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2201:1: '~'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0()); 
            match(input,21,FOLLOW_21_in_rule__WMLMacroCall__RelativeAssignment_24529); 
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
    // $ANTLR end "rule__WMLMacroCall__RelativeAssignment_2"


    // $ANTLR start "rule__WMLMacroCall__NameAssignment_3"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2216:1: rule__WMLMacroCall__NameAssignment_3 : ( RULE_ID ) ;
    public final void rule__WMLMacroCall__NameAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2220:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2221:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2221:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2222:1: RULE_ID
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_3_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_34568); 
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
    // $ANTLR end "rule__WMLMacroCall__NameAssignment_3"


    // $ANTLR start "rule__WMLMacroCall__ParametersAssignment_4"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2231:1: rule__WMLMacroCall__ParametersAssignment_4 : ( ruleWMLMacroCallParameter ) ;
    public final void rule__WMLMacroCall__ParametersAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2235:1: ( ( ruleWMLMacroCallParameter ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2236:1: ( ruleWMLMacroCallParameter )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2236:1: ( ruleWMLMacroCallParameter )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2237:1: ruleWMLMacroCallParameter
            {
             before(grammarAccess.getWMLMacroCallAccess().getParametersWMLMacroCallParameterParserRuleCall_4_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_rule__WMLMacroCall__ParametersAssignment_44599);
            ruleWMLMacroCallParameter();

            state._fsp--;

             after(grammarAccess.getWMLMacroCallAccess().getParametersWMLMacroCallParameterParserRuleCall_4_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroCall__ParametersAssignment_4"


    // $ANTLR start "rule__WMLArrayCall__ValueAssignment_1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2246:1: rule__WMLArrayCall__ValueAssignment_1 : ( ruleWMLValue ) ;
    public final void rule__WMLArrayCall__ValueAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2250:1: ( ( ruleWMLValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2251:1: ( ruleWMLValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2251:1: ( ruleWMLValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2252:1: ruleWMLValue
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_14630);
            ruleWMLValue();

            state._fsp--;

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
    // $ANTLR end "rule__WMLArrayCall__ValueAssignment_1"


    // $ANTLR start "rule__WMLMacroDefine__NameAssignment_0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2261:1: rule__WMLMacroDefine__NameAssignment_0 : ( RULE_DEFINE ) ;
    public final void rule__WMLMacroDefine__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2265:1: ( ( RULE_DEFINE ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2266:1: ( RULE_DEFINE )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2266:1: ( RULE_DEFINE )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2267:1: RULE_DEFINE
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0_0()); 
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment_04661); 
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
    // $ANTLR end "rule__WMLMacroDefine__NameAssignment_0"


    // $ANTLR start "rule__WMLMacroDefine__ExpressionsAssignment_1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2276:1: rule__WMLMacroDefine__ExpressionsAssignment_1 : ( ruleWMLValuedExpression ) ;
    public final void rule__WMLMacroDefine__ExpressionsAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2280:1: ( ( ruleWMLValuedExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2281:1: ( ruleWMLValuedExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2281:1: ( ruleWMLValuedExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2282:1: ruleWMLValuedExpression
            {
             before(grammarAccess.getWMLMacroDefineAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_rule__WMLMacroDefine__ExpressionsAssignment_14692);
            ruleWMLValuedExpression();

            state._fsp--;

             after(grammarAccess.getWMLMacroDefineAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroDefine__ExpressionsAssignment_1"


    // $ANTLR start "rule__WMLMacroDefine__EndNameAssignment_2"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2291:1: rule__WMLMacroDefine__EndNameAssignment_2 : ( RULE_ENDDEF ) ;
    public final void rule__WMLMacroDefine__EndNameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2295:1: ( ( RULE_ENDDEF ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2296:1: ( RULE_ENDDEF )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2296:1: ( RULE_ENDDEF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2297:1: RULE_ENDDEF
            {
             before(grammarAccess.getWMLMacroDefineAccess().getEndNameENDDEFTerminalRuleCall_2_0()); 
            match(input,RULE_ENDDEF,FOLLOW_RULE_ENDDEF_in_rule__WMLMacroDefine__EndNameAssignment_24723); 
             after(grammarAccess.getWMLMacroDefineAccess().getEndNameENDDEFTerminalRuleCall_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroDefine__EndNameAssignment_2"


    // $ANTLR start "rule__WMLPreprocIF__NameAssignment_0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2306:1: rule__WMLPreprocIF__NameAssignment_0 : ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) ) ;
    public final void rule__WMLPreprocIF__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2310:1: ( ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2311:1: ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2311:1: ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2312:1: ( rule__WMLPreprocIF__NameAlternatives_0_0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getNameAlternatives_0_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2313:1: ( rule__WMLPreprocIF__NameAlternatives_0_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2313:2: rule__WMLPreprocIF__NameAlternatives_0_0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__NameAlternatives_0_0_in_rule__WMLPreprocIF__NameAssignment_04754);
            rule__WMLPreprocIF__NameAlternatives_0_0();

            state._fsp--;


            }

             after(grammarAccess.getWMLPreprocIFAccess().getNameAlternatives_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__NameAssignment_0"


    // $ANTLR start "rule__WMLPreprocIF__ExpressionsAssignment_1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2322:1: rule__WMLPreprocIF__ExpressionsAssignment_1 : ( ruleWMLValuedExpression ) ;
    public final void rule__WMLPreprocIF__ExpressionsAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2326:1: ( ( ruleWMLValuedExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2327:1: ( ruleWMLValuedExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2327:1: ( ruleWMLValuedExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2328:1: ruleWMLValuedExpression
            {
             before(grammarAccess.getWMLPreprocIFAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ExpressionsAssignment_14787);
            ruleWMLValuedExpression();

            state._fsp--;

             after(grammarAccess.getWMLPreprocIFAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__ExpressionsAssignment_1"


    // $ANTLR start "rule__WMLPreprocIF__ElsesAssignment_2_0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2337:1: rule__WMLPreprocIF__ElsesAssignment_2_0 : ( RULE_ELSE ) ;
    public final void rule__WMLPreprocIF__ElsesAssignment_2_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2341:1: ( ( RULE_ELSE ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2342:1: ( RULE_ELSE )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2342:1: ( RULE_ELSE )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2343:1: RULE_ELSE
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElsesELSETerminalRuleCall_2_0_0()); 
            match(input,RULE_ELSE,FOLLOW_RULE_ELSE_in_rule__WMLPreprocIF__ElsesAssignment_2_04818); 
             after(grammarAccess.getWMLPreprocIFAccess().getElsesELSETerminalRuleCall_2_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__ElsesAssignment_2_0"


    // $ANTLR start "rule__WMLPreprocIF__ElseExpressionsAssignment_2_1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2352:1: rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 : ( ruleWMLValuedExpression ) ;
    public final void rule__WMLPreprocIF__ElseExpressionsAssignment_2_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2356:1: ( ( ruleWMLValuedExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2357:1: ( ruleWMLValuedExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2357:1: ( ruleWMLValuedExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2358:1: ruleWMLValuedExpression
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsWMLValuedExpressionParserRuleCall_2_1_0()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ElseExpressionsAssignment_2_14849);
            ruleWMLValuedExpression();

            state._fsp--;

             after(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsWMLValuedExpressionParserRuleCall_2_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__ElseExpressionsAssignment_2_1"


    // $ANTLR start "rule__WMLPreprocIF__EndNameAssignment_3"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2367:1: rule__WMLPreprocIF__EndNameAssignment_3 : ( RULE_ENDIF ) ;
    public final void rule__WMLPreprocIF__EndNameAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2371:1: ( ( RULE_ENDIF ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2372:1: ( RULE_ENDIF )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2372:1: ( RULE_ENDIF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2373:1: RULE_ENDIF
            {
             before(grammarAccess.getWMLPreprocIFAccess().getEndNameENDIFTerminalRuleCall_3_0()); 
            match(input,RULE_ENDIF,FOLLOW_RULE_ENDIF_in_rule__WMLPreprocIF__EndNameAssignment_34880); 
             after(grammarAccess.getWMLPreprocIFAccess().getEndNameENDIFTerminalRuleCall_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLPreprocIF__EndNameAssignment_3"


    // $ANTLR start "rule__WMLTextdomain__NameAssignment"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2382:1: rule__WMLTextdomain__NameAssignment : ( RULE_TEXTDOMAIN ) ;
    public final void rule__WMLTextdomain__NameAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2386:1: ( ( RULE_TEXTDOMAIN ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2387:1: ( RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2387:1: ( RULE_TEXTDOMAIN )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2388:1: RULE_TEXTDOMAIN
            {
             before(grammarAccess.getWMLTextdomainAccess().getNameTEXTDOMAINTerminalRuleCall_0()); 
            match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment4911); 
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
    // $ANTLR end "rule__WMLTextdomain__NameAssignment"


    // $ANTLR start "rule__WMLLuaCode__ValueAssignment"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2397:1: rule__WMLLuaCode__ValueAssignment : ( RULE_LUA_CODE ) ;
    public final void rule__WMLLuaCode__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2401:1: ( ( RULE_LUA_CODE ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2402:1: ( RULE_LUA_CODE )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2402:1: ( RULE_LUA_CODE )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2403:1: RULE_LUA_CODE
            {
             before(grammarAccess.getWMLLuaCodeAccess().getValueLUA_CODETerminalRuleCall_0()); 
            match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment4942); 
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
    // $ANTLR end "rule__WMLLuaCode__ValueAssignment"


    // $ANTLR start "rule__WMLValue__ValueAssignment"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2412:1: rule__WMLValue__ValueAssignment : ( ( rule__WMLValue__ValueAlternatives_0 ) ) ;
    public final void rule__WMLValue__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2416:1: ( ( ( rule__WMLValue__ValueAlternatives_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2417:1: ( ( rule__WMLValue__ValueAlternatives_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2417:1: ( ( rule__WMLValue__ValueAlternatives_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2418:1: ( rule__WMLValue__ValueAlternatives_0 )
            {
             before(grammarAccess.getWMLValueAccess().getValueAlternatives_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2419:1: ( rule__WMLValue__ValueAlternatives_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2419:2: rule__WMLValue__ValueAlternatives_0
            {
            pushFollow(FOLLOW_rule__WMLValue__ValueAlternatives_0_in_rule__WMLValue__ValueAssignment4973);
            rule__WMLValue__ValueAlternatives_0();

            state._fsp--;


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
    // $ANTLR end "rule__WMLValue__ValueAssignment"


    // $ANTLR start "rule__MacroTokens__ValueAssignment"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2428:1: rule__MacroTokens__ValueAssignment : ( ( rule__MacroTokens__ValueAlternatives_0 ) ) ;
    public final void rule__MacroTokens__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2432:1: ( ( ( rule__MacroTokens__ValueAlternatives_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2433:1: ( ( rule__MacroTokens__ValueAlternatives_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2433:1: ( ( rule__MacroTokens__ValueAlternatives_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2434:1: ( rule__MacroTokens__ValueAlternatives_0 )
            {
             before(grammarAccess.getMacroTokensAccess().getValueAlternatives_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2435:1: ( rule__MacroTokens__ValueAlternatives_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2435:2: rule__MacroTokens__ValueAlternatives_0
            {
            pushFollow(FOLLOW_rule__MacroTokens__ValueAlternatives_0_in_rule__MacroTokens__ValueAssignment5006);
            rule__MacroTokens__ValueAlternatives_0();

            state._fsp--;


            }

             after(grammarAccess.getMacroTokensAccess().getValueAlternatives_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__MacroTokens__ValueAssignment"

    // Delegated rules


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot61 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot68 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__ExpressionsAssignment_in_ruleWMLRoot94 = new BitSet(new long[]{0x00000002200223C2L});
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
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_entryRuleWMLMacroCallParameter371 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCallParameter378 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Alternatives_in_ruleWMLMacroCallParameter404 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall431 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLArrayCall438 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__0_in_ruleWMLArrayCall464 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine491 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine498 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__0_in_ruleWMLMacroDefine524 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPreprocIF_in_entryRuleWMLPreprocIF551 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLPreprocIF558 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__0_in_ruleWMLPreprocIF584 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_entryRuleWMLRootExpression611 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRootExpression618 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRootExpression__Alternatives_in_ruleWMLRootExpression644 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_entryRuleWMLExpression671 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLExpression678 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLExpression__Alternatives_in_ruleWMLExpression704 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_entryRuleWMLValuedExpression731 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValuedExpression738 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValuedExpression__Alternatives_in_ruleWMLValuedExpression764 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain791 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTextdomain798 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTextdomain__NameAssignment_in_ruleWMLTextdomain824 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode851 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLLuaCode858 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLLuaCode__ValueAssignment_in_ruleWMLLuaCode884 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter911 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroParameter918 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroParameter__Alternatives_in_ruleWMLMacroParameter944 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_entryRuleWMLValue971 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValue978 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValue__ValueAssignment_in_ruleWMLValue1004 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens1031 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleMacroTokens1038 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__MacroTokens__ValueAssignment_in_ruleMacroTokens1064 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__EolAlternatives_4_01100 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_SL_COMMENT_in_rule__WMLKey__EolAlternatives_4_01117 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLKeyValue__Alternatives1149 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives1166 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_rule__WMLKeyValue__Alternatives1183 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_rule__WMLKeyValue__Alternatives1200 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_rule__WMLMacroCallParameter__Alternatives1232 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCallParameter__Alternatives1249 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFDEF_in_rule__WMLPreprocIF__NameAlternatives_0_01281 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFNDEF_in_rule__WMLPreprocIF__NameAlternatives_0_01298 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFHAVE_in_rule__WMLPreprocIF__NameAlternatives_0_01315 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFNHAVE_in_rule__WMLPreprocIF__NameAlternatives_0_01332 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRootExpression__Alternatives1364 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLRootExpression__Alternatives1381 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLRootExpression__Alternatives1398 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_rule__WMLRootExpression__Alternatives1415 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPreprocIF_in_rule__WMLRootExpression__Alternatives1432 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_rule__WMLExpression__Alternatives1464 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLExpression__Alternatives1481 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_rule__WMLValuedExpression__Alternatives1513 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLValuedExpression__Alternatives1530 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLMacroParameter__Alternatives1562 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_rule__WMLMacroParameter__Alternatives1579 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLValue__ValueAlternatives_01611 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLValue__ValueAlternatives_01628 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__WMLValue__ValueAlternatives_01646 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLValue__ValueAlternatives_01666 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__WMLValue__ValueAlternatives_01686 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__WMLValue__ValueAlternatives_01706 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_rule__WMLValue__ValueAlternatives_01726 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__WMLValue__ValueAlternatives_01746 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_rule__WMLValue__ValueAlternatives_01766 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rule__WMLValue__ValueAlternatives_01786 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__ValueAlternatives_01805 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__MacroTokens__ValueAlternatives_01838 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__MacroTokens__ValueAlternatives_01858 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__MacroTokens__ValueAlternatives_01878 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__MacroTokens__ValueAlternatives_01898 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__MacroTokens__ValueAlternatives_01918 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01950 = new BitSet(new long[]{0x0000000080000400L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01953 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__WMLTag__Group__0__Impl1981 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__12012 = new BitSet(new long[]{0x0000000080000400L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__12015 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl2042 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__22073 = new BitSet(new long[]{0x0000000040000000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__22076 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl2103 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__32133 = new BitSet(new long[]{0x00000003200227C0L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__32136 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__WMLTag__Group__3__Impl2164 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__42195 = new BitSet(new long[]{0x00000003200227C0L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__42198 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__ExpressionsAssignment_4_in_rule__WMLTag__Group__4__Impl2225 = new BitSet(new long[]{0x00000002200227C2L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__52256 = new BitSet(new long[]{0x0000000000000400L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__52259 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__WMLTag__Group__5__Impl2287 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__62318 = new BitSet(new long[]{0x0000000040000000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__62321 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl2348 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__72378 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__WMLTag__Group__7__Impl2406 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02453 = new BitSet(new long[]{0x0000000010000000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02456 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl2483 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12513 = new BitSet(new long[]{0x00000002AFF41C30L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12516 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__WMLKey__Group__1__Impl2544 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22575 = new BitSet(new long[]{0x00000002AFF41C30L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__22578 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValuesAssignment_2_in_rule__WMLKey__Group__2__Impl2605 = new BitSet(new long[]{0x000000022FF41C02L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__32636 = new BitSet(new long[]{0x00000002AFF41C30L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__4_in_rule__WMLKey__Group__32639 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__0_in_rule__WMLKey__Group__3__Impl2666 = new BitSet(new long[]{0x0000000080000012L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__4__Impl_in_rule__WMLKey__Group__42697 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__EolAssignment_4_in_rule__WMLKey__Group__4__Impl2724 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__0__Impl_in_rule__WMLKey__Group_3__02764 = new BitSet(new long[]{0x0000000080000010L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__1_in_rule__WMLKey__Group_3__02767 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__0__Impl2795 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__1__Impl_in_rule__WMLKey__Group_3__12826 = new BitSet(new long[]{0x000000022FF41C10L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__2_in_rule__WMLKey__Group_3__12829 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__WMLKey__Group_3__1__Impl2857 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__2__Impl_in_rule__WMLKey__Group_3__22888 = new BitSet(new long[]{0x000000022FF41C10L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__3_in_rule__WMLKey__Group_3__22891 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__2__Impl2919 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__3__Impl_in_rule__WMLKey__Group_3__32950 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValuesAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2979 = new BitSet(new long[]{0x000000022FF41C12L});
    public static final BitSet FOLLOW_rule__WMLKey__ValuesAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2991 = new BitSet(new long[]{0x000000022FF41C12L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__03032 = new BitSet(new long[]{0x0000000000A00400L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__03035 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_33_in_rule__WMLMacroCall__Group__0__Impl3063 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__13094 = new BitSet(new long[]{0x0000000000A00400L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__13097 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__PointAssignment_1_in_rule__WMLMacroCall__Group__1__Impl3124 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__23155 = new BitSet(new long[]{0x0000000000A00400L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__23158 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__RelativeAssignment_2_in_rule__WMLMacroCall__Group__2__Impl3185 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__33216 = new BitSet(new long[]{0x00000007FFF01C00L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__33219 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__NameAssignment_3_in_rule__WMLMacroCall__Group__3__Impl3246 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__43276 = new BitSet(new long[]{0x00000007FFF01C00L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__5_in_rule__WMLMacroCall__Group__43279 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ParametersAssignment_4_in_rule__WMLMacroCall__Group__4__Impl3306 = new BitSet(new long[]{0x00000003FFF01C02L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__5__Impl_in_rule__WMLMacroCall__Group__53337 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_34_in_rule__WMLMacroCall__Group__5__Impl3365 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__03408 = new BitSet(new long[]{0x000000000FF01C00L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__03411 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__WMLArrayCall__Group__0__Impl3439 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__13470 = new BitSet(new long[]{0x0000000040000000L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__13473 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3502 = new BitSet(new long[]{0x000000000FF01C02L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3514 = new BitSet(new long[]{0x000000000FF01C02L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__23547 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__WMLArrayCall__Group__2__Impl3575 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__0__Impl_in_rule__WMLMacroDefine__Group__03612 = new BitSet(new long[]{0x000000022FF27FC0L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__1_in_rule__WMLMacroDefine__Group__03615 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__NameAssignment_0_in_rule__WMLMacroDefine__Group__0__Impl3642 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__1__Impl_in_rule__WMLMacroDefine__Group__13672 = new BitSet(new long[]{0x000000022FF27FC0L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__2_in_rule__WMLMacroDefine__Group__13675 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__ExpressionsAssignment_1_in_rule__WMLMacroDefine__Group__1__Impl3702 = new BitSet(new long[]{0x000000022FF23FC2L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__2__Impl_in_rule__WMLMacroDefine__Group__23733 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__EndNameAssignment_2_in_rule__WMLMacroDefine__Group__2__Impl3760 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__0__Impl_in_rule__WMLPreprocIF__Group__03796 = new BitSet(new long[]{0x000000022FF3BFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__1_in_rule__WMLPreprocIF__Group__03799 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__NameAssignment_0_in_rule__WMLPreprocIF__Group__0__Impl3826 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__1__Impl_in_rule__WMLPreprocIF__Group__13856 = new BitSet(new long[]{0x000000022FF3BFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__2_in_rule__WMLPreprocIF__Group__13859 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ExpressionsAssignment_1_in_rule__WMLPreprocIF__Group__1__Impl3886 = new BitSet(new long[]{0x000000022FF23FC2L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__2__Impl_in_rule__WMLPreprocIF__Group__23917 = new BitSet(new long[]{0x000000022FF3BFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__3_in_rule__WMLPreprocIF__Group__23920 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__0_in_rule__WMLPreprocIF__Group__2__Impl3947 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__3__Impl_in_rule__WMLPreprocIF__Group__33978 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__EndNameAssignment_3_in_rule__WMLPreprocIF__Group__3__Impl4005 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__0__Impl_in_rule__WMLPreprocIF__Group_2__04043 = new BitSet(new long[]{0x000000022FF23FC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__1_in_rule__WMLPreprocIF__Group_2__04046 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ElsesAssignment_2_0_in_rule__WMLPreprocIF__Group_2__0__Impl4073 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__1__Impl_in_rule__WMLPreprocIF__Group_2__14103 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4132 = new BitSet(new long[]{0x000000022FF23FC2L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4144 = new BitSet(new long[]{0x000000022FF23FC2L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_rule__WMLRoot__ExpressionsAssignment4186 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__WMLTag__PlusAssignment_14222 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_24261 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_rule__WMLTag__ExpressionsAssignment_44292 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_64323 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_04354 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValuesAssignment_24385 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValuesAssignment_3_34416 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__EolAlternatives_4_0_in_rule__WMLKey__EolAssignment_44447 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__WMLMacroCall__PointAssignment_14485 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLMacroCall__RelativeAssignment_24529 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_34568 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_rule__WMLMacroCall__ParametersAssignment_44599 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_14630 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment_04661 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_rule__WMLMacroDefine__ExpressionsAssignment_14692 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ENDDEF_in_rule__WMLMacroDefine__EndNameAssignment_24723 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__NameAlternatives_0_0_in_rule__WMLPreprocIF__NameAssignment_04754 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ExpressionsAssignment_14787 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ELSE_in_rule__WMLPreprocIF__ElsesAssignment_2_04818 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ElseExpressionsAssignment_2_14849 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ENDIF_in_rule__WMLPreprocIF__EndNameAssignment_34880 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment4911 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment4942 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValue__ValueAlternatives_0_in_rule__WMLValue__ValueAssignment4973 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__MacroTokens__ValueAlternatives_0_in_rule__MacroTokens__ValueAssignment5006 = new BitSet(new long[]{0x0000000000000002L});

}