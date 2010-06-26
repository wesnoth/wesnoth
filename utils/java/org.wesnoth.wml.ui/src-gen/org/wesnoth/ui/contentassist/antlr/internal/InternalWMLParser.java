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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_PATH", "RULE_STRING", "RULE_IDLIST", "RULE_SL_COMMENT", "RULE_WS", "RULE_ANY_OTHER", "'#textdomain '", "'{'", "'}'", "'['", "']'", "'/'", "'+'", "'='", "'_'", "'~'"
    };
    public static final int RULE_ID=4;
    public static final int RULE_STRING=6;
    public static final int RULE_IDLIST=7;
    public static final int RULE_ANY_OTHER=10;
    public static final int RULE_PATH=5;
    public static final int RULE_WS=9;
    public static final int RULE_SL_COMMENT=8;
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




    // $ANTLR start entryRuleRoot
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:61:1: entryRuleRoot : ruleRoot EOF ;
    public final void entryRuleRoot() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:62:1: ( ruleRoot EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:63:1: ruleRoot EOF
            {
             before(grammarAccess.getRootRule()); 
            pushFollow(FOLLOW_ruleRoot_in_entryRuleRoot61);
            ruleRoot();
            _fsp--;

             after(grammarAccess.getRootRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleRoot68); 

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
    // $ANTLR end entryRuleRoot


    // $ANTLR start ruleRoot
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:70:1: ruleRoot : ( ( rule__Root__Group__0 ) ) ;
    public final void ruleRoot() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:74:2: ( ( ( rule__Root__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:75:1: ( ( rule__Root__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:75:1: ( ( rule__Root__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:76:1: ( rule__Root__Group__0 )
            {
             before(grammarAccess.getRootAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:77:1: ( rule__Root__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:77:2: rule__Root__Group__0
            {
            pushFollow(FOLLOW_rule__Root__Group__0_in_ruleRoot94);
            rule__Root__Group__0();
            _fsp--;


            }

             after(grammarAccess.getRootAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleRoot


    // $ANTLR start entryRuleTextDomain
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:89:1: entryRuleTextDomain : ruleTextDomain EOF ;
    public final void entryRuleTextDomain() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:90:1: ( ruleTextDomain EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:91:1: ruleTextDomain EOF
            {
             before(grammarAccess.getTextDomainRule()); 
            pushFollow(FOLLOW_ruleTextDomain_in_entryRuleTextDomain121);
            ruleTextDomain();
            _fsp--;

             after(grammarAccess.getTextDomainRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTextDomain128); 

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
    // $ANTLR end entryRuleTextDomain


    // $ANTLR start ruleTextDomain
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:98:1: ruleTextDomain : ( ( rule__TextDomain__Group__0 ) ) ;
    public final void ruleTextDomain() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:102:2: ( ( ( rule__TextDomain__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:103:1: ( ( rule__TextDomain__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:103:1: ( ( rule__TextDomain__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:104:1: ( rule__TextDomain__Group__0 )
            {
             before(grammarAccess.getTextDomainAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:105:1: ( rule__TextDomain__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:105:2: rule__TextDomain__Group__0
            {
            pushFollow(FOLLOW_rule__TextDomain__Group__0_in_ruleTextDomain154);
            rule__TextDomain__Group__0();
            _fsp--;


            }

             after(grammarAccess.getTextDomainAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleTextDomain


    // $ANTLR start entryRulePreprocessor
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:117:1: entryRulePreprocessor : rulePreprocessor EOF ;
    public final void entryRulePreprocessor() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:118:1: ( rulePreprocessor EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:119:1: rulePreprocessor EOF
            {
             before(grammarAccess.getPreprocessorRule()); 
            pushFollow(FOLLOW_rulePreprocessor_in_entryRulePreprocessor181);
            rulePreprocessor();
            _fsp--;

             after(grammarAccess.getPreprocessorRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePreprocessor188); 

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
    // $ANTLR end entryRulePreprocessor


    // $ANTLR start rulePreprocessor
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:126:1: rulePreprocessor : ( ( rule__Preprocessor__Alternatives ) ) ;
    public final void rulePreprocessor() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:130:2: ( ( ( rule__Preprocessor__Alternatives ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:131:1: ( ( rule__Preprocessor__Alternatives ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:131:1: ( ( rule__Preprocessor__Alternatives ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:132:1: ( rule__Preprocessor__Alternatives )
            {
             before(grammarAccess.getPreprocessorAccess().getAlternatives()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:133:1: ( rule__Preprocessor__Alternatives )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:133:2: rule__Preprocessor__Alternatives
            {
            pushFollow(FOLLOW_rule__Preprocessor__Alternatives_in_rulePreprocessor214);
            rule__Preprocessor__Alternatives();
            _fsp--;


            }

             after(grammarAccess.getPreprocessorAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rulePreprocessor


    // $ANTLR start entryRuleMacro
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:145:1: entryRuleMacro : ruleMacro EOF ;
    public final void entryRuleMacro() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:146:1: ( ruleMacro EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:147:1: ruleMacro EOF
            {
             before(grammarAccess.getMacroRule()); 
            pushFollow(FOLLOW_ruleMacro_in_entryRuleMacro241);
            ruleMacro();
            _fsp--;

             after(grammarAccess.getMacroRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleMacro248); 

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
    // $ANTLR end entryRuleMacro


    // $ANTLR start ruleMacro
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:154:1: ruleMacro : ( ( rule__Macro__Group__0 ) ) ;
    public final void ruleMacro() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:158:2: ( ( ( rule__Macro__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:159:1: ( ( rule__Macro__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:159:1: ( ( rule__Macro__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:160:1: ( rule__Macro__Group__0 )
            {
             before(grammarAccess.getMacroAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:161:1: ( rule__Macro__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:161:2: rule__Macro__Group__0
            {
            pushFollow(FOLLOW_rule__Macro__Group__0_in_ruleMacro274);
            rule__Macro__Group__0();
            _fsp--;


            }

             after(grammarAccess.getMacroAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleMacro


    // $ANTLR start entryRulePathInclude
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:173:1: entryRulePathInclude : rulePathInclude EOF ;
    public final void entryRulePathInclude() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:174:1: ( rulePathInclude EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:175:1: rulePathInclude EOF
            {
             before(grammarAccess.getPathIncludeRule()); 
            pushFollow(FOLLOW_rulePathInclude_in_entryRulePathInclude301);
            rulePathInclude();
            _fsp--;

             after(grammarAccess.getPathIncludeRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePathInclude308); 

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
    // $ANTLR end entryRulePathInclude


    // $ANTLR start rulePathInclude
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:182:1: rulePathInclude : ( ( rule__PathInclude__Group__0 ) ) ;
    public final void rulePathInclude() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:186:2: ( ( ( rule__PathInclude__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:187:1: ( ( rule__PathInclude__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:187:1: ( ( rule__PathInclude__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:188:1: ( rule__PathInclude__Group__0 )
            {
             before(grammarAccess.getPathIncludeAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:189:1: ( rule__PathInclude__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:189:2: rule__PathInclude__Group__0
            {
            pushFollow(FOLLOW_rule__PathInclude__Group__0_in_rulePathInclude334);
            rule__PathInclude__Group__0();
            _fsp--;


            }

             after(grammarAccess.getPathIncludeAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rulePathInclude


    // $ANTLR start entryRuleRootType
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:201:1: entryRuleRootType : ruleRootType EOF ;
    public final void entryRuleRootType() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:202:1: ( ruleRootType EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:203:1: ruleRootType EOF
            {
             before(grammarAccess.getRootTypeRule()); 
            pushFollow(FOLLOW_ruleRootType_in_entryRuleRootType361);
            ruleRootType();
            _fsp--;

             after(grammarAccess.getRootTypeRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleRootType368); 

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
    // $ANTLR end entryRuleRootType


    // $ANTLR start ruleRootType
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:210:1: ruleRootType : ( ( rule__RootType__Group__0 ) ) ;
    public final void ruleRootType() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:214:2: ( ( ( rule__RootType__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:215:1: ( ( rule__RootType__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:215:1: ( ( rule__RootType__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:216:1: ( rule__RootType__Group__0 )
            {
             before(grammarAccess.getRootTypeAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:217:1: ( rule__RootType__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:217:2: rule__RootType__Group__0
            {
            pushFollow(FOLLOW_rule__RootType__Group__0_in_ruleRootType394);
            rule__RootType__Group__0();
            _fsp--;


            }

             after(grammarAccess.getRootTypeAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleRootType


    // $ANTLR start entryRuleRootTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:229:1: entryRuleRootTag : ruleRootTag EOF ;
    public final void entryRuleRootTag() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:230:1: ( ruleRootTag EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:231:1: ruleRootTag EOF
            {
             before(grammarAccess.getRootTagRule()); 
            pushFollow(FOLLOW_ruleRootTag_in_entryRuleRootTag421);
            ruleRootTag();
            _fsp--;

             after(grammarAccess.getRootTagRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleRootTag428); 

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
    // $ANTLR end entryRuleRootTag


    // $ANTLR start ruleRootTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:238:1: ruleRootTag : ( ( rule__RootTag__Alternatives ) ) ;
    public final void ruleRootTag() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:242:2: ( ( ( rule__RootTag__Alternatives ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:243:1: ( ( rule__RootTag__Alternatives ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:243:1: ( ( rule__RootTag__Alternatives ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:244:1: ( rule__RootTag__Alternatives )
            {
             before(grammarAccess.getRootTagAccess().getAlternatives()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:245:1: ( rule__RootTag__Alternatives )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:245:2: rule__RootTag__Alternatives
            {
            pushFollow(FOLLOW_rule__RootTag__Alternatives_in_ruleRootTag454);
            rule__RootTag__Alternatives();
            _fsp--;


            }

             after(grammarAccess.getRootTagAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleRootTag


    // $ANTLR start entryRuleSimpleTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:257:1: entryRuleSimpleTag : ruleSimpleTag EOF ;
    public final void entryRuleSimpleTag() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:258:1: ( ruleSimpleTag EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:259:1: ruleSimpleTag EOF
            {
             before(grammarAccess.getSimpleTagRule()); 
            pushFollow(FOLLOW_ruleSimpleTag_in_entryRuleSimpleTag481);
            ruleSimpleTag();
            _fsp--;

             after(grammarAccess.getSimpleTagRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleSimpleTag488); 

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
    // $ANTLR end entryRuleSimpleTag


    // $ANTLR start ruleSimpleTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:266:1: ruleSimpleTag : ( ( rule__SimpleTag__Group__0 ) ) ;
    public final void ruleSimpleTag() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:270:2: ( ( ( rule__SimpleTag__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:271:1: ( ( rule__SimpleTag__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:271:1: ( ( rule__SimpleTag__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:272:1: ( rule__SimpleTag__Group__0 )
            {
             before(grammarAccess.getSimpleTagAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:273:1: ( rule__SimpleTag__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:273:2: rule__SimpleTag__Group__0
            {
            pushFollow(FOLLOW_rule__SimpleTag__Group__0_in_ruleSimpleTag514);
            rule__SimpleTag__Group__0();
            _fsp--;


            }

             after(grammarAccess.getSimpleTagAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleSimpleTag


    // $ANTLR start entryRuleAddedTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:285:1: entryRuleAddedTag : ruleAddedTag EOF ;
    public final void entryRuleAddedTag() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:286:1: ( ruleAddedTag EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:287:1: ruleAddedTag EOF
            {
             before(grammarAccess.getAddedTagRule()); 
            pushFollow(FOLLOW_ruleAddedTag_in_entryRuleAddedTag541);
            ruleAddedTag();
            _fsp--;

             after(grammarAccess.getAddedTagRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleAddedTag548); 

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
    // $ANTLR end entryRuleAddedTag


    // $ANTLR start ruleAddedTag
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:294:1: ruleAddedTag : ( ( rule__AddedTag__Group__0 ) ) ;
    public final void ruleAddedTag() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:298:2: ( ( ( rule__AddedTag__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:299:1: ( ( rule__AddedTag__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:299:1: ( ( rule__AddedTag__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:300:1: ( rule__AddedTag__Group__0 )
            {
             before(grammarAccess.getAddedTagAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:301:1: ( rule__AddedTag__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:301:2: rule__AddedTag__Group__0
            {
            pushFollow(FOLLOW_rule__AddedTag__Group__0_in_ruleAddedTag574);
            rule__AddedTag__Group__0();
            _fsp--;


            }

             after(grammarAccess.getAddedTagAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleAddedTag


    // $ANTLR start entryRuleRootTagsList
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:313:1: entryRuleRootTagsList : ruleRootTagsList EOF ;
    public final void entryRuleRootTagsList() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:314:1: ( ruleRootTagsList EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:315:1: ruleRootTagsList EOF
            {
             before(grammarAccess.getRootTagsListRule()); 
            pushFollow(FOLLOW_ruleRootTagsList_in_entryRuleRootTagsList601);
            ruleRootTagsList();
            _fsp--;

             after(grammarAccess.getRootTagsListRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleRootTagsList608); 

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
    // $ANTLR end entryRuleRootTagsList


    // $ANTLR start ruleRootTagsList
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:322:1: ruleRootTagsList : ( RULE_ID ) ;
    public final void ruleRootTagsList() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:326:2: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:327:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:327:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:328:1: RULE_ID
            {
             before(grammarAccess.getRootTagsListAccess().getIDTerminalRuleCall()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleRootTagsList634); 
             after(grammarAccess.getRootTagsListAccess().getIDTerminalRuleCall()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleRootTagsList


    // $ANTLR start entryRuleAttributes
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:341:1: entryRuleAttributes : ruleAttributes EOF ;
    public final void entryRuleAttributes() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:342:1: ( ruleAttributes EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:343:1: ruleAttributes EOF
            {
             before(grammarAccess.getAttributesRule()); 
            pushFollow(FOLLOW_ruleAttributes_in_entryRuleAttributes660);
            ruleAttributes();
            _fsp--;

             after(grammarAccess.getAttributesRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleAttributes667); 

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
    // $ANTLR end entryRuleAttributes


    // $ANTLR start ruleAttributes
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:350:1: ruleAttributes : ( ( rule__Attributes__Group__0 ) ) ;
    public final void ruleAttributes() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:354:2: ( ( ( rule__Attributes__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:355:1: ( ( rule__Attributes__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:355:1: ( ( rule__Attributes__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:356:1: ( rule__Attributes__Group__0 )
            {
             before(grammarAccess.getAttributesAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:357:1: ( rule__Attributes__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:357:2: rule__Attributes__Group__0
            {
            pushFollow(FOLLOW_rule__Attributes__Group__0_in_ruleAttributes693);
            rule__Attributes__Group__0();
            _fsp--;


            }

             after(grammarAccess.getAttributesAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleAttributes


    // $ANTLR start entryRuleTSTRING
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:369:1: entryRuleTSTRING : ruleTSTRING EOF ;
    public final void entryRuleTSTRING() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:370:1: ( ruleTSTRING EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:371:1: ruleTSTRING EOF
            {
             before(grammarAccess.getTSTRINGRule()); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING720);
            ruleTSTRING();
            _fsp--;

             after(grammarAccess.getTSTRINGRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING727); 

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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:378:1: ruleTSTRING : ( ( rule__TSTRING__Group__0 ) ) ;
    public final void ruleTSTRING() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:382:2: ( ( ( rule__TSTRING__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:383:1: ( ( rule__TSTRING__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:383:1: ( ( rule__TSTRING__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:384:1: ( rule__TSTRING__Group__0 )
            {
             before(grammarAccess.getTSTRINGAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:385:1: ( rule__TSTRING__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:385:2: rule__TSTRING__Group__0
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0_in_ruleTSTRING753);
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


    // $ANTLR start entryRuleHOMEPATH
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:397:1: entryRuleHOMEPATH : ruleHOMEPATH EOF ;
    public final void entryRuleHOMEPATH() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:398:1: ( ruleHOMEPATH EOF )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:399:1: ruleHOMEPATH EOF
            {
             before(grammarAccess.getHOMEPATHRule()); 
            pushFollow(FOLLOW_ruleHOMEPATH_in_entryRuleHOMEPATH780);
            ruleHOMEPATH();
            _fsp--;

             after(grammarAccess.getHOMEPATHRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleHOMEPATH787); 

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
    // $ANTLR end entryRuleHOMEPATH


    // $ANTLR start ruleHOMEPATH
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:406:1: ruleHOMEPATH : ( ( rule__HOMEPATH__Group__0 ) ) ;
    public final void ruleHOMEPATH() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:410:2: ( ( ( rule__HOMEPATH__Group__0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:411:1: ( ( rule__HOMEPATH__Group__0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:411:1: ( ( rule__HOMEPATH__Group__0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:412:1: ( rule__HOMEPATH__Group__0 )
            {
             before(grammarAccess.getHOMEPATHAccess().getGroup()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:413:1: ( rule__HOMEPATH__Group__0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:413:2: rule__HOMEPATH__Group__0
            {
            pushFollow(FOLLOW_rule__HOMEPATH__Group__0_in_ruleHOMEPATH813);
            rule__HOMEPATH__Group__0();
            _fsp--;


            }

             after(grammarAccess.getHOMEPATHAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleHOMEPATH


    // $ANTLR start rule__Preprocessor__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:425:1: rule__Preprocessor__Alternatives : ( ( ruleMacro ) | ( rulePathInclude ) );
    public final void rule__Preprocessor__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:429:1: ( ( ruleMacro ) | ( rulePathInclude ) )
            int alt1=2;
            int LA1_0 = input.LA(1);

            if ( (LA1_0==12) ) {
                int LA1_1 = input.LA(2);

                if ( (LA1_1==RULE_PATH||LA1_1==20) ) {
                    alt1=2;
                }
                else if ( (LA1_1==RULE_ID) ) {
                    alt1=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("425:1: rule__Preprocessor__Alternatives : ( ( ruleMacro ) | ( rulePathInclude ) );", 1, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("425:1: rule__Preprocessor__Alternatives : ( ( ruleMacro ) | ( rulePathInclude ) );", 1, 0, input);

                throw nvae;
            }
            switch (alt1) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:430:1: ( ruleMacro )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:430:1: ( ruleMacro )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:431:1: ruleMacro
                    {
                     before(grammarAccess.getPreprocessorAccess().getMacroParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleMacro_in_rule__Preprocessor__Alternatives849);
                    ruleMacro();
                    _fsp--;

                     after(grammarAccess.getPreprocessorAccess().getMacroParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:436:6: ( rulePathInclude )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:436:6: ( rulePathInclude )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:437:1: rulePathInclude
                    {
                     before(grammarAccess.getPreprocessorAccess().getPathIncludeParserRuleCall_1()); 
                    pushFollow(FOLLOW_rulePathInclude_in_rule__Preprocessor__Alternatives866);
                    rulePathInclude();
                    _fsp--;

                     after(grammarAccess.getPreprocessorAccess().getPathIncludeParserRuleCall_1()); 

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
    // $ANTLR end rule__Preprocessor__Alternatives


    // $ANTLR start rule__PathInclude__PathAlternatives_1_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:447:1: rule__PathInclude__PathAlternatives_1_0 : ( ( ruleHOMEPATH ) | ( RULE_PATH ) );
    public final void rule__PathInclude__PathAlternatives_1_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:451:1: ( ( ruleHOMEPATH ) | ( RULE_PATH ) )
            int alt2=2;
            int LA2_0 = input.LA(1);

            if ( (LA2_0==20) ) {
                alt2=1;
            }
            else if ( (LA2_0==RULE_PATH) ) {
                alt2=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("447:1: rule__PathInclude__PathAlternatives_1_0 : ( ( ruleHOMEPATH ) | ( RULE_PATH ) );", 2, 0, input);

                throw nvae;
            }
            switch (alt2) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:452:1: ( ruleHOMEPATH )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:452:1: ( ruleHOMEPATH )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:453:1: ruleHOMEPATH
                    {
                     before(grammarAccess.getPathIncludeAccess().getPathHOMEPATHParserRuleCall_1_0_0()); 
                    pushFollow(FOLLOW_ruleHOMEPATH_in_rule__PathInclude__PathAlternatives_1_0898);
                    ruleHOMEPATH();
                    _fsp--;

                     after(grammarAccess.getPathIncludeAccess().getPathHOMEPATHParserRuleCall_1_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:458:6: ( RULE_PATH )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:458:6: ( RULE_PATH )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:459:1: RULE_PATH
                    {
                     before(grammarAccess.getPathIncludeAccess().getPathPATHTerminalRuleCall_1_0_1()); 
                    match(input,RULE_PATH,FOLLOW_RULE_PATH_in_rule__PathInclude__PathAlternatives_1_0915); 
                     after(grammarAccess.getPathIncludeAccess().getPathPATHTerminalRuleCall_1_0_1()); 

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
    // $ANTLR end rule__PathInclude__PathAlternatives_1_0


    // $ANTLR start rule__RootTag__Alternatives
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:469:1: rule__RootTag__Alternatives : ( ( ruleSimpleTag ) | ( ruleAddedTag ) );
    public final void rule__RootTag__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:473:1: ( ( ruleSimpleTag ) | ( ruleAddedTag ) )
            int alt3=2;
            int LA3_0 = input.LA(1);

            if ( (LA3_0==RULE_ID||LA3_0==16) ) {
                alt3=1;
            }
            else if ( (LA3_0==17) ) {
                alt3=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("469:1: rule__RootTag__Alternatives : ( ( ruleSimpleTag ) | ( ruleAddedTag ) );", 3, 0, input);

                throw nvae;
            }
            switch (alt3) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:474:1: ( ruleSimpleTag )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:474:1: ( ruleSimpleTag )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:475:1: ruleSimpleTag
                    {
                     before(grammarAccess.getRootTagAccess().getSimpleTagParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleSimpleTag_in_rule__RootTag__Alternatives947);
                    ruleSimpleTag();
                    _fsp--;

                     after(grammarAccess.getRootTagAccess().getSimpleTagParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:480:6: ( ruleAddedTag )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:480:6: ( ruleAddedTag )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:481:1: ruleAddedTag
                    {
                     before(grammarAccess.getRootTagAccess().getAddedTagParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleAddedTag_in_rule__RootTag__Alternatives964);
                    ruleAddedTag();
                    _fsp--;

                     after(grammarAccess.getRootTagAccess().getAddedTagParserRuleCall_1()); 

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
    // $ANTLR end rule__RootTag__Alternatives


    // $ANTLR start rule__Attributes__AttrValueAlternatives_2_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:491:1: rule__Attributes__AttrValueAlternatives_2_0 : ( ( ruleTSTRING ) | ( RULE_STRING ) | ( RULE_PATH ) | ( RULE_ID ) | ( RULE_IDLIST ) );
    public final void rule__Attributes__AttrValueAlternatives_2_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:495:1: ( ( ruleTSTRING ) | ( RULE_STRING ) | ( RULE_PATH ) | ( RULE_ID ) | ( RULE_IDLIST ) )
            int alt4=5;
            switch ( input.LA(1) ) {
            case 19:
                {
                alt4=1;
                }
                break;
            case RULE_STRING:
                {
                alt4=2;
                }
                break;
            case RULE_PATH:
                {
                alt4=3;
                }
                break;
            case RULE_ID:
                {
                alt4=4;
                }
                break;
            case RULE_IDLIST:
                {
                alt4=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("491:1: rule__Attributes__AttrValueAlternatives_2_0 : ( ( ruleTSTRING ) | ( RULE_STRING ) | ( RULE_PATH ) | ( RULE_ID ) | ( RULE_IDLIST ) );", 4, 0, input);

                throw nvae;
            }

            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:496:1: ( ruleTSTRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:496:1: ( ruleTSTRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:497:1: ruleTSTRING
                    {
                     before(grammarAccess.getAttributesAccess().getAttrValueTSTRINGParserRuleCall_2_0_0()); 
                    pushFollow(FOLLOW_ruleTSTRING_in_rule__Attributes__AttrValueAlternatives_2_0996);
                    ruleTSTRING();
                    _fsp--;

                     after(grammarAccess.getAttributesAccess().getAttrValueTSTRINGParserRuleCall_2_0_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:502:6: ( RULE_STRING )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:502:6: ( RULE_STRING )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:503:1: RULE_STRING
                    {
                     before(grammarAccess.getAttributesAccess().getAttrValueSTRINGTerminalRuleCall_2_0_1()); 
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__Attributes__AttrValueAlternatives_2_01013); 
                     after(grammarAccess.getAttributesAccess().getAttrValueSTRINGTerminalRuleCall_2_0_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:508:6: ( RULE_PATH )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:508:6: ( RULE_PATH )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:509:1: RULE_PATH
                    {
                     before(grammarAccess.getAttributesAccess().getAttrValuePATHTerminalRuleCall_2_0_2()); 
                    match(input,RULE_PATH,FOLLOW_RULE_PATH_in_rule__Attributes__AttrValueAlternatives_2_01030); 
                     after(grammarAccess.getAttributesAccess().getAttrValuePATHTerminalRuleCall_2_0_2()); 

                    }


                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:514:6: ( RULE_ID )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:514:6: ( RULE_ID )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:515:1: RULE_ID
                    {
                     before(grammarAccess.getAttributesAccess().getAttrValueIDTerminalRuleCall_2_0_3()); 
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__Attributes__AttrValueAlternatives_2_01047); 
                     after(grammarAccess.getAttributesAccess().getAttrValueIDTerminalRuleCall_2_0_3()); 

                    }


                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:520:6: ( RULE_IDLIST )
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:520:6: ( RULE_IDLIST )
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:521:1: RULE_IDLIST
                    {
                     before(grammarAccess.getAttributesAccess().getAttrValueIDLISTTerminalRuleCall_2_0_4()); 
                    match(input,RULE_IDLIST,FOLLOW_RULE_IDLIST_in_rule__Attributes__AttrValueAlternatives_2_01064); 
                     after(grammarAccess.getAttributesAccess().getAttrValueIDLISTTerminalRuleCall_2_0_4()); 

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
    // $ANTLR end rule__Attributes__AttrValueAlternatives_2_0


    // $ANTLR start rule__Root__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:533:1: rule__Root__Group__0 : rule__Root__Group__0__Impl rule__Root__Group__1 ;
    public final void rule__Root__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:537:1: ( rule__Root__Group__0__Impl rule__Root__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:538:2: rule__Root__Group__0__Impl rule__Root__Group__1
            {
            pushFollow(FOLLOW_rule__Root__Group__0__Impl_in_rule__Root__Group__01094);
            rule__Root__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__Root__Group__1_in_rule__Root__Group__01097);
            rule__Root__Group__1();
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
    // $ANTLR end rule__Root__Group__0


    // $ANTLR start rule__Root__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:545:1: rule__Root__Group__0__Impl : ( ( rule__Root__TextdomainsAssignment_0 )* ) ;
    public final void rule__Root__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:549:1: ( ( ( rule__Root__TextdomainsAssignment_0 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:550:1: ( ( rule__Root__TextdomainsAssignment_0 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:550:1: ( ( rule__Root__TextdomainsAssignment_0 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:551:1: ( rule__Root__TextdomainsAssignment_0 )*
            {
             before(grammarAccess.getRootAccess().getTextdomainsAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:552:1: ( rule__Root__TextdomainsAssignment_0 )*
            loop5:
            do {
                int alt5=2;
                int LA5_0 = input.LA(1);

                if ( (LA5_0==11) ) {
                    alt5=1;
                }


                switch (alt5) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:552:2: rule__Root__TextdomainsAssignment_0
            	    {
            	    pushFollow(FOLLOW_rule__Root__TextdomainsAssignment_0_in_rule__Root__Group__0__Impl1124);
            	    rule__Root__TextdomainsAssignment_0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop5;
                }
            } while (true);

             after(grammarAccess.getRootAccess().getTextdomainsAssignment_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Root__Group__0__Impl


    // $ANTLR start rule__Root__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:562:1: rule__Root__Group__1 : rule__Root__Group__1__Impl rule__Root__Group__2 ;
    public final void rule__Root__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:566:1: ( rule__Root__Group__1__Impl rule__Root__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:567:2: rule__Root__Group__1__Impl rule__Root__Group__2
            {
            pushFollow(FOLLOW_rule__Root__Group__1__Impl_in_rule__Root__Group__11155);
            rule__Root__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__Root__Group__2_in_rule__Root__Group__11158);
            rule__Root__Group__2();
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
    // $ANTLR end rule__Root__Group__1


    // $ANTLR start rule__Root__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:574:1: rule__Root__Group__1__Impl : ( ( rule__Root__PreprocAssignment_1 )* ) ;
    public final void rule__Root__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:578:1: ( ( ( rule__Root__PreprocAssignment_1 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:579:1: ( ( rule__Root__PreprocAssignment_1 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:579:1: ( ( rule__Root__PreprocAssignment_1 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:580:1: ( rule__Root__PreprocAssignment_1 )*
            {
             before(grammarAccess.getRootAccess().getPreprocAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:581:1: ( rule__Root__PreprocAssignment_1 )*
            loop6:
            do {
                int alt6=2;
                int LA6_0 = input.LA(1);

                if ( (LA6_0==12) ) {
                    alt6=1;
                }


                switch (alt6) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:581:2: rule__Root__PreprocAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__Root__PreprocAssignment_1_in_rule__Root__Group__1__Impl1185);
            	    rule__Root__PreprocAssignment_1();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop6;
                }
            } while (true);

             after(grammarAccess.getRootAccess().getPreprocAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Root__Group__1__Impl


    // $ANTLR start rule__Root__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:591:1: rule__Root__Group__2 : rule__Root__Group__2__Impl ;
    public final void rule__Root__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:595:1: ( rule__Root__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:596:2: rule__Root__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__Root__Group__2__Impl_in_rule__Root__Group__21216);
            rule__Root__Group__2__Impl();
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
    // $ANTLR end rule__Root__Group__2


    // $ANTLR start rule__Root__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:602:1: rule__Root__Group__2__Impl : ( ( rule__Root__RootsAssignment_2 )* ) ;
    public final void rule__Root__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:606:1: ( ( ( rule__Root__RootsAssignment_2 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:607:1: ( ( rule__Root__RootsAssignment_2 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:607:1: ( ( rule__Root__RootsAssignment_2 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:608:1: ( rule__Root__RootsAssignment_2 )*
            {
             before(grammarAccess.getRootAccess().getRootsAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:609:1: ( rule__Root__RootsAssignment_2 )*
            loop7:
            do {
                int alt7=2;
                int LA7_0 = input.LA(1);

                if ( (LA7_0==14) ) {
                    alt7=1;
                }


                switch (alt7) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:609:2: rule__Root__RootsAssignment_2
            	    {
            	    pushFollow(FOLLOW_rule__Root__RootsAssignment_2_in_rule__Root__Group__2__Impl1243);
            	    rule__Root__RootsAssignment_2();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop7;
                }
            } while (true);

             after(grammarAccess.getRootAccess().getRootsAssignment_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Root__Group__2__Impl


    // $ANTLR start rule__TextDomain__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:625:1: rule__TextDomain__Group__0 : rule__TextDomain__Group__0__Impl rule__TextDomain__Group__1 ;
    public final void rule__TextDomain__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:629:1: ( rule__TextDomain__Group__0__Impl rule__TextDomain__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:630:2: rule__TextDomain__Group__0__Impl rule__TextDomain__Group__1
            {
            pushFollow(FOLLOW_rule__TextDomain__Group__0__Impl_in_rule__TextDomain__Group__01280);
            rule__TextDomain__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TextDomain__Group__1_in_rule__TextDomain__Group__01283);
            rule__TextDomain__Group__1();
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
    // $ANTLR end rule__TextDomain__Group__0


    // $ANTLR start rule__TextDomain__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:637:1: rule__TextDomain__Group__0__Impl : ( '#textdomain ' ) ;
    public final void rule__TextDomain__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:641:1: ( ( '#textdomain ' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:642:1: ( '#textdomain ' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:642:1: ( '#textdomain ' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:643:1: '#textdomain '
            {
             before(grammarAccess.getTextDomainAccess().getTextdomainKeyword_0()); 
            match(input,11,FOLLOW_11_in_rule__TextDomain__Group__0__Impl1311); 
             after(grammarAccess.getTextDomainAccess().getTextdomainKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TextDomain__Group__0__Impl


    // $ANTLR start rule__TextDomain__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:656:1: rule__TextDomain__Group__1 : rule__TextDomain__Group__1__Impl ;
    public final void rule__TextDomain__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:660:1: ( rule__TextDomain__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:661:2: rule__TextDomain__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__TextDomain__Group__1__Impl_in_rule__TextDomain__Group__11342);
            rule__TextDomain__Group__1__Impl();
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
    // $ANTLR end rule__TextDomain__Group__1


    // $ANTLR start rule__TextDomain__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:667:1: rule__TextDomain__Group__1__Impl : ( ( rule__TextDomain__DomainNameAssignment_1 ) ) ;
    public final void rule__TextDomain__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:671:1: ( ( ( rule__TextDomain__DomainNameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:672:1: ( ( rule__TextDomain__DomainNameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:672:1: ( ( rule__TextDomain__DomainNameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:673:1: ( rule__TextDomain__DomainNameAssignment_1 )
            {
             before(grammarAccess.getTextDomainAccess().getDomainNameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:674:1: ( rule__TextDomain__DomainNameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:674:2: rule__TextDomain__DomainNameAssignment_1
            {
            pushFollow(FOLLOW_rule__TextDomain__DomainNameAssignment_1_in_rule__TextDomain__Group__1__Impl1369);
            rule__TextDomain__DomainNameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getTextDomainAccess().getDomainNameAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TextDomain__Group__1__Impl


    // $ANTLR start rule__Macro__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:688:1: rule__Macro__Group__0 : rule__Macro__Group__0__Impl rule__Macro__Group__1 ;
    public final void rule__Macro__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:692:1: ( rule__Macro__Group__0__Impl rule__Macro__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:693:2: rule__Macro__Group__0__Impl rule__Macro__Group__1
            {
            pushFollow(FOLLOW_rule__Macro__Group__0__Impl_in_rule__Macro__Group__01403);
            rule__Macro__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__Macro__Group__1_in_rule__Macro__Group__01406);
            rule__Macro__Group__1();
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
    // $ANTLR end rule__Macro__Group__0


    // $ANTLR start rule__Macro__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:700:1: rule__Macro__Group__0__Impl : ( '{' ) ;
    public final void rule__Macro__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:704:1: ( ( '{' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:705:1: ( '{' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:705:1: ( '{' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:706:1: '{'
            {
             before(grammarAccess.getMacroAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,12,FOLLOW_12_in_rule__Macro__Group__0__Impl1434); 
             after(grammarAccess.getMacroAccess().getLeftCurlyBracketKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Macro__Group__0__Impl


    // $ANTLR start rule__Macro__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:719:1: rule__Macro__Group__1 : rule__Macro__Group__1__Impl rule__Macro__Group__2 ;
    public final void rule__Macro__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:723:1: ( rule__Macro__Group__1__Impl rule__Macro__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:724:2: rule__Macro__Group__1__Impl rule__Macro__Group__2
            {
            pushFollow(FOLLOW_rule__Macro__Group__1__Impl_in_rule__Macro__Group__11465);
            rule__Macro__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__Macro__Group__2_in_rule__Macro__Group__11468);
            rule__Macro__Group__2();
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
    // $ANTLR end rule__Macro__Group__1


    // $ANTLR start rule__Macro__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:731:1: rule__Macro__Group__1__Impl : ( ( ( rule__Macro__MacroContentAssignment_1 ) ) ( ( rule__Macro__MacroContentAssignment_1 )* ) ) ;
    public final void rule__Macro__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:735:1: ( ( ( ( rule__Macro__MacroContentAssignment_1 ) ) ( ( rule__Macro__MacroContentAssignment_1 )* ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:736:1: ( ( ( rule__Macro__MacroContentAssignment_1 ) ) ( ( rule__Macro__MacroContentAssignment_1 )* ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:736:1: ( ( ( rule__Macro__MacroContentAssignment_1 ) ) ( ( rule__Macro__MacroContentAssignment_1 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:737:1: ( ( rule__Macro__MacroContentAssignment_1 ) ) ( ( rule__Macro__MacroContentAssignment_1 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:737:1: ( ( rule__Macro__MacroContentAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:738:1: ( rule__Macro__MacroContentAssignment_1 )
            {
             before(grammarAccess.getMacroAccess().getMacroContentAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:739:1: ( rule__Macro__MacroContentAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:739:2: rule__Macro__MacroContentAssignment_1
            {
            pushFollow(FOLLOW_rule__Macro__MacroContentAssignment_1_in_rule__Macro__Group__1__Impl1497);
            rule__Macro__MacroContentAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getMacroAccess().getMacroContentAssignment_1()); 

            }

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:742:1: ( ( rule__Macro__MacroContentAssignment_1 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:743:1: ( rule__Macro__MacroContentAssignment_1 )*
            {
             before(grammarAccess.getMacroAccess().getMacroContentAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:744:1: ( rule__Macro__MacroContentAssignment_1 )*
            loop8:
            do {
                int alt8=2;
                int LA8_0 = input.LA(1);

                if ( (LA8_0==RULE_ID) ) {
                    alt8=1;
                }


                switch (alt8) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:744:2: rule__Macro__MacroContentAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__Macro__MacroContentAssignment_1_in_rule__Macro__Group__1__Impl1509);
            	    rule__Macro__MacroContentAssignment_1();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop8;
                }
            } while (true);

             after(grammarAccess.getMacroAccess().getMacroContentAssignment_1()); 

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
    // $ANTLR end rule__Macro__Group__1__Impl


    // $ANTLR start rule__Macro__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:755:1: rule__Macro__Group__2 : rule__Macro__Group__2__Impl ;
    public final void rule__Macro__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:759:1: ( rule__Macro__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:760:2: rule__Macro__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__Macro__Group__2__Impl_in_rule__Macro__Group__21542);
            rule__Macro__Group__2__Impl();
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
    // $ANTLR end rule__Macro__Group__2


    // $ANTLR start rule__Macro__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:766:1: rule__Macro__Group__2__Impl : ( '}' ) ;
    public final void rule__Macro__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:770:1: ( ( '}' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:771:1: ( '}' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:771:1: ( '}' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:772:1: '}'
            {
             before(grammarAccess.getMacroAccess().getRightCurlyBracketKeyword_2()); 
            match(input,13,FOLLOW_13_in_rule__Macro__Group__2__Impl1570); 
             after(grammarAccess.getMacroAccess().getRightCurlyBracketKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Macro__Group__2__Impl


    // $ANTLR start rule__PathInclude__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:791:1: rule__PathInclude__Group__0 : rule__PathInclude__Group__0__Impl rule__PathInclude__Group__1 ;
    public final void rule__PathInclude__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:795:1: ( rule__PathInclude__Group__0__Impl rule__PathInclude__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:796:2: rule__PathInclude__Group__0__Impl rule__PathInclude__Group__1
            {
            pushFollow(FOLLOW_rule__PathInclude__Group__0__Impl_in_rule__PathInclude__Group__01607);
            rule__PathInclude__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PathInclude__Group__1_in_rule__PathInclude__Group__01610);
            rule__PathInclude__Group__1();
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
    // $ANTLR end rule__PathInclude__Group__0


    // $ANTLR start rule__PathInclude__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:803:1: rule__PathInclude__Group__0__Impl : ( '{' ) ;
    public final void rule__PathInclude__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:807:1: ( ( '{' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:808:1: ( '{' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:808:1: ( '{' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:809:1: '{'
            {
             before(grammarAccess.getPathIncludeAccess().getLeftCurlyBracketKeyword_0()); 
            match(input,12,FOLLOW_12_in_rule__PathInclude__Group__0__Impl1638); 
             after(grammarAccess.getPathIncludeAccess().getLeftCurlyBracketKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PathInclude__Group__0__Impl


    // $ANTLR start rule__PathInclude__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:822:1: rule__PathInclude__Group__1 : rule__PathInclude__Group__1__Impl rule__PathInclude__Group__2 ;
    public final void rule__PathInclude__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:826:1: ( rule__PathInclude__Group__1__Impl rule__PathInclude__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:827:2: rule__PathInclude__Group__1__Impl rule__PathInclude__Group__2
            {
            pushFollow(FOLLOW_rule__PathInclude__Group__1__Impl_in_rule__PathInclude__Group__11669);
            rule__PathInclude__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__PathInclude__Group__2_in_rule__PathInclude__Group__11672);
            rule__PathInclude__Group__2();
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
    // $ANTLR end rule__PathInclude__Group__1


    // $ANTLR start rule__PathInclude__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:834:1: rule__PathInclude__Group__1__Impl : ( ( rule__PathInclude__PathAssignment_1 ) ) ;
    public final void rule__PathInclude__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:838:1: ( ( ( rule__PathInclude__PathAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:839:1: ( ( rule__PathInclude__PathAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:839:1: ( ( rule__PathInclude__PathAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:840:1: ( rule__PathInclude__PathAssignment_1 )
            {
             before(grammarAccess.getPathIncludeAccess().getPathAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:841:1: ( rule__PathInclude__PathAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:841:2: rule__PathInclude__PathAssignment_1
            {
            pushFollow(FOLLOW_rule__PathInclude__PathAssignment_1_in_rule__PathInclude__Group__1__Impl1699);
            rule__PathInclude__PathAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getPathIncludeAccess().getPathAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PathInclude__Group__1__Impl


    // $ANTLR start rule__PathInclude__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:851:1: rule__PathInclude__Group__2 : rule__PathInclude__Group__2__Impl ;
    public final void rule__PathInclude__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:855:1: ( rule__PathInclude__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:856:2: rule__PathInclude__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__PathInclude__Group__2__Impl_in_rule__PathInclude__Group__21729);
            rule__PathInclude__Group__2__Impl();
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
    // $ANTLR end rule__PathInclude__Group__2


    // $ANTLR start rule__PathInclude__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:862:1: rule__PathInclude__Group__2__Impl : ( '}' ) ;
    public final void rule__PathInclude__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:866:1: ( ( '}' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:867:1: ( '}' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:867:1: ( '}' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:868:1: '}'
            {
             before(grammarAccess.getPathIncludeAccess().getRightCurlyBracketKeyword_2()); 
            match(input,13,FOLLOW_13_in_rule__PathInclude__Group__2__Impl1757); 
             after(grammarAccess.getPathIncludeAccess().getRightCurlyBracketKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PathInclude__Group__2__Impl


    // $ANTLR start rule__RootType__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:887:1: rule__RootType__Group__0 : rule__RootType__Group__0__Impl rule__RootType__Group__1 ;
    public final void rule__RootType__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:891:1: ( rule__RootType__Group__0__Impl rule__RootType__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:892:2: rule__RootType__Group__0__Impl rule__RootType__Group__1
            {
            pushFollow(FOLLOW_rule__RootType__Group__0__Impl_in_rule__RootType__Group__01794);
            rule__RootType__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__RootType__Group__1_in_rule__RootType__Group__01797);
            rule__RootType__Group__1();
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
    // $ANTLR end rule__RootType__Group__0


    // $ANTLR start rule__RootType__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:899:1: rule__RootType__Group__0__Impl : ( '[' ) ;
    public final void rule__RootType__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:903:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:904:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:904:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:905:1: '['
            {
             before(grammarAccess.getRootTypeAccess().getLeftSquareBracketKeyword_0()); 
            match(input,14,FOLLOW_14_in_rule__RootType__Group__0__Impl1825); 
             after(grammarAccess.getRootTypeAccess().getLeftSquareBracketKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__Group__0__Impl


    // $ANTLR start rule__RootType__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:918:1: rule__RootType__Group__1 : rule__RootType__Group__1__Impl rule__RootType__Group__2 ;
    public final void rule__RootType__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:922:1: ( rule__RootType__Group__1__Impl rule__RootType__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:923:2: rule__RootType__Group__1__Impl rule__RootType__Group__2
            {
            pushFollow(FOLLOW_rule__RootType__Group__1__Impl_in_rule__RootType__Group__11856);
            rule__RootType__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__RootType__Group__2_in_rule__RootType__Group__11859);
            rule__RootType__Group__2();
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
    // $ANTLR end rule__RootType__Group__1


    // $ANTLR start rule__RootType__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:930:1: rule__RootType__Group__1__Impl : ( ( rule__RootType__StartTagAssignment_1 ) ) ;
    public final void rule__RootType__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:934:1: ( ( ( rule__RootType__StartTagAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:935:1: ( ( rule__RootType__StartTagAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:935:1: ( ( rule__RootType__StartTagAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:936:1: ( rule__RootType__StartTagAssignment_1 )
            {
             before(grammarAccess.getRootTypeAccess().getStartTagAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:937:1: ( rule__RootType__StartTagAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:937:2: rule__RootType__StartTagAssignment_1
            {
            pushFollow(FOLLOW_rule__RootType__StartTagAssignment_1_in_rule__RootType__Group__1__Impl1886);
            rule__RootType__StartTagAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getRootTypeAccess().getStartTagAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__Group__1__Impl


    // $ANTLR start rule__RootType__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:947:1: rule__RootType__Group__2 : rule__RootType__Group__2__Impl rule__RootType__Group__3 ;
    public final void rule__RootType__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:951:1: ( rule__RootType__Group__2__Impl rule__RootType__Group__3 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:952:2: rule__RootType__Group__2__Impl rule__RootType__Group__3
            {
            pushFollow(FOLLOW_rule__RootType__Group__2__Impl_in_rule__RootType__Group__21916);
            rule__RootType__Group__2__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__RootType__Group__3_in_rule__RootType__Group__21919);
            rule__RootType__Group__3();
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
    // $ANTLR end rule__RootType__Group__2


    // $ANTLR start rule__RootType__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:959:1: rule__RootType__Group__2__Impl : ( ']' ) ;
    public final void rule__RootType__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:963:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:964:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:964:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:965:1: ']'
            {
             before(grammarAccess.getRootTypeAccess().getRightSquareBracketKeyword_2()); 
            match(input,15,FOLLOW_15_in_rule__RootType__Group__2__Impl1947); 
             after(grammarAccess.getRootTypeAccess().getRightSquareBracketKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__Group__2__Impl


    // $ANTLR start rule__RootType__Group__3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:978:1: rule__RootType__Group__3 : rule__RootType__Group__3__Impl rule__RootType__Group__4 ;
    public final void rule__RootType__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:982:1: ( rule__RootType__Group__3__Impl rule__RootType__Group__4 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:983:2: rule__RootType__Group__3__Impl rule__RootType__Group__4
            {
            pushFollow(FOLLOW_rule__RootType__Group__3__Impl_in_rule__RootType__Group__31978);
            rule__RootType__Group__3__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__RootType__Group__4_in_rule__RootType__Group__31981);
            rule__RootType__Group__4();
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
    // $ANTLR end rule__RootType__Group__3


    // $ANTLR start rule__RootType__Group__3__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:990:1: rule__RootType__Group__3__Impl : ( ( rule__RootType__SubTypesAssignment_3 )* ) ;
    public final void rule__RootType__Group__3__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:994:1: ( ( ( rule__RootType__SubTypesAssignment_3 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:995:1: ( ( rule__RootType__SubTypesAssignment_3 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:995:1: ( ( rule__RootType__SubTypesAssignment_3 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:996:1: ( rule__RootType__SubTypesAssignment_3 )*
            {
             before(grammarAccess.getRootTypeAccess().getSubTypesAssignment_3()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:997:1: ( rule__RootType__SubTypesAssignment_3 )*
            loop9:
            do {
                int alt9=2;
                int LA9_0 = input.LA(1);

                if ( (LA9_0==14) ) {
                    int LA9_2 = input.LA(2);

                    if ( (LA9_2==16) ) {
                        int LA9_3 = input.LA(3);

                        if ( (LA9_3==RULE_ID) ) {
                            int LA9_5 = input.LA(4);

                            if ( (LA9_5==15) ) {
                                alt9=1;
                            }


                        }


                    }
                    else if ( (LA9_2==RULE_ID||LA9_2==17) ) {
                        alt9=1;
                    }


                }


                switch (alt9) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:997:2: rule__RootType__SubTypesAssignment_3
            	    {
            	    pushFollow(FOLLOW_rule__RootType__SubTypesAssignment_3_in_rule__RootType__Group__3__Impl2008);
            	    rule__RootType__SubTypesAssignment_3();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop9;
                }
            } while (true);

             after(grammarAccess.getRootTypeAccess().getSubTypesAssignment_3()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__Group__3__Impl


    // $ANTLR start rule__RootType__Group__4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1007:1: rule__RootType__Group__4 : rule__RootType__Group__4__Impl rule__RootType__Group__5 ;
    public final void rule__RootType__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1011:1: ( rule__RootType__Group__4__Impl rule__RootType__Group__5 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1012:2: rule__RootType__Group__4__Impl rule__RootType__Group__5
            {
            pushFollow(FOLLOW_rule__RootType__Group__4__Impl_in_rule__RootType__Group__42039);
            rule__RootType__Group__4__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__RootType__Group__5_in_rule__RootType__Group__42042);
            rule__RootType__Group__5();
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
    // $ANTLR end rule__RootType__Group__4


    // $ANTLR start rule__RootType__Group__4__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1019:1: rule__RootType__Group__4__Impl : ( ( rule__RootType__AtAssignment_4 )* ) ;
    public final void rule__RootType__Group__4__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1023:1: ( ( ( rule__RootType__AtAssignment_4 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1024:1: ( ( rule__RootType__AtAssignment_4 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1024:1: ( ( rule__RootType__AtAssignment_4 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1025:1: ( rule__RootType__AtAssignment_4 )*
            {
             before(grammarAccess.getRootTypeAccess().getAtAssignment_4()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1026:1: ( rule__RootType__AtAssignment_4 )*
            loop10:
            do {
                int alt10=2;
                int LA10_0 = input.LA(1);

                if ( (LA10_0==RULE_ID) ) {
                    alt10=1;
                }


                switch (alt10) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1026:2: rule__RootType__AtAssignment_4
            	    {
            	    pushFollow(FOLLOW_rule__RootType__AtAssignment_4_in_rule__RootType__Group__4__Impl2069);
            	    rule__RootType__AtAssignment_4();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop10;
                }
            } while (true);

             after(grammarAccess.getRootTypeAccess().getAtAssignment_4()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__Group__4__Impl


    // $ANTLR start rule__RootType__Group__5
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1036:1: rule__RootType__Group__5 : rule__RootType__Group__5__Impl rule__RootType__Group__6 ;
    public final void rule__RootType__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1040:1: ( rule__RootType__Group__5__Impl rule__RootType__Group__6 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1041:2: rule__RootType__Group__5__Impl rule__RootType__Group__6
            {
            pushFollow(FOLLOW_rule__RootType__Group__5__Impl_in_rule__RootType__Group__52100);
            rule__RootType__Group__5__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__RootType__Group__6_in_rule__RootType__Group__52103);
            rule__RootType__Group__6();
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
    // $ANTLR end rule__RootType__Group__5


    // $ANTLR start rule__RootType__Group__5__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1048:1: rule__RootType__Group__5__Impl : ( ( rule__RootType__OkpreprocAssignment_5 )* ) ;
    public final void rule__RootType__Group__5__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1052:1: ( ( ( rule__RootType__OkpreprocAssignment_5 )* ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1053:1: ( ( rule__RootType__OkpreprocAssignment_5 )* )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1053:1: ( ( rule__RootType__OkpreprocAssignment_5 )* )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1054:1: ( rule__RootType__OkpreprocAssignment_5 )*
            {
             before(grammarAccess.getRootTypeAccess().getOkpreprocAssignment_5()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1055:1: ( rule__RootType__OkpreprocAssignment_5 )*
            loop11:
            do {
                int alt11=2;
                int LA11_0 = input.LA(1);

                if ( (LA11_0==12) ) {
                    alt11=1;
                }


                switch (alt11) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1055:2: rule__RootType__OkpreprocAssignment_5
            	    {
            	    pushFollow(FOLLOW_rule__RootType__OkpreprocAssignment_5_in_rule__RootType__Group__5__Impl2130);
            	    rule__RootType__OkpreprocAssignment_5();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop11;
                }
            } while (true);

             after(grammarAccess.getRootTypeAccess().getOkpreprocAssignment_5()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__Group__5__Impl


    // $ANTLR start rule__RootType__Group__6
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1065:1: rule__RootType__Group__6 : rule__RootType__Group__6__Impl rule__RootType__Group__7 ;
    public final void rule__RootType__Group__6() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1069:1: ( rule__RootType__Group__6__Impl rule__RootType__Group__7 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1070:2: rule__RootType__Group__6__Impl rule__RootType__Group__7
            {
            pushFollow(FOLLOW_rule__RootType__Group__6__Impl_in_rule__RootType__Group__62161);
            rule__RootType__Group__6__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__RootType__Group__7_in_rule__RootType__Group__62164);
            rule__RootType__Group__7();
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
    // $ANTLR end rule__RootType__Group__6


    // $ANTLR start rule__RootType__Group__6__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1077:1: rule__RootType__Group__6__Impl : ( '[' ) ;
    public final void rule__RootType__Group__6__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1081:1: ( ( '[' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1082:1: ( '[' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1082:1: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1083:1: '['
            {
             before(grammarAccess.getRootTypeAccess().getLeftSquareBracketKeyword_6()); 
            match(input,14,FOLLOW_14_in_rule__RootType__Group__6__Impl2192); 
             after(grammarAccess.getRootTypeAccess().getLeftSquareBracketKeyword_6()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__Group__6__Impl


    // $ANTLR start rule__RootType__Group__7
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1096:1: rule__RootType__Group__7 : rule__RootType__Group__7__Impl rule__RootType__Group__8 ;
    public final void rule__RootType__Group__7() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1100:1: ( rule__RootType__Group__7__Impl rule__RootType__Group__8 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1101:2: rule__RootType__Group__7__Impl rule__RootType__Group__8
            {
            pushFollow(FOLLOW_rule__RootType__Group__7__Impl_in_rule__RootType__Group__72223);
            rule__RootType__Group__7__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__RootType__Group__8_in_rule__RootType__Group__72226);
            rule__RootType__Group__8();
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
    // $ANTLR end rule__RootType__Group__7


    // $ANTLR start rule__RootType__Group__7__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1108:1: rule__RootType__Group__7__Impl : ( '/' ) ;
    public final void rule__RootType__Group__7__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1112:1: ( ( '/' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1113:1: ( '/' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1113:1: ( '/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1114:1: '/'
            {
             before(grammarAccess.getRootTypeAccess().getSolidusKeyword_7()); 
            match(input,16,FOLLOW_16_in_rule__RootType__Group__7__Impl2254); 
             after(grammarAccess.getRootTypeAccess().getSolidusKeyword_7()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__Group__7__Impl


    // $ANTLR start rule__RootType__Group__8
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1127:1: rule__RootType__Group__8 : rule__RootType__Group__8__Impl rule__RootType__Group__9 ;
    public final void rule__RootType__Group__8() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1131:1: ( rule__RootType__Group__8__Impl rule__RootType__Group__9 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1132:2: rule__RootType__Group__8__Impl rule__RootType__Group__9
            {
            pushFollow(FOLLOW_rule__RootType__Group__8__Impl_in_rule__RootType__Group__82285);
            rule__RootType__Group__8__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__RootType__Group__9_in_rule__RootType__Group__82288);
            rule__RootType__Group__9();
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
    // $ANTLR end rule__RootType__Group__8


    // $ANTLR start rule__RootType__Group__8__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1139:1: rule__RootType__Group__8__Impl : ( ( rule__RootType__EndTagAssignment_8 ) ) ;
    public final void rule__RootType__Group__8__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1143:1: ( ( ( rule__RootType__EndTagAssignment_8 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1144:1: ( ( rule__RootType__EndTagAssignment_8 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1144:1: ( ( rule__RootType__EndTagAssignment_8 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1145:1: ( rule__RootType__EndTagAssignment_8 )
            {
             before(grammarAccess.getRootTypeAccess().getEndTagAssignment_8()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1146:1: ( rule__RootType__EndTagAssignment_8 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1146:2: rule__RootType__EndTagAssignment_8
            {
            pushFollow(FOLLOW_rule__RootType__EndTagAssignment_8_in_rule__RootType__Group__8__Impl2315);
            rule__RootType__EndTagAssignment_8();
            _fsp--;


            }

             after(grammarAccess.getRootTypeAccess().getEndTagAssignment_8()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__Group__8__Impl


    // $ANTLR start rule__RootType__Group__9
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1156:1: rule__RootType__Group__9 : rule__RootType__Group__9__Impl ;
    public final void rule__RootType__Group__9() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1160:1: ( rule__RootType__Group__9__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1161:2: rule__RootType__Group__9__Impl
            {
            pushFollow(FOLLOW_rule__RootType__Group__9__Impl_in_rule__RootType__Group__92345);
            rule__RootType__Group__9__Impl();
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
    // $ANTLR end rule__RootType__Group__9


    // $ANTLR start rule__RootType__Group__9__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1167:1: rule__RootType__Group__9__Impl : ( ']' ) ;
    public final void rule__RootType__Group__9__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1171:1: ( ( ']' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1172:1: ( ']' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1172:1: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1173:1: ']'
            {
             before(grammarAccess.getRootTypeAccess().getRightSquareBracketKeyword_9()); 
            match(input,15,FOLLOW_15_in_rule__RootType__Group__9__Impl2373); 
             after(grammarAccess.getRootTypeAccess().getRightSquareBracketKeyword_9()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__Group__9__Impl


    // $ANTLR start rule__SimpleTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1206:1: rule__SimpleTag__Group__0 : rule__SimpleTag__Group__0__Impl rule__SimpleTag__Group__1 ;
    public final void rule__SimpleTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1210:1: ( rule__SimpleTag__Group__0__Impl rule__SimpleTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1211:2: rule__SimpleTag__Group__0__Impl rule__SimpleTag__Group__1
            {
            pushFollow(FOLLOW_rule__SimpleTag__Group__0__Impl_in_rule__SimpleTag__Group__02424);
            rule__SimpleTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__SimpleTag__Group__1_in_rule__SimpleTag__Group__02427);
            rule__SimpleTag__Group__1();
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
    // $ANTLR end rule__SimpleTag__Group__0


    // $ANTLR start rule__SimpleTag__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1218:1: rule__SimpleTag__Group__0__Impl : ( ( rule__SimpleTag__EndTagAssignment_0 )? ) ;
    public final void rule__SimpleTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1222:1: ( ( ( rule__SimpleTag__EndTagAssignment_0 )? ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1223:1: ( ( rule__SimpleTag__EndTagAssignment_0 )? )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1223:1: ( ( rule__SimpleTag__EndTagAssignment_0 )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1224:1: ( rule__SimpleTag__EndTagAssignment_0 )?
            {
             before(grammarAccess.getSimpleTagAccess().getEndTagAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1225:1: ( rule__SimpleTag__EndTagAssignment_0 )?
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0==16) ) {
                alt12=1;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1225:2: rule__SimpleTag__EndTagAssignment_0
                    {
                    pushFollow(FOLLOW_rule__SimpleTag__EndTagAssignment_0_in_rule__SimpleTag__Group__0__Impl2454);
                    rule__SimpleTag__EndTagAssignment_0();
                    _fsp--;


                    }
                    break;

            }

             after(grammarAccess.getSimpleTagAccess().getEndTagAssignment_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__SimpleTag__Group__0__Impl


    // $ANTLR start rule__SimpleTag__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1235:1: rule__SimpleTag__Group__1 : rule__SimpleTag__Group__1__Impl ;
    public final void rule__SimpleTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1239:1: ( rule__SimpleTag__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1240:2: rule__SimpleTag__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__SimpleTag__Group__1__Impl_in_rule__SimpleTag__Group__12485);
            rule__SimpleTag__Group__1__Impl();
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
    // $ANTLR end rule__SimpleTag__Group__1


    // $ANTLR start rule__SimpleTag__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1246:1: rule__SimpleTag__Group__1__Impl : ( ( rule__SimpleTag__TagNameAssignment_1 ) ) ;
    public final void rule__SimpleTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1250:1: ( ( ( rule__SimpleTag__TagNameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1251:1: ( ( rule__SimpleTag__TagNameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1251:1: ( ( rule__SimpleTag__TagNameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1252:1: ( rule__SimpleTag__TagNameAssignment_1 )
            {
             before(grammarAccess.getSimpleTagAccess().getTagNameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1253:1: ( rule__SimpleTag__TagNameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1253:2: rule__SimpleTag__TagNameAssignment_1
            {
            pushFollow(FOLLOW_rule__SimpleTag__TagNameAssignment_1_in_rule__SimpleTag__Group__1__Impl2512);
            rule__SimpleTag__TagNameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getSimpleTagAccess().getTagNameAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__SimpleTag__Group__1__Impl


    // $ANTLR start rule__AddedTag__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1267:1: rule__AddedTag__Group__0 : rule__AddedTag__Group__0__Impl rule__AddedTag__Group__1 ;
    public final void rule__AddedTag__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1271:1: ( rule__AddedTag__Group__0__Impl rule__AddedTag__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1272:2: rule__AddedTag__Group__0__Impl rule__AddedTag__Group__1
            {
            pushFollow(FOLLOW_rule__AddedTag__Group__0__Impl_in_rule__AddedTag__Group__02546);
            rule__AddedTag__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__AddedTag__Group__1_in_rule__AddedTag__Group__02549);
            rule__AddedTag__Group__1();
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
    // $ANTLR end rule__AddedTag__Group__0


    // $ANTLR start rule__AddedTag__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1279:1: rule__AddedTag__Group__0__Impl : ( '+' ) ;
    public final void rule__AddedTag__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1283:1: ( ( '+' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1284:1: ( '+' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1284:1: ( '+' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1285:1: '+'
            {
             before(grammarAccess.getAddedTagAccess().getPlusSignKeyword_0()); 
            match(input,17,FOLLOW_17_in_rule__AddedTag__Group__0__Impl2577); 
             after(grammarAccess.getAddedTagAccess().getPlusSignKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__AddedTag__Group__0__Impl


    // $ANTLR start rule__AddedTag__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1298:1: rule__AddedTag__Group__1 : rule__AddedTag__Group__1__Impl ;
    public final void rule__AddedTag__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1302:1: ( rule__AddedTag__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1303:2: rule__AddedTag__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__AddedTag__Group__1__Impl_in_rule__AddedTag__Group__12608);
            rule__AddedTag__Group__1__Impl();
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
    // $ANTLR end rule__AddedTag__Group__1


    // $ANTLR start rule__AddedTag__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1309:1: rule__AddedTag__Group__1__Impl : ( ( rule__AddedTag__TagNameAssignment_1 ) ) ;
    public final void rule__AddedTag__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1313:1: ( ( ( rule__AddedTag__TagNameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1314:1: ( ( rule__AddedTag__TagNameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1314:1: ( ( rule__AddedTag__TagNameAssignment_1 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1315:1: ( rule__AddedTag__TagNameAssignment_1 )
            {
             before(grammarAccess.getAddedTagAccess().getTagNameAssignment_1()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1316:1: ( rule__AddedTag__TagNameAssignment_1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1316:2: rule__AddedTag__TagNameAssignment_1
            {
            pushFollow(FOLLOW_rule__AddedTag__TagNameAssignment_1_in_rule__AddedTag__Group__1__Impl2635);
            rule__AddedTag__TagNameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getAddedTagAccess().getTagNameAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__AddedTag__Group__1__Impl


    // $ANTLR start rule__Attributes__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1330:1: rule__Attributes__Group__0 : rule__Attributes__Group__0__Impl rule__Attributes__Group__1 ;
    public final void rule__Attributes__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1334:1: ( rule__Attributes__Group__0__Impl rule__Attributes__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1335:2: rule__Attributes__Group__0__Impl rule__Attributes__Group__1
            {
            pushFollow(FOLLOW_rule__Attributes__Group__0__Impl_in_rule__Attributes__Group__02669);
            rule__Attributes__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__Attributes__Group__1_in_rule__Attributes__Group__02672);
            rule__Attributes__Group__1();
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
    // $ANTLR end rule__Attributes__Group__0


    // $ANTLR start rule__Attributes__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1342:1: rule__Attributes__Group__0__Impl : ( ( rule__Attributes__AttrNameAssignment_0 ) ) ;
    public final void rule__Attributes__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1346:1: ( ( ( rule__Attributes__AttrNameAssignment_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1347:1: ( ( rule__Attributes__AttrNameAssignment_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1347:1: ( ( rule__Attributes__AttrNameAssignment_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1348:1: ( rule__Attributes__AttrNameAssignment_0 )
            {
             before(grammarAccess.getAttributesAccess().getAttrNameAssignment_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1349:1: ( rule__Attributes__AttrNameAssignment_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1349:2: rule__Attributes__AttrNameAssignment_0
            {
            pushFollow(FOLLOW_rule__Attributes__AttrNameAssignment_0_in_rule__Attributes__Group__0__Impl2699);
            rule__Attributes__AttrNameAssignment_0();
            _fsp--;


            }

             after(grammarAccess.getAttributesAccess().getAttrNameAssignment_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Attributes__Group__0__Impl


    // $ANTLR start rule__Attributes__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1359:1: rule__Attributes__Group__1 : rule__Attributes__Group__1__Impl rule__Attributes__Group__2 ;
    public final void rule__Attributes__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1363:1: ( rule__Attributes__Group__1__Impl rule__Attributes__Group__2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1364:2: rule__Attributes__Group__1__Impl rule__Attributes__Group__2
            {
            pushFollow(FOLLOW_rule__Attributes__Group__1__Impl_in_rule__Attributes__Group__12729);
            rule__Attributes__Group__1__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__Attributes__Group__2_in_rule__Attributes__Group__12732);
            rule__Attributes__Group__2();
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
    // $ANTLR end rule__Attributes__Group__1


    // $ANTLR start rule__Attributes__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1371:1: rule__Attributes__Group__1__Impl : ( '=' ) ;
    public final void rule__Attributes__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1375:1: ( ( '=' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1376:1: ( '=' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1376:1: ( '=' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1377:1: '='
            {
             before(grammarAccess.getAttributesAccess().getEqualsSignKeyword_1()); 
            match(input,18,FOLLOW_18_in_rule__Attributes__Group__1__Impl2760); 
             after(grammarAccess.getAttributesAccess().getEqualsSignKeyword_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Attributes__Group__1__Impl


    // $ANTLR start rule__Attributes__Group__2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1390:1: rule__Attributes__Group__2 : rule__Attributes__Group__2__Impl ;
    public final void rule__Attributes__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1394:1: ( rule__Attributes__Group__2__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1395:2: rule__Attributes__Group__2__Impl
            {
            pushFollow(FOLLOW_rule__Attributes__Group__2__Impl_in_rule__Attributes__Group__22791);
            rule__Attributes__Group__2__Impl();
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
    // $ANTLR end rule__Attributes__Group__2


    // $ANTLR start rule__Attributes__Group__2__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1401:1: rule__Attributes__Group__2__Impl : ( ( rule__Attributes__AttrValueAssignment_2 ) ) ;
    public final void rule__Attributes__Group__2__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1405:1: ( ( ( rule__Attributes__AttrValueAssignment_2 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1406:1: ( ( rule__Attributes__AttrValueAssignment_2 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1406:1: ( ( rule__Attributes__AttrValueAssignment_2 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1407:1: ( rule__Attributes__AttrValueAssignment_2 )
            {
             before(grammarAccess.getAttributesAccess().getAttrValueAssignment_2()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1408:1: ( rule__Attributes__AttrValueAssignment_2 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1408:2: rule__Attributes__AttrValueAssignment_2
            {
            pushFollow(FOLLOW_rule__Attributes__AttrValueAssignment_2_in_rule__Attributes__Group__2__Impl2818);
            rule__Attributes__AttrValueAssignment_2();
            _fsp--;


            }

             after(grammarAccess.getAttributesAccess().getAttrValueAssignment_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Attributes__Group__2__Impl


    // $ANTLR start rule__TSTRING__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1424:1: rule__TSTRING__Group__0 : rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 ;
    public final void rule__TSTRING__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1428:1: ( rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1429:2: rule__TSTRING__Group__0__Impl rule__TSTRING__Group__1
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__02854);
            rule__TSTRING__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__02857);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1436:1: rule__TSTRING__Group__0__Impl : ( '_' ) ;
    public final void rule__TSTRING__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1440:1: ( ( '_' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1441:1: ( '_' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1441:1: ( '_' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1442:1: '_'
            {
             before(grammarAccess.getTSTRINGAccess().get_Keyword_0()); 
            match(input,19,FOLLOW_19_in_rule__TSTRING__Group__0__Impl2885); 
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1455:1: rule__TSTRING__Group__1 : rule__TSTRING__Group__1__Impl ;
    public final void rule__TSTRING__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1459:1: ( rule__TSTRING__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1460:2: rule__TSTRING__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__12916);
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
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1466:1: rule__TSTRING__Group__1__Impl : ( RULE_STRING ) ;
    public final void rule__TSTRING__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1470:1: ( ( RULE_STRING ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1471:1: ( RULE_STRING )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1471:1: ( RULE_STRING )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1472:1: RULE_STRING
            {
             before(grammarAccess.getTSTRINGAccess().getSTRINGTerminalRuleCall_1()); 
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl2943); 
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


    // $ANTLR start rule__HOMEPATH__Group__0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1487:1: rule__HOMEPATH__Group__0 : rule__HOMEPATH__Group__0__Impl rule__HOMEPATH__Group__1 ;
    public final void rule__HOMEPATH__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1491:1: ( rule__HOMEPATH__Group__0__Impl rule__HOMEPATH__Group__1 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1492:2: rule__HOMEPATH__Group__0__Impl rule__HOMEPATH__Group__1
            {
            pushFollow(FOLLOW_rule__HOMEPATH__Group__0__Impl_in_rule__HOMEPATH__Group__02976);
            rule__HOMEPATH__Group__0__Impl();
            _fsp--;

            pushFollow(FOLLOW_rule__HOMEPATH__Group__1_in_rule__HOMEPATH__Group__02979);
            rule__HOMEPATH__Group__1();
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
    // $ANTLR end rule__HOMEPATH__Group__0


    // $ANTLR start rule__HOMEPATH__Group__0__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1499:1: rule__HOMEPATH__Group__0__Impl : ( '~' ) ;
    public final void rule__HOMEPATH__Group__0__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1503:1: ( ( '~' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1504:1: ( '~' )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1504:1: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1505:1: '~'
            {
             before(grammarAccess.getHOMEPATHAccess().getTildeKeyword_0()); 
            match(input,20,FOLLOW_20_in_rule__HOMEPATH__Group__0__Impl3007); 
             after(grammarAccess.getHOMEPATHAccess().getTildeKeyword_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__HOMEPATH__Group__0__Impl


    // $ANTLR start rule__HOMEPATH__Group__1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1518:1: rule__HOMEPATH__Group__1 : rule__HOMEPATH__Group__1__Impl ;
    public final void rule__HOMEPATH__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1522:1: ( rule__HOMEPATH__Group__1__Impl )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1523:2: rule__HOMEPATH__Group__1__Impl
            {
            pushFollow(FOLLOW_rule__HOMEPATH__Group__1__Impl_in_rule__HOMEPATH__Group__13038);
            rule__HOMEPATH__Group__1__Impl();
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
    // $ANTLR end rule__HOMEPATH__Group__1


    // $ANTLR start rule__HOMEPATH__Group__1__Impl
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1529:1: rule__HOMEPATH__Group__1__Impl : ( RULE_PATH ) ;
    public final void rule__HOMEPATH__Group__1__Impl() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1533:1: ( ( RULE_PATH ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1534:1: ( RULE_PATH )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1534:1: ( RULE_PATH )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1535:1: RULE_PATH
            {
             before(grammarAccess.getHOMEPATHAccess().getPATHTerminalRuleCall_1()); 
            match(input,RULE_PATH,FOLLOW_RULE_PATH_in_rule__HOMEPATH__Group__1__Impl3065); 
             after(grammarAccess.getHOMEPATHAccess().getPATHTerminalRuleCall_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__HOMEPATH__Group__1__Impl


    // $ANTLR start rule__Root__TextdomainsAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1551:1: rule__Root__TextdomainsAssignment_0 : ( ruleTextDomain ) ;
    public final void rule__Root__TextdomainsAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1555:1: ( ( ruleTextDomain ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1556:1: ( ruleTextDomain )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1556:1: ( ruleTextDomain )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1557:1: ruleTextDomain
            {
             before(grammarAccess.getRootAccess().getTextdomainsTextDomainParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleTextDomain_in_rule__Root__TextdomainsAssignment_03103);
            ruleTextDomain();
            _fsp--;

             after(grammarAccess.getRootAccess().getTextdomainsTextDomainParserRuleCall_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Root__TextdomainsAssignment_0


    // $ANTLR start rule__Root__PreprocAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1566:1: rule__Root__PreprocAssignment_1 : ( rulePreprocessor ) ;
    public final void rule__Root__PreprocAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1570:1: ( ( rulePreprocessor ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1571:1: ( rulePreprocessor )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1571:1: ( rulePreprocessor )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1572:1: rulePreprocessor
            {
             before(grammarAccess.getRootAccess().getPreprocPreprocessorParserRuleCall_1_0()); 
            pushFollow(FOLLOW_rulePreprocessor_in_rule__Root__PreprocAssignment_13134);
            rulePreprocessor();
            _fsp--;

             after(grammarAccess.getRootAccess().getPreprocPreprocessorParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Root__PreprocAssignment_1


    // $ANTLR start rule__Root__RootsAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1581:1: rule__Root__RootsAssignment_2 : ( ruleRootType ) ;
    public final void rule__Root__RootsAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1585:1: ( ( ruleRootType ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1586:1: ( ruleRootType )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1586:1: ( ruleRootType )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1587:1: ruleRootType
            {
             before(grammarAccess.getRootAccess().getRootsRootTypeParserRuleCall_2_0()); 
            pushFollow(FOLLOW_ruleRootType_in_rule__Root__RootsAssignment_23165);
            ruleRootType();
            _fsp--;

             after(grammarAccess.getRootAccess().getRootsRootTypeParserRuleCall_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Root__RootsAssignment_2


    // $ANTLR start rule__TextDomain__DomainNameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1596:1: rule__TextDomain__DomainNameAssignment_1 : ( RULE_ID ) ;
    public final void rule__TextDomain__DomainNameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1600:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1601:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1601:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1602:1: RULE_ID
            {
             before(grammarAccess.getTextDomainAccess().getDomainNameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__TextDomain__DomainNameAssignment_13196); 
             after(grammarAccess.getTextDomainAccess().getDomainNameIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__TextDomain__DomainNameAssignment_1


    // $ANTLR start rule__Macro__MacroContentAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1611:1: rule__Macro__MacroContentAssignment_1 : ( RULE_ID ) ;
    public final void rule__Macro__MacroContentAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1615:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1616:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1616:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1617:1: RULE_ID
            {
             before(grammarAccess.getMacroAccess().getMacroContentIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__Macro__MacroContentAssignment_13227); 
             after(grammarAccess.getMacroAccess().getMacroContentIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Macro__MacroContentAssignment_1


    // $ANTLR start rule__PathInclude__PathAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1626:1: rule__PathInclude__PathAssignment_1 : ( ( rule__PathInclude__PathAlternatives_1_0 ) ) ;
    public final void rule__PathInclude__PathAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1630:1: ( ( ( rule__PathInclude__PathAlternatives_1_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1631:1: ( ( rule__PathInclude__PathAlternatives_1_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1631:1: ( ( rule__PathInclude__PathAlternatives_1_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1632:1: ( rule__PathInclude__PathAlternatives_1_0 )
            {
             before(grammarAccess.getPathIncludeAccess().getPathAlternatives_1_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1633:1: ( rule__PathInclude__PathAlternatives_1_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1633:2: rule__PathInclude__PathAlternatives_1_0
            {
            pushFollow(FOLLOW_rule__PathInclude__PathAlternatives_1_0_in_rule__PathInclude__PathAssignment_13258);
            rule__PathInclude__PathAlternatives_1_0();
            _fsp--;


            }

             after(grammarAccess.getPathIncludeAccess().getPathAlternatives_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__PathInclude__PathAssignment_1


    // $ANTLR start rule__RootType__StartTagAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1642:1: rule__RootType__StartTagAssignment_1 : ( ruleRootTag ) ;
    public final void rule__RootType__StartTagAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1646:1: ( ( ruleRootTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1647:1: ( ruleRootTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1647:1: ( ruleRootTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1648:1: ruleRootTag
            {
             before(grammarAccess.getRootTypeAccess().getStartTagRootTagParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleRootTag_in_rule__RootType__StartTagAssignment_13291);
            ruleRootTag();
            _fsp--;

             after(grammarAccess.getRootTypeAccess().getStartTagRootTagParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__StartTagAssignment_1


    // $ANTLR start rule__RootType__SubTypesAssignment_3
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1657:1: rule__RootType__SubTypesAssignment_3 : ( ruleRootType ) ;
    public final void rule__RootType__SubTypesAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1661:1: ( ( ruleRootType ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1662:1: ( ruleRootType )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1662:1: ( ruleRootType )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1663:1: ruleRootType
            {
             before(grammarAccess.getRootTypeAccess().getSubTypesRootTypeParserRuleCall_3_0()); 
            pushFollow(FOLLOW_ruleRootType_in_rule__RootType__SubTypesAssignment_33322);
            ruleRootType();
            _fsp--;

             after(grammarAccess.getRootTypeAccess().getSubTypesRootTypeParserRuleCall_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__SubTypesAssignment_3


    // $ANTLR start rule__RootType__AtAssignment_4
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1672:1: rule__RootType__AtAssignment_4 : ( ruleAttributes ) ;
    public final void rule__RootType__AtAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1676:1: ( ( ruleAttributes ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1677:1: ( ruleAttributes )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1677:1: ( ruleAttributes )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1678:1: ruleAttributes
            {
             before(grammarAccess.getRootTypeAccess().getAtAttributesParserRuleCall_4_0()); 
            pushFollow(FOLLOW_ruleAttributes_in_rule__RootType__AtAssignment_43353);
            ruleAttributes();
            _fsp--;

             after(grammarAccess.getRootTypeAccess().getAtAttributesParserRuleCall_4_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__AtAssignment_4


    // $ANTLR start rule__RootType__OkpreprocAssignment_5
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1687:1: rule__RootType__OkpreprocAssignment_5 : ( rulePreprocessor ) ;
    public final void rule__RootType__OkpreprocAssignment_5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1691:1: ( ( rulePreprocessor ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1692:1: ( rulePreprocessor )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1692:1: ( rulePreprocessor )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1693:1: rulePreprocessor
            {
             before(grammarAccess.getRootTypeAccess().getOkpreprocPreprocessorParserRuleCall_5_0()); 
            pushFollow(FOLLOW_rulePreprocessor_in_rule__RootType__OkpreprocAssignment_53384);
            rulePreprocessor();
            _fsp--;

             after(grammarAccess.getRootTypeAccess().getOkpreprocPreprocessorParserRuleCall_5_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__OkpreprocAssignment_5


    // $ANTLR start rule__RootType__EndTagAssignment_8
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1702:1: rule__RootType__EndTagAssignment_8 : ( ruleRootTag ) ;
    public final void rule__RootType__EndTagAssignment_8() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1706:1: ( ( ruleRootTag ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1707:1: ( ruleRootTag )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1707:1: ( ruleRootTag )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1708:1: ruleRootTag
            {
             before(grammarAccess.getRootTypeAccess().getEndTagRootTagParserRuleCall_8_0()); 
            pushFollow(FOLLOW_ruleRootTag_in_rule__RootType__EndTagAssignment_83415);
            ruleRootTag();
            _fsp--;

             after(grammarAccess.getRootTypeAccess().getEndTagRootTagParserRuleCall_8_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__RootType__EndTagAssignment_8


    // $ANTLR start rule__SimpleTag__EndTagAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1717:1: rule__SimpleTag__EndTagAssignment_0 : ( ( '/' ) ) ;
    public final void rule__SimpleTag__EndTagAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1721:1: ( ( ( '/' ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1722:1: ( ( '/' ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1722:1: ( ( '/' ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1723:1: ( '/' )
            {
             before(grammarAccess.getSimpleTagAccess().getEndTagSolidusKeyword_0_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1724:1: ( '/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1725:1: '/'
            {
             before(grammarAccess.getSimpleTagAccess().getEndTagSolidusKeyword_0_0()); 
            match(input,16,FOLLOW_16_in_rule__SimpleTag__EndTagAssignment_03451); 
             after(grammarAccess.getSimpleTagAccess().getEndTagSolidusKeyword_0_0()); 

            }

             after(grammarAccess.getSimpleTagAccess().getEndTagSolidusKeyword_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__SimpleTag__EndTagAssignment_0


    // $ANTLR start rule__SimpleTag__TagNameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1740:1: rule__SimpleTag__TagNameAssignment_1 : ( ruleRootTagsList ) ;
    public final void rule__SimpleTag__TagNameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1744:1: ( ( ruleRootTagsList ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1745:1: ( ruleRootTagsList )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1745:1: ( ruleRootTagsList )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1746:1: ruleRootTagsList
            {
             before(grammarAccess.getSimpleTagAccess().getTagNameRootTagsListParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleRootTagsList_in_rule__SimpleTag__TagNameAssignment_13490);
            ruleRootTagsList();
            _fsp--;

             after(grammarAccess.getSimpleTagAccess().getTagNameRootTagsListParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__SimpleTag__TagNameAssignment_1


    // $ANTLR start rule__AddedTag__TagNameAssignment_1
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1755:1: rule__AddedTag__TagNameAssignment_1 : ( ruleRootTagsList ) ;
    public final void rule__AddedTag__TagNameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1759:1: ( ( ruleRootTagsList ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1760:1: ( ruleRootTagsList )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1760:1: ( ruleRootTagsList )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1761:1: ruleRootTagsList
            {
             before(grammarAccess.getAddedTagAccess().getTagNameRootTagsListParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleRootTagsList_in_rule__AddedTag__TagNameAssignment_13521);
            ruleRootTagsList();
            _fsp--;

             after(grammarAccess.getAddedTagAccess().getTagNameRootTagsListParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__AddedTag__TagNameAssignment_1


    // $ANTLR start rule__Attributes__AttrNameAssignment_0
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1770:1: rule__Attributes__AttrNameAssignment_0 : ( RULE_ID ) ;
    public final void rule__Attributes__AttrNameAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1774:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1775:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1775:1: ( RULE_ID )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1776:1: RULE_ID
            {
             before(grammarAccess.getAttributesAccess().getAttrNameIDTerminalRuleCall_0_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__Attributes__AttrNameAssignment_03552); 
             after(grammarAccess.getAttributesAccess().getAttrNameIDTerminalRuleCall_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Attributes__AttrNameAssignment_0


    // $ANTLR start rule__Attributes__AttrValueAssignment_2
    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1785:1: rule__Attributes__AttrValueAssignment_2 : ( ( rule__Attributes__AttrValueAlternatives_2_0 ) ) ;
    public final void rule__Attributes__AttrValueAssignment_2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1789:1: ( ( ( rule__Attributes__AttrValueAlternatives_2_0 ) ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1790:1: ( ( rule__Attributes__AttrValueAlternatives_2_0 ) )
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1790:1: ( ( rule__Attributes__AttrValueAlternatives_2_0 ) )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1791:1: ( rule__Attributes__AttrValueAlternatives_2_0 )
            {
             before(grammarAccess.getAttributesAccess().getAttrValueAlternatives_2_0()); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1792:1: ( rule__Attributes__AttrValueAlternatives_2_0 )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1792:2: rule__Attributes__AttrValueAlternatives_2_0
            {
            pushFollow(FOLLOW_rule__Attributes__AttrValueAlternatives_2_0_in_rule__Attributes__AttrValueAssignment_23583);
            rule__Attributes__AttrValueAlternatives_2_0();
            _fsp--;


            }

             after(grammarAccess.getAttributesAccess().getAttrValueAlternatives_2_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Attributes__AttrValueAssignment_2


 

    public static final BitSet FOLLOW_ruleRoot_in_entryRuleRoot61 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleRoot68 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Root__Group__0_in_ruleRoot94 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTextDomain_in_entryRuleTextDomain121 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTextDomain128 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TextDomain__Group__0_in_ruleTextDomain154 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePreprocessor_in_entryRulePreprocessor181 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePreprocessor188 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Preprocessor__Alternatives_in_rulePreprocessor214 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacro_in_entryRuleMacro241 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleMacro248 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Macro__Group__0_in_ruleMacro274 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePathInclude_in_entryRulePathInclude301 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePathInclude308 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PathInclude__Group__0_in_rulePathInclude334 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootType_in_entryRuleRootType361 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleRootType368 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__Group__0_in_ruleRootType394 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootTag_in_entryRuleRootTag421 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleRootTag428 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootTag__Alternatives_in_ruleRootTag454 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleSimpleTag_in_entryRuleSimpleTag481 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleSimpleTag488 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__SimpleTag__Group__0_in_ruleSimpleTag514 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleAddedTag_in_entryRuleAddedTag541 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleAddedTag548 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__AddedTag__Group__0_in_ruleAddedTag574 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootTagsList_in_entryRuleRootTagsList601 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleRootTagsList608 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleRootTagsList634 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleAttributes_in_entryRuleAttributes660 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleAttributes667 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Attributes__Group__0_in_ruleAttributes693 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING720 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING727 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0_in_ruleTSTRING753 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleHOMEPATH_in_entryRuleHOMEPATH780 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleHOMEPATH787 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__HOMEPATH__Group__0_in_ruleHOMEPATH813 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacro_in_rule__Preprocessor__Alternatives849 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePathInclude_in_rule__Preprocessor__Alternatives866 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleHOMEPATH_in_rule__PathInclude__PathAlternatives_1_0898 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_PATH_in_rule__PathInclude__PathAlternatives_1_0915 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleSimpleTag_in_rule__RootTag__Alternatives947 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleAddedTag_in_rule__RootTag__Alternatives964 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_rule__Attributes__AttrValueAlternatives_2_0996 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__Attributes__AttrValueAlternatives_2_01013 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_PATH_in_rule__Attributes__AttrValueAlternatives_2_01030 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__Attributes__AttrValueAlternatives_2_01047 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IDLIST_in_rule__Attributes__AttrValueAlternatives_2_01064 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Root__Group__0__Impl_in_rule__Root__Group__01094 = new BitSet(new long[]{0x0000000000005002L});
    public static final BitSet FOLLOW_rule__Root__Group__1_in_rule__Root__Group__01097 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Root__TextdomainsAssignment_0_in_rule__Root__Group__0__Impl1124 = new BitSet(new long[]{0x0000000000000802L});
    public static final BitSet FOLLOW_rule__Root__Group__1__Impl_in_rule__Root__Group__11155 = new BitSet(new long[]{0x0000000000004002L});
    public static final BitSet FOLLOW_rule__Root__Group__2_in_rule__Root__Group__11158 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Root__PreprocAssignment_1_in_rule__Root__Group__1__Impl1185 = new BitSet(new long[]{0x0000000000001002L});
    public static final BitSet FOLLOW_rule__Root__Group__2__Impl_in_rule__Root__Group__21216 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Root__RootsAssignment_2_in_rule__Root__Group__2__Impl1243 = new BitSet(new long[]{0x0000000000004002L});
    public static final BitSet FOLLOW_rule__TextDomain__Group__0__Impl_in_rule__TextDomain__Group__01280 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__TextDomain__Group__1_in_rule__TextDomain__Group__01283 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_rule__TextDomain__Group__0__Impl1311 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TextDomain__Group__1__Impl_in_rule__TextDomain__Group__11342 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TextDomain__DomainNameAssignment_1_in_rule__TextDomain__Group__1__Impl1369 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Macro__Group__0__Impl_in_rule__Macro__Group__01403 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__Macro__Group__1_in_rule__Macro__Group__01406 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__Macro__Group__0__Impl1434 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Macro__Group__1__Impl_in_rule__Macro__Group__11465 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_rule__Macro__Group__2_in_rule__Macro__Group__11468 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Macro__MacroContentAssignment_1_in_rule__Macro__Group__1__Impl1497 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__Macro__MacroContentAssignment_1_in_rule__Macro__Group__1__Impl1509 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__Macro__Group__2__Impl_in_rule__Macro__Group__21542 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__Macro__Group__2__Impl1570 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PathInclude__Group__0__Impl_in_rule__PathInclude__Group__01607 = new BitSet(new long[]{0x0000000000100020L});
    public static final BitSet FOLLOW_rule__PathInclude__Group__1_in_rule__PathInclude__Group__01610 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__PathInclude__Group__0__Impl1638 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PathInclude__Group__1__Impl_in_rule__PathInclude__Group__11669 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_rule__PathInclude__Group__2_in_rule__PathInclude__Group__11672 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PathInclude__PathAssignment_1_in_rule__PathInclude__Group__1__Impl1699 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PathInclude__Group__2__Impl_in_rule__PathInclude__Group__21729 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__PathInclude__Group__2__Impl1757 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__Group__0__Impl_in_rule__RootType__Group__01794 = new BitSet(new long[]{0x0000000000030010L});
    public static final BitSet FOLLOW_rule__RootType__Group__1_in_rule__RootType__Group__01797 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__RootType__Group__0__Impl1825 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__Group__1__Impl_in_rule__RootType__Group__11856 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_rule__RootType__Group__2_in_rule__RootType__Group__11859 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__StartTagAssignment_1_in_rule__RootType__Group__1__Impl1886 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__Group__2__Impl_in_rule__RootType__Group__21916 = new BitSet(new long[]{0x0000000000005010L});
    public static final BitSet FOLLOW_rule__RootType__Group__3_in_rule__RootType__Group__21919 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__RootType__Group__2__Impl1947 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__Group__3__Impl_in_rule__RootType__Group__31978 = new BitSet(new long[]{0x0000000000005010L});
    public static final BitSet FOLLOW_rule__RootType__Group__4_in_rule__RootType__Group__31981 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__SubTypesAssignment_3_in_rule__RootType__Group__3__Impl2008 = new BitSet(new long[]{0x0000000000004002L});
    public static final BitSet FOLLOW_rule__RootType__Group__4__Impl_in_rule__RootType__Group__42039 = new BitSet(new long[]{0x0000000000005000L});
    public static final BitSet FOLLOW_rule__RootType__Group__5_in_rule__RootType__Group__42042 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__AtAssignment_4_in_rule__RootType__Group__4__Impl2069 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_rule__RootType__Group__5__Impl_in_rule__RootType__Group__52100 = new BitSet(new long[]{0x0000000000004000L});
    public static final BitSet FOLLOW_rule__RootType__Group__6_in_rule__RootType__Group__52103 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__OkpreprocAssignment_5_in_rule__RootType__Group__5__Impl2130 = new BitSet(new long[]{0x0000000000001002L});
    public static final BitSet FOLLOW_rule__RootType__Group__6__Impl_in_rule__RootType__Group__62161 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_rule__RootType__Group__7_in_rule__RootType__Group__62164 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__RootType__Group__6__Impl2192 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__Group__7__Impl_in_rule__RootType__Group__72223 = new BitSet(new long[]{0x0000000000030010L});
    public static final BitSet FOLLOW_rule__RootType__Group__8_in_rule__RootType__Group__72226 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__RootType__Group__7__Impl2254 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__Group__8__Impl_in_rule__RootType__Group__82285 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_rule__RootType__Group__9_in_rule__RootType__Group__82288 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__EndTagAssignment_8_in_rule__RootType__Group__8__Impl2315 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__RootType__Group__9__Impl_in_rule__RootType__Group__92345 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__RootType__Group__9__Impl2373 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__SimpleTag__Group__0__Impl_in_rule__SimpleTag__Group__02424 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__SimpleTag__Group__1_in_rule__SimpleTag__Group__02427 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__SimpleTag__EndTagAssignment_0_in_rule__SimpleTag__Group__0__Impl2454 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__SimpleTag__Group__1__Impl_in_rule__SimpleTag__Group__12485 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__SimpleTag__TagNameAssignment_1_in_rule__SimpleTag__Group__1__Impl2512 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__AddedTag__Group__0__Impl_in_rule__AddedTag__Group__02546 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__AddedTag__Group__1_in_rule__AddedTag__Group__02549 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_rule__AddedTag__Group__0__Impl2577 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__AddedTag__Group__1__Impl_in_rule__AddedTag__Group__12608 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__AddedTag__TagNameAssignment_1_in_rule__AddedTag__Group__1__Impl2635 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Attributes__Group__0__Impl_in_rule__Attributes__Group__02669 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_rule__Attributes__Group__1_in_rule__Attributes__Group__02672 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Attributes__AttrNameAssignment_0_in_rule__Attributes__Group__0__Impl2699 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Attributes__Group__1__Impl_in_rule__Attributes__Group__12729 = new BitSet(new long[]{0x00000000000800F0L});
    public static final BitSet FOLLOW_rule__Attributes__Group__2_in_rule__Attributes__Group__12732 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_rule__Attributes__Group__1__Impl2760 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Attributes__Group__2__Impl_in_rule__Attributes__Group__22791 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Attributes__AttrValueAssignment_2_in_rule__Attributes__Group__2__Impl2818 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__0__Impl_in_rule__TSTRING__Group__02854 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1_in_rule__TSTRING__Group__02857 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_rule__TSTRING__Group__0__Impl2885 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__TSTRING__Group__1__Impl_in_rule__TSTRING__Group__12916 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__TSTRING__Group__1__Impl2943 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__HOMEPATH__Group__0__Impl_in_rule__HOMEPATH__Group__02976 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_rule__HOMEPATH__Group__1_in_rule__HOMEPATH__Group__02979 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__HOMEPATH__Group__0__Impl3007 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__HOMEPATH__Group__1__Impl_in_rule__HOMEPATH__Group__13038 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_PATH_in_rule__HOMEPATH__Group__1__Impl3065 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTextDomain_in_rule__Root__TextdomainsAssignment_03103 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePreprocessor_in_rule__Root__PreprocAssignment_13134 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootType_in_rule__Root__RootsAssignment_23165 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__TextDomain__DomainNameAssignment_13196 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__Macro__MacroContentAssignment_13227 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__PathInclude__PathAlternatives_1_0_in_rule__PathInclude__PathAssignment_13258 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootTag_in_rule__RootType__StartTagAssignment_13291 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootType_in_rule__RootType__SubTypesAssignment_33322 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleAttributes_in_rule__RootType__AtAssignment_43353 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePreprocessor_in_rule__RootType__OkpreprocAssignment_53384 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootTag_in_rule__RootType__EndTagAssignment_83415 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__SimpleTag__EndTagAssignment_03451 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootTagsList_in_rule__SimpleTag__TagNameAssignment_13490 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootTagsList_in_rule__AddedTag__TagNameAssignment_13521 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__Attributes__AttrNameAssignment_03552 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Attributes__AttrValueAlternatives_2_0_in_rule__Attributes__AttrValueAssignment_23583 = new BitSet(new long[]{0x0000000000000002L});

}