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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_STRING", "RULE_INT", "RULE_SL_COMMENT", "RULE_WS", "RULE_IDENH", "'{'", "'}'", "'['", "']'", "'[/'", "'='", "'.'"
    };
    public static final int RULE_ID=4;
    public static final int RULE_STRING=5;
    public static final int RULE_IDENH=9;
    public static final int RULE_INT=6;
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

                if ( (LA1_0==10||LA1_0==12) ) {
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


    // $ANTLR start entryRuleWMLStartTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:145:1: entryRuleWMLStartTag : ruleWMLStartTag EOF ;
    public final void entryRuleWMLStartTag() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:146:1: ( ruleWMLStartTag EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:147:1: ruleWMLStartTag EOF
            {
             before(grammarAccess.getWMLStartTagRule()); 
            pushFollow(FOLLOW_ruleWMLStartTag_in_entryRuleWMLStartTag242);
            ruleWMLStartTag();
            _fsp--;

             after(grammarAccess.getWMLStartTagRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLStartTag249); 

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
    // $ANTLR end entryRuleWMLStartTag


    // $ANTLR start ruleWMLStartTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:154:1: ruleWMLStartTag : ( ( rule__WMLStartTag__Group__0 ) ) ;
    public final void ruleWMLStartTag() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:158:2: ( ( ( rule__WMLStartTag__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:159:1: ( ( rule__WMLStartTag__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:159:1: ( ( rule__WMLStartTag__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:160:1: ( rule__WMLStartTag__Group__0 )
            {
             before(grammarAccess.getWMLStartTagAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:161:1: ( rule__WMLStartTag__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:161:2: rule__WMLStartTag__Group__0
            {
            pushFollow(FOLLOW_rule__WMLStartTag__Group__0_in_ruleWMLStartTag275);
            rule__WMLStartTag__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLStartTagAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLStartTag


    // $ANTLR start entryRuleWMLEndTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:173:1: entryRuleWMLEndTag : ruleWMLEndTag EOF ;
    public final void entryRuleWMLEndTag() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:174:1: ( ruleWMLEndTag EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:175:1: ruleWMLEndTag EOF
            {
             before(grammarAccess.getWMLEndTagRule()); 
            pushFollow(FOLLOW_ruleWMLEndTag_in_entryRuleWMLEndTag302);
            ruleWMLEndTag();
            _fsp--;

             after(grammarAccess.getWMLEndTagRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLEndTag309); 

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
    // $ANTLR end entryRuleWMLEndTag


    // $ANTLR start ruleWMLEndTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:182:1: ruleWMLEndTag : ( ( rule__WMLEndTag__Group__0 ) ) ;
    public final void ruleWMLEndTag() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:186:2: ( ( ( rule__WMLEndTag__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:187:1: ( ( rule__WMLEndTag__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:187:1: ( ( rule__WMLEndTag__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:188:1: ( rule__WMLEndTag__Group__0 )
            {
             before(grammarAccess.getWMLEndTagAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:189:1: ( rule__WMLEndTag__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:189:2: rule__WMLEndTag__Group__0
            {
            pushFollow(FOLLOW_rule__WMLEndTag__Group__0_in_ruleWMLEndTag335);
            rule__WMLEndTag__Group__0();
            _fsp--;


            }

             after(grammarAccess.getWMLEndTagAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleWMLEndTag


    // $ANTLR start entryRuleWMLKey
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:201:1: entryRuleWMLKey : ruleWMLKey EOF ;
    public final void entryRuleWMLKey() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:202:1: ( ruleWMLKey EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:203:1: ruleWMLKey EOF
            {
             before(grammarAccess.getWMLKeyRule()); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey362);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLKeyRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey369); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:210:1: ruleWMLKey : ( ( rule__WMLKey__Group__0 ) ) ;
    public final void ruleWMLKey() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:214:2: ( ( ( rule__WMLKey__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:215:1: ( ( rule__WMLKey__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:215:1: ( ( rule__WMLKey__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:216:1: ( rule__WMLKey__Group__0 )
            {
             before(grammarAccess.getWMLKeyAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:217:1: ( rule__WMLKey__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:217:2: rule__WMLKey__Group__0
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey395);
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


    // $ANTLR start entryRuleFLOAT
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:229:1: entryRuleFLOAT : ruleFLOAT EOF ;
    public final void entryRuleFLOAT() throws RecognitionException {

        	HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();

        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:233:1: ( ruleFLOAT EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:234:1: ruleFLOAT EOF
            {
             before(grammarAccess.getFLOATRule()); 
            pushFollow(FOLLOW_ruleFLOAT_in_entryRuleFLOAT427);
            ruleFLOAT();
            _fsp--;

             after(grammarAccess.getFLOATRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleFLOAT434); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:244:1: ruleFLOAT : ( ( rule__FLOAT__Group__0 ) ) ;
    public final void ruleFLOAT() throws RecognitionException {

        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();
        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:249:2: ( ( ( rule__FLOAT__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:250:1: ( ( rule__FLOAT__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:250:1: ( ( rule__FLOAT__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:251:1: ( rule__FLOAT__Group__0 )
            {
             before(grammarAccess.getFLOATAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:252:1: ( rule__FLOAT__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:252:2: rule__FLOAT__Group__0
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__0_in_ruleFLOAT464);
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


    // $ANTLR start rule__WMLRoot__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:265:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__RtagsAssignment_0 ) ) | ( ( rule__WMLRoot__RmacrosAssignment_1 ) ) );
    public final void rule__WMLRoot__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:269:1: ( ( ( rule__WMLRoot__RtagsAssignment_0 ) ) | ( ( rule__WMLRoot__RmacrosAssignment_1 ) ) )
            int alt2=2;
            int LA2_0 = input.LA(1);

            if ( (LA2_0==12) ) {
                alt2=1;
            }
            else if ( (LA2_0==10) ) {
                alt2=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("265:1: rule__WMLRoot__Alternatives : ( ( ( rule__WMLRoot__RtagsAssignment_0 ) ) | ( ( rule__WMLRoot__RmacrosAssignment_1 ) ) );", 2, 0, input);

                throw nvae;
            }
            switch (alt2) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:270:1: ( ( rule__WMLRoot__RtagsAssignment_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:270:1: ( ( rule__WMLRoot__RtagsAssignment_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:271:1: ( rule__WMLRoot__RtagsAssignment_0 )
                    {
                     before(grammarAccess.getWMLRootAccess().getRtagsAssignment_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:272:1: ( rule__WMLRoot__RtagsAssignment_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:272:2: rule__WMLRoot__RtagsAssignment_0
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__RtagsAssignment_0_in_rule__WMLRoot__Alternatives500);
                    rule__WMLRoot__RtagsAssignment_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLRootAccess().getRtagsAssignment_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:276:6: ( ( rule__WMLRoot__RmacrosAssignment_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:276:6: ( ( rule__WMLRoot__RmacrosAssignment_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:277:1: ( rule__WMLRoot__RmacrosAssignment_1 )
                    {
                     before(grammarAccess.getWMLRootAccess().getRmacrosAssignment_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:278:1: ( rule__WMLRoot__RmacrosAssignment_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:278:2: rule__WMLRoot__RmacrosAssignment_1
                    {
                    pushFollow(FOLLOW_rule__WMLRoot__RmacrosAssignment_1_in_rule__WMLRoot__Alternatives518);
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


    // $ANTLR start rule__WMLTag__Alternatives_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:287:1: rule__WMLTag__Alternatives_1 : ( ( ( rule__WMLTag__TtagsAssignment_1_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_1_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_1_2 ) ) );
    public final void rule__WMLTag__Alternatives_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:291:1: ( ( ( rule__WMLTag__TtagsAssignment_1_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_1_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_1_2 ) ) )
            int alt3=3;
            switch ( input.LA(1) ) {
            case 12:
                {
                alt3=1;
                }
                break;
            case RULE_ID:
                {
                alt3=2;
                }
                break;
            case 10:
                {
                alt3=3;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("287:1: rule__WMLTag__Alternatives_1 : ( ( ( rule__WMLTag__TtagsAssignment_1_0 ) ) | ( ( rule__WMLTag__TkeysAssignment_1_1 ) ) | ( ( rule__WMLTag__TmacrosAssignment_1_2 ) ) );", 3, 0, input);

                throw nvae;
            }

            switch (alt3) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:292:1: ( ( rule__WMLTag__TtagsAssignment_1_0 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:292:1: ( ( rule__WMLTag__TtagsAssignment_1_0 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:293:1: ( rule__WMLTag__TtagsAssignment_1_0 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTtagsAssignment_1_0()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:294:1: ( rule__WMLTag__TtagsAssignment_1_0 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:294:2: rule__WMLTag__TtagsAssignment_1_0
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TtagsAssignment_1_0_in_rule__WMLTag__Alternatives_1551);
                    rule__WMLTag__TtagsAssignment_1_0();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTtagsAssignment_1_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:298:6: ( ( rule__WMLTag__TkeysAssignment_1_1 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:298:6: ( ( rule__WMLTag__TkeysAssignment_1_1 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:299:1: ( rule__WMLTag__TkeysAssignment_1_1 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTkeysAssignment_1_1()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:300:1: ( rule__WMLTag__TkeysAssignment_1_1 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:300:2: rule__WMLTag__TkeysAssignment_1_1
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TkeysAssignment_1_1_in_rule__WMLTag__Alternatives_1569);
                    rule__WMLTag__TkeysAssignment_1_1();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTkeysAssignment_1_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:304:6: ( ( rule__WMLTag__TmacrosAssignment_1_2 ) )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:304:6: ( ( rule__WMLTag__TmacrosAssignment_1_2 ) )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:305:1: ( rule__WMLTag__TmacrosAssignment_1_2 )
                    {
                     before(grammarAccess.getWMLTagAccess().getTmacrosAssignment_1_2()); 
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:306:1: ( rule__WMLTag__TmacrosAssignment_1_2 )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:306:2: rule__WMLTag__TmacrosAssignment_1_2
                    {
                    pushFollow(FOLLOW_rule__WMLTag__TmacrosAssignment_1_2_in_rule__WMLTag__Alternatives_1587);
                    rule__WMLTag__TmacrosAssignment_1_2();
                    _fsp--;


                    }

                     after(grammarAccess.getWMLTagAccess().getTmacrosAssignment_1_2()); 

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
    // $ANTLR end rule__WMLTag__Alternatives_1


    // $ANTLR start rule__WMLKey__KeyValueAlternatives_2_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:315:1: rule__WMLKey__KeyValueAlternatives_2_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleFLOAT ) );
    public final void rule__WMLKey__KeyValueAlternatives_2_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:319:1: ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleFLOAT ) )
            int alt4=3;
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
            case RULE_INT:
                {
                alt4=3;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("315:1: rule__WMLKey__KeyValueAlternatives_2_0 : ( ( RULE_ID ) | ( RULE_STRING ) | ( ruleFLOAT ) );", 4, 0, input);

                throw nvae;
            }

            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:320:1: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:320:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:321:1: RULE_ID
                    {
                     before(grammarAccess.getWMLKeyAccess().getKeyValueIDTerminalRuleCall_2_0_0()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__KeyValueAlternatives_2_0620); 
                     after(grammarAccess.getWMLKeyAccess().getKeyValueIDTerminalRuleCall_2_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:326:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:326:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:327:1: RULE_STRING
                    {
                     before(grammarAccess.getWMLKeyAccess().getKeyValueSTRINGTerminalRuleCall_2_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__WMLKey__KeyValueAlternatives_2_0637); 
                     after(grammarAccess.getWMLKeyAccess().getKeyValueSTRINGTerminalRuleCall_2_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:332:6: ( ruleFLOAT )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:332:6: ( ruleFLOAT )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:333:1: ruleFLOAT
                    {
                     before(grammarAccess.getWMLKeyAccess().getKeyValueFLOATParserRuleCall_2_0_2()); 
                    pushFollow(FOLLOW_ruleFLOAT_in_rule__WMLKey__KeyValueAlternatives_2_0654);
                    ruleFLOAT();
                    _fsp--;

                     after(grammarAccess.getWMLKeyAccess().getKeyValueFLOATParserRuleCall_2_0_2()); 

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
    // $ANTLR end rule__WMLKey__KeyValueAlternatives_2_0


    // $ANTLR start rule__WMLMacro__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:345:1: rule__WMLMacro__Group__0 : rule__WMLMacro__Group__0__Impl rule__WMLMacro__Group__1 ;
    public final void rule__WMLMacro__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:349:1: ( rule__WMLMacro__Group__0__Impl rule__WMLMacro__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:350:2: rule__WMLMacro__Group__0__Impl rule__WMLMacro__Group__1
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__0__Impl_in_rule__WMLMacro__Group__0684);
            rule__WMLMacro__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacro__Group__1_in_rule__WMLMacro__Group__0687);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:357:1: rule__WMLMacro__Group__0__Impl : ( '{' ) ;
    public final void rule__WMLMacro__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:361:1: ( ( '{' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:362:1: ( '{' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:362:1: ( '{' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:363:1: '{'
            {
             before(grammarAccess.getWMLMacroAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,10,FOLLOW_10_in_rule__WMLMacro__Group__0__Impl715); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:376:1: rule__WMLMacro__Group__1 : rule__WMLMacro__Group__1__Impl rule__WMLMacro__Group__2 ;
    public final void rule__WMLMacro__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:380:1: ( rule__WMLMacro__Group__1__Impl rule__WMLMacro__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:381:2: rule__WMLMacro__Group__1__Impl rule__WMLMacro__Group__2
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__1__Impl_in_rule__WMLMacro__Group__1746);
            rule__WMLMacro__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLMacro__Group__2_in_rule__WMLMacro__Group__1749);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:388:1: rule__WMLMacro__Group__1__Impl : ( ( ( rule__WMLMacro__TagcontentAssignment_1 ) ) ( ( rule__WMLMacro__TagcontentAssignment_1 )* ) ) ;
    public final void rule__WMLMacro__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:392:1: ( ( ( ( rule__WMLMacro__TagcontentAssignment_1 ) ) ( ( rule__WMLMacro__TagcontentAssignment_1 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:393:1: ( ( ( rule__WMLMacro__TagcontentAssignment_1 ) ) ( ( rule__WMLMacro__TagcontentAssignment_1 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:393:1: ( ( ( rule__WMLMacro__TagcontentAssignment_1 ) ) ( ( rule__WMLMacro__TagcontentAssignment_1 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:394:1: ( ( rule__WMLMacro__TagcontentAssignment_1 ) ) ( ( rule__WMLMacro__TagcontentAssignment_1 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:394:1: ( ( rule__WMLMacro__TagcontentAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:395:1: ( rule__WMLMacro__TagcontentAssignment_1 )
            {
             before(grammarAccess.getWMLMacroAccess().getTagcontentAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:396:1: ( rule__WMLMacro__TagcontentAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:396:2: rule__WMLMacro__TagcontentAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLMacro__TagcontentAssignment_1_in_rule__WMLMacro__Group__1__Impl778);
            rule__WMLMacro__TagcontentAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLMacroAccess().getTagcontentAssignment_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:399:1: ( ( rule__WMLMacro__TagcontentAssignment_1 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:400:1: ( rule__WMLMacro__TagcontentAssignment_1 )*
            {
             before(grammarAccess.getWMLMacroAccess().getTagcontentAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:401:1: ( rule__WMLMacro__TagcontentAssignment_1 )*
            loop5:
            do {
                int alt5=2;
                int LA5_0 = input.LA(1);

                if ( (LA5_0==RULE_ID) ) {
                    alt5=1;
                }


                switch (alt5) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:401:2: rule__WMLMacro__TagcontentAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLMacro__TagcontentAssignment_1_in_rule__WMLMacro__Group__1__Impl790);
            	    rule__WMLMacro__TagcontentAssignment_1();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop5;
                }
            } while (true);

             after(grammarAccess.getWMLMacroAccess().getTagcontentAssignment_1()); 

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
    // $ANTLR end rule__WMLMacro__Group__1__Impl


    // $ANTLR start rule__WMLMacro__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:412:1: rule__WMLMacro__Group__2 : rule__WMLMacro__Group__2__Impl ;
    public final void rule__WMLMacro__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:416:1: ( rule__WMLMacro__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:417:2: rule__WMLMacro__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLMacro__Group__2__Impl_in_rule__WMLMacro__Group__2823);
            rule__WMLMacro__Group__2__Impl();
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:423:1: rule__WMLMacro__Group__2__Impl : ( '}' ) ;
    public final void rule__WMLMacro__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:427:1: ( ( '}' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:428:1: ( '}' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:428:1: ( '}' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:429:1: '}'
            {
             before(grammarAccess.getWMLMacroAccess().getRightCurlyBracketKeyword_2()); 
            match(input,11,FOLLOW_11_in_rule__WMLMacro__Group__2__Impl851); 
             after(grammarAccess.getWMLMacroAccess().getRightCurlyBracketKeyword_2()); 

            }


            }

        }
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


    // $ANTLR start rule__WMLTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:448:1: rule__WMLTag__Group__0 : rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 ;
    public final void rule__WMLTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:452:1: ( rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:453:2: rule__WMLTag__Group__0__Impl rule__WMLTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__0888);
            rule__WMLTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__0891);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:460:1: rule__WMLTag__Group__0__Impl : ( ( rule__WMLTag__StartAssignment_0 ) ) ;
    public final void rule__WMLTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:464:1: ( ( ( rule__WMLTag__StartAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:465:1: ( ( rule__WMLTag__StartAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:465:1: ( ( rule__WMLTag__StartAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:466:1: ( rule__WMLTag__StartAssignment_0 )
            {
             before(grammarAccess.getWMLTagAccess().getStartAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:467:1: ( rule__WMLTag__StartAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:467:2: rule__WMLTag__StartAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLTag__StartAssignment_0_in_rule__WMLTag__Group__0__Impl918);
            rule__WMLTag__StartAssignment_0();
            _fsp--;


            }

             after(grammarAccess.getWMLTagAccess().getStartAssignment_0()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:477:1: rule__WMLTag__Group__1 : rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 ;
    public final void rule__WMLTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:481:1: ( rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:482:2: rule__WMLTag__Group__1__Impl rule__WMLTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__1948);
            rule__WMLTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__1951);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:489:1: rule__WMLTag__Group__1__Impl : ( ( rule__WMLTag__Alternatives_1 )* ) ;
    public final void rule__WMLTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:493:1: ( ( ( rule__WMLTag__Alternatives_1 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:494:1: ( ( rule__WMLTag__Alternatives_1 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:494:1: ( ( rule__WMLTag__Alternatives_1 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:495:1: ( rule__WMLTag__Alternatives_1 )*
            {
             before(grammarAccess.getWMLTagAccess().getAlternatives_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:496:1: ( rule__WMLTag__Alternatives_1 )*
            loop6:
            do {
                int alt6=2;
                int LA6_0 = input.LA(1);

                if ( (LA6_0==RULE_ID||LA6_0==10||LA6_0==12) ) {
                    alt6=1;
                }


                switch (alt6) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:496:2: rule__WMLTag__Alternatives_1
            	    {
            	    pushFollow(FOLLOW_rule__WMLTag__Alternatives_1_in_rule__WMLTag__Group__1__Impl978);
            	    rule__WMLTag__Alternatives_1();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop6;
                }
            } while (true);

             after(grammarAccess.getWMLTagAccess().getAlternatives_1()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:506:1: rule__WMLTag__Group__2 : rule__WMLTag__Group__2__Impl ;
    public final void rule__WMLTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:510:1: ( rule__WMLTag__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:511:2: rule__WMLTag__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__21009);
            rule__WMLTag__Group__2__Impl();
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:517:1: rule__WMLTag__Group__2__Impl : ( ( rule__WMLTag__EndAssignment_2 ) ) ;
    public final void rule__WMLTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:521:1: ( ( ( rule__WMLTag__EndAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:522:1: ( ( rule__WMLTag__EndAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:522:1: ( ( rule__WMLTag__EndAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:523:1: ( rule__WMLTag__EndAssignment_2 )
            {
             before(grammarAccess.getWMLTagAccess().getEndAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:524:1: ( rule__WMLTag__EndAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:524:2: rule__WMLTag__EndAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLTag__EndAssignment_2_in_rule__WMLTag__Group__2__Impl1036);
            rule__WMLTag__EndAssignment_2();
            _fsp--;


            }

             after(grammarAccess.getWMLTagAccess().getEndAssignment_2()); 

            }


            }

        }
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


    // $ANTLR start rule__WMLStartTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:540:1: rule__WMLStartTag__Group__0 : rule__WMLStartTag__Group__0__Impl rule__WMLStartTag__Group__1 ;
    public final void rule__WMLStartTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:544:1: ( rule__WMLStartTag__Group__0__Impl rule__WMLStartTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:545:2: rule__WMLStartTag__Group__0__Impl rule__WMLStartTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLStartTag__Group__0__Impl_in_rule__WMLStartTag__Group__01072);
            rule__WMLStartTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLStartTag__Group__1_in_rule__WMLStartTag__Group__01075);
            rule__WMLStartTag__Group__1();
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
    // $ANTLR end rule__WMLStartTag__Group__0


    // $ANTLR start rule__WMLStartTag__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:552:1: rule__WMLStartTag__Group__0__Impl : ( '[' ) ;
    public final void rule__WMLStartTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:556:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:557:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:557:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:558:1: '['
            {
             before(grammarAccess.getWMLStartTagAccess().getLeftSquareBracketKeyword_0()); 
            match(input,12,FOLLOW_12_in_rule__WMLStartTag__Group__0__Impl1103); 
             after(grammarAccess.getWMLStartTagAccess().getLeftSquareBracketKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLStartTag__Group__0__Impl


    // $ANTLR start rule__WMLStartTag__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:571:1: rule__WMLStartTag__Group__1 : rule__WMLStartTag__Group__1__Impl rule__WMLStartTag__Group__2 ;
    public final void rule__WMLStartTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:575:1: ( rule__WMLStartTag__Group__1__Impl rule__WMLStartTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:576:2: rule__WMLStartTag__Group__1__Impl rule__WMLStartTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLStartTag__Group__1__Impl_in_rule__WMLStartTag__Group__11134);
            rule__WMLStartTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLStartTag__Group__2_in_rule__WMLStartTag__Group__11137);
            rule__WMLStartTag__Group__2();
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
    // $ANTLR end rule__WMLStartTag__Group__1


    // $ANTLR start rule__WMLStartTag__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:583:1: rule__WMLStartTag__Group__1__Impl : ( ( rule__WMLStartTag__TagnameAssignment_1 ) ) ;
    public final void rule__WMLStartTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:587:1: ( ( ( rule__WMLStartTag__TagnameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:588:1: ( ( rule__WMLStartTag__TagnameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:588:1: ( ( rule__WMLStartTag__TagnameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:589:1: ( rule__WMLStartTag__TagnameAssignment_1 )
            {
             before(grammarAccess.getWMLStartTagAccess().getTagnameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:590:1: ( rule__WMLStartTag__TagnameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:590:2: rule__WMLStartTag__TagnameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLStartTag__TagnameAssignment_1_in_rule__WMLStartTag__Group__1__Impl1164);
            rule__WMLStartTag__TagnameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLStartTagAccess().getTagnameAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLStartTag__Group__1__Impl


    // $ANTLR start rule__WMLStartTag__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:600:1: rule__WMLStartTag__Group__2 : rule__WMLStartTag__Group__2__Impl ;
    public final void rule__WMLStartTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:604:1: ( rule__WMLStartTag__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:605:2: rule__WMLStartTag__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLStartTag__Group__2__Impl_in_rule__WMLStartTag__Group__21194);
            rule__WMLStartTag__Group__2__Impl();
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
    // $ANTLR end rule__WMLStartTag__Group__2


    // $ANTLR start rule__WMLStartTag__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:611:1: rule__WMLStartTag__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLStartTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:615:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:616:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:616:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:617:1: ']'
            {
             before(grammarAccess.getWMLStartTagAccess().getRightSquareBracketKeyword_2()); 
            match(input,13,FOLLOW_13_in_rule__WMLStartTag__Group__2__Impl1222); 
             after(grammarAccess.getWMLStartTagAccess().getRightSquareBracketKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLStartTag__Group__2__Impl


    // $ANTLR start rule__WMLEndTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:636:1: rule__WMLEndTag__Group__0 : rule__WMLEndTag__Group__0__Impl rule__WMLEndTag__Group__1 ;
    public final void rule__WMLEndTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:640:1: ( rule__WMLEndTag__Group__0__Impl rule__WMLEndTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:641:2: rule__WMLEndTag__Group__0__Impl rule__WMLEndTag__Group__1
            {
            pushFollow(FOLLOW_rule__WMLEndTag__Group__0__Impl_in_rule__WMLEndTag__Group__01259);
            rule__WMLEndTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLEndTag__Group__1_in_rule__WMLEndTag__Group__01262);
            rule__WMLEndTag__Group__1();
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
    // $ANTLR end rule__WMLEndTag__Group__0


    // $ANTLR start rule__WMLEndTag__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:648:1: rule__WMLEndTag__Group__0__Impl : ( '[/' ) ;
    public final void rule__WMLEndTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:652:1: ( ( '[/' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:653:1: ( '[/' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:653:1: ( '[/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:654:1: '[/'
            {
             before(grammarAccess.getWMLEndTagAccess().getLeftSquareBracketSolidusKeyword_0()); 
            match(input,14,FOLLOW_14_in_rule__WMLEndTag__Group__0__Impl1290); 
             after(grammarAccess.getWMLEndTagAccess().getLeftSquareBracketSolidusKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLEndTag__Group__0__Impl


    // $ANTLR start rule__WMLEndTag__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:667:1: rule__WMLEndTag__Group__1 : rule__WMLEndTag__Group__1__Impl rule__WMLEndTag__Group__2 ;
    public final void rule__WMLEndTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:671:1: ( rule__WMLEndTag__Group__1__Impl rule__WMLEndTag__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:672:2: rule__WMLEndTag__Group__1__Impl rule__WMLEndTag__Group__2
            {
            pushFollow(FOLLOW_rule__WMLEndTag__Group__1__Impl_in_rule__WMLEndTag__Group__11321);
            rule__WMLEndTag__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLEndTag__Group__2_in_rule__WMLEndTag__Group__11324);
            rule__WMLEndTag__Group__2();
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
    // $ANTLR end rule__WMLEndTag__Group__1


    // $ANTLR start rule__WMLEndTag__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:679:1: rule__WMLEndTag__Group__1__Impl : ( ( rule__WMLEndTag__TagnameAssignment_1 ) ) ;
    public final void rule__WMLEndTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:683:1: ( ( ( rule__WMLEndTag__TagnameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:684:1: ( ( rule__WMLEndTag__TagnameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:684:1: ( ( rule__WMLEndTag__TagnameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:685:1: ( rule__WMLEndTag__TagnameAssignment_1 )
            {
             before(grammarAccess.getWMLEndTagAccess().getTagnameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:686:1: ( rule__WMLEndTag__TagnameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:686:2: rule__WMLEndTag__TagnameAssignment_1
            {
            pushFollow(FOLLOW_rule__WMLEndTag__TagnameAssignment_1_in_rule__WMLEndTag__Group__1__Impl1351);
            rule__WMLEndTag__TagnameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getWMLEndTagAccess().getTagnameAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLEndTag__Group__1__Impl


    // $ANTLR start rule__WMLEndTag__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:696:1: rule__WMLEndTag__Group__2 : rule__WMLEndTag__Group__2__Impl ;
    public final void rule__WMLEndTag__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:700:1: ( rule__WMLEndTag__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:701:2: rule__WMLEndTag__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLEndTag__Group__2__Impl_in_rule__WMLEndTag__Group__21381);
            rule__WMLEndTag__Group__2__Impl();
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
    // $ANTLR end rule__WMLEndTag__Group__2


    // $ANTLR start rule__WMLEndTag__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:707:1: rule__WMLEndTag__Group__2__Impl : ( ']' ) ;
    public final void rule__WMLEndTag__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:711:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:712:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:712:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:713:1: ']'
            {
             before(grammarAccess.getWMLEndTagAccess().getRightSquareBracketKeyword_2()); 
            match(input,13,FOLLOW_13_in_rule__WMLEndTag__Group__2__Impl1409); 
             after(grammarAccess.getWMLEndTagAccess().getRightSquareBracketKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLEndTag__Group__2__Impl


    // $ANTLR start rule__WMLKey__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:732:1: rule__WMLKey__Group__0 : rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 ;
    public final void rule__WMLKey__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:736:1: ( rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:737:2: rule__WMLKey__Group__0__Impl rule__WMLKey__Group__1
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__01446);
            rule__WMLKey__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__01449);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:744:1: rule__WMLKey__Group__0__Impl : ( ( rule__WMLKey__KeyNameAssignment_0 ) ) ;
    public final void rule__WMLKey__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:748:1: ( ( ( rule__WMLKey__KeyNameAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:749:1: ( ( rule__WMLKey__KeyNameAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:749:1: ( ( rule__WMLKey__KeyNameAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:750:1: ( rule__WMLKey__KeyNameAssignment_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getKeyNameAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:751:1: ( rule__WMLKey__KeyNameAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:751:2: rule__WMLKey__KeyNameAssignment_0
            {
            pushFollow(FOLLOW_rule__WMLKey__KeyNameAssignment_0_in_rule__WMLKey__Group__0__Impl1476);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:761:1: rule__WMLKey__Group__1 : rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 ;
    public final void rule__WMLKey__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:765:1: ( rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:766:2: rule__WMLKey__Group__1__Impl rule__WMLKey__Group__2
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__11506);
            rule__WMLKey__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__11509);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:773:1: rule__WMLKey__Group__1__Impl : ( '=' ) ;
    public final void rule__WMLKey__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:777:1: ( ( '=' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:778:1: ( '=' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:778:1: ( '=' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:779:1: '='
            {
             before(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1()); 
            match(input,15,FOLLOW_15_in_rule__WMLKey__Group__1__Impl1537); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:792:1: rule__WMLKey__Group__2 : rule__WMLKey__Group__2__Impl ;
    public final void rule__WMLKey__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:796:1: ( rule__WMLKey__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:797:2: rule__WMLKey__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__21568);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:803:1: rule__WMLKey__Group__2__Impl : ( ( rule__WMLKey__KeyValueAssignment_2 ) ) ;
    public final void rule__WMLKey__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:807:1: ( ( ( rule__WMLKey__KeyValueAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:808:1: ( ( rule__WMLKey__KeyValueAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:808:1: ( ( rule__WMLKey__KeyValueAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:809:1: ( rule__WMLKey__KeyValueAssignment_2 )
            {
             before(grammarAccess.getWMLKeyAccess().getKeyValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:810:1: ( rule__WMLKey__KeyValueAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:810:2: rule__WMLKey__KeyValueAssignment_2
            {
            pushFollow(FOLLOW_rule__WMLKey__KeyValueAssignment_2_in_rule__WMLKey__Group__2__Impl1595);
            rule__WMLKey__KeyValueAssignment_2();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getKeyValueAssignment_2()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:826:1: rule__FLOAT__Group__0 : rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1 ;
    public final void rule__FLOAT__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:830:1: ( rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:831:2: rule__FLOAT__Group__0__Impl rule__FLOAT__Group__1
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__0__Impl_in_rule__FLOAT__Group__01631);
            rule__FLOAT__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__FLOAT__Group__1_in_rule__FLOAT__Group__01634);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:838:1: rule__FLOAT__Group__0__Impl : ( RULE_INT ) ;
    public final void rule__FLOAT__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:842:1: ( ( RULE_INT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:843:1: ( RULE_INT )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:843:1: ( RULE_INT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:844:1: RULE_INT
            {
             before(grammarAccess.getFLOATAccess().getINTTerminalRuleCall_0()); 
            match(input,RULE_INT,FOLLOW_RULE_INT_in_rule__FLOAT__Group__0__Impl1661); 
             after(grammarAccess.getFLOATAccess().getINTTerminalRuleCall_0()); 

            }


            }

        }
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:855:1: rule__FLOAT__Group__1 : rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2 ;
    public final void rule__FLOAT__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:859:1: ( rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:860:2: rule__FLOAT__Group__1__Impl rule__FLOAT__Group__2
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__1__Impl_in_rule__FLOAT__Group__11690);
            rule__FLOAT__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__FLOAT__Group__2_in_rule__FLOAT__Group__11693);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:867:1: rule__FLOAT__Group__1__Impl : ( '.' ) ;
    public final void rule__FLOAT__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:871:1: ( ( '.' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:872:1: ( '.' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:872:1: ( '.' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:873:1: '.'
            {
             before(grammarAccess.getFLOATAccess().getFullStopKeyword_1()); 
            match(input,16,FOLLOW_16_in_rule__FLOAT__Group__1__Impl1721); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:886:1: rule__FLOAT__Group__2 : rule__FLOAT__Group__2__Impl ;
    public final void rule__FLOAT__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:890:1: ( rule__FLOAT__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:891:2: rule__FLOAT__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__FLOAT__Group__2__Impl_in_rule__FLOAT__Group__21752);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:897:1: rule__FLOAT__Group__2__Impl : ( ( ( RULE_INT ) ) ( ( RULE_INT )* ) ) ;
    public final void rule__FLOAT__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:901:1: ( ( ( ( RULE_INT ) ) ( ( RULE_INT )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:902:1: ( ( ( RULE_INT ) ) ( ( RULE_INT )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:902:1: ( ( ( RULE_INT ) ) ( ( RULE_INT )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:903:1: ( ( RULE_INT ) ) ( ( RULE_INT )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:903:1: ( ( RULE_INT ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:904:1: ( RULE_INT )
            {
             before(grammarAccess.getFLOATAccess().getINTTerminalRuleCall_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:905:1: ( RULE_INT )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:905:3: RULE_INT
            {
            match(input,RULE_INT,FOLLOW_RULE_INT_in_rule__FLOAT__Group__2__Impl1782); 

            }

             after(grammarAccess.getFLOATAccess().getINTTerminalRuleCall_2()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:908:1: ( ( RULE_INT )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:909:1: ( RULE_INT )*
            {
             before(grammarAccess.getFLOATAccess().getINTTerminalRuleCall_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:910:1: ( RULE_INT )*
            loop7:
            do {
                int alt7=2;
                int LA7_0 = input.LA(1);

                if ( (LA7_0==RULE_INT) ) {
                    alt7=1;
                }


                switch (alt7) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:910:3: RULE_INT
            	    {
            	    match(input,RULE_INT,FOLLOW_RULE_INT_in_rule__FLOAT__Group__2__Impl1795); 

            	    }
            	    break;

            	default :
            	    break loop7;
                }
            } while (true);

             after(grammarAccess.getFLOATAccess().getINTTerminalRuleCall_2()); 

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


    // $ANTLR start rule__WMLRoot__RtagsAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:928:1: rule__WMLRoot__RtagsAssignment_0 : ( ruleWMLTag ) ;
    public final void rule__WMLRoot__RtagsAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:932:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:933:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:933:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:934:1: ruleWMLTag
            {
             before(grammarAccess.getWMLRootAccess().getRtagsWMLTagParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLRoot__RtagsAssignment_01839);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:943:1: rule__WMLRoot__RmacrosAssignment_1 : ( ruleWMLMacro ) ;
    public final void rule__WMLRoot__RmacrosAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:947:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:948:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:948:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:949:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLRootAccess().getRmacrosWMLMacroParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLRoot__RmacrosAssignment_11870);
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


    // $ANTLR start rule__WMLMacro__TagcontentAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:958:1: rule__WMLMacro__TagcontentAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLMacro__TagcontentAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:962:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:963:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:963:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:964:1: RULE_ID
            {
             before(grammarAccess.getWMLMacroAccess().getTagcontentIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLMacro__TagcontentAssignment_11901); 
             after(grammarAccess.getWMLMacroAccess().getTagcontentIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLMacro__TagcontentAssignment_1


    // $ANTLR start rule__WMLTag__StartAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:973:1: rule__WMLTag__StartAssignment_0 : ( ruleWMLStartTag ) ;
    public final void rule__WMLTag__StartAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:977:1: ( ( ruleWMLStartTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:978:1: ( ruleWMLStartTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:978:1: ( ruleWMLStartTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:979:1: ruleWMLStartTag
            {
             before(grammarAccess.getWMLTagAccess().getStartWMLStartTagParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleWMLStartTag_in_rule__WMLTag__StartAssignment_01932);
            ruleWMLStartTag();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getStartWMLStartTagParserRuleCall_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__StartAssignment_0


    // $ANTLR start rule__WMLTag__TtagsAssignment_1_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:988:1: rule__WMLTag__TtagsAssignment_1_0 : ( ruleWMLTag ) ;
    public final void rule__WMLTag__TtagsAssignment_1_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:992:1: ( ( ruleWMLTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:993:1: ( ruleWMLTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:993:1: ( ruleWMLTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:994:1: ruleWMLTag
            {
             before(grammarAccess.getWMLTagAccess().getTtagsWMLTagParserRuleCall_1_0_0()); 
            pushFollow(FOLLOW_ruleWMLTag_in_rule__WMLTag__TtagsAssignment_1_01963);
            ruleWMLTag();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getTtagsWMLTagParserRuleCall_1_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__TtagsAssignment_1_0


    // $ANTLR start rule__WMLTag__TkeysAssignment_1_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1003:1: rule__WMLTag__TkeysAssignment_1_1 : ( ruleWMLKey ) ;
    public final void rule__WMLTag__TkeysAssignment_1_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1007:1: ( ( ruleWMLKey ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1008:1: ( ruleWMLKey )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1008:1: ( ruleWMLKey )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1009:1: ruleWMLKey
            {
             before(grammarAccess.getWMLTagAccess().getTkeysWMLKeyParserRuleCall_1_1_0()); 
            pushFollow(FOLLOW_ruleWMLKey_in_rule__WMLTag__TkeysAssignment_1_11994);
            ruleWMLKey();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getTkeysWMLKeyParserRuleCall_1_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__TkeysAssignment_1_1


    // $ANTLR start rule__WMLTag__TmacrosAssignment_1_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1018:1: rule__WMLTag__TmacrosAssignment_1_2 : ( ruleWMLMacro ) ;
    public final void rule__WMLTag__TmacrosAssignment_1_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1022:1: ( ( ruleWMLMacro ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1023:1: ( ruleWMLMacro )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1023:1: ( ruleWMLMacro )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1024:1: ruleWMLMacro
            {
             before(grammarAccess.getWMLTagAccess().getTmacrosWMLMacroParserRuleCall_1_2_0()); 
            pushFollow(FOLLOW_ruleWMLMacro_in_rule__WMLTag__TmacrosAssignment_1_22025);
            ruleWMLMacro();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getTmacrosWMLMacroParserRuleCall_1_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__TmacrosAssignment_1_2


    // $ANTLR start rule__WMLTag__EndAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1033:1: rule__WMLTag__EndAssignment_2 : ( ruleWMLEndTag ) ;
    public final void rule__WMLTag__EndAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1037:1: ( ( ruleWMLEndTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1038:1: ( ruleWMLEndTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1038:1: ( ruleWMLEndTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1039:1: ruleWMLEndTag
            {
             before(grammarAccess.getWMLTagAccess().getEndWMLEndTagParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleWMLEndTag_in_rule__WMLTag__EndAssignment_22056);
            ruleWMLEndTag();
            _fsp--;

             after(grammarAccess.getWMLTagAccess().getEndWMLEndTagParserRuleCall_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLTag__EndAssignment_2


    // $ANTLR start rule__WMLStartTag__TagnameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1048:1: rule__WMLStartTag__TagnameAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLStartTag__TagnameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1052:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1053:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1053:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1054:1: RULE_ID
            {
             before(grammarAccess.getWMLStartTagAccess().getTagnameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLStartTag__TagnameAssignment_12087); 
             after(grammarAccess.getWMLStartTagAccess().getTagnameIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLStartTag__TagnameAssignment_1


    // $ANTLR start rule__WMLEndTag__TagnameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1063:1: rule__WMLEndTag__TagnameAssignment_1 : ( RULE_ID ) ;
    public final void rule__WMLEndTag__TagnameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1067:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1068:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1068:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1069:1: RULE_ID
            {
             before(grammarAccess.getWMLEndTagAccess().getTagnameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLEndTag__TagnameAssignment_12118); 
             after(grammarAccess.getWMLEndTagAccess().getTagnameIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLEndTag__TagnameAssignment_1


    // $ANTLR start rule__WMLKey__KeyNameAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1078:1: rule__WMLKey__KeyNameAssignment_0 : ( RULE_ID ) ;
    public final void rule__WMLKey__KeyNameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1082:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1083:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1083:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1084:1: RULE_ID
            {
             before(grammarAccess.getWMLKeyAccess().getKeyNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__WMLKey__KeyNameAssignment_02149); 
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


    // $ANTLR start rule__WMLKey__KeyValueAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1093:1: rule__WMLKey__KeyValueAssignment_2 : ( ( rule__WMLKey__KeyValueAlternatives_2_0 ) ) ;
    public final void rule__WMLKey__KeyValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1097:1: ( ( ( rule__WMLKey__KeyValueAlternatives_2_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1098:1: ( ( rule__WMLKey__KeyValueAlternatives_2_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1098:1: ( ( rule__WMLKey__KeyValueAlternatives_2_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1099:1: ( rule__WMLKey__KeyValueAlternatives_2_0 )
            {
             before(grammarAccess.getWMLKeyAccess().getKeyValueAlternatives_2_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1100:1: ( rule__WMLKey__KeyValueAlternatives_2_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1100:2: rule__WMLKey__KeyValueAlternatives_2_0
            {
            pushFollow(FOLLOW_rule__WMLKey__KeyValueAlternatives_2_0_in_rule__WMLKey__KeyValueAssignment_22180);
            rule__WMLKey__KeyValueAlternatives_2_0();
            _fsp--;


            }

             after(grammarAccess.getWMLKeyAccess().getKeyValueAlternatives_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__WMLKey__KeyValueAssignment_2


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot61 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot68 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__Alternatives_in_ruleWMLRoot94 = new BitSet(new long[]{0x0000000000001402L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_entryRuleWMLMacro122 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacro129 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__0_in_ruleWMLMacro155 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag182 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag189 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0_in_ruleWMLTag215 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLStartTag_in_entryRuleWMLStartTag242 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLStartTag249 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLStartTag__Group__0_in_ruleWMLStartTag275 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLEndTag_in_entryRuleWMLEndTag302 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLEndTag309 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__0_in_ruleWMLEndTag335 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey362 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey369 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0_in_ruleWMLKey395 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_entryRuleFLOAT427 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleFLOAT434 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__0_in_ruleFLOAT464 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__RtagsAssignment_0_in_rule__WMLRoot__Alternatives500 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLRoot__RmacrosAssignment_1_in_rule__WMLRoot__Alternatives518 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TtagsAssignment_1_0_in_rule__WMLTag__Alternatives_1551 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TkeysAssignment_1_1_in_rule__WMLTag__Alternatives_1569 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__TmacrosAssignment_1_2_in_rule__WMLTag__Alternatives_1587 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__KeyValueAlternatives_2_0620 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__WMLKey__KeyValueAlternatives_2_0637 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rule__WMLKey__KeyValueAlternatives_2_0654 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__0__Impl_in_rule__WMLMacro__Group__0684 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__1_in_rule__WMLMacro__Group__0687 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_rule__WMLMacro__Group__0__Impl715 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__1__Impl_in_rule__WMLMacro__Group__1746 = new BitSet(new long[]{0x0000000000000800L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__2_in_rule__WMLMacro__Group__1749 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLMacro__TagcontentAssignment_1_in_rule__WMLMacro__Group__1__Impl778 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__WMLMacro__TagcontentAssignment_1_in_rule__WMLMacro__Group__1__Impl790 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__WMLMacro__Group__2__Impl_in_rule__WMLMacro__Group__2823 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_rule__WMLMacro__Group__2__Impl851 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__0__Impl_in_rule__WMLTag__Group__0888 = new BitSet(new long[]{0x0000000000005410L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1_in_rule__WMLTag__Group__0891 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__StartAssignment_0_in_rule__WMLTag__Group__0__Impl918 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__1__Impl_in_rule__WMLTag__Group__1948 = new BitSet(new long[]{0x0000000000004000L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2_in_rule__WMLTag__Group__1951 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__Alternatives_1_in_rule__WMLTag__Group__1__Impl978 = new BitSet(new long[]{0x0000000000001412L});
    public static final BitSet FOLLOW_rule__WMLTag__Group__2__Impl_in_rule__WMLTag__Group__21009 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLTag__EndAssignment_2_in_rule__WMLTag__Group__2__Impl1036 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLStartTag__Group__0__Impl_in_rule__WMLStartTag__Group__01072 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLStartTag__Group__1_in_rule__WMLStartTag__Group__01075 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__WMLStartTag__Group__0__Impl1103 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLStartTag__Group__1__Impl_in_rule__WMLStartTag__Group__11134 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_rule__WMLStartTag__Group__2_in_rule__WMLStartTag__Group__11137 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLStartTag__TagnameAssignment_1_in_rule__WMLStartTag__Group__1__Impl1164 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLStartTag__Group__2__Impl_in_rule__WMLStartTag__Group__21194 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__WMLStartTag__Group__2__Impl1222 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__0__Impl_in_rule__WMLEndTag__Group__01259 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__1_in_rule__WMLEndTag__Group__01262 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__WMLEndTag__Group__0__Impl1290 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__1__Impl_in_rule__WMLEndTag__Group__11321 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__2_in_rule__WMLEndTag__Group__11324 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__TagnameAssignment_1_in_rule__WMLEndTag__Group__1__Impl1351 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLEndTag__Group__2__Impl_in_rule__WMLEndTag__Group__21381 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__WMLEndTag__Group__2__Impl1409 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__0__Impl_in_rule__WMLKey__Group__01446 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1_in_rule__WMLKey__Group__01449 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__KeyNameAssignment_0_in_rule__WMLKey__Group__0__Impl1476 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__1__Impl_in_rule__WMLKey__Group__11506 = new BitSet(new long[]{0x0000000000000070L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2_in_rule__WMLKey__Group__11509 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__WMLKey__Group__1__Impl1537 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__Group__2__Impl_in_rule__WMLKey__Group__21568 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__KeyValueAssignment_2_in_rule__WMLKey__Group__2__Impl1595 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__0__Impl_in_rule__FLOAT__Group__01631 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__1_in_rule__FLOAT__Group__01634 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_INT_in_rule__FLOAT__Group__0__Impl1661 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__1__Impl_in_rule__FLOAT__Group__11690 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__2_in_rule__FLOAT__Group__11693 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__FLOAT__Group__1__Impl1721 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__FLOAT__Group__2__Impl_in_rule__FLOAT__Group__21752 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_INT_in_rule__FLOAT__Group__2__Impl1782 = new BitSet(new long[]{0x0000000000000042L});
    public static final BitSet FOLLOW_RULE_INT_in_rule__FLOAT__Group__2__Impl1795 = new BitSet(new long[]{0x0000000000000042L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLRoot__RtagsAssignment_01839 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLRoot__RmacrosAssignment_11870 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLMacro__TagcontentAssignment_11901 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLStartTag_in_rule__WMLTag__StartAssignment_01932 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_rule__WMLTag__TtagsAssignment_1_01963 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_rule__WMLTag__TkeysAssignment_1_11994 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_rule__WMLTag__TmacrosAssignment_1_22025 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLEndTag_in_rule__WMLTag__EndAssignment_22056 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLStartTag__TagnameAssignment_12087 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLEndTag__TagnameAssignment_12118 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__WMLKey__KeyNameAssignment_02149 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__WMLKey__KeyValueAlternatives_2_0_in_rule__WMLKey__KeyValueAssignment_22180 = new BitSet(new long[]{0x0000000000000002L});

}