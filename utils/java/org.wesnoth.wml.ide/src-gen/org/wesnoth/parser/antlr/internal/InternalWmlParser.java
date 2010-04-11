package org.wesnoth.parser.antlr.internal; 

import java.io.InputStream;
import org.eclipse.xtext.*;
import org.eclipse.xtext.parser.*;
import org.eclipse.xtext.parser.impl.*;
import org.eclipse.xtext.parsetree.*;
import org.eclipse.emf.ecore.util.EcoreUtil;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.parser.antlr.AbstractInternalAntlrParser;
import org.eclipse.xtext.parser.antlr.XtextTokenStream;
import org.eclipse.xtext.parser.antlr.XtextTokenStream.HiddenTokens;
import org.eclipse.xtext.parser.antlr.AntlrDatatypeRuleToken;
import org.eclipse.xtext.conversion.ValueConverterException;
import org.wesnoth.services.WmlGrammarAccess;



import org.antlr.runtime.*;
import java.util.Stack;
import java.util.List;
import java.util.ArrayList;

public class InternalWmlParser extends AbstractInternalAntlrParser {
    public static final String[] tokenNames = new String[] {
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_STRING", "RULE_ID", "RULE_INT", "RULE_ML_COMMENT", "RULE_SL_COMMENT", "RULE_WS", "RULE_ANY_OTHER", "'import'", "'type'", "'[campaign]'", "'[/campaign]'", "'entity'", "'extends'", "'{'", "'}'", "'property'", "':'", "'[]'"
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
    public String getGrammarFileName() { return "../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g"; }


     
     	private WmlGrammarAccess grammarAccess;
     	
        public InternalWmlParser(TokenStream input, IAstFactory factory, WmlGrammarAccess grammarAccess) {
            this(input);
            this.factory = factory;
            registerRules(grammarAccess.getGrammar());
            this.grammarAccess = grammarAccess;
        }
        
        @Override
        protected InputStream getTokenFile() {
        	ClassLoader classLoader = getClass().getClassLoader();
        	return classLoader.getResourceAsStream("org/wesnoth/parser/antlr/internal/InternalWml.tokens");
        }
        
        @Override
        protected String getFirstRuleName() {
        	return "Model";	
       	} 



    // $ANTLR start entryRuleModel
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:72:1: entryRuleModel returns [EObject current=null] : iv_ruleModel= ruleModel EOF ;
    public final EObject entryRuleModel() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleModel = null;


        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:72:47: (iv_ruleModel= ruleModel EOF )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:73:2: iv_ruleModel= ruleModel EOF
            {
             currentNode = createCompositeNode(grammarAccess.getModelRule(), currentNode); 
            pushFollow(FOLLOW_ruleModel_in_entryRuleModel73);
            iv_ruleModel=ruleModel();
            _fsp--;

             current =iv_ruleModel; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleModel83); 

            }

        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end entryRuleModel


