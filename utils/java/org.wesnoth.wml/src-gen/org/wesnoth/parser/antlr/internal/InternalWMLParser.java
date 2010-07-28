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
import org.wesnoth.services.WMLGrammarAccess;



import org.antlr.runtime.*;
import java.util.Stack;
import java.util.List;
import java.util.ArrayList;

@SuppressWarnings("all")
public class InternalWMLParser extends AbstractInternalAntlrParser {
    public static final String[] tokenNames = new String[] {
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_MACRO", "RULE_ID", "RULE_STRING", "RULE_IINT", "RULE_SL_COMMENT", "RULE_WS", "'['", "']'", "'[/'", "'='", "'.'", "' '", "'_'", "'-'", "'/'", "'n'", "'s'", "'w'", "'e'", "'sw'", "'se'", "'ne'", "'nw'", "','", "'~'", "':'"
    };
    public static final int RULE_ID=5;
    public static final int RULE_STRING=6;
    public static final int RULE_IINT=7;
    public static final int RULE_WS=9;
    public static final int RULE_SL_COMMENT=8;
    public static final int EOF=-1;
    public static final int RULE_MACRO=4;

        public InternalWMLParser(TokenStream input) {
            super(input);
        }
        

    public String[] getTokenNames() { return tokenNames; }
    public String getGrammarFileName() { return "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g"; }



     	private WMLGrammarAccess grammarAccess;
     	
        public InternalWMLParser(TokenStream input, IAstFactory factory, WMLGrammarAccess grammarAccess) {
            this(input);
            this.factory = factory;
            registerRules(grammarAccess.getGrammar());
            this.grammarAccess = grammarAccess;
        }
        
        @Override
        protected InputStream getTokenFile() {
        	ClassLoader classLoader = getClass().getClassLoader();
        	return classLoader.getResourceAsStream("org/wesnoth/parser/antlr/internal/InternalWML.tokens");
        }
        
        @Override
        protected String getFirstRuleName() {
        	return "WMLRoot";	
       	}
       	
       	@Override
       	protected WMLGrammarAccess getGrammarAccess() {
       		return grammarAccess;
       	}



    // $ANTLR start entryRuleWMLRoot
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:77:1: entryRuleWMLRoot returns [EObject current=null] : iv_ruleWMLRoot= ruleWMLRoot EOF ;
    public final EObject entryRuleWMLRoot() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLRoot = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:78:2: (iv_ruleWMLRoot= ruleWMLRoot EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:79:2: iv_ruleWMLRoot= ruleWMLRoot EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLRootRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot75);
            iv_ruleWMLRoot=ruleWMLRoot();
            _fsp--;

