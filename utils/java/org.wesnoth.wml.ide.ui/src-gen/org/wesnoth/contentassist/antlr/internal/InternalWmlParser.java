package org.wesnoth.contentassist.antlr.internal; 

import java.io.InputStream;
import org.eclipse.xtext.*;
import org.eclipse.xtext.parser.*;
import org.eclipse.xtext.parser.impl.*;
import org.eclipse.xtext.parsetree.*;
import org.eclipse.emf.ecore.util.EcoreUtil;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.parser.antlr.XtextTokenStream;
import org.eclipse.xtext.parser.antlr.XtextTokenStream.HiddenTokens;
import org.eclipse.xtext.ui.common.editor.contentassist.antlr.internal.AbstractInternalContentAssistParser;
import org.wesnoth.services.WmlGrammarAccess;



import org.antlr.runtime.*;
import java.util.Stack;
import java.util.List;
import java.util.ArrayList;

public class InternalWmlParser extends AbstractInternalContentAssistParser {
    public static final String[] tokenNames = new String[] {
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_STRING", "RULE_ID", "RULE_INT", "RULE_ML_COMMENT", "RULE_SL_COMMENT", "RULE_WS", "RULE_ANY_OTHER", "'import'", "'type'", "'[campaign]'", "'[/campaign]'", "'entity'", "'{'", "'}'", "'extends'", "'property'", "':'", "'[]'"
    };
    public static final int RULE_ID=5;
    public static final int RULE_STRING=4;
    public static final int RULE_ANY_OTHER=10;
    public static final int RULE_INT=6;
    public static final int RULE_WS=9;
    public static final int RULE_SL_COMMENT=8;
    public static final int EOF=-1;
    public static final int RULE_ML_COMMENT=7;

        public InternalWmlParser(TokenStream input) {
            super(input);
        }
        

    public String[] getTokenNames() { return tokenNames; }
    public String getGrammarFileName() { return "../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g"; }


     
     	private WmlGrammarAccess grammarAccess;
     	
