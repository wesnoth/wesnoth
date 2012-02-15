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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:91:1: entryRuleWMLTag : ruleWMLTag EOF ;
    public final void entryRuleWMLTag() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:92:1: ( ruleWMLTag EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:93:1: ruleWMLTag EOF
            {
             before(grammarAccess.getWMLTagRule()); 
            pushFollow(FOLLOW_ruleWMLTag_in_entryRuleWMLTag124);
            ruleWMLTag();

            state._fsp--;

             after(grammarAccess.getWMLTagRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTag131); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:100:1: ruleWMLTag : ( ( rule__WMLTag__Group__0 ) ) ;
    public final void ruleWMLTag() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:104:2: ( ( ( rule__WMLTag__Group__0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:105:1: ( ( rule__WMLTag__Group__0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:105:1: ( ( rule__WMLTag__Group__0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:106:1: ( rule__WMLTag__Group__0 )
            {
             before(grammarAccess.getWMLTagAccess().getGroup()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:107:1: ( rule__WMLTag__Group__0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:107:2: rule__WMLTag__Group__0
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0_in_ruleWMLTag157);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:119:1: entryRuleWMLKey : ruleWMLKey EOF ;
    public final void entryRuleWMLKey() throws RecognitionException {

        	HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");

        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:123:1: ( ruleWMLKey EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:124:1: ruleWMLKey EOF
            {
             before(grammarAccess.getWMLKeyRule()); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey189);
            ruleWMLKey();

            state._fsp--;

             after(grammarAccess.getWMLKeyRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey196); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:134:1: ruleWMLKey : ( ( rule__WMLKey__Group__0 ) ) ;
    public final void ruleWMLKey() throws RecognitionException {

        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:139:2: ( ( ( rule__WMLKey__Group__0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:140:1: ( ( rule__WMLKey__Group__0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:140:1: ( ( rule__WMLKey__Group__0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:141:1: ( rule__WMLKey__Group__0 )
            {
             before(grammarAccess.getWMLKeyAccess().getGroup()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:142:1: ( rule__WMLKey__Group__0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:142:2: rule__WMLKey__Group__0
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey226);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:155:1: entryRuleWMLKeyValue : ruleWMLKeyValue EOF ;
    public final void entryRuleWMLKeyValue() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:156:1: ( ruleWMLKeyValue EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:157:1: ruleWMLKeyValue EOF
            {
             before(grammarAccess.getWMLKeyValueRule()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue253);
            ruleWMLKeyValue();

            state._fsp--;

             after(grammarAccess.getWMLKeyValueRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue260); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:164:1: ruleWMLKeyValue : ( ( rule__WMLKeyValue__Alternatives ) ) ;
    public final void ruleWMLKeyValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:168:2: ( ( ( rule__WMLKeyValue__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:169:1: ( ( rule__WMLKeyValue__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:169:1: ( ( rule__WMLKeyValue__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:170:1: ( rule__WMLKeyValue__Alternatives )
            {
             before(grammarAccess.getWMLKeyValueAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:171:1: ( rule__WMLKeyValue__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:171:2: rule__WMLKeyValue__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue286);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:183:1: entryRuleWMLMacroCall : ruleWMLMacroCall EOF ;
    public final void entryRuleWMLMacroCall() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:184:1: ( ruleWMLMacroCall EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:185:1: ruleWMLMacroCall EOF
            {
             before(grammarAccess.getWMLMacroCallRule()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall313);
            ruleWMLMacroCall();

            state._fsp--;

             after(grammarAccess.getWMLMacroCallRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCall320); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:192:1: ruleWMLMacroCall : ( ( rule__WMLMacroCall__Group__0 ) ) ;
    public final void ruleWMLMacroCall() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:196:2: ( ( ( rule__WMLMacroCall__Group__0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:197:1: ( ( rule__WMLMacroCall__Group__0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:197:1: ( ( rule__WMLMacroCall__Group__0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:198:1: ( rule__WMLMacroCall__Group__0 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getGroup()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:199:1: ( rule__WMLMacroCall__Group__0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:199:2: rule__WMLMacroCall__Group__0
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__0_in_ruleWMLMacroCall346);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:211:1: entryRuleWMLMacroCallParameter : ruleWMLMacroCallParameter EOF ;
    public final void entryRuleWMLMacroCallParameter() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:212:1: ( ruleWMLMacroCallParameter EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:213:1: ruleWMLMacroCallParameter EOF
            {
             before(grammarAccess.getWMLMacroCallParameterRule()); 
            pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_entryRuleWMLMacroCallParameter373);
            ruleWMLMacroCallParameter();

            state._fsp--;

             after(grammarAccess.getWMLMacroCallParameterRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCallParameter380); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:220:1: ruleWMLMacroCallParameter : ( ( rule__WMLMacroCallParameter__Alternatives ) ) ;
    public final void ruleWMLMacroCallParameter() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:224:2: ( ( ( rule__WMLMacroCallParameter__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:225:1: ( ( rule__WMLMacroCallParameter__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:225:1: ( ( rule__WMLMacroCallParameter__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:226:1: ( rule__WMLMacroCallParameter__Alternatives )
            {
             before(grammarAccess.getWMLMacroCallParameterAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:227:1: ( rule__WMLMacroCallParameter__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:227:2: rule__WMLMacroCallParameter__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLMacroCallParameter__Alternatives_in_ruleWMLMacroCallParameter406);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:239:1: entryRuleWMLArrayCall : ruleWMLArrayCall EOF ;
    public final void entryRuleWMLArrayCall() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:240:1: ( ruleWMLArrayCall EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:241:1: ruleWMLArrayCall EOF
            {
             before(grammarAccess.getWMLArrayCallRule()); 
            pushFollow(FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall433);
            ruleWMLArrayCall();

            state._fsp--;

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
    // $ANTLR end "entryRuleWMLArrayCall"


    // $ANTLR start "ruleWMLArrayCall"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:248:1: ruleWMLArrayCall : ( ( rule__WMLArrayCall__Group__0 ) ) ;
    public final void ruleWMLArrayCall() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:252:2: ( ( ( rule__WMLArrayCall__Group__0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:253:1: ( ( rule__WMLArrayCall__Group__0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:253:1: ( ( rule__WMLArrayCall__Group__0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:254:1: ( rule__WMLArrayCall__Group__0 )
            {
             before(grammarAccess.getWMLArrayCallAccess().getGroup()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:255:1: ( rule__WMLArrayCall__Group__0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:255:2: rule__WMLArrayCall__Group__0
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__0_in_ruleWMLArrayCall466);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:267:1: entryRuleWMLMacroDefine : ruleWMLMacroDefine EOF ;
    public final void entryRuleWMLMacroDefine() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:268:1: ( ruleWMLMacroDefine EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:269:1: ruleWMLMacroDefine EOF
            {
             before(grammarAccess.getWMLMacroDefineRule()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine493);
            ruleWMLMacroDefine();

            state._fsp--;

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
    // $ANTLR end "entryRuleWMLMacroDefine"


    // $ANTLR start "ruleWMLMacroDefine"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:276:1: ruleWMLMacroDefine : ( ( rule__WMLMacroDefine__Group__0 ) ) ;
    public final void ruleWMLMacroDefine() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:280:2: ( ( ( rule__WMLMacroDefine__Group__0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:1: ( ( rule__WMLMacroDefine__Group__0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:281:1: ( ( rule__WMLMacroDefine__Group__0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:282:1: ( rule__WMLMacroDefine__Group__0 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getGroup()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:283:1: ( rule__WMLMacroDefine__Group__0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:283:2: rule__WMLMacroDefine__Group__0
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__0_in_ruleWMLMacroDefine526);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:295:1: entryRuleWMLPreprocIF : ruleWMLPreprocIF EOF ;
    public final void entryRuleWMLPreprocIF() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:296:1: ( ruleWMLPreprocIF EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:297:1: ruleWMLPreprocIF EOF
            {
             before(grammarAccess.getWMLPreprocIFRule()); 
            pushFollow(FOLLOW_ruleWMLPreprocIF_in_entryRuleWMLPreprocIF553);
            ruleWMLPreprocIF();

            state._fsp--;

             after(grammarAccess.getWMLPreprocIFRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLPreprocIF560); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:304:1: ruleWMLPreprocIF : ( ( rule__WMLPreprocIF__Group__0 ) ) ;
    public final void ruleWMLPreprocIF() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:308:2: ( ( ( rule__WMLPreprocIF__Group__0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:1: ( ( rule__WMLPreprocIF__Group__0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:309:1: ( ( rule__WMLPreprocIF__Group__0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:310:1: ( rule__WMLPreprocIF__Group__0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getGroup()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:311:1: ( rule__WMLPreprocIF__Group__0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:311:2: rule__WMLPreprocIF__Group__0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__0_in_ruleWMLPreprocIF586);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:323:1: entryRuleWMLRootExpression : ruleWMLRootExpression EOF ;
    public final void entryRuleWMLRootExpression() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:324:1: ( ruleWMLRootExpression EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:325:1: ruleWMLRootExpression EOF
            {
             before(grammarAccess.getWMLRootExpressionRule()); 
            pushFollow(FOLLOW_ruleWMLRootExpression_in_entryRuleWMLRootExpression613);
            ruleWMLRootExpression();

            state._fsp--;

             after(grammarAccess.getWMLRootExpressionRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLRootExpression620); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:332:1: ruleWMLRootExpression : ( ( rule__WMLRootExpression__Alternatives ) ) ;
    public final void ruleWMLRootExpression() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:336:2: ( ( ( rule__WMLRootExpression__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:1: ( ( rule__WMLRootExpression__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:337:1: ( ( rule__WMLRootExpression__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:338:1: ( rule__WMLRootExpression__Alternatives )
            {
             before(grammarAccess.getWMLRootExpressionAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:339:1: ( rule__WMLRootExpression__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:339:2: rule__WMLRootExpression__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLRootExpression__Alternatives_in_ruleWMLRootExpression646);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:351:1: entryRuleWMLExpression : ruleWMLExpression EOF ;
    public final void entryRuleWMLExpression() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:352:1: ( ruleWMLExpression EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:353:1: ruleWMLExpression EOF
            {
             before(grammarAccess.getWMLExpressionRule()); 
            pushFollow(FOLLOW_ruleWMLExpression_in_entryRuleWMLExpression673);
            ruleWMLExpression();

            state._fsp--;

             after(grammarAccess.getWMLExpressionRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLExpression680); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:360:1: ruleWMLExpression : ( ( rule__WMLExpression__Alternatives ) ) ;
    public final void ruleWMLExpression() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:364:2: ( ( ( rule__WMLExpression__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:365:1: ( ( rule__WMLExpression__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:365:1: ( ( rule__WMLExpression__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:366:1: ( rule__WMLExpression__Alternatives )
            {
             before(grammarAccess.getWMLExpressionAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:367:1: ( rule__WMLExpression__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:367:2: rule__WMLExpression__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLExpression__Alternatives_in_ruleWMLExpression706);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:379:1: entryRuleWMLValuedExpression : ruleWMLValuedExpression EOF ;
    public final void entryRuleWMLValuedExpression() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:380:1: ( ruleWMLValuedExpression EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:381:1: ruleWMLValuedExpression EOF
            {
             before(grammarAccess.getWMLValuedExpressionRule()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_entryRuleWMLValuedExpression733);
            ruleWMLValuedExpression();

            state._fsp--;

             after(grammarAccess.getWMLValuedExpressionRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValuedExpression740); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:388:1: ruleWMLValuedExpression : ( ( rule__WMLValuedExpression__Alternatives ) ) ;
    public final void ruleWMLValuedExpression() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:392:2: ( ( ( rule__WMLValuedExpression__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:393:1: ( ( rule__WMLValuedExpression__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:393:1: ( ( rule__WMLValuedExpression__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:394:1: ( rule__WMLValuedExpression__Alternatives )
            {
             before(grammarAccess.getWMLValuedExpressionAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:395:1: ( rule__WMLValuedExpression__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:395:2: rule__WMLValuedExpression__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLValuedExpression__Alternatives_in_ruleWMLValuedExpression766);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:407:1: entryRuleWMLTextdomain : ruleWMLTextdomain EOF ;
    public final void entryRuleWMLTextdomain() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:408:1: ( ruleWMLTextdomain EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:409:1: ruleWMLTextdomain EOF
            {
             before(grammarAccess.getWMLTextdomainRule()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain793);
            ruleWMLTextdomain();

            state._fsp--;

             after(grammarAccess.getWMLTextdomainRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTextdomain800); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:416:1: ruleWMLTextdomain : ( ( rule__WMLTextdomain__NameAssignment ) ) ;
    public final void ruleWMLTextdomain() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:420:2: ( ( ( rule__WMLTextdomain__NameAssignment ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:421:1: ( ( rule__WMLTextdomain__NameAssignment ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:421:1: ( ( rule__WMLTextdomain__NameAssignment ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:422:1: ( rule__WMLTextdomain__NameAssignment )
            {
             before(grammarAccess.getWMLTextdomainAccess().getNameAssignment()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:423:1: ( rule__WMLTextdomain__NameAssignment )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:423:2: rule__WMLTextdomain__NameAssignment
            {
            pushFollow(FOLLOW_rule__WMLTextdomain__NameAssignment_in_ruleWMLTextdomain826);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:435:1: entryRuleWMLLuaCode : ruleWMLLuaCode EOF ;
    public final void entryRuleWMLLuaCode() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:436:1: ( ruleWMLLuaCode EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:437:1: ruleWMLLuaCode EOF
            {
             before(grammarAccess.getWMLLuaCodeRule()); 
            pushFollow(FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode853);
            ruleWMLLuaCode();

            state._fsp--;

             after(grammarAccess.getWMLLuaCodeRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLLuaCode860); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:444:1: ruleWMLLuaCode : ( ( rule__WMLLuaCode__ValueAssignment ) ) ;
    public final void ruleWMLLuaCode() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:448:2: ( ( ( rule__WMLLuaCode__ValueAssignment ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:449:1: ( ( rule__WMLLuaCode__ValueAssignment ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:449:1: ( ( rule__WMLLuaCode__ValueAssignment ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:450:1: ( rule__WMLLuaCode__ValueAssignment )
            {
             before(grammarAccess.getWMLLuaCodeAccess().getValueAssignment()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:451:1: ( rule__WMLLuaCode__ValueAssignment )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:451:2: rule__WMLLuaCode__ValueAssignment
            {
            pushFollow(FOLLOW_rule__WMLLuaCode__ValueAssignment_in_ruleWMLLuaCode886);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:463:1: entryRuleWMLMacroParameter : ruleWMLMacroParameter EOF ;
    public final void entryRuleWMLMacroParameter() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:464:1: ( ruleWMLMacroParameter EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:465:1: ruleWMLMacroParameter EOF
            {
             before(grammarAccess.getWMLMacroParameterRule()); 
            pushFollow(FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter913);
            ruleWMLMacroParameter();

            state._fsp--;

             after(grammarAccess.getWMLMacroParameterRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroParameter920); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:472:1: ruleWMLMacroParameter : ( ( rule__WMLMacroParameter__Alternatives ) ) ;
    public final void ruleWMLMacroParameter() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:476:2: ( ( ( rule__WMLMacroParameter__Alternatives ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:477:1: ( ( rule__WMLMacroParameter__Alternatives ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:477:1: ( ( rule__WMLMacroParameter__Alternatives ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:478:1: ( rule__WMLMacroParameter__Alternatives )
            {
             before(grammarAccess.getWMLMacroParameterAccess().getAlternatives()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:479:1: ( rule__WMLMacroParameter__Alternatives )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:479:2: rule__WMLMacroParameter__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLMacroParameter__Alternatives_in_ruleWMLMacroParameter946);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:491:1: entryRuleWMLValue : ruleWMLValue EOF ;
    public final void entryRuleWMLValue() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:492:1: ( ruleWMLValue EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:493:1: ruleWMLValue EOF
            {
             before(grammarAccess.getWMLValueRule()); 
            pushFollow(FOLLOW_ruleWMLValue_in_entryRuleWMLValue973);
            ruleWMLValue();

            state._fsp--;

             after(grammarAccess.getWMLValueRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValue980); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:500:1: ruleWMLValue : ( ( rule__WMLValue__ValueAssignment ) ) ;
    public final void ruleWMLValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:504:2: ( ( ( rule__WMLValue__ValueAssignment ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:505:1: ( ( rule__WMLValue__ValueAssignment ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:505:1: ( ( rule__WMLValue__ValueAssignment ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:506:1: ( rule__WMLValue__ValueAssignment )
            {
             before(grammarAccess.getWMLValueAccess().getValueAssignment()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:507:1: ( rule__WMLValue__ValueAssignment )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:507:2: rule__WMLValue__ValueAssignment
            {
            pushFollow(FOLLOW_rule__WMLValue__ValueAssignment_in_ruleWMLValue1006);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:519:1: entryRuleMacroTokens : ruleMacroTokens EOF ;
    public final void entryRuleMacroTokens() throws RecognitionException {
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:520:1: ( ruleMacroTokens EOF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:521:1: ruleMacroTokens EOF
            {
             before(grammarAccess.getMacroTokensRule()); 
            pushFollow(FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens1033);
            ruleMacroTokens();

            state._fsp--;

             after(grammarAccess.getMacroTokensRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleMacroTokens1040); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:528:1: ruleMacroTokens : ( ( rule__MacroTokens__ValueAssignment ) ) ;
    public final void ruleMacroTokens() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:532:2: ( ( ( rule__MacroTokens__ValueAssignment ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:1: ( ( rule__MacroTokens__ValueAssignment ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:1: ( ( rule__MacroTokens__ValueAssignment ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:534:1: ( rule__MacroTokens__ValueAssignment )
            {
             before(grammarAccess.getMacroTokensAccess().getValueAssignment()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:535:1: ( rule__MacroTokens__ValueAssignment )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:535:2: rule__MacroTokens__ValueAssignment
            {
            pushFollow(FOLLOW_rule__MacroTokens__ValueAssignment_in_ruleMacroTokens1066);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:548:1: rule__WMLKey__EolAlternatives_4_0 : ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) );
    public final void rule__WMLKey__EolAlternatives_4_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:552:1: ( ( RULE_EOL ) | ( RULE_SL_COMMENT ) )
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:553:1: ( RULE_EOL )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:553:1: ( RULE_EOL )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:554:1: RULE_EOL
                    {
                     before(grammarAccess.getWMLKeyAccess().getEolEOLTerminalRuleCall_4_0_0()); 
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__EolAlternatives_4_01103); 
                     after(grammarAccess.getWMLKeyAccess().getEolEOLTerminalRuleCall_4_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:559:6: ( RULE_SL_COMMENT )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:559:6: ( RULE_SL_COMMENT )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:560:1: RULE_SL_COMMENT
                    {
                     before(grammarAccess.getWMLKeyAccess().getEolSL_COMMENTTerminalRuleCall_4_0_1()); 
                    match(input,RULE_SL_COMMENT,FOLLOW_RULE_SL_COMMENT_in_rule__WMLKey__EolAlternatives_4_01120); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:570:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) );
    public final void rule__WMLKeyValue__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:574:1: ( ( ruleWMLValue ) | ( ruleWMLMacroCall ) | ( ruleWMLLuaCode ) | ( ruleWMLArrayCall ) )
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:575:1: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:575:1: ( ruleWMLValue )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:576:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLKeyValue__Alternatives1152);
                    ruleWMLValue();

                    state._fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:581:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:581:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:582:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives1169);
                    ruleWMLMacroCall();

                    state._fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:587:6: ( ruleWMLLuaCode )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:587:6: ( ruleWMLLuaCode )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:588:1: ruleWMLLuaCode
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2()); 
                    pushFollow(FOLLOW_ruleWMLLuaCode_in_rule__WMLKeyValue__Alternatives1186);
                    ruleWMLLuaCode();

                    state._fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:593:6: ( ruleWMLArrayCall )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:593:6: ( ruleWMLArrayCall )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:594:1: ruleWMLArrayCall
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLArrayCallParserRuleCall_3()); 
                    pushFollow(FOLLOW_ruleWMLArrayCall_in_rule__WMLKeyValue__Alternatives1203);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:604:1: rule__WMLMacroCallParameter__Alternatives : ( ( ruleWMLMacroParameter ) | ( ruleWMLMacroCall ) );
    public final void rule__WMLMacroCallParameter__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:608:1: ( ( ruleWMLMacroParameter ) | ( ruleWMLMacroCall ) )
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:609:1: ( ruleWMLMacroParameter )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:609:1: ( ruleWMLMacroParameter )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:610:1: ruleWMLMacroParameter
                    {
                     before(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroParameterParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLMacroParameter_in_rule__WMLMacroCallParameter__Alternatives1235);
                    ruleWMLMacroParameter();

                    state._fsp--;

                     after(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroParameterParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:615:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:615:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:616:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroCallParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCallParameter__Alternatives1252);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:626:1: rule__WMLPreprocIF__NameAlternatives_0_0 : ( ( RULE_IFDEF ) | ( RULE_IFNDEF ) | ( RULE_IFHAVE ) | ( RULE_IFNHAVE ) );
    public final void rule__WMLPreprocIF__NameAlternatives_0_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:630:1: ( ( RULE_IFDEF ) | ( RULE_IFNDEF ) | ( RULE_IFHAVE ) | ( RULE_IFNHAVE ) )
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:631:1: ( RULE_IFDEF )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:631:1: ( RULE_IFDEF )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:632:1: RULE_IFDEF
                    {
                     before(grammarAccess.getWMLPreprocIFAccess().getNameIFDEFTerminalRuleCall_0_0_0()); 
                    match(input,RULE_IFDEF,FOLLOW_RULE_IFDEF_in_rule__WMLPreprocIF__NameAlternatives_0_01284); 
                     after(grammarAccess.getWMLPreprocIFAccess().getNameIFDEFTerminalRuleCall_0_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:637:6: ( RULE_IFNDEF )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:637:6: ( RULE_IFNDEF )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:638:1: RULE_IFNDEF
                    {
                     before(grammarAccess.getWMLPreprocIFAccess().getNameIFNDEFTerminalRuleCall_0_0_1()); 
                    match(input,RULE_IFNDEF,FOLLOW_RULE_IFNDEF_in_rule__WMLPreprocIF__NameAlternatives_0_01301); 
                     after(grammarAccess.getWMLPreprocIFAccess().getNameIFNDEFTerminalRuleCall_0_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:643:6: ( RULE_IFHAVE )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:643:6: ( RULE_IFHAVE )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:644:1: RULE_IFHAVE
                    {
                     before(grammarAccess.getWMLPreprocIFAccess().getNameIFHAVETerminalRuleCall_0_0_2()); 
                    match(input,RULE_IFHAVE,FOLLOW_RULE_IFHAVE_in_rule__WMLPreprocIF__NameAlternatives_0_01318); 
                     after(grammarAccess.getWMLPreprocIFAccess().getNameIFHAVETerminalRuleCall_0_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:649:6: ( RULE_IFNHAVE )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:649:6: ( RULE_IFNHAVE )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:650:1: RULE_IFNHAVE
                    {
                     before(grammarAccess.getWMLPreprocIFAccess().getNameIFNHAVETerminalRuleCall_0_0_3()); 
                    match(input,RULE_IFNHAVE,FOLLOW_RULE_IFNHAVE_in_rule__WMLPreprocIF__NameAlternatives_0_01335); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:660:1: rule__WMLRootExpression__Alternatives : ( ( ruleWMLTag ) | ( ruleWMLMacroCall ) | ( ruleWMLMacroDefine ) | ( ruleWMLTextdomain ) | ( ruleWMLPreprocIF ) );
    public final void rule__WMLRootExpression__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:664:1: ( ( ruleWMLTag ) | ( ruleWMLMacroCall ) | ( ruleWMLMacroDefine ) | ( ruleWMLTextdomain ) | ( ruleWMLPreprocIF ) )
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:665:1: ( ruleWMLTag )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:665:1: ( ruleWMLTag )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:666:1: ruleWMLTag
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLTagParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRootExpression__Alternatives1367);
                    ruleWMLTag();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLTagParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:671:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:671:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:672:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLMacroCallParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLRootExpression__Alternatives1384);
                    ruleWMLMacroCall();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLMacroCallParserRuleCall_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:677:6: ( ruleWMLMacroDefine )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:677:6: ( ruleWMLMacroDefine )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:678:1: ruleWMLMacroDefine
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLMacroDefineParserRuleCall_2()); 
                    pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLRootExpression__Alternatives1401);
                    ruleWMLMacroDefine();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLMacroDefineParserRuleCall_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:683:6: ( ruleWMLTextdomain )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:683:6: ( ruleWMLTextdomain )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:684:1: ruleWMLTextdomain
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLTextdomainParserRuleCall_3()); 
                    pushFollow(FOLLOW_ruleWMLTextdomain_in_rule__WMLRootExpression__Alternatives1418);
                    ruleWMLTextdomain();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLTextdomainParserRuleCall_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:689:6: ( ruleWMLPreprocIF )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:689:6: ( ruleWMLPreprocIF )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:690:1: ruleWMLPreprocIF
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLPreprocIFParserRuleCall_4()); 
                    pushFollow(FOLLOW_ruleWMLPreprocIF_in_rule__WMLRootExpression__Alternatives1435);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:700:1: rule__WMLExpression__Alternatives : ( ( ruleWMLRootExpression ) | ( ruleWMLKey ) );
    public final void rule__WMLExpression__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:704:1: ( ( ruleWMLRootExpression ) | ( ruleWMLKey ) )
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:705:1: ( ruleWMLRootExpression )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:705:1: ( ruleWMLRootExpression )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:706:1: ruleWMLRootExpression
                    {
                     before(grammarAccess.getWMLExpressionAccess().getWMLRootExpressionParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLRootExpression_in_rule__WMLExpression__Alternatives1467);
                    ruleWMLRootExpression();

                    state._fsp--;

                     after(grammarAccess.getWMLExpressionAccess().getWMLRootExpressionParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:711:6: ( ruleWMLKey )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:711:6: ( ruleWMLKey )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:712:1: ruleWMLKey
                    {
                     before(grammarAccess.getWMLExpressionAccess().getWMLKeyParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLExpression__Alternatives1484);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:722:1: rule__WMLValuedExpression__Alternatives : ( ( ruleWMLExpression ) | ( ruleWMLValue ) );
    public final void rule__WMLValuedExpression__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:726:1: ( ( ruleWMLExpression ) | ( ruleWMLValue ) )
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:727:1: ( ruleWMLExpression )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:727:1: ( ruleWMLExpression )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:728:1: ruleWMLExpression
                    {
                     before(grammarAccess.getWMLValuedExpressionAccess().getWMLExpressionParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLExpression_in_rule__WMLValuedExpression__Alternatives1516);
                    ruleWMLExpression();

                    state._fsp--;

                     after(grammarAccess.getWMLValuedExpressionAccess().getWMLExpressionParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:733:6: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:733:6: ( ruleWMLValue )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:734:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLValuedExpressionAccess().getWMLValueParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLValuedExpression__Alternatives1533);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:744:1: rule__WMLMacroParameter__Alternatives : ( ( ruleWMLValue ) | ( ruleMacroTokens ) );
    public final void rule__WMLMacroParameter__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:748:1: ( ( ruleWMLValue ) | ( ruleMacroTokens ) )
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:749:1: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:749:1: ( ruleWMLValue )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:750:1: ruleWMLValue
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:755:6: ( ruleMacroTokens )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:755:6: ( ruleMacroTokens )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:756:1: ruleMacroTokens
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


    // $ANTLR start "rule__WMLValue__ValueAlternatives_0"
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:766:1: rule__WMLValue__ValueAlternatives_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( '~' ) | ( '.' ) | ( './' ) | ( '$' ) | ( '/' ) | ( '(' ) | ( ')' ) | ( RULE_ANY_OTHER ) );
    public final void rule__WMLValue__ValueAlternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:770:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( '~' ) | ( '.' ) | ( './' ) | ( '$' ) | ( '/' ) | ( '(' ) | ( ')' ) | ( RULE_ANY_OTHER ) )
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:771:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:771:1: ( RULE_ID )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:772:1: RULE_ID
                    {
                     before(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLValue__ValueAlternatives_01614); 
                     after(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:777:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:777:6: ( RULE_STRING )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:778:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLValue__ValueAlternatives_01631); 
                     after(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:783:6: ( '_' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:783:6: ( '_' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:784:1: '_'
                    {
                     before(grammarAccess.getWMLValueAccess().getValue_Keyword_0_2()); 
                    match(input,20,FOLLOW_20_in_rule__WMLValue__ValueAlternatives_01649); 
                     after(grammarAccess.getWMLValueAccess().getValue_Keyword_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:791:6: ( '~' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:791:6: ( '~' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:792:1: '~'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_3()); 
                    match(input,21,FOLLOW_21_in_rule__WMLValue__ValueAlternatives_01669); 
                     after(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:799:6: ( '.' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:799:6: ( '.' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:800:1: '.'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueFullStopKeyword_0_4()); 
                    match(input,22,FOLLOW_22_in_rule__WMLValue__ValueAlternatives_01689); 
                     after(grammarAccess.getWMLValueAccess().getValueFullStopKeyword_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:807:6: ( './' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:807:6: ( './' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:808:1: './'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueFullStopSolidusKeyword_0_5()); 
                    match(input,23,FOLLOW_23_in_rule__WMLValue__ValueAlternatives_01709); 
                     after(grammarAccess.getWMLValueAccess().getValueFullStopSolidusKeyword_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:815:6: ( '$' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:815:6: ( '$' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:816:1: '$'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueDollarSignKeyword_0_6()); 
                    match(input,24,FOLLOW_24_in_rule__WMLValue__ValueAlternatives_01729); 
                     after(grammarAccess.getWMLValueAccess().getValueDollarSignKeyword_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:823:6: ( '/' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:823:6: ( '/' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:824:1: '/'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueSolidusKeyword_0_7()); 
                    match(input,25,FOLLOW_25_in_rule__WMLValue__ValueAlternatives_01749); 
                     after(grammarAccess.getWMLValueAccess().getValueSolidusKeyword_0_7()); 

                    }


                    }
                    break;
                case 9 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:831:6: ( '(' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:831:6: ( '(' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:832:1: '('
                    {
                     before(grammarAccess.getWMLValueAccess().getValueLeftParenthesisKeyword_0_8()); 
                    match(input,26,FOLLOW_26_in_rule__WMLValue__ValueAlternatives_01769); 
                     after(grammarAccess.getWMLValueAccess().getValueLeftParenthesisKeyword_0_8()); 

                    }


                    }
                    break;
                case 10 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:839:6: ( ')' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:839:6: ( ')' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:840:1: ')'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueRightParenthesisKeyword_0_9()); 
                    match(input,27,FOLLOW_27_in_rule__WMLValue__ValueAlternatives_01789); 
                     after(grammarAccess.getWMLValueAccess().getValueRightParenthesisKeyword_0_9()); 

                    }


                    }
                    break;
                case 11 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:847:6: ( RULE_ANY_OTHER )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:847:6: ( RULE_ANY_OTHER )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:848:1: RULE_ANY_OTHER
                    {
                     before(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_10()); 
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__ValueAlternatives_01808); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:858:1: rule__MacroTokens__ValueAlternatives_0 : ( ( '=' ) | ( '[' ) | ( ']' ) | ( '+' ) | ( '[/' ) );
    public final void rule__MacroTokens__ValueAlternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:862:1: ( ( '=' ) | ( '[' ) | ( ']' ) | ( '+' ) | ( '[/' ) )
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:863:1: ( '=' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:863:1: ( '=' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:864:1: '='
                    {
                     before(grammarAccess.getMacroTokensAccess().getValueEqualsSignKeyword_0_0()); 
                    match(input,28,FOLLOW_28_in_rule__MacroTokens__ValueAlternatives_01841); 
                     after(grammarAccess.getMacroTokensAccess().getValueEqualsSignKeyword_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:871:6: ( '[' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:871:6: ( '[' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:872:1: '['
                    {
                     before(grammarAccess.getMacroTokensAccess().getValueLeftSquareBracketKeyword_0_1()); 
                    match(input,29,FOLLOW_29_in_rule__MacroTokens__ValueAlternatives_01861); 
                     after(grammarAccess.getMacroTokensAccess().getValueLeftSquareBracketKeyword_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:879:6: ( ']' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:879:6: ( ']' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:880:1: ']'
                    {
                     before(grammarAccess.getMacroTokensAccess().getValueRightSquareBracketKeyword_0_2()); 
                    match(input,30,FOLLOW_30_in_rule__MacroTokens__ValueAlternatives_01881); 
                     after(grammarAccess.getMacroTokensAccess().getValueRightSquareBracketKeyword_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:887:6: ( '+' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:887:6: ( '+' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:888:1: '+'
                    {
                     before(grammarAccess.getMacroTokensAccess().getValuePlusSignKeyword_0_3()); 
                    match(input,31,FOLLOW_31_in_rule__MacroTokens__ValueAlternatives_01901); 
                     after(grammarAccess.getMacroTokensAccess().getValuePlusSignKeyword_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:895:6: ( '[/' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:895:6: ( '[/' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:896:1: '[/'
                    {
                     before(grammarAccess.getMacroTokensAccess().getValueLeftSquareBracketSolidusKeyword_0_4()); 
                    match(input,32,FOLLOW_32_in_rule__MacroTokens__ValueAlternatives_01921); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:910:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:914:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:915:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:922:1: rule__WMLTag__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:926:1: ( ( '[' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:927:1: ( '[' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:927:1: ( '[' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:928:1: '['
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:941:1: rule__WMLTag__Group__1 : rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 ;
    public final void rule__WMLTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:945:1: ( rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:946:2: rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:953:1: rule__WMLTag__Group__1__Impl : ( ( rule__WMLTag__PlusAssignment_1 )? ) ;
    public final void rule__WMLTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:957:1: ( ( ( rule__WMLTag__PlusAssignment_1 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:958:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:958:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:959:1: ( rule__WMLTag__PlusAssignment_1 )?
            {
             before(grammarAccess.getWMLTagAccess().getPlusAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:960:1: ( rule__WMLTag__PlusAssignment_1 )?
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0==31) ) {
                alt12=1;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:960:2: rule__WMLTag__PlusAssignment_1
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:970:1: rule__WMLTag__Group__2 : rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 ;
    public final void rule__WMLTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:974:1: ( rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:975:2: rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:982:1: rule__WMLTag__Group__2__Impl : ( ( rule__WMLTag__NameAssignment_2 ) ) ;
    public final void rule__WMLTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:986:1: ( ( ( rule__WMLTag__NameAssignment_2 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:987:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:987:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:988:1: ( rule__WMLTag__NameAssignment_2 )
            {
             before(grammarAccess.getWMLTagAccess().getNameAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:989:1: ( rule__WMLTag__NameAssignment_2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:989:2: rule__WMLTag__NameAssignment_2
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:999:1: rule__WMLTag__Group__3 : rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 ;
    public final void rule__WMLTag__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1003:1: ( rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1004:2: rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1011:1: rule__WMLTag__Group__3__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1015:1: ( ( ']' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1016:1: ( ']' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1016:1: ( ']' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1017:1: ']'
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1030:1: rule__WMLTag__Group__4 : rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 ;
    public final void rule__WMLTag__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1034:1: ( rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1035:2: rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1042:1: rule__WMLTag__Group__4__Impl : ( ( rule__WMLTag__ExpressionsAssignment_4 )* ) ;
    public final void rule__WMLTag__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1046:1: ( ( ( rule__WMLTag__ExpressionsAssignment_4 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1047:1: ( ( rule__WMLTag__ExpressionsAssignment_4 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1047:1: ( ( rule__WMLTag__ExpressionsAssignment_4 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1048:1: ( rule__WMLTag__ExpressionsAssignment_4 )*
            {
             before(grammarAccess.getWMLTagAccess().getExpressionsAssignment_4()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1049:1: ( rule__WMLTag__ExpressionsAssignment_4 )*
            loop13:
            do {
                int alt13=2;
                int LA13_0 = input.LA(1);

                if ( ((LA13_0>=RULE_IFDEF && LA13_0<=RULE_ID)||LA13_0==RULE_DEFINE||LA13_0==RULE_TEXTDOMAIN||LA13_0==29||LA13_0==33) ) {
                    alt13=1;
                }


                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1049:2: rule__WMLTag__ExpressionsAssignment_4
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1059:1: rule__WMLTag__Group__5 : rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 ;
    public final void rule__WMLTag__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1063:1: ( rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1064:2: rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1071:1: rule__WMLTag__Group__5__Impl : ( '[/' ) ;
    public final void rule__WMLTag__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1075:1: ( ( '[/' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1076:1: ( '[/' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1076:1: ( '[/' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1077:1: '[/'
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1090:1: rule__WMLTag__Group__6 : rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 ;
    public final void rule__WMLTag__Group__6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1094:1: ( rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1095:2: rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1102:1: rule__WMLTag__Group__6__Impl : ( ( rule__WMLTag__EndNameAssignment_6 ) ) ;
    public final void rule__WMLTag__Group__6__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1106:1: ( ( ( rule__WMLTag__EndNameAssignment_6 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1107:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1107:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1108:1: ( rule__WMLTag__EndNameAssignment_6 )
            {
             before(grammarAccess.getWMLTagAccess().getEndNameAssignment_6()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1109:1: ( rule__WMLTag__EndNameAssignment_6 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1109:2: rule__WMLTag__EndNameAssignment_6
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1119:1: rule__WMLTag__Group__7 : rule__WMLTag__Group__7__Impl ;
    public final void rule__WMLTag__Group__7() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1123:1: ( rule__WMLTag__Group__7__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1124:2: rule__WMLTag__Group__7__Impl
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1130:1: rule__WMLTag__Group__7__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__7__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1134:1: ( ( ']' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1135:1: ( ']' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1135:1: ( ']' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1136:1: ']'
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1165:1: rule__WMLKey__Group__0 : rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 ;
    public final void rule__WMLKey__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1169:1: ( rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1170:2: rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1177:1: rule__WMLKey__Group__0__Impl : ( ( rule__WMLKey__NameAssignment_0 ) ) ;
    public final void rule__WMLKey__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1181:1: ( ( ( rule__WMLKey__NameAssignment_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1182:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1182:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1183:1: ( rule__WMLKey__NameAssignment_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getNameAssignment_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1184:1: ( rule__WMLKey__NameAssignment_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1184:2: rule__WMLKey__NameAssignment_0
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1194:1: rule__WMLKey__Group__1 : rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 ;
    public final void rule__WMLKey__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1198:1: ( rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1199:2: rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1206:1: rule__WMLKey__Group__1__Impl : ( '=' ) ;
    public final void rule__WMLKey__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1210:1: ( ( '=' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1211:1: ( '=' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1211:1: ( '=' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1212:1: '='
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1225:1: rule__WMLKey__Group__2 : rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 ;
    public final void rule__WMLKey__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1229:1: ( rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1230:2: rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1237:1: rule__WMLKey__Group__2__Impl : ( ( rule__WMLKey__ValuesAssignment_2 )* ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1241:1: ( ( ( rule__WMLKey__ValuesAssignment_2 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1242:1: ( ( rule__WMLKey__ValuesAssignment_2 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1242:1: ( ( rule__WMLKey__ValuesAssignment_2 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1243:1: ( rule__WMLKey__ValuesAssignment_2 )*
            {
             before(grammarAccess.getWMLKeyAccess().getValuesAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1244:1: ( rule__WMLKey__ValuesAssignment_2 )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( ((LA14_0>=RULE_ID && LA14_0<=RULE_ANY_OTHER)||LA14_0==RULE_LUA_CODE||(LA14_0>=20 && LA14_0<=27)||LA14_0==29||LA14_0==33) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1244:2: rule__WMLKey__ValuesAssignment_2
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__ValuesAssignment_2_in_rule__WMLKey__Group__2__Impl2608);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1254:1: rule__WMLKey__Group__3 : rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4 ;
    public final void rule__WMLKey__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1258:1: ( rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1259:2: rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1266:1: rule__WMLKey__Group__3__Impl : ( ( rule__WMLKey__Group_3__0 )* ) ;
    public final void rule__WMLKey__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1270:1: ( ( ( rule__WMLKey__Group_3__0 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1271:1: ( ( rule__WMLKey__Group_3__0 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1271:1: ( ( rule__WMLKey__Group_3__0 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1272:1: ( rule__WMLKey__Group_3__0 )*
            {
             before(grammarAccess.getWMLKeyAccess().getGroup_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1273:1: ( rule__WMLKey__Group_3__0 )*
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
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1273:2: rule__WMLKey__Group_3__0
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1283:1: rule__WMLKey__Group__4 : rule__WMLKey__Group__4__Impl ;
    public final void rule__WMLKey__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1287:1: ( rule__WMLKey__Group__4__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1288:2: rule__WMLKey__Group__4__Impl
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1294:1: rule__WMLKey__Group__4__Impl : ( ( rule__WMLKey__EolAssignment_4 ) ) ;
    public final void rule__WMLKey__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1298:1: ( ( ( rule__WMLKey__EolAssignment_4 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1299:1: ( ( rule__WMLKey__EolAssignment_4 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1299:1: ( ( rule__WMLKey__EolAssignment_4 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1300:1: ( rule__WMLKey__EolAssignment_4 )
            {
             before(grammarAccess.getWMLKeyAccess().getEolAssignment_4()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1301:1: ( rule__WMLKey__EolAssignment_4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1301:2: rule__WMLKey__EolAssignment_4
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1321:1: rule__WMLKey__Group_3__0 : rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1 ;
    public final void rule__WMLKey__Group_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1325:1: ( rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1326:2: rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1333:1: rule__WMLKey__Group_3__0__Impl : ( ( RULE_EOL )? ) ;
    public final void rule__WMLKey__Group_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1337:1: ( ( ( RULE_EOL )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1338:1: ( ( RULE_EOL )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1338:1: ( ( RULE_EOL )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1339:1: ( RULE_EOL )?
            {
             before(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1340:1: ( RULE_EOL )?
            int alt16=2;
            int LA16_0 = input.LA(1);

            if ( (LA16_0==RULE_EOL) ) {
                alt16=1;
            }
            switch (alt16) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1340:3: RULE_EOL
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1350:1: rule__WMLKey__Group_3__1 : rule__WMLKey__Group_3__1__Impl rule__WMLKey__Group_3__2 ;
    public final void rule__WMLKey__Group_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1354:1: ( rule__WMLKey__Group_3__1__Impl rule__WMLKey__Group_3__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1355:2: rule__WMLKey__Group_3__1__Impl rule__WMLKey__Group_3__2
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1362:1: rule__WMLKey__Group_3__1__Impl : ( '+' ) ;
    public final void rule__WMLKey__Group_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1366:1: ( ( '+' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1367:1: ( '+' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1367:1: ( '+' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1368:1: '+'
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1381:1: rule__WMLKey__Group_3__2 : rule__WMLKey__Group_3__2__Impl rule__WMLKey__Group_3__3 ;
    public final void rule__WMLKey__Group_3__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1385:1: ( rule__WMLKey__Group_3__2__Impl rule__WMLKey__Group_3__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1386:2: rule__WMLKey__Group_3__2__Impl rule__WMLKey__Group_3__3
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1393:1: rule__WMLKey__Group_3__2__Impl : ( ( RULE_EOL )? ) ;
    public final void rule__WMLKey__Group_3__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1397:1: ( ( ( RULE_EOL )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1398:1: ( ( RULE_EOL )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1398:1: ( ( RULE_EOL )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1399:1: ( RULE_EOL )?
            {
             before(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1400:1: ( RULE_EOL )?
            int alt17=2;
            int LA17_0 = input.LA(1);

            if ( (LA17_0==RULE_EOL) ) {
                alt17=1;
            }
            switch (alt17) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1400:3: RULE_EOL
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1410:1: rule__WMLKey__Group_3__3 : rule__WMLKey__Group_3__3__Impl ;
    public final void rule__WMLKey__Group_3__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1414:1: ( rule__WMLKey__Group_3__3__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1415:2: rule__WMLKey__Group_3__3__Impl
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1421:1: rule__WMLKey__Group_3__3__Impl : ( ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* ) ) ;
    public final void rule__WMLKey__Group_3__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1425:1: ( ( ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1426:1: ( ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1426:1: ( ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1427:1: ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1427:1: ( ( rule__WMLKey__ValuesAssignment_3_3 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1428:1: ( rule__WMLKey__ValuesAssignment_3_3 )
            {
             before(grammarAccess.getWMLKeyAccess().getValuesAssignment_3_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1429:1: ( rule__WMLKey__ValuesAssignment_3_3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1429:2: rule__WMLKey__ValuesAssignment_3_3
            {
            pushFollow(FOLLOW_rule__WMLKey__ValuesAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2982);
            rule__WMLKey__ValuesAssignment_3_3();

            state._fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getValuesAssignment_3_3()); 

            }

            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1432:1: ( ( rule__WMLKey__ValuesAssignment_3_3 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1433:1: ( rule__WMLKey__ValuesAssignment_3_3 )*
            {
             before(grammarAccess.getWMLKeyAccess().getValuesAssignment_3_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1434:1: ( rule__WMLKey__ValuesAssignment_3_3 )*
            loop18:
            do {
                int alt18=2;
                int LA18_0 = input.LA(1);

                if ( ((LA18_0>=RULE_ID && LA18_0<=RULE_ANY_OTHER)||LA18_0==RULE_LUA_CODE||(LA18_0>=20 && LA18_0<=27)||LA18_0==29||LA18_0==33) ) {
                    alt18=1;
                }


                switch (alt18) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1434:2: rule__WMLKey__ValuesAssignment_3_3
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__ValuesAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2994);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1453:1: rule__WMLMacroCall__Group__0 : rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 ;
    public final void rule__WMLMacroCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1457:1: ( rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1458:2: rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__03035);
            rule__WMLMacroCall__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__03038);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1465:1: rule__WMLMacroCall__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacroCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1469:1: ( ( '{' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1470:1: ( '{' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1470:1: ( '{' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1471:1: '{'
            {
             before(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,33,FOLLOW_33_in_rule__WMLMacroCall__Group__0__Impl3066); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1484:1: rule__WMLMacroCall__Group__1 : rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 ;
    public final void rule__WMLMacroCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1488:1: ( rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1489:2: rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__13097);
            rule__WMLMacroCall__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__13100);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1496:1: rule__WMLMacroCall__Group__1__Impl : ( ( rule__WMLMacroCall__PointAssignment_1 )? ) ;
    public final void rule__WMLMacroCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1500:1: ( ( ( rule__WMLMacroCall__PointAssignment_1 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1501:1: ( ( rule__WMLMacroCall__PointAssignment_1 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1501:1: ( ( rule__WMLMacroCall__PointAssignment_1 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1502:1: ( rule__WMLMacroCall__PointAssignment_1 )?
            {
             before(grammarAccess.getWMLMacroCallAccess().getPointAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1503:1: ( rule__WMLMacroCall__PointAssignment_1 )?
            int alt19=2;
            int LA19_0 = input.LA(1);

            if ( (LA19_0==23) ) {
                alt19=1;
            }
            switch (alt19) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1503:2: rule__WMLMacroCall__PointAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__PointAssignment_1_in_rule__WMLMacroCall__Group__1__Impl3127);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1513:1: rule__WMLMacroCall__Group__2 : rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 ;
    public final void rule__WMLMacroCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1517:1: ( rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1518:2: rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__23158);
            rule__WMLMacroCall__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__23161);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1525:1: rule__WMLMacroCall__Group__2__Impl : ( ( rule__WMLMacroCall__RelativeAssignment_2 )? ) ;
    public final void rule__WMLMacroCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1529:1: ( ( ( rule__WMLMacroCall__RelativeAssignment_2 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1530:1: ( ( rule__WMLMacroCall__RelativeAssignment_2 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1530:1: ( ( rule__WMLMacroCall__RelativeAssignment_2 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1531:1: ( rule__WMLMacroCall__RelativeAssignment_2 )?
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1532:1: ( rule__WMLMacroCall__RelativeAssignment_2 )?
            int alt20=2;
            int LA20_0 = input.LA(1);

            if ( (LA20_0==21) ) {
                alt20=1;
            }
            switch (alt20) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1532:2: rule__WMLMacroCall__RelativeAssignment_2
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__RelativeAssignment_2_in_rule__WMLMacroCall__Group__2__Impl3188);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1542:1: rule__WMLMacroCall__Group__3 : rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 ;
    public final void rule__WMLMacroCall__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1546:1: ( rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1547:2: rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__33219);
            rule__WMLMacroCall__Group__3__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__33222);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1554:1: rule__WMLMacroCall__Group__3__Impl : ( ( rule__WMLMacroCall__NameAssignment_3 ) ) ;
    public final void rule__WMLMacroCall__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1558:1: ( ( ( rule__WMLMacroCall__NameAssignment_3 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1559:1: ( ( rule__WMLMacroCall__NameAssignment_3 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1559:1: ( ( rule__WMLMacroCall__NameAssignment_3 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1560:1: ( rule__WMLMacroCall__NameAssignment_3 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameAssignment_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1561:1: ( rule__WMLMacroCall__NameAssignment_3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1561:2: rule__WMLMacroCall__NameAssignment_3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__NameAssignment_3_in_rule__WMLMacroCall__Group__3__Impl3249);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1571:1: rule__WMLMacroCall__Group__4 : rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5 ;
    public final void rule__WMLMacroCall__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1575:1: ( rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1576:2: rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__43279);
            rule__WMLMacroCall__Group__4__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__5_in_rule__WMLMacroCall__Group__43282);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1583:1: rule__WMLMacroCall__Group__4__Impl : ( ( rule__WMLMacroCall__ParametersAssignment_4 )* ) ;
    public final void rule__WMLMacroCall__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1587:1: ( ( ( rule__WMLMacroCall__ParametersAssignment_4 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1588:1: ( ( rule__WMLMacroCall__ParametersAssignment_4 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1588:1: ( ( rule__WMLMacroCall__ParametersAssignment_4 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1589:1: ( rule__WMLMacroCall__ParametersAssignment_4 )*
            {
             before(grammarAccess.getWMLMacroCallAccess().getParametersAssignment_4()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1590:1: ( rule__WMLMacroCall__ParametersAssignment_4 )*
            loop21:
            do {
                int alt21=2;
                int LA21_0 = input.LA(1);

                if ( ((LA21_0>=RULE_ID && LA21_0<=RULE_ANY_OTHER)||(LA21_0>=20 && LA21_0<=33)) ) {
                    alt21=1;
                }


                switch (alt21) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1590:2: rule__WMLMacroCall__ParametersAssignment_4
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroCall__ParametersAssignment_4_in_rule__WMLMacroCall__Group__4__Impl3309);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1600:1: rule__WMLMacroCall__Group__5 : rule__WMLMacroCall__Group__5__Impl ;
    public final void rule__WMLMacroCall__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1604:1: ( rule__WMLMacroCall__Group__5__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1605:2: rule__WMLMacroCall__Group__5__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__5__Impl_in_rule__WMLMacroCall__Group__53340);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1611:1: rule__WMLMacroCall__Group__5__Impl : ( '}' ) ;
    public final void rule__WMLMacroCall__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1615:1: ( ( '}' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1616:1: ( '}' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1616:1: ( '}' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1617:1: '}'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_5()); 
            match(input,34,FOLLOW_34_in_rule__WMLMacroCall__Group__5__Impl3368); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1642:1: rule__WMLArrayCall__Group__0 : rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 ;
    public final void rule__WMLArrayCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1646:1: ( rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1647:2: rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__03411);
            rule__WMLArrayCall__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__03414);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1654:1: rule__WMLArrayCall__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLArrayCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1658:1: ( ( '[' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1659:1: ( '[' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1659:1: ( '[' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1660:1: '['
            {
             before(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0()); 
            match(input,29,FOLLOW_29_in_rule__WMLArrayCall__Group__0__Impl3442); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1673:1: rule__WMLArrayCall__Group__1 : rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 ;
    public final void rule__WMLArrayCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1677:1: ( rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1678:2: rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__13473);
            rule__WMLArrayCall__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__13476);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1685:1: rule__WMLArrayCall__Group__1__Impl : ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) ;
    public final void rule__WMLArrayCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1689:1: ( ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1690:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1690:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1691:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1691:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1692:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1693:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1693:2: rule__WMLArrayCall__ValueAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3505);
            rule__WMLArrayCall__ValueAssignment_1();

            state._fsp--;


            }

             after(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 

            }

            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1696:1: ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1697:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1698:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            loop22:
            do {
                int alt22=2;
                int LA22_0 = input.LA(1);

                if ( ((LA22_0>=RULE_ID && LA22_0<=RULE_ANY_OTHER)||(LA22_0>=20 && LA22_0<=27)) ) {
                    alt22=1;
                }


                switch (alt22) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1698:2: rule__WMLArrayCall__ValueAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3517);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1709:1: rule__WMLArrayCall__Group__2 : rule__WMLArrayCall__Group__2__Impl ;
    public final void rule__WMLArrayCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1713:1: ( rule__WMLArrayCall__Group__2__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1714:2: rule__WMLArrayCall__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__23550);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1720:1: rule__WMLArrayCall__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLArrayCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1724:1: ( ( ']' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1725:1: ( ']' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1725:1: ( ']' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1726:1: ']'
            {
             before(grammarAccess.getWMLArrayCallAccess().getRightSquareBracketKeyword_2()); 
            match(input,30,FOLLOW_30_in_rule__WMLArrayCall__Group__2__Impl3578); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1745:1: rule__WMLMacroDefine__Group__0 : rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1 ;
    public final void rule__WMLMacroDefine__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1749:1: ( rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1750:2: rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__0__Impl_in_rule__WMLMacroDefine__Group__03615);
            rule__WMLMacroDefine__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__1_in_rule__WMLMacroDefine__Group__03618);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1757:1: rule__WMLMacroDefine__Group__0__Impl : ( ( rule__WMLMacroDefine__NameAssignment_0 ) ) ;
    public final void rule__WMLMacroDefine__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1761:1: ( ( ( rule__WMLMacroDefine__NameAssignment_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1762:1: ( ( rule__WMLMacroDefine__NameAssignment_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1762:1: ( ( rule__WMLMacroDefine__NameAssignment_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1763:1: ( rule__WMLMacroDefine__NameAssignment_0 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameAssignment_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1764:1: ( rule__WMLMacroDefine__NameAssignment_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1764:2: rule__WMLMacroDefine__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__NameAssignment_0_in_rule__WMLMacroDefine__Group__0__Impl3645);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1774:1: rule__WMLMacroDefine__Group__1 : rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2 ;
    public final void rule__WMLMacroDefine__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1778:1: ( rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1779:2: rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__1__Impl_in_rule__WMLMacroDefine__Group__13675);
            rule__WMLMacroDefine__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__2_in_rule__WMLMacroDefine__Group__13678);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1786:1: rule__WMLMacroDefine__Group__1__Impl : ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* ) ;
    public final void rule__WMLMacroDefine__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1790:1: ( ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1791:1: ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1791:1: ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1792:1: ( rule__WMLMacroDefine__ExpressionsAssignment_1 )*
            {
             before(grammarAccess.getWMLMacroDefineAccess().getExpressionsAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1793:1: ( rule__WMLMacroDefine__ExpressionsAssignment_1 )*
            loop23:
            do {
                int alt23=2;
                int LA23_0 = input.LA(1);

                if ( ((LA23_0>=RULE_IFDEF && LA23_0<=RULE_DEFINE)||LA23_0==RULE_TEXTDOMAIN||(LA23_0>=20 && LA23_0<=27)||LA23_0==29||LA23_0==33) ) {
                    alt23=1;
                }


                switch (alt23) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1793:2: rule__WMLMacroDefine__ExpressionsAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroDefine__ExpressionsAssignment_1_in_rule__WMLMacroDefine__Group__1__Impl3705);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1803:1: rule__WMLMacroDefine__Group__2 : rule__WMLMacroDefine__Group__2__Impl ;
    public final void rule__WMLMacroDefine__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1807:1: ( rule__WMLMacroDefine__Group__2__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1808:2: rule__WMLMacroDefine__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__2__Impl_in_rule__WMLMacroDefine__Group__23736);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1814:1: rule__WMLMacroDefine__Group__2__Impl : ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) ) ;
    public final void rule__WMLMacroDefine__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1818:1: ( ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1819:1: ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1819:1: ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1820:1: ( rule__WMLMacroDefine__EndNameAssignment_2 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getEndNameAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1821:1: ( rule__WMLMacroDefine__EndNameAssignment_2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1821:2: rule__WMLMacroDefine__EndNameAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__EndNameAssignment_2_in_rule__WMLMacroDefine__Group__2__Impl3763);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1837:1: rule__WMLPreprocIF__Group__0 : rule__WMLPreprocIF__Group__0__Impl rule__WMLPreprocIF__Group__1 ;
    public final void rule__WMLPreprocIF__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1841:1: ( rule__WMLPreprocIF__Group__0__Impl rule__WMLPreprocIF__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1842:2: rule__WMLPreprocIF__Group__0__Impl rule__WMLPreprocIF__Group__1
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__0__Impl_in_rule__WMLPreprocIF__Group__03799);
            rule__WMLPreprocIF__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__1_in_rule__WMLPreprocIF__Group__03802);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1849:1: rule__WMLPreprocIF__Group__0__Impl : ( ( rule__WMLPreprocIF__NameAssignment_0 ) ) ;
    public final void rule__WMLPreprocIF__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1853:1: ( ( ( rule__WMLPreprocIF__NameAssignment_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1854:1: ( ( rule__WMLPreprocIF__NameAssignment_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1854:1: ( ( rule__WMLPreprocIF__NameAssignment_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1855:1: ( rule__WMLPreprocIF__NameAssignment_0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getNameAssignment_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1856:1: ( rule__WMLPreprocIF__NameAssignment_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1856:2: rule__WMLPreprocIF__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__NameAssignment_0_in_rule__WMLPreprocIF__Group__0__Impl3829);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1866:1: rule__WMLPreprocIF__Group__1 : rule__WMLPreprocIF__Group__1__Impl rule__WMLPreprocIF__Group__2 ;
    public final void rule__WMLPreprocIF__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1870:1: ( rule__WMLPreprocIF__Group__1__Impl rule__WMLPreprocIF__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1871:2: rule__WMLPreprocIF__Group__1__Impl rule__WMLPreprocIF__Group__2
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__1__Impl_in_rule__WMLPreprocIF__Group__13859);
            rule__WMLPreprocIF__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__2_in_rule__WMLPreprocIF__Group__13862);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1878:1: rule__WMLPreprocIF__Group__1__Impl : ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* ) ;
    public final void rule__WMLPreprocIF__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1882:1: ( ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1883:1: ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1883:1: ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1884:1: ( rule__WMLPreprocIF__ExpressionsAssignment_1 )*
            {
             before(grammarAccess.getWMLPreprocIFAccess().getExpressionsAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1885:1: ( rule__WMLPreprocIF__ExpressionsAssignment_1 )*
            loop24:
            do {
                int alt24=2;
                int LA24_0 = input.LA(1);

                if ( ((LA24_0>=RULE_IFDEF && LA24_0<=RULE_DEFINE)||LA24_0==RULE_TEXTDOMAIN||(LA24_0>=20 && LA24_0<=27)||LA24_0==29||LA24_0==33) ) {
                    alt24=1;
                }


                switch (alt24) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1885:2: rule__WMLPreprocIF__ExpressionsAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLPreprocIF__ExpressionsAssignment_1_in_rule__WMLPreprocIF__Group__1__Impl3889);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1895:1: rule__WMLPreprocIF__Group__2 : rule__WMLPreprocIF__Group__2__Impl rule__WMLPreprocIF__Group__3 ;
    public final void rule__WMLPreprocIF__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1899:1: ( rule__WMLPreprocIF__Group__2__Impl rule__WMLPreprocIF__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1900:2: rule__WMLPreprocIF__Group__2__Impl rule__WMLPreprocIF__Group__3
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__2__Impl_in_rule__WMLPreprocIF__Group__23920);
            rule__WMLPreprocIF__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__3_in_rule__WMLPreprocIF__Group__23923);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1907:1: rule__WMLPreprocIF__Group__2__Impl : ( ( rule__WMLPreprocIF__Group_2__0 )? ) ;
    public final void rule__WMLPreprocIF__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1911:1: ( ( ( rule__WMLPreprocIF__Group_2__0 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1912:1: ( ( rule__WMLPreprocIF__Group_2__0 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1912:1: ( ( rule__WMLPreprocIF__Group_2__0 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1913:1: ( rule__WMLPreprocIF__Group_2__0 )?
            {
             before(grammarAccess.getWMLPreprocIFAccess().getGroup_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1914:1: ( rule__WMLPreprocIF__Group_2__0 )?
            int alt25=2;
            int LA25_0 = input.LA(1);

            if ( (LA25_0==RULE_ELSE) ) {
                alt25=1;
            }
            switch (alt25) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1914:2: rule__WMLPreprocIF__Group_2__0
                    {
                    pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__0_in_rule__WMLPreprocIF__Group__2__Impl3950);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1924:1: rule__WMLPreprocIF__Group__3 : rule__WMLPreprocIF__Group__3__Impl ;
    public final void rule__WMLPreprocIF__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1928:1: ( rule__WMLPreprocIF__Group__3__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1929:2: rule__WMLPreprocIF__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__3__Impl_in_rule__WMLPreprocIF__Group__33981);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1935:1: rule__WMLPreprocIF__Group__3__Impl : ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) ) ;
    public final void rule__WMLPreprocIF__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1939:1: ( ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1940:1: ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1940:1: ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1941:1: ( rule__WMLPreprocIF__EndNameAssignment_3 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getEndNameAssignment_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1942:1: ( rule__WMLPreprocIF__EndNameAssignment_3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1942:2: rule__WMLPreprocIF__EndNameAssignment_3
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__EndNameAssignment_3_in_rule__WMLPreprocIF__Group__3__Impl4008);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1960:1: rule__WMLPreprocIF__Group_2__0 : rule__WMLPreprocIF__Group_2__0__Impl rule__WMLPreprocIF__Group_2__1 ;
    public final void rule__WMLPreprocIF__Group_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1964:1: ( rule__WMLPreprocIF__Group_2__0__Impl rule__WMLPreprocIF__Group_2__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1965:2: rule__WMLPreprocIF__Group_2__0__Impl rule__WMLPreprocIF__Group_2__1
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__0__Impl_in_rule__WMLPreprocIF__Group_2__04046);
            rule__WMLPreprocIF__Group_2__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__1_in_rule__WMLPreprocIF__Group_2__04049);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1972:1: rule__WMLPreprocIF__Group_2__0__Impl : ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) ) ;
    public final void rule__WMLPreprocIF__Group_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1976:1: ( ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1977:1: ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1977:1: ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1978:1: ( rule__WMLPreprocIF__ElsesAssignment_2_0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElsesAssignment_2_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1979:1: ( rule__WMLPreprocIF__ElsesAssignment_2_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1979:2: rule__WMLPreprocIF__ElsesAssignment_2_0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__ElsesAssignment_2_0_in_rule__WMLPreprocIF__Group_2__0__Impl4076);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1989:1: rule__WMLPreprocIF__Group_2__1 : rule__WMLPreprocIF__Group_2__1__Impl ;
    public final void rule__WMLPreprocIF__Group_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1993:1: ( rule__WMLPreprocIF__Group_2__1__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1994:2: rule__WMLPreprocIF__Group_2__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__1__Impl_in_rule__WMLPreprocIF__Group_2__14106);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2000:1: rule__WMLPreprocIF__Group_2__1__Impl : ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) ) ;
    public final void rule__WMLPreprocIF__Group_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2004:1: ( ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2005:1: ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2005:1: ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2006:1: ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2006:1: ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2007:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2008:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2008:2: rule__WMLPreprocIF__ElseExpressionsAssignment_2_1
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4135);
            rule__WMLPreprocIF__ElseExpressionsAssignment_2_1();

            state._fsp--;


            }

             after(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 

            }

            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2011:1: ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2012:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )*
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2013:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )*
            loop26:
            do {
                int alt26=2;
                int LA26_0 = input.LA(1);

                if ( ((LA26_0>=RULE_IFDEF && LA26_0<=RULE_DEFINE)||LA26_0==RULE_TEXTDOMAIN||(LA26_0>=20 && LA26_0<=27)||LA26_0==29||LA26_0==33) ) {
                    alt26=1;
                }


                switch (alt26) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2013:2: rule__WMLPreprocIF__ElseExpressionsAssignment_2_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4147);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2029:1: rule__WMLRoot__ExpressionsAssignment : ( ruleWMLRootExpression ) ;
    public final void rule__WMLRoot__ExpressionsAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2033:1: ( ( ruleWMLRootExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2034:1: ( ruleWMLRootExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2034:1: ( ruleWMLRootExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2035:1: ruleWMLRootExpression
            {
             before(grammarAccess.getWMLRootAccess().getExpressionsWMLRootExpressionParserRuleCall_0()); 
            pushFollow(FOLLOW_ruleWMLRootExpression_in_rule__WMLRoot__ExpressionsAssignment4189);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2044:1: rule__WMLTag__PlusAssignment_1 : ( ( '+' ) ) ;
    public final void rule__WMLTag__PlusAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2048:1: ( ( ( '+' ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2049:1: ( ( '+' ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2049:1: ( ( '+' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2050:1: ( '+' )
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2051:1: ( '+' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2052:1: '+'
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            match(input,31,FOLLOW_31_in_rule__WMLTag__PlusAssignment_14225); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2067:1: rule__WMLTag__NameAssignment_2 : ( RULE_ID ) ;
    public final void rule__WMLTag__NameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2071:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2072:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2072:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2073:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_2_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_24264); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2082:1: rule__WMLTag__ExpressionsAssignment_4 : ( ruleWMLExpression ) ;
    public final void rule__WMLTag__ExpressionsAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2086:1: ( ( ruleWMLExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2087:1: ( ruleWMLExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2087:1: ( ruleWMLExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2088:1: ruleWMLExpression
            {
             before(grammarAccess.getWMLTagAccess().getExpressionsWMLExpressionParserRuleCall_4_0()); 
            pushFollow(FOLLOW_ruleWMLExpression_in_rule__WMLTag__ExpressionsAssignment_44295);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2097:1: rule__WMLTag__EndNameAssignment_6 : ( RULE_ID ) ;
    public final void rule__WMLTag__EndNameAssignment_6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2101:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2102:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2102:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2103:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_64326); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2112:1: rule__WMLKey__NameAssignment_0 : ( RULE_ID ) ;
    public final void rule__WMLKey__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2116:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2117:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2117:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2118:1: RULE_ID
            {
             before(grammarAccess.getWMLKeyAccess().getNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_04357); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2127:1: rule__WMLKey__ValuesAssignment_2 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValuesAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2131:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2132:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2132:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2133:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValuesWMLKeyValueParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValuesAssignment_24388);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2142:1: rule__WMLKey__ValuesAssignment_3_3 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValuesAssignment_3_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2146:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2147:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2147:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2148:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValuesWMLKeyValueParserRuleCall_3_3_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValuesAssignment_3_34419);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2157:1: rule__WMLKey__EolAssignment_4 : ( ( rule__WMLKey__EolAlternatives_4_0 ) ) ;
    public final void rule__WMLKey__EolAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2161:1: ( ( ( rule__WMLKey__EolAlternatives_4_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2162:1: ( ( rule__WMLKey__EolAlternatives_4_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2162:1: ( ( rule__WMLKey__EolAlternatives_4_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2163:1: ( rule__WMLKey__EolAlternatives_4_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getEolAlternatives_4_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2164:1: ( rule__WMLKey__EolAlternatives_4_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2164:2: rule__WMLKey__EolAlternatives_4_0
            {
            pushFollow(FOLLOW_rule__WMLKey__EolAlternatives_4_0_in_rule__WMLKey__EolAssignment_44450);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2173:1: rule__WMLMacroCall__PointAssignment_1 : ( ( './' ) ) ;
    public final void rule__WMLMacroCall__PointAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2177:1: ( ( ( './' ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2178:1: ( ( './' ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2178:1: ( ( './' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2179:1: ( './' )
            {
             before(grammarAccess.getWMLMacroCallAccess().getPointFullStopSolidusKeyword_1_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2180:1: ( './' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2181:1: './'
            {
             before(grammarAccess.getWMLMacroCallAccess().getPointFullStopSolidusKeyword_1_0()); 
            match(input,23,FOLLOW_23_in_rule__WMLMacroCall__PointAssignment_14488); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2196:1: rule__WMLMacroCall__RelativeAssignment_2 : ( ( '~' ) ) ;
    public final void rule__WMLMacroCall__RelativeAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2200:1: ( ( ( '~' ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2201:1: ( ( '~' ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2201:1: ( ( '~' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2202:1: ( '~' )
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2203:1: ( '~' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2204:1: '~'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0()); 
            match(input,21,FOLLOW_21_in_rule__WMLMacroCall__RelativeAssignment_24532); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2219:1: rule__WMLMacroCall__NameAssignment_3 : ( RULE_ID ) ;
    public final void rule__WMLMacroCall__NameAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2223:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2224:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2224:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2225:1: RULE_ID
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_3_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_34571); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2234:1: rule__WMLMacroCall__ParametersAssignment_4 : ( ruleWMLMacroCallParameter ) ;
    public final void rule__WMLMacroCall__ParametersAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2238:1: ( ( ruleWMLMacroCallParameter ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2239:1: ( ruleWMLMacroCallParameter )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2239:1: ( ruleWMLMacroCallParameter )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2240:1: ruleWMLMacroCallParameter
            {
             before(grammarAccess.getWMLMacroCallAccess().getParametersWMLMacroCallParameterParserRuleCall_4_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_rule__WMLMacroCall__ParametersAssignment_44602);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2249:1: rule__WMLArrayCall__ValueAssignment_1 : ( ruleWMLValue ) ;
    public final void rule__WMLArrayCall__ValueAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2253:1: ( ( ruleWMLValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2254:1: ( ruleWMLValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2254:1: ( ruleWMLValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2255:1: ruleWMLValue
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_14633);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2264:1: rule__WMLMacroDefine__NameAssignment_0 : ( RULE_DEFINE ) ;
    public final void rule__WMLMacroDefine__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2268:1: ( ( RULE_DEFINE ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2269:1: ( RULE_DEFINE )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2269:1: ( RULE_DEFINE )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2270:1: RULE_DEFINE
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0_0()); 
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment_04664); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2279:1: rule__WMLMacroDefine__ExpressionsAssignment_1 : ( ruleWMLValuedExpression ) ;
    public final void rule__WMLMacroDefine__ExpressionsAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2283:1: ( ( ruleWMLValuedExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2284:1: ( ruleWMLValuedExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2284:1: ( ruleWMLValuedExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2285:1: ruleWMLValuedExpression
            {
             before(grammarAccess.getWMLMacroDefineAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_rule__WMLMacroDefine__ExpressionsAssignment_14695);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2294:1: rule__WMLMacroDefine__EndNameAssignment_2 : ( RULE_ENDDEF ) ;
    public final void rule__WMLMacroDefine__EndNameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2298:1: ( ( RULE_ENDDEF ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2299:1: ( RULE_ENDDEF )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2299:1: ( RULE_ENDDEF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2300:1: RULE_ENDDEF
            {
             before(grammarAccess.getWMLMacroDefineAccess().getEndNameENDDEFTerminalRuleCall_2_0()); 
            match(input,RULE_ENDDEF,FOLLOW_RULE_ENDDEF_in_rule__WMLMacroDefine__EndNameAssignment_24726); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2309:1: rule__WMLPreprocIF__NameAssignment_0 : ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) ) ;
    public final void rule__WMLPreprocIF__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2313:1: ( ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2314:1: ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2314:1: ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2315:1: ( rule__WMLPreprocIF__NameAlternatives_0_0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getNameAlternatives_0_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2316:1: ( rule__WMLPreprocIF__NameAlternatives_0_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2316:2: rule__WMLPreprocIF__NameAlternatives_0_0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__NameAlternatives_0_0_in_rule__WMLPreprocIF__NameAssignment_04757);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2325:1: rule__WMLPreprocIF__ExpressionsAssignment_1 : ( ruleWMLValuedExpression ) ;
    public final void rule__WMLPreprocIF__ExpressionsAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2329:1: ( ( ruleWMLValuedExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2330:1: ( ruleWMLValuedExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2330:1: ( ruleWMLValuedExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2331:1: ruleWMLValuedExpression
            {
             before(grammarAccess.getWMLPreprocIFAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ExpressionsAssignment_14790);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2340:1: rule__WMLPreprocIF__ElsesAssignment_2_0 : ( RULE_ELSE ) ;
    public final void rule__WMLPreprocIF__ElsesAssignment_2_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2344:1: ( ( RULE_ELSE ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2345:1: ( RULE_ELSE )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2345:1: ( RULE_ELSE )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2346:1: RULE_ELSE
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElsesELSETerminalRuleCall_2_0_0()); 
            match(input,RULE_ELSE,FOLLOW_RULE_ELSE_in_rule__WMLPreprocIF__ElsesAssignment_2_04821); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2355:1: rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 : ( ruleWMLValuedExpression ) ;
    public final void rule__WMLPreprocIF__ElseExpressionsAssignment_2_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2359:1: ( ( ruleWMLValuedExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2360:1: ( ruleWMLValuedExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2360:1: ( ruleWMLValuedExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2361:1: ruleWMLValuedExpression
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsWMLValuedExpressionParserRuleCall_2_1_0()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ElseExpressionsAssignment_2_14852);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2370:1: rule__WMLPreprocIF__EndNameAssignment_3 : ( RULE_ENDIF ) ;
    public final void rule__WMLPreprocIF__EndNameAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2374:1: ( ( RULE_ENDIF ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2375:1: ( RULE_ENDIF )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2375:1: ( RULE_ENDIF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2376:1: RULE_ENDIF
            {
             before(grammarAccess.getWMLPreprocIFAccess().getEndNameENDIFTerminalRuleCall_3_0()); 
            match(input,RULE_ENDIF,FOLLOW_RULE_ENDIF_in_rule__WMLPreprocIF__EndNameAssignment_34883); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2385:1: rule__WMLTextdomain__NameAssignment : ( RULE_TEXTDOMAIN ) ;
    public final void rule__WMLTextdomain__NameAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2389:1: ( ( RULE_TEXTDOMAIN ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2390:1: ( RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2390:1: ( RULE_TEXTDOMAIN )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2391:1: RULE_TEXTDOMAIN
            {
             before(grammarAccess.getWMLTextdomainAccess().getNameTEXTDOMAINTerminalRuleCall_0()); 
            match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment4914); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2400:1: rule__WMLLuaCode__ValueAssignment : ( RULE_LUA_CODE ) ;
    public final void rule__WMLLuaCode__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2404:1: ( ( RULE_LUA_CODE ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2405:1: ( RULE_LUA_CODE )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2405:1: ( RULE_LUA_CODE )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2406:1: RULE_LUA_CODE
            {
             before(grammarAccess.getWMLLuaCodeAccess().getValueLUA_CODETerminalRuleCall_0()); 
            match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment4945); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2415:1: rule__WMLValue__ValueAssignment : ( ( rule__WMLValue__ValueAlternatives_0 ) ) ;
    public final void rule__WMLValue__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2419:1: ( ( ( rule__WMLValue__ValueAlternatives_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2420:1: ( ( rule__WMLValue__ValueAlternatives_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2420:1: ( ( rule__WMLValue__ValueAlternatives_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2421:1: ( rule__WMLValue__ValueAlternatives_0 )
            {
             before(grammarAccess.getWMLValueAccess().getValueAlternatives_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2422:1: ( rule__WMLValue__ValueAlternatives_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2422:2: rule__WMLValue__ValueAlternatives_0
            {
            pushFollow(FOLLOW_rule__WMLValue__ValueAlternatives_0_in_rule__WMLValue__ValueAssignment4976);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2431:1: rule__MacroTokens__ValueAssignment : ( ( rule__MacroTokens__ValueAlternatives_0 ) ) ;
    public final void rule__MacroTokens__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2435:1: ( ( ( rule__MacroTokens__ValueAlternatives_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2436:1: ( ( rule__MacroTokens__ValueAlternatives_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2436:1: ( ( rule__MacroTokens__ValueAlternatives_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2437:1: ( rule__MacroTokens__ValueAlternatives_0 )
            {
             before(grammarAccess.getMacroTokensAccess().getValueAlternatives_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2438:1: ( rule__MacroTokens__ValueAlternatives_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2438:2: rule__MacroTokens__ValueAlternatives_0
            {
            pushFollow(FOLLOW_rule__MacroTokens__ValueAlternatives_0_in_rule__MacroTokens__ValueAssignment5009);
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
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag124 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag131 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0_in_ruleWMLTag157 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey189 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey196 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey226 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue253 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue260 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue286 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall313 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCall320 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__0_in_ruleWMLMacroCall346 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_entryRuleWMLMacroCallParameter373 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCallParameter380 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Alternatives_in_ruleWMLMacroCallParameter406 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall433 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLArrayCall440 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__0_in_ruleWMLArrayCall466 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine493 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine500 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__0_in_ruleWMLMacroDefine526 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPreprocIF_in_entryRuleWMLPreprocIF553 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLPreprocIF560 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__0_in_ruleWMLPreprocIF586 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_entryRuleWMLRootExpression613 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRootExpression620 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRootExpression__Alternatives_in_ruleWMLRootExpression646 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_entryRuleWMLExpression673 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLExpression680 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLExpression__Alternatives_in_ruleWMLExpression706 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_entryRuleWMLValuedExpression733 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValuedExpression740 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValuedExpression__Alternatives_in_ruleWMLValuedExpression766 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain793 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTextdomain800 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTextdomain__NameAssignment_in_ruleWMLTextdomain826 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode853 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLLuaCode860 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLLuaCode__ValueAssignment_in_ruleWMLLuaCode886 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter913 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroParameter920 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroParameter__Alternatives_in_ruleWMLMacroParameter946 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_entryRuleWMLValue973 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValue980 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValue__ValueAssignment_in_ruleWMLValue1006 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens1033 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleMacroTokens1040 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__MacroTokens__ValueAssignment_in_ruleMacroTokens1066 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__EolAlternatives_4_01103 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_SL_COMMENT_in_rule__WMLKey__EolAlternatives_4_01120 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLKeyValue__Alternatives1152 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives1169 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_rule__WMLKeyValue__Alternatives1186 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_rule__WMLKeyValue__Alternatives1203 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_rule__WMLMacroCallParameter__Alternatives1235 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCallParameter__Alternatives1252 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFDEF_in_rule__WMLPreprocIF__NameAlternatives_0_01284 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFNDEF_in_rule__WMLPreprocIF__NameAlternatives_0_01301 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFHAVE_in_rule__WMLPreprocIF__NameAlternatives_0_01318 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFNHAVE_in_rule__WMLPreprocIF__NameAlternatives_0_01335 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRootExpression__Alternatives1367 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLRootExpression__Alternatives1384 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLRootExpression__Alternatives1401 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_rule__WMLRootExpression__Alternatives1418 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPreprocIF_in_rule__WMLRootExpression__Alternatives1435 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_rule__WMLExpression__Alternatives1467 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLExpression__Alternatives1484 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_rule__WMLValuedExpression__Alternatives1516 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLValuedExpression__Alternatives1533 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLMacroParameter__Alternatives1565 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_rule__WMLMacroParameter__Alternatives1582 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLValue__ValueAlternatives_01614 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLValue__ValueAlternatives_01631 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__WMLValue__ValueAlternatives_01649 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLValue__ValueAlternatives_01669 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__WMLValue__ValueAlternatives_01689 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__WMLValue__ValueAlternatives_01709 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_rule__WMLValue__ValueAlternatives_01729 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__WMLValue__ValueAlternatives_01749 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_rule__WMLValue__ValueAlternatives_01769 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rule__WMLValue__ValueAlternatives_01789 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__ValueAlternatives_01808 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__MacroTokens__ValueAlternatives_01841 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__MacroTokens__ValueAlternatives_01861 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__MacroTokens__ValueAlternatives_01881 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__MacroTokens__ValueAlternatives_01901 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__MacroTokens__ValueAlternatives_01921 = new BitSet(new long[]{0x0000000000000002L});
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
    public static final BitSet FOLLOW_rule__WMLKey__ValuesAssignment_2_in_rule__WMLKey__Group__2__Impl2608 = new BitSet(new long[]{0x000000022FF41C02L});
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
    public static final BitSet FOLLOW_rule__WMLKey__ValuesAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2982 = new BitSet(new long[]{0x000000022FF41C12L});
    public static final BitSet FOLLOW_rule__WMLKey__ValuesAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl2994 = new BitSet(new long[]{0x000000022FF41C12L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__03035 = new BitSet(new long[]{0x0000000000A00400L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__03038 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_33_in_rule__WMLMacroCall__Group__0__Impl3066 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__13097 = new BitSet(new long[]{0x0000000000A00400L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__13100 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__PointAssignment_1_in_rule__WMLMacroCall__Group__1__Impl3127 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__23158 = new BitSet(new long[]{0x0000000000A00400L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__23161 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__RelativeAssignment_2_in_rule__WMLMacroCall__Group__2__Impl3188 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__33219 = new BitSet(new long[]{0x00000007FFF01C00L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__33222 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__NameAssignment_3_in_rule__WMLMacroCall__Group__3__Impl3249 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__43279 = new BitSet(new long[]{0x00000007FFF01C00L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__5_in_rule__WMLMacroCall__Group__43282 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ParametersAssignment_4_in_rule__WMLMacroCall__Group__4__Impl3309 = new BitSet(new long[]{0x00000003FFF01C02L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__5__Impl_in_rule__WMLMacroCall__Group__53340 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_34_in_rule__WMLMacroCall__Group__5__Impl3368 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__03411 = new BitSet(new long[]{0x000000000FF01C00L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__03414 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__WMLArrayCall__Group__0__Impl3442 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__13473 = new BitSet(new long[]{0x0000000040000000L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__13476 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3505 = new BitSet(new long[]{0x000000000FF01C02L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3517 = new BitSet(new long[]{0x000000000FF01C02L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__23550 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__WMLArrayCall__Group__2__Impl3578 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__0__Impl_in_rule__WMLMacroDefine__Group__03615 = new BitSet(new long[]{0x000000022FF27FC0L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__1_in_rule__WMLMacroDefine__Group__03618 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__NameAssignment_0_in_rule__WMLMacroDefine__Group__0__Impl3645 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__1__Impl_in_rule__WMLMacroDefine__Group__13675 = new BitSet(new long[]{0x000000022FF27FC0L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__2_in_rule__WMLMacroDefine__Group__13678 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__ExpressionsAssignment_1_in_rule__WMLMacroDefine__Group__1__Impl3705 = new BitSet(new long[]{0x000000022FF23FC2L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__2__Impl_in_rule__WMLMacroDefine__Group__23736 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__EndNameAssignment_2_in_rule__WMLMacroDefine__Group__2__Impl3763 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__0__Impl_in_rule__WMLPreprocIF__Group__03799 = new BitSet(new long[]{0x000000022FF3BFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__1_in_rule__WMLPreprocIF__Group__03802 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__NameAssignment_0_in_rule__WMLPreprocIF__Group__0__Impl3829 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__1__Impl_in_rule__WMLPreprocIF__Group__13859 = new BitSet(new long[]{0x000000022FF3BFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__2_in_rule__WMLPreprocIF__Group__13862 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ExpressionsAssignment_1_in_rule__WMLPreprocIF__Group__1__Impl3889 = new BitSet(new long[]{0x000000022FF23FC2L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__2__Impl_in_rule__WMLPreprocIF__Group__23920 = new BitSet(new long[]{0x000000022FF3BFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__3_in_rule__WMLPreprocIF__Group__23923 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__0_in_rule__WMLPreprocIF__Group__2__Impl3950 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__3__Impl_in_rule__WMLPreprocIF__Group__33981 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__EndNameAssignment_3_in_rule__WMLPreprocIF__Group__3__Impl4008 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__0__Impl_in_rule__WMLPreprocIF__Group_2__04046 = new BitSet(new long[]{0x000000022FF23FC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__1_in_rule__WMLPreprocIF__Group_2__04049 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ElsesAssignment_2_0_in_rule__WMLPreprocIF__Group_2__0__Impl4076 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__1__Impl_in_rule__WMLPreprocIF__Group_2__14106 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4135 = new BitSet(new long[]{0x000000022FF23FC2L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4147 = new BitSet(new long[]{0x000000022FF23FC2L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_rule__WMLRoot__ExpressionsAssignment4189 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__WMLTag__PlusAssignment_14225 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_24264 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_rule__WMLTag__ExpressionsAssignment_44295 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_64326 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_04357 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValuesAssignment_24388 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValuesAssignment_3_34419 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__EolAlternatives_4_0_in_rule__WMLKey__EolAssignment_44450 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__WMLMacroCall__PointAssignment_14488 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLMacroCall__RelativeAssignment_24532 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_34571 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_rule__WMLMacroCall__ParametersAssignment_44602 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_14633 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment_04664 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_rule__WMLMacroDefine__ExpressionsAssignment_14695 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ENDDEF_in_rule__WMLMacroDefine__EndNameAssignment_24726 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__NameAlternatives_0_0_in_rule__WMLPreprocIF__NameAssignment_04757 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ExpressionsAssignment_14790 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ELSE_in_rule__WMLPreprocIF__ElsesAssignment_2_04821 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ElseExpressionsAssignment_2_14852 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ENDIF_in_rule__WMLPreprocIF__EndNameAssignment_34883 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment4914 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment4945 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValue__ValueAlternatives_0_in_rule__WMLValue__ValueAssignment4976 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__MacroTokens__ValueAlternatives_0_in_rule__MacroTokens__ValueAssignment5009 = new BitSet(new long[]{0x0000000000000002L});

}