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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_EOL", "RULE_SL_COMMENT", "RULE_IFDEF", "RULE_IFNDEF", "RULE_IFHAVE", "RULE_IFNHAVE", "RULE_IFVER", "RULE_IFNVER", "RULE_ID", "RULE_STRING", "RULE_ANY_OTHER", "RULE_DEFINE", "RULE_ENDDEF", "RULE_ELSE", "RULE_ENDIF", "RULE_TEXTDOMAIN", "RULE_LUA_CODE", "RULE_WS", "'_'", "'~'", "'.'", "'./'", "'$'", "'/'", "'('", "')'", "'='", "'['", "']'", "'+'", "'[/'", "'{'", "'}'"
    };
    public static final int RULE_LUA_CODE=20;
    public static final int RULE_ID=12;
    public static final int RULE_IFDEF=6;
    public static final int T__29=29;
    public static final int T__28=28;
    public static final int T__27=27;
    public static final int T__26=26;
    public static final int T__25=25;
    public static final int T__24=24;
    public static final int T__23=23;
    public static final int T__22=22;
    public static final int RULE_ANY_OTHER=14;
    public static final int RULE_IFNDEF=7;
    public static final int RULE_EOL=4;
    public static final int RULE_TEXTDOMAIN=19;
    public static final int RULE_IFNHAVE=9;
    public static final int RULE_SL_COMMENT=5;
    public static final int EOF=-1;
    public static final int RULE_IFNVER=11;
    public static final int T__30=30;
    public static final int T__31=31;
    public static final int RULE_STRING=13;
    public static final int T__32=32;
    public static final int T__33=33;
    public static final int RULE_ENDIF=18;
    public static final int T__34=34;
    public static final int T__35=35;
    public static final int T__36=36;
    public static final int RULE_DEFINE=15;
    public static final int RULE_IFVER=10;
    public static final int RULE_ENDDEF=16;
    public static final int RULE_IFHAVE=8;
    public static final int RULE_WS=21;
    public static final int RULE_ELSE=17;

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

                if ( ((LA1_0>=RULE_IFDEF && LA1_0<=RULE_IFNVER)||LA1_0==RULE_DEFINE||LA1_0==RULE_TEXTDOMAIN||LA1_0==31||LA1_0==35) ) {
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
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
                {
                alt3=1;
                }
                break;
            case 35:
                {
                alt3=2;
                }
                break;
            case RULE_LUA_CODE:
                {
                alt3=3;
                }
                break;
            case 31:
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

            if ( ((LA4_0>=RULE_ID && LA4_0<=RULE_ANY_OTHER)||(LA4_0>=22 && LA4_0<=34)) ) {
                alt4=1;
            }
            else if ( (LA4_0==35) ) {
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:626:1: rule__WMLPreprocIF__NameAlternatives_0_0 : ( ( RULE_IFDEF ) | ( RULE_IFNDEF ) | ( RULE_IFHAVE ) | ( RULE_IFNHAVE ) | ( RULE_IFVER ) | ( RULE_IFNVER ) );
    public final void rule__WMLPreprocIF__NameAlternatives_0_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:630:1: ( ( RULE_IFDEF ) | ( RULE_IFNDEF ) | ( RULE_IFHAVE ) | ( RULE_IFNHAVE ) | ( RULE_IFVER ) | ( RULE_IFNVER ) )
            int alt5=6;
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
            case RULE_IFVER:
                {
                alt5=5;
                }
                break;
            case RULE_IFNVER:
                {
                alt5=6;
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
                case 5 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:655:6: ( RULE_IFVER )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:655:6: ( RULE_IFVER )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:656:1: RULE_IFVER
                    {
                     before(grammarAccess.getWMLPreprocIFAccess().getNameIFVERTerminalRuleCall_0_0_4()); 
                    match(input,RULE_IFVER,FOLLOW_RULE_IFVER_in_rule__WMLPreprocIF__NameAlternatives_0_01352); 
                     after(grammarAccess.getWMLPreprocIFAccess().getNameIFVERTerminalRuleCall_0_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:661:6: ( RULE_IFNVER )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:661:6: ( RULE_IFNVER )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:662:1: RULE_IFNVER
                    {
                     before(grammarAccess.getWMLPreprocIFAccess().getNameIFNVERTerminalRuleCall_0_0_5()); 
                    match(input,RULE_IFNVER,FOLLOW_RULE_IFNVER_in_rule__WMLPreprocIF__NameAlternatives_0_01369); 
                     after(grammarAccess.getWMLPreprocIFAccess().getNameIFNVERTerminalRuleCall_0_0_5()); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:672:1: rule__WMLRootExpression__Alternatives : ( ( ruleWMLTag ) | ( ruleWMLMacroCall ) | ( ruleWMLMacroDefine ) | ( ruleWMLTextdomain ) | ( ruleWMLPreprocIF ) );
    public final void rule__WMLRootExpression__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:676:1: ( ( ruleWMLTag ) | ( ruleWMLMacroCall ) | ( ruleWMLMacroDefine ) | ( ruleWMLTextdomain ) | ( ruleWMLPreprocIF ) )
            int alt6=5;
            switch ( input.LA(1) ) {
            case 31:
                {
                alt6=1;
                }
                break;
            case 35:
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
            case RULE_IFVER:
            case RULE_IFNVER:
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:677:1: ( ruleWMLTag )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:677:1: ( ruleWMLTag )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:678:1: ruleWMLTag
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLTagParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRootExpression__Alternatives1401);
                    ruleWMLTag();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLTagParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:683:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:683:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:684:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLMacroCallParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLRootExpression__Alternatives1418);
                    ruleWMLMacroCall();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLMacroCallParserRuleCall_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:689:6: ( ruleWMLMacroDefine )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:689:6: ( ruleWMLMacroDefine )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:690:1: ruleWMLMacroDefine
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLMacroDefineParserRuleCall_2()); 
                    pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLRootExpression__Alternatives1435);
                    ruleWMLMacroDefine();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLMacroDefineParserRuleCall_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:695:6: ( ruleWMLTextdomain )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:695:6: ( ruleWMLTextdomain )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:696:1: ruleWMLTextdomain
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLTextdomainParserRuleCall_3()); 
                    pushFollow(FOLLOW_ruleWMLTextdomain_in_rule__WMLRootExpression__Alternatives1452);
                    ruleWMLTextdomain();

                    state._fsp--;

                     after(grammarAccess.getWMLRootExpressionAccess().getWMLTextdomainParserRuleCall_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:701:6: ( ruleWMLPreprocIF )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:701:6: ( ruleWMLPreprocIF )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:702:1: ruleWMLPreprocIF
                    {
                     before(grammarAccess.getWMLRootExpressionAccess().getWMLPreprocIFParserRuleCall_4()); 
                    pushFollow(FOLLOW_ruleWMLPreprocIF_in_rule__WMLRootExpression__Alternatives1469);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:712:1: rule__WMLExpression__Alternatives : ( ( ruleWMLRootExpression ) | ( ruleWMLKey ) );
    public final void rule__WMLExpression__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:716:1: ( ( ruleWMLRootExpression ) | ( ruleWMLKey ) )
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( ((LA7_0>=RULE_IFDEF && LA7_0<=RULE_IFNVER)||LA7_0==RULE_DEFINE||LA7_0==RULE_TEXTDOMAIN||LA7_0==31||LA7_0==35) ) {
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:717:1: ( ruleWMLRootExpression )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:717:1: ( ruleWMLRootExpression )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:718:1: ruleWMLRootExpression
                    {
                     before(grammarAccess.getWMLExpressionAccess().getWMLRootExpressionParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLRootExpression_in_rule__WMLExpression__Alternatives1501);
                    ruleWMLRootExpression();

                    state._fsp--;

                     after(grammarAccess.getWMLExpressionAccess().getWMLRootExpressionParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:723:6: ( ruleWMLKey )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:723:6: ( ruleWMLKey )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:724:1: ruleWMLKey
                    {
                     before(grammarAccess.getWMLExpressionAccess().getWMLKeyParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLExpression__Alternatives1518);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:734:1: rule__WMLValuedExpression__Alternatives : ( ( ruleWMLExpression ) | ( ruleWMLValue ) );
    public final void rule__WMLValuedExpression__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:738:1: ( ( ruleWMLExpression ) | ( ruleWMLValue ) )
            int alt8=2;
            switch ( input.LA(1) ) {
            case RULE_IFDEF:
            case RULE_IFNDEF:
            case RULE_IFHAVE:
            case RULE_IFNHAVE:
            case RULE_IFVER:
            case RULE_IFNVER:
            case RULE_DEFINE:
            case RULE_TEXTDOMAIN:
            case 31:
            case 35:
                {
                alt8=1;
                }
                break;
            case RULE_ID:
                {
                int LA8_2 = input.LA(2);

                if ( (LA8_2==EOF||(LA8_2>=RULE_IFDEF && LA8_2<=RULE_TEXTDOMAIN)||(LA8_2>=22 && LA8_2<=29)||LA8_2==31||LA8_2==35) ) {
                    alt8=2;
                }
                else if ( (LA8_2==30) ) {
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
            case 22:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
            case 28:
            case 29:
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:739:1: ( ruleWMLExpression )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:739:1: ( ruleWMLExpression )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:740:1: ruleWMLExpression
                    {
                     before(grammarAccess.getWMLValuedExpressionAccess().getWMLExpressionParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLExpression_in_rule__WMLValuedExpression__Alternatives1550);
                    ruleWMLExpression();

                    state._fsp--;

                     after(grammarAccess.getWMLValuedExpressionAccess().getWMLExpressionParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:745:6: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:745:6: ( ruleWMLValue )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:746:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLValuedExpressionAccess().getWMLValueParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLValuedExpression__Alternatives1567);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:756:1: rule__WMLMacroParameter__Alternatives : ( ( ruleWMLValue ) | ( ruleMacroTokens ) );
    public final void rule__WMLMacroParameter__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:760:1: ( ( ruleWMLValue ) | ( ruleMacroTokens ) )
            int alt9=2;
            int LA9_0 = input.LA(1);

            if ( ((LA9_0>=RULE_ID && LA9_0<=RULE_ANY_OTHER)||(LA9_0>=22 && LA9_0<=29)) ) {
                alt9=1;
            }
            else if ( ((LA9_0>=30 && LA9_0<=34)) ) {
                alt9=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("", 9, 0, input);

                throw nvae;
            }
            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:761:1: ( ruleWMLValue )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:761:1: ( ruleWMLValue )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:762:1: ruleWMLValue
                    {
                     before(grammarAccess.getWMLMacroParameterAccess().getWMLValueParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLMacroParameter__Alternatives1599);
                    ruleWMLValue();

                    state._fsp--;

                     after(grammarAccess.getWMLMacroParameterAccess().getWMLValueParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:767:6: ( ruleMacroTokens )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:767:6: ( ruleMacroTokens )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:768:1: ruleMacroTokens
                    {
                     before(grammarAccess.getWMLMacroParameterAccess().getMacroTokensParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleMacroTokens_in_rule__WMLMacroParameter__Alternatives1616);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:778:1: rule__WMLValue__ValueAlternatives_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( '~' ) | ( '.' ) | ( './' ) | ( '$' ) | ( '/' ) | ( '(' ) | ( ')' ) | ( RULE_ANY_OTHER ) );
    public final void rule__WMLValue__ValueAlternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:782:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( '_' ) | ( '~' ) | ( '.' ) | ( './' ) | ( '$' ) | ( '/' ) | ( '(' ) | ( ')' ) | ( RULE_ANY_OTHER ) )
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
            case 22:
                {
                alt10=3;
                }
                break;
            case 23:
                {
                alt10=4;
                }
                break;
            case 24:
                {
                alt10=5;
                }
                break;
            case 25:
                {
                alt10=6;
                }
                break;
            case 26:
                {
                alt10=7;
                }
                break;
            case 27:
                {
                alt10=8;
                }
                break;
            case 28:
                {
                alt10=9;
                }
                break;
            case 29:
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:783:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:783:1: ( RULE_ID )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:784:1: RULE_ID
                    {
                     before(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLValue__ValueAlternatives_01648); 
                     after(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:789:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:789:6: ( RULE_STRING )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:790:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLValue__ValueAlternatives_01665); 
                     after(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:795:6: ( '_' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:795:6: ( '_' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:796:1: '_'
                    {
                     before(grammarAccess.getWMLValueAccess().getValue_Keyword_0_2()); 
                    match(input,22,FOLLOW_22_in_rule__WMLValue__ValueAlternatives_01683); 
                     after(grammarAccess.getWMLValueAccess().getValue_Keyword_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:803:6: ( '~' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:803:6: ( '~' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:804:1: '~'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_3()); 
                    match(input,23,FOLLOW_23_in_rule__WMLValue__ValueAlternatives_01703); 
                     after(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:811:6: ( '.' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:811:6: ( '.' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:812:1: '.'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueFullStopKeyword_0_4()); 
                    match(input,24,FOLLOW_24_in_rule__WMLValue__ValueAlternatives_01723); 
                     after(grammarAccess.getWMLValueAccess().getValueFullStopKeyword_0_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:819:6: ( './' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:819:6: ( './' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:820:1: './'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueFullStopSolidusKeyword_0_5()); 
                    match(input,25,FOLLOW_25_in_rule__WMLValue__ValueAlternatives_01743); 
                     after(grammarAccess.getWMLValueAccess().getValueFullStopSolidusKeyword_0_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:827:6: ( '$' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:827:6: ( '$' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:828:1: '$'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueDollarSignKeyword_0_6()); 
                    match(input,26,FOLLOW_26_in_rule__WMLValue__ValueAlternatives_01763); 
                     after(grammarAccess.getWMLValueAccess().getValueDollarSignKeyword_0_6()); 

                    }


                    }
                    break;
                case 8 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:835:6: ( '/' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:835:6: ( '/' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:836:1: '/'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueSolidusKeyword_0_7()); 
                    match(input,27,FOLLOW_27_in_rule__WMLValue__ValueAlternatives_01783); 
                     after(grammarAccess.getWMLValueAccess().getValueSolidusKeyword_0_7()); 

                    }


                    }
                    break;
                case 9 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:843:6: ( '(' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:843:6: ( '(' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:844:1: '('
                    {
                     before(grammarAccess.getWMLValueAccess().getValueLeftParenthesisKeyword_0_8()); 
                    match(input,28,FOLLOW_28_in_rule__WMLValue__ValueAlternatives_01803); 
                     after(grammarAccess.getWMLValueAccess().getValueLeftParenthesisKeyword_0_8()); 

                    }


                    }
                    break;
                case 10 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:851:6: ( ')' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:851:6: ( ')' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:852:1: ')'
                    {
                     before(grammarAccess.getWMLValueAccess().getValueRightParenthesisKeyword_0_9()); 
                    match(input,29,FOLLOW_29_in_rule__WMLValue__ValueAlternatives_01823); 
                     after(grammarAccess.getWMLValueAccess().getValueRightParenthesisKeyword_0_9()); 

                    }


                    }
                    break;
                case 11 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:859:6: ( RULE_ANY_OTHER )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:859:6: ( RULE_ANY_OTHER )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:860:1: RULE_ANY_OTHER
                    {
                     before(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_10()); 
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__ValueAlternatives_01842); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:870:1: rule__MacroTokens__ValueAlternatives_0 : ( ( '=' ) | ( '[' ) | ( ']' ) | ( '+' ) | ( '[/' ) );
    public final void rule__MacroTokens__ValueAlternatives_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:874:1: ( ( '=' ) | ( '[' ) | ( ']' ) | ( '+' ) | ( '[/' ) )
            int alt11=5;
            switch ( input.LA(1) ) {
            case 30:
                {
                alt11=1;
                }
                break;
            case 31:
                {
                alt11=2;
                }
                break;
            case 32:
                {
                alt11=3;
                }
                break;
            case 33:
                {
                alt11=4;
                }
                break;
            case 34:
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
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:875:1: ( '=' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:875:1: ( '=' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:876:1: '='
                    {
                     before(grammarAccess.getMacroTokensAccess().getValueEqualsSignKeyword_0_0()); 
                    match(input,30,FOLLOW_30_in_rule__MacroTokens__ValueAlternatives_01875); 
                     after(grammarAccess.getMacroTokensAccess().getValueEqualsSignKeyword_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:883:6: ( '[' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:883:6: ( '[' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:884:1: '['
                    {
                     before(grammarAccess.getMacroTokensAccess().getValueLeftSquareBracketKeyword_0_1()); 
                    match(input,31,FOLLOW_31_in_rule__MacroTokens__ValueAlternatives_01895); 
                     after(grammarAccess.getMacroTokensAccess().getValueLeftSquareBracketKeyword_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:891:6: ( ']' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:891:6: ( ']' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:892:1: ']'
                    {
                     before(grammarAccess.getMacroTokensAccess().getValueRightSquareBracketKeyword_0_2()); 
                    match(input,32,FOLLOW_32_in_rule__MacroTokens__ValueAlternatives_01915); 
                     after(grammarAccess.getMacroTokensAccess().getValueRightSquareBracketKeyword_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:899:6: ( '+' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:899:6: ( '+' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:900:1: '+'
                    {
                     before(grammarAccess.getMacroTokensAccess().getValuePlusSignKeyword_0_3()); 
                    match(input,33,FOLLOW_33_in_rule__MacroTokens__ValueAlternatives_01935); 
                     after(grammarAccess.getMacroTokensAccess().getValuePlusSignKeyword_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:907:6: ( '[/' )
                    {
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:907:6: ( '[/' )
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:908:1: '[/'
                    {
                     before(grammarAccess.getMacroTokensAccess().getValueLeftSquareBracketSolidusKeyword_0_4()); 
                    match(input,34,FOLLOW_34_in_rule__MacroTokens__ValueAlternatives_01955); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:922:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:926:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:927:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01987);
            rule__WMLTag__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01990);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:934:1: rule__WMLTag__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:938:1: ( ( '[' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:939:1: ( '[' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:939:1: ( '[' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:940:1: '['
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0()); 
            match(input,31,FOLLOW_31_in_rule__WMLTag__Group__0__Impl2018); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:953:1: rule__WMLTag__Group__1 : rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 ;
    public final void rule__WMLTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:957:1: ( rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:958:2: rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__12049);
            rule__WMLTag__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__12052);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:965:1: rule__WMLTag__Group__1__Impl : ( ( rule__WMLTag__PlusAssignment_1 )? ) ;
    public final void rule__WMLTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:969:1: ( ( ( rule__WMLTag__PlusAssignment_1 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:970:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:970:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:971:1: ( rule__WMLTag__PlusAssignment_1 )?
            {
             before(grammarAccess.getWMLTagAccess().getPlusAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:972:1: ( rule__WMLTag__PlusAssignment_1 )?
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0==33) ) {
                alt12=1;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:972:2: rule__WMLTag__PlusAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl2079);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:982:1: rule__WMLTag__Group__2 : rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 ;
    public final void rule__WMLTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:986:1: ( rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:987:2: rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__22110);
            rule__WMLTag__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__22113);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:994:1: rule__WMLTag__Group__2__Impl : ( ( rule__WMLTag__NameAssignment_2 ) ) ;
    public final void rule__WMLTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:998:1: ( ( ( rule__WMLTag__NameAssignment_2 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:999:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:999:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1000:1: ( rule__WMLTag__NameAssignment_2 )
            {
             before(grammarAccess.getWMLTagAccess().getNameAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1001:1: ( rule__WMLTag__NameAssignment_2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1001:2: rule__WMLTag__NameAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl2140);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1011:1: rule__WMLTag__Group__3 : rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 ;
    public final void rule__WMLTag__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1015:1: ( rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1016:2: rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__32170);
            rule__WMLTag__Group__3__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__32173);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1023:1: rule__WMLTag__Group__3__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1027:1: ( ( ']' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1028:1: ( ']' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1028:1: ( ']' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1029:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3()); 
            match(input,32,FOLLOW_32_in_rule__WMLTag__Group__3__Impl2201); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1042:1: rule__WMLTag__Group__4 : rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 ;
    public final void rule__WMLTag__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1046:1: ( rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1047:2: rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__42232);
            rule__WMLTag__Group__4__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__42235);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1054:1: rule__WMLTag__Group__4__Impl : ( ( rule__WMLTag__ExpressionsAssignment_4 )* ) ;
    public final void rule__WMLTag__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1058:1: ( ( ( rule__WMLTag__ExpressionsAssignment_4 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1059:1: ( ( rule__WMLTag__ExpressionsAssignment_4 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1059:1: ( ( rule__WMLTag__ExpressionsAssignment_4 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1060:1: ( rule__WMLTag__ExpressionsAssignment_4 )*
            {
             before(grammarAccess.getWMLTagAccess().getExpressionsAssignment_4()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1061:1: ( rule__WMLTag__ExpressionsAssignment_4 )*
            loop13:
            do {
                int alt13=2;
                int LA13_0 = input.LA(1);

                if ( ((LA13_0>=RULE_IFDEF && LA13_0<=RULE_ID)||LA13_0==RULE_DEFINE||LA13_0==RULE_TEXTDOMAIN||LA13_0==31||LA13_0==35) ) {
                    alt13=1;
                }


                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1061:2: rule__WMLTag__ExpressionsAssignment_4
            	    {
            	    pushFollow(FOLLOW_rule__WMLTag__ExpressionsAssignment_4_in_rule__WMLTag__Group__4__Impl2262);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1071:1: rule__WMLTag__Group__5 : rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 ;
    public final void rule__WMLTag__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1075:1: ( rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1076:2: rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__52293);
            rule__WMLTag__Group__5__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__52296);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1083:1: rule__WMLTag__Group__5__Impl : ( '[/' ) ;
    public final void rule__WMLTag__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1087:1: ( ( '[/' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1088:1: ( '[/' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1088:1: ( '[/' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1089:1: '[/'
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5()); 
            match(input,34,FOLLOW_34_in_rule__WMLTag__Group__5__Impl2324); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1102:1: rule__WMLTag__Group__6 : rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 ;
    public final void rule__WMLTag__Group__6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1106:1: ( rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1107:2: rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__62355);
            rule__WMLTag__Group__6__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__62358);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1114:1: rule__WMLTag__Group__6__Impl : ( ( rule__WMLTag__EndNameAssignment_6 ) ) ;
    public final void rule__WMLTag__Group__6__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1118:1: ( ( ( rule__WMLTag__EndNameAssignment_6 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1119:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1119:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1120:1: ( rule__WMLTag__EndNameAssignment_6 )
            {
             before(grammarAccess.getWMLTagAccess().getEndNameAssignment_6()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1121:1: ( rule__WMLTag__EndNameAssignment_6 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1121:2: rule__WMLTag__EndNameAssignment_6
            {
            pushFollow(FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl2385);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1131:1: rule__WMLTag__Group__7 : rule__WMLTag__Group__7__Impl ;
    public final void rule__WMLTag__Group__7() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1135:1: ( rule__WMLTag__Group__7__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1136:2: rule__WMLTag__Group__7__Impl
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__72415);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1142:1: rule__WMLTag__Group__7__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__7__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1146:1: ( ( ']' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1147:1: ( ']' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1147:1: ( ']' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1148:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_7()); 
            match(input,32,FOLLOW_32_in_rule__WMLTag__Group__7__Impl2443); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1177:1: rule__WMLKey__Group__0 : rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 ;
    public final void rule__WMLKey__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1181:1: ( rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1182:2: rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02490);
            rule__WMLKey__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02493);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1189:1: rule__WMLKey__Group__0__Impl : ( ( rule__WMLKey__NameAssignment_0 ) ) ;
    public final void rule__WMLKey__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1193:1: ( ( ( rule__WMLKey__NameAssignment_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1194:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1194:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1195:1: ( rule__WMLKey__NameAssignment_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getNameAssignment_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1196:1: ( rule__WMLKey__NameAssignment_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1196:2: rule__WMLKey__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl2520);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1206:1: rule__WMLKey__Group__1 : rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 ;
    public final void rule__WMLKey__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1210:1: ( rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1211:2: rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12550);
            rule__WMLKey__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12553);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1218:1: rule__WMLKey__Group__1__Impl : ( '=' ) ;
    public final void rule__WMLKey__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1222:1: ( ( '=' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1223:1: ( '=' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1223:1: ( '=' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1224:1: '='
            {
             before(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1()); 
            match(input,30,FOLLOW_30_in_rule__WMLKey__Group__1__Impl2581); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1237:1: rule__WMLKey__Group__2 : rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 ;
    public final void rule__WMLKey__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1241:1: ( rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1242:2: rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22612);
            rule__WMLKey__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__22615);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1249:1: rule__WMLKey__Group__2__Impl : ( ( rule__WMLKey__ValuesAssignment_2 )* ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1253:1: ( ( ( rule__WMLKey__ValuesAssignment_2 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1254:1: ( ( rule__WMLKey__ValuesAssignment_2 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1254:1: ( ( rule__WMLKey__ValuesAssignment_2 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1255:1: ( rule__WMLKey__ValuesAssignment_2 )*
            {
             before(grammarAccess.getWMLKeyAccess().getValuesAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1256:1: ( rule__WMLKey__ValuesAssignment_2 )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( ((LA14_0>=RULE_ID && LA14_0<=RULE_ANY_OTHER)||LA14_0==RULE_LUA_CODE||(LA14_0>=22 && LA14_0<=29)||LA14_0==31||LA14_0==35) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1256:2: rule__WMLKey__ValuesAssignment_2
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__ValuesAssignment_2_in_rule__WMLKey__Group__2__Impl2642);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1266:1: rule__WMLKey__Group__3 : rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4 ;
    public final void rule__WMLKey__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1270:1: ( rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1271:2: rule__WMLKey__Group__3__Impl rule__WMLKey__Group__4
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__32673);
            rule__WMLKey__Group__3__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__4_in_rule__WMLKey__Group__32676);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1278:1: rule__WMLKey__Group__3__Impl : ( ( rule__WMLKey__Group_3__0 )* ) ;
    public final void rule__WMLKey__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1282:1: ( ( ( rule__WMLKey__Group_3__0 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1283:1: ( ( rule__WMLKey__Group_3__0 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1283:1: ( ( rule__WMLKey__Group_3__0 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1284:1: ( rule__WMLKey__Group_3__0 )*
            {
             before(grammarAccess.getWMLKeyAccess().getGroup_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1285:1: ( rule__WMLKey__Group_3__0 )*
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( (LA15_0==RULE_EOL) ) {
                    int LA15_1 = input.LA(2);

                    if ( (LA15_1==33) ) {
                        alt15=1;
                    }


                }
                else if ( (LA15_0==33) ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1285:2: rule__WMLKey__Group_3__0
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__Group_3__0_in_rule__WMLKey__Group__3__Impl2703);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1295:1: rule__WMLKey__Group__4 : rule__WMLKey__Group__4__Impl ;
    public final void rule__WMLKey__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1299:1: ( rule__WMLKey__Group__4__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1300:2: rule__WMLKey__Group__4__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__4__Impl_in_rule__WMLKey__Group__42734);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1306:1: rule__WMLKey__Group__4__Impl : ( ( rule__WMLKey__EolAssignment_4 ) ) ;
    public final void rule__WMLKey__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1310:1: ( ( ( rule__WMLKey__EolAssignment_4 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1311:1: ( ( rule__WMLKey__EolAssignment_4 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1311:1: ( ( rule__WMLKey__EolAssignment_4 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1312:1: ( rule__WMLKey__EolAssignment_4 )
            {
             before(grammarAccess.getWMLKeyAccess().getEolAssignment_4()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1313:1: ( rule__WMLKey__EolAssignment_4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1313:2: rule__WMLKey__EolAssignment_4
            {
            pushFollow(FOLLOW_rule__WMLKey__EolAssignment_4_in_rule__WMLKey__Group__4__Impl2761);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1333:1: rule__WMLKey__Group_3__0 : rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1 ;
    public final void rule__WMLKey__Group_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1337:1: ( rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1338:2: rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__0__Impl_in_rule__WMLKey__Group_3__02801);
            rule__WMLKey__Group_3__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__1_in_rule__WMLKey__Group_3__02804);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1345:1: rule__WMLKey__Group_3__0__Impl : ( ( RULE_EOL )? ) ;
    public final void rule__WMLKey__Group_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1349:1: ( ( ( RULE_EOL )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1350:1: ( ( RULE_EOL )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1350:1: ( ( RULE_EOL )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1351:1: ( RULE_EOL )?
            {
             before(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1352:1: ( RULE_EOL )?
            int alt16=2;
            int LA16_0 = input.LA(1);

            if ( (LA16_0==RULE_EOL) ) {
                alt16=1;
            }
            switch (alt16) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1352:3: RULE_EOL
                    {
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__0__Impl2832); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1362:1: rule__WMLKey__Group_3__1 : rule__WMLKey__Group_3__1__Impl rule__WMLKey__Group_3__2 ;
    public final void rule__WMLKey__Group_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1366:1: ( rule__WMLKey__Group_3__1__Impl rule__WMLKey__Group_3__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1367:2: rule__WMLKey__Group_3__1__Impl rule__WMLKey__Group_3__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__1__Impl_in_rule__WMLKey__Group_3__12863);
            rule__WMLKey__Group_3__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__2_in_rule__WMLKey__Group_3__12866);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1374:1: rule__WMLKey__Group_3__1__Impl : ( '+' ) ;
    public final void rule__WMLKey__Group_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1378:1: ( ( '+' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1379:1: ( '+' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1379:1: ( '+' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1380:1: '+'
            {
             before(grammarAccess.getWMLKeyAccess().getPlusSignKeyword_3_1()); 
            match(input,33,FOLLOW_33_in_rule__WMLKey__Group_3__1__Impl2894); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1393:1: rule__WMLKey__Group_3__2 : rule__WMLKey__Group_3__2__Impl rule__WMLKey__Group_3__3 ;
    public final void rule__WMLKey__Group_3__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1397:1: ( rule__WMLKey__Group_3__2__Impl rule__WMLKey__Group_3__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1398:2: rule__WMLKey__Group_3__2__Impl rule__WMLKey__Group_3__3
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__2__Impl_in_rule__WMLKey__Group_3__22925);
            rule__WMLKey__Group_3__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__3_in_rule__WMLKey__Group_3__22928);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1405:1: rule__WMLKey__Group_3__2__Impl : ( ( RULE_EOL )? ) ;
    public final void rule__WMLKey__Group_3__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1409:1: ( ( ( RULE_EOL )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1410:1: ( ( RULE_EOL )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1410:1: ( ( RULE_EOL )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1411:1: ( RULE_EOL )?
            {
             before(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1412:1: ( RULE_EOL )?
            int alt17=2;
            int LA17_0 = input.LA(1);

            if ( (LA17_0==RULE_EOL) ) {
                alt17=1;
            }
            switch (alt17) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1412:3: RULE_EOL
                    {
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__2__Impl2956); 

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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1422:1: rule__WMLKey__Group_3__3 : rule__WMLKey__Group_3__3__Impl ;
    public final void rule__WMLKey__Group_3__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1426:1: ( rule__WMLKey__Group_3__3__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1427:2: rule__WMLKey__Group_3__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__3__Impl_in_rule__WMLKey__Group_3__32987);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1433:1: rule__WMLKey__Group_3__3__Impl : ( ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* ) ) ;
    public final void rule__WMLKey__Group_3__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1437:1: ( ( ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1438:1: ( ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1438:1: ( ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1439:1: ( ( rule__WMLKey__ValuesAssignment_3_3 ) ) ( ( rule__WMLKey__ValuesAssignment_3_3 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1439:1: ( ( rule__WMLKey__ValuesAssignment_3_3 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1440:1: ( rule__WMLKey__ValuesAssignment_3_3 )
            {
             before(grammarAccess.getWMLKeyAccess().getValuesAssignment_3_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1441:1: ( rule__WMLKey__ValuesAssignment_3_3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1441:2: rule__WMLKey__ValuesAssignment_3_3
            {
            pushFollow(FOLLOW_rule__WMLKey__ValuesAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl3016);
            rule__WMLKey__ValuesAssignment_3_3();

            state._fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getValuesAssignment_3_3()); 

            }

            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1444:1: ( ( rule__WMLKey__ValuesAssignment_3_3 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1445:1: ( rule__WMLKey__ValuesAssignment_3_3 )*
            {
             before(grammarAccess.getWMLKeyAccess().getValuesAssignment_3_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1446:1: ( rule__WMLKey__ValuesAssignment_3_3 )*
            loop18:
            do {
                int alt18=2;
                int LA18_0 = input.LA(1);

                if ( ((LA18_0>=RULE_ID && LA18_0<=RULE_ANY_OTHER)||LA18_0==RULE_LUA_CODE||(LA18_0>=22 && LA18_0<=29)||LA18_0==31||LA18_0==35) ) {
                    alt18=1;
                }


                switch (alt18) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1446:2: rule__WMLKey__ValuesAssignment_3_3
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__ValuesAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl3028);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1465:1: rule__WMLMacroCall__Group__0 : rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 ;
    public final void rule__WMLMacroCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1469:1: ( rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1470:2: rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__03069);
            rule__WMLMacroCall__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__03072);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1477:1: rule__WMLMacroCall__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacroCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1481:1: ( ( '{' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1482:1: ( '{' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1482:1: ( '{' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1483:1: '{'
            {
             before(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,35,FOLLOW_35_in_rule__WMLMacroCall__Group__0__Impl3100); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1496:1: rule__WMLMacroCall__Group__1 : rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 ;
    public final void rule__WMLMacroCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1500:1: ( rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1501:2: rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__13131);
            rule__WMLMacroCall__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__13134);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1508:1: rule__WMLMacroCall__Group__1__Impl : ( ( rule__WMLMacroCall__PointAssignment_1 )? ) ;
    public final void rule__WMLMacroCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1512:1: ( ( ( rule__WMLMacroCall__PointAssignment_1 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1513:1: ( ( rule__WMLMacroCall__PointAssignment_1 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1513:1: ( ( rule__WMLMacroCall__PointAssignment_1 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1514:1: ( rule__WMLMacroCall__PointAssignment_1 )?
            {
             before(grammarAccess.getWMLMacroCallAccess().getPointAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1515:1: ( rule__WMLMacroCall__PointAssignment_1 )?
            int alt19=2;
            int LA19_0 = input.LA(1);

            if ( (LA19_0==25) ) {
                alt19=1;
            }
            switch (alt19) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1515:2: rule__WMLMacroCall__PointAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__PointAssignment_1_in_rule__WMLMacroCall__Group__1__Impl3161);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1525:1: rule__WMLMacroCall__Group__2 : rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 ;
    public final void rule__WMLMacroCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1529:1: ( rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1530:2: rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__23192);
            rule__WMLMacroCall__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__23195);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1537:1: rule__WMLMacroCall__Group__2__Impl : ( ( rule__WMLMacroCall__RelativeAssignment_2 )? ) ;
    public final void rule__WMLMacroCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1541:1: ( ( ( rule__WMLMacroCall__RelativeAssignment_2 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1542:1: ( ( rule__WMLMacroCall__RelativeAssignment_2 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1542:1: ( ( rule__WMLMacroCall__RelativeAssignment_2 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1543:1: ( rule__WMLMacroCall__RelativeAssignment_2 )?
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1544:1: ( rule__WMLMacroCall__RelativeAssignment_2 )?
            int alt20=2;
            int LA20_0 = input.LA(1);

            if ( (LA20_0==23) ) {
                alt20=1;
            }
            switch (alt20) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1544:2: rule__WMLMacroCall__RelativeAssignment_2
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__RelativeAssignment_2_in_rule__WMLMacroCall__Group__2__Impl3222);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1554:1: rule__WMLMacroCall__Group__3 : rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 ;
    public final void rule__WMLMacroCall__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1558:1: ( rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1559:2: rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__33253);
            rule__WMLMacroCall__Group__3__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__33256);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1566:1: rule__WMLMacroCall__Group__3__Impl : ( ( rule__WMLMacroCall__NameAssignment_3 ) ) ;
    public final void rule__WMLMacroCall__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1570:1: ( ( ( rule__WMLMacroCall__NameAssignment_3 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1571:1: ( ( rule__WMLMacroCall__NameAssignment_3 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1571:1: ( ( rule__WMLMacroCall__NameAssignment_3 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1572:1: ( rule__WMLMacroCall__NameAssignment_3 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameAssignment_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1573:1: ( rule__WMLMacroCall__NameAssignment_3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1573:2: rule__WMLMacroCall__NameAssignment_3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__NameAssignment_3_in_rule__WMLMacroCall__Group__3__Impl3283);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1583:1: rule__WMLMacroCall__Group__4 : rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5 ;
    public final void rule__WMLMacroCall__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1587:1: ( rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1588:2: rule__WMLMacroCall__Group__4__Impl rule__WMLMacroCall__Group__5
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__43313);
            rule__WMLMacroCall__Group__4__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__5_in_rule__WMLMacroCall__Group__43316);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1595:1: rule__WMLMacroCall__Group__4__Impl : ( ( rule__WMLMacroCall__ParametersAssignment_4 )* ) ;
    public final void rule__WMLMacroCall__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1599:1: ( ( ( rule__WMLMacroCall__ParametersAssignment_4 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1600:1: ( ( rule__WMLMacroCall__ParametersAssignment_4 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1600:1: ( ( rule__WMLMacroCall__ParametersAssignment_4 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1601:1: ( rule__WMLMacroCall__ParametersAssignment_4 )*
            {
             before(grammarAccess.getWMLMacroCallAccess().getParametersAssignment_4()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1602:1: ( rule__WMLMacroCall__ParametersAssignment_4 )*
            loop21:
            do {
                int alt21=2;
                int LA21_0 = input.LA(1);

                if ( ((LA21_0>=RULE_ID && LA21_0<=RULE_ANY_OTHER)||(LA21_0>=22 && LA21_0<=35)) ) {
                    alt21=1;
                }


                switch (alt21) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1602:2: rule__WMLMacroCall__ParametersAssignment_4
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroCall__ParametersAssignment_4_in_rule__WMLMacroCall__Group__4__Impl3343);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1612:1: rule__WMLMacroCall__Group__5 : rule__WMLMacroCall__Group__5__Impl ;
    public final void rule__WMLMacroCall__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1616:1: ( rule__WMLMacroCall__Group__5__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1617:2: rule__WMLMacroCall__Group__5__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__5__Impl_in_rule__WMLMacroCall__Group__53374);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1623:1: rule__WMLMacroCall__Group__5__Impl : ( '}' ) ;
    public final void rule__WMLMacroCall__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1627:1: ( ( '}' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1628:1: ( '}' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1628:1: ( '}' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1629:1: '}'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_5()); 
            match(input,36,FOLLOW_36_in_rule__WMLMacroCall__Group__5__Impl3402); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1654:1: rule__WMLArrayCall__Group__0 : rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 ;
    public final void rule__WMLArrayCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1658:1: ( rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1659:2: rule__WMLArrayCall__Group__0__Impl rule__WMLArrayCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__03445);
            rule__WMLArrayCall__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__03448);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1666:1: rule__WMLArrayCall__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLArrayCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1670:1: ( ( '[' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1671:1: ( '[' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1671:1: ( '[' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1672:1: '['
            {
             before(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0()); 
            match(input,31,FOLLOW_31_in_rule__WMLArrayCall__Group__0__Impl3476); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1685:1: rule__WMLArrayCall__Group__1 : rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 ;
    public final void rule__WMLArrayCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1689:1: ( rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1690:2: rule__WMLArrayCall__Group__1__Impl rule__WMLArrayCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__13507);
            rule__WMLArrayCall__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__13510);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1697:1: rule__WMLArrayCall__Group__1__Impl : ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) ;
    public final void rule__WMLArrayCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1701:1: ( ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1702:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1702:1: ( ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1703:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) ) ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1703:1: ( ( rule__WMLArrayCall__ValueAssignment_1 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1704:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1705:1: ( rule__WMLArrayCall__ValueAssignment_1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1705:2: rule__WMLArrayCall__ValueAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3539);
            rule__WMLArrayCall__ValueAssignment_1();

            state._fsp--;


            }

             after(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 

            }

            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1708:1: ( ( rule__WMLArrayCall__ValueAssignment_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1709:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1710:1: ( rule__WMLArrayCall__ValueAssignment_1 )*
            loop22:
            do {
                int alt22=2;
                int LA22_0 = input.LA(1);

                if ( ((LA22_0>=RULE_ID && LA22_0<=RULE_ANY_OTHER)||(LA22_0>=22 && LA22_0<=29)) ) {
                    alt22=1;
                }


                switch (alt22) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1710:2: rule__WMLArrayCall__ValueAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3551);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1721:1: rule__WMLArrayCall__Group__2 : rule__WMLArrayCall__Group__2__Impl ;
    public final void rule__WMLArrayCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1725:1: ( rule__WMLArrayCall__Group__2__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1726:2: rule__WMLArrayCall__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__23584);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1732:1: rule__WMLArrayCall__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLArrayCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1736:1: ( ( ']' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1737:1: ( ']' )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1737:1: ( ']' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1738:1: ']'
            {
             before(grammarAccess.getWMLArrayCallAccess().getRightSquareBracketKeyword_2()); 
            match(input,32,FOLLOW_32_in_rule__WMLArrayCall__Group__2__Impl3612); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1757:1: rule__WMLMacroDefine__Group__0 : rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1 ;
    public final void rule__WMLMacroDefine__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1761:1: ( rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1762:2: rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__0__Impl_in_rule__WMLMacroDefine__Group__03649);
            rule__WMLMacroDefine__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__1_in_rule__WMLMacroDefine__Group__03652);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1769:1: rule__WMLMacroDefine__Group__0__Impl : ( ( rule__WMLMacroDefine__NameAssignment_0 ) ) ;
    public final void rule__WMLMacroDefine__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1773:1: ( ( ( rule__WMLMacroDefine__NameAssignment_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1774:1: ( ( rule__WMLMacroDefine__NameAssignment_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1774:1: ( ( rule__WMLMacroDefine__NameAssignment_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1775:1: ( rule__WMLMacroDefine__NameAssignment_0 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameAssignment_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1776:1: ( rule__WMLMacroDefine__NameAssignment_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1776:2: rule__WMLMacroDefine__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__NameAssignment_0_in_rule__WMLMacroDefine__Group__0__Impl3679);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1786:1: rule__WMLMacroDefine__Group__1 : rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2 ;
    public final void rule__WMLMacroDefine__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1790:1: ( rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1791:2: rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__1__Impl_in_rule__WMLMacroDefine__Group__13709);
            rule__WMLMacroDefine__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__2_in_rule__WMLMacroDefine__Group__13712);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1798:1: rule__WMLMacroDefine__Group__1__Impl : ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* ) ;
    public final void rule__WMLMacroDefine__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1802:1: ( ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1803:1: ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1803:1: ( ( rule__WMLMacroDefine__ExpressionsAssignment_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1804:1: ( rule__WMLMacroDefine__ExpressionsAssignment_1 )*
            {
             before(grammarAccess.getWMLMacroDefineAccess().getExpressionsAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1805:1: ( rule__WMLMacroDefine__ExpressionsAssignment_1 )*
            loop23:
            do {
                int alt23=2;
                int LA23_0 = input.LA(1);

                if ( ((LA23_0>=RULE_IFDEF && LA23_0<=RULE_DEFINE)||LA23_0==RULE_TEXTDOMAIN||(LA23_0>=22 && LA23_0<=29)||LA23_0==31||LA23_0==35) ) {
                    alt23=1;
                }


                switch (alt23) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1805:2: rule__WMLMacroDefine__ExpressionsAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroDefine__ExpressionsAssignment_1_in_rule__WMLMacroDefine__Group__1__Impl3739);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1815:1: rule__WMLMacroDefine__Group__2 : rule__WMLMacroDefine__Group__2__Impl ;
    public final void rule__WMLMacroDefine__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1819:1: ( rule__WMLMacroDefine__Group__2__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1820:2: rule__WMLMacroDefine__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__2__Impl_in_rule__WMLMacroDefine__Group__23770);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1826:1: rule__WMLMacroDefine__Group__2__Impl : ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) ) ;
    public final void rule__WMLMacroDefine__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1830:1: ( ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1831:1: ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1831:1: ( ( rule__WMLMacroDefine__EndNameAssignment_2 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1832:1: ( rule__WMLMacroDefine__EndNameAssignment_2 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getEndNameAssignment_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1833:1: ( rule__WMLMacroDefine__EndNameAssignment_2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1833:2: rule__WMLMacroDefine__EndNameAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__EndNameAssignment_2_in_rule__WMLMacroDefine__Group__2__Impl3797);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1849:1: rule__WMLPreprocIF__Group__0 : rule__WMLPreprocIF__Group__0__Impl rule__WMLPreprocIF__Group__1 ;
    public final void rule__WMLPreprocIF__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1853:1: ( rule__WMLPreprocIF__Group__0__Impl rule__WMLPreprocIF__Group__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1854:2: rule__WMLPreprocIF__Group__0__Impl rule__WMLPreprocIF__Group__1
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__0__Impl_in_rule__WMLPreprocIF__Group__03833);
            rule__WMLPreprocIF__Group__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__1_in_rule__WMLPreprocIF__Group__03836);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1861:1: rule__WMLPreprocIF__Group__0__Impl : ( ( rule__WMLPreprocIF__NameAssignment_0 ) ) ;
    public final void rule__WMLPreprocIF__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1865:1: ( ( ( rule__WMLPreprocIF__NameAssignment_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1866:1: ( ( rule__WMLPreprocIF__NameAssignment_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1866:1: ( ( rule__WMLPreprocIF__NameAssignment_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1867:1: ( rule__WMLPreprocIF__NameAssignment_0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getNameAssignment_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1868:1: ( rule__WMLPreprocIF__NameAssignment_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1868:2: rule__WMLPreprocIF__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__NameAssignment_0_in_rule__WMLPreprocIF__Group__0__Impl3863);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1878:1: rule__WMLPreprocIF__Group__1 : rule__WMLPreprocIF__Group__1__Impl rule__WMLPreprocIF__Group__2 ;
    public final void rule__WMLPreprocIF__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1882:1: ( rule__WMLPreprocIF__Group__1__Impl rule__WMLPreprocIF__Group__2 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1883:2: rule__WMLPreprocIF__Group__1__Impl rule__WMLPreprocIF__Group__2
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__1__Impl_in_rule__WMLPreprocIF__Group__13893);
            rule__WMLPreprocIF__Group__1__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__2_in_rule__WMLPreprocIF__Group__13896);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1890:1: rule__WMLPreprocIF__Group__1__Impl : ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* ) ;
    public final void rule__WMLPreprocIF__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1894:1: ( ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1895:1: ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1895:1: ( ( rule__WMLPreprocIF__ExpressionsAssignment_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1896:1: ( rule__WMLPreprocIF__ExpressionsAssignment_1 )*
            {
             before(grammarAccess.getWMLPreprocIFAccess().getExpressionsAssignment_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1897:1: ( rule__WMLPreprocIF__ExpressionsAssignment_1 )*
            loop24:
            do {
                int alt24=2;
                int LA24_0 = input.LA(1);

                if ( ((LA24_0>=RULE_IFDEF && LA24_0<=RULE_DEFINE)||LA24_0==RULE_TEXTDOMAIN||(LA24_0>=22 && LA24_0<=29)||LA24_0==31||LA24_0==35) ) {
                    alt24=1;
                }


                switch (alt24) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1897:2: rule__WMLPreprocIF__ExpressionsAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLPreprocIF__ExpressionsAssignment_1_in_rule__WMLPreprocIF__Group__1__Impl3923);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1907:1: rule__WMLPreprocIF__Group__2 : rule__WMLPreprocIF__Group__2__Impl rule__WMLPreprocIF__Group__3 ;
    public final void rule__WMLPreprocIF__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1911:1: ( rule__WMLPreprocIF__Group__2__Impl rule__WMLPreprocIF__Group__3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1912:2: rule__WMLPreprocIF__Group__2__Impl rule__WMLPreprocIF__Group__3
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__2__Impl_in_rule__WMLPreprocIF__Group__23954);
            rule__WMLPreprocIF__Group__2__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__3_in_rule__WMLPreprocIF__Group__23957);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1919:1: rule__WMLPreprocIF__Group__2__Impl : ( ( rule__WMLPreprocIF__Group_2__0 )? ) ;
    public final void rule__WMLPreprocIF__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1923:1: ( ( ( rule__WMLPreprocIF__Group_2__0 )? ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1924:1: ( ( rule__WMLPreprocIF__Group_2__0 )? )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1924:1: ( ( rule__WMLPreprocIF__Group_2__0 )? )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1925:1: ( rule__WMLPreprocIF__Group_2__0 )?
            {
             before(grammarAccess.getWMLPreprocIFAccess().getGroup_2()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1926:1: ( rule__WMLPreprocIF__Group_2__0 )?
            int alt25=2;
            int LA25_0 = input.LA(1);

            if ( (LA25_0==RULE_ELSE) ) {
                alt25=1;
            }
            switch (alt25) {
                case 1 :
                    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1926:2: rule__WMLPreprocIF__Group_2__0
                    {
                    pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__0_in_rule__WMLPreprocIF__Group__2__Impl3984);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1936:1: rule__WMLPreprocIF__Group__3 : rule__WMLPreprocIF__Group__3__Impl ;
    public final void rule__WMLPreprocIF__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1940:1: ( rule__WMLPreprocIF__Group__3__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1941:2: rule__WMLPreprocIF__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group__3__Impl_in_rule__WMLPreprocIF__Group__34015);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1947:1: rule__WMLPreprocIF__Group__3__Impl : ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) ) ;
    public final void rule__WMLPreprocIF__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1951:1: ( ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1952:1: ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1952:1: ( ( rule__WMLPreprocIF__EndNameAssignment_3 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1953:1: ( rule__WMLPreprocIF__EndNameAssignment_3 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getEndNameAssignment_3()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1954:1: ( rule__WMLPreprocIF__EndNameAssignment_3 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1954:2: rule__WMLPreprocIF__EndNameAssignment_3
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__EndNameAssignment_3_in_rule__WMLPreprocIF__Group__3__Impl4042);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1972:1: rule__WMLPreprocIF__Group_2__0 : rule__WMLPreprocIF__Group_2__0__Impl rule__WMLPreprocIF__Group_2__1 ;
    public final void rule__WMLPreprocIF__Group_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1976:1: ( rule__WMLPreprocIF__Group_2__0__Impl rule__WMLPreprocIF__Group_2__1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1977:2: rule__WMLPreprocIF__Group_2__0__Impl rule__WMLPreprocIF__Group_2__1
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__0__Impl_in_rule__WMLPreprocIF__Group_2__04080);
            rule__WMLPreprocIF__Group_2__0__Impl();

            state._fsp--;

            pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__1_in_rule__WMLPreprocIF__Group_2__04083);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1984:1: rule__WMLPreprocIF__Group_2__0__Impl : ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) ) ;
    public final void rule__WMLPreprocIF__Group_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1988:1: ( ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1989:1: ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1989:1: ( ( rule__WMLPreprocIF__ElsesAssignment_2_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1990:1: ( rule__WMLPreprocIF__ElsesAssignment_2_0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElsesAssignment_2_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1991:1: ( rule__WMLPreprocIF__ElsesAssignment_2_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1991:2: rule__WMLPreprocIF__ElsesAssignment_2_0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__ElsesAssignment_2_0_in_rule__WMLPreprocIF__Group_2__0__Impl4110);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2001:1: rule__WMLPreprocIF__Group_2__1 : rule__WMLPreprocIF__Group_2__1__Impl ;
    public final void rule__WMLPreprocIF__Group_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2005:1: ( rule__WMLPreprocIF__Group_2__1__Impl )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2006:2: rule__WMLPreprocIF__Group_2__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__Group_2__1__Impl_in_rule__WMLPreprocIF__Group_2__14140);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2012:1: rule__WMLPreprocIF__Group_2__1__Impl : ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) ) ;
    public final void rule__WMLPreprocIF__Group_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2016:1: ( ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2017:1: ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2017:1: ( ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2018:1: ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) ) ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2018:1: ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2019:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2020:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2020:2: rule__WMLPreprocIF__ElseExpressionsAssignment_2_1
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4169);
            rule__WMLPreprocIF__ElseExpressionsAssignment_2_1();

            state._fsp--;


            }

             after(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 

            }

            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2023:1: ( ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )* )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2024:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )*
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsAssignment_2_1()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2025:1: ( rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 )*
            loop26:
            do {
                int alt26=2;
                int LA26_0 = input.LA(1);

                if ( ((LA26_0>=RULE_IFDEF && LA26_0<=RULE_DEFINE)||LA26_0==RULE_TEXTDOMAIN||(LA26_0>=22 && LA26_0<=29)||LA26_0==31||LA26_0==35) ) {
                    alt26=1;
                }


                switch (alt26) {
            	case 1 :
            	    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2025:2: rule__WMLPreprocIF__ElseExpressionsAssignment_2_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4181);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2041:1: rule__WMLRoot__ExpressionsAssignment : ( ruleWMLRootExpression ) ;
    public final void rule__WMLRoot__ExpressionsAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2045:1: ( ( ruleWMLRootExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2046:1: ( ruleWMLRootExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2046:1: ( ruleWMLRootExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2047:1: ruleWMLRootExpression
            {
             before(grammarAccess.getWMLRootAccess().getExpressionsWMLRootExpressionParserRuleCall_0()); 
            pushFollow(FOLLOW_ruleWMLRootExpression_in_rule__WMLRoot__ExpressionsAssignment4223);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2056:1: rule__WMLTag__PlusAssignment_1 : ( ( '+' ) ) ;
    public final void rule__WMLTag__PlusAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2060:1: ( ( ( '+' ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2061:1: ( ( '+' ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2061:1: ( ( '+' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2062:1: ( '+' )
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2063:1: ( '+' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2064:1: '+'
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            match(input,33,FOLLOW_33_in_rule__WMLTag__PlusAssignment_14259); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2079:1: rule__WMLTag__NameAssignment_2 : ( RULE_ID ) ;
    public final void rule__WMLTag__NameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2083:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2084:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2084:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2085:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_2_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_24298); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2094:1: rule__WMLTag__ExpressionsAssignment_4 : ( ruleWMLExpression ) ;
    public final void rule__WMLTag__ExpressionsAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2098:1: ( ( ruleWMLExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2099:1: ( ruleWMLExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2099:1: ( ruleWMLExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2100:1: ruleWMLExpression
            {
             before(grammarAccess.getWMLTagAccess().getExpressionsWMLExpressionParserRuleCall_4_0()); 
            pushFollow(FOLLOW_ruleWMLExpression_in_rule__WMLTag__ExpressionsAssignment_44329);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2109:1: rule__WMLTag__EndNameAssignment_6 : ( RULE_ID ) ;
    public final void rule__WMLTag__EndNameAssignment_6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2113:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2114:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2114:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2115:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_64360); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2124:1: rule__WMLKey__NameAssignment_0 : ( RULE_ID ) ;
    public final void rule__WMLKey__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2128:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2129:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2129:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2130:1: RULE_ID
            {
             before(grammarAccess.getWMLKeyAccess().getNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_04391); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2139:1: rule__WMLKey__ValuesAssignment_2 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValuesAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2143:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2144:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2144:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2145:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValuesWMLKeyValueParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValuesAssignment_24422);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2154:1: rule__WMLKey__ValuesAssignment_3_3 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValuesAssignment_3_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2158:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2159:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2159:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2160:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValuesWMLKeyValueParserRuleCall_3_3_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValuesAssignment_3_34453);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2169:1: rule__WMLKey__EolAssignment_4 : ( ( rule__WMLKey__EolAlternatives_4_0 ) ) ;
    public final void rule__WMLKey__EolAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2173:1: ( ( ( rule__WMLKey__EolAlternatives_4_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2174:1: ( ( rule__WMLKey__EolAlternatives_4_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2174:1: ( ( rule__WMLKey__EolAlternatives_4_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2175:1: ( rule__WMLKey__EolAlternatives_4_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getEolAlternatives_4_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2176:1: ( rule__WMLKey__EolAlternatives_4_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2176:2: rule__WMLKey__EolAlternatives_4_0
            {
            pushFollow(FOLLOW_rule__WMLKey__EolAlternatives_4_0_in_rule__WMLKey__EolAssignment_44484);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2185:1: rule__WMLMacroCall__PointAssignment_1 : ( ( './' ) ) ;
    public final void rule__WMLMacroCall__PointAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2189:1: ( ( ( './' ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2190:1: ( ( './' ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2190:1: ( ( './' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2191:1: ( './' )
            {
             before(grammarAccess.getWMLMacroCallAccess().getPointFullStopSolidusKeyword_1_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2192:1: ( './' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2193:1: './'
            {
             before(grammarAccess.getWMLMacroCallAccess().getPointFullStopSolidusKeyword_1_0()); 
            match(input,25,FOLLOW_25_in_rule__WMLMacroCall__PointAssignment_14522); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2208:1: rule__WMLMacroCall__RelativeAssignment_2 : ( ( '~' ) ) ;
    public final void rule__WMLMacroCall__RelativeAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2212:1: ( ( ( '~' ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2213:1: ( ( '~' ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2213:1: ( ( '~' ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2214:1: ( '~' )
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2215:1: ( '~' )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2216:1: '~'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0()); 
            match(input,23,FOLLOW_23_in_rule__WMLMacroCall__RelativeAssignment_24566); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2231:1: rule__WMLMacroCall__NameAssignment_3 : ( RULE_ID ) ;
    public final void rule__WMLMacroCall__NameAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2235:1: ( ( RULE_ID ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2236:1: ( RULE_ID )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2236:1: ( RULE_ID )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2237:1: RULE_ID
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_3_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_34605); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2246:1: rule__WMLMacroCall__ParametersAssignment_4 : ( ruleWMLMacroCallParameter ) ;
    public final void rule__WMLMacroCall__ParametersAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2250:1: ( ( ruleWMLMacroCallParameter ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2251:1: ( ruleWMLMacroCallParameter )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2251:1: ( ruleWMLMacroCallParameter )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2252:1: ruleWMLMacroCallParameter
            {
             before(grammarAccess.getWMLMacroCallAccess().getParametersWMLMacroCallParameterParserRuleCall_4_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_rule__WMLMacroCall__ParametersAssignment_44636);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2261:1: rule__WMLArrayCall__ValueAssignment_1 : ( ruleWMLValue ) ;
    public final void rule__WMLArrayCall__ValueAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2265:1: ( ( ruleWMLValue ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2266:1: ( ruleWMLValue )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2266:1: ( ruleWMLValue )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2267:1: ruleWMLValue
            {
             before(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_14667);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2276:1: rule__WMLMacroDefine__NameAssignment_0 : ( RULE_DEFINE ) ;
    public final void rule__WMLMacroDefine__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2280:1: ( ( RULE_DEFINE ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2281:1: ( RULE_DEFINE )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2281:1: ( RULE_DEFINE )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2282:1: RULE_DEFINE
            {
             before(grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0_0()); 
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment_04698); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2291:1: rule__WMLMacroDefine__ExpressionsAssignment_1 : ( ruleWMLValuedExpression ) ;
    public final void rule__WMLMacroDefine__ExpressionsAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2295:1: ( ( ruleWMLValuedExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2296:1: ( ruleWMLValuedExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2296:1: ( ruleWMLValuedExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2297:1: ruleWMLValuedExpression
            {
             before(grammarAccess.getWMLMacroDefineAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_rule__WMLMacroDefine__ExpressionsAssignment_14729);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2306:1: rule__WMLMacroDefine__EndNameAssignment_2 : ( RULE_ENDDEF ) ;
    public final void rule__WMLMacroDefine__EndNameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2310:1: ( ( RULE_ENDDEF ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2311:1: ( RULE_ENDDEF )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2311:1: ( RULE_ENDDEF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2312:1: RULE_ENDDEF
            {
             before(grammarAccess.getWMLMacroDefineAccess().getEndNameENDDEFTerminalRuleCall_2_0()); 
            match(input,RULE_ENDDEF,FOLLOW_RULE_ENDDEF_in_rule__WMLMacroDefine__EndNameAssignment_24760); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2321:1: rule__WMLPreprocIF__NameAssignment_0 : ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) ) ;
    public final void rule__WMLPreprocIF__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2325:1: ( ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2326:1: ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2326:1: ( ( rule__WMLPreprocIF__NameAlternatives_0_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2327:1: ( rule__WMLPreprocIF__NameAlternatives_0_0 )
            {
             before(grammarAccess.getWMLPreprocIFAccess().getNameAlternatives_0_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2328:1: ( rule__WMLPreprocIF__NameAlternatives_0_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2328:2: rule__WMLPreprocIF__NameAlternatives_0_0
            {
            pushFollow(FOLLOW_rule__WMLPreprocIF__NameAlternatives_0_0_in_rule__WMLPreprocIF__NameAssignment_04791);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2337:1: rule__WMLPreprocIF__ExpressionsAssignment_1 : ( ruleWMLValuedExpression ) ;
    public final void rule__WMLPreprocIF__ExpressionsAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2341:1: ( ( ruleWMLValuedExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2342:1: ( ruleWMLValuedExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2342:1: ( ruleWMLValuedExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2343:1: ruleWMLValuedExpression
            {
             before(grammarAccess.getWMLPreprocIFAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ExpressionsAssignment_14824);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2352:1: rule__WMLPreprocIF__ElsesAssignment_2_0 : ( RULE_ELSE ) ;
    public final void rule__WMLPreprocIF__ElsesAssignment_2_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2356:1: ( ( RULE_ELSE ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2357:1: ( RULE_ELSE )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2357:1: ( RULE_ELSE )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2358:1: RULE_ELSE
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElsesELSETerminalRuleCall_2_0_0()); 
            match(input,RULE_ELSE,FOLLOW_RULE_ELSE_in_rule__WMLPreprocIF__ElsesAssignment_2_04855); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2367:1: rule__WMLPreprocIF__ElseExpressionsAssignment_2_1 : ( ruleWMLValuedExpression ) ;
    public final void rule__WMLPreprocIF__ElseExpressionsAssignment_2_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2371:1: ( ( ruleWMLValuedExpression ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2372:1: ( ruleWMLValuedExpression )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2372:1: ( ruleWMLValuedExpression )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2373:1: ruleWMLValuedExpression
            {
             before(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsWMLValuedExpressionParserRuleCall_2_1_0()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ElseExpressionsAssignment_2_14886);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2382:1: rule__WMLPreprocIF__EndNameAssignment_3 : ( RULE_ENDIF ) ;
    public final void rule__WMLPreprocIF__EndNameAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2386:1: ( ( RULE_ENDIF ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2387:1: ( RULE_ENDIF )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2387:1: ( RULE_ENDIF )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2388:1: RULE_ENDIF
            {
             before(grammarAccess.getWMLPreprocIFAccess().getEndNameENDIFTerminalRuleCall_3_0()); 
            match(input,RULE_ENDIF,FOLLOW_RULE_ENDIF_in_rule__WMLPreprocIF__EndNameAssignment_34917); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2397:1: rule__WMLTextdomain__NameAssignment : ( RULE_TEXTDOMAIN ) ;
    public final void rule__WMLTextdomain__NameAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2401:1: ( ( RULE_TEXTDOMAIN ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2402:1: ( RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2402:1: ( RULE_TEXTDOMAIN )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2403:1: RULE_TEXTDOMAIN
            {
             before(grammarAccess.getWMLTextdomainAccess().getNameTEXTDOMAINTerminalRuleCall_0()); 
            match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment4948); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2412:1: rule__WMLLuaCode__ValueAssignment : ( RULE_LUA_CODE ) ;
    public final void rule__WMLLuaCode__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2416:1: ( ( RULE_LUA_CODE ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2417:1: ( RULE_LUA_CODE )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2417:1: ( RULE_LUA_CODE )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2418:1: RULE_LUA_CODE
            {
             before(grammarAccess.getWMLLuaCodeAccess().getValueLUA_CODETerminalRuleCall_0()); 
            match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment4979); 
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2427:1: rule__WMLValue__ValueAssignment : ( ( rule__WMLValue__ValueAlternatives_0 ) ) ;
    public final void rule__WMLValue__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2431:1: ( ( ( rule__WMLValue__ValueAlternatives_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2432:1: ( ( rule__WMLValue__ValueAlternatives_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2432:1: ( ( rule__WMLValue__ValueAlternatives_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2433:1: ( rule__WMLValue__ValueAlternatives_0 )
            {
             before(grammarAccess.getWMLValueAccess().getValueAlternatives_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2434:1: ( rule__WMLValue__ValueAlternatives_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2434:2: rule__WMLValue__ValueAlternatives_0
            {
            pushFollow(FOLLOW_rule__WMLValue__ValueAlternatives_0_in_rule__WMLValue__ValueAssignment5010);
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
    // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2443:1: rule__MacroTokens__ValueAssignment : ( ( rule__MacroTokens__ValueAlternatives_0 ) ) ;
    public final void rule__MacroTokens__ValueAssignment() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2447:1: ( ( ( rule__MacroTokens__ValueAlternatives_0 ) ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2448:1: ( ( rule__MacroTokens__ValueAlternatives_0 ) )
            {
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2448:1: ( ( rule__MacroTokens__ValueAlternatives_0 ) )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2449:1: ( rule__MacroTokens__ValueAlternatives_0 )
            {
             before(grammarAccess.getMacroTokensAccess().getValueAlternatives_0()); 
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2450:1: ( rule__MacroTokens__ValueAlternatives_0 )
            // ../org.wesnoth.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2450:2: rule__MacroTokens__ValueAlternatives_0
            {
            pushFollow(FOLLOW_rule__MacroTokens__ValueAlternatives_0_in_rule__MacroTokens__ValueAssignment5043);
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
    public static final BitSet FOLLOW_rule__WMLRoot__ExpressionsAssignment_in_ruleWMLRoot94 = new BitSet(new long[]{0x0000000880088FC2L});
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
    public static final BitSet FOLLOW_RULE_IFVER_in_rule__WMLPreprocIF__NameAlternatives_0_01352 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFNVER_in_rule__WMLPreprocIF__NameAlternatives_0_01369 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRootExpression__Alternatives1401 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLRootExpression__Alternatives1418 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLRootExpression__Alternatives1435 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_rule__WMLRootExpression__Alternatives1452 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPreprocIF_in_rule__WMLRootExpression__Alternatives1469 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_rule__WMLExpression__Alternatives1501 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLExpression__Alternatives1518 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_rule__WMLValuedExpression__Alternatives1550 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLValuedExpression__Alternatives1567 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLMacroParameter__Alternatives1599 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_rule__WMLMacroParameter__Alternatives1616 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLValue__ValueAlternatives_01648 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLValue__ValueAlternatives_01665 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__WMLValue__ValueAlternatives_01683 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__WMLValue__ValueAlternatives_01703 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_rule__WMLValue__ValueAlternatives_01723 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__WMLValue__ValueAlternatives_01743 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_rule__WMLValue__ValueAlternatives_01763 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rule__WMLValue__ValueAlternatives_01783 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_28_in_rule__WMLValue__ValueAlternatives_01803 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_rule__WMLValue__ValueAlternatives_01823 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_rule__WMLValue__ValueAlternatives_01842 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__MacroTokens__ValueAlternatives_01875 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__MacroTokens__ValueAlternatives_01895 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__MacroTokens__ValueAlternatives_01915 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_33_in_rule__MacroTokens__ValueAlternatives_01935 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_34_in_rule__MacroTokens__ValueAlternatives_01955 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01987 = new BitSet(new long[]{0x0000000200001000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01990 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__WMLTag__Group__0__Impl2018 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__12049 = new BitSet(new long[]{0x0000000200001000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__12052 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl2079 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__22110 = new BitSet(new long[]{0x0000000100000000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__22113 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl2140 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__32170 = new BitSet(new long[]{0x0000000C80089FC0L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__32173 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__WMLTag__Group__3__Impl2201 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__42232 = new BitSet(new long[]{0x0000000C80089FC0L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__42235 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__ExpressionsAssignment_4_in_rule__WMLTag__Group__4__Impl2262 = new BitSet(new long[]{0x0000000880089FC2L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__52293 = new BitSet(new long[]{0x0000000000001000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__52296 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_34_in_rule__WMLTag__Group__5__Impl2324 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__62355 = new BitSet(new long[]{0x0000000100000000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__62358 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl2385 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__72415 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__WMLTag__Group__7__Impl2443 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__02490 = new BitSet(new long[]{0x0000000040000000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__02493 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl2520 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__12550 = new BitSet(new long[]{0x0000000ABFD07030L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__12553 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_rule__WMLKey__Group__1__Impl2581 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__22612 = new BitSet(new long[]{0x0000000ABFD07030L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__22615 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValuesAssignment_2_in_rule__WMLKey__Group__2__Impl2642 = new BitSet(new long[]{0x00000008BFD07002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__32673 = new BitSet(new long[]{0x0000000ABFD07030L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__4_in_rule__WMLKey__Group__32676 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__0_in_rule__WMLKey__Group__3__Impl2703 = new BitSet(new long[]{0x0000000200000012L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__4__Impl_in_rule__WMLKey__Group__42734 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__EolAssignment_4_in_rule__WMLKey__Group__4__Impl2761 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__0__Impl_in_rule__WMLKey__Group_3__02801 = new BitSet(new long[]{0x0000000200000010L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__1_in_rule__WMLKey__Group_3__02804 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__0__Impl2832 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__1__Impl_in_rule__WMLKey__Group_3__12863 = new BitSet(new long[]{0x00000008BFD07010L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__2_in_rule__WMLKey__Group_3__12866 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_33_in_rule__WMLKey__Group_3__1__Impl2894 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__2__Impl_in_rule__WMLKey__Group_3__22925 = new BitSet(new long[]{0x00000008BFD07010L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__3_in_rule__WMLKey__Group_3__22928 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_EOL_in_rule__WMLKey__Group_3__2__Impl2956 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__3__Impl_in_rule__WMLKey__Group_3__32987 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValuesAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl3016 = new BitSet(new long[]{0x00000008BFD07012L});
    public static final BitSet FOLLOW_rule__WMLKey__ValuesAssignment_3_3_in_rule__WMLKey__Group_3__3__Impl3028 = new BitSet(new long[]{0x00000008BFD07012L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__03069 = new BitSet(new long[]{0x0000000002801000L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__03072 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_35_in_rule__WMLMacroCall__Group__0__Impl3100 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__13131 = new BitSet(new long[]{0x0000000002801000L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__13134 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__PointAssignment_1_in_rule__WMLMacroCall__Group__1__Impl3161 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__23192 = new BitSet(new long[]{0x0000000002801000L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__23195 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__RelativeAssignment_2_in_rule__WMLMacroCall__Group__2__Impl3222 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__33253 = new BitSet(new long[]{0x0000001FFFC07000L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__33256 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__NameAssignment_3_in_rule__WMLMacroCall__Group__3__Impl3283 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__43313 = new BitSet(new long[]{0x0000001FFFC07000L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__5_in_rule__WMLMacroCall__Group__43316 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ParametersAssignment_4_in_rule__WMLMacroCall__Group__4__Impl3343 = new BitSet(new long[]{0x0000000FFFC07002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__5__Impl_in_rule__WMLMacroCall__Group__53374 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_36_in_rule__WMLMacroCall__Group__5__Impl3402 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__0__Impl_in_rule__WMLArrayCall__Group__03445 = new BitSet(new long[]{0x000000003FC07000L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1_in_rule__WMLArrayCall__Group__03448 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_rule__WMLArrayCall__Group__0__Impl3476 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__1__Impl_in_rule__WMLArrayCall__Group__13507 = new BitSet(new long[]{0x0000000100000000L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2_in_rule__WMLArrayCall__Group__13510 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3539 = new BitSet(new long[]{0x000000003FC07002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__ValueAssignment_1_in_rule__WMLArrayCall__Group__1__Impl3551 = new BitSet(new long[]{0x000000003FC07002L});
    public static final BitSet FOLLOW_rule__WMLArrayCall__Group__2__Impl_in_rule__WMLArrayCall__Group__23584 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_rule__WMLArrayCall__Group__2__Impl3612 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__0__Impl_in_rule__WMLMacroDefine__Group__03649 = new BitSet(new long[]{0x00000008BFC9FFC0L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__1_in_rule__WMLMacroDefine__Group__03652 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__NameAssignment_0_in_rule__WMLMacroDefine__Group__0__Impl3679 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__1__Impl_in_rule__WMLMacroDefine__Group__13709 = new BitSet(new long[]{0x00000008BFC9FFC0L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__2_in_rule__WMLMacroDefine__Group__13712 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__ExpressionsAssignment_1_in_rule__WMLMacroDefine__Group__1__Impl3739 = new BitSet(new long[]{0x00000008BFC8FFC2L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__2__Impl_in_rule__WMLMacroDefine__Group__23770 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__EndNameAssignment_2_in_rule__WMLMacroDefine__Group__2__Impl3797 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__0__Impl_in_rule__WMLPreprocIF__Group__03833 = new BitSet(new long[]{0x00000008BFCEFFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__1_in_rule__WMLPreprocIF__Group__03836 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__NameAssignment_0_in_rule__WMLPreprocIF__Group__0__Impl3863 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__1__Impl_in_rule__WMLPreprocIF__Group__13893 = new BitSet(new long[]{0x00000008BFCEFFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__2_in_rule__WMLPreprocIF__Group__13896 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ExpressionsAssignment_1_in_rule__WMLPreprocIF__Group__1__Impl3923 = new BitSet(new long[]{0x00000008BFC8FFC2L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__2__Impl_in_rule__WMLPreprocIF__Group__23954 = new BitSet(new long[]{0x00000008BFCEFFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__3_in_rule__WMLPreprocIF__Group__23957 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__0_in_rule__WMLPreprocIF__Group__2__Impl3984 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group__3__Impl_in_rule__WMLPreprocIF__Group__34015 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__EndNameAssignment_3_in_rule__WMLPreprocIF__Group__3__Impl4042 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__0__Impl_in_rule__WMLPreprocIF__Group_2__04080 = new BitSet(new long[]{0x00000008BFC8FFC0L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__1_in_rule__WMLPreprocIF__Group_2__04083 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ElsesAssignment_2_0_in_rule__WMLPreprocIF__Group_2__0__Impl4110 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__Group_2__1__Impl_in_rule__WMLPreprocIF__Group_2__14140 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4169 = new BitSet(new long[]{0x00000008BFC8FFC2L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__ElseExpressionsAssignment_2_1_in_rule__WMLPreprocIF__Group_2__1__Impl4181 = new BitSet(new long[]{0x00000008BFC8FFC2L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_rule__WMLRoot__ExpressionsAssignment4223 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_33_in_rule__WMLTag__PlusAssignment_14259 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_24298 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_rule__WMLTag__ExpressionsAssignment_44329 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_64360 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__NameAssignment_04391 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValuesAssignment_24422 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValuesAssignment_3_34453 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__EolAlternatives_4_0_in_rule__WMLKey__EolAssignment_44484 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__WMLMacroCall__PointAssignment_14522 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__WMLMacroCall__RelativeAssignment_24566 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_34605 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_rule__WMLMacroCall__ParametersAssignment_44636 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_rule__WMLArrayCall__ValueAssignment_14667 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__NameAssignment_04698 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_rule__WMLMacroDefine__ExpressionsAssignment_14729 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ENDDEF_in_rule__WMLMacroDefine__EndNameAssignment_24760 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPreprocIF__NameAlternatives_0_0_in_rule__WMLPreprocIF__NameAssignment_04791 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ExpressionsAssignment_14824 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ELSE_in_rule__WMLPreprocIF__ElsesAssignment_2_04855 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_rule__WMLPreprocIF__ElseExpressionsAssignment_2_14886 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ENDIF_in_rule__WMLPreprocIF__EndNameAssignment_34917 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_rule__WMLTextdomain__NameAssignment4948 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_rule__WMLLuaCode__ValueAssignment4979 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLValue__ValueAlternatives_0_in_rule__WMLValue__ValueAssignment5010 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__MacroTokens__ValueAlternatives_0_in_rule__MacroTokens__ValueAssignment5043 = new BitSet(new long[]{0x0000000000000002L});

}