        public void setGrammarAccess(WmlGrammarAccess grammarAccess) {
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




    // $ANTLR start entryRuleModel
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:60:1: entryRuleModel : ruleModel EOF ;
    public final void entryRuleModel() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:60:16: ( ruleModel EOF )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:61:1: ruleModel EOF
            {
             before(grammarAccess.getModelRule()); 
            pushFollow(FOLLOW_ruleModel_in_entryRuleModel60);
            ruleModel();
            _fsp--;

             after(grammarAccess.getModelRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleModel67); 

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
    // $ANTLR end entryRuleModel


    // $ANTLR start ruleModel
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:68:1: ruleModel : ( ( rule__Model__Group__0 ) ) ;
    public final void ruleModel() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:72:2: ( ( ( rule__Model__Group__0 ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:73:1: ( ( rule__Model__Group__0 ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:73:1: ( ( rule__Model__Group__0 ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:74:1: ( rule__Model__Group__0 )
            {
             before(grammarAccess.getModelAccess().getGroup()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:75:1: ( rule__Model__Group__0 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:75:2: rule__Model__Group__0
            {
            pushFollow(FOLLOW_rule__Model__Group__0_in_ruleModel94);
            rule__Model__Group__0();
            _fsp--;


            }

             after(grammarAccess.getModelAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleModel


    // $ANTLR start entryRuleImport
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:87:1: entryRuleImport : ruleImport EOF ;
    public final void entryRuleImport() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:87:17: ( ruleImport EOF )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:88:1: ruleImport EOF
            {
             before(grammarAccess.getImportRule()); 
            pushFollow(FOLLOW_ruleImport_in_entryRuleImport120);
            ruleImport();
            _fsp--;

             after(grammarAccess.getImportRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleImport127); 

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
    // $ANTLR end entryRuleImport


    // $ANTLR start ruleImport
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:95:1: ruleImport : ( ( rule__Import__Group__0 ) ) ;
    public final void ruleImport() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:99:2: ( ( ( rule__Import__Group__0 ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:100:1: ( ( rule__Import__Group__0 ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:100:1: ( ( rule__Import__Group__0 ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:101:1: ( rule__Import__Group__0 )
            {
             before(grammarAccess.getImportAccess().getGroup()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:102:1: ( rule__Import__Group__0 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:102:2: rule__Import__Group__0
            {
            pushFollow(FOLLOW_rule__Import__Group__0_in_ruleImport154);
            rule__Import__Group__0();
            _fsp--;


            }

             after(grammarAccess.getImportAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleImport


    // $ANTLR start entryRuleType
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:114:1: entryRuleType : ruleType EOF ;
    public final void entryRuleType() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:114:15: ( ruleType EOF )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:115:1: ruleType EOF
            {
             before(grammarAccess.getTypeRule()); 
            pushFollow(FOLLOW_ruleType_in_entryRuleType180);
            ruleType();
            _fsp--;

             after(grammarAccess.getTypeRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleType187); 

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
    // $ANTLR end entryRuleType


    // $ANTLR start ruleType
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:122:1: ruleType : ( ( rule__Type__Alternatives ) ) ;
    public final void ruleType() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:126:2: ( ( ( rule__Type__Alternatives ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:127:1: ( ( rule__Type__Alternatives ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:127:1: ( ( rule__Type__Alternatives ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:128:1: ( rule__Type__Alternatives )
            {
             before(grammarAccess.getTypeAccess().getAlternatives()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:129:1: ( rule__Type__Alternatives )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:129:2: rule__Type__Alternatives
            {
            pushFollow(FOLLOW_rule__Type__Alternatives_in_ruleType214);
            rule__Type__Alternatives();
            _fsp--;


            }

             after(grammarAccess.getTypeAccess().getAlternatives()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleType


    // $ANTLR start entryRuleSimpleType
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:141:1: entryRuleSimpleType : ruleSimpleType EOF ;
    public final void entryRuleSimpleType() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:141:21: ( ruleSimpleType EOF )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:142:1: ruleSimpleType EOF
            {
             before(grammarAccess.getSimpleTypeRule()); 
            pushFollow(FOLLOW_ruleSimpleType_in_entryRuleSimpleType240);
            ruleSimpleType();
            _fsp--;

             after(grammarAccess.getSimpleTypeRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleSimpleType247); 

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
    // $ANTLR end entryRuleSimpleType


    // $ANTLR start ruleSimpleType
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:149:1: ruleSimpleType : ( ( rule__SimpleType__Group__0 ) ) ;
    public final void ruleSimpleType() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:153:2: ( ( ( rule__SimpleType__Group__0 ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:154:1: ( ( rule__SimpleType__Group__0 ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:154:1: ( ( rule__SimpleType__Group__0 ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:155:1: ( rule__SimpleType__Group__0 )
            {
             before(grammarAccess.getSimpleTypeAccess().getGroup()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:156:1: ( rule__SimpleType__Group__0 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:156:2: rule__SimpleType__Group__0
            {
            pushFollow(FOLLOW_rule__SimpleType__Group__0_in_ruleSimpleType274);
            rule__SimpleType__Group__0();
            _fsp--;


            }

             after(grammarAccess.getSimpleTypeAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleSimpleType


    // $ANTLR start entryRuleCampaignType
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:168:1: entryRuleCampaignType : ruleCampaignType EOF ;
    public final void entryRuleCampaignType() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:168:23: ( ruleCampaignType EOF )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:169:1: ruleCampaignType EOF
            {
             before(grammarAccess.getCampaignTypeRule()); 
            pushFollow(FOLLOW_ruleCampaignType_in_entryRuleCampaignType300);
            ruleCampaignType();
            _fsp--;

             after(grammarAccess.getCampaignTypeRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleCampaignType307); 

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
    // $ANTLR end entryRuleCampaignType


    // $ANTLR start ruleCampaignType
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:176:1: ruleCampaignType : ( ( rule__CampaignType__Group__0 ) ) ;
    public final void ruleCampaignType() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:180:2: ( ( ( rule__CampaignType__Group__0 ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:181:1: ( ( rule__CampaignType__Group__0 ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:181:1: ( ( rule__CampaignType__Group__0 ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:182:1: ( rule__CampaignType__Group__0 )
            {
             before(grammarAccess.getCampaignTypeAccess().getGroup()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:183:1: ( rule__CampaignType__Group__0 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:183:2: rule__CampaignType__Group__0
            {
            pushFollow(FOLLOW_rule__CampaignType__Group__0_in_ruleCampaignType334);
            rule__CampaignType__Group__0();
            _fsp--;


            }

             after(grammarAccess.getCampaignTypeAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleCampaignType


    // $ANTLR start entryRuleEntity
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:195:1: entryRuleEntity : ruleEntity EOF ;
    public final void entryRuleEntity() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:195:17: ( ruleEntity EOF )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:196:1: ruleEntity EOF
            {
             before(grammarAccess.getEntityRule()); 
            pushFollow(FOLLOW_ruleEntity_in_entryRuleEntity360);
            ruleEntity();
            _fsp--;

             after(grammarAccess.getEntityRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleEntity367); 

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
    // $ANTLR end entryRuleEntity


    // $ANTLR start ruleEntity
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:203:1: ruleEntity : ( ( rule__Entity__Group__0 ) ) ;
    public final void ruleEntity() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:207:2: ( ( ( rule__Entity__Group__0 ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:208:1: ( ( rule__Entity__Group__0 ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:208:1: ( ( rule__Entity__Group__0 ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:209:1: ( rule__Entity__Group__0 )
            {
             before(grammarAccess.getEntityAccess().getGroup()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:210:1: ( rule__Entity__Group__0 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:210:2: rule__Entity__Group__0
            {
            pushFollow(FOLLOW_rule__Entity__Group__0_in_ruleEntity394);
            rule__Entity__Group__0();
            _fsp--;


            }

             after(grammarAccess.getEntityAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleEntity


    // $ANTLR start entryRuleProperty
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:222:1: entryRuleProperty : ruleProperty EOF ;
    public final void entryRuleProperty() throws RecognitionException {
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:222:19: ( ruleProperty EOF )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:223:1: ruleProperty EOF
            {
             before(grammarAccess.getPropertyRule()); 
            pushFollow(FOLLOW_ruleProperty_in_entryRuleProperty420);
            ruleProperty();
            _fsp--;

             after(grammarAccess.getPropertyRule()); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleProperty427); 

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
    // $ANTLR end entryRuleProperty


    // $ANTLR start ruleProperty
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:230:1: ruleProperty : ( ( rule__Property__Group__0 ) ) ;
    public final void ruleProperty() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:234:2: ( ( ( rule__Property__Group__0 ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:235:1: ( ( rule__Property__Group__0 ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:235:1: ( ( rule__Property__Group__0 ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:236:1: ( rule__Property__Group__0 )
            {
             before(grammarAccess.getPropertyAccess().getGroup()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:237:1: ( rule__Property__Group__0 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:237:2: rule__Property__Group__0
            {
            pushFollow(FOLLOW_rule__Property__Group__0_in_ruleProperty454);
            rule__Property__Group__0();
            _fsp--;


            }

             after(grammarAccess.getPropertyAccess().getGroup()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end ruleProperty


    // $ANTLR start rule__Type__Alternatives
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:249:1: rule__Type__Alternatives : ( ( ruleSimpleType ) | ( ruleEntity ) | ( ruleCampaignType ) );
    public final void rule__Type__Alternatives() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:253:1: ( ( ruleSimpleType ) | ( ruleEntity ) | ( ruleCampaignType ) )
            int alt1=3;
            switch ( input.LA(1) ) {
            case 12:
                {
                alt1=1;
                }
                break;
            case 15:
                {
                alt1=2;
                }
                break;
            case 13:
                {
                alt1=3;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("249:1: rule__Type__Alternatives : ( ( ruleSimpleType ) | ( ruleEntity ) | ( ruleCampaignType ) );", 1, 0, input);

                throw nvae;
            }

            switch (alt1) {
                case 1 :
                    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:254:1: ( ruleSimpleType )
                    {
                    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:254:1: ( ruleSimpleType )
                    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:255:1: ruleSimpleType
                    {
                     before(grammarAccess.getTypeAccess().getSimpleTypeParserRuleCall_0()); 
                    pushFollow(FOLLOW_ruleSimpleType_in_rule__Type__Alternatives490);
                    ruleSimpleType();
                    _fsp--;

                     after(grammarAccess.getTypeAccess().getSimpleTypeParserRuleCall_0()); 

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:260:6: ( ruleEntity )
                    {
                    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:260:6: ( ruleEntity )
                    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:261:1: ruleEntity
                    {
                     before(grammarAccess.getTypeAccess().getEntityParserRuleCall_1()); 
                    pushFollow(FOLLOW_ruleEntity_in_rule__Type__Alternatives507);
                    ruleEntity();
                    _fsp--;

                     after(grammarAccess.getTypeAccess().getEntityParserRuleCall_1()); 

                    }


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:266:6: ( ruleCampaignType )
                    {
                    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:266:6: ( ruleCampaignType )
                    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:267:1: ruleCampaignType
                    {
                     before(grammarAccess.getTypeAccess().getCampaignTypeParserRuleCall_2()); 
                    pushFollow(FOLLOW_ruleCampaignType_in_rule__Type__Alternatives524);
                    ruleCampaignType();
                    _fsp--;

                     after(grammarAccess.getTypeAccess().getCampaignTypeParserRuleCall_2()); 

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
    // $ANTLR end rule__Type__Alternatives


    // $ANTLR start rule__Model__Group__0
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:279:1: rule__Model__Group__0 : ( ( rule__Model__ImportsAssignment_0 )* ) rule__Model__Group__1 ;
    public final void rule__Model__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:283:1: ( ( ( rule__Model__ImportsAssignment_0 )* ) rule__Model__Group__1 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:284:1: ( ( rule__Model__ImportsAssignment_0 )* ) rule__Model__Group__1
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:284:1: ( ( rule__Model__ImportsAssignment_0 )* )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:285:1: ( rule__Model__ImportsAssignment_0 )*
            {
             before(grammarAccess.getModelAccess().getImportsAssignment_0()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:286:1: ( rule__Model__ImportsAssignment_0 )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( (LA2_0==11) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:286:2: rule__Model__ImportsAssignment_0
            	    {
            	    pushFollow(FOLLOW_rule__Model__ImportsAssignment_0_in_rule__Model__Group__0558);
            	    rule__Model__ImportsAssignment_0();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);

             after(grammarAccess.getModelAccess().getImportsAssignment_0()); 

            }

            pushFollow(FOLLOW_rule__Model__Group__1_in_rule__Model__Group__0568);
            rule__Model__Group__1();
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
    // $ANTLR end rule__Model__Group__0


    // $ANTLR start rule__Model__Group__1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:297:1: rule__Model__Group__1 : ( ( rule__Model__ElementsAssignment_1 )* ) ;
    public final void rule__Model__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:301:1: ( ( ( rule__Model__ElementsAssignment_1 )* ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:302:1: ( ( rule__Model__ElementsAssignment_1 )* )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:302:1: ( ( rule__Model__ElementsAssignment_1 )* )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:303:1: ( rule__Model__ElementsAssignment_1 )*
            {
             before(grammarAccess.getModelAccess().getElementsAssignment_1()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:304:1: ( rule__Model__ElementsAssignment_1 )*
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( ((LA3_0>=12 && LA3_0<=13)||LA3_0==15) ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:304:2: rule__Model__ElementsAssignment_1
            	    {
            	    pushFollow(FOLLOW_rule__Model__ElementsAssignment_1_in_rule__Model__Group__1596);
            	    rule__Model__ElementsAssignment_1();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop3;
                }
            } while (true);

             after(grammarAccess.getModelAccess().getElementsAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Model__Group__1


    // $ANTLR start rule__Import__Group__0
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:318:1: rule__Import__Group__0 : ( 'import' ) rule__Import__Group__1 ;
    public final void rule__Import__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:322:1: ( ( 'import' ) rule__Import__Group__1 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:323:1: ( 'import' ) rule__Import__Group__1
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:323:1: ( 'import' )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:324:1: 'import'
            {
             before(grammarAccess.getImportAccess().getImportKeyword_0()); 
            match(input,11,FOLLOW_11_in_rule__Import__Group__0636); 
             after(grammarAccess.getImportAccess().getImportKeyword_0()); 

            }

            pushFollow(FOLLOW_rule__Import__Group__1_in_rule__Import__Group__0646);
            rule__Import__Group__1();
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
    // $ANTLR end rule__Import__Group__0


    // $ANTLR start rule__Import__Group__1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:338:1: rule__Import__Group__1 : ( ( rule__Import__ImportURIAssignment_1 ) ) ;
    public final void rule__Import__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:342:1: ( ( ( rule__Import__ImportURIAssignment_1 ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:343:1: ( ( rule__Import__ImportURIAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:343:1: ( ( rule__Import__ImportURIAssignment_1 ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:344:1: ( rule__Import__ImportURIAssignment_1 )
            {
             before(grammarAccess.getImportAccess().getImportURIAssignment_1()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:345:1: ( rule__Import__ImportURIAssignment_1 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:345:2: rule__Import__ImportURIAssignment_1
            {
            pushFollow(FOLLOW_rule__Import__ImportURIAssignment_1_in_rule__Import__Group__1674);
            rule__Import__ImportURIAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getImportAccess().getImportURIAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Import__Group__1


    // $ANTLR start rule__SimpleType__Group__0
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:359:1: rule__SimpleType__Group__0 : ( 'type' ) rule__SimpleType__Group__1 ;
    public final void rule__SimpleType__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:363:1: ( ( 'type' ) rule__SimpleType__Group__1 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:364:1: ( 'type' ) rule__SimpleType__Group__1
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:364:1: ( 'type' )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:365:1: 'type'
            {
             before(grammarAccess.getSimpleTypeAccess().getTypeKeyword_0()); 
            match(input,12,FOLLOW_12_in_rule__SimpleType__Group__0713); 
             after(grammarAccess.getSimpleTypeAccess().getTypeKeyword_0()); 

            }

            pushFollow(FOLLOW_rule__SimpleType__Group__1_in_rule__SimpleType__Group__0723);
            rule__SimpleType__Group__1();
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
    // $ANTLR end rule__SimpleType__Group__0


    // $ANTLR start rule__SimpleType__Group__1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:379:1: rule__SimpleType__Group__1 : ( ( rule__SimpleType__NameAssignment_1 ) ) ;
    public final void rule__SimpleType__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:383:1: ( ( ( rule__SimpleType__NameAssignment_1 ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:384:1: ( ( rule__SimpleType__NameAssignment_1 ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:384:1: ( ( rule__SimpleType__NameAssignment_1 ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:385:1: ( rule__SimpleType__NameAssignment_1 )
            {
             before(grammarAccess.getSimpleTypeAccess().getNameAssignment_1()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:386:1: ( rule__SimpleType__NameAssignment_1 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:386:2: rule__SimpleType__NameAssignment_1
            {
            pushFollow(FOLLOW_rule__SimpleType__NameAssignment_1_in_rule__SimpleType__Group__1751);
            rule__SimpleType__NameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getSimpleTypeAccess().getNameAssignment_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__SimpleType__Group__1


    // $ANTLR start rule__CampaignType__Group__0
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:400:1: rule__CampaignType__Group__0 : ( '[campaign]' ) rule__CampaignType__Group__1 ;
    public final void rule__CampaignType__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:404:1: ( ( '[campaign]' ) rule__CampaignType__Group__1 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:405:1: ( '[campaign]' ) rule__CampaignType__Group__1
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:405:1: ( '[campaign]' )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:406:1: '[campaign]'
            {
             before(grammarAccess.getCampaignTypeAccess().getCampaignKeyword_0()); 
            match(input,13,FOLLOW_13_in_rule__CampaignType__Group__0790); 
             after(grammarAccess.getCampaignTypeAccess().getCampaignKeyword_0()); 

            }

            pushFollow(FOLLOW_rule__CampaignType__Group__1_in_rule__CampaignType__Group__0800);
            rule__CampaignType__Group__1();
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
    // $ANTLR end rule__CampaignType__Group__0


    // $ANTLR start rule__CampaignType__Group__1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:420:1: rule__CampaignType__Group__1 : ( ( rule__CampaignType__NameAssignment_1 )? ) rule__CampaignType__Group__2 ;
    public final void rule__CampaignType__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:424:1: ( ( ( rule__CampaignType__NameAssignment_1 )? ) rule__CampaignType__Group__2 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:425:1: ( ( rule__CampaignType__NameAssignment_1 )? ) rule__CampaignType__Group__2
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:425:1: ( ( rule__CampaignType__NameAssignment_1 )? )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:426:1: ( rule__CampaignType__NameAssignment_1 )?
            {
             before(grammarAccess.getCampaignTypeAccess().getNameAssignment_1()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:427:1: ( rule__CampaignType__NameAssignment_1 )?
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( (LA4_0==RULE_ID) ) {
                alt4=1;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:427:2: rule__CampaignType__NameAssignment_1
                    {
                    pushFollow(FOLLOW_rule__CampaignType__NameAssignment_1_in_rule__CampaignType__Group__1828);
                    rule__CampaignType__NameAssignment_1();
                    _fsp--;


                    }
                    break;

            }

             after(grammarAccess.getCampaignTypeAccess().getNameAssignment_1()); 

            }

            pushFollow(FOLLOW_rule__CampaignType__Group__2_in_rule__CampaignType__Group__1838);
            rule__CampaignType__Group__2();
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
    // $ANTLR end rule__CampaignType__Group__1


    // $ANTLR start rule__CampaignType__Group__2
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:438:1: rule__CampaignType__Group__2 : ( '[/campaign]' ) ;
    public final void rule__CampaignType__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:442:1: ( ( '[/campaign]' ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:443:1: ( '[/campaign]' )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:443:1: ( '[/campaign]' )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:444:1: '[/campaign]'
            {
             before(grammarAccess.getCampaignTypeAccess().getCampaignKeyword_2()); 
            match(input,14,FOLLOW_14_in_rule__CampaignType__Group__2867); 
             after(grammarAccess.getCampaignTypeAccess().getCampaignKeyword_2()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__CampaignType__Group__2


    // $ANTLR start rule__Entity__Group__0
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:463:1: rule__Entity__Group__0 : ( 'entity' ) rule__Entity__Group__1 ;
    public final void rule__Entity__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:467:1: ( ( 'entity' ) rule__Entity__Group__1 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:468:1: ( 'entity' ) rule__Entity__Group__1
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:468:1: ( 'entity' )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:469:1: 'entity'
            {
             before(grammarAccess.getEntityAccess().getEntityKeyword_0()); 
            match(input,15,FOLLOW_15_in_rule__Entity__Group__0909); 
             after(grammarAccess.getEntityAccess().getEntityKeyword_0()); 

            }

            pushFollow(FOLLOW_rule__Entity__Group__1_in_rule__Entity__Group__0919);
            rule__Entity__Group__1();
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
    // $ANTLR end rule__Entity__Group__0


    // $ANTLR start rule__Entity__Group__1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:483:1: rule__Entity__Group__1 : ( ( rule__Entity__NameAssignment_1 ) ) rule__Entity__Group__2 ;
    public final void rule__Entity__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:487:1: ( ( ( rule__Entity__NameAssignment_1 ) ) rule__Entity__Group__2 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:488:1: ( ( rule__Entity__NameAssignment_1 ) ) rule__Entity__Group__2
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:488:1: ( ( rule__Entity__NameAssignment_1 ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:489:1: ( rule__Entity__NameAssignment_1 )
            {
             before(grammarAccess.getEntityAccess().getNameAssignment_1()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:490:1: ( rule__Entity__NameAssignment_1 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:490:2: rule__Entity__NameAssignment_1
            {
            pushFollow(FOLLOW_rule__Entity__NameAssignment_1_in_rule__Entity__Group__1947);
            rule__Entity__NameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getEntityAccess().getNameAssignment_1()); 

            }

            pushFollow(FOLLOW_rule__Entity__Group__2_in_rule__Entity__Group__1956);
            rule__Entity__Group__2();
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
    // $ANTLR end rule__Entity__Group__1


    // $ANTLR start rule__Entity__Group__2
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:501:1: rule__Entity__Group__2 : ( ( rule__Entity__Group_2__0 )? ) rule__Entity__Group__3 ;
    public final void rule__Entity__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:505:1: ( ( ( rule__Entity__Group_2__0 )? ) rule__Entity__Group__3 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:506:1: ( ( rule__Entity__Group_2__0 )? ) rule__Entity__Group__3
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:506:1: ( ( rule__Entity__Group_2__0 )? )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:507:1: ( rule__Entity__Group_2__0 )?
            {
             before(grammarAccess.getEntityAccess().getGroup_2()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:508:1: ( rule__Entity__Group_2__0 )?
            int alt5=2;
            int LA5_0 = input.LA(1);

            if ( (LA5_0==18) ) {
                alt5=1;
            }
            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:508:2: rule__Entity__Group_2__0
                    {
                    pushFollow(FOLLOW_rule__Entity__Group_2__0_in_rule__Entity__Group__2984);
                    rule__Entity__Group_2__0();
                    _fsp--;


                    }
                    break;

            }

             after(grammarAccess.getEntityAccess().getGroup_2()); 

            }

            pushFollow(FOLLOW_rule__Entity__Group__3_in_rule__Entity__Group__2994);
            rule__Entity__Group__3();
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
    // $ANTLR end rule__Entity__Group__2


    // $ANTLR start rule__Entity__Group__3
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:519:1: rule__Entity__Group__3 : ( '{' ) rule__Entity__Group__4 ;
    public final void rule__Entity__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:523:1: ( ( '{' ) rule__Entity__Group__4 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:524:1: ( '{' ) rule__Entity__Group__4
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:524:1: ( '{' )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:525:1: '{'
            {
             before(grammarAccess.getEntityAccess().getLeftCurlyBracketKeyword_3()); 
            match(input,16,FOLLOW_16_in_rule__Entity__Group__31023); 
             after(grammarAccess.getEntityAccess().getLeftCurlyBracketKeyword_3()); 

            }

            pushFollow(FOLLOW_rule__Entity__Group__4_in_rule__Entity__Group__31033);
            rule__Entity__Group__4();
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
    // $ANTLR end rule__Entity__Group__3


    // $ANTLR start rule__Entity__Group__4
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:539:1: rule__Entity__Group__4 : ( ( rule__Entity__PropertiesAssignment_4 )* ) rule__Entity__Group__5 ;
    public final void rule__Entity__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:543:1: ( ( ( rule__Entity__PropertiesAssignment_4 )* ) rule__Entity__Group__5 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:544:1: ( ( rule__Entity__PropertiesAssignment_4 )* ) rule__Entity__Group__5
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:544:1: ( ( rule__Entity__PropertiesAssignment_4 )* )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:545:1: ( rule__Entity__PropertiesAssignment_4 )*
            {
             before(grammarAccess.getEntityAccess().getPropertiesAssignment_4()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:546:1: ( rule__Entity__PropertiesAssignment_4 )*
            loop6:
            do {
                int alt6=2;
                int LA6_0 = input.LA(1);

                if ( (LA6_0==19) ) {
                    alt6=1;
                }


                switch (alt6) {
            	case 1 :
            	    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:546:2: rule__Entity__PropertiesAssignment_4
            	    {
            	    pushFollow(FOLLOW_rule__Entity__PropertiesAssignment_4_in_rule__Entity__Group__41061);
            	    rule__Entity__PropertiesAssignment_4();
            	    _fsp--;


            	    }
            	    break;

            	default :
            	    break loop6;
                }
            } while (true);

             after(grammarAccess.getEntityAccess().getPropertiesAssignment_4()); 

            }

            pushFollow(FOLLOW_rule__Entity__Group__5_in_rule__Entity__Group__41071);
            rule__Entity__Group__5();
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
    // $ANTLR end rule__Entity__Group__4


    // $ANTLR start rule__Entity__Group__5
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:557:1: rule__Entity__Group__5 : ( '}' ) ;
    public final void rule__Entity__Group__5() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:561:1: ( ( '}' ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:562:1: ( '}' )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:562:1: ( '}' )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:563:1: '}'
            {
             before(grammarAccess.getEntityAccess().getRightCurlyBracketKeyword_5()); 
            match(input,17,FOLLOW_17_in_rule__Entity__Group__51100); 
             after(grammarAccess.getEntityAccess().getRightCurlyBracketKeyword_5()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Entity__Group__5


    // $ANTLR start rule__Entity__Group_2__0
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:588:1: rule__Entity__Group_2__0 : ( 'extends' ) rule__Entity__Group_2__1 ;
    public final void rule__Entity__Group_2__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:592:1: ( ( 'extends' ) rule__Entity__Group_2__1 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:593:1: ( 'extends' ) rule__Entity__Group_2__1
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:593:1: ( 'extends' )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:594:1: 'extends'
            {
             before(grammarAccess.getEntityAccess().getExtendsKeyword_2_0()); 
            match(input,18,FOLLOW_18_in_rule__Entity__Group_2__01148); 
             after(grammarAccess.getEntityAccess().getExtendsKeyword_2_0()); 

            }

            pushFollow(FOLLOW_rule__Entity__Group_2__1_in_rule__Entity__Group_2__01158);
            rule__Entity__Group_2__1();
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
    // $ANTLR end rule__Entity__Group_2__0


    // $ANTLR start rule__Entity__Group_2__1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:608:1: rule__Entity__Group_2__1 : ( ( rule__Entity__ExtendsAssignment_2_1 ) ) ;
    public final void rule__Entity__Group_2__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:612:1: ( ( ( rule__Entity__ExtendsAssignment_2_1 ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:613:1: ( ( rule__Entity__ExtendsAssignment_2_1 ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:613:1: ( ( rule__Entity__ExtendsAssignment_2_1 ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:614:1: ( rule__Entity__ExtendsAssignment_2_1 )
            {
             before(grammarAccess.getEntityAccess().getExtendsAssignment_2_1()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:615:1: ( rule__Entity__ExtendsAssignment_2_1 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:615:2: rule__Entity__ExtendsAssignment_2_1
            {
            pushFollow(FOLLOW_rule__Entity__ExtendsAssignment_2_1_in_rule__Entity__Group_2__11186);
            rule__Entity__ExtendsAssignment_2_1();
            _fsp--;


            }

             after(grammarAccess.getEntityAccess().getExtendsAssignment_2_1()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Entity__Group_2__1


    // $ANTLR start rule__Property__Group__0
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:629:1: rule__Property__Group__0 : ( 'property' ) rule__Property__Group__1 ;
    public final void rule__Property__Group__0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:633:1: ( ( 'property' ) rule__Property__Group__1 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:634:1: ( 'property' ) rule__Property__Group__1
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:634:1: ( 'property' )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:635:1: 'property'
            {
             before(grammarAccess.getPropertyAccess().getPropertyKeyword_0()); 
            match(input,19,FOLLOW_19_in_rule__Property__Group__01225); 
             after(grammarAccess.getPropertyAccess().getPropertyKeyword_0()); 

            }

            pushFollow(FOLLOW_rule__Property__Group__1_in_rule__Property__Group__01235);
            rule__Property__Group__1();
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
    // $ANTLR end rule__Property__Group__0


    // $ANTLR start rule__Property__Group__1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:649:1: rule__Property__Group__1 : ( ( rule__Property__NameAssignment_1 ) ) rule__Property__Group__2 ;
    public final void rule__Property__Group__1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:653:1: ( ( ( rule__Property__NameAssignment_1 ) ) rule__Property__Group__2 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:654:1: ( ( rule__Property__NameAssignment_1 ) ) rule__Property__Group__2
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:654:1: ( ( rule__Property__NameAssignment_1 ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:655:1: ( rule__Property__NameAssignment_1 )
            {
             before(grammarAccess.getPropertyAccess().getNameAssignment_1()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:656:1: ( rule__Property__NameAssignment_1 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:656:2: rule__Property__NameAssignment_1
            {
            pushFollow(FOLLOW_rule__Property__NameAssignment_1_in_rule__Property__Group__11263);
            rule__Property__NameAssignment_1();
            _fsp--;


            }

             after(grammarAccess.getPropertyAccess().getNameAssignment_1()); 

            }

            pushFollow(FOLLOW_rule__Property__Group__2_in_rule__Property__Group__11272);
            rule__Property__Group__2();
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
    // $ANTLR end rule__Property__Group__1


    // $ANTLR start rule__Property__Group__2
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:667:1: rule__Property__Group__2 : ( ':' ) rule__Property__Group__3 ;
    public final void rule__Property__Group__2() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:671:1: ( ( ':' ) rule__Property__Group__3 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:672:1: ( ':' ) rule__Property__Group__3
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:672:1: ( ':' )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:673:1: ':'
            {
             before(grammarAccess.getPropertyAccess().getColonKeyword_2()); 
            match(input,20,FOLLOW_20_in_rule__Property__Group__21301); 
             after(grammarAccess.getPropertyAccess().getColonKeyword_2()); 

            }

            pushFollow(FOLLOW_rule__Property__Group__3_in_rule__Property__Group__21311);
            rule__Property__Group__3();
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
    // $ANTLR end rule__Property__Group__2


    // $ANTLR start rule__Property__Group__3
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:687:1: rule__Property__Group__3 : ( ( rule__Property__TypeAssignment_3 ) ) rule__Property__Group__4 ;
    public final void rule__Property__Group__3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:691:1: ( ( ( rule__Property__TypeAssignment_3 ) ) rule__Property__Group__4 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:692:1: ( ( rule__Property__TypeAssignment_3 ) ) rule__Property__Group__4
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:692:1: ( ( rule__Property__TypeAssignment_3 ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:693:1: ( rule__Property__TypeAssignment_3 )
            {
             before(grammarAccess.getPropertyAccess().getTypeAssignment_3()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:694:1: ( rule__Property__TypeAssignment_3 )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:694:2: rule__Property__TypeAssignment_3
            {
            pushFollow(FOLLOW_rule__Property__TypeAssignment_3_in_rule__Property__Group__31339);
            rule__Property__TypeAssignment_3();
            _fsp--;


            }

             after(grammarAccess.getPropertyAccess().getTypeAssignment_3()); 

            }

            pushFollow(FOLLOW_rule__Property__Group__4_in_rule__Property__Group__31348);
            rule__Property__Group__4();
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
    // $ANTLR end rule__Property__Group__3


    // $ANTLR start rule__Property__Group__4
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:705:1: rule__Property__Group__4 : ( ( rule__Property__ManyAssignment_4 )? ) ;
    public final void rule__Property__Group__4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:709:1: ( ( ( rule__Property__ManyAssignment_4 )? ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:710:1: ( ( rule__Property__ManyAssignment_4 )? )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:710:1: ( ( rule__Property__ManyAssignment_4 )? )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:711:1: ( rule__Property__ManyAssignment_4 )?
            {
             before(grammarAccess.getPropertyAccess().getManyAssignment_4()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:712:1: ( rule__Property__ManyAssignment_4 )?
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( (LA7_0==21) ) {
                alt7=1;
            }
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:712:2: rule__Property__ManyAssignment_4
                    {
                    pushFollow(FOLLOW_rule__Property__ManyAssignment_4_in_rule__Property__Group__41376);
                    rule__Property__ManyAssignment_4();
                    _fsp--;


                    }
                    break;

            }

             after(grammarAccess.getPropertyAccess().getManyAssignment_4()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Property__Group__4


    // $ANTLR start rule__Model__ImportsAssignment_0
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:732:1: rule__Model__ImportsAssignment_0 : ( ruleImport ) ;
    public final void rule__Model__ImportsAssignment_0() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:736:1: ( ( ruleImport ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:737:1: ( ruleImport )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:737:1: ( ruleImport )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:738:1: ruleImport
            {
             before(grammarAccess.getModelAccess().getImportsImportParserRuleCall_0_0()); 
            pushFollow(FOLLOW_ruleImport_in_rule__Model__ImportsAssignment_01421);
            ruleImport();
            _fsp--;

             after(grammarAccess.getModelAccess().getImportsImportParserRuleCall_0_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Model__ImportsAssignment_0


    // $ANTLR start rule__Model__ElementsAssignment_1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:747:1: rule__Model__ElementsAssignment_1 : ( ruleType ) ;
    public final void rule__Model__ElementsAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:751:1: ( ( ruleType ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:752:1: ( ruleType )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:752:1: ( ruleType )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:753:1: ruleType
            {
             before(grammarAccess.getModelAccess().getElementsTypeParserRuleCall_1_0()); 
            pushFollow(FOLLOW_ruleType_in_rule__Model__ElementsAssignment_11452);
            ruleType();
            _fsp--;

             after(grammarAccess.getModelAccess().getElementsTypeParserRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Model__ElementsAssignment_1


    // $ANTLR start rule__Import__ImportURIAssignment_1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:762:1: rule__Import__ImportURIAssignment_1 : ( RULE_STRING ) ;
    public final void rule__Import__ImportURIAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:766:1: ( ( RULE_STRING ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:767:1: ( RULE_STRING )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:767:1: ( RULE_STRING )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:768:1: RULE_STRING
            {
             before(grammarAccess.getImportAccess().getImportURISTRINGTerminalRuleCall_1_0()); 
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_rule__Import__ImportURIAssignment_11483); 
             after(grammarAccess.getImportAccess().getImportURISTRINGTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Import__ImportURIAssignment_1


    // $ANTLR start rule__SimpleType__NameAssignment_1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:777:1: rule__SimpleType__NameAssignment_1 : ( RULE_ID ) ;
    public final void rule__SimpleType__NameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:781:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:782:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:782:1: ( RULE_ID )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:783:1: RULE_ID
            {
             before(grammarAccess.getSimpleTypeAccess().getNameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__SimpleType__NameAssignment_11514); 
             after(grammarAccess.getSimpleTypeAccess().getNameIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__SimpleType__NameAssignment_1


    // $ANTLR start rule__CampaignType__NameAssignment_1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:792:1: rule__CampaignType__NameAssignment_1 : ( RULE_ID ) ;
    public final void rule__CampaignType__NameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:796:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:797:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:797:1: ( RULE_ID )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:798:1: RULE_ID
            {
             before(grammarAccess.getCampaignTypeAccess().getNameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__CampaignType__NameAssignment_11545); 
             after(grammarAccess.getCampaignTypeAccess().getNameIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__CampaignType__NameAssignment_1


    // $ANTLR start rule__Entity__NameAssignment_1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:807:1: rule__Entity__NameAssignment_1 : ( RULE_ID ) ;
    public final void rule__Entity__NameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:811:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:812:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:812:1: ( RULE_ID )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:813:1: RULE_ID
            {
             before(grammarAccess.getEntityAccess().getNameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__Entity__NameAssignment_11576); 
             after(grammarAccess.getEntityAccess().getNameIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Entity__NameAssignment_1


    // $ANTLR start rule__Entity__ExtendsAssignment_2_1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:822:1: rule__Entity__ExtendsAssignment_2_1 : ( ( RULE_ID ) ) ;
    public final void rule__Entity__ExtendsAssignment_2_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:826:1: ( ( ( RULE_ID ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:827:1: ( ( RULE_ID ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:827:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:828:1: ( RULE_ID )
            {
             before(grammarAccess.getEntityAccess().getExtendsEntityCrossReference_2_1_0()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:829:1: ( RULE_ID )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:830:1: RULE_ID
            {
             before(grammarAccess.getEntityAccess().getExtendsEntityIDTerminalRuleCall_2_1_0_1()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__Entity__ExtendsAssignment_2_11611); 
             after(grammarAccess.getEntityAccess().getExtendsEntityIDTerminalRuleCall_2_1_0_1()); 

            }

             after(grammarAccess.getEntityAccess().getExtendsEntityCrossReference_2_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Entity__ExtendsAssignment_2_1


    // $ANTLR start rule__Entity__PropertiesAssignment_4
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:841:1: rule__Entity__PropertiesAssignment_4 : ( ruleProperty ) ;
    public final void rule__Entity__PropertiesAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:845:1: ( ( ruleProperty ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:846:1: ( ruleProperty )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:846:1: ( ruleProperty )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:847:1: ruleProperty
            {
             before(grammarAccess.getEntityAccess().getPropertiesPropertyParserRuleCall_4_0()); 
            pushFollow(FOLLOW_ruleProperty_in_rule__Entity__PropertiesAssignment_41646);
            ruleProperty();
            _fsp--;

             after(grammarAccess.getEntityAccess().getPropertiesPropertyParserRuleCall_4_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Entity__PropertiesAssignment_4


    // $ANTLR start rule__Property__NameAssignment_1
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:856:1: rule__Property__NameAssignment_1 : ( RULE_ID ) ;
    public final void rule__Property__NameAssignment_1() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:860:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:861:1: ( RULE_ID )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:861:1: ( RULE_ID )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:862:1: RULE_ID
            {
             before(grammarAccess.getPropertyAccess().getNameIDTerminalRuleCall_1_0()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__Property__NameAssignment_11677); 
             after(grammarAccess.getPropertyAccess().getNameIDTerminalRuleCall_1_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Property__NameAssignment_1


    // $ANTLR start rule__Property__TypeAssignment_3
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:871:1: rule__Property__TypeAssignment_3 : ( ( RULE_ID ) ) ;
    public final void rule__Property__TypeAssignment_3() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:875:1: ( ( ( RULE_ID ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:876:1: ( ( RULE_ID ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:876:1: ( ( RULE_ID ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:877:1: ( RULE_ID )
            {
             before(grammarAccess.getPropertyAccess().getTypeTypeCrossReference_3_0()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:878:1: ( RULE_ID )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:879:1: RULE_ID
            {
             before(grammarAccess.getPropertyAccess().getTypeTypeIDTerminalRuleCall_3_0_1()); 
            match(input,RULE_ID,FOLLOW_RULE_ID_in_rule__Property__TypeAssignment_31712); 
             after(grammarAccess.getPropertyAccess().getTypeTypeIDTerminalRuleCall_3_0_1()); 

            }

             after(grammarAccess.getPropertyAccess().getTypeTypeCrossReference_3_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Property__TypeAssignment_3


    // $ANTLR start rule__Property__ManyAssignment_4
    // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:890:1: rule__Property__ManyAssignment_4 : ( ( '[]' ) ) ;
    public final void rule__Property__ManyAssignment_4() throws RecognitionException {

        		int stackSize = keepStackSize();
            
        try {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:894:1: ( ( ( '[]' ) ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:895:1: ( ( '[]' ) )
            {
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:895:1: ( ( '[]' ) )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:896:1: ( '[]' )
            {
             before(grammarAccess.getPropertyAccess().getManyLeftSquareBracketRightSquareBracketKeyword_4_0()); 
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:897:1: ( '[]' )
            // ../org.wesnoth.wml.ide.ui/src-gen/org/wesnoth/contentassist/antlr/internal/InternalWml.g:898:1: '[]'
            {
             before(grammarAccess.getPropertyAccess().getManyLeftSquareBracketRightSquareBracketKeyword_4_0()); 
            match(input,21,FOLLOW_21_in_rule__Property__ManyAssignment_41752); 
             after(grammarAccess.getPropertyAccess().getManyLeftSquareBracketRightSquareBracketKeyword_4_0()); 

            }

             after(grammarAccess.getPropertyAccess().getManyLeftSquareBracketRightSquareBracketKeyword_4_0()); 

            }


            }

        }
        catch (RecognitionException re) {
            reportError(re);
            recover(input,re);
        }
        finally {

            	restoreStackSize(stackSize);

        }
        return ;
    }
    // $ANTLR end rule__Property__ManyAssignment_4


 

    public static final BitSet FOLLOW_ruleModel_in_entryRuleModel60 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleModel67 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Model__Group__0_in_ruleModel94 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleImport_in_entryRuleImport120 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleImport127 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Import__Group__0_in_ruleImport154 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleType_in_entryRuleType180 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleType187 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Type__Alternatives_in_ruleType214 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleSimpleType_in_entryRuleSimpleType240 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleSimpleType247 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__SimpleType__Group__0_in_ruleSimpleType274 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleCampaignType_in_entryRuleCampaignType300 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleCampaignType307 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__CampaignType__Group__0_in_ruleCampaignType334 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleEntity_in_entryRuleEntity360 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleEntity367 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Entity__Group__0_in_ruleEntity394 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleProperty_in_entryRuleProperty420 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleProperty427 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Property__Group__0_in_ruleProperty454 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleSimpleType_in_rule__Type__Alternatives490 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleEntity_in_rule__Type__Alternatives507 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleCampaignType_in_rule__Type__Alternatives524 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Model__ImportsAssignment_0_in_rule__Model__Group__0558 = new BitSet(new long[]{0x000000000000B802L});
    public static final BitSet FOLLOW_rule__Model__Group__1_in_rule__Model__Group__0568 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Model__ElementsAssignment_1_in_rule__Model__Group__1596 = new BitSet(new long[]{0x000000000000B002L});
    public static final BitSet FOLLOW_11_in_rule__Import__Group__0636 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_rule__Import__Group__1_in_rule__Import__Group__0646 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Import__ImportURIAssignment_1_in_rule__Import__Group__1674 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rule__SimpleType__Group__0713 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_rule__SimpleType__Group__1_in_rule__SimpleType__Group__0723 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__SimpleType__NameAssignment_1_in_rule__SimpleType__Group__1751 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_rule__CampaignType__Group__0790 = new BitSet(new long[]{0x0000000000004020L});
    public static final BitSet FOLLOW_rule__CampaignType__Group__1_in_rule__CampaignType__Group__0800 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__CampaignType__NameAssignment_1_in_rule__CampaignType__Group__1828 = new BitSet(new long[]{0x0000000000004000L});
    public static final BitSet FOLLOW_rule__CampaignType__Group__2_in_rule__CampaignType__Group__1838 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_rule__CampaignType__Group__2867 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_rule__Entity__Group__0909 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_rule__Entity__Group__1_in_rule__Entity__Group__0919 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Entity__NameAssignment_1_in_rule__Entity__Group__1947 = new BitSet(new long[]{0x0000000000050000L});
    public static final BitSet FOLLOW_rule__Entity__Group__2_in_rule__Entity__Group__1956 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Entity__Group_2__0_in_rule__Entity__Group__2984 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_rule__Entity__Group__3_in_rule__Entity__Group__2994 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_rule__Entity__Group__31023 = new BitSet(new long[]{0x00000000000A0000L});
    public static final BitSet FOLLOW_rule__Entity__Group__4_in_rule__Entity__Group__31033 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Entity__PropertiesAssignment_4_in_rule__Entity__Group__41061 = new BitSet(new long[]{0x00000000000A0000L});
    public static final BitSet FOLLOW_rule__Entity__Group__5_in_rule__Entity__Group__41071 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_rule__Entity__Group__51100 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_rule__Entity__Group_2__01148 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_rule__Entity__Group_2__1_in_rule__Entity__Group_2__01158 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Entity__ExtendsAssignment_2_1_in_rule__Entity__Group_2__11186 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_rule__Property__Group__01225 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_rule__Property__Group__1_in_rule__Property__Group__01235 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Property__NameAssignment_1_in_rule__Property__Group__11263 = new BitSet(new long[]{0x0000000000100000L});
    public static final BitSet FOLLOW_rule__Property__Group__2_in_rule__Property__Group__11272 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_rule__Property__Group__21301 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_rule__Property__Group__3_in_rule__Property__Group__21311 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Property__TypeAssignment_3_in_rule__Property__Group__31339 = new BitSet(new long[]{0x0000000000200002L});
    public static final BitSet FOLLOW_rule__Property__Group__4_in_rule__Property__Group__31348 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rule__Property__ManyAssignment_4_in_rule__Property__Group__41376 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleImport_in_rule__Model__ImportsAssignment_01421 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleType_in_rule__Model__ElementsAssignment_11452 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_rule__Import__ImportURIAssignment_11483 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__SimpleType__NameAssignment_11514 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__CampaignType__NameAssignment_11545 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__Entity__NameAssignment_11576 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__Entity__ExtendsAssignment_2_11611 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleProperty_in_rule__Entity__PropertiesAssignment_41646 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__Property__NameAssignment_11677 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rule__Property__TypeAssignment_31712 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_rule__Property__ManyAssignment_41752 = new BitSet(new long[]{0x0000000000000002L});

}