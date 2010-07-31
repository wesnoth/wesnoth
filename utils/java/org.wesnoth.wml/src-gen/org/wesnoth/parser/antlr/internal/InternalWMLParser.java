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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_EOL", "RULE_SL_COMMENT", "RULE_MACRO", "RULE_LUA_CODE", "RULE_DEFINE", "RULE_TEXTDOMAIN", "RULE_STRING", "RULE_ANY_OTHER", "RULE_WS", "'['", "'+'", "']'", "'[/'", "'='"
    };
    public static final int RULE_LUA_CODE=8;
    public static final int RULE_ID=4;
    public static final int RULE_STRING=11;
    public static final int RULE_DEFINE=9;
    public static final int RULE_ANY_OTHER=12;
    public static final int RULE_EOL=5;
    public static final int RULE_TEXTDOMAIN=10;
    public static final int RULE_WS=13;
    public static final int EOF=-1;
    public static final int RULE_SL_COMMENT=6;
    public static final int RULE_MACRO=7;

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:86:1: ruleWMLRoot returns [EObject current=null] : ( ( (lv_Tags_0_0= ruleWMLTag ) ) | ( (lv_MacroCalls_1_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_2_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_3_0= ruleWMLTextdomain ) ) )* ;
    public final EObject ruleWMLRoot() throws RecognitionException {
        EObject current = null;

        EObject lv_Tags_0_0 = null;

        EObject lv_MacroCalls_1_0 = null;

        EObject lv_MacroDefines_2_0 = null;

        EObject lv_Textdomains_3_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:91:6: ( ( ( (lv_Tags_0_0= ruleWMLTag ) ) | ( (lv_MacroCalls_1_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_2_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_3_0= ruleWMLTextdomain ) ) )* )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( ( (lv_Tags_0_0= ruleWMLTag ) ) | ( (lv_MacroCalls_1_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_2_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_3_0= ruleWMLTextdomain ) ) )*
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( ( (lv_Tags_0_0= ruleWMLTag ) ) | ( (lv_MacroCalls_1_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_2_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_3_0= ruleWMLTextdomain ) ) )*
            loop1:
            do {
                int alt1=5;
                switch ( input.LA(1) ) {
                case 14:
                    {
                    alt1=1;
                    }
                    break;
                case RULE_MACRO:
                    {
                    alt1=2;
                    }
                    break;
                case RULE_DEFINE:
                    {
                    alt1=3;
                    }
                    break;
                case RULE_TEXTDOMAIN:
                    {
                    alt1=4;
                    }
                    break;

                }

                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:2: ( (lv_Tags_0_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:2: ( (lv_Tags_0_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:93:1: (lv_Tags_0_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:93:1: (lv_Tags_0_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:94:3: lv_Tags_0_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getTagsWMLTagParserRuleCall_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLRoot131);
            	    lv_Tags_0_0=ruleWMLTag();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Tags",
            	    	        		lv_Tags_0_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:117:6: ( (lv_MacroCalls_1_0= ruleWMLMacroCall ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:117:6: ( (lv_MacroCalls_1_0= ruleWMLMacroCall ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:118:1: (lv_MacroCalls_1_0= ruleWMLMacroCall )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:118:1: (lv_MacroCalls_1_0= ruleWMLMacroCall )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:119:3: lv_MacroCalls_1_0= ruleWMLMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getMacroCallsWMLMacroCallParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLRoot158);
            	    lv_MacroCalls_1_0=ruleWMLMacroCall();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"MacroCalls",
            	    	        		lv_MacroCalls_1_0, 
            	    	        		"WMLMacroCall", 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:142:6: ( (lv_MacroDefines_2_0= ruleWMLMacroDefine ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:142:6: ( (lv_MacroDefines_2_0= ruleWMLMacroDefine ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:143:1: (lv_MacroDefines_2_0= ruleWMLMacroDefine )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:143:1: (lv_MacroDefines_2_0= ruleWMLMacroDefine )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:144:3: lv_MacroDefines_2_0= ruleWMLMacroDefine
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getMacroDefinesWMLMacroDefineParserRuleCall_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroDefine_in_ruleWMLRoot185);
            	    lv_MacroDefines_2_0=ruleWMLMacroDefine();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"MacroDefines",
            	    	        		lv_MacroDefines_2_0, 
            	    	        		"WMLMacroDefine", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }


            	    }
            	    break;
            	case 4 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:167:6: ( (lv_Textdomains_3_0= ruleWMLTextdomain ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:167:6: ( (lv_Textdomains_3_0= ruleWMLTextdomain ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:168:1: (lv_Textdomains_3_0= ruleWMLTextdomain )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:168:1: (lv_Textdomains_3_0= ruleWMLTextdomain )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:169:3: lv_Textdomains_3_0= ruleWMLTextdomain
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getTextdomainsWMLTextdomainParserRuleCall_3_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTextdomain_in_ruleWMLRoot212);
            	    lv_Textdomains_3_0=ruleWMLTextdomain();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Textdomains",
            	    	        		lv_Textdomains_3_0, 
            	    	        		"WMLTextdomain", 
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


    // $ANTLR start entryRuleWMLTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:199:1: entryRuleWMLTag returns [EObject current=null] : iv_ruleWMLTag= ruleWMLTag EOF ;
    public final EObject entryRuleWMLTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:200:2: (iv_ruleWMLTag= ruleWMLTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:201:2: iv_ruleWMLTag= ruleWMLTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLTag_in_entryRuleWMLTag249);
            iv_ruleWMLTag=ruleWMLTag();
            _fsp--;

             current =iv_ruleWMLTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTag259); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:208:1: ruleWMLTag returns [EObject current=null] : ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_Tags_4_0= ruleWMLTag ) ) | ( (lv_Keys_5_0= ruleWMLKey ) ) | ( (lv_MacroCalls_6_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_7_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_8_0= ruleWMLTextdomain ) ) )* '[/' ( (lv_endName_10_0= RULE_ID ) ) ']' ) ;
    public final EObject ruleWMLTag() throws RecognitionException {
        EObject current = null;

        Token lv_plus_1_0=null;
        Token lv_name_2_0=null;
        Token lv_endName_10_0=null;
        EObject lv_Tags_4_0 = null;

        EObject lv_Keys_5_0 = null;

        EObject lv_MacroCalls_6_0 = null;

        EObject lv_MacroDefines_7_0 = null;

        EObject lv_Textdomains_8_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:213:6: ( ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_Tags_4_0= ruleWMLTag ) ) | ( (lv_Keys_5_0= ruleWMLKey ) ) | ( (lv_MacroCalls_6_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_7_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_8_0= ruleWMLTextdomain ) ) )* '[/' ( (lv_endName_10_0= RULE_ID ) ) ']' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:214:1: ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_Tags_4_0= ruleWMLTag ) ) | ( (lv_Keys_5_0= ruleWMLKey ) ) | ( (lv_MacroCalls_6_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_7_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_8_0= ruleWMLTextdomain ) ) )* '[/' ( (lv_endName_10_0= RULE_ID ) ) ']' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:214:1: ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_Tags_4_0= ruleWMLTag ) ) | ( (lv_Keys_5_0= ruleWMLKey ) ) | ( (lv_MacroCalls_6_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_7_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_8_0= ruleWMLTextdomain ) ) )* '[/' ( (lv_endName_10_0= RULE_ID ) ) ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:214:3: '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_Tags_4_0= ruleWMLTag ) ) | ( (lv_Keys_5_0= ruleWMLKey ) ) | ( (lv_MacroCalls_6_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_7_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_8_0= ruleWMLTextdomain ) ) )* '[/' ( (lv_endName_10_0= RULE_ID ) ) ']'
            {
            match(input,14,FOLLOW_14_in_ruleWMLTag294); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:218:1: ( (lv_plus_1_0= '+' ) )?
            int alt2=2;
            int LA2_0 = input.LA(1);

            if ( (LA2_0==15) ) {
                alt2=1;
            }
            switch (alt2) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:219:1: (lv_plus_1_0= '+' )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:219:1: (lv_plus_1_0= '+' )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:220:3: lv_plus_1_0= '+'
                    {
                    lv_plus_1_0=(Token)input.LT(1);
                    match(input,15,FOLLOW_15_in_ruleWMLTag312); 

                            createLeafNode(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0(), "plus"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "plus", true, "+", lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }


                    }
                    break;

            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:239:3: ( (lv_name_2_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:240:1: (lv_name_2_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:240:1: (lv_name_2_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:241:3: lv_name_2_0= RULE_ID
            {
            lv_name_2_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag343); 

            			createLeafNode(grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_2_0(), "name"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"name",
            	        		lv_name_2_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            match(input,16,FOLLOW_16_in_ruleWMLTag358); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:267:1: ( ( (lv_Tags_4_0= ruleWMLTag ) ) | ( (lv_Keys_5_0= ruleWMLKey ) ) | ( (lv_MacroCalls_6_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_7_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_8_0= ruleWMLTextdomain ) ) )*
            loop3:
            do {
                int alt3=6;
                switch ( input.LA(1) ) {
                case 14:
                    {
                    alt3=1;
                    }
                    break;
                case RULE_ID:
                    {
                    alt3=2;
                    }
                    break;
                case RULE_MACRO:
                    {
                    alt3=3;
                    }
                    break;
                case RULE_DEFINE:
                    {
                    alt3=4;
                    }
                    break;
                case RULE_TEXTDOMAIN:
                    {
                    alt3=5;
                    }
                    break;

                }

                switch (alt3) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:267:2: ( (lv_Tags_4_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:267:2: ( (lv_Tags_4_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:268:1: (lv_Tags_4_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:268:1: (lv_Tags_4_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:269:3: lv_Tags_4_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTagsWMLTagParserRuleCall_4_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLTag380);
            	    lv_Tags_4_0=ruleWMLTag();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Tags",
            	    	        		lv_Tags_4_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:292:6: ( (lv_Keys_5_0= ruleWMLKey ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:292:6: ( (lv_Keys_5_0= ruleWMLKey ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:293:1: (lv_Keys_5_0= ruleWMLKey )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:293:1: (lv_Keys_5_0= ruleWMLKey )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:294:3: lv_Keys_5_0= ruleWMLKey
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getKeysWMLKeyParserRuleCall_4_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLTag407);
            	    lv_Keys_5_0=ruleWMLKey();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Keys",
            	    	        		lv_Keys_5_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:317:6: ( (lv_MacroCalls_6_0= ruleWMLMacroCall ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:317:6: ( (lv_MacroCalls_6_0= ruleWMLMacroCall ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:318:1: (lv_MacroCalls_6_0= ruleWMLMacroCall )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:318:1: (lv_MacroCalls_6_0= ruleWMLMacroCall )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:319:3: lv_MacroCalls_6_0= ruleWMLMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getMacroCallsWMLMacroCallParserRuleCall_4_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLTag434);
            	    lv_MacroCalls_6_0=ruleWMLMacroCall();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"MacroCalls",
            	    	        		lv_MacroCalls_6_0, 
            	    	        		"WMLMacroCall", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }


            	    }
            	    break;
            	case 4 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:342:6: ( (lv_MacroDefines_7_0= ruleWMLMacroDefine ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:342:6: ( (lv_MacroDefines_7_0= ruleWMLMacroDefine ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:343:1: (lv_MacroDefines_7_0= ruleWMLMacroDefine )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:343:1: (lv_MacroDefines_7_0= ruleWMLMacroDefine )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:344:3: lv_MacroDefines_7_0= ruleWMLMacroDefine
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getMacroDefinesWMLMacroDefineParserRuleCall_4_3_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroDefine_in_ruleWMLTag461);
            	    lv_MacroDefines_7_0=ruleWMLMacroDefine();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"MacroDefines",
            	    	        		lv_MacroDefines_7_0, 
            	    	        		"WMLMacroDefine", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }


            	    }
            	    break;
            	case 5 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:367:6: ( (lv_Textdomains_8_0= ruleWMLTextdomain ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:367:6: ( (lv_Textdomains_8_0= ruleWMLTextdomain ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:368:1: (lv_Textdomains_8_0= ruleWMLTextdomain )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:368:1: (lv_Textdomains_8_0= ruleWMLTextdomain )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:369:3: lv_Textdomains_8_0= ruleWMLTextdomain
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTextdomainsWMLTextdomainParserRuleCall_4_4_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTextdomain_in_ruleWMLTag488);
            	    lv_Textdomains_8_0=ruleWMLTextdomain();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Textdomains",
            	    	        		lv_Textdomains_8_0, 
            	    	        		"WMLTextdomain", 
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
            	    break loop3;
                }
            } while (true);

            match(input,17,FOLLOW_17_in_ruleWMLTag500); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:395:1: ( (lv_endName_10_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:396:1: (lv_endName_10_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:396:1: (lv_endName_10_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:397:3: lv_endName_10_0= RULE_ID
            {
            lv_endName_10_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag517); 

            			createLeafNode(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0(), "endName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"endName",
            	        		lv_endName_10_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            match(input,16,FOLLOW_16_in_ruleWMLTag532); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_7(), null); 
                

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:431:1: entryRuleWMLKey returns [EObject current=null] : iv_ruleWMLKey= ruleWMLKey EOF ;
    public final EObject entryRuleWMLKey() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKey = null;


         
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
        	
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:435:2: (iv_ruleWMLKey= ruleWMLKey EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:436:2: iv_ruleWMLKey= ruleWMLKey EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey574);
            iv_ruleWMLKey=ruleWMLKey();
            _fsp--;

             current =iv_ruleWMLKey; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey584); 

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
    // $ANTLR end entryRuleWMLKey


    // $ANTLR start ruleWMLKey
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:446:1: ruleWMLKey returns [EObject current=null] : ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )+ ( RULE_EOL | RULE_SL_COMMENT ) ) ;
    public final EObject ruleWMLKey() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;
        EObject lv_value_2_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:452:6: ( ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )+ ( RULE_EOL | RULE_SL_COMMENT ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:453:1: ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )+ ( RULE_EOL | RULE_SL_COMMENT ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:453:1: ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )+ ( RULE_EOL | RULE_SL_COMMENT ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:453:2: ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )+ ( RULE_EOL | RULE_SL_COMMENT )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:453:2: ( (lv_name_0_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:454:1: (lv_name_0_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:454:1: (lv_name_0_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:455:3: lv_name_0_0= RULE_ID
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLKey630); 

            			createLeafNode(grammarAccess.getWMLKeyAccess().getNameIDTerminalRuleCall_0_0(), "name"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"name",
            	        		lv_name_0_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            match(input,18,FOLLOW_18_in_ruleWMLKey645); 

                    createLeafNode(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:481:1: ( (lv_value_2_0= ruleWMLKeyValue ) )+
            int cnt4=0;
            loop4:
            do {
                int alt4=2;
                int LA4_0 = input.LA(1);

                if ( (LA4_0==RULE_ID||(LA4_0>=RULE_MACRO && LA4_0<=RULE_LUA_CODE)||(LA4_0>=RULE_STRING && LA4_0<=RULE_ANY_OTHER)||(LA4_0>=14 && LA4_0<=15)) ) {
                    alt4=1;
                }


                switch (alt4) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:482:1: (lv_value_2_0= ruleWMLKeyValue )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:482:1: (lv_value_2_0= ruleWMLKeyValue )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:483:3: lv_value_2_0= ruleWMLKeyValue
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKeyValue_in_ruleWMLKey666);
            	    lv_value_2_0=ruleWMLKeyValue();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
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
            	    break;

            	default :
            	    if ( cnt4 >= 1 ) break loop4;
                        EarlyExitException eee =
                            new EarlyExitException(4, input);
                        throw eee;
                }
                cnt4++;
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:505:3: ( RULE_EOL | RULE_SL_COMMENT )
            int alt5=2;
            int LA5_0 = input.LA(1);

            if ( (LA5_0==RULE_EOL) ) {
                alt5=1;
            }
            else if ( (LA5_0==RULE_SL_COMMENT) ) {
                alt5=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("505:3: ( RULE_EOL | RULE_SL_COMMENT )", 5, 0, input);

                throw nvae;
            }
            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:505:4: RULE_EOL
                    {
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_ruleWMLKey677); 
                     
                        createLeafNode(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_0(), null); 
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:510:6: RULE_SL_COMMENT
                    {
                    match(input,RULE_SL_COMMENT,FOLLOW_RULE_SL_COMMENT_in_ruleWMLKey691); 
                     
                        createLeafNode(grammarAccess.getWMLKeyAccess().getSL_COMMENTTerminalRuleCall_3_1(), null); 
                        

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

            	myHiddenTokenState.restore();

        }
        return current;
    }
    // $ANTLR end ruleWMLKey


    // $ANTLR start entryRuleWMLKeyValue
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:525:1: entryRuleWMLKeyValue returns [EObject current=null] : iv_ruleWMLKeyValue= ruleWMLKeyValue EOF ;
    public final EObject entryRuleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKeyValue = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:526:2: (iv_ruleWMLKeyValue= ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:527:2: iv_ruleWMLKeyValue= ruleWMLKeyValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue731);
            iv_ruleWMLKeyValue=ruleWMLKeyValue();
            _fsp--;

             current =iv_ruleWMLKeyValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue741); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:534:1: ruleWMLKeyValue returns [EObject current=null] : (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall ) ;
    public final EObject ruleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject this_WMLValue_0 = null;

        EObject this_WMLMacroCall_1 = null;

        EObject this_WMLLuaCode_2 = null;

        EObject this_WMLArrayCall_3 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:539:6: ( (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:540:1: (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:540:1: (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall )
            int alt6=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
            case RULE_STRING:
            case RULE_ANY_OTHER:
            case 15:
                {
                alt6=1;
                }
                break;
            case RULE_MACRO:
                {
                alt6=2;
                }
                break;
            case RULE_LUA_CODE:
                {
                alt6=3;
                }
                break;
            case 14:
                {
                alt6=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("540:1: (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall )", 6, 0, input);

                throw nvae;
            }

            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:541:5: this_WMLValue_0= ruleWMLValue
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLKeyValue788);
                    this_WMLValue_0=ruleWMLValue();
                    _fsp--;

                     
                            current = this_WMLValue_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:551:5: this_WMLMacroCall_1= ruleWMLMacroCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyValue815);
                    this_WMLMacroCall_1=ruleWMLMacroCall();
                    _fsp--;

                     
                            current = this_WMLMacroCall_1; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:561:5: this_WMLLuaCode_2= ruleWMLLuaCode
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLLuaCode_in_ruleWMLKeyValue842);
                    this_WMLLuaCode_2=ruleWMLLuaCode();
                    _fsp--;

                     
                            current = this_WMLLuaCode_2; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:571:5: this_WMLArrayCall_3= ruleWMLArrayCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLArrayCallParserRuleCall_3(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLArrayCall_in_ruleWMLKeyValue869);
                    this_WMLArrayCall_3=ruleWMLArrayCall();
                    _fsp--;

                     
                            current = this_WMLArrayCall_3; 
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
    // $ANTLR end ruleWMLKeyValue


    // $ANTLR start entryRuleWMLMacroCall
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:587:1: entryRuleWMLMacroCall returns [EObject current=null] : iv_ruleWMLMacroCall= ruleWMLMacroCall EOF ;
    public final EObject entryRuleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroCall = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:588:2: (iv_ruleWMLMacroCall= ruleWMLMacroCall EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:589:2: iv_ruleWMLMacroCall= ruleWMLMacroCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall904);
            iv_ruleWMLMacroCall=ruleWMLMacroCall();
            _fsp--;

             current =iv_ruleWMLMacroCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCall914); 

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
    // $ANTLR end entryRuleWMLMacroCall


    // $ANTLR start ruleWMLMacroCall
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:596:1: ruleWMLMacroCall returns [EObject current=null] : ( (lv_name_0_0= RULE_MACRO ) ) ;
    public final EObject ruleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:601:6: ( ( (lv_name_0_0= RULE_MACRO ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:602:1: ( (lv_name_0_0= RULE_MACRO ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:602:1: ( (lv_name_0_0= RULE_MACRO ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:603:1: (lv_name_0_0= RULE_MACRO )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:603:1: (lv_name_0_0= RULE_MACRO )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:604:3: lv_name_0_0= RULE_MACRO
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_MACRO,FOLLOW_RULE_MACRO_in_ruleWMLMacroCall955); 

            			createLeafNode(grammarAccess.getWMLMacroCallAccess().getNameMACROTerminalRuleCall_0(), "name"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
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
    // $ANTLR end ruleWMLMacroCall


    // $ANTLR start entryRuleWMLLuaCode
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:634:1: entryRuleWMLLuaCode returns [EObject current=null] : iv_ruleWMLLuaCode= ruleWMLLuaCode EOF ;
    public final EObject entryRuleWMLLuaCode() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLLuaCode = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:635:2: (iv_ruleWMLLuaCode= ruleWMLLuaCode EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:636:2: iv_ruleWMLLuaCode= ruleWMLLuaCode EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLLuaCodeRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode995);
            iv_ruleWMLLuaCode=ruleWMLLuaCode();
            _fsp--;

             current =iv_ruleWMLLuaCode; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLLuaCode1005); 

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
    // $ANTLR end entryRuleWMLLuaCode


    // $ANTLR start ruleWMLLuaCode
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:643:1: ruleWMLLuaCode returns [EObject current=null] : ( (lv_value_0_0= RULE_LUA_CODE ) ) ;
    public final EObject ruleWMLLuaCode() throws RecognitionException {
        EObject current = null;

        Token lv_value_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:648:6: ( ( (lv_value_0_0= RULE_LUA_CODE ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:649:1: ( (lv_value_0_0= RULE_LUA_CODE ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:649:1: ( (lv_value_0_0= RULE_LUA_CODE ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:650:1: (lv_value_0_0= RULE_LUA_CODE )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:650:1: (lv_value_0_0= RULE_LUA_CODE )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:651:3: lv_value_0_0= RULE_LUA_CODE
            {
            lv_value_0_0=(Token)input.LT(1);
            match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_ruleWMLLuaCode1046); 

            			createLeafNode(grammarAccess.getWMLLuaCodeAccess().getValueLUA_CODETerminalRuleCall_0(), "value"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLLuaCodeRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"value",
            	        		lv_value_0_0, 
            	        		"LUA_CODE", 
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
    // $ANTLR end ruleWMLLuaCode


    // $ANTLR start entryRuleWMLArrayCall
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:681:1: entryRuleWMLArrayCall returns [EObject current=null] : iv_ruleWMLArrayCall= ruleWMLArrayCall EOF ;
    public final EObject entryRuleWMLArrayCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLArrayCall = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:682:2: (iv_ruleWMLArrayCall= ruleWMLArrayCall EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:683:2: iv_ruleWMLArrayCall= ruleWMLArrayCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLArrayCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall1086);
            iv_ruleWMLArrayCall=ruleWMLArrayCall();
            _fsp--;

             current =iv_ruleWMLArrayCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLArrayCall1096); 

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
    // $ANTLR end entryRuleWMLArrayCall


    // $ANTLR start ruleWMLArrayCall
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:690:1: ruleWMLArrayCall returns [EObject current=null] : ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' ) ;
    public final EObject ruleWMLArrayCall() throws RecognitionException {
        EObject current = null;

        EObject lv_value_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:695:6: ( ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:696:1: ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:696:1: ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:696:3: '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']'
            {
            match(input,14,FOLLOW_14_in_ruleWMLArrayCall1131); 

                    createLeafNode(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:700:1: ( (lv_value_1_0= ruleWMLValue ) )+
            int cnt7=0;
            loop7:
            do {
                int alt7=2;
                int LA7_0 = input.LA(1);

                if ( (LA7_0==RULE_ID||(LA7_0>=RULE_STRING && LA7_0<=RULE_ANY_OTHER)||LA7_0==15) ) {
                    alt7=1;
                }


                switch (alt7) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:701:1: (lv_value_1_0= ruleWMLValue )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:701:1: (lv_value_1_0= ruleWMLValue )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:702:3: lv_value_1_0= ruleWMLValue
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLArrayCall1152);
            	    lv_value_1_0=ruleWMLValue();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLArrayCallRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"value",
            	    	        		lv_value_1_0, 
            	    	        		"WMLValue", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt7 >= 1 ) break loop7;
                        EarlyExitException eee =
                            new EarlyExitException(7, input);
                        throw eee;
                }
                cnt7++;
            } while (true);

            match(input,16,FOLLOW_16_in_ruleWMLArrayCall1163); 

                    createLeafNode(grammarAccess.getWMLArrayCallAccess().getRightSquareBracketKeyword_2(), null); 
                

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
    // $ANTLR end ruleWMLArrayCall


    // $ANTLR start entryRuleWMLMacroDefine
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:736:1: entryRuleWMLMacroDefine returns [EObject current=null] : iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF ;
    public final EObject entryRuleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroDefine = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:737:2: (iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:738:2: iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroDefineRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1199);
            iv_ruleWMLMacroDefine=ruleWMLMacroDefine();
            _fsp--;

             current =iv_ruleWMLMacroDefine; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroDefine1209); 

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
    // $ANTLR end entryRuleWMLMacroDefine


    // $ANTLR start ruleWMLMacroDefine
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:745:1: ruleWMLMacroDefine returns [EObject current=null] : ( (lv_name_0_0= RULE_DEFINE ) ) ;
    public final EObject ruleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:750:6: ( ( (lv_name_0_0= RULE_DEFINE ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:751:1: ( (lv_name_0_0= RULE_DEFINE ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:751:1: ( (lv_name_0_0= RULE_DEFINE ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:752:1: (lv_name_0_0= RULE_DEFINE )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:752:1: (lv_name_0_0= RULE_DEFINE )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:753:3: lv_name_0_0= RULE_DEFINE
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1250); 

            			createLeafNode(grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0(), "name"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"name",
            	        		lv_name_0_0, 
            	        		"DEFINE", 
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
    // $ANTLR end ruleWMLMacroDefine


    // $ANTLR start entryRuleWMLTextdomain
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:783:1: entryRuleWMLTextdomain returns [EObject current=null] : iv_ruleWMLTextdomain= ruleWMLTextdomain EOF ;
    public final EObject entryRuleWMLTextdomain() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTextdomain = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:784:2: (iv_ruleWMLTextdomain= ruleWMLTextdomain EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:785:2: iv_ruleWMLTextdomain= ruleWMLTextdomain EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLTextdomainRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain1290);
            iv_ruleWMLTextdomain=ruleWMLTextdomain();
            _fsp--;

             current =iv_ruleWMLTextdomain; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTextdomain1300); 

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
    // $ANTLR end entryRuleWMLTextdomain


    // $ANTLR start ruleWMLTextdomain
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:792:1: ruleWMLTextdomain returns [EObject current=null] : ( (lv_name_0_0= RULE_TEXTDOMAIN ) ) ;
    public final EObject ruleWMLTextdomain() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:797:6: ( ( (lv_name_0_0= RULE_TEXTDOMAIN ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:798:1: ( (lv_name_0_0= RULE_TEXTDOMAIN ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:798:1: ( (lv_name_0_0= RULE_TEXTDOMAIN ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:799:1: (lv_name_0_0= RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:799:1: (lv_name_0_0= RULE_TEXTDOMAIN )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:800:3: lv_name_0_0= RULE_TEXTDOMAIN
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_ruleWMLTextdomain1341); 

            			createLeafNode(grammarAccess.getWMLTextdomainAccess().getNameTEXTDOMAINTerminalRuleCall_0(), "name"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLTextdomainRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"name",
            	        		lv_name_0_0, 
            	        		"TEXTDOMAIN", 
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
    // $ANTLR end ruleWMLTextdomain


    // $ANTLR start entryRuleWMLValue
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:830:1: entryRuleWMLValue returns [EObject current=null] : iv_ruleWMLValue= ruleWMLValue EOF ;
    public final EObject entryRuleWMLValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLValue = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:831:2: (iv_ruleWMLValue= ruleWMLValue EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:832:2: iv_ruleWMLValue= ruleWMLValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLValue_in_entryRuleWMLValue1381);
            iv_ruleWMLValue=ruleWMLValue();
            _fsp--;

             current =iv_ruleWMLValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValue1391); 

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
    // $ANTLR end entryRuleWMLValue


    // $ANTLR start ruleWMLValue
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:839:1: ruleWMLValue returns [EObject current=null] : ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= RULE_ANY_OTHER | lv_value_0_4= '+' ) ) ) ;
    public final EObject ruleWMLValue() throws RecognitionException {
        EObject current = null;

        Token lv_value_0_1=null;
        Token lv_value_0_2=null;
        Token lv_value_0_3=null;
        Token lv_value_0_4=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:844:6: ( ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= RULE_ANY_OTHER | lv_value_0_4= '+' ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:845:1: ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= RULE_ANY_OTHER | lv_value_0_4= '+' ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:845:1: ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= RULE_ANY_OTHER | lv_value_0_4= '+' ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:846:1: ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= RULE_ANY_OTHER | lv_value_0_4= '+' ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:846:1: ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= RULE_ANY_OTHER | lv_value_0_4= '+' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:847:1: (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= RULE_ANY_OTHER | lv_value_0_4= '+' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:847:1: (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= RULE_ANY_OTHER | lv_value_0_4= '+' )
            int alt8=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt8=1;
                }
                break;
            case RULE_STRING:
                {
                alt8=2;
                }
                break;
            case RULE_ANY_OTHER:
                {
                alt8=3;
                }
                break;
            case 15:
                {
                alt8=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("847:1: (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= RULE_ANY_OTHER | lv_value_0_4= '+' )", 8, 0, input);

                throw nvae;
            }

            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:848:3: lv_value_0_1= RULE_ID
                    {
                    lv_value_0_1=(Token)input.LT(1);
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLValue1434); 

                    			createLeafNode(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0(), "value"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"value",
                    	        		lv_value_0_1, 
                    	        		"ID", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:869:8: lv_value_0_2= RULE_STRING
                    {
                    lv_value_0_2=(Token)input.LT(1);
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLValue1454); 

                    			createLeafNode(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1(), "value"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"value",
                    	        		lv_value_0_2, 
                    	        		"STRING", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:890:8: lv_value_0_3= RULE_ANY_OTHER
                    {
                    lv_value_0_3=(Token)input.LT(1);
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_ruleWMLValue1474); 

                    			createLeafNode(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_2(), "value"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"value",
                    	        		lv_value_0_3, 
                    	        		"ANY_OTHER", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:911:8: lv_value_0_4= '+'
                    {
                    lv_value_0_4=(Token)input.LT(1);
                    match(input,15,FOLLOW_15_in_ruleWMLValue1495); 

                            createLeafNode(grammarAccess.getWMLValueAccess().getValuePlusSignKeyword_0_3(), "value"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "value", lv_value_0_4, null, lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;

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
    // $ANTLR end ruleWMLValue


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot75 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot85 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLRoot131 = new BitSet(new long[]{0x0000000000004682L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLRoot158 = new BitSet(new long[]{0x0000000000004682L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLRoot185 = new BitSet(new long[]{0x0000000000004682L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_ruleWMLRoot212 = new BitSet(new long[]{0x0000000000004682L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag249 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag259 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_ruleWMLTag294 = new BitSet(new long[]{0x0000000000008010L});
    public static final BitSet FOLLOW_15_in_ruleWMLTag312 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag343 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_ruleWMLTag358 = new BitSet(new long[]{0x0000000000024690L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLTag380 = new BitSet(new long[]{0x0000000000024690L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLTag407 = new BitSet(new long[]{0x0000000000024690L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLTag434 = new BitSet(new long[]{0x0000000000024690L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLTag461 = new BitSet(new long[]{0x0000000000024690L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_ruleWMLTag488 = new BitSet(new long[]{0x0000000000024690L});
    public static final BitSet FOLLOW_17_in_ruleWMLTag500 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag517 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_ruleWMLTag532 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey574 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey584 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKey630 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_ruleWMLKey645 = new BitSet(new long[]{0x000000000000D990L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLKey666 = new BitSet(new long[]{0x000000000000D9F0L});
    public static final BitSet FOLLOW_RULE_EOL_in_ruleWMLKey677 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_SL_COMMENT_in_ruleWMLKey691 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue731 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue741 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLKeyValue788 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyValue815 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_ruleWMLKeyValue842 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_ruleWMLKeyValue869 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall904 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCall914 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_MACRO_in_ruleWMLMacroCall955 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode995 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLLuaCode1005 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_ruleWMLLuaCode1046 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall1086 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLArrayCall1096 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_ruleWMLArrayCall1131 = new BitSet(new long[]{0x0000000000009810L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLArrayCall1152 = new BitSet(new long[]{0x0000000000019810L});
    public static final BitSet FOLLOW_16_in_ruleWMLArrayCall1163 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1199 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine1209 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1250 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain1290 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTextdomain1300 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_ruleWMLTextdomain1341 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_entryRuleWMLValue1381 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValue1391 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLValue1434 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLValue1454 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_ruleWMLValue1474 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_ruleWMLValue1495 = new BitSet(new long[]{0x0000000000000002L});

}