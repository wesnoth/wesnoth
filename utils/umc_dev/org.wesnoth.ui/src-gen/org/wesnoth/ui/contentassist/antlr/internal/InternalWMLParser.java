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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:498:1: ruleWMLValue : ( ( rule__WMLValue__Alternatives ) ) ;
    public final void ruleWMLValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:502:2: ( ( ( rule__WMLValue__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:503:1: ( ( rule__WMLValue__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:503:1: ( ( rule__WMLValue__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:504:1: ( rule__WMLValue__Alternatives )
            {
             before(grammarAccess.getWMLValueAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:505:1: ( rule__WMLValue__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:505:2: rule__WMLValue__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLValue__Alternatives_in_ruleWMLValue1004);
            rule__WMLValue__Alternatives();

            state._fsp--;


            }

             after(grammarAccess.getWMLValueAccess().getAlternatives()); 

            }


            }

        }
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:526:1: ruleMacroTokens : ( ( rule__MacroTokens__Alternatives ) ) ;
    public final void ruleMacroTokens() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:530:2: ( ( ( rule__MacroTokens__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:531:1: ( ( rule__MacroTokens__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:531:1: ( ( rule__MacroTokens__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:532:1: ( rule__MacroTokens__Alternatives )
            {
             before(grammarAccess.getMacroTokensAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:1: ( rule__MacroTokens__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:2: rule__MacroTokens__Alternatives
            {
            pushFollow(FOLLOW_rule__MacroTokens__Alternatives_in_ruleMacroTokens1064);
            rule__MacroTokens__Alternatives();

            state._fsp--;


            }

             after(grammarAccess.getMacroTokensAccess().getAlternatives()); 

            }


            }

        }
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:567:1: rule__WMLKeyValue__Alternatives : ( ( ( rule__WMLKeyValue__Group_0__0 ) ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) );
    public final void rule__WMLKeyValue__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:571:1: ( ( ( rule__WMLKeyValue__Group_0__0 ) ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) )
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:572:1: ( ( rule__WMLKeyValue__Group_0__0 ) )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:572:1: ( ( rule__WMLKeyValue__Group_0__0 ) )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:573:1: ( rule__WMLKeyValue__Group_0__0 )
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getGroup_0()); 
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:574:1: ( rule__WMLKeyValue__Group_0__0 )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:574:2: rule__WMLKeyValue__Group_0__0
                    {
                    pushFollow(FOLLOW_rule__WMLKeyValue__Group_0__0_in_rule__WMLKeyValue__Alternatives1149);
                    rule__WMLKeyValue__Group_0__0();

                    state._fsp--;


                    }

                     after(grammarAccess.getWMLKeyValueAccess().getGroup_0()); 

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
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives1167);
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
                    pushFollow(FOLLOW_ruleWMLLuaCode_in_rule__WMLKeyValue__Alternatives1184);
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
                    pushFollow(FOLLOW_ruleWMLArrayCall_in_rule__WMLKeyValue__Alternatives1201);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:601:1: rule__WMLMacroCallParameter__Alternatives : ( ( ( rule__WMLMacroCallParameter__Group_0__0 ) ) | ( ruleWMLMacroCall ) );
    public final void rule__WMLMacroCallParameter__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:605:1: ( ( ( rule__WMLMacroCallParameter__Group_0__0 ) ) | ( ruleWMLMacroCall ) )
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:606:1: ( ( rule__WMLMacroCallParameter__Group_0__0 ) )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:606:1: ( ( rule__WMLMacroCallParameter__Group_0__0 ) )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:607:1: ( rule__WMLMacroCallParameter__Group_0__0 )
                    {
                     before(grammarAccess.getWMLMacroCallParameterAccess().getGroup_0()); 
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:608:1: ( rule__WMLMacroCallParameter__Group_0__0 )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:608:2: rule__WMLMacroCallParameter__Group_0__0
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCallParameter__Group_0__0_in_rule__WMLMacroCallParameter__Alternatives1233);
                    rule__WMLMacroCallParameter__Group_0__0();

                    state._fsp--;


                    }

                     after(grammarAccess.getWMLMacroCallParameterAccess().getGroup_0()); 

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
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCallParameter__Alternatives1251);
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
                    match(input,RULE_IFDEF,FOLLOW_RULE_IFDEF_in_rule__WMLPreprocIF__NameAlternatives_0_01283); 
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
                    match(input,RULE_IFNDEF,FOLLOW_RULE_IFNDEF_in_rule__WMLPreprocIF__NameAlternatives_0_01300); 
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
                    match(input,RULE_IFHAVE,FOLLOW_RULE_IFHAVE_in_rule__WMLPreprocIF__NameAlternatives_0_01317); 
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
                    match(input,RULE_IFNHAVE,FOLLOW_RULE_IFNHAVE_in_rule__WMLPreprocIF__NameAlternatives_0_01334); 
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
                    pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRootExpression__Alternatives1366);
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
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLRootExpression__Alternatives1383);
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
                    pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLRootExpression__Alternatives1400);
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
                    pushFollow(FOLLOW_ruleWMLTextdomain_in_rule__WMLRootExpression__Alternatives1417);
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
                    pushFollow(FOLLOW_ruleWMLPreprocIF_in_rule__WMLRootExpression__Alternatives1434);
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
                    pushFollow(FOLLOW_ruleWMLRootExpression_in_rule__WMLExpression__Alternatives1466);
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
                    pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLExpression__Alternatives1483);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:719:1: rule__WMLValuedExpression__Alternatives : ( ( ruleWMLExpression ) | ( ( rule__WMLValuedExpression__Group_1__0 ) ) );
    public final void rule__WMLValuedExpression__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:723:1: ( ( ruleWMLExpression ) | ( ( rule__WMLValuedExpression__Group_1__0 ) ) )
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
                    pushFollow(FOLLOW_ruleWMLExpression_in_rule__WMLValuedExpression__Alternatives1515);
                    ruleWMLExpression();

                    state._fsp--;

                     after(grammarAccess.getWMLValuedExpressionAccess().getWMLExpressionParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:730:6: ( ( rule__WMLValuedExpression__Group_1__0 ) )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:730:6: ( ( rule__WMLValuedExpression__Group_1__0 ) )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:731:1: ( rule__WMLValuedExpression__Group_1__0 )
                    {
                     before(grammarAccess.getWMLValuedExpressionAccess().getGroup_1()); 
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:732:1: ( rule__WMLValuedExpression__Group_1__0 )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:732:2: rule__WMLValuedExpression__Group_1__0
                    {
                    pushFollow(FOLLOW_rule__WMLValuedExpression__Group_1__0_in_rule__WMLValuedExpression__Alternatives1532);
                    rule__WMLValuedExpression__Group_1__0();

                    state._fsp--;


                    }

                     after(grammarAccess.getWMLValuedExpressionAccess().getGroup_1()); 

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
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLMacroParameter__Alternatives1565);
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
                    pushFollow(FOLLOW_ruleMacroTokens_in_rule__WMLMacroParameter__Alternatives1582);
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


    // $ANTLR start "rule__WMLValue__Alternatives"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:763:1: rule__WMLValue__Alternatives : ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( '~' ) | ( '.' ) | ( './' ) | ( '$' ) | ( '/' ) | ( '(' ) | ( ')' ) | ( RULE_ANY_OTHER ) );
    public final void rule__WMLValue__Alternatives() throws RecognitionException {

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
                     before(grammarAccess.getWMLValueAccess().getIDTerminalRuleCall_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLValue__Alternatives1614); 
                     after(grammarAccess.getWMLValueAccess().getIDTerminalRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:774:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:774:6: ( RULE_STRING )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:775:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLValueAccess().getSTRINGTerminalRuleCall_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLValue__Alternatives1631); 
                     after(grammarAccess.getWMLValueAccess().getSTRINGTerminalRuleCall_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:780:6: ( '_' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:780:6: ( '_' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:781:1: '_'
                    {
                     before(grammarAccess.getWMLValueAccess().get_Keyword_2()); 
                    match(input,20,FOLLOW_20_in_rule__WMLValue__Alternatives1649); 
                     after(grammarAccess.getWMLValueAccess().get_Keyword_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:788:6: ( '~' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:788:6: ( '~' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:789:1: '~'
                    {
                     before(grammarAccess.getWMLValueAccess().getTildeKeyword_3()); 
                    match(input,21,FOLLOW_21_in_rule__WMLValue__Alternatives1669); 
                     after(grammarAccess.getWMLValueAccess().getTildeKeyword_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:796:6: ( '.' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:796:6: ( '.' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:797:1: '.'
                    {
                     before(grammarAccess.getWMLValueAccess().getFullStopKeyword_4()); 
                    match(input,22,FOLLOW_22_in_rule__WMLValue__Alternatives1689); 
                     after(grammarAccess.getWMLValueAccess().getFullStopKeyword_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:804:6: ( './' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:804:6: ( './' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:805:1: './'
                    {
                     before(grammarAccess.getWMLValueAccess().getFullStopSolidusKeyword_5()); 
                    match(input,23,FOLLOW_23_in_rule__WMLValue__Alternatives1709); 
                     after(grammarAccess.getWMLValueAccess().getFullStopSolidusKeyword_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:812:6: ( '$' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:812:6: ( '$' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:813:1: '$'
                    {
                     before(grammarAccess.getWMLValueAccess().getDollarSignKeyword_6()); 
                    match(input,24,FOLLOW_24_in_rule__WMLValue__Alternatives1729); 
                     after(grammarAccess.getWMLValueAccess().getDollarSignKeyword_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:820:6: ( '/' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:820:6: ( '/' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:821:1: '/'
                    {
                     before(grammarAccess.getWMLValueAccess().getSolidusKeyword_7()); 
                    match(input,25,FOLLOW_25_in_rule__WMLValue__Alternatives1749); 
                     after(grammarAccess.getWMLValueAccess().getSolidusKeyword_7()); 

                    }


                    }
                    break;
                case 9 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:828:6: ( '(' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:828:6: ( '(' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:829:1: '('
                    {
                     before(grammarAccess.getWMLValueAccess().getLeftParenthesisKeyword_8()); 
                    match(input,26,FOLLOW_26_in_rule__WMLValue__Alternatives1769); 
                     after(grammarAccess.getWMLValueAccess().getLeftParenthesisKeyword_8()); 

                    }


                    }
                    break;
                case 10 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:836:6: ( ')' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:836:6: ( ')' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:837:1: ')'
                    {
                     before(grammarAccess.getWMLValueAccess().getRightParenthesisKeyword_9()); 
                    match(input,27,FOLLOW_27_in_rule__WMLValue__Alternatives1789); 
                     after(grammarAccess.getWMLValueAccess().getRightParenthesisKeyword_9()); 

                    }


                    }
                    break;
                case 11 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:844:6: ( RULE_ANY_OTHER )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:844:6: ( RULE_ANY_OTHER )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:845:1: RULE_ANY_OTHER
                    {
                     before(grammarAccess.getWMLValueAccess().getANY_OTHERTerminalRuleCall_10()); 
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__Alternatives1808); 
                     after(grammarAccess.getWMLValueAccess().getANY_OTHERTerminalRuleCall_10()); 

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
    // $ANTLR end "rule__WMLValue__Alternatives"


    // $ANTLR start "rule__MacroTokens__Alternatives"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:855:1: rule__MacroTokens__Alternatives : ( ( '=' ) | ( '[' ) | ( ']' ) | ( '+' ) | ( '[/' ) );
    public final void rule__MacroTokens__Alternatives() throws RecognitionException {

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
                     before(grammarAccess.getMacroTokensAccess().getEqualsSignKeyword_0()); 
                    match(input,28,FOLLOW_28_in_rule__MacroTokens__Alternatives1841); 
                     after(grammarAccess.getMacroTokensAccess().getEqualsSignKeyword_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:868:6: ( '[' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:868:6: ( '[' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:869:1: '['
                    {
                     before(grammarAccess.getMacroTokensAccess().getLeftSquareBracketKeyword_1()); 
                    match(input,29,FOLLOW_29_in_rule__MacroTokens__Alternatives1861); 
                     after(grammarAccess.getMacroTokensAccess().getLeftSquareBracketKeyword_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:876:6: ( ']' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:876:6: ( ']' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:877:1: ']'
                    {
                     before(grammarAccess.getMacroTokensAccess().getRightSquareBracketKeyword_2()); 
                    match(input,30,FOLLOW_30_in_rule__MacroTokens__Alternatives1881); 
                     after(grammarAccess.getMacroTokensAccess().getRightSquareBracketKeyword_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:884:6: ( '+' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:884:6: ( '+' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:885:1: '+'
                    {
                     before(grammarAccess.getMacroTokensAccess().getPlusSignKeyword_3()); 
                    match(input,31,FOLLOW_31_in_rule__MacroTokens__Alternatives1901); 
                     after(grammarAccess.getMacroTokensAccess().getPlusSignKeyword_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:892:6: ( '[/' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:892:6: ( '[/' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:893:1: '[/'
                    {
                     before(grammarAccess.getMacroTokensAccess().getLeftSquareBracketSolidusKeyword_4()); 
                    match(input,32,FOLLOW_32_in_rule__MacroTokens__Alternatives1921); 
                     after(grammarAccess.getMacroTokensAccess().getLeftSquareBracketSolidusKeyword_4()); 

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
    // $ANTLR end "rule__MacroTokens__Alternatives"


    // $ANTLR start "rule__WMLTag__Group__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:907:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:911:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:912:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01953);
            rule__WMLTag__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01956);
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
            match(input,29,FOLLOW_29_in_rule__WMLTag__Group__0__Impl1984); 
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
            pushFollow(FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__12015);
            rule__WMLTag__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__12018);
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
                    pushFollow(FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl2045);
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
            pushFollow(FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__22076);
            rule__WMLTag__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__22079);
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
            pushFollow(FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl2106);
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
            pushFollow(FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__32136);
            rule__WMLTag__Group__3__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__32139);
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
            match(input,30,FOLLOW_30_in_rule__WMLTag__Group__3__Impl2167); 
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
            pushFollow(FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__42198);
            rule__WMLTag__Group__4__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__42201);
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
            	    pushFollow(FOLLOW_rule__WMLTag__ExpressionsAssignment_4_in_rule__WMLTag__Group__4__Impl2228);
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
            pushFollow(FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__52259);
            rule__WMLTag__Group__5__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__52262);
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
            match(input,32,FOLLOW_32_in_rule__WMLTag__Group__5__Impl2290); 
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
            pushFollow(FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__62321);
            rule__WMLTag__Group__6__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__62324);
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
            pushFollow(FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl2351);
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
            pushFollow(FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__72381);
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
            match(input,30,FOLLOW_30_in_rule__WMLTag__Group__7__Impl2409); 
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
            pushFollow(FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02456);
            rule__WMLKey__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02459);
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
            pushFollow(FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl2486);
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
            pushFollow(FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12516);
            rule__WMLKey__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12519);
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
            match(input,28,FOLLOW_28_in_rule__WMLKey__Group__1__Impl2547); 
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
            pushFollow(FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22578);
            rule__WMLKey__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__22581);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1234:1: rule__WMLKey__Group__2__Impl : ( ( rule__WMLKey__ValueAssignment_2 )* ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1238:1: ( ( ( rule__WMLKey__ValueAssignment_2 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1239:1: ( ( rule__WMLKey__ValueAssignment_2 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1239:1: ( ( rule__WMLKey__ValueAssignment_2 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1240:1: ( rule__WMLKey__ValueAssignment_2 )*
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1241:1: ( rule__WMLKey__ValueAssignment_2 )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( ((LA14_0>=RULE_ID && LA14_0<=RULE_ANY_OTHER)||LA14_0==RULE_LUA_CODE||(LA14_0>=20 && LA14_0<=27)||LA14_0==29||LA14_0==33) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1241:2: rule__WMLKey__ValueAssignment_2
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2608);
            	    rule__WMLKey__ValueAssignment_2();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    break loop14;
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
    // $ANTLR end "rule__WMLKey__Group__2__Impl"


    // $ANTLR start "rule__WMLKey__Group__3"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1251:1: rule__WMLKey__Group__3 : rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4 ;
    public final void rule__WMLKey__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1255:1: ( rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1256:2: rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__32639);
            rule__WMLKey__Group__3__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__4_in_rule__WMLKey__Group__32642);
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
            	    pushFollow(FOLLOW_rule__WMLKey__Group_3__0_in_rule__WMLKey__Group__3__Impl2669);
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
            pushFollow(FOLLOW_rule__WMLKey__Group__4__Impl_in_rule__WMLKey__Group__42700);
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
            pushFollow(FOLLOW_rule__WMLKey__EolAssignment_4_in_rule__WMLKey__Group__4__Impl2727);
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
            pushFollow(FOLLOW_rule__WMLKey__Group_3__0__Impl_in_rule__WMLKey__Group_3__02767);
            rule__WMLKey__Group_3__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__1_in_rule__WMLKey__Group_3__02770);
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
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__0__Impl2798); 

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
            pushFollow(FOLLOW_rule__WMLKey__Group_3__1__Impl_in_rule__WMLKey__Group_3__12829);
            rule__WMLKey__Group_3__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__2_in_rule__WMLKey__Group_3__12832);
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
            match(input,31,FOLLOW_31_in_rule__WMLKey__Group_3__1__Impl2860); 
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
            pushFollow(FOLLOW_rule__WMLKey__Group_3__2__Impl_in_rule__WMLKey__Group_3__22891);
            rule__WMLKey__Group_3__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__3_in_rule__WMLKey__Group_3__22894);
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
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__2__Impl2922); 

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
            pushFollow(FOLLOW_rule__WMLKey__Group_3__3__Impl_in_rule__WMLKey__Group_3__32953);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1418:1: rule__WMLKey__Group_3__3__Impl : ( ( ( rule__WMLKey__ValueAssignment_3_3 ) ) ( ( rule__WMLKey__ValueAssignment_3_3 )* ) ) ;
    public final void rule__WMLKey__Group_3__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1422:1: ( ( ( ( rule__WMLKey__ValueAssignment_3_3 ) ) ( ( rule__WMLKey__ValueAssignment_3_3 )* ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1423:1: ( ( ( rule__WMLKey__ValueAssignment_3_3 ) ) ( ( rule__WMLKey__ValueAssignment_3_3 )* ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1423:1: ( ( ( rule__WMLKey__ValueAssignment_3_3 ) ) ( ( rule__WMLKey__ValueAssignment_3_3 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1424:1: ( ( rule__WMLKey__ValueAssignment_3_3 ) ) ( ( rule__WMLKey__ValueAssignment_3_3 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1424:1: ( ( rule__WMLKey__ValueAssignment_3_3 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1425:1: ( rule__WMLKey__ValueAssignment_3_3 )
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_3_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1426:1: ( rule__WMLKey__ValueAssignment_3_3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1426:2: rule__WMLKey__ValueAssignment_3_3
            {
            pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2982);
            rule__WMLKey__ValueAssignment_3_3();

            state._fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getValueAssignment_3_3()); 

            }

            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1429:1: ( ( rule__WMLKey__ValueAssignment_3_3 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1430:1: ( rule__WMLKey__ValueAssignment_3_3 )*
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_3_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1431:1: ( rule__WMLKey__ValueAssignment_3_3 )*
            loop18:
            do {
                int alt18=2;
                int LA18_0 = input.LA(1);

                if ( ((LA18_0>=RULE_ID && LA18_0<=RULE_ANY_OTHER)||LA18_0==RULE_LUA_CODE||(LA18_0>=20 && LA18_0<=27)||LA18_0==29||LA18_0==33) ) {
                    alt18=1;
                }


                switch (alt18) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1431:2: rule__WMLKey__ValueAssignment_3_3
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2994);
            	    rule__WMLKey__ValueAssignment_3_3();

            	    state._fsp--;


            	    }
            	    break;

            	default :
            	    break loop18;
                }
            } while (true);

             after(grammarAccess.getWMLKeyAccess().getValueAssignment_3_3()); 

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


    // $ANTLR start "rule__WMLKeyValue__Group_0__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1450:1: rule__WMLKeyValue__Group_0__0 : rule__WMLKeyValue__Group_0__0__Impl rule__WMLKeyValue__Group_0__1 ;
    public final void rule__WMLKeyValue__Group_0__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1454:1: ( rule__WMLKeyValue__Group_0__0__Impl rule__WMLKeyValue__Group_0__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1455:2: rule__WMLKeyValue__Group_0__0__Impl rule__WMLKeyValue__Group_0__1
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Group_0__0__Impl_in_rule__WMLKeyValue__Group_0__03035);
            rule__WMLKeyValue__Group_0__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKeyValue__Group_0__1_in_rule__WMLKeyValue__Group_0__03038);
            rule__WMLKeyValue__Group_0__1();

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
    // $ANTLR end "rule__WMLKeyValue__Group_0__0"


    // $ANTLR start "rule__WMLKeyValue__Group_0__0__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1462:1: rule__WMLKeyValue__Group_0__0__Impl : ( () ) ;
    public final void rule__WMLKeyValue__Group_0__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1466:1: ( ( () ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1467:1: ( () )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1467:1: ( () )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1468:1: ()
            {
             before(grammarAccess.getWMLKeyValueAccess().getWMLKeyValueAction_0_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1469:1: ()
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1471:1: 
            {
            }

             after(grammarAccess.getWMLKeyValueAccess().getWMLKeyValueAction_0_0()); 

            }


            }

        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKeyValue__Group_0__0__Impl"


    // $ANTLR start "rule__WMLKeyValue__Group_0__1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1481:1: rule__WMLKeyValue__Group_0__1 : rule__WMLKeyValue__Group_0__1__Impl ;
    public final void rule__WMLKeyValue__Group_0__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1485:1: ( rule__WMLKeyValue__Group_0__1__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1486:2: rule__WMLKeyValue__Group_0__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Group_0__1__Impl_in_rule__WMLKeyValue__Group_0__13096);
            rule__WMLKeyValue__Group_0__1__Impl();

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
    // $ANTLR end "rule__WMLKeyValue__Group_0__1"


    // $ANTLR start "rule__WMLKeyValue__Group_0__1__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1492:1: rule__WMLKeyValue__Group_0__1__Impl : ( ruleWMLValue ) ;
    public final void rule__WMLKeyValue__Group_0__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1496:1: ( ( ruleWMLValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1497:1: ( ruleWMLValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1497:1: ( ruleWMLValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1498:1: ruleWMLValue
            {
             before(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0_1()); 
            pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLKeyValue__Group_0__1__Impl3123);
            ruleWMLValue();

            state._fsp--;

             after(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLKeyValue__Group_0__1__Impl"


    // $ANTLR start "rule__WMLMacroCall__Group__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1513:1: rule__WMLMacroCall__Group__0 : rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 ;
    public final void rule__WMLMacroCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1517:1: ( rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1518:2: rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__03156);
            rule__WMLMacroCall__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__03159);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1525:1: rule__WMLMacroCall__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacroCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1529:1: ( ( '{' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1530:1: ( '{' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1530:1: ( '{' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1531:1: '{'
            {
             before(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,33,FOLLOW_33_in_rule__WMLMacroCall__Group__0__Impl3187); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1544:1: rule__WMLMacroCall__Group__1 : rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 ;
    public final void rule__WMLMacroCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1548:1: ( rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1549:2: rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__13218);
            rule__WMLMacroCall__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__13221);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1556:1: rule__WMLMacroCall__Group__1__Impl : ( ( rule__WMLMacroCall__PointAssignment_1 )? ) ;
    public final void rule__WMLMacroCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1560:1: ( ( ( rule__WMLMacroCall__PointAssignment_1 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1561:1: ( ( rule__WMLMacroCall__PointAssignment_1 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1561:1: ( ( rule__WMLMacroCall__PointAssignment_1 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1562:1: ( rule__WMLMacroCall__PointAssignment_1 )?
            {
             before(grammarAccess.getWMLMacroCallAccess().getPointAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1563:1: ( rule__WMLMacroCall__PointAssignment_1 )?
            int alt19=2;
            int LA19_0 = input.LA(1);

            if ( (LA19_0==23) ) {
                alt19=1;
            }
            switch (alt19) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1563:2: rule__WMLMacroCall__PointAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__PointAssignment_1_in_rule__WMLMacroCall__Group__1__Impl3248);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1573:1: rule__WMLMacroCall__Group__2 : rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 ;
    public final void rule__WMLMacroCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1577:1: ( rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1578:2: rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__23279);
            rule__WMLMacroCall__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__23282);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1585:1: rule__WMLMacroCall__Group__2__Impl : ( ( rule__WMLMacroCall__RelativeAssignment_2 )? ) ;
    public final void rule__WMLMacroCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1589:1: ( ( ( rule__WMLMacroCall__RelativeAssignment_2 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1590:1: ( ( rule__WMLMacroCall__RelativeAssignment_2 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1590:1: ( ( rule__WMLMacroCall__RelativeAssignment_2 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1591:1: ( rule__WMLMacroCall__RelativeAssignment_2 )?
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1592:1: ( rule__WMLMacroCall__RelativeAssignment_2 )?
            int alt20=2;
            int LA20_0 = input.LA(1);

            if ( (LA20_0==21) ) {
                alt20=1;
            }
            switch (alt20) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1592:2: rule__WMLMacroCall__RelativeAssignment_2
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__RelativeAssignment_2_in_rule__WMLMacroCall__Group__2__Impl3309);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1602:1: rule__WMLMacroCall__Group__3 : rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 ;
    public final void rule__WMLMacroCall__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1606:1: ( rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1607:2: rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__33340);
            rule__WMLMacroCall__Group__3__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__33343);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1614:1: rule__WMLMacroCall__Group__3__Impl : ( ( rule__WMLMacroCall__NameAssignment_3 ) ) ;
    public final void rule__WMLMacroCall__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1618:1: ( ( ( rule__WMLMacroCall__NameAssignment_3 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1619:1: ( ( rule__WMLMacroCall__NameAssignment_3 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1619:1: ( ( rule__WMLMacroCall__NameAssignment_3 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1620:1: ( rule__WMLMacroCall__NameAssignment_3 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameAssignment_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1621:1: ( rule__WMLMacroCall__NameAssignment_3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1621:2: rule__WMLMacroCall__NameAssignment_3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__NameAssignment_3_in_rule__WMLMacroCall__Group__3__Impl3370);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1631:1: rule__WMLMacroCall__Group__4 : rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5 ;
    public final void rule__WMLMacroCall__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1635:1: ( rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1636:2: rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__43400);
            rule__WMLMacroCall__Group__4__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__5_in_rule__WMLMacroCall__Group__43403);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1643:1: rule__WMLMacroCall__Group__4__Impl : ( ( rule__WMLMacroCall__ParametersAssignment_4 )* ) ;
    public final void rule__WMLMacroCall__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1647:1: ( ( ( rule__WMLMacroCall__ParametersAssignment_4 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1648:1: ( ( rule__WMLMacroCall__ParametersAssignment_4 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1648:1: ( ( rule__WMLMacroCall__ParametersAssignment_4 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1649:1: ( rule__WMLMacroCall__ParametersAssignment_4 )*
            {
             before(grammarAccess.getWMLMacroCallAccess().getParametersAssignment_4()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1650:1: ( rule__WMLMacroCall__ParametersAssignment_4 )*
            loop21:
            do {
                int alt21=2;
                int LA21_0 = input.LA(1);

                if ( ((LA21_0>=RULE_ID && LA21_0<=RULE_ANY_OTHER)||(LA21_0>=20 && LA21_0<=33)) ) {
                    alt21=1;
                }


                switch (alt21) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1650:2: rule__WMLMacroCall__ParametersAssignment_4
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroCall__ParametersAssignment_4_in_rule__WMLMacroCall__Group__4__Impl3430);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1660:1: rule__WMLMacroCall__Group__5 : rule__WMLMacroCall__Group__5__Impl ;
    public final void rule__WMLMacroCall__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1664:1: ( rule__WMLMacroCall__Group__5__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1665:2: rule__WMLMacroCall__Group__5__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__5__Impl_in_rule__WMLMacroCall__Group__53461);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1671:1: rule__WMLMacroCall__Group__5__Impl : ( '}' ) ;
    public final void rule__WMLMacroCall__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1675:1: ( ( '}' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1676:1: ( '}' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1676:1: ( '}' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1677:1: '}'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_5()); 
            match(input,34,FOLLOW_34_in_rule__WMLMacroCall__Group__5__Impl3489); 
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


    // $ANTLR start "rule__WMLMacroCallParameter__Group_0__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1702:1: rule__WMLMacroCallParameter__Group_0__0 : rule__WMLMacroCallParameter__Group_0__0__Impl rule__WMLMacroCallParameter__Group_0__1 ;
    public final void rule__WMLMacroCallParameter__Group_0__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1706:1: ( rule__WMLMacroCallParameter__Group_0__0__Impl rule__WMLMacroCallParameter__Group_0__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1707:2: rule__WMLMacroCallParameter__Group_0__0__Impl rule__WMLMacroCallParameter__Group_0__1
            {
            pushFollow(FOLLOW_rule__WMLMacroCallParameter__Group_0__0__Impl_in_rule__WMLMacroCallParameter__Group_0__03532);
            rule__WMLMacroCallParameter__Group_0__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCallParameter__Group_0__1_in_rule__WMLMacroCallParameter__Group_0__03535);
            rule__WMLMacroCallParameter__Group_0__1();

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
    // $ANTLR end "rule__WMLMacroCallParameter__Group_0__0"


    // $ANTLR start "rule__WMLMacroCallParameter__Group_0__0__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1714:1: rule__WMLMacroCallParameter__Group_0__0__Impl : ( () ) ;
    public final void rule__WMLMacroCallParameter__Group_0__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1718:1: ( ( () ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1719:1: ( () )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1719:1: ( () )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1720:1: ()
            {
             before(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroCallParameterAction_0_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1721:1: ()
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1723:1: 
            {
            }

             after(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroCallParameterAction_0_0()); 

            }


            }

        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroCallParameter__Group_0__0__Impl"


    // $ANTLR start "rule__WMLMacroCallParameter__Group_0__1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1733:1: rule__WMLMacroCallParameter__Group_0__1 : rule__WMLMacroCallParameter__Group_0__1__Impl ;
    public final void rule__WMLMacroCallParameter__Group_0__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1737:1: ( rule__WMLMacroCallParameter__Group_0__1__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1738:2: rule__WMLMacroCallParameter__Group_0__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroCallParameter__Group_0__1__Impl_in_rule__WMLMacroCallParameter__Group_0__13593);
            rule__WMLMacroCallParameter__Group_0__1__Impl();

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
    // $ANTLR end "rule__WMLMacroCallParameter__Group_0__1"


    // $ANTLR start "rule__WMLMacroCallParameter__Group_0__1__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1744:1: rule__WMLMacroCallParameter__Group_0__1__Impl : ( ruleWMLMacroParameter ) ;
    public final void rule__WMLMacroCallParameter__Group_0__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1748:1: ( ( ruleWMLMacroParameter ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1749:1: ( ruleWMLMacroParameter )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1749:1: ( ruleWMLMacroParameter )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1750:1: ruleWMLMacroParameter
            {
             before(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroParameterParserRuleCall_0_1()); 
            pushFollow(FOLLOW_ruleWMLMacroParameter_in_rule__WMLMacroCallParameter__Group_0__1__Impl3620);
            ruleWMLMacroParameter();

            state._fsp--;

             after(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroParameterParserRuleCall_0_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLMacroCallParameter__Group_0__1__Impl"


    // $ANTLR start "rule__WMLArrayCall__Group__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1765:1: rule__WMLArrayCall__Group__0 : rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 ;
    public final void rule__WMLArrayCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1769:1: ( rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1770:2: rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__03653);
            rule__WMLArrayCall__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__03656);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1777:1: rule__WMLArrayCall__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLArrayCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1781:1: ( ( '[' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1782:1: ( '[' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1782:1: ( '[' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1783:1: '['
            {
             before(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0()); 
            match(input,29,FOLLOW_29_in_rule__WMLArrayCall__Group__0__Impl3684); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1796:1: rule__WMLArrayCall__Group__1 : rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 ;
    public final void rule__WMLArrayCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1800:1: ( rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1801:2: rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__13715);
            rule__WMLArrayCall__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__13718);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1808:1: rule__WMLArrayCall__Group__1__Impl : ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) ;
    public final void rule__WMLArrayCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1812:1: ( ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1813:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1813:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1814:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1814:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1815:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1816:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1816:2: rule__WMLArrayCall__ValueAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3747);
            rule__WMLArrayCall__ValueAssignment_1();

            state._fsp--;


            }

             after(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 

            }

            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1819:1: ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1820:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1821:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            loop22:
            do {
                int alt22=2;
                int LA22_0 = input.LA(1);

                if ( ((LA22_0>=RULE_ID && LA22_0<=RULE_ANY_OTHER)||(LA22_0>=20 && LA22_0<=27)) ) {
                    alt22=1;
                }


                switch (alt22) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1821:2: rule__WMLArrayCall__ValueAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3759);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1832:1: rule__WMLArrayCall__Group__2 : rule__WMLArrayCall__Group__2__Impl ;
    public final void rule__WMLArrayCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1836:1: ( rule__WMLArrayCall__Group__2__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1837:2: rule__WMLArrayCall__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__23792);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1843:1: rule__WMLArrayCall__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLArrayCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1847:1: ( ( ']' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1848:1: ( ']' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1848:1: ( ']' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1849:1: ']'
            {
             before(grammarAccess.getWMLArrayCallAccess().getRightSquareBracketKeyword_2()); 
            match(input,30,FOLLOW_30_in_rule__WMLArrayCall__Group__2__Impl3820); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1868:1: rule__WMLMacroDefine__Group__0 : rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1 ;
    public final void rule__WMLMacroDefine__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1872:1: ( rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1873:2: rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__0__Impl_in_rule__WMLMacroDefine__Group__03857);
            rule__WMLMacroDefine__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__1_in_rule__WMLMacroDefine__Group__03860);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1880:1: rule__WMLMacroDefine__Group__0__Impl : ( ( rule__WMLMacroDefine__NameAssignment_0 ) ) ;
    public final void rule__WMLMacroDefine__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1884:1: ( ( ( rule__WMLMacroDefine__NameAssignment_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1885:1: ( ( rule__WMLMacroDefine__NameAssignment_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1885:1: ( ( rule__WMLMacroDefine__NameAssignment_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1886:1: ( rule__WMLMacroDefine__NameAssignment_0 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameAssignment_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1887:1: ( rule__WMLMacroDefine__NameAssignment_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1887:2: rule__WMLMacroDefine__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__NameAssignment_0_in_rule__WMLMacroDefine__Group__0__Impl3887);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1897:1: rule__WMLMacroDefine__Group__1 : rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2 ;
    public final void rule__WMLMacroDefine__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1901:1: ( rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1902:2: rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__1__Impl_in_rule__WMLMacroDefine__Group__13917);
            rule__WMLMacroDefine__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__2_in_rule__WMLMacroDefine__Group__13920);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1909:1: rule__WMLMacroDefine__Group__1__Impl : ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* ) ;
    public final void rule__WMLMacroDefine__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1913:1: ( ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1914:1: ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1914:1: ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1915:1: ( rule__WMLMacroDefine__ExpressionsAssignment_1 )*
            {
             before(grammarAccess.getWMLMacroDefineAccess().getExpressionsAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1916:1: ( rule__WMLMacroDefine__ExpressionsAssignment_1 )*
            loop23:
            do {
                int alt23=2;
                int LA23_0 = input.LA(1);

                if ( ((LA23_0>=RULE_IFDEF && LA23_0<=RULE_DEFINE)||LA23_0==RULE_TEXTDOMAIN||(LA23_0>=20 && LA23_0<=27)||LA23_0==29||LA23_0==33) ) {
                    alt23=1;
                }


                switch (alt23) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1916:2: rule__WMLMacroDefine__ExpressionsAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroDefine__ExpressionsAssignment_1_in_rule__WMLMacroDefine__Group__1__Impl3947);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1926:1: rule__WMLMacroDefine__Group__2 : rule__WMLMacroDefine__Group__2__Impl ;
    public final void rule__WMLMacroDefine__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1930:1: ( rule__WMLMacroDefine__Group__2__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1931:2: rule__WMLMacroDefine__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__2__Impl_in_rule__WMLMacroDefine__Group__23978);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1937:1: rule__WMLMacroDefine__Group__2__Impl : ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) ) ;
    public final void rule__WMLMacroDefine__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1941:1: ( ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1942:1: ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1942:1: ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1943:1: ( rule__WMLMacroDefine__EndNameAssignment_2 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getEndNameAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1944:1: ( rule__WMLMacroDefine__EndNameAssignment_2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1944:2: rule__WMLMacroDefine__EndNameAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__EndNameAssignment_2_in_rule__WMLMacroDefine__Group__2__Impl4005);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1960:1: rule__WMLPreprocIF__Group__0 : rule__WMLPreprocIF__Group__0__Impl rule__WMLPreprocIF__Group__1 ;
    public final void rule__WMLPreprocIF__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1964:1: ( rule__WMLPreprocIF__Group__0__Impl rule__WMLPreprocIF__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1965:2: rule__WMLPreprocIF__Group__0__Impl rule__WMLPreprocIF__Group__1
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__0__Impl_in_rule__WMLPreprocIF__Group__04041);
            rule__WMLPreprocIF__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__1_in_rule__WMLPreprocIF__Group__04044);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1972:1: rule__WMLPreprocIF__Group__0__Impl : ( ( rule__WMLPreprocIF__NameAssignment_0 ) ) ;
    public final void rule__WMLPreprocIF__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1976:1: ( ( ( rule__WMLPreprocIF__NameAssignment_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1977:1: ( ( rule__WMLPreprocIF__NameAssignment_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1977:1: ( ( rule__WMLPreprocIF__NameAssignment_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1978:1: ( rule__WMLPreprocIF__NameAssignment_0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getNameAssignment_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1979:1: ( rule__WMLPreprocIF__NameAssignment_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1979:2: rule__WMLPreprocIF__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__NameAssignment_0_in_rule__WMLPreprocIF__Group__0__Impl4071);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1989:1: rule__WMLPreprocIF__Group__1 : rule__WMLPreprocIF__Group__1__Impl rule__WMLPreprocIF__Group__2 ;
    public final void rule__WMLPreprocIF__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1993:1: ( rule__WMLPreprocIF__Group__1__Impl rule__WMLPreprocIF__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1994:2: rule__WMLPreprocIF__Group__1__Impl rule__WMLPreprocIF__Group__2
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__1__Impl_in_rule__WMLPreprocIF__Group__14101);
            rule__WMLPreprocIF__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__2_in_rule__WMLPreprocIF__Group__14104);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2001:1: rule__WMLPreprocIF__Group__1__Impl : ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* ) ;
    public final void rule__WMLPreprocIF__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2005:1: ( ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2006:1: ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2006:1: ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2007:1: ( rule__WMLPreprocIF__ExpressionsAssignment_1 )*
            {
             before(grammarAccess.getWMLPreprocIFAccess().getExpressionsAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2008:1: ( rule__WMLPreprocIF__ExpressionsAssignment_1 )*
            loop24:
            do {
                int alt24=2;
                int LA24_0 = input.LA(1);

                if ( ((LA24_0>=RULE_IFDEF && LA24_0<=RULE_DEFINE)||LA24_0==RULE_TEXTDOMAIN||(LA24_0>=20 && LA24_0<=27)||LA24_0==29||LA24_0==33) ) {
                    alt24=1;
                }


                switch (alt24) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2008:2: rule__WMLPreprocIF__ExpressionsAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLPreprocIF__ExpressionsAssignment_1_in_rule__WMLPreprocIF__Group__1__Impl4131);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2018:1: rule__WMLPreprocIF__Group__2 : rule__WMLPreprocIF__Group__2__Impl rule__WMLPreprocIF__Group__3 ;
    public final void rule__WMLPreprocIF__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2022:1: ( rule__WMLPreprocIF__Group__2__Impl rule__WMLPreprocIF__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2023:2: rule__WMLPreprocIF__Group__2__Impl rule__WMLPreprocIF__Group__3
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__2__Impl_in_rule__WMLPreprocIF__Group__24162);
            rule__WMLPreprocIF__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__3_in_rule__WMLPreprocIF__Group__24165);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2030:1: rule__WMLPreprocIF__Group__2__Impl : ( ( rule__WMLPreprocIF__Group_2__0 )? ) ;
    public final void rule__WMLPreprocIF__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2034:1: ( ( ( rule__WMLPreprocIF__Group_2__0 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2035:1: ( ( rule__WMLPreprocIF__Group_2__0 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2035:1: ( ( rule__WMLPreprocIF__Group_2__0 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2036:1: ( rule__WMLPreprocIF__Group_2__0 )?
            {
             before(grammarAccess.getWMLPreprocIFAccess().getGroup_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2037:1: ( rule__WMLPreprocIF__Group_2__0 )?
            int alt25=2;
            int LA25_0 = input.LA(1);

            if ( (LA25_0==RULE_ELSE) ) {
                alt25=1;
            }
            switch (alt25) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2037:2: rule__WMLPreprocIF__Group_2__0
                    {
                    pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__0_in_rule__WMLPreprocIF__Group__2__Impl4192);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2047:1: rule__WMLPreprocIF__Group__3 : rule__WMLPreprocIF__Group__3__Impl ;
    public final void rule__WMLPreprocIF__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2051:1: ( rule__WMLPreprocIF__Group__3__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2052:2: rule__WMLPreprocIF__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__3__Impl_in_rule__WMLPreprocIF__Group__34223);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2058:1: rule__WMLPreprocIF__Group__3__Impl : ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) ) ;
    public final void rule__WMLPreprocIF__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2062:1: ( ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2063:1: ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2063:1: ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2064:1: ( rule__WMLPreprocIF__EndNameAssignment_3 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getEndNameAssignment_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2065:1: ( rule__WMLPreprocIF__EndNameAssignment_3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2065:2: rule__WMLPreprocIF__EndNameAssignment_3
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__EndNameAssignment_3_in_rule__WMLPreprocIF__Group__3__Impl4250);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2083:1: rule__WMLPreprocIF__Group_2__0 : rule__WMLPreprocIF__Group_2__0__Impl rule__WMLPreprocIF__Group_2__1 ;
    public final void rule__WMLPreprocIF__Group_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2087:1: ( rule__WMLPreprocIF__Group_2__0__Impl rule__WMLPreprocIF__Group_2__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2088:2: rule__WMLPreprocIF__Group_2__0__Impl rule__WMLPreprocIF__Group_2__1
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__0__Impl_in_rule__WMLPreprocIF__Group_2__04288);
            rule__WMLPreprocIF__Group_2__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__1_in_rule__WMLPreprocIF__Group_2__04291);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2095:1: rule__WMLPreprocIF__Group_2__0__Impl : ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) ) ;
    public final void rule__WMLPreprocIF__Group_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2099:1: ( ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2100:1: ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2100:1: ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2101:1: ( rule__WMLPreprocIF__ElsesAssignment_2_0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElsesAssignment_2_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2102:1: ( rule__WMLPreprocIF__ElsesAssignment_2_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2102:2: rule__WMLPreprocIF__ElsesAssignment_2_0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__ElsesAssignment_2_0_in_rule__WMLPreprocIF__Group_2__0__Impl4318);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2112:1: rule__WMLPreprocIF__Group_2__1 : rule__WMLPreprocIF__Group_2__1__Impl ;
    public final void rule__WMLPreprocIF__Group_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2116:1: ( rule__WMLPreprocIF__Group_2__1__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2117:2: rule__WMLPreprocIF__Group_2__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__1__Impl_in_rule__WMLPreprocIF__Group_2__14348);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2123:1: rule__WMLPreprocIF__Group_2__1__Impl : ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) ) ;
    public final void rule__WMLPreprocIF__Group_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2127:1: ( ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2128:1: ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2128:1: ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2129:1: ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2129:1: ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2130:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2131:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2131:2: rule__WMLPreprocIF__ElseExpressionsAssignment_2_1
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4377);
            rule__WMLPreprocIF__ElseExpressionsAssignment_2_1();

            state._fsp--;


            }

             after(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 

            }

            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2134:1: ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2135:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )*
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2136:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )*
            loop26:
            do {
                int alt26=2;
                int LA26_0 = input.LA(1);

                if ( ((LA26_0>=RULE_IFDEF && LA26_0<=RULE_DEFINE)||LA26_0==RULE_TEXTDOMAIN||(LA26_0>=20 && LA26_0<=27)||LA26_0==29||LA26_0==33) ) {
                    alt26=1;
                }


                switch (alt26) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2136:2: rule__WMLPreprocIF__ElseExpressionsAssignment_2_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4389);
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


    // $ANTLR start "rule__WMLValuedExpression__Group_1__0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2151:1: rule__WMLValuedExpression__Group_1__0 : rule__WMLValuedExpression__Group_1__0__Impl rule__WMLValuedExpression__Group_1__1 ;
    public final void rule__WMLValuedExpression__Group_1__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2155:1: ( rule__WMLValuedExpression__Group_1__0__Impl rule__WMLValuedExpression__Group_1__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2156:2: rule__WMLValuedExpression__Group_1__0__Impl rule__WMLValuedExpression__Group_1__1
            {
            pushFollow(FOLLOW_rule__WMLValuedExpression__Group_1__0__Impl_in_rule__WMLValuedExpression__Group_1__04426);
            rule__WMLValuedExpression__Group_1__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLValuedExpression__Group_1__1_in_rule__WMLValuedExpression__Group_1__04429);
            rule__WMLValuedExpression__Group_1__1();

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
    // $ANTLR end "rule__WMLValuedExpression__Group_1__0"


    // $ANTLR start "rule__WMLValuedExpression__Group_1__0__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2163:1: rule__WMLValuedExpression__Group_1__0__Impl : ( () ) ;
    public final void rule__WMLValuedExpression__Group_1__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2167:1: ( ( () ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2168:1: ( () )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2168:1: ( () )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2169:1: ()
            {
             before(grammarAccess.getWMLValuedExpressionAccess().getWMLValuedExpressionAction_1_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2170:1: ()
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2172:1: 
            {
            }

             after(grammarAccess.getWMLValuedExpressionAccess().getWMLValuedExpressionAction_1_0()); 

            }


            }

        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLValuedExpression__Group_1__0__Impl"


    // $ANTLR start "rule__WMLValuedExpression__Group_1__1"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2182:1: rule__WMLValuedExpression__Group_1__1 : rule__WMLValuedExpression__Group_1__1__Impl ;
    public final void rule__WMLValuedExpression__Group_1__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2186:1: ( rule__WMLValuedExpression__Group_1__1__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2187:2: rule__WMLValuedExpression__Group_1__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLValuedExpression__Group_1__1__Impl_in_rule__WMLValuedExpression__Group_1__14487);
            rule__WMLValuedExpression__Group_1__1__Impl();

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
    // $ANTLR end "rule__WMLValuedExpression__Group_1__1"


    // $ANTLR start "rule__WMLValuedExpression__Group_1__1__Impl"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2193:1: rule__WMLValuedExpression__Group_1__1__Impl : ( ruleWMLValue ) ;
    public final void rule__WMLValuedExpression__Group_1__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2197:1: ( ( ruleWMLValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2198:1: ( ruleWMLValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2198:1: ( ruleWMLValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2199:1: ruleWMLValue
            {
             before(grammarAccess.getWMLValuedExpressionAccess().getWMLValueParserRuleCall_1_1()); 
            pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLValuedExpression__Group_1__1__Impl4514);
            ruleWMLValue();

            state._fsp--;

             after(grammarAccess.getWMLValuedExpressionAccess().getWMLValueParserRuleCall_1_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end "rule__WMLValuedExpression__Group_1__1__Impl"


    // $ANTLR start "rule__WMLRoot__ExpressionsAssignment"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2215:1: rule__WMLRoot__ExpressionsAssignment : ( ruleWMLRootExpression ) ;
    public final void rule__WMLRoot__ExpressionsAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2219:1: ( ( ruleWMLRootExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2220:1: ( ruleWMLRootExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2220:1: ( ruleWMLRootExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2221:1: ruleWMLRootExpression
            {
             before(grammarAccess.getWMLRootAccess().getExpressionsWMLRootExpressionParserRuleCall_0()); 
            pushFollow(FOLLOW_ruleWMLRootExpression_in_rule__WMLRoot__ExpressionsAssignment4552);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2230:1: rule__WMLTag__PlusAssignment_1 : ( ( '+' ) ) ;
    public final void rule__WMLTag__PlusAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2234:1: ( ( ( '+' ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2235:1: ( ( '+' ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2235:1: ( ( '+' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2236:1: ( '+' )
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2237:1: ( '+' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2238:1: '+'
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            match(input,31,FOLLOW_31_in_rule__WMLTag__PlusAssignment_14588); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2253:1: rule__WMLTag__NameAssignment_2 : ( RULE_ID ) ;
    public final void rule__WMLTag__NameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2257:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2258:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2258:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2259:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_2_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_24627); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2268:1: rule__WMLTag__ExpressionsAssignment_4 : ( ruleWMLExpression ) ;
    public final void rule__WMLTag__ExpressionsAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2272:1: ( ( ruleWMLExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2273:1: ( ruleWMLExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2273:1: ( ruleWMLExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2274:1: ruleWMLExpression
            {
             before(grammarAccess.getWMLTagAccess().getExpressionsWMLExpressionParserRuleCall_4_0()); 
            pushFollow(FOLLOW_ruleWMLExpression_in_rule__WMLTag__ExpressionsAssignment_44658);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2283:1: rule__WMLTag__EndNameAssignment_6 : ( RULE_ID ) ;
    public final void rule__WMLTag__EndNameAssignment_6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2287:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2288:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2288:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2289:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_64689); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2298:1: rule__WMLKey__NameAssignment_0 : ( RULE_ID ) ;
    public final void rule__WMLKey__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2302:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2303:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2303:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2304:1: RULE_ID
            {
             before(grammarAccess.getWMLKeyAccess().getNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_04720); 
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


    // $ANTLR start "rule__WMLKey__ValueAssignment_2"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2313:1: rule__WMLKey__ValueAssignment_2 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2317:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2318:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2318:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2319:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_24751);
            ruleWMLKeyValue();

            state._fsp--;

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
    // $ANTLR end "rule__WMLKey__ValueAssignment_2"


    // $ANTLR start "rule__WMLKey__ValueAssignment_3_3"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2328:1: rule__WMLKey__ValueAssignment_3_3 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValueAssignment_3_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2332:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2333:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2333:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2334:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_3_3_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_3_34782);
            ruleWMLKeyValue();

            state._fsp--;

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
    // $ANTLR end "rule__WMLKey__ValueAssignment_3_3"


    // $ANTLR start "rule__WMLKey__EolAssignment_4"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2343:1: rule__WMLKey__EolAssignment_4 : ( ( rule__WMLKey__EolAlternatives_4_0 ) ) ;
    public final void rule__WMLKey__EolAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2347:1: ( ( ( rule__WMLKey__EolAlternatives_4_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2348:1: ( ( rule__WMLKey__EolAlternatives_4_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2348:1: ( ( rule__WMLKey__EolAlternatives_4_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2349:1: ( rule__WMLKey__EolAlternatives_4_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getEolAlternatives_4_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2350:1: ( rule__WMLKey__EolAlternatives_4_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2350:2: rule__WMLKey__EolAlternatives_4_0
            {
            pushFollow(FOLLOW_rule__WMLKey__EolAlternatives_4_0_in_rule__WMLKey__EolAssignment_44813);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2359:1: rule__WMLMacroCall__PointAssignment_1 : ( ( './' ) ) ;
    public final void rule__WMLMacroCall__PointAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2363:1: ( ( ( './' ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2364:1: ( ( './' ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2364:1: ( ( './' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2365:1: ( './' )
            {
             before(grammarAccess.getWMLMacroCallAccess().getPointFullStopSolidusKeyword_1_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2366:1: ( './' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2367:1: './'
            {
             before(grammarAccess.getWMLMacroCallAccess().getPointFullStopSolidusKeyword_1_0()); 
            match(input,23,FOLLOW_23_in_rule__WMLMacroCall__PointAssignment_14851); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2382:1: rule__WMLMacroCall__RelativeAssignment_2 : ( ( '~' ) ) ;
    public final void rule__WMLMacroCall__RelativeAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2386:1: ( ( ( '~' ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2387:1: ( ( '~' ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2387:1: ( ( '~' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2388:1: ( '~' )
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2389:1: ( '~' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2390:1: '~'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0()); 
            match(input,21,FOLLOW_21_in_rule__WMLMacroCall__RelativeAssignment_24895); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2405:1: rule__WMLMacroCall__NameAssignment_3 : ( RULE_ID ) ;
    public final void rule__WMLMacroCall__NameAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2409:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2410:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2410:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2411:1: RULE_ID
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_3_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_34934); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2420:1: rule__WMLMacroCall__ParametersAssignment_4 : ( ruleWMLMacroCallParameter ) ;
    public final void rule__WMLMacroCall__ParametersAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2424:1: ( ( ruleWMLMacroCallParameter ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2425:1: ( ruleWMLMacroCallParameter )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2425:1: ( ruleWMLMacroCallParameter )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2426:1: ruleWMLMacroCallParameter
            {
             before(grammarAccess.getWMLMacroCallAccess().getParametersWMLMacroCallParameterParserRuleCall_4_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_rule__WMLMacroCall__ParametersAssignment_44965);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2435:1: rule__WMLArrayCall__ValueAssignment_1 : ( ruleWMLValue ) ;
    public final void rule__WMLArrayCall__ValueAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2439:1: ( ( ruleWMLValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2440:1: ( ruleWMLValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2440:1: ( ruleWMLValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2441:1: ruleWMLValue
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_14996);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2450:1: rule__WMLMacroDefine__NameAssignment_0 : ( RULE_DEFINE ) ;
    public final void rule__WMLMacroDefine__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2454:1: ( ( RULE_DEFINE ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2455:1: ( RULE_DEFINE )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2455:1: ( RULE_DEFINE )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2456:1: RULE_DEFINE
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0_0()); 
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment_05027); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2465:1: rule__WMLMacroDefine__ExpressionsAssignment_1 : ( ruleWMLValuedExpression ) ;
    public final void rule__WMLMacroDefine__ExpressionsAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2469:1: ( ( ruleWMLValuedExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2470:1: ( ruleWMLValuedExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2470:1: ( ruleWMLValuedExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2471:1: ruleWMLValuedExpression
            {
             before(grammarAccess.getWMLMacroDefineAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_rule__WMLMacroDefine__ExpressionsAssignment_15058);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2480:1: rule__WMLMacroDefine__EndNameAssignment_2 : ( RULE_ENDDEF ) ;
    public final void rule__WMLMacroDefine__EndNameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2484:1: ( ( RULE_ENDDEF ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2485:1: ( RULE_ENDDEF )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2485:1: ( RULE_ENDDEF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2486:1: RULE_ENDDEF
            {
             before(grammarAccess.getWMLMacroDefineAccess().getEndNameENDDEFTerminalRuleCall_2_0()); 
            match(input,RULE_ENDDEF,FOLLOW_RULE_ENDDEF_in_rule__WMLMacroDefine__EndNameAssignment_25089); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2495:1: rule__WMLPreprocIF__NameAssignment_0 : ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) ) ;
    public final void rule__WMLPreprocIF__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2499:1: ( ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2500:1: ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2500:1: ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2501:1: ( rule__WMLPreprocIF__NameAlternatives_0_0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getNameAlternatives_0_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2502:1: ( rule__WMLPreprocIF__NameAlternatives_0_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2502:2: rule__WMLPreprocIF__NameAlternatives_0_0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__NameAlternatives_0_0_in_rule__WMLPreprocIF__NameAssignment_05120);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2511:1: rule__WMLPreprocIF__ExpressionsAssignment_1 : ( ruleWMLValuedExpression ) ;
    public final void rule__WMLPreprocIF__ExpressionsAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2515:1: ( ( ruleWMLValuedExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2516:1: ( ruleWMLValuedExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2516:1: ( ruleWMLValuedExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2517:1: ruleWMLValuedExpression
            {
             before(grammarAccess.getWMLPreprocIFAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ExpressionsAssignment_15153);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2526:1: rule__WMLPreprocIF__ElsesAssignment_2_0 : ( RULE_ELSE ) ;
    public final void rule__WMLPreprocIF__ElsesAssignment_2_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2530:1: ( ( RULE_ELSE ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2531:1: ( RULE_ELSE )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2531:1: ( RULE_ELSE )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2532:1: RULE_ELSE
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElsesELSETerminalRuleCall_2_0_0()); 
            match(input,RULE_ELSE,FOLLOW_RULE_ELSE_in_rule__WMLPreprocIF__ElsesAssignment_2_05184); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2541:1: rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 : ( ruleWMLValuedExpression ) ;
    public final void rule__WMLPreprocIF__ElseExpressionsAssignment_2_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2545:1: ( ( ruleWMLValuedExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2546:1: ( ruleWMLValuedExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2546:1: ( ruleWMLValuedExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2547:1: ruleWMLValuedExpression
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsWMLValuedExpressionParserRuleCall_2_1_0()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ElseExpressionsAssignment_2_15215);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2556:1: rule__WMLPreprocIF__EndNameAssignment_3 : ( RULE_ENDIF ) ;
    public final void rule__WMLPreprocIF__EndNameAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2560:1: ( ( RULE_ENDIF ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2561:1: ( RULE_ENDIF )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2561:1: ( RULE_ENDIF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2562:1: RULE_ENDIF
            {
             before(grammarAccess.getWMLPreprocIFAccess().getEndNameENDIFTerminalRuleCall_3_0()); 
            match(input,RULE_ENDIF,FOLLOW_RULE_ENDIF_in_rule__WMLPreprocIF__EndNameAssignment_35246); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2571:1: rule__WMLTextdomain__NameAssignment : ( RULE_TEXTDOMAIN ) ;
    public final void rule__WMLTextdomain__NameAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2575:1: ( ( RULE_TEXTDOMAIN ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2576:1: ( RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2576:1: ( RULE_TEXTDOMAIN )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2577:1: RULE_TEXTDOMAIN
            {
             before(grammarAccess.getWMLTextdomainAccess().getNameTEXTDOMAINTerminalRuleCall_0()); 
            match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment5277); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2586:1: rule__WMLLuaCode__ValueAssignment : ( RULE_LUA_CODE ) ;
    public final void rule__WMLLuaCode__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2590:1: ( ( RULE_LUA_CODE ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2591:1: ( RULE_LUA_CODE )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2591:1: ( RULE_LUA_CODE )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2592:1: RULE_LUA_CODE
            {
             before(grammarAccess.getWMLLuaCodeAccess().getValueLUA_CODETerminalRuleCall_0()); 
            match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment5308); 
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
    public static final BitSet FOLLOW_rule__WMLValue__Alternatives_in_ruleWMLValue1004 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens1031 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleMacroTokens1038 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__MacroTokens__Alternatives_in_ruleMacroTokens1064 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__EolAlternatives_4_01100 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_SL_COMMENT_in_rule__WMLKey__EolAlternatives_4_01117 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Group_0__0_in_rule__WMLKeyValue__Alternatives1149 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives1167 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_rule__WMLKeyValue__Alternatives1184 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_rule__WMLKeyValue__Alternatives1201 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Group_0__0_in_rule__WMLMacroCallParameter__Alternatives1233 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCallParameter__Alternatives1251 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFDEF_in_rule__WMLPreprocIF__NameAlternatives_0_01283 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFNDEF_in_rule__WMLPreprocIF__NameAlternatives_0_01300 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFHAVE_in_rule__WMLPreprocIF__NameAlternatives_0_01317 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFNHAVE_in_rule__WMLPreprocIF__NameAlternatives_0_01334 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRootExpression__Alternatives1366 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLRootExpression__Alternatives1383 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLRootExpression__Alternatives1400 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_rule__WMLRootExpression__Alternatives1417 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPreprocIF_in_rule__WMLRootExpression__Alternatives1434 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_rule__WMLExpression__Alternatives1466 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLExpression__Alternatives1483 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_rule__WMLValuedExpression__Alternatives1515 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValuedExpression__Group_1__0_in_rule__WMLValuedExpression__Alternatives1532 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLMacroParameter__Alternatives1565 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_rule__WMLMacroParameter__Alternatives1582 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLValue__Alternatives1614 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLValue__Alternatives1631 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__WMLValue__Alternatives1649 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLValue__Alternatives1669 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__WMLValue__Alternatives1689 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__WMLValue__Alternatives1709 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_rule__WMLValue__Alternatives1729 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__WMLValue__Alternatives1749 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_rule__WMLValue__Alternatives1769 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rule__WMLValue__Alternatives1789 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__Alternatives1808 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__MacroTokens__Alternatives1841 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__MacroTokens__Alternatives1861 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__MacroTokens__Alternatives1881 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__MacroTokens__Alternatives1901 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__MacroTokens__Alternatives1921 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01953 = new BitSet(new long[]{0x0000000080000400L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01956 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__WMLTag__Group__0__Impl1984 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__12015 = new BitSet(new long[]{0x0000000080000400L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__12018 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl2045 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__22076 = new BitSet(new long[]{0x0000000040000000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__22079 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl2106 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__32136 = new BitSet(new long[]{0x00000003200227C0L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__32139 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__WMLTag__Group__3__Impl2167 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__42198 = new BitSet(new long[]{0x00000003200227C0L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__42201 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__ExpressionsAssignment_4_in_rule__WMLTag__Group__4__Impl2228 = new BitSet(new long[]{0x00000002200227C2L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__52259 = new BitSet(new long[]{0x0000000000000400L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__52262 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__WMLTag__Group__5__Impl2290 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__62321 = new BitSet(new long[]{0x0000000040000000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__62324 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl2351 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__72381 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__WMLTag__Group__7__Impl2409 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02456 = new BitSet(new long[]{0x0000000010000000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02459 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl2486 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12516 = new BitSet(new long[]{0x00000002AFF41C30L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12519 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__WMLKey__Group__1__Impl2547 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22578 = new BitSet(new long[]{0x00000002AFF41C30L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__22581 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl2608 = new BitSet(new long[]{0x000000022FF41C02L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__32639 = new BitSet(new long[]{0x00000002AFF41C30L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__4_in_rule__WMLKey__Group__32642 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__0_in_rule__WMLKey__Group__3__Impl2669 = new BitSet(new long[]{0x0000000080000012L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__4__Impl_in_rule__WMLKey__Group__42700 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__EolAssignment_4_in_rule__WMLKey__Group__4__Impl2727 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__0__Impl_in_rule__WMLKey__Group_3__02767 = new BitSet(new long[]{0x0000000080000010L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__1_in_rule__WMLKey__Group_3__02770 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__0__Impl2798 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__1__Impl_in_rule__WMLKey__Group_3__12829 = new BitSet(new long[]{0x000000022FF41C10L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__2_in_rule__WMLKey__Group_3__12832 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__WMLKey__Group_3__1__Impl2860 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__2__Impl_in_rule__WMLKey__Group_3__22891 = new BitSet(new long[]{0x000000022FF41C10L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__3_in_rule__WMLKey__Group_3__22894 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__2__Impl2922 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__3__Impl_in_rule__WMLKey__Group_3__32953 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2982 = new BitSet(new long[]{0x000000022FF41C12L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2994 = new BitSet(new long[]{0x000000022FF41C12L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Group_0__0__Impl_in_rule__WMLKeyValue__Group_0__03035 = new BitSet(new long[]{0x000000000FF01C00L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Group_0__1_in_rule__WMLKeyValue__Group_0__03038 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Group_0__1__Impl_in_rule__WMLKeyValue__Group_0__13096 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLKeyValue__Group_0__1__Impl3123 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__03156 = new BitSet(new long[]{0x0000000000A00400L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__03159 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_33_in_rule__WMLMacroCall__Group__0__Impl3187 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__13218 = new BitSet(new long[]{0x0000000000A00400L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__13221 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__PointAssignment_1_in_rule__WMLMacroCall__Group__1__Impl3248 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__23279 = new BitSet(new long[]{0x0000000000A00400L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__23282 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__RelativeAssignment_2_in_rule__WMLMacroCall__Group__2__Impl3309 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__33340 = new BitSet(new long[]{0x00000007FFF01C00L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__33343 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__NameAssignment_3_in_rule__WMLMacroCall__Group__3__Impl3370 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__43400 = new BitSet(new long[]{0x00000007FFF01C00L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__5_in_rule__WMLMacroCall__Group__43403 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ParametersAssignment_4_in_rule__WMLMacroCall__Group__4__Impl3430 = new BitSet(new long[]{0x00000003FFF01C02L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__5__Impl_in_rule__WMLMacroCall__Group__53461 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_34_in_rule__WMLMacroCall__Group__5__Impl3489 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Group_0__0__Impl_in_rule__WMLMacroCallParameter__Group_0__03532 = new BitSet(new long[]{0x00000001FFF01C00L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Group_0__1_in_rule__WMLMacroCallParameter__Group_0__03535 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Group_0__1__Impl_in_rule__WMLMacroCallParameter__Group_0__13593 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_rule__WMLMacroCallParameter__Group_0__1__Impl3620 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__03653 = new BitSet(new long[]{0x000000000FF01C00L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__03656 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__WMLArrayCall__Group__0__Impl3684 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__13715 = new BitSet(new long[]{0x0000000040000000L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__13718 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3747 = new BitSet(new long[]{0x000000000FF01C02L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3759 = new BitSet(new long[]{0x000000000FF01C02L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__23792 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__WMLArrayCall__Group__2__Impl3820 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__0__Impl_in_rule__WMLMacroDefine__Group__03857 = new BitSet(new long[]{0x000000022FF27FC0L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__1_in_rule__WMLMacroDefine__Group__03860 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__NameAssignment_0_in_rule__WMLMacroDefine__Group__0__Impl3887 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__1__Impl_in_rule__WMLMacroDefine__Group__13917 = new BitSet(new long[]{0x000000022FF27FC0L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__2_in_rule__WMLMacroDefine__Group__13920 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__ExpressionsAssignment_1_in_rule__WMLMacroDefine__Group__1__Impl3947 = new BitSet(new long[]{0x000000022FF23FC2L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__2__Impl_in_rule__WMLMacroDefine__Group__23978 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__EndNameAssignment_2_in_rule__WMLMacroDefine__Group__2__Impl4005 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__0__Impl_in_rule__WMLPreprocIF__Group__04041 = new BitSet(new long[]{0x000000022FF3BFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__1_in_rule__WMLPreprocIF__Group__04044 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__NameAssignment_0_in_rule__WMLPreprocIF__Group__0__Impl4071 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__1__Impl_in_rule__WMLPreprocIF__Group__14101 = new BitSet(new long[]{0x000000022FF3BFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__2_in_rule__WMLPreprocIF__Group__14104 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ExpressionsAssignment_1_in_rule__WMLPreprocIF__Group__1__Impl4131 = new BitSet(new long[]{0x000000022FF23FC2L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__2__Impl_in_rule__WMLPreprocIF__Group__24162 = new BitSet(new long[]{0x000000022FF3BFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__3_in_rule__WMLPreprocIF__Group__24165 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__0_in_rule__WMLPreprocIF__Group__2__Impl4192 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__3__Impl_in_rule__WMLPreprocIF__Group__34223 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__EndNameAssignment_3_in_rule__WMLPreprocIF__Group__3__Impl4250 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__0__Impl_in_rule__WMLPreprocIF__Group_2__04288 = new BitSet(new long[]{0x000000022FF23FC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__1_in_rule__WMLPreprocIF__Group_2__04291 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ElsesAssignment_2_0_in_rule__WMLPreprocIF__Group_2__0__Impl4318 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__1__Impl_in_rule__WMLPreprocIF__Group_2__14348 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4377 = new BitSet(new long[]{0x000000022FF23FC2L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4389 = new BitSet(new long[]{0x000000022FF23FC2L});
    public static final BitSet FOLLOW_rule__WMLValuedExpression__Group_1__0__Impl_in_rule__WMLValuedExpression__Group_1__04426 = new BitSet(new long[]{0x000000022FF23FC0L});
    public static final BitSet FOLLOW_rule__WMLValuedExpression__Group_1__1_in_rule__WMLValuedExpression__Group_1__04429 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValuedExpression__Group_1__1__Impl_in_rule__WMLValuedExpression__Group_1__14487 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLValuedExpression__Group_1__1__Impl4514 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_rule__WMLRoot__ExpressionsAssignment4552 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__WMLTag__PlusAssignment_14588 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_24627 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_rule__WMLTag__ExpressionsAssignment_44658 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_64689 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_04720 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_24751 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_3_34782 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__EolAlternatives_4_0_in_rule__WMLKey__EolAssignment_44813 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__WMLMacroCall__PointAssignment_14851 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLMacroCall__RelativeAssignment_24895 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_34934 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_rule__WMLMacroCall__ParametersAssignment_44965 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_14996 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment_05027 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_rule__WMLMacroDefine__ExpressionsAssignment_15058 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ENDDEF_in_rule__WMLMacroDefine__EndNameAssignment_25089 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__NameAlternatives_0_0_in_rule__WMLPreprocIF__NameAssignment_05120 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ExpressionsAssignment_15153 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ELSE_in_rule__WMLPreprocIF__ElsesAssignment_2_05184 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ElseExpressionsAssignment_2_15215 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ENDIF_in_rule__WMLPreprocIF__EndNameAssignment_35246 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment5277 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment5308 = new BitSet(new long[]{0x0000000000000002L});

}