             current =iv_ruleWMLRoot; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLRoot85); 

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
    // $ANTLR end entryRuleWMLRoot


    // $ANTLR start ruleWMLRoot
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:86:1: ruleWMLRoot returns [EObject current=null] : ( ( (lv_Rtags_0_0= ruleWMLTag ) ) | ( (lv_Rmacros_1_0= ruleWMLMacro ) ) )* ;
    public final EObject ruleWMLRoot() throws RecognitionException {
        EObject current = null;

        EObject lv_Rtags_0_0 = null;

        EObject lv_Rmacros_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:91:6: ( ( ( (lv_Rtags_0_0= ruleWMLTag ) ) | ( (lv_Rmacros_1_0= ruleWMLMacro ) ) )* )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( ( (lv_Rtags_0_0= ruleWMLTag ) ) | ( (lv_Rmacros_1_0= ruleWMLMacro ) ) )*
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( ( (lv_Rtags_0_0= ruleWMLTag ) ) | ( (lv_Rmacros_1_0= ruleWMLMacro ) ) )*
            loop1:
            do {
                int alt1=3;
                int LA1_0 = input.LA(1);

                if ( (LA1_0==10) ) {
                    alt1=1;
                }
                else if ( (LA1_0==RULE_MACRO) ) {
                    alt1=2;
                }


                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:2: ( (lv_Rtags_0_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:2: ( (lv_Rtags_0_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:93:1: (lv_Rtags_0_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:93:1: (lv_Rtags_0_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:94:3: lv_Rtags_0_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getRtagsWMLTagParserRuleCall_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLRoot131);
            	    lv_Rtags_0_0=ruleWMLTag();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Rtags",
            	    	        		lv_Rtags_0_0, 
            	    	        		"WMLTag", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }


            	    }
            	    break;
            	case 2 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:117:6: ( (lv_Rmacros_1_0= ruleWMLMacro ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:117:6: ( (lv_Rmacros_1_0= ruleWMLMacro ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:118:1: (lv_Rmacros_1_0= ruleWMLMacro )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:118:1: (lv_Rmacros_1_0= ruleWMLMacro )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:119:3: lv_Rmacros_1_0= ruleWMLMacro
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getRmacrosWMLMacroParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacro_in_ruleWMLRoot158);
            	    lv_Rmacros_1_0=ruleWMLMacro();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Rmacros",
            	    	        		lv_Rmacros_1_0, 
            	    	        		"WMLMacro", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }


            	    }
            	    break;

            	default :
            	    break loop1;
                }
            } while (true);


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
    // $ANTLR end ruleWMLRoot


    // $ANTLR start entryRuleWMLMacro
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:149:1: entryRuleWMLMacro returns [EObject current=null] : iv_ruleWMLMacro= ruleWMLMacro EOF ;
    public final EObject entryRuleWMLMacro() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacro = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:150:2: (iv_ruleWMLMacro= ruleWMLMacro EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:151:2: iv_ruleWMLMacro= ruleWMLMacro EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacro_in_entryRuleWMLMacro195);
            iv_ruleWMLMacro=ruleWMLMacro();
            _fsp--;

             current =iv_ruleWMLMacro; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacro205); 

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
    // $ANTLR end entryRuleWMLMacro


    // $ANTLR start ruleWMLMacro
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:158:1: ruleWMLMacro returns [EObject current=null] : ( (lv_name_0_0= RULE_MACRO ) ) ;
    public final EObject ruleWMLMacro() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:163:6: ( ( (lv_name_0_0= RULE_MACRO ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:1: ( (lv_name_0_0= RULE_MACRO ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:1: ( (lv_name_0_0= RULE_MACRO ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:165:1: (lv_name_0_0= RULE_MACRO )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:165:1: (lv_name_0_0= RULE_MACRO )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:166:3: lv_name_0_0= RULE_MACRO
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_MACRO,FOLLOW_RULE_MACRO_in_ruleWMLMacro246); 

            			createLeafNode(grammarAccess.getWMLMacroAccess().getNameMACROTerminalRuleCall_0(), "name"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"name",
            	        		lv_name_0_0, 
            	        		"MACRO", 
            	        		lastConsumedNode);
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
    // $ANTLR end ruleWMLMacro


    // $ANTLR start entryRuleWMLTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:196:1: entryRuleWMLTag returns [EObject current=null] : iv_ruleWMLTag= ruleWMLTag EOF ;
    public final EObject entryRuleWMLTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:197:2: (iv_ruleWMLTag= ruleWMLTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:198:2: iv_ruleWMLTag= ruleWMLTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLTag_in_entryRuleWMLTag286);
            iv_ruleWMLTag=ruleWMLTag();
            _fsp--;

             current =iv_ruleWMLTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTag296); 

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
    // $ANTLR end entryRuleWMLTag


    // $ANTLR start ruleWMLTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:205:1: ruleWMLTag returns [EObject current=null] : ( '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* '[/' ( (lv_endName_7_0= RULE_ID ) ) ']' ) ;
    public final EObject ruleWMLTag() throws RecognitionException {
        EObject current = null;

        Token lv_name_1_0=null;
        Token lv_endName_7_0=null;
        EObject lv_Ttags_3_0 = null;

        EObject lv_Tkeys_4_0 = null;

        EObject lv_Tmacros_5_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:210:6: ( ( '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* '[/' ( (lv_endName_7_0= RULE_ID ) ) ']' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:211:1: ( '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* '[/' ( (lv_endName_7_0= RULE_ID ) ) ']' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:211:1: ( '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* '[/' ( (lv_endName_7_0= RULE_ID ) ) ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:211:3: '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* '[/' ( (lv_endName_7_0= RULE_ID ) ) ']'
            {
            match(input,10,FOLLOW_10_in_ruleWMLTag331); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:215:1: ( (lv_name_1_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:216:1: (lv_name_1_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:216:1: (lv_name_1_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:217:3: lv_name_1_0= RULE_ID
            {
            lv_name_1_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag348); 

            			createLeafNode(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_1_0(), "name"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"name",
            	        		lv_name_1_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            match(input,11,FOLLOW_11_in_ruleWMLTag363); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_2(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:243:1: ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )*
            loop2:
            do {
                int alt2=4;
                switch ( input.LA(1) ) {
                case 10:
                    {
                    alt2=1;
                    }
                    break;
                case RULE_ID:
                    {
                    alt2=2;
                    }
                    break;
                case RULE_MACRO:
                    {
                    alt2=3;
                    }
                    break;

                }

                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:243:2: ( (lv_Ttags_3_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:243:2: ( (lv_Ttags_3_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:244:1: (lv_Ttags_3_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:244:1: (lv_Ttags_3_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:245:3: lv_Ttags_3_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTtagsWMLTagParserRuleCall_3_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLTag385);
            	    lv_Ttags_3_0=ruleWMLTag();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Ttags",
            	    	        		lv_Ttags_3_0, 
            	    	        		"WMLTag", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }


            	    }
            	    break;
            	case 2 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:268:6: ( (lv_Tkeys_4_0= ruleWMLKey ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:268:6: ( (lv_Tkeys_4_0= ruleWMLKey ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:269:1: (lv_Tkeys_4_0= ruleWMLKey )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:269:1: (lv_Tkeys_4_0= ruleWMLKey )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:270:3: lv_Tkeys_4_0= ruleWMLKey
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTkeysWMLKeyParserRuleCall_3_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLTag412);
            	    lv_Tkeys_4_0=ruleWMLKey();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Tkeys",
            	    	        		lv_Tkeys_4_0, 
            	    	        		"WMLKey", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }


            	    }
            	    break;
            	case 3 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:293:6: ( (lv_Tmacros_5_0= ruleWMLMacro ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:293:6: ( (lv_Tmacros_5_0= ruleWMLMacro ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:294:1: (lv_Tmacros_5_0= ruleWMLMacro )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:294:1: (lv_Tmacros_5_0= ruleWMLMacro )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:295:3: lv_Tmacros_5_0= ruleWMLMacro
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTmacrosWMLMacroParserRuleCall_3_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacro_in_ruleWMLTag439);
            	    lv_Tmacros_5_0=ruleWMLMacro();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Tmacros",
            	    	        		lv_Tmacros_5_0, 
            	    	        		"WMLMacro", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }


            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);

            match(input,12,FOLLOW_12_in_ruleWMLTag451); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_4(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:321:1: ( (lv_endName_7_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:322:1: (lv_endName_7_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:322:1: (lv_endName_7_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:323:3: lv_endName_7_0= RULE_ID
            {
            lv_endName_7_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag468); 

            			createLeafNode(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_5_0(), "endName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"endName",
            	        		lv_endName_7_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            match(input,11,FOLLOW_11_in_ruleWMLTag483); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_6(), null); 
                

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
    // $ANTLR end ruleWMLTag


    // $ANTLR start entryRuleWMLKey
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:357:1: entryRuleWMLKey returns [EObject current=null] : iv_ruleWMLKey= ruleWMLKey EOF ;
    public final EObject entryRuleWMLKey() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKey = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:358:2: (iv_ruleWMLKey= ruleWMLKey EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:359:2: iv_ruleWMLKey= ruleWMLKey EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey519);
            iv_ruleWMLKey=ruleWMLKey();
            _fsp--;

             current =iv_ruleWMLKey; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey529); 

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
    // $ANTLR end entryRuleWMLKey


    // $ANTLR start ruleWMLKey
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:366:1: ruleWMLKey returns [EObject current=null] : ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ) ;
    public final EObject ruleWMLKey() throws RecognitionException {
        EObject current = null;

        Token lv_keyName_0_0=null;
        EObject lv_value_2_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:371:6: ( ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:372:1: ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:372:1: ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:372:2: ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:372:2: ( (lv_keyName_0_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:373:1: (lv_keyName_0_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:373:1: (lv_keyName_0_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:374:3: lv_keyName_0_0= RULE_ID
            {
            lv_keyName_0_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLKey571); 

            			createLeafNode(grammarAccess.getWMLKeyAccess().getKeyNameIDTerminalRuleCall_0_0(), "keyName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"keyName",
            	        		lv_keyName_0_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            match(input,13,FOLLOW_13_in_ruleWMLKey586); 

                    createLeafNode(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:400:1: ( (lv_value_2_0= ruleWMLKeyValue ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:401:1: (lv_value_2_0= ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:401:1: (lv_value_2_0= ruleWMLKeyValue )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:402:3: lv_value_2_0= ruleWMLKeyValue
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleWMLKeyValue_in_ruleWMLKey607);
            lv_value_2_0=ruleWMLKeyValue();
            _fsp--;


            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"value",
            	        		lv_value_2_0, 
            	        		"WMLKeyValue", 
            	        		currentNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	        currentNode = currentNode.getParent();
            	    

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
    // $ANTLR end ruleWMLKey


    // $ANTLR start entryRuleWMLKeyValue
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:432:1: entryRuleWMLKeyValue returns [EObject current=null] : iv_ruleWMLKeyValue= ruleWMLKeyValue EOF ;
    public final EObject entryRuleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKeyValue = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:433:2: (iv_ruleWMLKeyValue= ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:434:2: iv_ruleWMLKeyValue= ruleWMLKeyValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue643);
            iv_ruleWMLKeyValue=ruleWMLKeyValue();
            _fsp--;

             current =iv_ruleWMLKeyValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue653); 

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
    // $ANTLR end entryRuleWMLKeyValue


    // $ANTLR start ruleWMLKeyValue
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:441:1: ruleWMLKeyValue returns [EObject current=null] : ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) ) ;
    public final EObject ruleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        Token lv_key1Value_0_1=null;
        Token lv_key1Value_0_2=null;
        Token lv_key1Value_0_5=null;
        AntlrDatatypeRuleToken lv_key1Value_0_3 = null;

        AntlrDatatypeRuleToken lv_key1Value_0_4 = null;

        AntlrDatatypeRuleToken lv_key1Value_0_6 = null;

        AntlrDatatypeRuleToken lv_key1Value_0_7 = null;

        AntlrDatatypeRuleToken lv_key1Value_0_8 = null;

        AntlrDatatypeRuleToken lv_key1Value_0_9 = null;

        EObject lv_key2Value_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:446:6: ( ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:447:1: ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:447:1: ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) )
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( ((LA4_0>=RULE_ID && LA4_0<=RULE_IINT)||(LA4_0>=15 && LA4_0<=16)||(LA4_0>=19 && LA4_0<=26)) ) {
                alt4=1;
            }
            else if ( (LA4_0==RULE_MACRO) ) {
                alt4=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("447:1: ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) )", 4, 0, input);

                throw nvae;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:447:2: ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:447:2: ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:448:1: ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:448:1: ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:449:1: (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:449:1: (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE )
                    int alt3=9;
                    alt3 = dfa3.predict(input);
                    switch (alt3) {
                        case 1 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:450:3: lv_key1Value_0_1= RULE_ID
                            {
                            lv_key1Value_0_1=(Token)input.LT(1);
                            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLKeyValue697); 

                            			createLeafNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueIDTerminalRuleCall_0_0_0(), "key1Value"); 
                            		

                            	        if (current==null) {
                            	            current = factory.create(grammarAccess.getWMLKeyValueRule().getType().getClassifier());
                            	            associateNodeWithAstElement(currentNode, current);
                            	        }
                            	        try {
                            	       		set(
                            	       			current, 
                            	       			"key1Value",
                            	        		lv_key1Value_0_1, 
                            	        		"ID", 
                            	        		lastConsumedNode);
                            	        } catch (ValueConverterException vce) {
                            				handleValueConverterException(vce);
                            	        }
                            	    

                            }
                            break;
                        case 2 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:471:8: lv_key1Value_0_2= RULE_STRING
                            {
                            lv_key1Value_0_2=(Token)input.LT(1);
                            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLKeyValue717); 

                            			createLeafNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueSTRINGTerminalRuleCall_0_0_1(), "key1Value"); 
                            		

                            	        if (current==null) {
                            	            current = factory.create(grammarAccess.getWMLKeyValueRule().getType().getClassifier());
                            	            associateNodeWithAstElement(currentNode, current);
                            	        }
                            	        try {
                            	       		set(
                            	       			current, 
                            	       			"key1Value",
                            	        		lv_key1Value_0_2, 
                            	        		"STRING", 
                            	        		lastConsumedNode);
                            	        } catch (ValueConverterException vce) {
                            				handleValueConverterException(vce);
                            	        }
                            	    

                            }
                            break;
                        case 3 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:492:8: lv_key1Value_0_3= ruleTSTRING
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueTSTRINGParserRuleCall_0_0_2(), currentNode); 
                            	    
                            pushFollow(FOLLOW_ruleTSTRING_in_ruleWMLKeyValue741);
                            lv_key1Value_0_3=ruleTSTRING();
                            _fsp--;


                            	        if (current==null) {
                            	            current = factory.create(grammarAccess.getWMLKeyValueRule().getType().getClassifier());
                            	            associateNodeWithAstElement(currentNode.getParent(), current);
                            	        }
                            	        try {
                            	       		set(
                            	       			current, 
                            	       			"key1Value",
                            	        		lv_key1Value_0_3, 
                            	        		"TSTRING", 
                            	        		currentNode);
                            	        } catch (ValueConverterException vce) {
                            				handleValueConverterException(vce);
                            	        }
                            	        currentNode = currentNode.getParent();
                            	    

                            }
                            break;
                        case 4 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:513:8: lv_key1Value_0_4= ruleFLOAT
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueFLOATParserRuleCall_0_0_3(), currentNode); 
                            	    
                            pushFollow(FOLLOW_ruleFLOAT_in_ruleWMLKeyValue760);
                            lv_key1Value_0_4=ruleFLOAT();
                            _fsp--;


                            	        if (current==null) {
                            	            current = factory.create(grammarAccess.getWMLKeyValueRule().getType().getClassifier());
                            	            associateNodeWithAstElement(currentNode.getParent(), current);
                            	        }
                            	        try {
                            	       		set(
                            	       			current, 
                            	       			"key1Value",
                            	        		lv_key1Value_0_4, 
                            	        		"FLOAT", 
                            	        		currentNode);
                            	        } catch (ValueConverterException vce) {
                            				handleValueConverterException(vce);
                            	        }
                            	        currentNode = currentNode.getParent();
                            	    

                            }
                            break;
                        case 5 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:534:8: lv_key1Value_0_5= RULE_IINT
                            {
                            lv_key1Value_0_5=(Token)input.LT(1);
                            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_ruleWMLKeyValue775); 

                            			createLeafNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueIINTTerminalRuleCall_0_0_4(), "key1Value"); 
                            		

                            	        if (current==null) {
                            	            current = factory.create(grammarAccess.getWMLKeyValueRule().getType().getClassifier());
                            	            associateNodeWithAstElement(currentNode, current);
                            	        }
                            	        try {
                            	       		set(
                            	       			current, 
                            	       			"key1Value",
                            	        		lv_key1Value_0_5, 
                            	        		"IINT", 
                            	        		lastConsumedNode);
                            	        } catch (ValueConverterException vce) {
                            				handleValueConverterException(vce);
                            	        }
                            	    

                            }
                            break;
                        case 6 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:555:8: lv_key1Value_0_6= rulePATH
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValuePATHParserRuleCall_0_0_5(), currentNode); 
                            	    
                            pushFollow(FOLLOW_rulePATH_in_ruleWMLKeyValue799);
                            lv_key1Value_0_6=rulePATH();
                            _fsp--;


                            	        if (current==null) {
                            	            current = factory.create(grammarAccess.getWMLKeyValueRule().getType().getClassifier());
                            	            associateNodeWithAstElement(currentNode.getParent(), current);
                            	        }
                            	        try {
                            	       		set(
                            	       			current, 
                            	       			"key1Value",
                            	        		lv_key1Value_0_6, 
                            	        		"PATH", 
                            	        		currentNode);
                            	        } catch (ValueConverterException vce) {
                            				handleValueConverterException(vce);
                            	        }
                            	        currentNode = currentNode.getParent();
                            	    

                            }
                            break;
                        case 7 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:576:8: lv_key1Value_0_7= ruleDIRECTION
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueDIRECTIONParserRuleCall_0_0_6(), currentNode); 
                            	    
                            pushFollow(FOLLOW_ruleDIRECTION_in_ruleWMLKeyValue818);
                            lv_key1Value_0_7=ruleDIRECTION();
                            _fsp--;


                            	        if (current==null) {
                            	            current = factory.create(grammarAccess.getWMLKeyValueRule().getType().getClassifier());
                            	            associateNodeWithAstElement(currentNode.getParent(), current);
                            	        }
                            	        try {
                            	       		set(
                            	       			current, 
                            	       			"key1Value",
                            	        		lv_key1Value_0_7, 
                            	        		"DIRECTION", 
                            	        		currentNode);
                            	        } catch (ValueConverterException vce) {
                            				handleValueConverterException(vce);
                            	        }
                            	        currentNode = currentNode.getParent();
                            	    

                            }
                            break;
                        case 8 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:597:8: lv_key1Value_0_8= ruleLIST
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueLISTParserRuleCall_0_0_7(), currentNode); 
                            	    
                            pushFollow(FOLLOW_ruleLIST_in_ruleWMLKeyValue837);
                            lv_key1Value_0_8=ruleLIST();
                            _fsp--;


                            	        if (current==null) {
                            	            current = factory.create(grammarAccess.getWMLKeyValueRule().getType().getClassifier());
                            	            associateNodeWithAstElement(currentNode.getParent(), current);
                            	        }
                            	        try {
                            	       		set(
                            	       			current, 
                            	       			"key1Value",
                            	        		lv_key1Value_0_8, 
                            	        		"LIST", 
                            	        		currentNode);
                            	        } catch (ValueConverterException vce) {
                            				handleValueConverterException(vce);
                            	        }
                            	        currentNode = currentNode.getParent();
                            	    

                            }
                            break;
                        case 9 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:618:8: lv_key1Value_0_9= rulePROGRESSIVE
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValuePROGRESSIVEParserRuleCall_0_0_8(), currentNode); 
                            	    
                            pushFollow(FOLLOW_rulePROGRESSIVE_in_ruleWMLKeyValue856);
                            lv_key1Value_0_9=rulePROGRESSIVE();
                            _fsp--;


                            	        if (current==null) {
                            	            current = factory.create(grammarAccess.getWMLKeyValueRule().getType().getClassifier());
                            	            associateNodeWithAstElement(currentNode.getParent(), current);
                            	        }
                            	        try {
                            	       		set(
                            	       			current, 
                            	       			"key1Value",
                            	        		lv_key1Value_0_9, 
                            	        		"PROGRESSIVE", 
                            	        		currentNode);
                            	        } catch (ValueConverterException vce) {
                            				handleValueConverterException(vce);
                            	        }
                            	        currentNode = currentNode.getParent();
                            	    

                            }
                            break;

                    }


                    }


                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:643:6: ( (lv_key2Value_1_0= ruleWMLMacro ) )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:643:6: ( (lv_key2Value_1_0= ruleWMLMacro ) )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:644:1: (lv_key2Value_1_0= ruleWMLMacro )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:644:1: (lv_key2Value_1_0= ruleWMLMacro )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:645:3: lv_key2Value_1_0= ruleWMLMacro
                    {
                     
                    	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey2ValueWMLMacroParserRuleCall_1_0(), currentNode); 
                    	    
                    pushFollow(FOLLOW_ruleWMLMacro_in_ruleWMLKeyValue886);
                    lv_key2Value_1_0=ruleWMLMacro();
                    _fsp--;


                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLKeyValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode.getParent(), current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"key2Value",
                    	        		lv_key2Value_1_0, 
                    	        		"WMLMacro", 
                    	        		currentNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	        currentNode = currentNode.getParent();
                    	    

                    }


                    }


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
    // $ANTLR end ruleWMLKeyValue


    // $ANTLR start entryRuleFLOAT
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:675:1: entryRuleFLOAT returns [String current=null] : iv_ruleFLOAT= ruleFLOAT EOF ;
    public final String entryRuleFLOAT() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleFLOAT = null;


         
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();
        	
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:679:2: (iv_ruleFLOAT= ruleFLOAT EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:680:2: iv_ruleFLOAT= ruleFLOAT EOF
            {
             currentNode = createCompositeNode(grammarAccess.getFLOATRule(), currentNode); 
            pushFollow(FOLLOW_ruleFLOAT_in_entryRuleFLOAT929);
            iv_ruleFLOAT=ruleFLOAT();
            _fsp--;

             current =iv_ruleFLOAT.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleFLOAT940); 

            }

        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {

            	myHiddenTokenState.restore();

        }
        return current;
    }
    // $ANTLR end entryRuleFLOAT


    // $ANTLR start ruleFLOAT
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:690:1: ruleFLOAT returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+ ) ;
    public final AntlrDatatypeRuleToken ruleFLOAT() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_IINT_0=null;
        Token kw=null;
        Token this_IINT_2=null;

         setCurrentLookahead(); resetLookahead(); 
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:696:6: ( (this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:697:1: (this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:697:1: (this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:697:6: this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+
            {
            this_IINT_0=(Token)input.LT(1);
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_ruleFLOAT984); 

            		current.merge(this_IINT_0);
                
             
                createLeafNode(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_0(), null); 
                
            kw=(Token)input.LT(1);
            match(input,14,FOLLOW_14_in_ruleFLOAT1002); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getFLOATAccess().getFullStopKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:710:1: (this_IINT_2= RULE_IINT )+
            int cnt5=0;
            loop5:
            do {
                int alt5=2;
                int LA5_0 = input.LA(1);

                if ( (LA5_0==RULE_IINT) ) {
                    alt5=1;
                }


                switch (alt5) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:710:6: this_IINT_2= RULE_IINT
            	    {
            	    this_IINT_2=(Token)input.LT(1);
            	    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_ruleFLOAT1018); 

            	    		current.merge(this_IINT_2);
            	        
            	     
            	        createLeafNode(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2(), null); 
            	        

            	    }
            	    break;

            	default :
            	    if ( cnt5 >= 1 ) break loop5;
                        EarlyExitException eee =
                            new EarlyExitException(5, input);
                        throw eee;
                }
                cnt5++;
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

            	myHiddenTokenState.restore();

        }
        return current;
    }
    // $ANTLR end ruleFLOAT


    // $ANTLR start entryRuleTSTRING
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:728:1: entryRuleTSTRING returns [String current=null] : iv_ruleTSTRING= ruleTSTRING EOF ;
    public final String entryRuleTSTRING() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleTSTRING = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:729:2: (iv_ruleTSTRING= ruleTSTRING EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:730:2: iv_ruleTSTRING= ruleTSTRING EOF
            {
             currentNode = createCompositeNode(grammarAccess.getTSTRINGRule(), currentNode); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING1070);
            iv_ruleTSTRING=ruleTSTRING();
            _fsp--;

             current =iv_ruleTSTRING.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING1081); 

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
    // $ANTLR end entryRuleTSTRING


    // $ANTLR start ruleTSTRING
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:737:1: ruleTSTRING returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING ) ;
    public final AntlrDatatypeRuleToken ruleTSTRING() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        Token this_STRING_3=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:742:6: ( ( ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:743:1: ( ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:743:1: ( ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:743:2: ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:743:2: ( (kw= ' ' )? kw= '_' (kw= ' ' )? )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:743:3: (kw= ' ' )? kw= '_' (kw= ' ' )?
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:743:3: (kw= ' ' )?
            int alt6=2;
            int LA6_0 = input.LA(1);

            if ( (LA6_0==15) ) {
                alt6=1;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:744:2: kw= ' '
                    {
                    kw=(Token)input.LT(1);
                    match(input,15,FOLLOW_15_in_ruleTSTRING1121); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_0(), null); 
                        

                    }
                    break;

            }

            kw=(Token)input.LT(1);
            match(input,16,FOLLOW_16_in_ruleTSTRING1136); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getTSTRINGAccess().get_Keyword_0_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:755:1: (kw= ' ' )?
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( (LA7_0==15) ) {
                alt7=1;
            }
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:756:2: kw= ' '
                    {
                    kw=(Token)input.LT(1);
                    match(input,15,FOLLOW_15_in_ruleTSTRING1150); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_2(), null); 
                        

                    }
                    break;

            }


            }

            this_STRING_3=(Token)input.LT(1);
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleTSTRING1168); 

            		current.merge(this_STRING_3);
                
             
                createLeafNode(grammarAccess.getTSTRINGAccess().getSTRINGTerminalRuleCall_1(), null); 
                

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
    // $ANTLR end ruleTSTRING


    // $ANTLR start entryRulePATH
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:776:1: entryRulePATH returns [String current=null] : iv_rulePATH= rulePATH EOF ;
    public final String entryRulePATH() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_rulePATH = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:777:2: (iv_rulePATH= rulePATH EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:778:2: iv_rulePATH= rulePATH EOF
            {
             currentNode = createCompositeNode(grammarAccess.getPATHRule(), currentNode); 
            pushFollow(FOLLOW_rulePATH_in_entryRulePATH1214);
            iv_rulePATH=rulePATH();
            _fsp--;

             current =iv_rulePATH.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePATH1225); 

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
    // $ANTLR end entryRulePATH


    // $ANTLR start rulePATH
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:785:1: rulePATH returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+ ) ;
    public final AntlrDatatypeRuleToken rulePATH() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;
        Token kw=null;
        Token this_ID_3=null;
        Token this_ID_5=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:790:6: ( ( ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:791:1: ( ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:791:1: ( ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:791:2: ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:791:2: ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )*
            loop10:
            do {
                int alt10=2;
                alt10 = dfa10.predict(input);
                switch (alt10) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:791:3: (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:791:3: (this_ID_0= RULE_ID )+
            	    int cnt8=0;
            	    loop8:
            	    do {
            	        int alt8=2;
            	        int LA8_0 = input.LA(1);

            	        if ( (LA8_0==RULE_ID) ) {
            	            alt8=1;
            	        }


            	        switch (alt8) {
            	    	case 1 :
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:791:8: this_ID_0= RULE_ID
            	    	    {
            	    	    this_ID_0=(Token)input.LT(1);
            	    	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rulePATH1267); 

            	    	    		current.merge(this_ID_0);
            	    	        
            	    	     
            	    	        createLeafNode(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0(), null); 
            	    	        

            	    	    }
            	    	    break;

            	    	default :
            	    	    if ( cnt8 >= 1 ) break loop8;
            	                EarlyExitException eee =
            	                    new EarlyExitException(8, input);
            	                throw eee;
            	        }
            	        cnt8++;
            	    } while (true);

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:798:3: (kw= '-' | kw= '/' )
            	    int alt9=2;
            	    int LA9_0 = input.LA(1);

            	    if ( (LA9_0==17) ) {
            	        alt9=1;
            	    }
            	    else if ( (LA9_0==18) ) {
            	        alt9=2;
            	    }
            	    else {
            	        NoViableAltException nvae =
            	            new NoViableAltException("798:3: (kw= '-' | kw= '/' )", 9, 0, input);

            	        throw nvae;
            	    }
            	    switch (alt9) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:799:2: kw= '-'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,17,FOLLOW_17_in_rulePATH1288); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getPATHAccess().getHyphenMinusKeyword_0_1_0(), null); 
            	                

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:806:2: kw= '/'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,18,FOLLOW_18_in_rulePATH1307); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getPATHAccess().getSolidusKeyword_0_1_1(), null); 
            	                

            	            }
            	            break;

            	    }


            	    }
            	    break;

            	default :
            	    break loop10;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:811:4: (this_ID_3= RULE_ID )+
            int cnt11=0;
            loop11:
            do {
                int alt11=2;
                int LA11_0 = input.LA(1);

                if ( (LA11_0==RULE_ID) ) {
                    alt11=1;
                }


                switch (alt11) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:811:9: this_ID_3= RULE_ID
            	    {
            	    this_ID_3=(Token)input.LT(1);
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rulePATH1326); 

            	    		current.merge(this_ID_3);
            	        
            	     
            	        createLeafNode(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1(), null); 
            	        

            	    }
            	    break;

            	default :
            	    if ( cnt11 >= 1 ) break loop11;
                        EarlyExitException eee =
                            new EarlyExitException(11, input);
                        throw eee;
                }
                cnt11++;
            } while (true);

            kw=(Token)input.LT(1);
            match(input,14,FOLLOW_14_in_rulePATH1346); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getPATHAccess().getFullStopKeyword_2(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:824:1: (this_ID_5= RULE_ID )+
            int cnt12=0;
            loop12:
            do {
                int alt12=2;
                int LA12_0 = input.LA(1);

                if ( (LA12_0==RULE_ID) ) {
                    int LA12_2 = input.LA(2);

                    if ( (LA12_2==EOF||(LA12_2>=RULE_MACRO && LA12_2<=RULE_ID)||LA12_2==10||LA12_2==12) ) {
                        alt12=1;
                    }


                }


                switch (alt12) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:824:6: this_ID_5= RULE_ID
            	    {
            	    this_ID_5=(Token)input.LT(1);
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rulePATH1362); 

            	    		current.merge(this_ID_5);
            	        
            	     
            	        createLeafNode(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3(), null); 
            	        

            	    }
            	    break;

            	default :
            	    if ( cnt12 >= 1 ) break loop12;
                        EarlyExitException eee =
                            new EarlyExitException(12, input);
                        throw eee;
                }
                cnt12++;
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
    // $ANTLR end rulePATH


    // $ANTLR start entryRuleDIRECTION
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:839:1: entryRuleDIRECTION returns [String current=null] : iv_ruleDIRECTION= ruleDIRECTION EOF ;
    public final String entryRuleDIRECTION() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleDIRECTION = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:840:2: (iv_ruleDIRECTION= ruleDIRECTION EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:841:2: iv_ruleDIRECTION= ruleDIRECTION EOF
            {
             currentNode = createCompositeNode(grammarAccess.getDIRECTIONRule(), currentNode); 
            pushFollow(FOLLOW_ruleDIRECTION_in_entryRuleDIRECTION1410);
            iv_ruleDIRECTION=ruleDIRECTION();
            _fsp--;

             current =iv_ruleDIRECTION.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleDIRECTION1421); 

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
    // $ANTLR end entryRuleDIRECTION


    // $ANTLR start ruleDIRECTION
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:848:1: ruleDIRECTION returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )? )+ ;
    public final AntlrDatatypeRuleToken ruleDIRECTION() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:853:6: ( ( (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )? )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:854:1: ( (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )? )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:854:1: ( (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )? )+
            int cnt15=0;
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( ((LA15_0>=19 && LA15_0<=26)) ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:854:2: (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )?
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:854:2: (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' )
            	    int alt13=8;
            	    switch ( input.LA(1) ) {
            	    case 19:
            	        {
            	        alt13=1;
            	        }
            	        break;
            	    case 20:
            	        {
            	        alt13=2;
            	        }
            	        break;
            	    case 21:
            	        {
            	        alt13=3;
            	        }
            	        break;
            	    case 22:
            	        {
            	        alt13=4;
            	        }
            	        break;
            	    case 23:
            	        {
            	        alt13=5;
            	        }
            	        break;
            	    case 24:
            	        {
            	        alt13=6;
            	        }
            	        break;
            	    case 25:
            	        {
            	        alt13=7;
            	        }
            	        break;
            	    case 26:
            	        {
            	        alt13=8;
            	        }
            	        break;
            	    default:
            	        NoViableAltException nvae =
            	            new NoViableAltException("854:2: (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' )", 13, 0, input);

            	        throw nvae;
            	    }

            	    switch (alt13) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:855:2: kw= 'n'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,19,FOLLOW_19_in_ruleDIRECTION1460); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getNKeyword_0_0(), null); 
            	                

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:862:2: kw= 's'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,20,FOLLOW_20_in_ruleDIRECTION1479); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getSKeyword_0_1(), null); 
            	                

            	            }
            	            break;
            	        case 3 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:869:2: kw= 'w'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,21,FOLLOW_21_in_ruleDIRECTION1498); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getWKeyword_0_2(), null); 
            	                

            	            }
            	            break;
            	        case 4 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:876:2: kw= 'e'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,22,FOLLOW_22_in_ruleDIRECTION1517); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getEKeyword_0_3(), null); 
            	                

            	            }
            	            break;
            	        case 5 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:883:2: kw= 'sw'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,23,FOLLOW_23_in_ruleDIRECTION1536); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getSwKeyword_0_4(), null); 
            	                

            	            }
            	            break;
            	        case 6 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:890:2: kw= 'se'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,24,FOLLOW_24_in_ruleDIRECTION1555); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getSeKeyword_0_5(), null); 
            	                

            	            }
            	            break;
            	        case 7 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:897:2: kw= 'ne'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,25,FOLLOW_25_in_ruleDIRECTION1574); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getNeKeyword_0_6(), null); 
            	                

            	            }
            	            break;
            	        case 8 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:904:2: kw= 'nw'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,26,FOLLOW_26_in_ruleDIRECTION1593); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getNwKeyword_0_7(), null); 
            	                

            	            }
            	            break;

            	    }

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:909:2: (kw= ',' )?
            	    int alt14=2;
            	    int LA14_0 = input.LA(1);

            	    if ( (LA14_0==27) ) {
            	        alt14=1;
            	    }
            	    switch (alt14) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:910:2: kw= ','
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,27,FOLLOW_27_in_ruleDIRECTION1608); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getCommaKeyword_1(), null); 
            	                

            	            }
            	            break;

            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt15 >= 1 ) break loop15;
                        EarlyExitException eee =
                            new EarlyExitException(15, input);
                        throw eee;
                }
                cnt15++;
            } while (true);


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
    // $ANTLR end ruleDIRECTION


    // $ANTLR start entryRuleLIST
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:923:1: entryRuleLIST returns [String current=null] : iv_ruleLIST= ruleLIST EOF ;
    public final String entryRuleLIST() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleLIST = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:924:2: (iv_ruleLIST= ruleLIST EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:925:2: iv_ruleLIST= ruleLIST EOF
            {
             currentNode = createCompositeNode(grammarAccess.getLISTRule(), currentNode); 
            pushFollow(FOLLOW_ruleLIST_in_entryRuleLIST1652);
            iv_ruleLIST=ruleLIST();
            _fsp--;

             current =iv_ruleLIST.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleLIST1663); 

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
    // $ANTLR end entryRuleLIST


    // $ANTLR start ruleLIST
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:932:1: ruleLIST returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+ ) ;
    public final AntlrDatatypeRuleToken ruleLIST() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;
        Token kw=null;
        Token this_ID_2=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:937:6: ( (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:938:1: (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:938:1: (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:938:6: this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+
            {
            this_ID_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleLIST1703); 

            		current.merge(this_ID_0);
                
             
                createLeafNode(grammarAccess.getLISTAccess().getIDTerminalRuleCall_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:945:1: (kw= ',' this_ID_2= RULE_ID )+
            int cnt16=0;
            loop16:
            do {
                int alt16=2;
                int LA16_0 = input.LA(1);

                if ( (LA16_0==27) ) {
                    alt16=1;
                }


                switch (alt16) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:946:2: kw= ',' this_ID_2= RULE_ID
            	    {
            	    kw=(Token)input.LT(1);
            	    match(input,27,FOLLOW_27_in_ruleLIST1722); 

            	            current.merge(kw);
            	            createLeafNode(grammarAccess.getLISTAccess().getCommaKeyword_1_0(), null); 
            	        
            	    this_ID_2=(Token)input.LT(1);
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleLIST1737); 

            	    		current.merge(this_ID_2);
            	        
            	     
            	        createLeafNode(grammarAccess.getLISTAccess().getIDTerminalRuleCall_1_1(), null); 
            	        

            	    }
            	    break;

            	default :
            	    if ( cnt16 >= 1 ) break loop16;
                        EarlyExitException eee =
                            new EarlyExitException(16, input);
                        throw eee;
                }
                cnt16++;
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
    // $ANTLR end ruleLIST


    // $ANTLR start entryRulePROGRESSIVE
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:966:1: entryRulePROGRESSIVE returns [String current=null] : iv_rulePROGRESSIVE= rulePROGRESSIVE EOF ;
    public final String entryRulePROGRESSIVE() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_rulePROGRESSIVE = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:967:2: (iv_rulePROGRESSIVE= rulePROGRESSIVE EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:968:2: iv_rulePROGRESSIVE= rulePROGRESSIVE EOF
            {
             currentNode = createCompositeNode(grammarAccess.getPROGRESSIVERule(), currentNode); 
            pushFollow(FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE1785);
            iv_rulePROGRESSIVE=rulePROGRESSIVE();
            _fsp--;

             current =iv_rulePROGRESSIVE.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePROGRESSIVE1796); 

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
    // $ANTLR end entryRulePROGRESSIVE


    // $ANTLR start rulePROGRESSIVE
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:975:1: rulePROGRESSIVE returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+ ) ;
    public final AntlrDatatypeRuleToken rulePROGRESSIVE() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_IINT_0=null;
        Token kw=null;
        Token this_IINT_3=null;
        Token this_IINT_6=null;
        Token this_IINT_8=null;
        Token this_IINT_11=null;
        Token this_IINT_14=null;
        AntlrDatatypeRuleToken this_FLOAT_1 = null;

        AntlrDatatypeRuleToken this_FLOAT_4 = null;

        AntlrDatatypeRuleToken this_FLOAT_9 = null;

        AntlrDatatypeRuleToken this_FLOAT_12 = null;


         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:980:6: ( ( (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:981:1: ( (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:981:1: ( (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:981:2: (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:981:2: (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT )
            int alt17=2;
            int LA17_0 = input.LA(1);

            if ( (LA17_0==RULE_IINT) ) {
                int LA17_1 = input.LA(2);

                if ( (LA17_1==14) ) {
                    alt17=2;
                }
                else if ( ((LA17_1>=27 && LA17_1<=29)) ) {
                    alt17=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("981:2: (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT )", 17, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("981:2: (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT )", 17, 0, input);

                throw nvae;
            }
            switch (alt17) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:981:7: this_IINT_0= RULE_IINT
                    {
                    this_IINT_0=(Token)input.LT(1);
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE1837); 

                    		current.merge(this_IINT_0);
                        
                     
                        createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_0_0(), null); 
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:990:5: this_FLOAT_1= ruleFLOAT
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_0_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleFLOAT_in_rulePROGRESSIVE1870);
                    this_FLOAT_1=ruleFLOAT();
                    _fsp--;


                    		current.merge(this_FLOAT_1);
                        
                     
                            currentNode = currentNode.getParent();
                        

                    }
                    break;

            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1000:2: (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )?
            int alt19=2;
            int LA19_0 = input.LA(1);

            if ( (LA19_0==28) ) {
                alt19=1;
            }
            switch (alt19) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1001:2: kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT )
                    {
                    kw=(Token)input.LT(1);
                    match(input,28,FOLLOW_28_in_rulePROGRESSIVE1890); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_1_0(), null); 
                        
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1006:1: (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT )
                    int alt18=2;
                    int LA18_0 = input.LA(1);

                    if ( (LA18_0==RULE_IINT) ) {
                        int LA18_1 = input.LA(2);

                        if ( (LA18_1==14) ) {
                            alt18=2;
                        }
                        else if ( (LA18_1==27||LA18_1==29) ) {
                            alt18=1;
                        }
                        else {
                            NoViableAltException nvae =
                                new NoViableAltException("1006:1: (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT )", 18, 1, input);

                            throw nvae;
                        }
                    }
                    else {
                        NoViableAltException nvae =
                            new NoViableAltException("1006:1: (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT )", 18, 0, input);

                        throw nvae;
                    }
                    switch (alt18) {
                        case 1 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1006:6: this_IINT_3= RULE_IINT
                            {
                            this_IINT_3=(Token)input.LT(1);
                            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE1906); 

                            		current.merge(this_IINT_3);
                                
                             
                                createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_1_1_0(), null); 
                                

                            }
                            break;
                        case 2 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1015:5: this_FLOAT_4= ruleFLOAT
                            {
                             
                                    currentNode=createCompositeNode(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_1_1_1(), currentNode); 
                                
                            pushFollow(FOLLOW_ruleFLOAT_in_rulePROGRESSIVE1939);
                            this_FLOAT_4=ruleFLOAT();
                            _fsp--;


                            		current.merge(this_FLOAT_4);
                                
                             
                                    currentNode = currentNode.getParent();
                                

                            }
                            break;

                    }


                    }
                    break;

            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1025:4: (kw= ':' this_IINT_6= RULE_IINT )?
            int alt20=2;
            int LA20_0 = input.LA(1);

            if ( (LA20_0==29) ) {
                alt20=1;
            }
            switch (alt20) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1026:2: kw= ':' this_IINT_6= RULE_IINT
                    {
                    kw=(Token)input.LT(1);
                    match(input,29,FOLLOW_29_in_rulePROGRESSIVE1961); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_2_0(), null); 
                        
                    this_IINT_6=(Token)input.LT(1);
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE1976); 

                    		current.merge(this_IINT_6);
                        
                     
                        createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_2_1(), null); 
                        

                    }
                    break;

            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1038:3: (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+
            int cnt25=0;
            loop25:
            do {
                int alt25=2;
                int LA25_0 = input.LA(1);

                if ( (LA25_0==27) ) {
                    alt25=1;
                }


                switch (alt25) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1039:2: kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )?
            	    {
            	    kw=(Token)input.LT(1);
            	    match(input,27,FOLLOW_27_in_rulePROGRESSIVE1997); 

            	            current.merge(kw);
            	            createLeafNode(grammarAccess.getPROGRESSIVEAccess().getCommaKeyword_3_0(), null); 
            	        
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1044:1: (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT )
            	    int alt21=2;
            	    int LA21_0 = input.LA(1);

            	    if ( (LA21_0==RULE_IINT) ) {
            	        int LA21_1 = input.LA(2);

            	        if ( (LA21_1==14) ) {
            	            alt21=2;
            	        }
            	        else if ( (LA21_1==EOF||(LA21_1>=RULE_MACRO && LA21_1<=RULE_ID)||LA21_1==10||LA21_1==12||(LA21_1>=27 && LA21_1<=29)) ) {
            	            alt21=1;
            	        }
            	        else {
            	            NoViableAltException nvae =
            	                new NoViableAltException("1044:1: (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT )", 21, 1, input);

            	            throw nvae;
            	        }
            	    }
            	    else {
            	        NoViableAltException nvae =
            	            new NoViableAltException("1044:1: (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT )", 21, 0, input);

            	        throw nvae;
            	    }
            	    switch (alt21) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1044:6: this_IINT_8= RULE_IINT
            	            {
            	            this_IINT_8=(Token)input.LT(1);
            	            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2013); 

            	            		current.merge(this_IINT_8);
            	                
            	             
            	                createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_1_0(), null); 
            	                

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1053:5: this_FLOAT_9= ruleFLOAT
            	            {
            	             
            	                    currentNode=createCompositeNode(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_1_1(), currentNode); 
            	                
            	            pushFollow(FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2046);
            	            this_FLOAT_9=ruleFLOAT();
            	            _fsp--;


            	            		current.merge(this_FLOAT_9);
            	                
            	             
            	                    currentNode = currentNode.getParent();
            	                

            	            }
            	            break;

            	    }

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1063:2: (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )?
            	    int alt23=2;
            	    int LA23_0 = input.LA(1);

            	    if ( (LA23_0==28) ) {
            	        alt23=1;
            	    }
            	    switch (alt23) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1064:2: kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT )
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,28,FOLLOW_28_in_rulePROGRESSIVE2066); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_3_2_0(), null); 
            	                
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1069:1: (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT )
            	            int alt22=2;
            	            int LA22_0 = input.LA(1);

            	            if ( (LA22_0==RULE_IINT) ) {
            	                int LA22_1 = input.LA(2);

            	                if ( (LA22_1==14) ) {
            	                    alt22=2;
            	                }
            	                else if ( (LA22_1==EOF||(LA22_1>=RULE_MACRO && LA22_1<=RULE_ID)||LA22_1==10||LA22_1==12||LA22_1==27||LA22_1==29) ) {
            	                    alt22=1;
            	                }
            	                else {
            	                    NoViableAltException nvae =
            	                        new NoViableAltException("1069:1: (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT )", 22, 1, input);

            	                    throw nvae;
            	                }
            	            }
            	            else {
            	                NoViableAltException nvae =
            	                    new NoViableAltException("1069:1: (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT )", 22, 0, input);

            	                throw nvae;
            	            }
            	            switch (alt22) {
            	                case 1 :
            	                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1069:6: this_IINT_11= RULE_IINT
            	                    {
            	                    this_IINT_11=(Token)input.LT(1);
            	                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2082); 

            	                    		current.merge(this_IINT_11);
            	                        
            	                     
            	                        createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_2_1_0(), null); 
            	                        

            	                    }
            	                    break;
            	                case 2 :
            	                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1078:5: this_FLOAT_12= ruleFLOAT
            	                    {
            	                     
            	                            currentNode=createCompositeNode(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_2_1_1(), currentNode); 
            	                        
            	                    pushFollow(FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2115);
            	                    this_FLOAT_12=ruleFLOAT();
            	                    _fsp--;


            	                    		current.merge(this_FLOAT_12);
            	                        
            	                     
            	                            currentNode = currentNode.getParent();
            	                        

            	                    }
            	                    break;

            	            }


            	            }
            	            break;

            	    }

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1088:4: (kw= ':' this_IINT_14= RULE_IINT )?
            	    int alt24=2;
            	    int LA24_0 = input.LA(1);

            	    if ( (LA24_0==29) ) {
            	        alt24=1;
            	    }
            	    switch (alt24) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1089:2: kw= ':' this_IINT_14= RULE_IINT
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,29,FOLLOW_29_in_rulePROGRESSIVE2137); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_3_3_0(), null); 
            	                
            	            this_IINT_14=(Token)input.LT(1);
            	            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2152); 

            	            		current.merge(this_IINT_14);
            	                
            	             
            	                createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_3_1(), null); 
            	                

            	            }
            	            break;

            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt25 >= 1 ) break loop25;
                        EarlyExitException eee =
                            new EarlyExitException(25, input);
                        throw eee;
                }
                cnt25++;
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
    // $ANTLR end rulePROGRESSIVE


    protected DFA3 dfa3 = new DFA3(this);
    protected DFA10 dfa10 = new DFA10(this);
    static final String DFA3_eotS =
        "\17\uffff";
    static final String DFA3_eofS =
        "\1\uffff\1\7\2\uffff\1\14\10\uffff\1\16\1\uffff";
    static final String DFA3_minS =
        "\1\5\1\4\2\uffff\1\4\3\uffff\1\5\1\uffff\1\7\2\uffff\1\4\1\uffff";
    static final String DFA3_maxS =
        "\1\32\1\33\2\uffff\1\35\3\uffff\1\22\1\uffff\1\7\2\uffff\1\35\1"+
        "\uffff";
    static final String DFA3_acceptS =
        "\2\uffff\1\2\1\3\1\uffff\1\7\1\10\1\1\1\uffff\1\6\1\uffff\1\11\1"+
        "\5\1\uffff\1\4";
    static final String DFA3_specialS =
        "\17\uffff}>";
    static final String[] DFA3_transitionS = {
            "\1\1\1\2\1\4\7\uffff\2\3\2\uffff\10\5",
            "\1\7\1\10\4\uffff\1\7\1\uffff\1\7\1\uffff\1\11\2\uffff\2\11"+
            "\10\uffff\1\6",
            "",
            "",
            "\2\14\4\uffff\1\14\1\uffff\1\14\1\uffff\1\12\14\uffff\3\13",
            "",
            "",
            "",
            "\1\11\7\uffff\1\7\1\11\2\uffff\2\11",
            "",
            "\1\15",
            "",
            "",
            "\2\16\1\uffff\1\15\2\uffff\1\16\1\uffff\1\16\16\uffff\3\13",
            ""
    };

    static final short[] DFA3_eot = DFA.unpackEncodedString(DFA3_eotS);
    static final short[] DFA3_eof = DFA.unpackEncodedString(DFA3_eofS);
    static final char[] DFA3_min = DFA.unpackEncodedStringToUnsignedChars(DFA3_minS);
    static final char[] DFA3_max = DFA.unpackEncodedStringToUnsignedChars(DFA3_maxS);
    static final short[] DFA3_accept = DFA.unpackEncodedString(DFA3_acceptS);
    static final short[] DFA3_special = DFA.unpackEncodedString(DFA3_specialS);
    static final short[][] DFA3_transition;

    static {
        int numStates = DFA3_transitionS.length;
        DFA3_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA3_transition[i] = DFA.unpackEncodedString(DFA3_transitionS[i]);
        }
    }

    class DFA3 extends DFA {

        public DFA3(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 3;
            this.eot = DFA3_eot;
            this.eof = DFA3_eof;
            this.min = DFA3_min;
            this.max = DFA3_max;
            this.accept = DFA3_accept;
            this.special = DFA3_special;
            this.transition = DFA3_transition;
        }
        public String getDescription() {
            return "449:1: (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE )";
        }
    }
    static final String DFA10_eotS =
        "\4\uffff";
    static final String DFA10_eofS =
        "\4\uffff";
    static final String DFA10_minS =
        "\2\5\2\uffff";
    static final String DFA10_maxS =
        "\1\5\1\22\2\uffff";
    static final String DFA10_acceptS =
        "\2\uffff\1\2\1\1";
    static final String DFA10_specialS =
        "\4\uffff}>";
    static final String[] DFA10_transitionS = {
            "\1\1",
            "\1\1\10\uffff\1\2\2\uffff\2\3",
            "",
            ""
    };

    static final short[] DFA10_eot = DFA.unpackEncodedString(DFA10_eotS);
    static final short[] DFA10_eof = DFA.unpackEncodedString(DFA10_eofS);
    static final char[] DFA10_min = DFA.unpackEncodedStringToUnsignedChars(DFA10_minS);
    static final char[] DFA10_max = DFA.unpackEncodedStringToUnsignedChars(DFA10_maxS);
    static final short[] DFA10_accept = DFA.unpackEncodedString(DFA10_acceptS);
    static final short[] DFA10_special = DFA.unpackEncodedString(DFA10_specialS);
    static final short[][] DFA10_transition;

    static {
        int numStates = DFA10_transitionS.length;
        DFA10_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA10_transition[i] = DFA.unpackEncodedString(DFA10_transitionS[i]);
        }
    }

    class DFA10 extends DFA {

        public DFA10(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 10;
            this.eot = DFA10_eot;
            this.eof = DFA10_eof;
            this.min = DFA10_min;
            this.max = DFA10_max;
            this.accept = DFA10_accept;
            this.special = DFA10_special;
            this.transition = DFA10_transition;
        }
        public String getDescription() {
            return "()* loopback of 791:2: ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )*";
        }
    }
 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot75 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot85 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLRoot131 = new BitSet(new long[]{0x0000000000000412L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_ruleWMLRoot158 = new BitSet(new long[]{0x0000000000000412L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_entryRuleWMLMacro195 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacro205 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_MACRO_in_ruleWMLMacro246 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag286 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag296 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_ruleWMLTag331 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag348 = new BitSet(new long[]{0x0000000000000800L});
    public static final BitSet FOLLOW_11_in_ruleWMLTag363 = new BitSet(new long[]{0x0000000000001430L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLTag385 = new BitSet(new long[]{0x0000000000001430L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLTag412 = new BitSet(new long[]{0x0000000000001430L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_ruleWMLTag439 = new BitSet(new long[]{0x0000000000001430L});
    public static final BitSet FOLLOW_12_in_ruleWMLTag451 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag468 = new BitSet(new long[]{0x0000000000000800L});
    public static final BitSet FOLLOW_11_in_ruleWMLTag483 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey519 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey529 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKey571 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_13_in_ruleWMLKey586 = new BitSet(new long[]{0x0000000007F980F0L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLKey607 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue643 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue653 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKeyValue697 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLKeyValue717 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_ruleWMLKeyValue741 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_ruleWMLKeyValue760 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_ruleWMLKeyValue775 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_ruleWMLKeyValue799 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_ruleWMLKeyValue818 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleLIST_in_ruleWMLKeyValue837 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_ruleWMLKeyValue856 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_ruleWMLKeyValue886 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_entryRuleFLOAT929 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleFLOAT940 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_ruleFLOAT984 = new BitSet(new long[]{0x0000000000004000L});
    public static final BitSet FOLLOW_14_in_ruleFLOAT1002 = new BitSet(new long[]{0x0000000000000080L});
    public static final BitSet FOLLOW_RULE_IINT_in_ruleFLOAT1018 = new BitSet(new long[]{0x0000000000000082L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING1070 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING1081 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_ruleTSTRING1121 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_ruleTSTRING1136 = new BitSet(new long[]{0x0000000000008040L});
    public static final BitSet FOLLOW_15_in_ruleTSTRING1150 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleTSTRING1168 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_entryRulePATH1214 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePATH1225 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rulePATH1267 = new BitSet(new long[]{0x0000000000060020L});
    public static final BitSet FOLLOW_17_in_rulePATH1288 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_18_in_rulePATH1307 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_ID_in_rulePATH1326 = new BitSet(new long[]{0x0000000000004020L});
    public static final BitSet FOLLOW_14_in_rulePATH1346 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_ID_in_rulePATH1362 = new BitSet(new long[]{0x0000000000000022L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_entryRuleDIRECTION1410 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleDIRECTION1421 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_ruleDIRECTION1460 = new BitSet(new long[]{0x000000000FF80002L});
    public static final BitSet FOLLOW_20_in_ruleDIRECTION1479 = new BitSet(new long[]{0x000000000FF80002L});
    public static final BitSet FOLLOW_21_in_ruleDIRECTION1498 = new BitSet(new long[]{0x000000000FF80002L});
    public static final BitSet FOLLOW_22_in_ruleDIRECTION1517 = new BitSet(new long[]{0x000000000FF80002L});
    public static final BitSet FOLLOW_23_in_ruleDIRECTION1536 = new BitSet(new long[]{0x000000000FF80002L});
    public static final BitSet FOLLOW_24_in_ruleDIRECTION1555 = new BitSet(new long[]{0x000000000FF80002L});
    public static final BitSet FOLLOW_25_in_ruleDIRECTION1574 = new BitSet(new long[]{0x000000000FF80002L});
    public static final BitSet FOLLOW_26_in_ruleDIRECTION1593 = new BitSet(new long[]{0x000000000FF80002L});
    public static final BitSet FOLLOW_27_in_ruleDIRECTION1608 = new BitSet(new long[]{0x0000000007F80002L});
    public static final BitSet FOLLOW_ruleLIST_in_entryRuleLIST1652 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleLIST1663 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleLIST1703 = new BitSet(new long[]{0x0000000008000000L});
    public static final BitSet FOLLOW_27_in_ruleLIST1722 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleLIST1737 = new BitSet(new long[]{0x0000000008000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE1785 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePROGRESSIVE1796 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE1837 = new BitSet(new long[]{0x0000000038000000L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rulePROGRESSIVE1870 = new BitSet(new long[]{0x0000000038000000L});
    public static final BitSet FOLLOW_28_in_rulePROGRESSIVE1890 = new BitSet(new long[]{0x0000000000000080L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE1906 = new BitSet(new long[]{0x0000000028000000L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rulePROGRESSIVE1939 = new BitSet(new long[]{0x0000000028000000L});
    public static final BitSet FOLLOW_29_in_rulePROGRESSIVE1961 = new BitSet(new long[]{0x0000000000000080L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE1976 = new BitSet(new long[]{0x0000000008000000L});
    public static final BitSet FOLLOW_27_in_rulePROGRESSIVE1997 = new BitSet(new long[]{0x0000000000000080L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2013 = new BitSet(new long[]{0x0000000038000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2046 = new BitSet(new long[]{0x0000000038000002L});
    public static final BitSet FOLLOW_28_in_rulePROGRESSIVE2066 = new BitSet(new long[]{0x0000000000000080L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2082 = new BitSet(new long[]{0x0000000028000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2115 = new BitSet(new long[]{0x0000000028000002L});
    public static final BitSet FOLLOW_29_in_rulePROGRESSIVE2137 = new BitSet(new long[]{0x0000000000000080L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2152 = new BitSet(new long[]{0x0000000008000002L});

}