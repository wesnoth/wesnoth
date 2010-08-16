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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_EOL", "RULE_SL_COMMENT", "RULE_LUA_CODE", "RULE_DEFINE", "RULE_ENDDEF", "RULE_TEXTDOMAIN", "RULE_STRING", "RULE_ANY_OTHER", "RULE_WS", "'['", "'+'", "']'", "'[/'", "'='", "'{'", "'./'", "'~'", "'}'", "'.'", "'('", "')'", "'_'", "'_$'"
    };
    public static final int RULE_LUA_CODE=7;
    public static final int RULE_ID=4;
    public static final int RULE_STRING=11;
    public static final int RULE_DEFINE=8;
    public static final int RULE_ANY_OTHER=12;
    public static final int RULE_ENDDEF=9;
    public static final int RULE_EOL=5;
    public static final int RULE_TEXTDOMAIN=10;
    public static final int RULE_WS=13;
    public static final int EOF=-1;
    public static final int RULE_SL_COMMENT=6;

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
                case 19:
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
                case 19:
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:446:1: ruleWMLKey returns [EObject current=null] : ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )* ( ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) ) )* ( RULE_EOL | RULE_SL_COMMENT ) ) ;
    public final EObject ruleWMLKey() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;
        EObject lv_value_2_0 = null;

        EObject lv_value_6_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:452:6: ( ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )* ( ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) ) )* ( RULE_EOL | RULE_SL_COMMENT ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:453:1: ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )* ( ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) ) )* ( RULE_EOL | RULE_SL_COMMENT ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:453:1: ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )* ( ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) ) )* ( RULE_EOL | RULE_SL_COMMENT ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:453:2: ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )* ( ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) ) )* ( RULE_EOL | RULE_SL_COMMENT )
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
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:481:1: ( (lv_value_2_0= ruleWMLKeyValue ) )*
            loop4:
            do {
                int alt4=2;
                int LA4_0 = input.LA(1);

                if ( (LA4_0==RULE_ID||LA4_0==RULE_LUA_CODE||(LA4_0>=RULE_STRING && LA4_0<=RULE_ANY_OTHER)||LA4_0==14||LA4_0==19||LA4_0==21||(LA4_0>=23 && LA4_0<=27)) ) {
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
            	    break loop4;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:505:3: ( ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) ) )*
            loop7:
            do {
                int alt7=2;
                int LA7_0 = input.LA(1);

                if ( (LA7_0==RULE_EOL) ) {
                    int LA7_1 = input.LA(2);

                    if ( (LA7_1==15) ) {
                        alt7=1;
                    }


                }
                else if ( (LA7_0==15) ) {
                    alt7=1;
                }


                switch (alt7) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:505:4: ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:505:4: ( RULE_EOL )?
            	    int alt5=2;
            	    int LA5_0 = input.LA(1);

            	    if ( (LA5_0==RULE_EOL) ) {
            	        alt5=1;
            	    }
            	    switch (alt5) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:505:5: RULE_EOL
            	            {
            	            match(input,RULE_EOL,FOLLOW_RULE_EOL_in_ruleWMLKey678); 
            	             
            	                createLeafNode(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_0(), null); 
            	                

            	            }
            	            break;

            	    }

            	    match(input,15,FOLLOW_15_in_ruleWMLKey689); 

            	            createLeafNode(grammarAccess.getWMLKeyAccess().getPlusSignKeyword_3_1(), null); 
            	        
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:513:1: ( RULE_EOL )?
            	    int alt6=2;
            	    int LA6_0 = input.LA(1);

            	    if ( (LA6_0==RULE_EOL) ) {
            	        alt6=1;
            	    }
            	    switch (alt6) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:513:2: RULE_EOL
            	            {
            	            match(input,RULE_EOL,FOLLOW_RULE_EOL_in_ruleWMLKey699); 
            	             
            	                createLeafNode(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_2(), null); 
            	                

            	            }
            	            break;

            	    }

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:517:3: ( (lv_value_6_0= ruleWMLKeyValue ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:518:1: (lv_value_6_0= ruleWMLKeyValue )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:518:1: (lv_value_6_0= ruleWMLKeyValue )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:519:3: lv_value_6_0= ruleWMLKeyValue
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_3_3_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKeyValue_in_ruleWMLKey721);
            	    lv_value_6_0=ruleWMLKeyValue();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"value",
            	    	        		lv_value_6_0, 
            	    	        		"WMLKeyValue", 
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
            	    break loop7;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:541:4: ( RULE_EOL | RULE_SL_COMMENT )
            int alt8=2;
            int LA8_0 = input.LA(1);

            if ( (LA8_0==RULE_EOL) ) {
                alt8=1;
            }
            else if ( (LA8_0==RULE_SL_COMMENT) ) {
                alt8=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("541:4: ( RULE_EOL | RULE_SL_COMMENT )", 8, 0, input);

                throw nvae;
            }
            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:541:5: RULE_EOL
                    {
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_ruleWMLKey733); 
                     
                        createLeafNode(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_4_0(), null); 
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:546:6: RULE_SL_COMMENT
                    {
                    match(input,RULE_SL_COMMENT,FOLLOW_RULE_SL_COMMENT_in_ruleWMLKey747); 
                     
                        createLeafNode(grammarAccess.getWMLKeyAccess().getSL_COMMENTTerminalRuleCall_4_1(), null); 
                        

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:561:1: entryRuleWMLKeyValue returns [EObject current=null] : iv_ruleWMLKeyValue= ruleWMLKeyValue EOF ;
    public final EObject entryRuleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKeyValue = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:562:2: (iv_ruleWMLKeyValue= ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:563:2: iv_ruleWMLKeyValue= ruleWMLKeyValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue787);
            iv_ruleWMLKeyValue=ruleWMLKeyValue();
            _fsp--;

             current =iv_ruleWMLKeyValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue797); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:570:1: ruleWMLKeyValue returns [EObject current=null] : (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall ) ;
    public final EObject ruleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject this_WMLValue_0 = null;

        EObject this_WMLMacroCall_1 = null;

        EObject this_WMLLuaCode_2 = null;

        EObject this_WMLArrayCall_3 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:575:6: ( (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:576:1: (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:576:1: (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall )
            int alt9=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
            case RULE_STRING:
            case RULE_ANY_OTHER:
            case 21:
            case 23:
            case 24:
            case 25:
            case 26:
            case 27:
                {
                alt9=1;
                }
                break;
            case 19:
                {
                alt9=2;
                }
                break;
            case RULE_LUA_CODE:
                {
                alt9=3;
                }
                break;
            case 14:
                {
                alt9=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("576:1: (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall )", 9, 0, input);

                throw nvae;
            }

            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:577:5: this_WMLValue_0= ruleWMLValue
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLKeyValue844);
                    this_WMLValue_0=ruleWMLValue();
                    _fsp--;

                     
                            current = this_WMLValue_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:587:5: this_WMLMacroCall_1= ruleWMLMacroCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyValue871);
                    this_WMLMacroCall_1=ruleWMLMacroCall();
                    _fsp--;

                     
                            current = this_WMLMacroCall_1; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:597:5: this_WMLLuaCode_2= ruleWMLLuaCode
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLLuaCode_in_ruleWMLKeyValue898);
                    this_WMLLuaCode_2=ruleWMLLuaCode();
                    _fsp--;

                     
                            current = this_WMLLuaCode_2; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:607:5: this_WMLArrayCall_3= ruleWMLArrayCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLArrayCallParserRuleCall_3(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLArrayCall_in_ruleWMLKeyValue925);
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:623:1: entryRuleWMLMacroCall returns [EObject current=null] : iv_ruleWMLMacroCall= ruleWMLMacroCall EOF ;
    public final EObject entryRuleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroCall = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:624:2: (iv_ruleWMLMacroCall= ruleWMLMacroCall EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:625:2: iv_ruleWMLMacroCall= ruleWMLMacroCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall960);
            iv_ruleWMLMacroCall=ruleWMLMacroCall();
            _fsp--;

             current =iv_ruleWMLMacroCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCall970); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:632:1: ruleWMLMacroCall returns [EObject current=null] : ( '{' ( './' )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( ( ( (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens ) ) ) | ( (lv_extraMacros_5_0= ruleWMLMacroCall ) ) )* '}' ) ;
    public final EObject ruleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        Token lv_relative_2_0=null;
        Token lv_name_3_0=null;
        EObject lv_params_4_1 = null;

        EObject lv_params_4_2 = null;

        EObject lv_extraMacros_5_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:637:6: ( ( '{' ( './' )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( ( ( (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens ) ) ) | ( (lv_extraMacros_5_0= ruleWMLMacroCall ) ) )* '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:638:1: ( '{' ( './' )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( ( ( (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens ) ) ) | ( (lv_extraMacros_5_0= ruleWMLMacroCall ) ) )* '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:638:1: ( '{' ( './' )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( ( ( (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens ) ) ) | ( (lv_extraMacros_5_0= ruleWMLMacroCall ) ) )* '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:638:3: '{' ( './' )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( ( ( (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens ) ) ) | ( (lv_extraMacros_5_0= ruleWMLMacroCall ) ) )* '}'
            {
            match(input,19,FOLLOW_19_in_ruleWMLMacroCall1005); 

                    createLeafNode(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:642:1: ( './' )?
            int alt10=2;
            int LA10_0 = input.LA(1);

            if ( (LA10_0==20) ) {
                alt10=1;
            }
            switch (alt10) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:642:3: './'
                    {
                    match(input,20,FOLLOW_20_in_ruleWMLMacroCall1016); 

                            createLeafNode(grammarAccess.getWMLMacroCallAccess().getFullStopSolidusKeyword_1(), null); 
                        

                    }
                    break;

            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:646:3: ( (lv_relative_2_0= '~' ) )?
            int alt11=2;
            int LA11_0 = input.LA(1);

            if ( (LA11_0==21) ) {
                alt11=1;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:647:1: (lv_relative_2_0= '~' )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:647:1: (lv_relative_2_0= '~' )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:648:3: lv_relative_2_0= '~'
                    {
                    lv_relative_2_0=(Token)input.LT(1);
                    match(input,21,FOLLOW_21_in_ruleWMLMacroCall1036); 

                            createLeafNode(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0(), "relative"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "relative", true, "~", lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }


                    }
                    break;

            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:667:3: ( (lv_name_3_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:668:1: (lv_name_3_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:668:1: (lv_name_3_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:669:3: lv_name_3_0= RULE_ID
            {
            lv_name_3_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacroCall1067); 

            			createLeafNode(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_3_0(), "name"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"name",
            	        		lv_name_3_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:691:2: ( ( ( (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens ) ) ) | ( (lv_extraMacros_5_0= ruleWMLMacroCall ) ) )*
            loop13:
            do {
                int alt13=3;
                int LA13_0 = input.LA(1);

                if ( (LA13_0==RULE_ID||(LA13_0>=RULE_STRING && LA13_0<=RULE_ANY_OTHER)||(LA13_0>=14 && LA13_0<=18)||LA13_0==21||(LA13_0>=23 && LA13_0<=27)) ) {
                    alt13=1;
                }
                else if ( (LA13_0==19) ) {
                    alt13=2;
                }


                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:691:3: ( ( (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens ) ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:691:3: ( ( (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens ) ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:692:1: ( (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:692:1: ( (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:693:1: (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:693:1: (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens )
            	    int alt12=2;
            	    int LA12_0 = input.LA(1);

            	    if ( (LA12_0==RULE_ID||(LA12_0>=RULE_STRING && LA12_0<=RULE_ANY_OTHER)||LA12_0==21||(LA12_0>=23 && LA12_0<=27)) ) {
            	        alt12=1;
            	    }
            	    else if ( ((LA12_0>=14 && LA12_0<=18)) ) {
            	        alt12=2;
            	    }
            	    else {
            	        NoViableAltException nvae =
            	            new NoViableAltException("693:1: (lv_params_4_1= ruleWMLValue | lv_params_4_2= ruleMacroTokens )", 12, 0, input);

            	        throw nvae;
            	    }
            	    switch (alt12) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:694:3: lv_params_4_1= ruleWMLValue
            	            {
            	             
            	            	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getParamsWMLValueParserRuleCall_4_0_0_0(), currentNode); 
            	            	    
            	            pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLMacroCall1096);
            	            lv_params_4_1=ruleWMLValue();
            	            _fsp--;


            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"params",
            	            	        		lv_params_4_1, 
            	            	        		"WMLValue", 
            	            	        		currentNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	        currentNode = currentNode.getParent();
            	            	    

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:715:8: lv_params_4_2= ruleMacroTokens
            	            {
            	             
            	            	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getParamsMacroTokensParserRuleCall_4_0_0_1(), currentNode); 
            	            	    
            	            pushFollow(FOLLOW_ruleMacroTokens_in_ruleWMLMacroCall1115);
            	            lv_params_4_2=ruleMacroTokens();
            	            _fsp--;


            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"params",
            	            	        		lv_params_4_2, 
            	            	        		"MacroTokens", 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:740:6: ( (lv_extraMacros_5_0= ruleWMLMacroCall ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:740:6: ( (lv_extraMacros_5_0= ruleWMLMacroCall ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:741:1: (lv_extraMacros_5_0= ruleWMLMacroCall )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:741:1: (lv_extraMacros_5_0= ruleWMLMacroCall )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:742:3: lv_extraMacros_5_0= ruleWMLMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getExtraMacrosWMLMacroCallParserRuleCall_4_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroCall1145);
            	    lv_extraMacros_5_0=ruleWMLMacroCall();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"extraMacros",
            	    	        		lv_extraMacros_5_0, 
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

            	default :
            	    break loop13;
                }
            } while (true);

            match(input,22,FOLLOW_22_in_ruleWMLMacroCall1157); 

                    createLeafNode(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_5(), null); 
                

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:776:1: entryRuleWMLLuaCode returns [EObject current=null] : iv_ruleWMLLuaCode= ruleWMLLuaCode EOF ;
    public final EObject entryRuleWMLLuaCode() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLLuaCode = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:777:2: (iv_ruleWMLLuaCode= ruleWMLLuaCode EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:778:2: iv_ruleWMLLuaCode= ruleWMLLuaCode EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLLuaCodeRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode1193);
            iv_ruleWMLLuaCode=ruleWMLLuaCode();
            _fsp--;

             current =iv_ruleWMLLuaCode; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLLuaCode1203); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:785:1: ruleWMLLuaCode returns [EObject current=null] : ( (lv_value_0_0= RULE_LUA_CODE ) ) ;
    public final EObject ruleWMLLuaCode() throws RecognitionException {
        EObject current = null;

        Token lv_value_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:790:6: ( ( (lv_value_0_0= RULE_LUA_CODE ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:791:1: ( (lv_value_0_0= RULE_LUA_CODE ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:791:1: ( (lv_value_0_0= RULE_LUA_CODE ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:792:1: (lv_value_0_0= RULE_LUA_CODE )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:792:1: (lv_value_0_0= RULE_LUA_CODE )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:793:3: lv_value_0_0= RULE_LUA_CODE
            {
            lv_value_0_0=(Token)input.LT(1);
            match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_ruleWMLLuaCode1244); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:823:1: entryRuleWMLArrayCall returns [EObject current=null] : iv_ruleWMLArrayCall= ruleWMLArrayCall EOF ;
    public final EObject entryRuleWMLArrayCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLArrayCall = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:824:2: (iv_ruleWMLArrayCall= ruleWMLArrayCall EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:825:2: iv_ruleWMLArrayCall= ruleWMLArrayCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLArrayCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall1284);
            iv_ruleWMLArrayCall=ruleWMLArrayCall();
            _fsp--;

             current =iv_ruleWMLArrayCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLArrayCall1294); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:832:1: ruleWMLArrayCall returns [EObject current=null] : ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' ) ;
    public final EObject ruleWMLArrayCall() throws RecognitionException {
        EObject current = null;

        EObject lv_value_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:837:6: ( ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:838:1: ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:838:1: ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:838:3: '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']'
            {
            match(input,14,FOLLOW_14_in_ruleWMLArrayCall1329); 

                    createLeafNode(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:842:1: ( (lv_value_1_0= ruleWMLValue ) )+
            int cnt14=0;
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( (LA14_0==RULE_ID||(LA14_0>=RULE_STRING && LA14_0<=RULE_ANY_OTHER)||LA14_0==21||(LA14_0>=23 && LA14_0<=27)) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:843:1: (lv_value_1_0= ruleWMLValue )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:843:1: (lv_value_1_0= ruleWMLValue )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:844:3: lv_value_1_0= ruleWMLValue
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLArrayCall1350);
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
            	    if ( cnt14 >= 1 ) break loop14;
                        EarlyExitException eee =
                            new EarlyExitException(14, input);
                        throw eee;
                }
                cnt14++;
            } while (true);

            match(input,16,FOLLOW_16_in_ruleWMLArrayCall1361); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:878:1: entryRuleWMLMacroDefine returns [EObject current=null] : iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF ;
    public final EObject entryRuleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroDefine = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:879:2: (iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:880:2: iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroDefineRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1397);
            iv_ruleWMLMacroDefine=ruleWMLMacroDefine();
            _fsp--;

             current =iv_ruleWMLMacroDefine; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroDefine1407); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:887:1: ruleWMLMacroDefine returns [EObject current=null] : ( ( (lv_name_0_0= RULE_DEFINE ) ) ( ( (lv_Tags_1_0= ruleWMLTag ) ) | ( (lv_Keys_2_0= ruleWMLKey ) ) | ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_5_0= ruleWMLTextdomain ) ) | ( (lv_Values_6_0= ruleWMLValue ) ) )* RULE_ENDDEF ) ;
    public final EObject ruleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;
        EObject lv_Tags_1_0 = null;

        EObject lv_Keys_2_0 = null;

        EObject lv_MacroCalls_3_0 = null;

        EObject lv_MacroDefines_4_0 = null;

        EObject lv_Textdomains_5_0 = null;

        EObject lv_Values_6_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:892:6: ( ( ( (lv_name_0_0= RULE_DEFINE ) ) ( ( (lv_Tags_1_0= ruleWMLTag ) ) | ( (lv_Keys_2_0= ruleWMLKey ) ) | ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_5_0= ruleWMLTextdomain ) ) | ( (lv_Values_6_0= ruleWMLValue ) ) )* RULE_ENDDEF ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:893:1: ( ( (lv_name_0_0= RULE_DEFINE ) ) ( ( (lv_Tags_1_0= ruleWMLTag ) ) | ( (lv_Keys_2_0= ruleWMLKey ) ) | ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_5_0= ruleWMLTextdomain ) ) | ( (lv_Values_6_0= ruleWMLValue ) ) )* RULE_ENDDEF )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:893:1: ( ( (lv_name_0_0= RULE_DEFINE ) ) ( ( (lv_Tags_1_0= ruleWMLTag ) ) | ( (lv_Keys_2_0= ruleWMLKey ) ) | ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_5_0= ruleWMLTextdomain ) ) | ( (lv_Values_6_0= ruleWMLValue ) ) )* RULE_ENDDEF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:893:2: ( (lv_name_0_0= RULE_DEFINE ) ) ( ( (lv_Tags_1_0= ruleWMLTag ) ) | ( (lv_Keys_2_0= ruleWMLKey ) ) | ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_5_0= ruleWMLTextdomain ) ) | ( (lv_Values_6_0= ruleWMLValue ) ) )* RULE_ENDDEF
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:893:2: ( (lv_name_0_0= RULE_DEFINE ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:894:1: (lv_name_0_0= RULE_DEFINE )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:894:1: (lv_name_0_0= RULE_DEFINE )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:895:3: lv_name_0_0= RULE_DEFINE
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1449); 

            			createLeafNode(grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0_0(), "name"); 
            		

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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:917:2: ( ( (lv_Tags_1_0= ruleWMLTag ) ) | ( (lv_Keys_2_0= ruleWMLKey ) ) | ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_5_0= ruleWMLTextdomain ) ) | ( (lv_Values_6_0= ruleWMLValue ) ) )*
            loop15:
            do {
                int alt15=7;
                switch ( input.LA(1) ) {
                case 14:
                    {
                    alt15=1;
                    }
                    break;
                case RULE_ID:
                    {
                    int LA15_3 = input.LA(2);

                    if ( (LA15_3==18) ) {
                        alt15=2;
                    }
                    else if ( (LA15_3==RULE_ID||(LA15_3>=RULE_DEFINE && LA15_3<=RULE_ANY_OTHER)||LA15_3==14||LA15_3==19||LA15_3==21||(LA15_3>=23 && LA15_3<=27)) ) {
                        alt15=6;
                    }


                    }
                    break;
                case 19:
                    {
                    alt15=3;
                    }
                    break;
                case RULE_DEFINE:
                    {
                    alt15=4;
                    }
                    break;
                case RULE_TEXTDOMAIN:
                    {
                    alt15=5;
                    }
                    break;
                case RULE_STRING:
                case RULE_ANY_OTHER:
                case 21:
                case 23:
                case 24:
                case 25:
                case 26:
                case 27:
                    {
                    alt15=6;
                    }
                    break;

                }

                switch (alt15) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:917:3: ( (lv_Tags_1_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:917:3: ( (lv_Tags_1_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:918:1: (lv_Tags_1_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:918:1: (lv_Tags_1_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:919:3: lv_Tags_1_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getTagsWMLTagParserRuleCall_1_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLMacroDefine1476);
            	    lv_Tags_1_0=ruleWMLTag();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Tags",
            	    	        		lv_Tags_1_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:942:6: ( (lv_Keys_2_0= ruleWMLKey ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:942:6: ( (lv_Keys_2_0= ruleWMLKey ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:1: (lv_Keys_2_0= ruleWMLKey )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:1: (lv_Keys_2_0= ruleWMLKey )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:944:3: lv_Keys_2_0= ruleWMLKey
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getKeysWMLKeyParserRuleCall_1_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLMacroDefine1503);
            	    lv_Keys_2_0=ruleWMLKey();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Keys",
            	    	        		lv_Keys_2_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:967:6: ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:967:6: ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:968:1: (lv_MacroCalls_3_0= ruleWMLMacroCall )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:968:1: (lv_MacroCalls_3_0= ruleWMLMacroCall )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:969:3: lv_MacroCalls_3_0= ruleWMLMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getMacroCallsWMLMacroCallParserRuleCall_1_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroDefine1530);
            	    lv_MacroCalls_3_0=ruleWMLMacroCall();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"MacroCalls",
            	    	        		lv_MacroCalls_3_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:992:6: ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:992:6: ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:993:1: (lv_MacroDefines_4_0= ruleWMLMacroDefine )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:993:1: (lv_MacroDefines_4_0= ruleWMLMacroDefine )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:994:3: lv_MacroDefines_4_0= ruleWMLMacroDefine
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getMacroDefinesWMLMacroDefineParserRuleCall_1_3_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroDefine_in_ruleWMLMacroDefine1557);
            	    lv_MacroDefines_4_0=ruleWMLMacroDefine();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"MacroDefines",
            	    	        		lv_MacroDefines_4_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1017:6: ( (lv_Textdomains_5_0= ruleWMLTextdomain ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1017:6: ( (lv_Textdomains_5_0= ruleWMLTextdomain ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1018:1: (lv_Textdomains_5_0= ruleWMLTextdomain )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1018:1: (lv_Textdomains_5_0= ruleWMLTextdomain )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1019:3: lv_Textdomains_5_0= ruleWMLTextdomain
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getTextdomainsWMLTextdomainParserRuleCall_1_4_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTextdomain_in_ruleWMLMacroDefine1584);
            	    lv_Textdomains_5_0=ruleWMLTextdomain();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Textdomains",
            	    	        		lv_Textdomains_5_0, 
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
            	case 6 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1042:6: ( (lv_Values_6_0= ruleWMLValue ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1042:6: ( (lv_Values_6_0= ruleWMLValue ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1043:1: (lv_Values_6_0= ruleWMLValue )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1043:1: (lv_Values_6_0= ruleWMLValue )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1044:3: lv_Values_6_0= ruleWMLValue
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getValuesWMLValueParserRuleCall_1_5_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLMacroDefine1611);
            	    lv_Values_6_0=ruleWMLValue();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Values",
            	    	        		lv_Values_6_0, 
            	    	        		"WMLValue", 
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
            	    break loop15;
                }
            } while (true);

            match(input,RULE_ENDDEF,FOLLOW_RULE_ENDDEF_in_ruleWMLMacroDefine1622); 
             
                createLeafNode(grammarAccess.getWMLMacroDefineAccess().getENDDEFTerminalRuleCall_2(), null); 
                

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1078:1: entryRuleWMLTextdomain returns [EObject current=null] : iv_ruleWMLTextdomain= ruleWMLTextdomain EOF ;
    public final EObject entryRuleWMLTextdomain() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTextdomain = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1079:2: (iv_ruleWMLTextdomain= ruleWMLTextdomain EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1080:2: iv_ruleWMLTextdomain= ruleWMLTextdomain EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLTextdomainRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain1657);
            iv_ruleWMLTextdomain=ruleWMLTextdomain();
            _fsp--;

             current =iv_ruleWMLTextdomain; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTextdomain1667); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1087:1: ruleWMLTextdomain returns [EObject current=null] : ( (lv_name_0_0= RULE_TEXTDOMAIN ) ) ;
    public final EObject ruleWMLTextdomain() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1092:6: ( ( (lv_name_0_0= RULE_TEXTDOMAIN ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1093:1: ( (lv_name_0_0= RULE_TEXTDOMAIN ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1093:1: ( (lv_name_0_0= RULE_TEXTDOMAIN ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1094:1: (lv_name_0_0= RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1094:1: (lv_name_0_0= RULE_TEXTDOMAIN )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1095:3: lv_name_0_0= RULE_TEXTDOMAIN
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_ruleWMLTextdomain1708); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1125:1: entryRuleWMLValue returns [EObject current=null] : iv_ruleWMLValue= ruleWMLValue EOF ;
    public final EObject entryRuleWMLValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLValue = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1126:2: (iv_ruleWMLValue= ruleWMLValue EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1127:2: iv_ruleWMLValue= ruleWMLValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLValue_in_entryRuleWMLValue1748);
            iv_ruleWMLValue=ruleWMLValue();
            _fsp--;

             current =iv_ruleWMLValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValue1758); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1134:1: ruleWMLValue returns [EObject current=null] : ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= ruleTSTRING | lv_value_0_3= ruleTVAR | lv_value_0_4= RULE_STRING | lv_value_0_5= '~' | lv_value_0_6= '.' | lv_value_0_7= '(' | lv_value_0_8= ')' | lv_value_0_9= RULE_ANY_OTHER ) ) ) ;
    public final EObject ruleWMLValue() throws RecognitionException {
        EObject current = null;

        Token lv_value_0_1=null;
        Token lv_value_0_4=null;
        Token lv_value_0_5=null;
        Token lv_value_0_6=null;
        Token lv_value_0_7=null;
        Token lv_value_0_8=null;
        Token lv_value_0_9=null;
        AntlrDatatypeRuleToken lv_value_0_2 = null;

        AntlrDatatypeRuleToken lv_value_0_3 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1139:6: ( ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= ruleTSTRING | lv_value_0_3= ruleTVAR | lv_value_0_4= RULE_STRING | lv_value_0_5= '~' | lv_value_0_6= '.' | lv_value_0_7= '(' | lv_value_0_8= ')' | lv_value_0_9= RULE_ANY_OTHER ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1140:1: ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= ruleTSTRING | lv_value_0_3= ruleTVAR | lv_value_0_4= RULE_STRING | lv_value_0_5= '~' | lv_value_0_6= '.' | lv_value_0_7= '(' | lv_value_0_8= ')' | lv_value_0_9= RULE_ANY_OTHER ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1140:1: ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= ruleTSTRING | lv_value_0_3= ruleTVAR | lv_value_0_4= RULE_STRING | lv_value_0_5= '~' | lv_value_0_6= '.' | lv_value_0_7= '(' | lv_value_0_8= ')' | lv_value_0_9= RULE_ANY_OTHER ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1141:1: ( (lv_value_0_1= RULE_ID | lv_value_0_2= ruleTSTRING | lv_value_0_3= ruleTVAR | lv_value_0_4= RULE_STRING | lv_value_0_5= '~' | lv_value_0_6= '.' | lv_value_0_7= '(' | lv_value_0_8= ')' | lv_value_0_9= RULE_ANY_OTHER ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1141:1: ( (lv_value_0_1= RULE_ID | lv_value_0_2= ruleTSTRING | lv_value_0_3= ruleTVAR | lv_value_0_4= RULE_STRING | lv_value_0_5= '~' | lv_value_0_6= '.' | lv_value_0_7= '(' | lv_value_0_8= ')' | lv_value_0_9= RULE_ANY_OTHER ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1142:1: (lv_value_0_1= RULE_ID | lv_value_0_2= ruleTSTRING | lv_value_0_3= ruleTVAR | lv_value_0_4= RULE_STRING | lv_value_0_5= '~' | lv_value_0_6= '.' | lv_value_0_7= '(' | lv_value_0_8= ')' | lv_value_0_9= RULE_ANY_OTHER )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1142:1: (lv_value_0_1= RULE_ID | lv_value_0_2= ruleTSTRING | lv_value_0_3= ruleTVAR | lv_value_0_4= RULE_STRING | lv_value_0_5= '~' | lv_value_0_6= '.' | lv_value_0_7= '(' | lv_value_0_8= ')' | lv_value_0_9= RULE_ANY_OTHER )
            int alt16=9;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt16=1;
                }
                break;
            case 26:
                {
                alt16=2;
                }
                break;
            case 27:
                {
                alt16=3;
                }
                break;
            case RULE_STRING:
                {
                alt16=4;
                }
                break;
            case 21:
                {
                alt16=5;
                }
                break;
            case 23:
                {
                alt16=6;
                }
                break;
            case 24:
                {
                alt16=7;
                }
                break;
            case 25:
                {
                alt16=8;
                }
                break;
            case RULE_ANY_OTHER:
                {
                alt16=9;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("1142:1: (lv_value_0_1= RULE_ID | lv_value_0_2= ruleTSTRING | lv_value_0_3= ruleTVAR | lv_value_0_4= RULE_STRING | lv_value_0_5= '~' | lv_value_0_6= '.' | lv_value_0_7= '(' | lv_value_0_8= ')' | lv_value_0_9= RULE_ANY_OTHER )", 16, 0, input);

                throw nvae;
            }

            switch (alt16) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1143:3: lv_value_0_1= RULE_ID
                    {
                    lv_value_0_1=(Token)input.LT(1);
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLValue1801); 

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
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1164:8: lv_value_0_2= ruleTSTRING
                    {
                     
                    	        currentNode=createCompositeNode(grammarAccess.getWMLValueAccess().getValueTSTRINGParserRuleCall_0_1(), currentNode); 
                    	    
                    pushFollow(FOLLOW_ruleTSTRING_in_ruleWMLValue1825);
                    lv_value_0_2=ruleTSTRING();
                    _fsp--;


                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode.getParent(), current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"value",
                    	        		lv_value_0_2, 
                    	        		"TSTRING", 
                    	        		currentNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	        currentNode = currentNode.getParent();
                    	    

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1185:8: lv_value_0_3= ruleTVAR
                    {
                     
                    	        currentNode=createCompositeNode(grammarAccess.getWMLValueAccess().getValueTVARParserRuleCall_0_2(), currentNode); 
                    	    
                    pushFollow(FOLLOW_ruleTVAR_in_ruleWMLValue1844);
                    lv_value_0_3=ruleTVAR();
                    _fsp--;


                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode.getParent(), current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"value",
                    	        		lv_value_0_3, 
                    	        		"TVAR", 
                    	        		currentNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	        currentNode = currentNode.getParent();
                    	    

                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1206:8: lv_value_0_4= RULE_STRING
                    {
                    lv_value_0_4=(Token)input.LT(1);
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLValue1859); 

                    			createLeafNode(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_3(), "value"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"value",
                    	        		lv_value_0_4, 
                    	        		"STRING", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1227:8: lv_value_0_5= '~'
                    {
                    lv_value_0_5=(Token)input.LT(1);
                    match(input,21,FOLLOW_21_in_ruleWMLValue1880); 

                            createLeafNode(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_4(), "value"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "value", lv_value_0_5, null, lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1245:8: lv_value_0_6= '.'
                    {
                    lv_value_0_6=(Token)input.LT(1);
                    match(input,23,FOLLOW_23_in_ruleWMLValue1909); 

                            createLeafNode(grammarAccess.getWMLValueAccess().getValueFullStopKeyword_0_5(), "value"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "value", lv_value_0_6, null, lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1263:8: lv_value_0_7= '('
                    {
                    lv_value_0_7=(Token)input.LT(1);
                    match(input,24,FOLLOW_24_in_ruleWMLValue1938); 

                            createLeafNode(grammarAccess.getWMLValueAccess().getValueLeftParenthesisKeyword_0_6(), "value"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "value", lv_value_0_7, null, lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 8 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1281:8: lv_value_0_8= ')'
                    {
                    lv_value_0_8=(Token)input.LT(1);
                    match(input,25,FOLLOW_25_in_ruleWMLValue1967); 

                            createLeafNode(grammarAccess.getWMLValueAccess().getValueRightParenthesisKeyword_0_7(), "value"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "value", lv_value_0_8, null, lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 9 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1299:8: lv_value_0_9= RULE_ANY_OTHER
                    {
                    lv_value_0_9=(Token)input.LT(1);
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_ruleWMLValue1995); 

                    			createLeafNode(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_8(), "value"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"value",
                    	        		lv_value_0_9, 
                    	        		"ANY_OTHER", 
                    	        		lastConsumedNode);
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


    // $ANTLR start entryRuleTSTRING
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1331:1: entryRuleTSTRING returns [String current=null] : iv_ruleTSTRING= ruleTSTRING EOF ;
    public final String entryRuleTSTRING() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleTSTRING = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1332:2: (iv_ruleTSTRING= ruleTSTRING EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1333:2: iv_ruleTSTRING= ruleTSTRING EOF
            {
             currentNode = createCompositeNode(grammarAccess.getTSTRINGRule(), currentNode); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING2039);
            iv_ruleTSTRING=ruleTSTRING();
            _fsp--;

             current =iv_ruleTSTRING.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING2050); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1340:1: ruleTSTRING returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (kw= '_' this_STRING_1= RULE_STRING ) ;
    public final AntlrDatatypeRuleToken ruleTSTRING() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        Token this_STRING_1=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1345:6: ( (kw= '_' this_STRING_1= RULE_STRING ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1346:1: (kw= '_' this_STRING_1= RULE_STRING )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1346:1: (kw= '_' this_STRING_1= RULE_STRING )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1347:2: kw= '_' this_STRING_1= RULE_STRING
            {
            kw=(Token)input.LT(1);
            match(input,26,FOLLOW_26_in_ruleTSTRING2088); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getTSTRINGAccess().get_Keyword_0(), null); 
                
            this_STRING_1=(Token)input.LT(1);
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleTSTRING2103); 

            		current.merge(this_STRING_1);
                
             
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


    // $ANTLR start entryRuleTVAR
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:1: entryRuleTVAR returns [String current=null] : iv_ruleTVAR= ruleTVAR EOF ;
    public final String entryRuleTVAR() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleTVAR = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1368:2: (iv_ruleTVAR= ruleTVAR EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1369:2: iv_ruleTVAR= ruleTVAR EOF
            {
             currentNode = createCompositeNode(grammarAccess.getTVARRule(), currentNode); 
            pushFollow(FOLLOW_ruleTVAR_in_entryRuleTVAR2149);
            iv_ruleTVAR=ruleTVAR();
            _fsp--;

             current =iv_ruleTVAR.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTVAR2160); 

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
    // $ANTLR end entryRuleTVAR


    // $ANTLR start ruleTVAR
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1376:1: ruleTVAR returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : kw= '_$' ;
    public final AntlrDatatypeRuleToken ruleTVAR() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1381:6: (kw= '_$' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1383:2: kw= '_$'
            {
            kw=(Token)input.LT(1);
            match(input,27,FOLLOW_27_in_ruleTVAR2197); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getTVARAccess().get_Keyword(), null); 
                

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
    // $ANTLR end ruleTVAR


    // $ANTLR start entryRuleMacroTokens
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1396:1: entryRuleMacroTokens returns [EObject current=null] : iv_ruleMacroTokens= ruleMacroTokens EOF ;
    public final EObject entryRuleMacroTokens() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleMacroTokens = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1397:2: (iv_ruleMacroTokens= ruleMacroTokens EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1398:2: iv_ruleMacroTokens= ruleMacroTokens EOF
            {
             currentNode = createCompositeNode(grammarAccess.getMacroTokensRule(), currentNode); 
            pushFollow(FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens2236);
            iv_ruleMacroTokens=ruleMacroTokens();
            _fsp--;

             current =iv_ruleMacroTokens; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleMacroTokens2246); 

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
    // $ANTLR end entryRuleMacroTokens


    // $ANTLR start ruleMacroTokens
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1405:1: ruleMacroTokens returns [EObject current=null] : ( ( (lv_val_0_1= '=' | lv_val_0_2= '[' | lv_val_0_3= ']' | lv_val_0_4= '+' | lv_val_0_5= '[/' ) ) ) ;
    public final EObject ruleMacroTokens() throws RecognitionException {
        EObject current = null;

        Token lv_val_0_1=null;
        Token lv_val_0_2=null;
        Token lv_val_0_3=null;
        Token lv_val_0_4=null;
        Token lv_val_0_5=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1410:6: ( ( ( (lv_val_0_1= '=' | lv_val_0_2= '[' | lv_val_0_3= ']' | lv_val_0_4= '+' | lv_val_0_5= '[/' ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1411:1: ( ( (lv_val_0_1= '=' | lv_val_0_2= '[' | lv_val_0_3= ']' | lv_val_0_4= '+' | lv_val_0_5= '[/' ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1411:1: ( ( (lv_val_0_1= '=' | lv_val_0_2= '[' | lv_val_0_3= ']' | lv_val_0_4= '+' | lv_val_0_5= '[/' ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1412:1: ( (lv_val_0_1= '=' | lv_val_0_2= '[' | lv_val_0_3= ']' | lv_val_0_4= '+' | lv_val_0_5= '[/' ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1412:1: ( (lv_val_0_1= '=' | lv_val_0_2= '[' | lv_val_0_3= ']' | lv_val_0_4= '+' | lv_val_0_5= '[/' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1413:1: (lv_val_0_1= '=' | lv_val_0_2= '[' | lv_val_0_3= ']' | lv_val_0_4= '+' | lv_val_0_5= '[/' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1413:1: (lv_val_0_1= '=' | lv_val_0_2= '[' | lv_val_0_3= ']' | lv_val_0_4= '+' | lv_val_0_5= '[/' )
            int alt17=5;
            switch ( input.LA(1) ) {
            case 18:
                {
                alt17=1;
                }
                break;
            case 14:
                {
                alt17=2;
                }
                break;
            case 16:
                {
                alt17=3;
                }
                break;
            case 15:
                {
                alt17=4;
                }
                break;
            case 17:
                {
                alt17=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("1413:1: (lv_val_0_1= '=' | lv_val_0_2= '[' | lv_val_0_3= ']' | lv_val_0_4= '+' | lv_val_0_5= '[/' )", 17, 0, input);

                throw nvae;
            }

            switch (alt17) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1414:3: lv_val_0_1= '='
                    {
                    lv_val_0_1=(Token)input.LT(1);
                    match(input,18,FOLLOW_18_in_ruleMacroTokens2290); 

                            createLeafNode(grammarAccess.getMacroTokensAccess().getValEqualsSignKeyword_0_0(), "val"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getMacroTokensRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "val", lv_val_0_1, null, lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1432:8: lv_val_0_2= '['
                    {
                    lv_val_0_2=(Token)input.LT(1);
                    match(input,14,FOLLOW_14_in_ruleMacroTokens2319); 

                            createLeafNode(grammarAccess.getMacroTokensAccess().getValLeftSquareBracketKeyword_0_1(), "val"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getMacroTokensRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "val", lv_val_0_2, null, lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1450:8: lv_val_0_3= ']'
                    {
                    lv_val_0_3=(Token)input.LT(1);
                    match(input,16,FOLLOW_16_in_ruleMacroTokens2348); 

                            createLeafNode(grammarAccess.getMacroTokensAccess().getValRightSquareBracketKeyword_0_2(), "val"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getMacroTokensRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "val", lv_val_0_3, null, lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1468:8: lv_val_0_4= '+'
                    {
                    lv_val_0_4=(Token)input.LT(1);
                    match(input,15,FOLLOW_15_in_ruleMacroTokens2377); 

                            createLeafNode(grammarAccess.getMacroTokensAccess().getValPlusSignKeyword_0_3(), "val"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getMacroTokensRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "val", lv_val_0_4, null, lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1486:8: lv_val_0_5= '[/'
                    {
                    lv_val_0_5=(Token)input.LT(1);
                    match(input,17,FOLLOW_17_in_ruleMacroTokens2406); 

                            createLeafNode(grammarAccess.getMacroTokensAccess().getValLeftSquareBracketSolidusKeyword_0_4(), "val"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getMacroTokensRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "val", lv_val_0_5, null, lastConsumedNode);
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
    // $ANTLR end ruleMacroTokens


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot75 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot85 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLRoot131 = new BitSet(new long[]{0x0000000000084502L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLRoot158 = new BitSet(new long[]{0x0000000000084502L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLRoot185 = new BitSet(new long[]{0x0000000000084502L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_ruleWMLRoot212 = new BitSet(new long[]{0x0000000000084502L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag249 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag259 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_ruleWMLTag294 = new BitSet(new long[]{0x0000000000008010L});
    public static final BitSet FOLLOW_15_in_ruleWMLTag312 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag343 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_ruleWMLTag358 = new BitSet(new long[]{0x00000000000A4510L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLTag380 = new BitSet(new long[]{0x00000000000A4510L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLTag407 = new BitSet(new long[]{0x00000000000A4510L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLTag434 = new BitSet(new long[]{0x00000000000A4510L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLTag461 = new BitSet(new long[]{0x00000000000A4510L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_ruleWMLTag488 = new BitSet(new long[]{0x00000000000A4510L});
    public static final BitSet FOLLOW_17_in_ruleWMLTag500 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag517 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_ruleWMLTag532 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey574 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey584 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKey630 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_ruleWMLKey645 = new BitSet(new long[]{0x000000000FA8D8F0L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLKey666 = new BitSet(new long[]{0x000000000FA8D8F0L});
    public static final BitSet FOLLOW_RULE_EOL_in_ruleWMLKey678 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_15_in_ruleWMLKey689 = new BitSet(new long[]{0x000000000FA858B0L});
    public static final BitSet FOLLOW_RULE_EOL_in_ruleWMLKey699 = new BitSet(new long[]{0x000000000FA85890L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLKey721 = new BitSet(new long[]{0x0000000000008060L});
    public static final BitSet FOLLOW_RULE_EOL_in_ruleWMLKey733 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_SL_COMMENT_in_ruleWMLKey747 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue787 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue797 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLKeyValue844 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyValue871 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_ruleWMLKeyValue898 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_ruleWMLKeyValue925 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall960 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCall970 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_ruleWMLMacroCall1005 = new BitSet(new long[]{0x0000000000300010L});
    public static final BitSet FOLLOW_20_in_ruleWMLMacroCall1016 = new BitSet(new long[]{0x0000000000200010L});
    public static final BitSet FOLLOW_21_in_ruleWMLMacroCall1036 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLMacroCall1067 = new BitSet(new long[]{0x000000000FEFD810L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLMacroCall1096 = new BitSet(new long[]{0x000000000FEFD810L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_ruleWMLMacroCall1115 = new BitSet(new long[]{0x000000000FEFD810L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroCall1145 = new BitSet(new long[]{0x000000000FEFD810L});
    public static final BitSet FOLLOW_22_in_ruleWMLMacroCall1157 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode1193 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLLuaCode1203 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_ruleWMLLuaCode1244 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall1284 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLArrayCall1294 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_ruleWMLArrayCall1329 = new BitSet(new long[]{0x000000000FA01810L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLArrayCall1350 = new BitSet(new long[]{0x000000000FA11810L});
    public static final BitSet FOLLOW_16_in_ruleWMLArrayCall1361 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1397 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine1407 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1449 = new BitSet(new long[]{0x000000000FA85F10L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLMacroDefine1476 = new BitSet(new long[]{0x000000000FA85F10L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLMacroDefine1503 = new BitSet(new long[]{0x000000000FA85F10L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroDefine1530 = new BitSet(new long[]{0x000000000FA85F10L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLMacroDefine1557 = new BitSet(new long[]{0x000000000FA85F10L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_ruleWMLMacroDefine1584 = new BitSet(new long[]{0x000000000FA85F10L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLMacroDefine1611 = new BitSet(new long[]{0x000000000FA85F10L});
    public static final BitSet FOLLOW_RULE_ENDDEF_in_ruleWMLMacroDefine1622 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain1657 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTextdomain1667 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_ruleWMLTextdomain1708 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_entryRuleWMLValue1748 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValue1758 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLValue1801 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_ruleWMLValue1825 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTVAR_in_ruleWMLValue1844 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLValue1859 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_ruleWMLValue1880 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_ruleWMLValue1909 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_ruleWMLValue1938 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_ruleWMLValue1967 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_ruleWMLValue1995 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING2039 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING2050 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_ruleTSTRING2088 = new BitSet(new long[]{0x0000000000000800L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleTSTRING2103 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTVAR_in_entryRuleTVAR2149 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTVAR2160 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_ruleTVAR2197 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens2236 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleMacroTokens2246 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_18_in_ruleMacroTokens2290 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_ruleMacroTokens2319 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_ruleMacroTokens2348 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_15_in_ruleMacroTokens2377 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_ruleMacroTokens2406 = new BitSet(new long[]{0x0000000000000002L});

}