    // $ANTLR start ruleModel
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:80:1: ruleModel returns [EObject current=null] : ( (lv_imports_0= ruleImport )* (lv_elements_1= ruleType )* ) ;
    public final EObject ruleModel() throws RecognitionException {
        EObject current = null;

        EObject lv_imports_0 = null;

        EObject lv_elements_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:85:6: ( ( (lv_imports_0= ruleImport )* (lv_elements_1= ruleType )* ) )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:86:1: ( (lv_imports_0= ruleImport )* (lv_elements_1= ruleType )* )
            {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:86:1: ( (lv_imports_0= ruleImport )* (lv_elements_1= ruleType )* )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:86:2: (lv_imports_0= ruleImport )* (lv_elements_1= ruleType )*
            {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:86:2: (lv_imports_0= ruleImport )*
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( (LA1_0==11) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:89:6: lv_imports_0= ruleImport
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getModelAccess().getImportsImportParserRuleCall_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleImport_in_ruleModel142);
            	    lv_imports_0=ruleImport();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getModelRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        
            	    	        try {
            	    	       		add(current, "imports", lv_imports_0, "Import", currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }
            	    break;

            	default :
            	    break loop1;
                }
            } while (true);

            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:107:3: (lv_elements_1= ruleType )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( ((LA2_0>=12 && LA2_0<=13)||LA2_0==15) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:110:6: lv_elements_1= ruleType
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getModelAccess().getElementsTypeParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleType_in_ruleModel181);
            	    lv_elements_1=ruleType();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getModelRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        
            	    	        try {
            	    	       		add(current, "elements", lv_elements_1, "Type", currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);


            }


            }

             resetLookahead(); 
                	lastConsumedNode = currentNode;
                
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end ruleModel


    // $ANTLR start entryRuleImport
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:135:1: entryRuleImport returns [EObject current=null] : iv_ruleImport= ruleImport EOF ;
    public final EObject entryRuleImport() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleImport = null;


        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:135:48: (iv_ruleImport= ruleImport EOF )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:136:2: iv_ruleImport= ruleImport EOF
            {
             currentNode = createCompositeNode(grammarAccess.getImportRule(), currentNode); 
            pushFollow(FOLLOW_ruleImport_in_entryRuleImport219);
            iv_ruleImport=ruleImport();
            _fsp--;

             current =iv_ruleImport; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleImport229); 

            }

        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end entryRuleImport


