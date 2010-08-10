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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_STRING", "RULE_ANY_OTHER", "RULE_DEFINE", "RULE_ENDDEFINE", "RULE_INT", "RULE_TEXTDOMAIN", "RULE_SL_COMMENT", "RULE_WS", "'-'", "'['", "']'", "'[/'", "'{'", "'}'", "'='", "'+'", "'('", "')'", "'~'", "'/'", "','", "'_'", "'.'"
    };
    public static final int RULE_ID=4;
    public static final int RULE_STRING=5;
    public static final int RULE_ENDDEFINE=8;
    public static final int RULE_DEFINE=7;
    public static final int RULE_ANY_OTHER=6;
    public static final int RULE_INT=9;
    public static final int RULE_TEXTDOMAIN=10;
    public static final int RULE_WS=12;
    public static final int RULE_SL_COMMENT=11;
    public static final int EOF=-1;

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

                if ( (LA1_0==RULE_DEFINE||LA1_0==14||LA1_0==17) ) {
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


    // $ANTLR start entryRuleWMLAbstractMacroCall
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:117:1: entryRuleWMLAbstractMacroCall : ruleWMLAbstractMacroCall EOF ;
    public final void entryRuleWMLAbstractMacroCall() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:118:1: ( ruleWMLAbstractMacroCall EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:119:1: ruleWMLAbstractMacroCall EOF
            {
             before(grammarAccess.getWMLAbstractMacroCallRule()); 
            pushFollow(FOLLOW_ruleWMLAbstractMacroCall_in_entryRuleWMLAbstractMacroCall182);
            ruleWMLAbstractMacroCall();
            _fsp--;

             after(grammarAccess.getWMLAbstractMacroCallRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLAbstractMacroCall189); 

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
    // $ANTLR end entryRuleWMLAbstractMacroCall


    // $ANTLR start ruleWMLAbstractMacroCall
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:126:1: ruleWMLAbstractMacroCall : ( ( rule__WMLAbstractMacroCall__Alternatives ) ) ;
    public final void ruleWMLAbstractMacroCall() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:130:2: ( ( ( rule__WMLAbstractMacroCall__Alternatives ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:131:1: ( ( rule__WMLAbstractMacroCall__Alternatives ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:131:1: ( ( rule__WMLAbstractMacroCall__Alternatives ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:132:1: ( rule__WMLAbstractMacroCall__Alternatives )
            {
             before(grammarAccess.getWMLAbstractMacroCallAccess().getAlternatives()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:133:1: ( rule__WMLAbstractMacroCall__Alternatives )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:133:2: rule__WMLAbstractMacroCall__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLAbstractMacroCall__Alternatives_in_ruleWMLAbstractMacroCall215);
            rule__WMLAbstractMacroCall__Alternatives();
            _fsp--;


            }

             after(grammarAccess.getWMLAbstractMacroCallAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLAbstractMacroCall


    // $ANTLR start entryRuleWMLMacroInclude
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:145:1: entryRuleWMLMacroInclude : ruleWMLMacroInclude EOF ;
    public final void entryRuleWMLMacroInclude() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:146:1: ( ruleWMLMacroInclude EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:147:1: ruleWMLMacroInclude EOF
            {
             before(grammarAccess.getWMLMacroIncludeRule()); 
            pushFollow(FOLLOW_ruleWMLMacroInclude_in_entryRuleWMLMacroInclude242);
            ruleWMLMacroInclude();
            _fsp--;

             after(grammarAccess.getWMLMacroIncludeRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroInclude249); 

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
    // $ANTLR end entryRuleWMLMacroInclude


    // $ANTLR start ruleWMLMacroInclude
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:154:1: ruleWMLMacroInclude : ( ( rule__WMLMacroInclude__Group__0 ) ) ;
    public final void ruleWMLMacroInclude() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:158:2: ( ( ( rule__WMLMacroInclude__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:159:1: ( ( rule__WMLMacroInclude__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:159:1: ( ( rule__WMLMacroInclude__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:160:1: ( rule__WMLMacroInclude__Group__0 )
            {
             before(grammarAccess.getWMLMacroIncludeAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:161:1: ( rule__WMLMacroInclude__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:161:2: rule__WMLMacroInclude__Group__0
            {
            pushFollow(FOLLOW_rule__WMLMacroInclude__Group__0_in_ruleWMLMacroInclude275);
            rule__WMLMacroInclude__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroIncludeAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLMacroInclude


    // $ANTLR start entryRuleWMLMacroCall
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:173:1: entryRuleWMLMacroCall : ruleWMLMacroCall EOF ;
    public final void entryRuleWMLMacroCall() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:174:1: ( ruleWMLMacroCall EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:175:1: ruleWMLMacroCall EOF
            {
             before(grammarAccess.getWMLMacroCallRule()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall302);
            ruleWMLMacroCall();
            _fsp--;

             after(grammarAccess.getWMLMacroCallRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCall309); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:182:1: ruleWMLMacroCall : ( ( rule__WMLMacroCall__Group__0 ) ) ;
    public final void ruleWMLMacroCall() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:186:2: ( ( ( rule__WMLMacroCall__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:187:1: ( ( rule__WMLMacroCall__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:187:1: ( ( rule__WMLMacroCall__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:188:1: ( rule__WMLMacroCall__Group__0 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:189:1: ( rule__WMLMacroCall__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:189:2: rule__WMLMacroCall__Group__0
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__0_in_ruleWMLMacroCall335);
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


    // $ANTLR start entryRuleWMLMacroDefine
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:201:1: entryRuleWMLMacroDefine : ruleWMLMacroDefine EOF ;
    public final void entryRuleWMLMacroDefine() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:202:1: ( ruleWMLMacroDefine EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:203:1: ruleWMLMacroDefine EOF
            {
             before(grammarAccess.getWMLMacroDefineRule()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine362);
            ruleWMLMacroDefine();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroDefine369); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:210:1: ruleWMLMacroDefine : ( ( rule__WMLMacroDefine__Group__0 ) ) ;
    public final void ruleWMLMacroDefine() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:214:2: ( ( ( rule__WMLMacroDefine__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:215:1: ( ( rule__WMLMacroDefine__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:215:1: ( ( rule__WMLMacroDefine__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:216:1: ( rule__WMLMacroDefine__Group__0 )
            {
             before(grammarAccess.getWMLMacroDefineAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:217:1: ( rule__WMLMacroDefine__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:217:2: rule__WMLMacroDefine__Group__0
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__0_in_ruleWMLMacroDefine395);
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


    // $ANTLR start entryRuleWMLKey
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:231:1: entryRuleWMLKey : ruleWMLKey EOF ;
    public final void entryRuleWMLKey() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:232:1: ( ruleWMLKey EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:233:1: ruleWMLKey EOF
            {
             before(grammarAccess.getWMLKeyRule()); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey424);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLKeyRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey431); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:240:1: ruleWMLKey : ( ( rule__WMLKey__Group__0 ) ) ;
    public final void ruleWMLKey() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:244:2: ( ( ( rule__WMLKey__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:245:1: ( ( rule__WMLKey__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:245:1: ( ( rule__WMLKey__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:246:1: ( rule__WMLKey__Group__0 )
            {
             before(grammarAccess.getWMLKeyAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:247:1: ( rule__WMLKey__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:247:2: rule__WMLKey__Group__0
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey457);
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


    // $ANTLR start entryRuleWMLKeyExtraArgs
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:259:1: entryRuleWMLKeyExtraArgs : ruleWMLKeyExtraArgs EOF ;
    public final void entryRuleWMLKeyExtraArgs() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:260:1: ( ruleWMLKeyExtraArgs EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:261:1: ruleWMLKeyExtraArgs EOF
            {
             before(grammarAccess.getWMLKeyExtraArgsRule()); 
            pushFollow(FOLLOW_ruleWMLKeyExtraArgs_in_entryRuleWMLKeyExtraArgs484);
            ruleWMLKeyExtraArgs();
            _fsp--;

             after(grammarAccess.getWMLKeyExtraArgsRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyExtraArgs491); 

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
    // $ANTLR end entryRuleWMLKeyExtraArgs


    // $ANTLR start ruleWMLKeyExtraArgs
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:268:1: ruleWMLKeyExtraArgs : ( ( rule__WMLKeyExtraArgs__Alternatives ) ) ;
    public final void ruleWMLKeyExtraArgs() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:272:2: ( ( ( rule__WMLKeyExtraArgs__Alternatives ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:273:1: ( ( rule__WMLKeyExtraArgs__Alternatives ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:273:1: ( ( rule__WMLKeyExtraArgs__Alternatives ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:274:1: ( rule__WMLKeyExtraArgs__Alternatives )
            {
             before(grammarAccess.getWMLKeyExtraArgsAccess().getAlternatives()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:275:1: ( rule__WMLKeyExtraArgs__Alternatives )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:275:2: rule__WMLKeyExtraArgs__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLKeyExtraArgs__Alternatives_in_ruleWMLKeyExtraArgs517);
            rule__WMLKeyExtraArgs__Alternatives();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyExtraArgsAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLKeyExtraArgs


    // $ANTLR start entryRuleWMLMacroCallParameter
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:287:1: entryRuleWMLMacroCallParameter : ruleWMLMacroCallParameter EOF ;
    public final void entryRuleWMLMacroCallParameter() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:288:1: ( ruleWMLMacroCallParameter EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:289:1: ruleWMLMacroCallParameter EOF
            {
             before(grammarAccess.getWMLMacroCallParameterRule()); 
            pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_entryRuleWMLMacroCallParameter544);
            ruleWMLMacroCallParameter();
            _fsp--;

             after(grammarAccess.getWMLMacroCallParameterRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCallParameter551); 

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
    // $ANTLR end entryRuleWMLMacroCallParameter


    // $ANTLR start ruleWMLMacroCallParameter
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:296:1: ruleWMLMacroCallParameter : ( ( rule__WMLMacroCallParameter__Group__0 ) ) ;
    public final void ruleWMLMacroCallParameter() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:300:2: ( ( ( rule__WMLMacroCallParameter__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:301:1: ( ( rule__WMLMacroCallParameter__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:301:1: ( ( rule__WMLMacroCallParameter__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:302:1: ( rule__WMLMacroCallParameter__Group__0 )
            {
             before(grammarAccess.getWMLMacroCallParameterAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:303:1: ( rule__WMLMacroCallParameter__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:303:2: rule__WMLMacroCallParameter__Group__0
            {
            pushFollow(FOLLOW_rule__WMLMacroCallParameter__Group__0_in_ruleWMLMacroCallParameter577);
            rule__WMLMacroCallParameter__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroCallParameterAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLMacroCallParameter


    // $ANTLR start entryRuleWMLKeyValue
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:315:1: entryRuleWMLKeyValue : ruleWMLKeyValue EOF ;
    public final void entryRuleWMLKeyValue() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:316:1: ( ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:317:1: ruleWMLKeyValue EOF
            {
             before(grammarAccess.getWMLKeyValueRule()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue604);
            ruleWMLKeyValue();
            _fsp--;

             after(grammarAccess.getWMLKeyValueRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue611); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:324:1: ruleWMLKeyValue : ( ( rule__WMLKeyValue__Alternatives ) ) ;
    public final void ruleWMLKeyValue() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:328:2: ( ( ( rule__WMLKeyValue__Alternatives ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:329:1: ( ( rule__WMLKeyValue__Alternatives ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:329:1: ( ( rule__WMLKeyValue__Alternatives ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:330:1: ( rule__WMLKeyValue__Alternatives )
            {
             before(grammarAccess.getWMLKeyValueAccess().getAlternatives()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:331:1: ( rule__WMLKeyValue__Alternatives )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:331:2: rule__WMLKeyValue__Alternatives
            {
            pushFollow(FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue637);
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


    // $ANTLR start entryRuleWMLPath
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:343:1: entryRuleWMLPath : ruleWMLPath EOF ;
    public final void entryRuleWMLPath() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:344:1: ( ruleWMLPath EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:345:1: ruleWMLPath EOF
            {
             before(grammarAccess.getWMLPathRule()); 
            pushFollow(FOLLOW_ruleWMLPath_in_entryRuleWMLPath664);
            ruleWMLPath();
            _fsp--;

             after(grammarAccess.getWMLPathRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLPath671); 

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
    // $ANTLR end entryRuleWMLPath


    // $ANTLR start ruleWMLPath
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:352:1: ruleWMLPath : ( ( rule__WMLPath__Group__0 ) ) ;
    public final void ruleWMLPath() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:356:2: ( ( ( rule__WMLPath__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:357:1: ( ( rule__WMLPath__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:357:1: ( ( rule__WMLPath__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:358:1: ( rule__WMLPath__Group__0 )
            {
             before(grammarAccess.getWMLPathAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:359:1: ( rule__WMLPath__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:359:2: rule__WMLPath__Group__0
            {
            pushFollow(FOLLOW_rule__WMLPath__Group__0_in_ruleWMLPath697);
            rule__WMLPath__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLPathAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLPath


    // $ANTLR start entryRuleWMLIDList
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:371:1: entryRuleWMLIDList : ruleWMLIDList EOF ;
    public final void entryRuleWMLIDList() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:372:1: ( ruleWMLIDList EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:373:1: ruleWMLIDList EOF
            {
             before(grammarAccess.getWMLIDListRule()); 
            pushFollow(FOLLOW_ruleWMLIDList_in_entryRuleWMLIDList724);
            ruleWMLIDList();
            _fsp--;

             after(grammarAccess.getWMLIDListRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLIDList731); 

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
    // $ANTLR end entryRuleWMLIDList


    // $ANTLR start ruleWMLIDList
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:380:1: ruleWMLIDList : ( ( rule__WMLIDList__Group__0 ) ) ;
    public final void ruleWMLIDList() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:384:2: ( ( ( rule__WMLIDList__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:385:1: ( ( rule__WMLIDList__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:385:1: ( ( rule__WMLIDList__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:386:1: ( rule__WMLIDList__Group__0 )
            {
             before(grammarAccess.getWMLIDListAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:387:1: ( rule__WMLIDList__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:387:2: rule__WMLIDList__Group__0
            {
            pushFollow(FOLLOW_rule__WMLIDList__Group__0_in_ruleWMLIDList757);
            rule__WMLIDList__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLIDListAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLIDList


    // $ANTLR start entryRuleWMLINTList
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:399:1: entryRuleWMLINTList : ruleWMLINTList EOF ;
    public final void entryRuleWMLINTList() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:400:1: ( ruleWMLINTList EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:401:1: ruleWMLINTList EOF
            {
             before(grammarAccess.getWMLINTListRule()); 
            pushFollow(FOLLOW_ruleWMLINTList_in_entryRuleWMLINTList784);
            ruleWMLINTList();
            _fsp--;

             after(grammarAccess.getWMLINTListRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLINTList791); 

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
    // $ANTLR end entryRuleWMLINTList


    // $ANTLR start ruleWMLINTList
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:408:1: ruleWMLINTList : ( ( rule__WMLINTList__Group__0 ) ) ;
    public final void ruleWMLINTList() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:412:2: ( ( ( rule__WMLINTList__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:413:1: ( ( rule__WMLINTList__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:413:1: ( ( rule__WMLINTList__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:414:1: ( rule__WMLINTList__Group__0 )
            {
             before(grammarAccess.getWMLINTListAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:415:1: ( rule__WMLINTList__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:415:2: rule__WMLINTList__Group__0
            {
            pushFollow(FOLLOW_rule__WMLINTList__Group__0_in_ruleWMLINTList817);
            rule__WMLINTList__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLINTListAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLINTList


    // $ANTLR start entryRuleTSTRING
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:427:1: entryRuleTSTRING : ruleTSTRING EOF ;
    public final void entryRuleTSTRING() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:428:1: ( ruleTSTRING EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:429:1: ruleTSTRING EOF
            {
             before(grammarAccess.getTSTRINGRule()); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING844);
            ruleTSTRING();
            _fsp--;

             after(grammarAccess.getTSTRINGRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING851); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:436:1: ruleTSTRING : ( ( rule__TSTRING__Group__0 ) ) ;
    public final void ruleTSTRING() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:440:2: ( ( ( rule__TSTRING__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:441:1: ( ( rule__TSTRING__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:441:1: ( ( rule__TSTRING__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:442:1: ( rule__TSTRING__Group__0 )
            {
             before(grammarAccess.getTSTRINGAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:443:1: ( rule__TSTRING__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:443:2: rule__TSTRING__Group__0
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0_in_ruleTSTRING877);
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


    // $ANTLR start entryRuleFILE
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:455:1: entryRuleFILE : ruleFILE EOF ;
    public final void entryRuleFILE() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:456:1: ( ruleFILE EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:457:1: ruleFILE EOF
            {
             before(grammarAccess.getFILERule()); 
            pushFollow(FOLLOW_ruleFILE_in_entryRuleFILE904);
            ruleFILE();
            _fsp--;

             after(grammarAccess.getFILERule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleFILE911); 

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
    // $ANTLR end entryRuleFILE


    // $ANTLR start ruleFILE
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:464:1: ruleFILE : ( ( rule__FILE__Group__0 ) ) ;
    public final void ruleFILE() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:468:2: ( ( ( rule__FILE__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:469:1: ( ( rule__FILE__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:469:1: ( ( rule__FILE__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:470:1: ( rule__FILE__Group__0 )
            {
             before(grammarAccess.getFILEAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:471:1: ( rule__FILE__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:471:2: rule__FILE__Group__0
            {
            pushFollow(FOLLOW_rule__FILE__Group__0_in_ruleFILE937);
            rule__FILE__Group__0();
            _fsp--;


            }

             after(grammarAccess.getFILEAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleFILE


    // $ANTLR start entryRulePATH_ID
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:483:1: entryRulePATH_ID : rulePATH_ID EOF ;
    public final void entryRulePATH_ID() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:484:1: ( rulePATH_ID EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:485:1: rulePATH_ID EOF
            {
             before(grammarAccess.getPATH_IDRule()); 
            pushFollow(FOLLOW_rulePATH_ID_in_entryRulePATH_ID964);
            rulePATH_ID();
            _fsp--;

             after(grammarAccess.getPATH_IDRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePATH_ID971); 

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
    // $ANTLR end entryRulePATH_ID


    // $ANTLR start rulePATH_ID
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:492:1: rulePATH_ID : ( ( rule__PATH_ID__Alternatives ) ) ;
    public final void rulePATH_ID() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:496:2: ( ( ( rule__PATH_ID__Alternatives ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:497:1: ( ( rule__PATH_ID__Alternatives ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:497:1: ( ( rule__PATH_ID__Alternatives ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:498:1: ( rule__PATH_ID__Alternatives )
            {
             before(grammarAccess.getPATH_IDAccess().getAlternatives()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:499:1: ( rule__PATH_ID__Alternatives )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:499:2: rule__PATH_ID__Alternatives
            {
            pushFollow(FOLLOW_rule__PATH_ID__Alternatives_in_rulePATH_ID997);
            rule__PATH_ID__Alternatives();
            _fsp--;


            }

             after(grammarAccess.getPATH_IDAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rulePATH_ID


    // $ANTLR start rule__WMLRoot__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:511:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__TagsAssignment_0 ) ) | ( ( rule__WMLRoot__MacrosAssignment_1 ) ) | ( ( rule__WMLRoot__MacrosDefinesAssignment_2 ) ) );
    public final void rule__WMLRoot__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:515:1: ( ( ( rule__WMLRoot__TagsAssignment_0 ) ) | ( ( rule__WMLRoot__MacrosAssignment_1 ) ) | ( ( rule__WMLRoot__MacrosDefinesAssignment_2 ) ) )
            int alt2=3;
            switch ( input.LA(1) ) {
            case 14:
                {
                alt2=1;
                }
                break;
            case 17:
                {
                alt2=2;
                }
                break;
            case RULE_DEFINE:
                {
                alt2=3;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("511:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__TagsAssignment_0 ) ) | ( ( rule__WMLRoot__MacrosAssignment_1 ) ) | ( ( rule__WMLRoot__MacrosDefinesAssignment_2 ) ) );", 2, 0, input);

                throw nvae;
            }

            switch (alt2) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:516:1: ( ( rule__WMLRoot__TagsAssignment_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:516:1: ( ( rule__WMLRoot__TagsAssignment_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:517:1: ( rule__WMLRoot__TagsAssignment_0 )
                    {
                     before(grammarAccess.getWMLRootAccess().getTagsAssignment_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:518:1: ( rule__WMLRoot__TagsAssignment_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:518:2: rule__WMLRoot__TagsAssignment_0
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__TagsAssignment_0_in_rule__WMLRoot__Alternatives1033);
                    rule__WMLRoot__TagsAssignment_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getTagsAssignment_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:522:6: ( ( rule__WMLRoot__MacrosAssignment_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:522:6: ( ( rule__WMLRoot__MacrosAssignment_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:523:1: ( rule__WMLRoot__MacrosAssignment_1 )
                    {
                     before(grammarAccess.getWMLRootAccess().getMacrosAssignment_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:524:1: ( rule__WMLRoot__MacrosAssignment_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:524:2: rule__WMLRoot__MacrosAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__MacrosAssignment_1_in_rule__WMLRoot__Alternatives1051);
                    rule__WMLRoot__MacrosAssignment_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getMacrosAssignment_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:528:6: ( ( rule__WMLRoot__MacrosDefinesAssignment_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:528:6: ( ( rule__WMLRoot__MacrosDefinesAssignment_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:529:1: ( rule__WMLRoot__MacrosDefinesAssignment_2 )
                    {
                     before(grammarAccess.getWMLRootAccess().getMacrosDefinesAssignment_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:530:1: ( rule__WMLRoot__MacrosDefinesAssignment_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:530:2: rule__WMLRoot__MacrosDefinesAssignment_2
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__MacrosDefinesAssignment_2_in_rule__WMLRoot__Alternatives1069);
                    rule__WMLRoot__MacrosDefinesAssignment_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getMacrosDefinesAssignment_2()); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:539:1: rule__WMLTag__Alternatives_4 : ( ( ( rule__WMLTag__TagsAssignment_4_0 ) ) | ( ( rule__WMLTag__MacrosAssignment_4_1 ) ) | ( ( rule__WMLTag__MacrosDefinesAssignment_4_2 ) ) | ( ( rule__WMLTag__KeysAssignment_4_3 ) ) );
    public final void rule__WMLTag__Alternatives_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:543:1: ( ( ( rule__WMLTag__TagsAssignment_4_0 ) ) | ( ( rule__WMLTag__MacrosAssignment_4_1 ) ) | ( ( rule__WMLTag__MacrosDefinesAssignment_4_2 ) ) | ( ( rule__WMLTag__KeysAssignment_4_3 ) ) )
            int alt3=4;
            switch ( input.LA(1) ) {
            case 14:
                {
                alt3=1;
                }
                break;
            case 17:
                {
                alt3=2;
                }
                break;
            case RULE_DEFINE:
                {
                alt3=3;
                }
                break;
            case RULE_ID:
                {
                alt3=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("539:1: rule__WMLTag__Alternatives_4 : ( ( ( rule__WMLTag__TagsAssignment_4_0 ) ) | ( ( rule__WMLTag__MacrosAssignment_4_1 ) ) | ( ( rule__WMLTag__MacrosDefinesAssignment_4_2 ) ) | ( ( rule__WMLTag__KeysAssignment_4_3 ) ) );", 3, 0, input);

                throw nvae;
            }

            switch (alt3) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:544:1: ( ( rule__WMLTag__TagsAssignment_4_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:544:1: ( ( rule__WMLTag__TagsAssignment_4_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:545:1: ( rule__WMLTag__TagsAssignment_4_0 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTagsAssignment_4_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:546:1: ( rule__WMLTag__TagsAssignment_4_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:546:2: rule__WMLTag__TagsAssignment_4_0
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TagsAssignment_4_0_in_rule__WMLTag__Alternatives_41102);
                    rule__WMLTag__TagsAssignment_4_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTagsAssignment_4_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:550:6: ( ( rule__WMLTag__MacrosAssignment_4_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:550:6: ( ( rule__WMLTag__MacrosAssignment_4_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:551:1: ( rule__WMLTag__MacrosAssignment_4_1 )
                    {
                     before(grammarAccess.getWMLTagAccess().getMacrosAssignment_4_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:552:1: ( rule__WMLTag__MacrosAssignment_4_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:552:2: rule__WMLTag__MacrosAssignment_4_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__MacrosAssignment_4_1_in_rule__WMLTag__Alternatives_41120);
                    rule__WMLTag__MacrosAssignment_4_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getMacrosAssignment_4_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:556:6: ( ( rule__WMLTag__MacrosDefinesAssignment_4_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:556:6: ( ( rule__WMLTag__MacrosDefinesAssignment_4_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:557:1: ( rule__WMLTag__MacrosDefinesAssignment_4_2 )
                    {
                     before(grammarAccess.getWMLTagAccess().getMacrosDefinesAssignment_4_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:558:1: ( rule__WMLTag__MacrosDefinesAssignment_4_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:558:2: rule__WMLTag__MacrosDefinesAssignment_4_2
                    {
                    pushFollow(FOLLOW_rule__WMLTag__MacrosDefinesAssignment_4_2_in_rule__WMLTag__Alternatives_41138);
                    rule__WMLTag__MacrosDefinesAssignment_4_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getMacrosDefinesAssignment_4_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:562:6: ( ( rule__WMLTag__KeysAssignment_4_3 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:562:6: ( ( rule__WMLTag__KeysAssignment_4_3 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:563:1: ( rule__WMLTag__KeysAssignment_4_3 )
                    {
                     before(grammarAccess.getWMLTagAccess().getKeysAssignment_4_3()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:564:1: ( rule__WMLTag__KeysAssignment_4_3 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:564:2: rule__WMLTag__KeysAssignment_4_3
                    {
                    pushFollow(FOLLOW_rule__WMLTag__KeysAssignment_4_3_in_rule__WMLTag__Alternatives_41156);
                    rule__WMLTag__KeysAssignment_4_3();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getKeysAssignment_4_3()); 

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


    // $ANTLR start rule__WMLAbstractMacroCall__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:573:1: rule__WMLAbstractMacroCall__Alternatives : ( ( ruleWMLMacroInclude ) | ( ruleWMLMacroCall ) );
    public final void rule__WMLAbstractMacroCall__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:577:1: ( ( ruleWMLMacroInclude ) | ( ruleWMLMacroCall ) )
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( (LA4_0==17) ) {
                int LA4_1 = input.LA(2);

                if ( (LA4_1==RULE_ID) ) {
                    int LA4_2 = input.LA(3);

                    if ( (LA4_2==24) ) {
                        alt4=1;
                    }
                    else if ( ((LA4_2>=RULE_ID && LA4_2<=RULE_DEFINE)||LA4_2==14||(LA4_2>=17 && LA4_2<=18)||LA4_2==21||LA4_2==26) ) {
                        alt4=2;
                    }
                    else {
                        NoViableAltException nvae =
                            new NoViableAltException("573:1: rule__WMLAbstractMacroCall__Alternatives : ( ( ruleWMLMacroInclude ) | ( ruleWMLMacroCall ) );", 4, 2, input);

                        throw nvae;
                    }
                }
                else if ( (LA4_1==13||LA4_1==23) ) {
                    alt4=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("573:1: rule__WMLAbstractMacroCall__Alternatives : ( ( ruleWMLMacroInclude ) | ( ruleWMLMacroCall ) );", 4, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("573:1: rule__WMLAbstractMacroCall__Alternatives : ( ( ruleWMLMacroInclude ) | ( ruleWMLMacroCall ) );", 4, 0, input);

                throw nvae;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:578:1: ( ruleWMLMacroInclude )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:578:1: ( ruleWMLMacroInclude )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:579:1: ruleWMLMacroInclude
                    {
                     before(grammarAccess.getWMLAbstractMacroCallAccess().getWMLMacroIncludeParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLMacroInclude_in_rule__WMLAbstractMacroCall__Alternatives1189);
                    ruleWMLMacroInclude();
                    _fsp--;

                     after(grammarAccess.getWMLAbstractMacroCallAccess().getWMLMacroIncludeParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:584:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:584:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:585:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLAbstractMacroCallAccess().getWMLMacroCallParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLAbstractMacroCall__Alternatives1206);
                    ruleWMLMacroCall();
                    _fsp--;

                     after(grammarAccess.getWMLAbstractMacroCallAccess().getWMLMacroCallParserRuleCall_1()); 

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
    // $ANTLR end rule__WMLAbstractMacroCall__Alternatives


    // $ANTLR start rule__WMLMacroCall__ArgsAlternatives_2_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:595:1: rule__WMLMacroCall__ArgsAlternatives_2_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( RULE_ANY_OTHER ) );
    public final void rule__WMLMacroCall__ArgsAlternatives_2_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:599:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( RULE_ANY_OTHER ) )
            int alt5=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt5=1;
                }
                break;
            case RULE_STRING:
                {
                alt5=2;
                }
                break;
            case 26:
                {
                alt5=3;
                }
                break;
            case RULE_ANY_OTHER:
                {
                alt5=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("595:1: rule__WMLMacroCall__ArgsAlternatives_2_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( RULE_ANY_OTHER ) );", 5, 0, input);

                throw nvae;
            }

            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:600:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:600:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:601:1: RULE_ID
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getArgsIDTerminalRuleCall_2_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacroCall__ArgsAlternatives_2_01238); 
                     after(grammarAccess.getWMLMacroCallAccess().getArgsIDTerminalRuleCall_2_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:606:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:606:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:607:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getArgsSTRINGTerminalRuleCall_2_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLMacroCall__ArgsAlternatives_2_01255); 
                     after(grammarAccess.getWMLMacroCallAccess().getArgsSTRINGTerminalRuleCall_2_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:612:6: ( ruleTSTRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:612:6: ( ruleTSTRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:613:1: ruleTSTRING
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getArgsTSTRINGParserRuleCall_2_0_2()); 
                    pushFollow(FOLLOW_ruleTSTRING_in_rule__WMLMacroCall__ArgsAlternatives_2_01272);
                    ruleTSTRING();
                    _fsp--;

                     after(grammarAccess.getWMLMacroCallAccess().getArgsTSTRINGParserRuleCall_2_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:618:6: ( RULE_ANY_OTHER )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:618:6: ( RULE_ANY_OTHER )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:619:1: RULE_ANY_OTHER
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getArgsANY_OTHERTerminalRuleCall_2_0_3()); 
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_rule__WMLMacroCall__ArgsAlternatives_2_01289); 
                     after(grammarAccess.getWMLMacroCallAccess().getArgsANY_OTHERTerminalRuleCall_2_0_3()); 

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
    // $ANTLR end rule__WMLMacroCall__ArgsAlternatives_2_0


    // $ANTLR start rule__WMLMacroCall__Alternatives_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:629:1: rule__WMLMacroCall__Alternatives_3 : ( ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) ) | ( ( rule__WMLMacroCall__TagsAssignment_3_1 ) ) | ( ( rule__WMLMacroCall__MacrosAssignment_3_2 ) ) | ( ( rule__WMLMacroCall__MacrosDefinesAssignment_3_3 ) ) | ( ( rule__WMLMacroCall__KeysAssignment_3_4 ) ) );
    public final void rule__WMLMacroCall__Alternatives_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:633:1: ( ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) ) | ( ( rule__WMLMacroCall__TagsAssignment_3_1 ) ) | ( ( rule__WMLMacroCall__MacrosAssignment_3_2 ) ) | ( ( rule__WMLMacroCall__MacrosDefinesAssignment_3_3 ) ) | ( ( rule__WMLMacroCall__KeysAssignment_3_4 ) ) )
            int alt6=5;
            switch ( input.LA(1) ) {
            case 21:
                {
                alt6=1;
                }
                break;
            case 14:
                {
                alt6=2;
                }
                break;
            case 17:
                {
                alt6=3;
                }
                break;
            case RULE_DEFINE:
                {
                alt6=4;
                }
                break;
            case RULE_ID:
                {
                alt6=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("629:1: rule__WMLMacroCall__Alternatives_3 : ( ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) ) | ( ( rule__WMLMacroCall__TagsAssignment_3_1 ) ) | ( ( rule__WMLMacroCall__MacrosAssignment_3_2 ) ) | ( ( rule__WMLMacroCall__MacrosDefinesAssignment_3_3 ) ) | ( ( rule__WMLMacroCall__KeysAssignment_3_4 ) ) );", 6, 0, input);

                throw nvae;
            }

            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:634:1: ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:634:1: ( ( rule__WMLMacroCall__ParamsAssignment_3_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:635:1: ( rule__WMLMacroCall__ParamsAssignment_3_0 )
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getParamsAssignment_3_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:636:1: ( rule__WMLMacroCall__ParamsAssignment_3_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:636:2: rule__WMLMacroCall__ParamsAssignment_3_0
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__ParamsAssignment_3_0_in_rule__WMLMacroCall__Alternatives_31321);
                    rule__WMLMacroCall__ParamsAssignment_3_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroCallAccess().getParamsAssignment_3_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:640:6: ( ( rule__WMLMacroCall__TagsAssignment_3_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:640:6: ( ( rule__WMLMacroCall__TagsAssignment_3_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:641:1: ( rule__WMLMacroCall__TagsAssignment_3_1 )
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getTagsAssignment_3_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:642:1: ( rule__WMLMacroCall__TagsAssignment_3_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:642:2: rule__WMLMacroCall__TagsAssignment_3_1
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__TagsAssignment_3_1_in_rule__WMLMacroCall__Alternatives_31339);
                    rule__WMLMacroCall__TagsAssignment_3_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroCallAccess().getTagsAssignment_3_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:646:6: ( ( rule__WMLMacroCall__MacrosAssignment_3_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:646:6: ( ( rule__WMLMacroCall__MacrosAssignment_3_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:647:1: ( rule__WMLMacroCall__MacrosAssignment_3_2 )
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getMacrosAssignment_3_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:648:1: ( rule__WMLMacroCall__MacrosAssignment_3_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:648:2: rule__WMLMacroCall__MacrosAssignment_3_2
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__MacrosAssignment_3_2_in_rule__WMLMacroCall__Alternatives_31357);
                    rule__WMLMacroCall__MacrosAssignment_3_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroCallAccess().getMacrosAssignment_3_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:652:6: ( ( rule__WMLMacroCall__MacrosDefinesAssignment_3_3 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:652:6: ( ( rule__WMLMacroCall__MacrosDefinesAssignment_3_3 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:653:1: ( rule__WMLMacroCall__MacrosDefinesAssignment_3_3 )
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getMacrosDefinesAssignment_3_3()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:654:1: ( rule__WMLMacroCall__MacrosDefinesAssignment_3_3 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:654:2: rule__WMLMacroCall__MacrosDefinesAssignment_3_3
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__MacrosDefinesAssignment_3_3_in_rule__WMLMacroCall__Alternatives_31375);
                    rule__WMLMacroCall__MacrosDefinesAssignment_3_3();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroCallAccess().getMacrosDefinesAssignment_3_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:658:6: ( ( rule__WMLMacroCall__KeysAssignment_3_4 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:658:6: ( ( rule__WMLMacroCall__KeysAssignment_3_4 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:659:1: ( rule__WMLMacroCall__KeysAssignment_3_4 )
                    {
                     before(grammarAccess.getWMLMacroCallAccess().getKeysAssignment_3_4()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:660:1: ( rule__WMLMacroCall__KeysAssignment_3_4 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:660:2: rule__WMLMacroCall__KeysAssignment_3_4
                    {
                    pushFollow(FOLLOW_rule__WMLMacroCall__KeysAssignment_3_4_in_rule__WMLMacroCall__Alternatives_31393);
                    rule__WMLMacroCall__KeysAssignment_3_4();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroCallAccess().getKeysAssignment_3_4()); 

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


    // $ANTLR start rule__WMLMacroDefine__Alternatives_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:669:1: rule__WMLMacroDefine__Alternatives_1 : ( ( ( rule__WMLMacroDefine__ParamsAssignment_1_0 ) ) | ( ( rule__WMLMacroDefine__TagsAssignment_1_1 ) ) | ( ( rule__WMLMacroDefine__MacrosAssignment_1_2 ) ) | ( ( rule__WMLMacroDefine__MacrosDefinesAssignment_1_3 ) ) | ( ( rule__WMLMacroDefine__KeysAssignment_1_4 ) ) );
    public final void rule__WMLMacroDefine__Alternatives_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:673:1: ( ( ( rule__WMLMacroDefine__ParamsAssignment_1_0 ) ) | ( ( rule__WMLMacroDefine__TagsAssignment_1_1 ) ) | ( ( rule__WMLMacroDefine__MacrosAssignment_1_2 ) ) | ( ( rule__WMLMacroDefine__MacrosDefinesAssignment_1_3 ) ) | ( ( rule__WMLMacroDefine__KeysAssignment_1_4 ) ) )
            int alt7=5;
            switch ( input.LA(1) ) {
            case 21:
                {
                alt7=1;
                }
                break;
            case 14:
                {
                alt7=2;
                }
                break;
            case 17:
                {
                alt7=3;
                }
                break;
            case RULE_DEFINE:
                {
                alt7=4;
                }
                break;
            case RULE_ID:
                {
                alt7=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("669:1: rule__WMLMacroDefine__Alternatives_1 : ( ( ( rule__WMLMacroDefine__ParamsAssignment_1_0 ) ) | ( ( rule__WMLMacroDefine__TagsAssignment_1_1 ) ) | ( ( rule__WMLMacroDefine__MacrosAssignment_1_2 ) ) | ( ( rule__WMLMacroDefine__MacrosDefinesAssignment_1_3 ) ) | ( ( rule__WMLMacroDefine__KeysAssignment_1_4 ) ) );", 7, 0, input);

                throw nvae;
            }

            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:674:1: ( ( rule__WMLMacroDefine__ParamsAssignment_1_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:674:1: ( ( rule__WMLMacroDefine__ParamsAssignment_1_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:675:1: ( rule__WMLMacroDefine__ParamsAssignment_1_0 )
                    {
                     before(grammarAccess.getWMLMacroDefineAccess().getParamsAssignment_1_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:676:1: ( rule__WMLMacroDefine__ParamsAssignment_1_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:676:2: rule__WMLMacroDefine__ParamsAssignment_1_0
                    {
                    pushFollow(FOLLOW_rule__WMLMacroDefine__ParamsAssignment_1_0_in_rule__WMLMacroDefine__Alternatives_11426);
                    rule__WMLMacroDefine__ParamsAssignment_1_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroDefineAccess().getParamsAssignment_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:680:6: ( ( rule__WMLMacroDefine__TagsAssignment_1_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:680:6: ( ( rule__WMLMacroDefine__TagsAssignment_1_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:681:1: ( rule__WMLMacroDefine__TagsAssignment_1_1 )
                    {
                     before(grammarAccess.getWMLMacroDefineAccess().getTagsAssignment_1_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:682:1: ( rule__WMLMacroDefine__TagsAssignment_1_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:682:2: rule__WMLMacroDefine__TagsAssignment_1_1
                    {
                    pushFollow(FOLLOW_rule__WMLMacroDefine__TagsAssignment_1_1_in_rule__WMLMacroDefine__Alternatives_11444);
                    rule__WMLMacroDefine__TagsAssignment_1_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroDefineAccess().getTagsAssignment_1_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:686:6: ( ( rule__WMLMacroDefine__MacrosAssignment_1_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:686:6: ( ( rule__WMLMacroDefine__MacrosAssignment_1_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:687:1: ( rule__WMLMacroDefine__MacrosAssignment_1_2 )
                    {
                     before(grammarAccess.getWMLMacroDefineAccess().getMacrosAssignment_1_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:688:1: ( rule__WMLMacroDefine__MacrosAssignment_1_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:688:2: rule__WMLMacroDefine__MacrosAssignment_1_2
                    {
                    pushFollow(FOLLOW_rule__WMLMacroDefine__MacrosAssignment_1_2_in_rule__WMLMacroDefine__Alternatives_11462);
                    rule__WMLMacroDefine__MacrosAssignment_1_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroDefineAccess().getMacrosAssignment_1_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:692:6: ( ( rule__WMLMacroDefine__MacrosDefinesAssignment_1_3 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:692:6: ( ( rule__WMLMacroDefine__MacrosDefinesAssignment_1_3 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:693:1: ( rule__WMLMacroDefine__MacrosDefinesAssignment_1_3 )
                    {
                     before(grammarAccess.getWMLMacroDefineAccess().getMacrosDefinesAssignment_1_3()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:694:1: ( rule__WMLMacroDefine__MacrosDefinesAssignment_1_3 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:694:2: rule__WMLMacroDefine__MacrosDefinesAssignment_1_3
                    {
                    pushFollow(FOLLOW_rule__WMLMacroDefine__MacrosDefinesAssignment_1_3_in_rule__WMLMacroDefine__Alternatives_11480);
                    rule__WMLMacroDefine__MacrosDefinesAssignment_1_3();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroDefineAccess().getMacrosDefinesAssignment_1_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:698:6: ( ( rule__WMLMacroDefine__KeysAssignment_1_4 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:698:6: ( ( rule__WMLMacroDefine__KeysAssignment_1_4 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:699:1: ( rule__WMLMacroDefine__KeysAssignment_1_4 )
                    {
                     before(grammarAccess.getWMLMacroDefineAccess().getKeysAssignment_1_4()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:700:1: ( rule__WMLMacroDefine__KeysAssignment_1_4 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:700:2: rule__WMLMacroDefine__KeysAssignment_1_4
                    {
                    pushFollow(FOLLOW_rule__WMLMacroDefine__KeysAssignment_1_4_in_rule__WMLMacroDefine__Alternatives_11498);
                    rule__WMLMacroDefine__KeysAssignment_1_4();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLMacroDefineAccess().getKeysAssignment_1_4()); 

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


    // $ANTLR start rule__WMLKeyExtraArgs__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:709:1: rule__WMLKeyExtraArgs__Alternatives : ( ( ruleWMLMacroCall ) | ( RULE_STRING ) | ( ruleTSTRING ) );
    public final void rule__WMLKeyExtraArgs__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:713:1: ( ( ruleWMLMacroCall ) | ( RULE_STRING ) | ( ruleTSTRING ) )
            int alt8=3;
            switch ( input.LA(1) ) {
            case 17:
                {
                alt8=1;
                }
                break;
            case RULE_STRING:
                {
                alt8=2;
                }
                break;
            case 26:
                {
                alt8=3;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("709:1: rule__WMLKeyExtraArgs__Alternatives : ( ( ruleWMLMacroCall ) | ( RULE_STRING ) | ( ruleTSTRING ) );", 8, 0, input);

                throw nvae;
            }

            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:714:1: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:714:1: ( ruleWMLMacroCall )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:715:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLKeyExtraArgsAccess().getWMLMacroCallParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyExtraArgs__Alternatives1531);
                    ruleWMLMacroCall();
                    _fsp--;

                     after(grammarAccess.getWMLKeyExtraArgsAccess().getWMLMacroCallParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:720:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:720:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:721:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLKeyExtraArgsAccess().getSTRINGTerminalRuleCall_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLKeyExtraArgs__Alternatives1548); 
                     after(grammarAccess.getWMLKeyExtraArgsAccess().getSTRINGTerminalRuleCall_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:726:6: ( ruleTSTRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:726:6: ( ruleTSTRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:727:1: ruleTSTRING
                    {
                     before(grammarAccess.getWMLKeyExtraArgsAccess().getTSTRINGParserRuleCall_2()); 
                    pushFollow(FOLLOW_ruleTSTRING_in_rule__WMLKeyExtraArgs__Alternatives1565);
                    ruleTSTRING();
                    _fsp--;

                     after(grammarAccess.getWMLKeyExtraArgsAccess().getTSTRINGParserRuleCall_2()); 

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
    // $ANTLR end rule__WMLKeyExtraArgs__Alternatives


    // $ANTLR start rule__WMLMacroCallParameter__Alternatives_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:737:1: rule__WMLMacroCallParameter__Alternatives_1 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFILE ) );
    public final void rule__WMLMacroCallParameter__Alternatives_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:741:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFILE ) )
            int alt9=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                int LA9_1 = input.LA(2);

                if ( (LA9_1==22) ) {
                    alt9=1;
                }
                else if ( (LA9_1==RULE_ID||LA9_1==13||LA9_1==27) ) {
                    alt9=4;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("737:1: rule__WMLMacroCallParameter__Alternatives_1 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFILE ) );", 9, 1, input);

                    throw nvae;
                }
                }
                break;
            case RULE_STRING:
                {
                alt9=2;
                }
                break;
            case 26:
                {
                alt9=3;
                }
                break;
            case 13:
                {
                alt9=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("737:1: rule__WMLMacroCallParameter__Alternatives_1 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleFILE ) );", 9, 0, input);

                throw nvae;
            }

            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:742:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:742:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:743:1: RULE_ID
                    {
                     before(grammarAccess.getWMLMacroCallParameterAccess().getIDTerminalRuleCall_1_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacroCallParameter__Alternatives_11597); 
                     after(grammarAccess.getWMLMacroCallParameterAccess().getIDTerminalRuleCall_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:748:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:748:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:749:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLMacroCallParameterAccess().getSTRINGTerminalRuleCall_1_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLMacroCallParameter__Alternatives_11614); 
                     after(grammarAccess.getWMLMacroCallParameterAccess().getSTRINGTerminalRuleCall_1_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:754:6: ( ruleTSTRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:754:6: ( ruleTSTRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:755:1: ruleTSTRING
                    {
                     before(grammarAccess.getWMLMacroCallParameterAccess().getTSTRINGParserRuleCall_1_2()); 
                    pushFollow(FOLLOW_ruleTSTRING_in_rule__WMLMacroCallParameter__Alternatives_11631);
                    ruleTSTRING();
                    _fsp--;

                     after(grammarAccess.getWMLMacroCallParameterAccess().getTSTRINGParserRuleCall_1_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:760:6: ( ruleFILE )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:760:6: ( ruleFILE )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:761:1: ruleFILE
                    {
                     before(grammarAccess.getWMLMacroCallParameterAccess().getFILEParserRuleCall_1_3()); 
                    pushFollow(FOLLOW_ruleFILE_in_rule__WMLMacroCallParameter__Alternatives_11648);
                    ruleFILE();
                    _fsp--;

                     after(grammarAccess.getWMLMacroCallParameterAccess().getFILEParserRuleCall_1_3()); 

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
    // $ANTLR end rule__WMLMacroCallParameter__Alternatives_1


    // $ANTLR start rule__WMLKeyValue__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:771:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLINTList ) | ( ruleWMLIDList ) | ( ruleWMLMacroCall ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleWMLPath ) | ( ruleFILE ) );
    public final void rule__WMLKeyValue__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:775:1: ( ( ruleWMLINTList ) | ( ruleWMLIDList ) | ( ruleWMLMacroCall ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleWMLPath ) | ( ruleFILE ) )
            int alt10=7;
            switch ( input.LA(1) ) {
            case RULE_INT:
                {
                alt10=1;
                }
                break;
            case RULE_ID:
                {
                switch ( input.LA(2) ) {
                case 24:
                    {
                    alt10=6;
                    }
                    break;
                case EOF:
                case RULE_DEFINE:
                case RULE_ENDDEFINE:
                case 14:
                case 16:
                case 17:
                case 18:
                case 20:
                case 21:
                case 25:
                    {
                    alt10=2;
                    }
                    break;
                case RULE_ID:
                    {
                    int LA10_9 = input.LA(3);

                    if ( (LA10_9==19||LA10_9==25) ) {
                        alt10=2;
                    }
                    else if ( (LA10_9==RULE_ID||LA10_9==13||LA10_9==27) ) {
                        alt10=7;
                    }
                    else {
                        NoViableAltException nvae =
                            new NoViableAltException("771:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLINTList ) | ( ruleWMLIDList ) | ( ruleWMLMacroCall ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleWMLPath ) | ( ruleFILE ) );", 10, 9, input);

                        throw nvae;
                    }
                    }
                    break;
                case 13:
                case 27:
                    {
                    alt10=7;
                    }
                    break;
                default:
                    NoViableAltException nvae =
                        new NoViableAltException("771:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLINTList ) | ( ruleWMLIDList ) | ( ruleWMLMacroCall ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleWMLPath ) | ( ruleFILE ) );", 10, 2, input);

                    throw nvae;
                }

                }
                break;
            case 17:
                {
                alt10=3;
                }
                break;
            case RULE_STRING:
                {
                alt10=4;
                }
                break;
            case 26:
                {
                alt10=5;
                }
                break;
            case 23:
                {
                alt10=6;
                }
                break;
            case 13:
                {
                int LA10_7 = input.LA(2);

                if ( (LA10_7==24) ) {
                    alt10=6;
                }
                else if ( (LA10_7==RULE_ID||LA10_7==13||LA10_7==27) ) {
                    alt10=7;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("771:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLINTList ) | ( ruleWMLIDList ) | ( ruleWMLMacroCall ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleWMLPath ) | ( ruleFILE ) );", 10, 7, input);

                    throw nvae;
                }
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("771:1: rule__WMLKeyValue__Alternatives : ( ( ruleWMLINTList ) | ( ruleWMLIDList ) | ( ruleWMLMacroCall ) | ( RULE_STRING ) | ( ruleTSTRING ) | ( ruleWMLPath ) | ( ruleFILE ) );", 10, 0, input);

                throw nvae;
            }

            switch (alt10) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:776:1: ( ruleWMLINTList )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:776:1: ( ruleWMLINTList )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:777:1: ruleWMLINTList
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLINTListParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleWMLINTList_in_rule__WMLKeyValue__Alternatives1680);
                    ruleWMLINTList();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLINTListParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:782:6: ( ruleWMLIDList )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:782:6: ( ruleWMLIDList )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:783:1: ruleWMLIDList
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLIDListParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleWMLIDList_in_rule__WMLKeyValue__Alternatives1697);
                    ruleWMLIDList();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLIDListParserRuleCall_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:788:6: ( ruleWMLMacroCall )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:788:6: ( ruleWMLMacroCall )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:789:1: ruleWMLMacroCall
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_2()); 
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives1714);
                    ruleWMLMacroCall();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:794:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:794:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:795:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getSTRINGTerminalRuleCall_3()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLKeyValue__Alternatives1731); 
                     after(grammarAccess.getWMLKeyValueAccess().getSTRINGTerminalRuleCall_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:800:6: ( ruleTSTRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:800:6: ( ruleTSTRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:801:1: ruleTSTRING
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getTSTRINGParserRuleCall_4()); 
                    pushFollow(FOLLOW_ruleTSTRING_in_rule__WMLKeyValue__Alternatives1748);
                    ruleTSTRING();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getTSTRINGParserRuleCall_4()); 

                    }


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:806:6: ( ruleWMLPath )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:806:6: ( ruleWMLPath )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:807:1: ruleWMLPath
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getWMLPathParserRuleCall_5()); 
                    pushFollow(FOLLOW_ruleWMLPath_in_rule__WMLKeyValue__Alternatives1765);
                    ruleWMLPath();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getWMLPathParserRuleCall_5()); 

                    }


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:812:6: ( ruleFILE )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:812:6: ( ruleFILE )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:813:1: ruleFILE
                    {
                     before(grammarAccess.getWMLKeyValueAccess().getFILEParserRuleCall_6()); 
                    pushFollow(FOLLOW_ruleFILE_in_rule__WMLKeyValue__Alternatives1782);
                    ruleFILE();
                    _fsp--;

                     after(grammarAccess.getWMLKeyValueAccess().getFILEParserRuleCall_6()); 

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


    // $ANTLR start rule__PATH_ID__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:823:1: rule__PATH_ID__Alternatives : ( ( RULE_ID ) | ( '-' ) );
    public final void rule__PATH_ID__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:827:1: ( ( RULE_ID ) | ( '-' ) )
            int alt11=2;
            int LA11_0 = input.LA(1);

            if ( (LA11_0==RULE_ID) ) {
                alt11=1;
            }
            else if ( (LA11_0==13) ) {
                alt11=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("823:1: rule__PATH_ID__Alternatives : ( ( RULE_ID ) | ( '-' ) );", 11, 0, input);

                throw nvae;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:828:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:828:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:829:1: RULE_ID
                    {
                     before(grammarAccess.getPATH_IDAccess().getIDTerminalRuleCall_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__PATH_ID__Alternatives1814); 
                     after(grammarAccess.getPATH_IDAccess().getIDTerminalRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:834:6: ( '-' )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:834:6: ( '-' )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:835:1: '-'
                    {
                     before(grammarAccess.getPATH_IDAccess().getHyphenMinusKeyword_1()); 
                    match(input,13,FOLLOW_13_in_rule__PATH_ID__Alternatives1832); 
                     after(grammarAccess.getPATH_IDAccess().getHyphenMinusKeyword_1()); 

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
    // $ANTLR end rule__PATH_ID__Alternatives


    // $ANTLR start rule__WMLTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:849:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:853:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:854:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01864);
            rule__WMLTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01867);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:861:1: rule__WMLTag__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:865:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:866:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:866:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:867:1: '['
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0()); 
            match(input,14,FOLLOW_14_in_rule__WMLTag__Group__0__Impl1895); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:880:1: rule__WMLTag__Group__1 : rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 ;
    public final void rule__WMLTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:884:1: ( rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:885:2: rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__11926);
            rule__WMLTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__11929);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:892:1: rule__WMLTag__Group__1__Impl : ( ( rule__WMLTag__PlusAssignment_1 )? ) ;
    public final void rule__WMLTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:896:1: ( ( ( rule__WMLTag__PlusAssignment_1 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:897:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:897:1: ( ( rule__WMLTag__PlusAssignment_1 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:898:1: ( rule__WMLTag__PlusAssignment_1 )?
            {
             before(grammarAccess.getWMLTagAccess().getPlusAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:899:1: ( rule__WMLTag__PlusAssignment_1 )?
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0==20) ) {
                alt12=1;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:899:2: rule__WMLTag__PlusAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl1956);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:909:1: rule__WMLTag__Group__2 : rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 ;
    public final void rule__WMLTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:913:1: ( rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:914:2: rule__WMLTag__Group__2__Impl rule__WMLTag__Group__3
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__21987);
            rule__WMLTag__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__21990);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:921:1: rule__WMLTag__Group__2__Impl : ( ( rule__WMLTag__NameAssignment_2 ) ) ;
    public final void rule__WMLTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:925:1: ( ( ( rule__WMLTag__NameAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:926:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:926:1: ( ( rule__WMLTag__NameAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:927:1: ( rule__WMLTag__NameAssignment_2 )
            {
             before(grammarAccess.getWMLTagAccess().getNameAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:928:1: ( rule__WMLTag__NameAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:928:2: rule__WMLTag__NameAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl2017);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:938:1: rule__WMLTag__Group__3 : rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 ;
    public final void rule__WMLTag__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:942:1: ( rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:943:2: rule__WMLTag__Group__3__Impl rule__WMLTag__Group__4
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__32047);
            rule__WMLTag__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__32050);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:950:1: rule__WMLTag__Group__3__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:954:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:955:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:955:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:956:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3()); 
            match(input,15,FOLLOW_15_in_rule__WMLTag__Group__3__Impl2078); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:969:1: rule__WMLTag__Group__4 : rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 ;
    public final void rule__WMLTag__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:973:1: ( rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:974:2: rule__WMLTag__Group__4__Impl rule__WMLTag__Group__5
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__42109);
            rule__WMLTag__Group__4__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__42112);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:981:1: rule__WMLTag__Group__4__Impl : ( ( rule__WMLTag__Alternatives_4 )* ) ;
    public final void rule__WMLTag__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:985:1: ( ( ( rule__WMLTag__Alternatives_4 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:986:1: ( ( rule__WMLTag__Alternatives_4 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:986:1: ( ( rule__WMLTag__Alternatives_4 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:987:1: ( rule__WMLTag__Alternatives_4 )*
            {
             before(grammarAccess.getWMLTagAccess().getAlternatives_4()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:988:1: ( rule__WMLTag__Alternatives_4 )*
            loop13:
            do {
                int alt13=2;
                int LA13_0 = input.LA(1);

                if ( (LA13_0==RULE_ID||LA13_0==RULE_DEFINE||LA13_0==14||LA13_0==17) ) {
                    alt13=1;
                }


                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:988:2: rule__WMLTag__Alternatives_4
            	    {
            	    pushFollow(FOLLOW_rule__WMLTag__Alternatives_4_in_rule__WMLTag__Group__4__Impl2139);
            	    rule__WMLTag__Alternatives_4();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop13;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:998:1: rule__WMLTag__Group__5 : rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 ;
    public final void rule__WMLTag__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1002:1: ( rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1003:2: rule__WMLTag__Group__5__Impl rule__WMLTag__Group__6
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__52170);
            rule__WMLTag__Group__5__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__52173);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1010:1: rule__WMLTag__Group__5__Impl : ( '[/' ) ;
    public final void rule__WMLTag__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1014:1: ( ( '[/' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1015:1: ( '[/' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1015:1: ( '[/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1016:1: '[/'
            {
             before(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5()); 
            match(input,16,FOLLOW_16_in_rule__WMLTag__Group__5__Impl2201); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1029:1: rule__WMLTag__Group__6 : rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 ;
    public final void rule__WMLTag__Group__6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1033:1: ( rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1034:2: rule__WMLTag__Group__6__Impl rule__WMLTag__Group__7
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__62232);
            rule__WMLTag__Group__6__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__62235);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1041:1: rule__WMLTag__Group__6__Impl : ( ( rule__WMLTag__EndNameAssignment_6 ) ) ;
    public final void rule__WMLTag__Group__6__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1045:1: ( ( ( rule__WMLTag__EndNameAssignment_6 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1046:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1046:1: ( ( rule__WMLTag__EndNameAssignment_6 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1047:1: ( rule__WMLTag__EndNameAssignment_6 )
            {
             before(grammarAccess.getWMLTagAccess().getEndNameAssignment_6()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1048:1: ( rule__WMLTag__EndNameAssignment_6 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1048:2: rule__WMLTag__EndNameAssignment_6
            {
            pushFollow(FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl2262);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1058:1: rule__WMLTag__Group__7 : rule__WMLTag__Group__7__Impl ;
    public final void rule__WMLTag__Group__7() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1062:1: ( rule__WMLTag__Group__7__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1063:2: rule__WMLTag__Group__7__Impl
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__72292);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1069:1: rule__WMLTag__Group__7__Impl : ( ']' ) ;
    public final void rule__WMLTag__Group__7__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1073:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1074:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1074:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1075:1: ']'
            {
             before(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_7()); 
            match(input,15,FOLLOW_15_in_rule__WMLTag__Group__7__Impl2320); 
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


    // $ANTLR start rule__WMLMacroInclude__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1104:1: rule__WMLMacroInclude__Group__0 : rule__WMLMacroInclude__Group__0__Impl rule__WMLMacroInclude__Group__1 ;
    public final void rule__WMLMacroInclude__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1108:1: ( rule__WMLMacroInclude__Group__0__Impl rule__WMLMacroInclude__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1109:2: rule__WMLMacroInclude__Group__0__Impl rule__WMLMacroInclude__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroInclude__Group__0__Impl_in_rule__WMLMacroInclude__Group__02367);
            rule__WMLMacroInclude__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroInclude__Group__1_in_rule__WMLMacroInclude__Group__02370);
            rule__WMLMacroInclude__Group__1();
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
    // $ANTLR end rule__WMLMacroInclude__Group__0


    // $ANTLR start rule__WMLMacroInclude__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1116:1: rule__WMLMacroInclude__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacroInclude__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1120:1: ( ( '{' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1121:1: ( '{' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1121:1: ( '{' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1122:1: '{'
            {
             before(grammarAccess.getWMLMacroIncludeAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,17,FOLLOW_17_in_rule__WMLMacroInclude__Group__0__Impl2398); 
             after(grammarAccess.getWMLMacroIncludeAccess().getLeftCurlyBracketKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroInclude__Group__0__Impl


    // $ANTLR start rule__WMLMacroInclude__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1135:1: rule__WMLMacroInclude__Group__1 : rule__WMLMacroInclude__Group__1__Impl rule__WMLMacroInclude__Group__2 ;
    public final void rule__WMLMacroInclude__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1139:1: ( rule__WMLMacroInclude__Group__1__Impl rule__WMLMacroInclude__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1140:2: rule__WMLMacroInclude__Group__1__Impl rule__WMLMacroInclude__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroInclude__Group__1__Impl_in_rule__WMLMacroInclude__Group__12429);
            rule__WMLMacroInclude__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroInclude__Group__2_in_rule__WMLMacroInclude__Group__12432);
            rule__WMLMacroInclude__Group__2();
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
    // $ANTLR end rule__WMLMacroInclude__Group__1


    // $ANTLR start rule__WMLMacroInclude__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1147:1: rule__WMLMacroInclude__Group__1__Impl : ( ( rule__WMLMacroInclude__NameAssignment_1 ) ) ;
    public final void rule__WMLMacroInclude__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1151:1: ( ( ( rule__WMLMacroInclude__NameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1152:1: ( ( rule__WMLMacroInclude__NameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1152:1: ( ( rule__WMLMacroInclude__NameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1153:1: ( rule__WMLMacroInclude__NameAssignment_1 )
            {
             before(grammarAccess.getWMLMacroIncludeAccess().getNameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1154:1: ( rule__WMLMacroInclude__NameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1154:2: rule__WMLMacroInclude__NameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLMacroInclude__NameAssignment_1_in_rule__WMLMacroInclude__Group__1__Impl2459);
            rule__WMLMacroInclude__NameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroIncludeAccess().getNameAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroInclude__Group__1__Impl


    // $ANTLR start rule__WMLMacroInclude__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1164:1: rule__WMLMacroInclude__Group__2 : rule__WMLMacroInclude__Group__2__Impl ;
    public final void rule__WMLMacroInclude__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1168:1: ( rule__WMLMacroInclude__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1169:2: rule__WMLMacroInclude__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroInclude__Group__2__Impl_in_rule__WMLMacroInclude__Group__22489);
            rule__WMLMacroInclude__Group__2__Impl();
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
    // $ANTLR end rule__WMLMacroInclude__Group__2


    // $ANTLR start rule__WMLMacroInclude__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1175:1: rule__WMLMacroInclude__Group__2__Impl : ( '}' ) ;
    public final void rule__WMLMacroInclude__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1179:1: ( ( '}' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1180:1: ( '}' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1180:1: ( '}' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1181:1: '}'
            {
             before(grammarAccess.getWMLMacroIncludeAccess().getRightCurlyBracketKeyword_2()); 
            match(input,18,FOLLOW_18_in_rule__WMLMacroInclude__Group__2__Impl2517); 
             after(grammarAccess.getWMLMacroIncludeAccess().getRightCurlyBracketKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroInclude__Group__2__Impl


    // $ANTLR start rule__WMLMacroCall__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1200:1: rule__WMLMacroCall__Group__0 : rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 ;
    public final void rule__WMLMacroCall__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1204:1: ( rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1205:2: rule__WMLMacroCall__Group__0__Impl rule__WMLMacroCall__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__02554);
            rule__WMLMacroCall__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__02557);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1212:1: rule__WMLMacroCall__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacroCall__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1216:1: ( ( '{' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1217:1: ( '{' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1217:1: ( '{' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1218:1: '{'
            {
             before(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,17,FOLLOW_17_in_rule__WMLMacroCall__Group__0__Impl2585); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1231:1: rule__WMLMacroCall__Group__1 : rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 ;
    public final void rule__WMLMacroCall__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1235:1: ( rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1236:2: rule__WMLMacroCall__Group__1__Impl rule__WMLMacroCall__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__12616);
            rule__WMLMacroCall__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__12619);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1243:1: rule__WMLMacroCall__Group__1__Impl : ( ( rule__WMLMacroCall__NameAssignment_1 ) ) ;
    public final void rule__WMLMacroCall__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1247:1: ( ( ( rule__WMLMacroCall__NameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1248:1: ( ( rule__WMLMacroCall__NameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1248:1: ( ( rule__WMLMacroCall__NameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1249:1: ( rule__WMLMacroCall__NameAssignment_1 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1250:1: ( rule__WMLMacroCall__NameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1250:2: rule__WMLMacroCall__NameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__NameAssignment_1_in_rule__WMLMacroCall__Group__1__Impl2646);
            rule__WMLMacroCall__NameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroCallAccess().getNameAssignment_1()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1260:1: rule__WMLMacroCall__Group__2 : rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 ;
    public final void rule__WMLMacroCall__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1264:1: ( rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1265:2: rule__WMLMacroCall__Group__2__Impl rule__WMLMacroCall__Group__3
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__22676);
            rule__WMLMacroCall__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__22679);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1272:1: rule__WMLMacroCall__Group__2__Impl : ( ( rule__WMLMacroCall__ArgsAssignment_2 )* ) ;
    public final void rule__WMLMacroCall__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1276:1: ( ( ( rule__WMLMacroCall__ArgsAssignment_2 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1277:1: ( ( rule__WMLMacroCall__ArgsAssignment_2 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1277:1: ( ( rule__WMLMacroCall__ArgsAssignment_2 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1278:1: ( rule__WMLMacroCall__ArgsAssignment_2 )*
            {
             before(grammarAccess.getWMLMacroCallAccess().getArgsAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1279:1: ( rule__WMLMacroCall__ArgsAssignment_2 )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( (LA14_0==RULE_ID) ) {
                    int LA14_2 = input.LA(2);

                    if ( ((LA14_2>=RULE_ID && LA14_2<=RULE_DEFINE)||LA14_2==14||(LA14_2>=17 && LA14_2<=18)||LA14_2==21||LA14_2==26) ) {
                        alt14=1;
                    }


                }
                else if ( ((LA14_0>=RULE_STRING && LA14_0<=RULE_ANY_OTHER)||LA14_0==26) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1279:2: rule__WMLMacroCall__ArgsAssignment_2
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroCall__ArgsAssignment_2_in_rule__WMLMacroCall__Group__2__Impl2706);
            	    rule__WMLMacroCall__ArgsAssignment_2();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop14;
                }
            } while (true);

             after(grammarAccess.getWMLMacroCallAccess().getArgsAssignment_2()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1289:1: rule__WMLMacroCall__Group__3 : rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 ;
    public final void rule__WMLMacroCall__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1293:1: ( rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1294:2: rule__WMLMacroCall__Group__3__Impl rule__WMLMacroCall__Group__4
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__32737);
            rule__WMLMacroCall__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__32740);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1301:1: rule__WMLMacroCall__Group__3__Impl : ( ( rule__WMLMacroCall__Alternatives_3 )* ) ;
    public final void rule__WMLMacroCall__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1305:1: ( ( ( rule__WMLMacroCall__Alternatives_3 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1306:1: ( ( rule__WMLMacroCall__Alternatives_3 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1306:1: ( ( rule__WMLMacroCall__Alternatives_3 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1307:1: ( rule__WMLMacroCall__Alternatives_3 )*
            {
             before(grammarAccess.getWMLMacroCallAccess().getAlternatives_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1308:1: ( rule__WMLMacroCall__Alternatives_3 )*
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( (LA15_0==RULE_ID||LA15_0==RULE_DEFINE||LA15_0==14||LA15_0==17||LA15_0==21) ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1308:2: rule__WMLMacroCall__Alternatives_3
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroCall__Alternatives_3_in_rule__WMLMacroCall__Group__3__Impl2767);
            	    rule__WMLMacroCall__Alternatives_3();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop15;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1318:1: rule__WMLMacroCall__Group__4 : rule__WMLMacroCall__Group__4__Impl ;
    public final void rule__WMLMacroCall__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1322:1: ( rule__WMLMacroCall__Group__4__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1323:2: rule__WMLMacroCall__Group__4__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__42798);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1329:1: rule__WMLMacroCall__Group__4__Impl : ( '}' ) ;
    public final void rule__WMLMacroCall__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1333:1: ( ( '}' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1334:1: ( '}' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1334:1: ( '}' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1335:1: '}'
            {
             before(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_4()); 
            match(input,18,FOLLOW_18_in_rule__WMLMacroCall__Group__4__Impl2826); 
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


    // $ANTLR start rule__WMLMacroDefine__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1358:1: rule__WMLMacroDefine__Group__0 : rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1 ;
    public final void rule__WMLMacroDefine__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1362:1: ( rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1363:2: rule__WMLMacroDefine__Group__0__Impl rule__WMLMacroDefine__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__0__Impl_in_rule__WMLMacroDefine__Group__02867);
            rule__WMLMacroDefine__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__1_in_rule__WMLMacroDefine__Group__02870);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1370:1: rule__WMLMacroDefine__Group__0__Impl : ( RULE_DEFINE ) ;
    public final void rule__WMLMacroDefine__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1374:1: ( ( RULE_DEFINE ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1375:1: ( RULE_DEFINE )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1375:1: ( RULE_DEFINE )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1376:1: RULE_DEFINE
            {
             before(grammarAccess.getWMLMacroDefineAccess().getDEFINETerminalRuleCall_0()); 
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__Group__0__Impl2897); 
             after(grammarAccess.getWMLMacroDefineAccess().getDEFINETerminalRuleCall_0()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1387:1: rule__WMLMacroDefine__Group__1 : rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2 ;
    public final void rule__WMLMacroDefine__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1391:1: ( rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1392:2: rule__WMLMacroDefine__Group__1__Impl rule__WMLMacroDefine__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__1__Impl_in_rule__WMLMacroDefine__Group__12926);
            rule__WMLMacroDefine__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__2_in_rule__WMLMacroDefine__Group__12929);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1399:1: rule__WMLMacroDefine__Group__1__Impl : ( ( rule__WMLMacroDefine__Alternatives_1 )* ) ;
    public final void rule__WMLMacroDefine__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1403:1: ( ( ( rule__WMLMacroDefine__Alternatives_1 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1404:1: ( ( rule__WMLMacroDefine__Alternatives_1 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1404:1: ( ( rule__WMLMacroDefine__Alternatives_1 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1405:1: ( rule__WMLMacroDefine__Alternatives_1 )*
            {
             before(grammarAccess.getWMLMacroDefineAccess().getAlternatives_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1406:1: ( rule__WMLMacroDefine__Alternatives_1 )*
            loop16:
            do {
                int alt16=2;
                int LA16_0 = input.LA(1);

                if ( (LA16_0==RULE_ID||LA16_0==RULE_DEFINE||LA16_0==14||LA16_0==17||LA16_0==21) ) {
                    alt16=1;
                }


                switch (alt16) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1406:2: rule__WMLMacroDefine__Alternatives_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacroDefine__Alternatives_1_in_rule__WMLMacroDefine__Group__1__Impl2956);
            	    rule__WMLMacroDefine__Alternatives_1();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop16;
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1416:1: rule__WMLMacroDefine__Group__2 : rule__WMLMacroDefine__Group__2__Impl ;
    public final void rule__WMLMacroDefine__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1420:1: ( rule__WMLMacroDefine__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1421:2: rule__WMLMacroDefine__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroDefine__Group__2__Impl_in_rule__WMLMacroDefine__Group__22987);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1427:1: rule__WMLMacroDefine__Group__2__Impl : ( RULE_ENDDEFINE ) ;
    public final void rule__WMLMacroDefine__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1431:1: ( ( RULE_ENDDEFINE ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1432:1: ( RULE_ENDDEFINE )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1432:1: ( RULE_ENDDEFINE )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1433:1: RULE_ENDDEFINE
            {
             before(grammarAccess.getWMLMacroDefineAccess().getENDDEFINETerminalRuleCall_2()); 
            match(input,RULE_ENDDEFINE,FOLLOW_RULE_ENDDEFINE_in_rule__WMLMacroDefine__Group__2__Impl3014); 
             after(grammarAccess.getWMLMacroDefineAccess().getENDDEFINETerminalRuleCall_2()); 

            }


            }

        }
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


    // $ANTLR start rule__WMLKey__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1450:1: rule__WMLKey__Group__0 : rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 ;
    public final void rule__WMLKey__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1454:1: ( rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1455:2: rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__03049);
            rule__WMLKey__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__03052);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1462:1: rule__WMLKey__Group__0__Impl : ( ( rule__WMLKey__NameAssignment_0 ) ) ;
    public final void rule__WMLKey__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1466:1: ( ( ( rule__WMLKey__NameAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1467:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1467:1: ( ( rule__WMLKey__NameAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1468:1: ( rule__WMLKey__NameAssignment_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getNameAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1469:1: ( rule__WMLKey__NameAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1469:2: rule__WMLKey__NameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl3079);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1479:1: rule__WMLKey__Group__1 : rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 ;
    public final void rule__WMLKey__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1483:1: ( rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1484:2: rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__13109);
            rule__WMLKey__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__13112);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1491:1: rule__WMLKey__Group__1__Impl : ( '=' ) ;
    public final void rule__WMLKey__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1495:1: ( ( '=' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1496:1: ( '=' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1496:1: ( '=' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1497:1: '='
            {
             before(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1()); 
            match(input,19,FOLLOW_19_in_rule__WMLKey__Group__1__Impl3140); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1510:1: rule__WMLKey__Group__2 : rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 ;
    public final void rule__WMLKey__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1514:1: ( rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1515:2: rule__WMLKey__Group__2__Impl rule__WMLKey__Group__3
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__23171);
            rule__WMLKey__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__23174);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1522:1: rule__WMLKey__Group__2__Impl : ( ( rule__WMLKey__ValueAssignment_2 ) ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1526:1: ( ( ( rule__WMLKey__ValueAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1527:1: ( ( rule__WMLKey__ValueAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1527:1: ( ( rule__WMLKey__ValueAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1528:1: ( rule__WMLKey__ValueAssignment_2 )
            {
             before(grammarAccess.getWMLKeyAccess().getValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1529:1: ( rule__WMLKey__ValueAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1529:2: rule__WMLKey__ValueAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl3201);
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


    // $ANTLR start rule__WMLKey__Group__3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1539:1: rule__WMLKey__Group__3 : rule__WMLKey__Group__3__Impl ;
    public final void rule__WMLKey__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1543:1: ( rule__WMLKey__Group__3__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1544:2: rule__WMLKey__Group__3__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__33231);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1550:1: rule__WMLKey__Group__3__Impl : ( ( rule__WMLKey__Group_3__0 )* ) ;
    public final void rule__WMLKey__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1554:1: ( ( ( rule__WMLKey__Group_3__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1555:1: ( ( rule__WMLKey__Group_3__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1555:1: ( ( rule__WMLKey__Group_3__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1556:1: ( rule__WMLKey__Group_3__0 )*
            {
             before(grammarAccess.getWMLKeyAccess().getGroup_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1557:1: ( rule__WMLKey__Group_3__0 )*
            loop17:
            do {
                int alt17=2;
                int LA17_0 = input.LA(1);

                if ( (LA17_0==20) ) {
                    alt17=1;
                }


                switch (alt17) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1557:2: rule__WMLKey__Group_3__0
            	    {
            	    pushFollow(FOLLOW_rule__WMLKey__Group_3__0_in_rule__WMLKey__Group__3__Impl3258);
            	    rule__WMLKey__Group_3__0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop17;
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


    // $ANTLR start rule__WMLKey__Group_3__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1575:1: rule__WMLKey__Group_3__0 : rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1 ;
    public final void rule__WMLKey__Group_3__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1579:1: ( rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1580:2: rule__WMLKey__Group_3__0__Impl rule__WMLKey__Group_3__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__0__Impl_in_rule__WMLKey__Group_3__03297);
            rule__WMLKey__Group_3__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group_3__1_in_rule__WMLKey__Group_3__03300);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1587:1: rule__WMLKey__Group_3__0__Impl : ( '+' ) ;
    public final void rule__WMLKey__Group_3__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1591:1: ( ( '+' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1592:1: ( '+' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1592:1: ( '+' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1593:1: '+'
            {
             before(grammarAccess.getWMLKeyAccess().getPlusSignKeyword_3_0()); 
            match(input,20,FOLLOW_20_in_rule__WMLKey__Group_3__0__Impl3328); 
             after(grammarAccess.getWMLKeyAccess().getPlusSignKeyword_3_0()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1606:1: rule__WMLKey__Group_3__1 : rule__WMLKey__Group_3__1__Impl ;
    public final void rule__WMLKey__Group_3__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1610:1: ( rule__WMLKey__Group_3__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1611:2: rule__WMLKey__Group_3__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group_3__1__Impl_in_rule__WMLKey__Group_3__13359);
            rule__WMLKey__Group_3__1__Impl();
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1617:1: rule__WMLKey__Group_3__1__Impl : ( ( rule__WMLKey__ExtraArgsAssignment_3_1 ) ) ;
    public final void rule__WMLKey__Group_3__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1621:1: ( ( ( rule__WMLKey__ExtraArgsAssignment_3_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1622:1: ( ( rule__WMLKey__ExtraArgsAssignment_3_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1622:1: ( ( rule__WMLKey__ExtraArgsAssignment_3_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1623:1: ( rule__WMLKey__ExtraArgsAssignment_3_1 )
            {
             before(grammarAccess.getWMLKeyAccess().getExtraArgsAssignment_3_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1624:1: ( rule__WMLKey__ExtraArgsAssignment_3_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1624:2: rule__WMLKey__ExtraArgsAssignment_3_1
            {
            pushFollow(FOLLOW_rule__WMLKey__ExtraArgsAssignment_3_1_in_rule__WMLKey__Group_3__1__Impl3386);
            rule__WMLKey__ExtraArgsAssignment_3_1();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getExtraArgsAssignment_3_1()); 

            }


            }

        }
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


    // $ANTLR start rule__WMLMacroCallParameter__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1638:1: rule__WMLMacroCallParameter__Group__0 : rule__WMLMacroCallParameter__Group__0__Impl rule__WMLMacroCallParameter__Group__1 ;
    public final void rule__WMLMacroCallParameter__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1642:1: ( rule__WMLMacroCallParameter__Group__0__Impl rule__WMLMacroCallParameter__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1643:2: rule__WMLMacroCallParameter__Group__0__Impl rule__WMLMacroCallParameter__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacroCallParameter__Group__0__Impl_in_rule__WMLMacroCallParameter__Group__03420);
            rule__WMLMacroCallParameter__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCallParameter__Group__1_in_rule__WMLMacroCallParameter__Group__03423);
            rule__WMLMacroCallParameter__Group__1();
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
    // $ANTLR end rule__WMLMacroCallParameter__Group__0


    // $ANTLR start rule__WMLMacroCallParameter__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1650:1: rule__WMLMacroCallParameter__Group__0__Impl : ( '(' ) ;
    public final void rule__WMLMacroCallParameter__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1654:1: ( ( '(' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1655:1: ( '(' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1655:1: ( '(' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1656:1: '('
            {
             before(grammarAccess.getWMLMacroCallParameterAccess().getLeftParenthesisKeyword_0()); 
            match(input,21,FOLLOW_21_in_rule__WMLMacroCallParameter__Group__0__Impl3451); 
             after(grammarAccess.getWMLMacroCallParameterAccess().getLeftParenthesisKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCallParameter__Group__0__Impl


    // $ANTLR start rule__WMLMacroCallParameter__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1669:1: rule__WMLMacroCallParameter__Group__1 : rule__WMLMacroCallParameter__Group__1__Impl rule__WMLMacroCallParameter__Group__2 ;
    public final void rule__WMLMacroCallParameter__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1673:1: ( rule__WMLMacroCallParameter__Group__1__Impl rule__WMLMacroCallParameter__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1674:2: rule__WMLMacroCallParameter__Group__1__Impl rule__WMLMacroCallParameter__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacroCallParameter__Group__1__Impl_in_rule__WMLMacroCallParameter__Group__13482);
            rule__WMLMacroCallParameter__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacroCallParameter__Group__2_in_rule__WMLMacroCallParameter__Group__13485);
            rule__WMLMacroCallParameter__Group__2();
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
    // $ANTLR end rule__WMLMacroCallParameter__Group__1


    // $ANTLR start rule__WMLMacroCallParameter__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1681:1: rule__WMLMacroCallParameter__Group__1__Impl : ( ( rule__WMLMacroCallParameter__Alternatives_1 ) ) ;
    public final void rule__WMLMacroCallParameter__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1685:1: ( ( ( rule__WMLMacroCallParameter__Alternatives_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1686:1: ( ( rule__WMLMacroCallParameter__Alternatives_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1686:1: ( ( rule__WMLMacroCallParameter__Alternatives_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1687:1: ( rule__WMLMacroCallParameter__Alternatives_1 )
            {
             before(grammarAccess.getWMLMacroCallParameterAccess().getAlternatives_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1688:1: ( rule__WMLMacroCallParameter__Alternatives_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1688:2: rule__WMLMacroCallParameter__Alternatives_1
            {
            pushFollow(FOLLOW_rule__WMLMacroCallParameter__Alternatives_1_in_rule__WMLMacroCallParameter__Group__1__Impl3512);
            rule__WMLMacroCallParameter__Alternatives_1();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroCallParameterAccess().getAlternatives_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCallParameter__Group__1__Impl


    // $ANTLR start rule__WMLMacroCallParameter__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1698:1: rule__WMLMacroCallParameter__Group__2 : rule__WMLMacroCallParameter__Group__2__Impl ;
    public final void rule__WMLMacroCallParameter__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1702:1: ( rule__WMLMacroCallParameter__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1703:2: rule__WMLMacroCallParameter__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacroCallParameter__Group__2__Impl_in_rule__WMLMacroCallParameter__Group__23542);
            rule__WMLMacroCallParameter__Group__2__Impl();
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
    // $ANTLR end rule__WMLMacroCallParameter__Group__2


    // $ANTLR start rule__WMLMacroCallParameter__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1709:1: rule__WMLMacroCallParameter__Group__2__Impl : ( ')' ) ;
    public final void rule__WMLMacroCallParameter__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1713:1: ( ( ')' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1714:1: ( ')' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1714:1: ( ')' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1715:1: ')'
            {
             before(grammarAccess.getWMLMacroCallParameterAccess().getRightParenthesisKeyword_2()); 
            match(input,22,FOLLOW_22_in_rule__WMLMacroCallParameter__Group__2__Impl3570); 
             after(grammarAccess.getWMLMacroCallParameterAccess().getRightParenthesisKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCallParameter__Group__2__Impl


    // $ANTLR start rule__WMLPath__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1734:1: rule__WMLPath__Group__0 : rule__WMLPath__Group__0__Impl rule__WMLPath__Group__1 ;
    public final void rule__WMLPath__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1738:1: ( rule__WMLPath__Group__0__Impl rule__WMLPath__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1739:2: rule__WMLPath__Group__0__Impl rule__WMLPath__Group__1
            {
            pushFollow(FOLLOW_rule__WMLPath__Group__0__Impl_in_rule__WMLPath__Group__03607);
            rule__WMLPath__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLPath__Group__1_in_rule__WMLPath__Group__03610);
            rule__WMLPath__Group__1();
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
    // $ANTLR end rule__WMLPath__Group__0


    // $ANTLR start rule__WMLPath__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1746:1: rule__WMLPath__Group__0__Impl : ( ( '~' )? ) ;
    public final void rule__WMLPath__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1750:1: ( ( ( '~' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1751:1: ( ( '~' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1751:1: ( ( '~' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1752:1: ( '~' )?
            {
             before(grammarAccess.getWMLPathAccess().getTildeKeyword_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1753:1: ( '~' )?
            int alt18=2;
            int LA18_0 = input.LA(1);

            if ( (LA18_0==23) ) {
                alt18=1;
            }
            switch (alt18) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1754:2: '~'
                    {
                    match(input,23,FOLLOW_23_in_rule__WMLPath__Group__0__Impl3639); 

                    }
                    break;

            }

             after(grammarAccess.getWMLPathAccess().getTildeKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLPath__Group__0__Impl


    // $ANTLR start rule__WMLPath__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1765:1: rule__WMLPath__Group__1 : rule__WMLPath__Group__1__Impl rule__WMLPath__Group__2 ;
    public final void rule__WMLPath__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1769:1: ( rule__WMLPath__Group__1__Impl rule__WMLPath__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1770:2: rule__WMLPath__Group__1__Impl rule__WMLPath__Group__2
            {
            pushFollow(FOLLOW_rule__WMLPath__Group__1__Impl_in_rule__WMLPath__Group__13672);
            rule__WMLPath__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLPath__Group__2_in_rule__WMLPath__Group__13675);
            rule__WMLPath__Group__2();
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
    // $ANTLR end rule__WMLPath__Group__1


    // $ANTLR start rule__WMLPath__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1777:1: rule__WMLPath__Group__1__Impl : ( rulePATH_ID ) ;
    public final void rule__WMLPath__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1781:1: ( ( rulePATH_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1782:1: ( rulePATH_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1782:1: ( rulePATH_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1783:1: rulePATH_ID
            {
             before(grammarAccess.getWMLPathAccess().getPATH_IDParserRuleCall_1()); 
            pushFollow(FOLLOW_rulePATH_ID_in_rule__WMLPath__Group__1__Impl3702);
            rulePATH_ID();
            _fsp--;

             after(grammarAccess.getWMLPathAccess().getPATH_IDParserRuleCall_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLPath__Group__1__Impl


    // $ANTLR start rule__WMLPath__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1794:1: rule__WMLPath__Group__2 : rule__WMLPath__Group__2__Impl rule__WMLPath__Group__3 ;
    public final void rule__WMLPath__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1798:1: ( rule__WMLPath__Group__2__Impl rule__WMLPath__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1799:2: rule__WMLPath__Group__2__Impl rule__WMLPath__Group__3
            {
            pushFollow(FOLLOW_rule__WMLPath__Group__2__Impl_in_rule__WMLPath__Group__23731);
            rule__WMLPath__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLPath__Group__3_in_rule__WMLPath__Group__23734);
            rule__WMLPath__Group__3();
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
    // $ANTLR end rule__WMLPath__Group__2


    // $ANTLR start rule__WMLPath__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1806:1: rule__WMLPath__Group__2__Impl : ( ( ( rule__WMLPath__Group_2__0 ) ) ( ( rule__WMLPath__Group_2__0 )* ) ) ;
    public final void rule__WMLPath__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1810:1: ( ( ( ( rule__WMLPath__Group_2__0 ) ) ( ( rule__WMLPath__Group_2__0 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1811:1: ( ( ( rule__WMLPath__Group_2__0 ) ) ( ( rule__WMLPath__Group_2__0 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1811:1: ( ( ( rule__WMLPath__Group_2__0 ) ) ( ( rule__WMLPath__Group_2__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1812:1: ( ( rule__WMLPath__Group_2__0 ) ) ( ( rule__WMLPath__Group_2__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1812:1: ( ( rule__WMLPath__Group_2__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1813:1: ( rule__WMLPath__Group_2__0 )
            {
             before(grammarAccess.getWMLPathAccess().getGroup_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1814:1: ( rule__WMLPath__Group_2__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1814:2: rule__WMLPath__Group_2__0
            {
            pushFollow(FOLLOW_rule__WMLPath__Group_2__0_in_rule__WMLPath__Group__2__Impl3763);
            rule__WMLPath__Group_2__0();
            _fsp--;


            }

             after(grammarAccess.getWMLPathAccess().getGroup_2()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1817:1: ( ( rule__WMLPath__Group_2__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1818:1: ( rule__WMLPath__Group_2__0 )*
            {
             before(grammarAccess.getWMLPathAccess().getGroup_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1819:1: ( rule__WMLPath__Group_2__0 )*
            loop19:
            do {
                int alt19=2;
                int LA19_0 = input.LA(1);

                if ( (LA19_0==24) ) {
                    int LA19_2 = input.LA(2);

                    if ( (LA19_2==RULE_ID) ) {
                        int LA19_3 = input.LA(3);

                        if ( (LA19_3==EOF||LA19_3==RULE_ID||(LA19_3>=RULE_DEFINE && LA19_3<=RULE_ENDDEFINE)||(LA19_3>=13 && LA19_3<=14)||(LA19_3>=16 && LA19_3<=18)||(LA19_3>=20 && LA19_3<=21)||LA19_3==24) ) {
                            alt19=1;
                        }


                    }
                    else if ( (LA19_2==13) ) {
                        alt19=1;
                    }


                }


                switch (alt19) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1819:2: rule__WMLPath__Group_2__0
            	    {
            	    pushFollow(FOLLOW_rule__WMLPath__Group_2__0_in_rule__WMLPath__Group__2__Impl3775);
            	    rule__WMLPath__Group_2__0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop19;
                }
            } while (true);

             after(grammarAccess.getWMLPathAccess().getGroup_2()); 

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
    // $ANTLR end rule__WMLPath__Group__2__Impl


    // $ANTLR start rule__WMLPath__Group__3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1830:1: rule__WMLPath__Group__3 : rule__WMLPath__Group__3__Impl rule__WMLPath__Group__4 ;
    public final void rule__WMLPath__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1834:1: ( rule__WMLPath__Group__3__Impl rule__WMLPath__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1835:2: rule__WMLPath__Group__3__Impl rule__WMLPath__Group__4
            {
            pushFollow(FOLLOW_rule__WMLPath__Group__3__Impl_in_rule__WMLPath__Group__33808);
            rule__WMLPath__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLPath__Group__4_in_rule__WMLPath__Group__33811);
            rule__WMLPath__Group__4();
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
    // $ANTLR end rule__WMLPath__Group__3


    // $ANTLR start rule__WMLPath__Group__3__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1842:1: rule__WMLPath__Group__3__Impl : ( ( ruleFILE )? ) ;
    public final void rule__WMLPath__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1846:1: ( ( ( ruleFILE )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1847:1: ( ( ruleFILE )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1847:1: ( ( ruleFILE )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1848:1: ( ruleFILE )?
            {
             before(grammarAccess.getWMLPathAccess().getFILEParserRuleCall_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1849:1: ( ruleFILE )?
            int alt20=2;
            int LA20_0 = input.LA(1);

            if ( (LA20_0==RULE_ID) ) {
                int LA20_1 = input.LA(2);

                if ( (LA20_1==RULE_ID||LA20_1==13||LA20_1==27) ) {
                    alt20=1;
                }
            }
            else if ( (LA20_0==13) ) {
                alt20=1;
            }
            switch (alt20) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1849:3: ruleFILE
                    {
                    pushFollow(FOLLOW_ruleFILE_in_rule__WMLPath__Group__3__Impl3839);
                    ruleFILE();
                    _fsp--;


                    }
                    break;

            }

             after(grammarAccess.getWMLPathAccess().getFILEParserRuleCall_3()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLPath__Group__3__Impl


    // $ANTLR start rule__WMLPath__Group__4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1859:1: rule__WMLPath__Group__4 : rule__WMLPath__Group__4__Impl ;
    public final void rule__WMLPath__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1863:1: ( rule__WMLPath__Group__4__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1864:2: rule__WMLPath__Group__4__Impl
            {
            pushFollow(FOLLOW_rule__WMLPath__Group__4__Impl_in_rule__WMLPath__Group__43870);
            rule__WMLPath__Group__4__Impl();
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
    // $ANTLR end rule__WMLPath__Group__4


    // $ANTLR start rule__WMLPath__Group__4__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1870:1: rule__WMLPath__Group__4__Impl : ( ( '/' )? ) ;
    public final void rule__WMLPath__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1874:1: ( ( ( '/' )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1875:1: ( ( '/' )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1875:1: ( ( '/' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1876:1: ( '/' )?
            {
             before(grammarAccess.getWMLPathAccess().getSolidusKeyword_4()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1877:1: ( '/' )?
            int alt21=2;
            int LA21_0 = input.LA(1);

            if ( (LA21_0==24) ) {
                alt21=1;
            }
            switch (alt21) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1878:2: '/'
                    {
                    match(input,24,FOLLOW_24_in_rule__WMLPath__Group__4__Impl3899); 

                    }
                    break;

            }

             after(grammarAccess.getWMLPathAccess().getSolidusKeyword_4()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLPath__Group__4__Impl


    // $ANTLR start rule__WMLPath__Group_2__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1899:1: rule__WMLPath__Group_2__0 : rule__WMLPath__Group_2__0__Impl rule__WMLPath__Group_2__1 ;
    public final void rule__WMLPath__Group_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1903:1: ( rule__WMLPath__Group_2__0__Impl rule__WMLPath__Group_2__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1904:2: rule__WMLPath__Group_2__0__Impl rule__WMLPath__Group_2__1
            {
            pushFollow(FOLLOW_rule__WMLPath__Group_2__0__Impl_in_rule__WMLPath__Group_2__03942);
            rule__WMLPath__Group_2__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLPath__Group_2__1_in_rule__WMLPath__Group_2__03945);
            rule__WMLPath__Group_2__1();
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
    // $ANTLR end rule__WMLPath__Group_2__0


    // $ANTLR start rule__WMLPath__Group_2__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1911:1: rule__WMLPath__Group_2__0__Impl : ( '/' ) ;
    public final void rule__WMLPath__Group_2__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1915:1: ( ( '/' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1916:1: ( '/' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1916:1: ( '/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1917:1: '/'
            {
             before(grammarAccess.getWMLPathAccess().getSolidusKeyword_2_0()); 
            match(input,24,FOLLOW_24_in_rule__WMLPath__Group_2__0__Impl3973); 
             after(grammarAccess.getWMLPathAccess().getSolidusKeyword_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLPath__Group_2__0__Impl


    // $ANTLR start rule__WMLPath__Group_2__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1930:1: rule__WMLPath__Group_2__1 : rule__WMLPath__Group_2__1__Impl ;
    public final void rule__WMLPath__Group_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1934:1: ( rule__WMLPath__Group_2__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1935:2: rule__WMLPath__Group_2__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLPath__Group_2__1__Impl_in_rule__WMLPath__Group_2__14004);
            rule__WMLPath__Group_2__1__Impl();
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
    // $ANTLR end rule__WMLPath__Group_2__1


    // $ANTLR start rule__WMLPath__Group_2__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1941:1: rule__WMLPath__Group_2__1__Impl : ( rulePATH_ID ) ;
    public final void rule__WMLPath__Group_2__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1945:1: ( ( rulePATH_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1946:1: ( rulePATH_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1946:1: ( rulePATH_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1947:1: rulePATH_ID
            {
             before(grammarAccess.getWMLPathAccess().getPATH_IDParserRuleCall_2_1()); 
            pushFollow(FOLLOW_rulePATH_ID_in_rule__WMLPath__Group_2__1__Impl4031);
            rulePATH_ID();
            _fsp--;

             after(grammarAccess.getWMLPathAccess().getPATH_IDParserRuleCall_2_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLPath__Group_2__1__Impl


    // $ANTLR start rule__WMLIDList__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1962:1: rule__WMLIDList__Group__0 : rule__WMLIDList__Group__0__Impl rule__WMLIDList__Group__1 ;
    public final void rule__WMLIDList__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1966:1: ( rule__WMLIDList__Group__0__Impl rule__WMLIDList__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1967:2: rule__WMLIDList__Group__0__Impl rule__WMLIDList__Group__1
            {
            pushFollow(FOLLOW_rule__WMLIDList__Group__0__Impl_in_rule__WMLIDList__Group__04064);
            rule__WMLIDList__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLIDList__Group__1_in_rule__WMLIDList__Group__04067);
            rule__WMLIDList__Group__1();
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
    // $ANTLR end rule__WMLIDList__Group__0


    // $ANTLR start rule__WMLIDList__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1974:1: rule__WMLIDList__Group__0__Impl : ( RULE_ID ) ;
    public final void rule__WMLIDList__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1978:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1979:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1979:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1980:1: RULE_ID
            {
             before(grammarAccess.getWMLIDListAccess().getIDTerminalRuleCall_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLIDList__Group__0__Impl4094); 
             after(grammarAccess.getWMLIDListAccess().getIDTerminalRuleCall_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLIDList__Group__0__Impl


    // $ANTLR start rule__WMLIDList__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1991:1: rule__WMLIDList__Group__1 : rule__WMLIDList__Group__1__Impl ;
    public final void rule__WMLIDList__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1995:1: ( rule__WMLIDList__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1996:2: rule__WMLIDList__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLIDList__Group__1__Impl_in_rule__WMLIDList__Group__14123);
            rule__WMLIDList__Group__1__Impl();
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
    // $ANTLR end rule__WMLIDList__Group__1


    // $ANTLR start rule__WMLIDList__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2002:1: rule__WMLIDList__Group__1__Impl : ( ( rule__WMLIDList__Group_1__0 )* ) ;
    public final void rule__WMLIDList__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2006:1: ( ( ( rule__WMLIDList__Group_1__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2007:1: ( ( rule__WMLIDList__Group_1__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2007:1: ( ( rule__WMLIDList__Group_1__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2008:1: ( rule__WMLIDList__Group_1__0 )*
            {
             before(grammarAccess.getWMLIDListAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2009:1: ( rule__WMLIDList__Group_1__0 )*
            loop22:
            do {
                int alt22=2;
                int LA22_0 = input.LA(1);

                if ( (LA22_0==25) ) {
                    alt22=1;
                }


                switch (alt22) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2009:2: rule__WMLIDList__Group_1__0
            	    {
            	    pushFollow(FOLLOW_rule__WMLIDList__Group_1__0_in_rule__WMLIDList__Group__1__Impl4150);
            	    rule__WMLIDList__Group_1__0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop22;
                }
            } while (true);

             after(grammarAccess.getWMLIDListAccess().getGroup_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLIDList__Group__1__Impl


    // $ANTLR start rule__WMLIDList__Group_1__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2023:1: rule__WMLIDList__Group_1__0 : rule__WMLIDList__Group_1__0__Impl rule__WMLIDList__Group_1__1 ;
    public final void rule__WMLIDList__Group_1__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2027:1: ( rule__WMLIDList__Group_1__0__Impl rule__WMLIDList__Group_1__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2028:2: rule__WMLIDList__Group_1__0__Impl rule__WMLIDList__Group_1__1
            {
            pushFollow(FOLLOW_rule__WMLIDList__Group_1__0__Impl_in_rule__WMLIDList__Group_1__04185);
            rule__WMLIDList__Group_1__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLIDList__Group_1__1_in_rule__WMLIDList__Group_1__04188);
            rule__WMLIDList__Group_1__1();
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
    // $ANTLR end rule__WMLIDList__Group_1__0


    // $ANTLR start rule__WMLIDList__Group_1__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2035:1: rule__WMLIDList__Group_1__0__Impl : ( ',' ) ;
    public final void rule__WMLIDList__Group_1__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2039:1: ( ( ',' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2040:1: ( ',' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2040:1: ( ',' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2041:1: ','
            {
             before(grammarAccess.getWMLIDListAccess().getCommaKeyword_1_0()); 
            match(input,25,FOLLOW_25_in_rule__WMLIDList__Group_1__0__Impl4216); 
             after(grammarAccess.getWMLIDListAccess().getCommaKeyword_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLIDList__Group_1__0__Impl


    // $ANTLR start rule__WMLIDList__Group_1__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2054:1: rule__WMLIDList__Group_1__1 : rule__WMLIDList__Group_1__1__Impl ;
    public final void rule__WMLIDList__Group_1__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2058:1: ( rule__WMLIDList__Group_1__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2059:2: rule__WMLIDList__Group_1__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLIDList__Group_1__1__Impl_in_rule__WMLIDList__Group_1__14247);
            rule__WMLIDList__Group_1__1__Impl();
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
    // $ANTLR end rule__WMLIDList__Group_1__1


    // $ANTLR start rule__WMLIDList__Group_1__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2065:1: rule__WMLIDList__Group_1__1__Impl : ( RULE_ID ) ;
    public final void rule__WMLIDList__Group_1__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2069:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2070:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2070:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2071:1: RULE_ID
            {
             before(grammarAccess.getWMLIDListAccess().getIDTerminalRuleCall_1_1()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLIDList__Group_1__1__Impl4274); 
             after(grammarAccess.getWMLIDListAccess().getIDTerminalRuleCall_1_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLIDList__Group_1__1__Impl


    // $ANTLR start rule__WMLINTList__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2086:1: rule__WMLINTList__Group__0 : rule__WMLINTList__Group__0__Impl rule__WMLINTList__Group__1 ;
    public final void rule__WMLINTList__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2090:1: ( rule__WMLINTList__Group__0__Impl rule__WMLINTList__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2091:2: rule__WMLINTList__Group__0__Impl rule__WMLINTList__Group__1
            {
            pushFollow(FOLLOW_rule__WMLINTList__Group__0__Impl_in_rule__WMLINTList__Group__04307);
            rule__WMLINTList__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLINTList__Group__1_in_rule__WMLINTList__Group__04310);
            rule__WMLINTList__Group__1();
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
    // $ANTLR end rule__WMLINTList__Group__0


    // $ANTLR start rule__WMLINTList__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2098:1: rule__WMLINTList__Group__0__Impl : ( RULE_INT ) ;
    public final void rule__WMLINTList__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2102:1: ( ( RULE_INT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2103:1: ( RULE_INT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2103:1: ( RULE_INT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2104:1: RULE_INT
            {
             before(grammarAccess.getWMLINTListAccess().getINTTerminalRuleCall_0()); 
            match(input,RULE_INT,FOLLOW_RULE_INT_in_rule__WMLINTList__Group__0__Impl4337); 
             after(grammarAccess.getWMLINTListAccess().getINTTerminalRuleCall_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLINTList__Group__0__Impl


    // $ANTLR start rule__WMLINTList__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2115:1: rule__WMLINTList__Group__1 : rule__WMLINTList__Group__1__Impl ;
    public final void rule__WMLINTList__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2119:1: ( rule__WMLINTList__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2120:2: rule__WMLINTList__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLINTList__Group__1__Impl_in_rule__WMLINTList__Group__14366);
            rule__WMLINTList__Group__1__Impl();
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
    // $ANTLR end rule__WMLINTList__Group__1


    // $ANTLR start rule__WMLINTList__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2126:1: rule__WMLINTList__Group__1__Impl : ( ( rule__WMLINTList__Group_1__0 )* ) ;
    public final void rule__WMLINTList__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2130:1: ( ( ( rule__WMLINTList__Group_1__0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2131:1: ( ( rule__WMLINTList__Group_1__0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2131:1: ( ( rule__WMLINTList__Group_1__0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2132:1: ( rule__WMLINTList__Group_1__0 )*
            {
             before(grammarAccess.getWMLINTListAccess().getGroup_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2133:1: ( rule__WMLINTList__Group_1__0 )*
            loop23:
            do {
                int alt23=2;
                int LA23_0 = input.LA(1);

                if ( (LA23_0==25) ) {
                    alt23=1;
                }


                switch (alt23) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2133:2: rule__WMLINTList__Group_1__0
            	    {
            	    pushFollow(FOLLOW_rule__WMLINTList__Group_1__0_in_rule__WMLINTList__Group__1__Impl4393);
            	    rule__WMLINTList__Group_1__0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop23;
                }
            } while (true);

             after(grammarAccess.getWMLINTListAccess().getGroup_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLINTList__Group__1__Impl


    // $ANTLR start rule__WMLINTList__Group_1__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2147:1: rule__WMLINTList__Group_1__0 : rule__WMLINTList__Group_1__0__Impl rule__WMLINTList__Group_1__1 ;
    public final void rule__WMLINTList__Group_1__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2151:1: ( rule__WMLINTList__Group_1__0__Impl rule__WMLINTList__Group_1__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2152:2: rule__WMLINTList__Group_1__0__Impl rule__WMLINTList__Group_1__1
            {
            pushFollow(FOLLOW_rule__WMLINTList__Group_1__0__Impl_in_rule__WMLINTList__Group_1__04428);
            rule__WMLINTList__Group_1__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLINTList__Group_1__1_in_rule__WMLINTList__Group_1__04431);
            rule__WMLINTList__Group_1__1();
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
    // $ANTLR end rule__WMLINTList__Group_1__0


    // $ANTLR start rule__WMLINTList__Group_1__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2159:1: rule__WMLINTList__Group_1__0__Impl : ( ',' ) ;
    public final void rule__WMLINTList__Group_1__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2163:1: ( ( ',' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2164:1: ( ',' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2164:1: ( ',' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2165:1: ','
            {
             before(grammarAccess.getWMLINTListAccess().getCommaKeyword_1_0()); 
            match(input,25,FOLLOW_25_in_rule__WMLINTList__Group_1__0__Impl4459); 
             after(grammarAccess.getWMLINTListAccess().getCommaKeyword_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLINTList__Group_1__0__Impl


    // $ANTLR start rule__WMLINTList__Group_1__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2178:1: rule__WMLINTList__Group_1__1 : rule__WMLINTList__Group_1__1__Impl ;
    public final void rule__WMLINTList__Group_1__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2182:1: ( rule__WMLINTList__Group_1__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2183:2: rule__WMLINTList__Group_1__1__Impl
            {
            pushFollow(FOLLOW_rule__WMLINTList__Group_1__1__Impl_in_rule__WMLINTList__Group_1__14490);
            rule__WMLINTList__Group_1__1__Impl();
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
    // $ANTLR end rule__WMLINTList__Group_1__1


    // $ANTLR start rule__WMLINTList__Group_1__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2189:1: rule__WMLINTList__Group_1__1__Impl : ( RULE_INT ) ;
    public final void rule__WMLINTList__Group_1__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2193:1: ( ( RULE_INT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2194:1: ( RULE_INT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2194:1: ( RULE_INT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2195:1: RULE_INT
            {
             before(grammarAccess.getWMLINTListAccess().getINTTerminalRuleCall_1_1()); 
            match(input,RULE_INT,FOLLOW_RULE_INT_in_rule__WMLINTList__Group_1__1__Impl4517); 
             after(grammarAccess.getWMLINTListAccess().getINTTerminalRuleCall_1_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLINTList__Group_1__1__Impl


    // $ANTLR start rule__TSTRING__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2210:1: rule__TSTRING__Group__0 : rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 ;
    public final void rule__TSTRING__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2214:1: ( rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2215:2: rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__04550);
            rule__TSTRING__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__04553);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2222:1: rule__TSTRING__Group__0__Impl : ( '_' ) ;
    public final void rule__TSTRING__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2226:1: ( ( '_' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2227:1: ( '_' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2227:1: ( '_' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2228:1: '_'
            {
             before(grammarAccess.getTSTRINGAccess().get_Keyword_0()); 
            match(input,26,FOLLOW_26_in_rule__TSTRING__Group__0__Impl4581); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2241:1: rule__TSTRING__Group__1 : rule__TSTRING__Group__1__Impl ;
    public final void rule__TSTRING__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2245:1: ( rule__TSTRING__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2246:2: rule__TSTRING__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__14612);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2252:1: rule__TSTRING__Group__1__Impl : ( RULE_STRING ) ;
    public final void rule__TSTRING__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2256:1: ( ( RULE_STRING ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2257:1: ( RULE_STRING )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2257:1: ( RULE_STRING )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2258:1: RULE_STRING
            {
             before(grammarAccess.getTSTRINGAccess().getSTRINGTerminalRuleCall_1()); 
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl4639); 
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


    // $ANTLR start rule__FILE__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2273:1: rule__FILE__Group__0 : rule__FILE__Group__0__Impl rule__FILE__Group__1 ;
    public final void rule__FILE__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2277:1: ( rule__FILE__Group__0__Impl rule__FILE__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2278:2: rule__FILE__Group__0__Impl rule__FILE__Group__1
            {
            pushFollow(FOLLOW_rule__FILE__Group__0__Impl_in_rule__FILE__Group__04672);
            rule__FILE__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__FILE__Group__1_in_rule__FILE__Group__04675);
            rule__FILE__Group__1();
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
    // $ANTLR end rule__FILE__Group__0


    // $ANTLR start rule__FILE__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2285:1: rule__FILE__Group__0__Impl : ( ( ( rulePATH_ID ) ) ( ( rulePATH_ID )* ) ) ;
    public final void rule__FILE__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2289:1: ( ( ( ( rulePATH_ID ) ) ( ( rulePATH_ID )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2290:1: ( ( ( rulePATH_ID ) ) ( ( rulePATH_ID )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2290:1: ( ( ( rulePATH_ID ) ) ( ( rulePATH_ID )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2291:1: ( ( rulePATH_ID ) ) ( ( rulePATH_ID )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2291:1: ( ( rulePATH_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2292:1: ( rulePATH_ID )
            {
             before(grammarAccess.getFILEAccess().getPATH_IDParserRuleCall_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2293:1: ( rulePATH_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2293:3: rulePATH_ID
            {
            pushFollow(FOLLOW_rulePATH_ID_in_rule__FILE__Group__0__Impl4705);
            rulePATH_ID();
            _fsp--;


            }

             after(grammarAccess.getFILEAccess().getPATH_IDParserRuleCall_0()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2296:1: ( ( rulePATH_ID )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2297:1: ( rulePATH_ID )*
            {
             before(grammarAccess.getFILEAccess().getPATH_IDParserRuleCall_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2298:1: ( rulePATH_ID )*
            loop24:
            do {
                int alt24=2;
                int LA24_0 = input.LA(1);

                if ( (LA24_0==RULE_ID||LA24_0==13) ) {
                    alt24=1;
                }


                switch (alt24) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2298:3: rulePATH_ID
            	    {
            	    pushFollow(FOLLOW_rulePATH_ID_in_rule__FILE__Group__0__Impl4718);
            	    rulePATH_ID();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop24;
                }
            } while (true);

             after(grammarAccess.getFILEAccess().getPATH_IDParserRuleCall_0()); 

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
    // $ANTLR end rule__FILE__Group__0__Impl


    // $ANTLR start rule__FILE__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2309:1: rule__FILE__Group__1 : rule__FILE__Group__1__Impl rule__FILE__Group__2 ;
    public final void rule__FILE__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2313:1: ( rule__FILE__Group__1__Impl rule__FILE__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2314:2: rule__FILE__Group__1__Impl rule__FILE__Group__2
            {
            pushFollow(FOLLOW_rule__FILE__Group__1__Impl_in_rule__FILE__Group__14751);
            rule__FILE__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__FILE__Group__2_in_rule__FILE__Group__14754);
            rule__FILE__Group__2();
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
    // $ANTLR end rule__FILE__Group__1


    // $ANTLR start rule__FILE__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2321:1: rule__FILE__Group__1__Impl : ( '.' ) ;
    public final void rule__FILE__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2325:1: ( ( '.' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2326:1: ( '.' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2326:1: ( '.' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2327:1: '.'
            {
             before(grammarAccess.getFILEAccess().getFullStopKeyword_1()); 
            match(input,27,FOLLOW_27_in_rule__FILE__Group__1__Impl4782); 
             after(grammarAccess.getFILEAccess().getFullStopKeyword_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__FILE__Group__1__Impl


    // $ANTLR start rule__FILE__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2340:1: rule__FILE__Group__2 : rule__FILE__Group__2__Impl ;
    public final void rule__FILE__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2344:1: ( rule__FILE__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2345:2: rule__FILE__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__FILE__Group__2__Impl_in_rule__FILE__Group__24813);
            rule__FILE__Group__2__Impl();
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
    // $ANTLR end rule__FILE__Group__2


    // $ANTLR start rule__FILE__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2351:1: rule__FILE__Group__2__Impl : ( RULE_ID ) ;
    public final void rule__FILE__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2355:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2356:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2356:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2357:1: RULE_ID
            {
             before(grammarAccess.getFILEAccess().getIDTerminalRuleCall_2()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__FILE__Group__2__Impl4840); 
             after(grammarAccess.getFILEAccess().getIDTerminalRuleCall_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__FILE__Group__2__Impl


    // $ANTLR start rule__WMLRoot__TagsAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2375:1: rule__WMLRoot__TagsAssignment_0 : ( ruleWMLTag ) ;
    public final void rule__WMLRoot__TagsAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2379:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2380:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2380:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2381:1: ruleWMLTag
            {
             before(grammarAccess.getWMLRootAccess().getTagsWMLTagParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRoot__TagsAssignment_04880);
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


    // $ANTLR start rule__WMLRoot__MacrosAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2390:1: rule__WMLRoot__MacrosAssignment_1 : ( ruleWMLAbstractMacroCall ) ;
    public final void rule__WMLRoot__MacrosAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2394:1: ( ( ruleWMLAbstractMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2395:1: ( ruleWMLAbstractMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2395:1: ( ruleWMLAbstractMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2396:1: ruleWMLAbstractMacroCall
            {
             before(grammarAccess.getWMLRootAccess().getMacrosWMLAbstractMacroCallParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLAbstractMacroCall_in_rule__WMLRoot__MacrosAssignment_14911);
            ruleWMLAbstractMacroCall();
            _fsp--;

             after(grammarAccess.getWMLRootAccess().getMacrosWMLAbstractMacroCallParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLRoot__MacrosAssignment_1


    // $ANTLR start rule__WMLRoot__MacrosDefinesAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2405:1: rule__WMLRoot__MacrosDefinesAssignment_2 : ( ruleWMLMacroDefine ) ;
    public final void rule__WMLRoot__MacrosDefinesAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2409:1: ( ( ruleWMLMacroDefine ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2410:1: ( ruleWMLMacroDefine )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2410:1: ( ruleWMLMacroDefine )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2411:1: ruleWMLMacroDefine
            {
             before(grammarAccess.getWMLRootAccess().getMacrosDefinesWMLMacroDefineParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLRoot__MacrosDefinesAssignment_24942);
            ruleWMLMacroDefine();
            _fsp--;

             after(grammarAccess.getWMLRootAccess().getMacrosDefinesWMLMacroDefineParserRuleCall_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLRoot__MacrosDefinesAssignment_2


    // $ANTLR start rule__WMLTag__PlusAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2420:1: rule__WMLTag__PlusAssignment_1 : ( ( '+' ) ) ;
    public final void rule__WMLTag__PlusAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2424:1: ( ( ( '+' ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2425:1: ( ( '+' ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2425:1: ( ( '+' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2426:1: ( '+' )
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2427:1: ( '+' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2428:1: '+'
            {
             before(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0()); 
            match(input,20,FOLLOW_20_in_rule__WMLTag__PlusAssignment_14978); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2443:1: rule__WMLTag__NameAssignment_2 : ( RULE_ID ) ;
    public final void rule__WMLTag__NameAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2447:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2448:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2448:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2449:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_2_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_25017); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2458:1: rule__WMLTag__TagsAssignment_4_0 : ( ruleWMLTag ) ;
    public final void rule__WMLTag__TagsAssignment_4_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2462:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2463:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2463:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2464:1: ruleWMLTag
            {
             before(grammarAccess.getWMLTagAccess().getTagsWMLTagParserRuleCall_4_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLTag__TagsAssignment_4_05048);
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


    // $ANTLR start rule__WMLTag__MacrosAssignment_4_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2473:1: rule__WMLTag__MacrosAssignment_4_1 : ( ruleWMLAbstractMacroCall ) ;
    public final void rule__WMLTag__MacrosAssignment_4_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2477:1: ( ( ruleWMLAbstractMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2478:1: ( ruleWMLAbstractMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2478:1: ( ruleWMLAbstractMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2479:1: ruleWMLAbstractMacroCall
            {
             before(grammarAccess.getWMLTagAccess().getMacrosWMLAbstractMacroCallParserRuleCall_4_1_0()); 
            pushFollow(FOLLOW_ruleWMLAbstractMacroCall_in_rule__WMLTag__MacrosAssignment_4_15079);
            ruleWMLAbstractMacroCall();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getMacrosWMLAbstractMacroCallParserRuleCall_4_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__MacrosAssignment_4_1


    // $ANTLR start rule__WMLTag__MacrosDefinesAssignment_4_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2488:1: rule__WMLTag__MacrosDefinesAssignment_4_2 : ( ruleWMLMacroDefine ) ;
    public final void rule__WMLTag__MacrosDefinesAssignment_4_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2492:1: ( ( ruleWMLMacroDefine ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2493:1: ( ruleWMLMacroDefine )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2493:1: ( ruleWMLMacroDefine )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2494:1: ruleWMLMacroDefine
            {
             before(grammarAccess.getWMLTagAccess().getMacrosDefinesWMLMacroDefineParserRuleCall_4_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLTag__MacrosDefinesAssignment_4_25110);
            ruleWMLMacroDefine();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getMacrosDefinesWMLMacroDefineParserRuleCall_4_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__MacrosDefinesAssignment_4_2


    // $ANTLR start rule__WMLTag__KeysAssignment_4_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2503:1: rule__WMLTag__KeysAssignment_4_3 : ( ruleWMLKey ) ;
    public final void rule__WMLTag__KeysAssignment_4_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2507:1: ( ( ruleWMLKey ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2508:1: ( ruleWMLKey )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2508:1: ( ruleWMLKey )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2509:1: ruleWMLKey
            {
             before(grammarAccess.getWMLTagAccess().getKeysWMLKeyParserRuleCall_4_3_0()); 
            pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLTag__KeysAssignment_4_35141);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getKeysWMLKeyParserRuleCall_4_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__KeysAssignment_4_3


    // $ANTLR start rule__WMLTag__EndNameAssignment_6
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2518:1: rule__WMLTag__EndNameAssignment_6 : ( RULE_ID ) ;
    public final void rule__WMLTag__EndNameAssignment_6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2522:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2523:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2523:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2524:1: RULE_ID
            {
             before(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_65172); 
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


    // $ANTLR start rule__WMLMacroInclude__NameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2533:1: rule__WMLMacroInclude__NameAssignment_1 : ( ruleWMLPath ) ;
    public final void rule__WMLMacroInclude__NameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2537:1: ( ( ruleWMLPath ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2538:1: ( ruleWMLPath )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2538:1: ( ruleWMLPath )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2539:1: ruleWMLPath
            {
             before(grammarAccess.getWMLMacroIncludeAccess().getNameWMLPathParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLPath_in_rule__WMLMacroInclude__NameAssignment_15203);
            ruleWMLPath();
            _fsp--;

             after(grammarAccess.getWMLMacroIncludeAccess().getNameWMLPathParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroInclude__NameAssignment_1


    // $ANTLR start rule__WMLMacroCall__NameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2548:1: rule__WMLMacroCall__NameAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLMacroCall__NameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2552:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2553:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2553:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2554:1: RULE_ID
            {
             before(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_15234); 
             after(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__NameAssignment_1


    // $ANTLR start rule__WMLMacroCall__ArgsAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2563:1: rule__WMLMacroCall__ArgsAssignment_2 : ( ( rule__WMLMacroCall__ArgsAlternatives_2_0 ) ) ;
    public final void rule__WMLMacroCall__ArgsAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2567:1: ( ( ( rule__WMLMacroCall__ArgsAlternatives_2_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2568:1: ( ( rule__WMLMacroCall__ArgsAlternatives_2_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2568:1: ( ( rule__WMLMacroCall__ArgsAlternatives_2_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2569:1: ( rule__WMLMacroCall__ArgsAlternatives_2_0 )
            {
             before(grammarAccess.getWMLMacroCallAccess().getArgsAlternatives_2_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2570:1: ( rule__WMLMacroCall__ArgsAlternatives_2_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2570:2: rule__WMLMacroCall__ArgsAlternatives_2_0
            {
            pushFollow(FOLLOW_rule__WMLMacroCall__ArgsAlternatives_2_0_in_rule__WMLMacroCall__ArgsAssignment_25265);
            rule__WMLMacroCall__ArgsAlternatives_2_0();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroCallAccess().getArgsAlternatives_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__ArgsAssignment_2


    // $ANTLR start rule__WMLMacroCall__ParamsAssignment_3_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2579:1: rule__WMLMacroCall__ParamsAssignment_3_0 : ( ruleWMLMacroCallParameter ) ;
    public final void rule__WMLMacroCall__ParamsAssignment_3_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2583:1: ( ( ruleWMLMacroCallParameter ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2584:1: ( ruleWMLMacroCallParameter )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2584:1: ( ruleWMLMacroCallParameter )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2585:1: ruleWMLMacroCallParameter
            {
             before(grammarAccess.getWMLMacroCallAccess().getParamsWMLMacroCallParameterParserRuleCall_3_0_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_rule__WMLMacroCall__ParamsAssignment_3_05298);
            ruleWMLMacroCallParameter();
            _fsp--;

             after(grammarAccess.getWMLMacroCallAccess().getParamsWMLMacroCallParameterParserRuleCall_3_0_0()); 

            }


            }

        }
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


    // $ANTLR start rule__WMLMacroCall__TagsAssignment_3_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2594:1: rule__WMLMacroCall__TagsAssignment_3_1 : ( ruleWMLTag ) ;
    public final void rule__WMLMacroCall__TagsAssignment_3_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2598:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2599:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2599:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2600:1: ruleWMLTag
            {
             before(grammarAccess.getWMLMacroCallAccess().getTagsWMLTagParserRuleCall_3_1_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLMacroCall__TagsAssignment_3_15329);
            ruleWMLTag();
            _fsp--;

             after(grammarAccess.getWMLMacroCallAccess().getTagsWMLTagParserRuleCall_3_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__TagsAssignment_3_1


    // $ANTLR start rule__WMLMacroCall__MacrosAssignment_3_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2609:1: rule__WMLMacroCall__MacrosAssignment_3_2 : ( ruleWMLMacroCall ) ;
    public final void rule__WMLMacroCall__MacrosAssignment_3_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2613:1: ( ( ruleWMLMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2614:1: ( ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2614:1: ( ruleWMLMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2615:1: ruleWMLMacroCall
            {
             before(grammarAccess.getWMLMacroCallAccess().getMacrosWMLMacroCallParserRuleCall_3_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCall__MacrosAssignment_3_25360);
            ruleWMLMacroCall();
            _fsp--;

             after(grammarAccess.getWMLMacroCallAccess().getMacrosWMLMacroCallParserRuleCall_3_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__MacrosAssignment_3_2


    // $ANTLR start rule__WMLMacroCall__MacrosDefinesAssignment_3_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2624:1: rule__WMLMacroCall__MacrosDefinesAssignment_3_3 : ( ruleWMLMacroDefine ) ;
    public final void rule__WMLMacroCall__MacrosDefinesAssignment_3_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2628:1: ( ( ruleWMLMacroDefine ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2629:1: ( ruleWMLMacroDefine )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2629:1: ( ruleWMLMacroDefine )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2630:1: ruleWMLMacroDefine
            {
             before(grammarAccess.getWMLMacroCallAccess().getMacrosDefinesWMLMacroDefineParserRuleCall_3_3_0()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLMacroCall__MacrosDefinesAssignment_3_35391);
            ruleWMLMacroDefine();
            _fsp--;

             after(grammarAccess.getWMLMacroCallAccess().getMacrosDefinesWMLMacroDefineParserRuleCall_3_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__MacrosDefinesAssignment_3_3


    // $ANTLR start rule__WMLMacroCall__KeysAssignment_3_4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2639:1: rule__WMLMacroCall__KeysAssignment_3_4 : ( ruleWMLKey ) ;
    public final void rule__WMLMacroCall__KeysAssignment_3_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2643:1: ( ( ruleWMLKey ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2644:1: ( ruleWMLKey )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2644:1: ( ruleWMLKey )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2645:1: ruleWMLKey
            {
             before(grammarAccess.getWMLMacroCallAccess().getKeysWMLKeyParserRuleCall_3_4_0()); 
            pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLMacroCall__KeysAssignment_3_45422);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLMacroCallAccess().getKeysWMLKeyParserRuleCall_3_4_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroCall__KeysAssignment_3_4


    // $ANTLR start rule__WMLMacroDefine__ParamsAssignment_1_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2654:1: rule__WMLMacroDefine__ParamsAssignment_1_0 : ( ruleWMLMacroCallParameter ) ;
    public final void rule__WMLMacroDefine__ParamsAssignment_1_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2658:1: ( ( ruleWMLMacroCallParameter ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2659:1: ( ruleWMLMacroCallParameter )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2659:1: ( ruleWMLMacroCallParameter )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2660:1: ruleWMLMacroCallParameter
            {
             before(grammarAccess.getWMLMacroDefineAccess().getParamsWMLMacroCallParameterParserRuleCall_1_0_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_rule__WMLMacroDefine__ParamsAssignment_1_05453);
            ruleWMLMacroCallParameter();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineAccess().getParamsWMLMacroCallParameterParserRuleCall_1_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__ParamsAssignment_1_0


    // $ANTLR start rule__WMLMacroDefine__TagsAssignment_1_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2669:1: rule__WMLMacroDefine__TagsAssignment_1_1 : ( ruleWMLTag ) ;
    public final void rule__WMLMacroDefine__TagsAssignment_1_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2673:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2674:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2674:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2675:1: ruleWMLTag
            {
             before(grammarAccess.getWMLMacroDefineAccess().getTagsWMLTagParserRuleCall_1_1_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLMacroDefine__TagsAssignment_1_15484);
            ruleWMLTag();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineAccess().getTagsWMLTagParserRuleCall_1_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__TagsAssignment_1_1


    // $ANTLR start rule__WMLMacroDefine__MacrosAssignment_1_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2684:1: rule__WMLMacroDefine__MacrosAssignment_1_2 : ( ruleWMLMacroCall ) ;
    public final void rule__WMLMacroDefine__MacrosAssignment_1_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2688:1: ( ( ruleWMLMacroCall ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2689:1: ( ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2689:1: ( ruleWMLMacroCall )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2690:1: ruleWMLMacroCall
            {
             before(grammarAccess.getWMLMacroDefineAccess().getMacrosWMLMacroCallParserRuleCall_1_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroDefine__MacrosAssignment_1_25515);
            ruleWMLMacroCall();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineAccess().getMacrosWMLMacroCallParserRuleCall_1_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__MacrosAssignment_1_2


    // $ANTLR start rule__WMLMacroDefine__MacrosDefinesAssignment_1_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2699:1: rule__WMLMacroDefine__MacrosDefinesAssignment_1_3 : ( ruleWMLMacroDefine ) ;
    public final void rule__WMLMacroDefine__MacrosDefinesAssignment_1_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2703:1: ( ( ruleWMLMacroDefine ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2704:1: ( ruleWMLMacroDefine )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2704:1: ( ruleWMLMacroDefine )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2705:1: ruleWMLMacroDefine
            {
             before(grammarAccess.getWMLMacroDefineAccess().getMacrosDefinesWMLMacroDefineParserRuleCall_1_3_0()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_rule__WMLMacroDefine__MacrosDefinesAssignment_1_35546);
            ruleWMLMacroDefine();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineAccess().getMacrosDefinesWMLMacroDefineParserRuleCall_1_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__MacrosDefinesAssignment_1_3


    // $ANTLR start rule__WMLMacroDefine__KeysAssignment_1_4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2714:1: rule__WMLMacroDefine__KeysAssignment_1_4 : ( ruleWMLKey ) ;
    public final void rule__WMLMacroDefine__KeysAssignment_1_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2718:1: ( ( ruleWMLKey ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2719:1: ( ruleWMLKey )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2719:1: ( ruleWMLKey )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2720:1: ruleWMLKey
            {
             before(grammarAccess.getWMLMacroDefineAccess().getKeysWMLKeyParserRuleCall_1_4_0()); 
            pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLMacroDefine__KeysAssignment_1_45577);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLMacroDefineAccess().getKeysWMLKeyParserRuleCall_1_4_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacroDefine__KeysAssignment_1_4


    // $ANTLR start rule__WMLKey__NameAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2730:1: rule__WMLKey__NameAssignment_0 : ( ruleWMLIDList ) ;
    public final void rule__WMLKey__NameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2734:1: ( ( ruleWMLIDList ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2735:1: ( ruleWMLIDList )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2735:1: ( ruleWMLIDList )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2736:1: ruleWMLIDList
            {
             before(grammarAccess.getWMLKeyAccess().getNameWMLIDListParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleWMLIDList_in_rule__WMLKey__NameAssignment_05609);
            ruleWMLIDList();
            _fsp--;

             after(grammarAccess.getWMLKeyAccess().getNameWMLIDListParserRuleCall_0_0()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2745:1: rule__WMLKey__ValueAssignment_2 : ( ruleWMLKeyValue ) ;
    public final void rule__WMLKey__ValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2749:1: ( ( ruleWMLKeyValue ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2750:1: ( ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2750:1: ( ruleWMLKeyValue )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2751:1: ruleWMLKeyValue
            {
             before(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_25640);
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


    // $ANTLR start rule__WMLKey__ExtraArgsAssignment_3_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2760:1: rule__WMLKey__ExtraArgsAssignment_3_1 : ( ruleWMLKeyExtraArgs ) ;
    public final void rule__WMLKey__ExtraArgsAssignment_3_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2764:1: ( ( ruleWMLKeyExtraArgs ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2765:1: ( ruleWMLKeyExtraArgs )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2765:1: ( ruleWMLKeyExtraArgs )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:2766:1: ruleWMLKeyExtraArgs
            {
             before(grammarAccess.getWMLKeyAccess().getExtraArgsWMLKeyExtraArgsParserRuleCall_3_1_0()); 
            pushFollow(FOLLOW_ruleWMLKeyExtraArgs_in_rule__WMLKey__ExtraArgsAssignment_3_15671);
            ruleWMLKeyExtraArgs();
            _fsp--;

             after(grammarAccess.getWMLKeyAccess().getExtraArgsWMLKeyExtraArgsParserRuleCall_3_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__ExtraArgsAssignment_3_1


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot61 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot68 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__Alternatives_in_ruleWMLRoot94 = new BitSet(new long[]{0x0000000000024082L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag122 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag129 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0_in_ruleWMLTag155 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLAbstractMacroCall_in_entryRuleWMLAbstractMacroCall182 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLAbstractMacroCall189 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLAbstractMacroCall__Alternatives_in_ruleWMLAbstractMacroCall215 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroInclude_in_entryRuleWMLMacroInclude242 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroInclude249 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroInclude__Group__0_in_ruleWMLMacroInclude275 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall302 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCall309 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__0_in_ruleWMLMacroCall335 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine362 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine369 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__0_in_ruleWMLMacroDefine395 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey424 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey431 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey457 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyExtraArgs_in_entryRuleWMLKeyExtraArgs484 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyExtraArgs491 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyExtraArgs__Alternatives_in_ruleWMLKeyExtraArgs517 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_entryRuleWMLMacroCallParameter544 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCallParameter551 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Group__0_in_ruleWMLMacroCallParameter577 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue604 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue611 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKeyValue__Alternatives_in_ruleWMLKeyValue637 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPath_in_entryRuleWMLPath664 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLPath671 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPath__Group__0_in_ruleWMLPath697 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLIDList_in_entryRuleWMLIDList724 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLIDList731 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLIDList__Group__0_in_ruleWMLIDList757 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLINTList_in_entryRuleWMLINTList784 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLINTList791 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLINTList__Group__0_in_ruleWMLINTList817 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING844 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING851 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0_in_ruleTSTRING877 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFILE_in_entryRuleFILE904 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleFILE911 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FILE__Group__0_in_ruleFILE937 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_ID_in_entryRulePATH_ID964 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePATH_ID971 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PATH_ID__Alternatives_in_rulePATH_ID997 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__TagsAssignment_0_in_rule__WMLRoot__Alternatives1033 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__MacrosAssignment_1_in_rule__WMLRoot__Alternatives1051 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__MacrosDefinesAssignment_2_in_rule__WMLRoot__Alternatives1069 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TagsAssignment_4_0_in_rule__WMLTag__Alternatives_41102 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__MacrosAssignment_4_1_in_rule__WMLTag__Alternatives_41120 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__MacrosDefinesAssignment_4_2_in_rule__WMLTag__Alternatives_41138 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__KeysAssignment_4_3_in_rule__WMLTag__Alternatives_41156 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroInclude_in_rule__WMLAbstractMacroCall__Alternatives1189 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLAbstractMacroCall__Alternatives1206 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacroCall__ArgsAlternatives_2_01238 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLMacroCall__ArgsAlternatives_2_01255 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_rule__WMLMacroCall__ArgsAlternatives_2_01272 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_rule__WMLMacroCall__ArgsAlternatives_2_01289 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ParamsAssignment_3_0_in_rule__WMLMacroCall__Alternatives_31321 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__TagsAssignment_3_1_in_rule__WMLMacroCall__Alternatives_31339 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__MacrosAssignment_3_2_in_rule__WMLMacroCall__Alternatives_31357 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__MacrosDefinesAssignment_3_3_in_rule__WMLMacroCall__Alternatives_31375 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__KeysAssignment_3_4_in_rule__WMLMacroCall__Alternatives_31393 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__ParamsAssignment_1_0_in_rule__WMLMacroDefine__Alternatives_11426 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__TagsAssignment_1_1_in_rule__WMLMacroDefine__Alternatives_11444 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__MacrosAssignment_1_2_in_rule__WMLMacroDefine__Alternatives_11462 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__MacrosDefinesAssignment_1_3_in_rule__WMLMacroDefine__Alternatives_11480 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__KeysAssignment_1_4_in_rule__WMLMacroDefine__Alternatives_11498 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyExtraArgs__Alternatives1531 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLKeyExtraArgs__Alternatives1548 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_rule__WMLKeyExtraArgs__Alternatives1565 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacroCallParameter__Alternatives_11597 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLMacroCallParameter__Alternatives_11614 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_rule__WMLMacroCallParameter__Alternatives_11631 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFILE_in_rule__WMLMacroCallParameter__Alternatives_11648 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLINTList_in_rule__WMLKeyValue__Alternatives1680 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLIDList_in_rule__WMLKeyValue__Alternatives1697 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLKeyValue__Alternatives1714 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLKeyValue__Alternatives1731 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_rule__WMLKeyValue__Alternatives1748 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPath_in_rule__WMLKeyValue__Alternatives1765 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFILE_in_rule__WMLKeyValue__Alternatives1782 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__PATH_ID__Alternatives1814 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__PATH_ID__Alternatives1832 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__01864 = new BitSet(new long[]{0x0000000000100010L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__01867 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__WMLTag__Group__0__Impl1895 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__11926 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__11929 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__PlusAssignment_1_in_rule__WMLTag__Group__1__Impl1956 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__21987 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3_in_rule__WMLTag__Group__21990 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__NameAssignment_2_in_rule__WMLTag__Group__2__Impl2017 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__3__Impl_in_rule__WMLTag__Group__32047 = new BitSet(new long[]{0x0000000000034090L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4_in_rule__WMLTag__Group__32050 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLTag__Group__3__Impl2078 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__4__Impl_in_rule__WMLTag__Group__42109 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5_in_rule__WMLTag__Group__42112 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Alternatives_4_in_rule__WMLTag__Group__4__Impl2139 = new BitSet(new long[]{0x0000000000024092L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__5__Impl_in_rule__WMLTag__Group__52170 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6_in_rule__WMLTag__Group__52173 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__WMLTag__Group__5__Impl2201 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__6__Impl_in_rule__WMLTag__Group__62232 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7_in_rule__WMLTag__Group__62235 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__EndNameAssignment_6_in_rule__WMLTag__Group__6__Impl2262 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__7__Impl_in_rule__WMLTag__Group__72292 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLTag__Group__7__Impl2320 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroInclude__Group__0__Impl_in_rule__WMLMacroInclude__Group__02367 = new BitSet(new long[]{0x0000000000802010L});
    public static final BitSet FOLLOW_rule__WMLMacroInclude__Group__1_in_rule__WMLMacroInclude__Group__02370 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_rule__WMLMacroInclude__Group__0__Impl2398 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroInclude__Group__1__Impl_in_rule__WMLMacroInclude__Group__12429 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_rule__WMLMacroInclude__Group__2_in_rule__WMLMacroInclude__Group__12432 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroInclude__NameAssignment_1_in_rule__WMLMacroInclude__Group__1__Impl2459 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroInclude__Group__2__Impl_in_rule__WMLMacroInclude__Group__22489 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_rule__WMLMacroInclude__Group__2__Impl2517 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__0__Impl_in_rule__WMLMacroCall__Group__02554 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1_in_rule__WMLMacroCall__Group__02557 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_rule__WMLMacroCall__Group__0__Impl2585 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__1__Impl_in_rule__WMLMacroCall__Group__12616 = new BitSet(new long[]{0x00000000042640F0L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2_in_rule__WMLMacroCall__Group__12619 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__NameAssignment_1_in_rule__WMLMacroCall__Group__1__Impl2646 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__2__Impl_in_rule__WMLMacroCall__Group__22676 = new BitSet(new long[]{0x0000000000264090L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3_in_rule__WMLMacroCall__Group__22679 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ArgsAssignment_2_in_rule__WMLMacroCall__Group__2__Impl2706 = new BitSet(new long[]{0x0000000004000072L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__3__Impl_in_rule__WMLMacroCall__Group__32737 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4_in_rule__WMLMacroCall__Group__32740 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Alternatives_3_in_rule__WMLMacroCall__Group__3__Impl2767 = new BitSet(new long[]{0x0000000000224092L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__Group__4__Impl_in_rule__WMLMacroCall__Group__42798 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_rule__WMLMacroCall__Group__4__Impl2826 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__0__Impl_in_rule__WMLMacroDefine__Group__02867 = new BitSet(new long[]{0x0000000000224190L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__1_in_rule__WMLMacroDefine__Group__02870 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_rule__WMLMacroDefine__Group__0__Impl2897 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__1__Impl_in_rule__WMLMacroDefine__Group__12926 = new BitSet(new long[]{0x0000000000000100L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__2_in_rule__WMLMacroDefine__Group__12929 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Alternatives_1_in_rule__WMLMacroDefine__Group__1__Impl2956 = new BitSet(new long[]{0x0000000000224092L});
    public static final BitSet FOLLOW_rule__WMLMacroDefine__Group__2__Impl_in_rule__WMLMacroDefine__Group__22987 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ENDDEFINE_in_rule__WMLMacroDefine__Group__2__Impl3014 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__03049 = new BitSet(new long[]{0x0000000000080000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__03052 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__NameAssignment_0_in_rule__WMLKey__Group__0__Impl3079 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__13109 = new BitSet(new long[]{0x0000000004822230L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__13112 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_rule__WMLKey__Group__1__Impl3140 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__23171 = new BitSet(new long[]{0x0000000000100002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3_in_rule__WMLKey__Group__23174 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ValueAssignment_2_in_rule__WMLKey__Group__2__Impl3201 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__3__Impl_in_rule__WMLKey__Group__33231 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__0_in_rule__WMLKey__Group__3__Impl3258 = new BitSet(new long[]{0x0000000000100002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__0__Impl_in_rule__WMLKey__Group_3__03297 = new BitSet(new long[]{0x0000000004020020L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__1_in_rule__WMLKey__Group_3__03300 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__WMLKey__Group_3__0__Impl3328 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group_3__1__Impl_in_rule__WMLKey__Group_3__13359 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__ExtraArgsAssignment_3_1_in_rule__WMLKey__Group_3__1__Impl3386 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Group__0__Impl_in_rule__WMLMacroCallParameter__Group__03420 = new BitSet(new long[]{0x0000000004002030L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Group__1_in_rule__WMLMacroCallParameter__Group__03423 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__WMLMacroCallParameter__Group__0__Impl3451 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Group__1__Impl_in_rule__WMLMacroCallParameter__Group__13482 = new BitSet(new long[]{0x0000000000400000L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Group__2_in_rule__WMLMacroCallParameter__Group__13485 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Alternatives_1_in_rule__WMLMacroCallParameter__Group__1__Impl3512 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCallParameter__Group__2__Impl_in_rule__WMLMacroCallParameter__Group__23542 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_rule__WMLMacroCallParameter__Group__2__Impl3570 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPath__Group__0__Impl_in_rule__WMLPath__Group__03607 = new BitSet(new long[]{0x0000000000002010L});
    public static final BitSet FOLLOW_rule__WMLPath__Group__1_in_rule__WMLPath__Group__03610 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_rule__WMLPath__Group__0__Impl3639 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPath__Group__1__Impl_in_rule__WMLPath__Group__13672 = new BitSet(new long[]{0x0000000001000000L});
    public static final BitSet FOLLOW_rule__WMLPath__Group__2_in_rule__WMLPath__Group__13675 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_ID_in_rule__WMLPath__Group__1__Impl3702 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPath__Group__2__Impl_in_rule__WMLPath__Group__23731 = new BitSet(new long[]{0x0000000001002012L});
    public static final BitSet FOLLOW_rule__WMLPath__Group__3_in_rule__WMLPath__Group__23734 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPath__Group_2__0_in_rule__WMLPath__Group__2__Impl3763 = new BitSet(new long[]{0x0000000001000002L});
    public static final BitSet FOLLOW_rule__WMLPath__Group_2__0_in_rule__WMLPath__Group__2__Impl3775 = new BitSet(new long[]{0x0000000001000002L});
    public static final BitSet FOLLOW_rule__WMLPath__Group__3__Impl_in_rule__WMLPath__Group__33808 = new BitSet(new long[]{0x0000000001000002L});
    public static final BitSet FOLLOW_rule__WMLPath__Group__4_in_rule__WMLPath__Group__33811 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFILE_in_rule__WMLPath__Group__3__Impl3839 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPath__Group__4__Impl_in_rule__WMLPath__Group__43870 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_rule__WMLPath__Group__4__Impl3899 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPath__Group_2__0__Impl_in_rule__WMLPath__Group_2__03942 = new BitSet(new long[]{0x0000000000002010L});
    public static final BitSet FOLLOW_rule__WMLPath__Group_2__1_in_rule__WMLPath__Group_2__03945 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_rule__WMLPath__Group_2__0__Impl3973 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLPath__Group_2__1__Impl_in_rule__WMLPath__Group_2__14004 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_ID_in_rule__WMLPath__Group_2__1__Impl4031 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLIDList__Group__0__Impl_in_rule__WMLIDList__Group__04064 = new BitSet(new long[]{0x0000000002000002L});
    public static final BitSet FOLLOW_rule__WMLIDList__Group__1_in_rule__WMLIDList__Group__04067 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLIDList__Group__0__Impl4094 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLIDList__Group__1__Impl_in_rule__WMLIDList__Group__14123 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLIDList__Group_1__0_in_rule__WMLIDList__Group__1__Impl4150 = new BitSet(new long[]{0x0000000002000002L});
    public static final BitSet FOLLOW_rule__WMLIDList__Group_1__0__Impl_in_rule__WMLIDList__Group_1__04185 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLIDList__Group_1__1_in_rule__WMLIDList__Group_1__04188 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__WMLIDList__Group_1__0__Impl4216 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLIDList__Group_1__1__Impl_in_rule__WMLIDList__Group_1__14247 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLIDList__Group_1__1__Impl4274 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLINTList__Group__0__Impl_in_rule__WMLINTList__Group__04307 = new BitSet(new long[]{0x0000000002000002L});
    public static final BitSet FOLLOW_rule__WMLINTList__Group__1_in_rule__WMLINTList__Group__04310 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_INT_in_rule__WMLINTList__Group__0__Impl4337 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLINTList__Group__1__Impl_in_rule__WMLINTList__Group__14366 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLINTList__Group_1__0_in_rule__WMLINTList__Group__1__Impl4393 = new BitSet(new long[]{0x0000000002000002L});
    public static final BitSet FOLLOW_rule__WMLINTList__Group_1__0__Impl_in_rule__WMLINTList__Group_1__04428 = new BitSet(new long[]{0x0000000000000200L});
    public static final BitSet FOLLOW_rule__WMLINTList__Group_1__1_in_rule__WMLINTList__Group_1__04431 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_rule__WMLINTList__Group_1__0__Impl4459 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLINTList__Group_1__1__Impl_in_rule__WMLINTList__Group_1__14490 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_INT_in_rule__WMLINTList__Group_1__1__Impl4517 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__04550 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__04553 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_rule__TSTRING__Group__0__Impl4581 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__14612 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl4639 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FILE__Group__0__Impl_in_rule__FILE__Group__04672 = new BitSet(new long[]{0x0000000008000000L});
    public static final BitSet FOLLOW_rule__FILE__Group__1_in_rule__FILE__Group__04675 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_ID_in_rule__FILE__Group__0__Impl4705 = new BitSet(new long[]{0x0000000000002012L});
    public static final BitSet FOLLOW_rulePATH_ID_in_rule__FILE__Group__0__Impl4718 = new BitSet(new long[]{0x0000000000002012L});
    public static final BitSet FOLLOW_rule__FILE__Group__1__Impl_in_rule__FILE__Group__14751 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__FILE__Group__2_in_rule__FILE__Group__14754 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rule__FILE__Group__1__Impl4782 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FILE__Group__2__Impl_in_rule__FILE__Group__24813 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__FILE__Group__2__Impl4840 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRoot__TagsAssignment_04880 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLAbstractMacroCall_in_rule__WMLRoot__MacrosAssignment_14911 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLRoot__MacrosDefinesAssignment_24942 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__WMLTag__PlusAssignment_14978 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__NameAssignment_25017 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLTag__TagsAssignment_4_05048 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLAbstractMacroCall_in_rule__WMLTag__MacrosAssignment_4_15079 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLTag__MacrosDefinesAssignment_4_25110 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLTag__KeysAssignment_4_35141 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLTag__EndNameAssignment_65172 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPath_in_rule__WMLMacroInclude__NameAssignment_15203 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacroCall__NameAssignment_15234 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacroCall__ArgsAlternatives_2_0_in_rule__WMLMacroCall__ArgsAssignment_25265 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_rule__WMLMacroCall__ParamsAssignment_3_05298 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLMacroCall__TagsAssignment_3_15329 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroCall__MacrosAssignment_3_25360 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLMacroCall__MacrosDefinesAssignment_3_35391 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLMacroCall__KeysAssignment_3_45422 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_rule__WMLMacroDefine__ParamsAssignment_1_05453 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLMacroDefine__TagsAssignment_1_15484 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_rule__WMLMacroDefine__MacrosAssignment_1_25515 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_rule__WMLMacroDefine__MacrosDefinesAssignment_1_35546 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLMacroDefine__KeysAssignment_1_45577 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLIDList_in_rule__WMLKey__NameAssignment_05609 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_rule__WMLKey__ValueAssignment_25640 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyExtraArgs_in_rule__WMLKey__ExtraArgsAssignment_3_15671 = new BitSet(new long[]{0x0000000000000002L});

}