    // $ANTLR start ruleImport
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:143:1: ruleImport returns [EObject current=null] : ( 'import' (lv_importURI_1= RULE_STRING ) ) ;
    public final EObject ruleImport() throws RecognitionException {
        EObject current = null;

        Token lv_importURI_1=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:148:6: ( ( 'import' (lv_importURI_1= RULE_STRING ) ) )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:149:1: ( 'import' (lv_importURI_1= RULE_STRING ) )
            {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:149:1: ( 'import' (lv_importURI_1= RULE_STRING ) )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:149:2: 'import' (lv_importURI_1= RULE_STRING )
            {
            match(input,11,FOLLOW_11_in_ruleImport263); 

                    createLeafNode(grammarAccess.getImportAccess().getImportKeyword_0(), null); 
                
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:153:1: (lv_importURI_1= RULE_STRING )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:155:6: lv_importURI_1= RULE_STRING
            {
            lv_importURI_1=(Token)input.LT(1);
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleImport285); 

            		createLeafNode(grammarAccess.getImportAccess().getImportURISTRINGTerminalRuleCall_1_0(), "importURI"); 
            	

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getImportRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        
            	        try {
            	       		set(current, "importURI", lv_importURI_1, "STRING", lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }


            }

             resetLookahead(); 
                	lastConsumedNode = currentNode;
                
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end ruleImport


    // $ANTLR start entryRuleType
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:180:1: entryRuleType returns [EObject current=null] : iv_ruleType= ruleType EOF ;
    public final EObject entryRuleType() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleType = null;


        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:180:46: (iv_ruleType= ruleType EOF )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:181:2: iv_ruleType= ruleType EOF
            {
             currentNode = createCompositeNode(grammarAccess.getTypeRule(), currentNode); 
            pushFollow(FOLLOW_ruleType_in_entryRuleType326);
            iv_ruleType=ruleType();
            _fsp--;

             current =iv_ruleType; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleType336); 

            }

        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end entryRuleType


    // $ANTLR start ruleType
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:188:1: ruleType returns [EObject current=null] : (this_SimpleType_0= ruleSimpleType | this_Entity_1= ruleEntity | this_CampaignType_2= ruleCampaignType ) ;
    public final EObject ruleType() throws RecognitionException {
        EObject current = null;

        EObject this_SimpleType_0 = null;

        EObject this_Entity_1 = null;

        EObject this_CampaignType_2 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:193:6: ( (this_SimpleType_0= ruleSimpleType | this_Entity_1= ruleEntity | this_CampaignType_2= ruleCampaignType ) )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:194:1: (this_SimpleType_0= ruleSimpleType | this_Entity_1= ruleEntity | this_CampaignType_2= ruleCampaignType )
            {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:194:1: (this_SimpleType_0= ruleSimpleType | this_Entity_1= ruleEntity | this_CampaignType_2= ruleCampaignType )
            int alt3=3;
            switch ( input.LA(1) ) {
            case 12:
                {
                alt3=1;
                }
                break;
            case 15:
                {
                alt3=2;
                }
                break;
            case 13:
                {
                alt3=3;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("194:1: (this_SimpleType_0= ruleSimpleType | this_Entity_1= ruleEntity | this_CampaignType_2= ruleCampaignType )", 3, 0, input);

                throw nvae;
            }

            switch (alt3) {
                case 1 :
                    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:195:5: this_SimpleType_0= ruleSimpleType
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getTypeAccess().getSimpleTypeParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleSimpleType_in_ruleType383);
                    this_SimpleType_0=ruleSimpleType();
                    _fsp--;

                     
                            current = this_SimpleType_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:205:5: this_Entity_1= ruleEntity
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getTypeAccess().getEntityParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleEntity_in_ruleType410);
                    this_Entity_1=ruleEntity();
                    _fsp--;

                     
                            current = this_Entity_1; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:215:5: this_CampaignType_2= ruleCampaignType
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getTypeAccess().getCampaignTypeParserRuleCall_2(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleCampaignType_in_ruleType437);
                    this_CampaignType_2=ruleCampaignType();
                    _fsp--;

                     
                            current = this_CampaignType_2; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;

            }


            }

             resetLookahead(); 
                	lastConsumedNode = currentNode;
                
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end ruleType


    // $ANTLR start entryRuleSimpleType
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:230:1: entryRuleSimpleType returns [EObject current=null] : iv_ruleSimpleType= ruleSimpleType EOF ;
    public final EObject entryRuleSimpleType() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleSimpleType = null;


        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:230:52: (iv_ruleSimpleType= ruleSimpleType EOF )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:231:2: iv_ruleSimpleType= ruleSimpleType EOF
            {
             currentNode = createCompositeNode(grammarAccess.getSimpleTypeRule(), currentNode); 
            pushFollow(FOLLOW_ruleSimpleType_in_entryRuleSimpleType469);
            iv_ruleSimpleType=ruleSimpleType();
            _fsp--;

             current =iv_ruleSimpleType; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleSimpleType479); 

            }

        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end entryRuleSimpleType


    // $ANTLR start ruleSimpleType
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:238:1: ruleSimpleType returns [EObject current=null] : ( 'type' (lv_name_1= RULE_ID ) ) ;
    public final EObject ruleSimpleType() throws RecognitionException {
        EObject current = null;

        Token lv_name_1=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:243:6: ( ( 'type' (lv_name_1= RULE_ID ) ) )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:244:1: ( 'type' (lv_name_1= RULE_ID ) )
            {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:244:1: ( 'type' (lv_name_1= RULE_ID ) )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:244:2: 'type' (lv_name_1= RULE_ID )
            {
            match(input,12,FOLLOW_12_in_ruleSimpleType513); 

                    createLeafNode(grammarAccess.getSimpleTypeAccess().getTypeKeyword_0(), null); 
                
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:248:1: (lv_name_1= RULE_ID )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:250:6: lv_name_1= RULE_ID
            {
            lv_name_1=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleSimpleType535); 

            		createLeafNode(grammarAccess.getSimpleTypeAccess().getNameIDTerminalRuleCall_1_0(), "name"); 
            	

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getSimpleTypeRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        
            	        try {
            	       		set(current, "name", lv_name_1, "ID", lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }


            }

             resetLookahead(); 
                	lastConsumedNode = currentNode;
                
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end ruleSimpleType


    // $ANTLR start entryRuleCampaignType
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:275:1: entryRuleCampaignType returns [EObject current=null] : iv_ruleCampaignType= ruleCampaignType EOF ;
    public final EObject entryRuleCampaignType() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleCampaignType = null;


        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:275:54: (iv_ruleCampaignType= ruleCampaignType EOF )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:276:2: iv_ruleCampaignType= ruleCampaignType EOF
            {
             currentNode = createCompositeNode(grammarAccess.getCampaignTypeRule(), currentNode); 
            pushFollow(FOLLOW_ruleCampaignType_in_entryRuleCampaignType576);
            iv_ruleCampaignType=ruleCampaignType();
            _fsp--;

             current =iv_ruleCampaignType; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleCampaignType586); 

            }

        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end entryRuleCampaignType


    // $ANTLR start ruleCampaignType
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:283:1: ruleCampaignType returns [EObject current=null] : ( '[campaign]' (lv_name_1= RULE_ID )? '[/campaign]' ) ;
    public final EObject ruleCampaignType() throws RecognitionException {
        EObject current = null;

        Token lv_name_1=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:288:6: ( ( '[campaign]' (lv_name_1= RULE_ID )? '[/campaign]' ) )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:289:1: ( '[campaign]' (lv_name_1= RULE_ID )? '[/campaign]' )
            {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:289:1: ( '[campaign]' (lv_name_1= RULE_ID )? '[/campaign]' )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:289:2: '[campaign]' (lv_name_1= RULE_ID )? '[/campaign]'
            {
            match(input,13,FOLLOW_13_in_ruleCampaignType620); 

                    createLeafNode(grammarAccess.getCampaignTypeAccess().getCampaignKeyword_0(), null); 
                
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:293:1: (lv_name_1= RULE_ID )?
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( (LA4_0==RULE_ID) ) {
                alt4=1;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:295:6: lv_name_1= RULE_ID
                    {
                    lv_name_1=(Token)input.LT(1);
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleCampaignType642); 

                    		createLeafNode(grammarAccess.getCampaignTypeAccess().getNameIDTerminalRuleCall_1_0(), "name"); 
                    	

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getCampaignTypeRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "name", lv_name_1, "ID", lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;

            }

            match(input,14,FOLLOW_14_in_ruleCampaignType660); 

                    createLeafNode(grammarAccess.getCampaignTypeAccess().getCampaignKeyword_2(), null); 
                

            }


            }

             resetLookahead(); 
                	lastConsumedNode = currentNode;
                
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end ruleCampaignType


    // $ANTLR start entryRuleEntity
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:324:1: entryRuleEntity returns [EObject current=null] : iv_ruleEntity= ruleEntity EOF ;
    public final EObject entryRuleEntity() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleEntity = null;


        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:324:48: (iv_ruleEntity= ruleEntity EOF )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:325:2: iv_ruleEntity= ruleEntity EOF
            {
             currentNode = createCompositeNode(grammarAccess.getEntityRule(), currentNode); 
            pushFollow(FOLLOW_ruleEntity_in_entryRuleEntity693);
            iv_ruleEntity=ruleEntity();
            _fsp--;

             current =iv_ruleEntity; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleEntity703); 

            }

        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end entryRuleEntity


    // $ANTLR start ruleEntity
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:332:1: ruleEntity returns [EObject current=null] : ( 'entity' (lv_name_1= RULE_ID ) ( 'extends' ( RULE_ID ) )? '{' (lv_properties_5= ruleProperty )* '}' ) ;
    public final EObject ruleEntity() throws RecognitionException {
        EObject current = null;

        Token lv_name_1=null;
        EObject lv_properties_5 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:337:6: ( ( 'entity' (lv_name_1= RULE_ID ) ( 'extends' ( RULE_ID ) )? '{' (lv_properties_5= ruleProperty )* '}' ) )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:338:1: ( 'entity' (lv_name_1= RULE_ID ) ( 'extends' ( RULE_ID ) )? '{' (lv_properties_5= ruleProperty )* '}' )
            {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:338:1: ( 'entity' (lv_name_1= RULE_ID ) ( 'extends' ( RULE_ID ) )? '{' (lv_properties_5= ruleProperty )* '}' )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:338:2: 'entity' (lv_name_1= RULE_ID ) ( 'extends' ( RULE_ID ) )? '{' (lv_properties_5= ruleProperty )* '}'
            {
            match(input,15,FOLLOW_15_in_ruleEntity737); 

                    createLeafNode(grammarAccess.getEntityAccess().getEntityKeyword_0(), null); 
                
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:342:1: (lv_name_1= RULE_ID )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:344:6: lv_name_1= RULE_ID
            {
            lv_name_1=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleEntity759); 

            		createLeafNode(grammarAccess.getEntityAccess().getNameIDTerminalRuleCall_1_0(), "name"); 
            	

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getEntityRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        
            	        try {
            	       		set(current, "name", lv_name_1, "ID", lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }

            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:362:2: ( 'extends' ( RULE_ID ) )?
            int alt5=2;
            int LA5_0 = input.LA(1);

            if ( (LA5_0==16) ) {
                alt5=1;
            }
            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:362:3: 'extends' ( RULE_ID )
                    {
                    match(input,16,FOLLOW_16_in_ruleEntity777); 

                            createLeafNode(grammarAccess.getEntityAccess().getExtendsKeyword_2_0(), null); 
                        
                    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:366:1: ( RULE_ID )
                    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:369:3: RULE_ID
                    {

                    			if (current==null) {
                    	            current = factory.create(grammarAccess.getEntityRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                            
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleEntity799); 

                    		createLeafNode(grammarAccess.getEntityAccess().getExtendsEntityCrossReference_2_1_0(), "extends"); 
                    	

                    }


                    }
                    break;

            }

            match(input,17,FOLLOW_17_in_ruleEntity813); 

                    createLeafNode(grammarAccess.getEntityAccess().getLeftCurlyBracketKeyword_3(), null); 
                
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:386:1: (lv_properties_5= ruleProperty )*
            loop6:
            do {
                int alt6=2;
                int LA6_0 = input.LA(1);

                if ( (LA6_0==19) ) {
                    alt6=1;
                }


                switch (alt6) {
            	case 1 :
            	    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:389:6: lv_properties_5= ruleProperty
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getEntityAccess().getPropertiesPropertyParserRuleCall_4_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleProperty_in_ruleEntity847);
            	    lv_properties_5=ruleProperty();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getEntityRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        
            	    	        try {
            	    	       		add(current, "properties", lv_properties_5, "Property", currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }
            	    break;

            	default :
            	    break loop6;
                }
            } while (true);

            match(input,18,FOLLOW_18_in_ruleEntity861); 

                    createLeafNode(grammarAccess.getEntityAccess().getRightCurlyBracketKeyword_5(), null); 
                

            }


            }

             resetLookahead(); 
                	lastConsumedNode = currentNode;
                
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end ruleEntity


    // $ANTLR start entryRuleProperty
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:418:1: entryRuleProperty returns [EObject current=null] : iv_ruleProperty= ruleProperty EOF ;
    public final EObject entryRuleProperty() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleProperty = null;


        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:418:50: (iv_ruleProperty= ruleProperty EOF )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:419:2: iv_ruleProperty= ruleProperty EOF
            {
             currentNode = createCompositeNode(grammarAccess.getPropertyRule(), currentNode); 
            pushFollow(FOLLOW_ruleProperty_in_entryRuleProperty894);
            iv_ruleProperty=ruleProperty();
            _fsp--;

             current =iv_ruleProperty; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleProperty904); 

            }

        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end entryRuleProperty


    // $ANTLR start ruleProperty
    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:426:1: ruleProperty returns [EObject current=null] : ( 'property' (lv_name_1= RULE_ID ) ':' ( RULE_ID ) (lv_many_4= '[]' )? ) ;
    public final EObject ruleProperty() throws RecognitionException {
        EObject current = null;

        Token lv_name_1=null;
        Token lv_many_4=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:431:6: ( ( 'property' (lv_name_1= RULE_ID ) ':' ( RULE_ID ) (lv_many_4= '[]' )? ) )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:432:1: ( 'property' (lv_name_1= RULE_ID ) ':' ( RULE_ID ) (lv_many_4= '[]' )? )
            {
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:432:1: ( 'property' (lv_name_1= RULE_ID ) ':' ( RULE_ID ) (lv_many_4= '[]' )? )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:432:2: 'property' (lv_name_1= RULE_ID ) ':' ( RULE_ID ) (lv_many_4= '[]' )?
            {
            match(input,19,FOLLOW_19_in_ruleProperty938); 

                    createLeafNode(grammarAccess.getPropertyAccess().getPropertyKeyword_0(), null); 
                
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:436:1: (lv_name_1= RULE_ID )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:438:6: lv_name_1= RULE_ID
            {
            lv_name_1=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleProperty960); 

            		createLeafNode(grammarAccess.getPropertyAccess().getNameIDTerminalRuleCall_1_0(), "name"); 
            	

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getPropertyRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        
            	        try {
            	       		set(current, "name", lv_name_1, "ID", lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }

            match(input,20,FOLLOW_20_in_ruleProperty977); 

                    createLeafNode(grammarAccess.getPropertyAccess().getColonKeyword_2(), null); 
                
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:460:1: ( RULE_ID )
            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:463:3: RULE_ID
            {

            			if (current==null) {
            	            current = factory.create(grammarAccess.getPropertyRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
                    
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleProperty999); 

            		createLeafNode(grammarAccess.getPropertyAccess().getTypeTypeCrossReference_3_0(), "type"); 
            	

            }

            // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:476:2: (lv_many_4= '[]' )?
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( (LA7_0==21) ) {
                alt7=1;
            }
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.wml.ide/src-gen/org/wesnoth/parser/antlr/internal/InternalWml.g:478:6: lv_many_4= '[]'
                    {
                    lv_many_4=(Token)input.LT(1);
                    match(input,21,FOLLOW_21_in_ruleProperty1023); 

                            createLeafNode(grammarAccess.getPropertyAccess().getManyLeftSquareBracketRightSquareBracketKeyword_4_0(), "many"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getPropertyRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "many", true, "[]", lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;

            }


            }


            }

             resetLookahead(); 
                	lastConsumedNode = currentNode;
                
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end ruleProperty


 

    public static final BitSet FOLLOW_ruleModel_in_entryRuleModel73 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleModel83 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleImport_in_ruleModel142 = new BitSet(new long[]{0x000000000000B802L});
    public static final BitSet FOLLOW_ruleType_in_ruleModel181 = new BitSet(new long[]{0x000000000000B002L});
    public static final BitSet FOLLOW_ruleImport_in_entryRuleImport219 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleImport229 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_ruleImport263 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleImport285 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleType_in_entryRuleType326 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleType336 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleSimpleType_in_ruleType383 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleEntity_in_ruleType410 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleCampaignType_in_ruleType437 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleSimpleType_in_entryRuleSimpleType469 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleSimpleType479 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_ruleSimpleType513 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleSimpleType535 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleCampaignType_in_entryRuleCampaignType576 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleCampaignType586 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_ruleCampaignType620 = new BitSet(new long[]{0x0000000000004020L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleCampaignType642 = new BitSet(new long[]{0x0000000000004000L});
    public static final BitSet FOLLOW_14_in_ruleCampaignType660 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleEntity_in_entryRuleEntity693 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleEntity703 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_ruleEntity737 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleEntity759 = new BitSet(new long[]{0x0000000000030000L});
    public static final BitSet FOLLOW_16_in_ruleEntity777 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleEntity799 = new BitSet(new long[]{0x0000000000020000L});
    public static final BitSet FOLLOW_17_in_ruleEntity813 = new BitSet(new long[]{0x00000000000C0000L});
    public static final BitSet FOLLOW_ruleProperty_in_ruleEntity847 = new BitSet(new long[]{0x00000000000C0000L});
    public static final BitSet FOLLOW_18_in_ruleEntity861 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleProperty_in_entryRuleProperty894 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleProperty904 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_ruleProperty938 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleProperty960 = new BitSet(new long[]{0x0000000000100000L});
    public static final BitSet FOLLOW_20_in_ruleProperty977 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleProperty999 = new BitSet(new long[]{0x0000000000200002L});
    public static final BitSet FOLLOW_21_in_ruleProperty1023 = new BitSet(new long[]{0x0000000000000002L});

}