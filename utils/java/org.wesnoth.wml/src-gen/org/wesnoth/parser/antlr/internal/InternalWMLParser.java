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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_LUA_CODE", "RULE_DEFINE", "RULE_ENDDEF", "RULE_TEXTDOMAIN", "RULE_STRING", "RULE_ANY_OTHER", "RULE_EOL", "RULE_WS", "RULE_SL_COMMENT", "'['", "'+'", "']'", "'[/'", "'='", "'{'", "'~'", "'}'", "'('", "')'"
    };
    public static final int RULE_LUA_CODE=5;
    public static final int RULE_ID=4;
    public static final int RULE_STRING=9;
    public static final int RULE_DEFINE=6;
    public static final int RULE_ANY_OTHER=10;
    public static final int RULE_ENDDEF=7;
    public static final int RULE_EOL=11;
    public static final int RULE_TEXTDOMAIN=8;
    public static final int RULE_WS=12;
    public static final int RULE_SL_COMMENT=13;
    public static final int EOF=-1;

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:446:1: ruleWMLKey returns [EObject current=null] : ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ) ;
    public final EObject ruleWMLKey() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;
        EObject lv_value_2_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:452:6: ( ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:453:1: ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:453:1: ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:453:2: ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )
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
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:481:1: ( (lv_value_2_0= ruleWMLKeyValue ) )
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

            	myHiddenTokenState.restore();

        }
        return current;
    }
    // $ANTLR end ruleWMLKey


    // $ANTLR start entryRuleWMLKeyValue
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:516:1: entryRuleWMLKeyValue returns [EObject current=null] : iv_ruleWMLKeyValue= ruleWMLKeyValue EOF ;
    public final EObject entryRuleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKeyValue = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:517:2: (iv_ruleWMLKeyValue= ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:518:2: iv_ruleWMLKeyValue= ruleWMLKeyValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue706);
            iv_ruleWMLKeyValue=ruleWMLKeyValue();
            _fsp--;

             current =iv_ruleWMLKeyValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue716); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:525:1: ruleWMLKeyValue returns [EObject current=null] : (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall ) ;
    public final EObject ruleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject this_WMLValue_0 = null;

        EObject this_WMLMacroCall_1 = null;

        EObject this_WMLLuaCode_2 = null;

        EObject this_WMLArrayCall_3 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:530:6: ( (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:531:1: (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:531:1: (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall )
            int alt4=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
            case RULE_STRING:
            case RULE_ANY_OTHER:
            case 15:
            case 20:
                {
                alt4=1;
                }
                break;
            case 19:
                {
                alt4=2;
                }
                break;
            case RULE_LUA_CODE:
                {
                alt4=3;
                }
                break;
            case 14:
                {
                alt4=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("531:1: (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall )", 4, 0, input);

                throw nvae;
            }

            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:532:5: this_WMLValue_0= ruleWMLValue
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLKeyValue763);
                    this_WMLValue_0=ruleWMLValue();
                    _fsp--;

                     
                            current = this_WMLValue_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:542:5: this_WMLMacroCall_1= ruleWMLMacroCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyValue790);
                    this_WMLMacroCall_1=ruleWMLMacroCall();
                    _fsp--;

                     
                            current = this_WMLMacroCall_1; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:552:5: this_WMLLuaCode_2= ruleWMLLuaCode
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLLuaCode_in_ruleWMLKeyValue817);
                    this_WMLLuaCode_2=ruleWMLLuaCode();
                    _fsp--;

                     
                            current = this_WMLLuaCode_2; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:562:5: this_WMLArrayCall_3= ruleWMLArrayCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLArrayCallParserRuleCall_3(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLArrayCall_in_ruleWMLKeyValue844);
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:578:1: entryRuleWMLMacroCall returns [EObject current=null] : iv_ruleWMLMacroCall= ruleWMLMacroCall EOF ;
    public final EObject entryRuleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroCall = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:579:2: (iv_ruleWMLMacroCall= ruleWMLMacroCall EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:580:2: iv_ruleWMLMacroCall= ruleWMLMacroCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall879);
            iv_ruleWMLMacroCall=ruleWMLMacroCall();
            _fsp--;

             current =iv_ruleWMLMacroCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCall889); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:587:1: ruleWMLMacroCall returns [EObject current=null] : ( '{' ( (lv_relative_1_0= '~' ) )? ( (lv_name_2_0= RULE_ID ) ) ( ( ( (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter ) ) ) | ( (lv_extraMacros_4_0= ruleWMLMacroCall ) ) )* '}' ) ;
    public final EObject ruleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        Token lv_relative_1_0=null;
        Token lv_name_2_0=null;
        EObject lv_params_3_1 = null;

        EObject lv_params_3_2 = null;

        EObject lv_extraMacros_4_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:592:6: ( ( '{' ( (lv_relative_1_0= '~' ) )? ( (lv_name_2_0= RULE_ID ) ) ( ( ( (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter ) ) ) | ( (lv_extraMacros_4_0= ruleWMLMacroCall ) ) )* '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:593:1: ( '{' ( (lv_relative_1_0= '~' ) )? ( (lv_name_2_0= RULE_ID ) ) ( ( ( (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter ) ) ) | ( (lv_extraMacros_4_0= ruleWMLMacroCall ) ) )* '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:593:1: ( '{' ( (lv_relative_1_0= '~' ) )? ( (lv_name_2_0= RULE_ID ) ) ( ( ( (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter ) ) ) | ( (lv_extraMacros_4_0= ruleWMLMacroCall ) ) )* '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:593:3: '{' ( (lv_relative_1_0= '~' ) )? ( (lv_name_2_0= RULE_ID ) ) ( ( ( (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter ) ) ) | ( (lv_extraMacros_4_0= ruleWMLMacroCall ) ) )* '}'
            {
            match(input,19,FOLLOW_19_in_ruleWMLMacroCall924); 

                    createLeafNode(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:597:1: ( (lv_relative_1_0= '~' ) )?
            int alt5=2;
            int LA5_0 = input.LA(1);

            if ( (LA5_0==20) ) {
                alt5=1;
            }
            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:598:1: (lv_relative_1_0= '~' )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:598:1: (lv_relative_1_0= '~' )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:599:3: lv_relative_1_0= '~'
                    {
                    lv_relative_1_0=(Token)input.LT(1);
                    match(input,20,FOLLOW_20_in_ruleWMLMacroCall942); 

                            createLeafNode(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_1_0(), "relative"); 
                        

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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:618:3: ( (lv_name_2_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:619:1: (lv_name_2_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:619:1: (lv_name_2_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:620:3: lv_name_2_0= RULE_ID
            {
            lv_name_2_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacroCall973); 

            			createLeafNode(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_2_0(), "name"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:642:2: ( ( ( (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter ) ) ) | ( (lv_extraMacros_4_0= ruleWMLMacroCall ) ) )*
            loop7:
            do {
                int alt7=3;
                int LA7_0 = input.LA(1);

                if ( (LA7_0==RULE_ID||(LA7_0>=RULE_STRING && LA7_0<=RULE_ANY_OTHER)||LA7_0==15||LA7_0==20||LA7_0==22) ) {
                    alt7=1;
                }
                else if ( (LA7_0==19) ) {
                    alt7=2;
                }


                switch (alt7) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:642:3: ( ( (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter ) ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:642:3: ( ( (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter ) ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:643:1: ( (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:643:1: ( (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:644:1: (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:644:1: (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter )
            	    int alt6=2;
            	    int LA6_0 = input.LA(1);

            	    if ( (LA6_0==RULE_ID||(LA6_0>=RULE_STRING && LA6_0<=RULE_ANY_OTHER)||LA6_0==15||LA6_0==20) ) {
            	        alt6=1;
            	    }
            	    else if ( (LA6_0==22) ) {
            	        alt6=2;
            	    }
            	    else {
            	        NoViableAltException nvae =
            	            new NoViableAltException("644:1: (lv_params_3_1= ruleWMLValue | lv_params_3_2= ruleWMLMacroParameter )", 6, 0, input);

            	        throw nvae;
            	    }
            	    switch (alt6) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:645:3: lv_params_3_1= ruleWMLValue
            	            {
            	             
            	            	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getParamsWMLValueParserRuleCall_3_0_0_0(), currentNode); 
            	            	    
            	            pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLMacroCall1002);
            	            lv_params_3_1=ruleWMLValue();
            	            _fsp--;


            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"params",
            	            	        		lv_params_3_1, 
            	            	        		"WMLValue", 
            	            	        		currentNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	        currentNode = currentNode.getParent();
            	            	    

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:666:8: lv_params_3_2= ruleWMLMacroParameter
            	            {
            	             
            	            	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getParamsWMLMacroParameterParserRuleCall_3_0_0_1(), currentNode); 
            	            	    
            	            pushFollow(FOLLOW_ruleWMLMacroParameter_in_ruleWMLMacroCall1021);
            	            lv_params_3_2=ruleWMLMacroParameter();
            	            _fsp--;


            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"params",
            	            	        		lv_params_3_2, 
            	            	        		"WMLMacroParameter", 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:691:6: ( (lv_extraMacros_4_0= ruleWMLMacroCall ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:691:6: ( (lv_extraMacros_4_0= ruleWMLMacroCall ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:692:1: (lv_extraMacros_4_0= ruleWMLMacroCall )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:692:1: (lv_extraMacros_4_0= ruleWMLMacroCall )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:693:3: lv_extraMacros_4_0= ruleWMLMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getExtraMacrosWMLMacroCallParserRuleCall_3_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroCall1051);
            	    lv_extraMacros_4_0=ruleWMLMacroCall();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"extraMacros",
            	    	        		lv_extraMacros_4_0, 
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
            	    break loop7;
                }
            } while (true);

            match(input,21,FOLLOW_21_in_ruleWMLMacroCall1063); 

                    createLeafNode(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_4(), null); 
                

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


    // $ANTLR start entryRuleWMLMacroParameter
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:727:1: entryRuleWMLMacroParameter returns [EObject current=null] : iv_ruleWMLMacroParameter= ruleWMLMacroParameter EOF ;
    public final EObject entryRuleWMLMacroParameter() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroParameter = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:728:2: (iv_ruleWMLMacroParameter= ruleWMLMacroParameter EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:729:2: iv_ruleWMLMacroParameter= ruleWMLMacroParameter EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroParameterRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter1099);
            iv_ruleWMLMacroParameter=ruleWMLMacroParameter();
            _fsp--;

             current =iv_ruleWMLMacroParameter; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroParameter1109); 

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
    // $ANTLR end entryRuleWMLMacroParameter


    // $ANTLR start ruleWMLMacroParameter
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:736:1: ruleWMLMacroParameter returns [EObject current=null] : ( () '(' ( ( (lv_param_2_1= ruleWMLValue | lv_param_2_2= ruleWMLTag | lv_param_2_3= ruleWMLMacroCall | lv_param_2_4= ruleWMLKey ) ) )* ')' ) ;
    public final EObject ruleWMLMacroParameter() throws RecognitionException {
        EObject current = null;

        EObject lv_param_2_1 = null;

        EObject lv_param_2_2 = null;

        EObject lv_param_2_3 = null;

        EObject lv_param_2_4 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:741:6: ( ( () '(' ( ( (lv_param_2_1= ruleWMLValue | lv_param_2_2= ruleWMLTag | lv_param_2_3= ruleWMLMacroCall | lv_param_2_4= ruleWMLKey ) ) )* ')' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:742:1: ( () '(' ( ( (lv_param_2_1= ruleWMLValue | lv_param_2_2= ruleWMLTag | lv_param_2_3= ruleWMLMacroCall | lv_param_2_4= ruleWMLKey ) ) )* ')' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:742:1: ( () '(' ( ( (lv_param_2_1= ruleWMLValue | lv_param_2_2= ruleWMLTag | lv_param_2_3= ruleWMLMacroCall | lv_param_2_4= ruleWMLKey ) ) )* ')' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:742:2: () '(' ( ( (lv_param_2_1= ruleWMLValue | lv_param_2_2= ruleWMLTag | lv_param_2_3= ruleWMLMacroCall | lv_param_2_4= ruleWMLKey ) ) )* ')'
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:742:2: ()
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:743:5: 
            {
             
                    temp=factory.create(grammarAccess.getWMLMacroParameterAccess().getWMLMacroParameterAction_0().getType().getClassifier());
                    current = temp; 
                    temp = null;
                    CompositeNode newNode = createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getWMLMacroParameterAction_0(), currentNode.getParent());
                newNode.getChildren().add(currentNode);
                moveLookaheadInfo(currentNode, newNode);
                currentNode = newNode; 
                    associateNodeWithAstElement(currentNode, current); 
                

            }

            match(input,22,FOLLOW_22_in_ruleWMLMacroParameter1153); 

                    createLeafNode(grammarAccess.getWMLMacroParameterAccess().getLeftParenthesisKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:757:1: ( ( (lv_param_2_1= ruleWMLValue | lv_param_2_2= ruleWMLTag | lv_param_2_3= ruleWMLMacroCall | lv_param_2_4= ruleWMLKey ) ) )*
            loop9:
            do {
                int alt9=2;
                int LA9_0 = input.LA(1);

                if ( (LA9_0==RULE_ID||(LA9_0>=RULE_STRING && LA9_0<=RULE_ANY_OTHER)||(LA9_0>=14 && LA9_0<=15)||(LA9_0>=19 && LA9_0<=20)) ) {
                    alt9=1;
                }


                switch (alt9) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:758:1: ( (lv_param_2_1= ruleWMLValue | lv_param_2_2= ruleWMLTag | lv_param_2_3= ruleWMLMacroCall | lv_param_2_4= ruleWMLKey ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:758:1: ( (lv_param_2_1= ruleWMLValue | lv_param_2_2= ruleWMLTag | lv_param_2_3= ruleWMLMacroCall | lv_param_2_4= ruleWMLKey ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:759:1: (lv_param_2_1= ruleWMLValue | lv_param_2_2= ruleWMLTag | lv_param_2_3= ruleWMLMacroCall | lv_param_2_4= ruleWMLKey )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:759:1: (lv_param_2_1= ruleWMLValue | lv_param_2_2= ruleWMLTag | lv_param_2_3= ruleWMLMacroCall | lv_param_2_4= ruleWMLKey )
            	    int alt8=4;
            	    switch ( input.LA(1) ) {
            	    case RULE_ID:
            	        {
            	        int LA8_1 = input.LA(2);

            	        if ( (LA8_1==18) ) {
            	            alt8=4;
            	        }
            	        else if ( (LA8_1==RULE_ID||(LA8_1>=RULE_STRING && LA8_1<=RULE_ANY_OTHER)||(LA8_1>=14 && LA8_1<=15)||(LA8_1>=19 && LA8_1<=20)||LA8_1==23) ) {
            	            alt8=1;
            	        }
            	        else {
            	            NoViableAltException nvae =
            	                new NoViableAltException("759:1: (lv_param_2_1= ruleWMLValue | lv_param_2_2= ruleWMLTag | lv_param_2_3= ruleWMLMacroCall | lv_param_2_4= ruleWMLKey )", 8, 1, input);

            	            throw nvae;
            	        }
            	        }
            	        break;
            	    case RULE_STRING:
            	    case RULE_ANY_OTHER:
            	    case 15:
            	    case 20:
            	        {
            	        alt8=1;
            	        }
            	        break;
            	    case 14:
            	        {
            	        alt8=2;
            	        }
            	        break;
            	    case 19:
            	        {
            	        alt8=3;
            	        }
            	        break;
            	    default:
            	        NoViableAltException nvae =
            	            new NoViableAltException("759:1: (lv_param_2_1= ruleWMLValue | lv_param_2_2= ruleWMLTag | lv_param_2_3= ruleWMLMacroCall | lv_param_2_4= ruleWMLKey )", 8, 0, input);

            	        throw nvae;
            	    }

            	    switch (alt8) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:760:3: lv_param_2_1= ruleWMLValue
            	            {
            	             
            	            	        currentNode=createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getParamWMLValueParserRuleCall_2_0_0(), currentNode); 
            	            	    
            	            pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLMacroParameter1176);
            	            lv_param_2_1=ruleWMLValue();
            	            _fsp--;


            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroParameterRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"param",
            	            	        		lv_param_2_1, 
            	            	        		"WMLValue", 
            	            	        		currentNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	        currentNode = currentNode.getParent();
            	            	    

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:781:8: lv_param_2_2= ruleWMLTag
            	            {
            	             
            	            	        currentNode=createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getParamWMLTagParserRuleCall_2_0_1(), currentNode); 
            	            	    
            	            pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLMacroParameter1195);
            	            lv_param_2_2=ruleWMLTag();
            	            _fsp--;


            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroParameterRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"param",
            	            	        		lv_param_2_2, 
            	            	        		"WMLTag", 
            	            	        		currentNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	        currentNode = currentNode.getParent();
            	            	    

            	            }
            	            break;
            	        case 3 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:802:8: lv_param_2_3= ruleWMLMacroCall
            	            {
            	             
            	            	        currentNode=createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getParamWMLMacroCallParserRuleCall_2_0_2(), currentNode); 
            	            	    
            	            pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroParameter1214);
            	            lv_param_2_3=ruleWMLMacroCall();
            	            _fsp--;


            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroParameterRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"param",
            	            	        		lv_param_2_3, 
            	            	        		"WMLMacroCall", 
            	            	        		currentNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	        currentNode = currentNode.getParent();
            	            	    

            	            }
            	            break;
            	        case 4 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:823:8: lv_param_2_4= ruleWMLKey
            	            {
            	             
            	            	        currentNode=createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getParamWMLKeyParserRuleCall_2_0_3(), currentNode); 
            	            	    
            	            pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLMacroParameter1233);
            	            lv_param_2_4=ruleWMLKey();
            	            _fsp--;


            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroParameterRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"param",
            	            	        		lv_param_2_4, 
            	            	        		"WMLKey", 
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
            	    break;

            	default :
            	    break loop9;
                }
            } while (true);

            match(input,23,FOLLOW_23_in_ruleWMLMacroParameter1247); 

                    createLeafNode(grammarAccess.getWMLMacroParameterAccess().getRightParenthesisKeyword_3(), null); 
                

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
    // $ANTLR end ruleWMLMacroParameter


    // $ANTLR start entryRuleWMLLuaCode
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:859:1: entryRuleWMLLuaCode returns [EObject current=null] : iv_ruleWMLLuaCode= ruleWMLLuaCode EOF ;
    public final EObject entryRuleWMLLuaCode() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLLuaCode = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:860:2: (iv_ruleWMLLuaCode= ruleWMLLuaCode EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:861:2: iv_ruleWMLLuaCode= ruleWMLLuaCode EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLLuaCodeRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode1283);
            iv_ruleWMLLuaCode=ruleWMLLuaCode();
            _fsp--;

             current =iv_ruleWMLLuaCode; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLLuaCode1293); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:868:1: ruleWMLLuaCode returns [EObject current=null] : ( (lv_value_0_0= RULE_LUA_CODE ) ) ;
    public final EObject ruleWMLLuaCode() throws RecognitionException {
        EObject current = null;

        Token lv_value_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:873:6: ( ( (lv_value_0_0= RULE_LUA_CODE ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:874:1: ( (lv_value_0_0= RULE_LUA_CODE ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:874:1: ( (lv_value_0_0= RULE_LUA_CODE ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:875:1: (lv_value_0_0= RULE_LUA_CODE )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:875:1: (lv_value_0_0= RULE_LUA_CODE )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:876:3: lv_value_0_0= RULE_LUA_CODE
            {
            lv_value_0_0=(Token)input.LT(1);
            match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_ruleWMLLuaCode1334); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:906:1: entryRuleWMLArrayCall returns [EObject current=null] : iv_ruleWMLArrayCall= ruleWMLArrayCall EOF ;
    public final EObject entryRuleWMLArrayCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLArrayCall = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:907:2: (iv_ruleWMLArrayCall= ruleWMLArrayCall EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:908:2: iv_ruleWMLArrayCall= ruleWMLArrayCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLArrayCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall1374);
            iv_ruleWMLArrayCall=ruleWMLArrayCall();
            _fsp--;

             current =iv_ruleWMLArrayCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLArrayCall1384); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:915:1: ruleWMLArrayCall returns [EObject current=null] : ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' ) ;
    public final EObject ruleWMLArrayCall() throws RecognitionException {
        EObject current = null;

        EObject lv_value_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:920:6: ( ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:921:1: ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:921:1: ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:921:3: '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']'
            {
            match(input,14,FOLLOW_14_in_ruleWMLArrayCall1419); 

                    createLeafNode(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:925:1: ( (lv_value_1_0= ruleWMLValue ) )+
            int cnt10=0;
            loop10:
            do {
                int alt10=2;
                int LA10_0 = input.LA(1);

                if ( (LA10_0==RULE_ID||(LA10_0>=RULE_STRING && LA10_0<=RULE_ANY_OTHER)||LA10_0==15||LA10_0==20) ) {
                    alt10=1;
                }


                switch (alt10) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:926:1: (lv_value_1_0= ruleWMLValue )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:926:1: (lv_value_1_0= ruleWMLValue )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:927:3: lv_value_1_0= ruleWMLValue
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLArrayCall1440);
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
            	    if ( cnt10 >= 1 ) break loop10;
                        EarlyExitException eee =
                            new EarlyExitException(10, input);
                        throw eee;
                }
                cnt10++;
            } while (true);

            match(input,16,FOLLOW_16_in_ruleWMLArrayCall1451); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:961:1: entryRuleWMLMacroDefine returns [EObject current=null] : iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF ;
    public final EObject entryRuleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroDefine = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:962:2: (iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:963:2: iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroDefineRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1487);
            iv_ruleWMLMacroDefine=ruleWMLMacroDefine();
            _fsp--;

             current =iv_ruleWMLMacroDefine; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroDefine1497); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:970:1: ruleWMLMacroDefine returns [EObject current=null] : ( ( (lv_name_0_0= RULE_DEFINE ) ) ( ( (lv_Tags_1_0= ruleWMLTag ) ) | ( (lv_Keys_2_0= ruleWMLKey ) ) | ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_5_0= ruleWMLTextdomain ) ) )* RULE_ENDDEF ) ;
    public final EObject ruleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;
        EObject lv_Tags_1_0 = null;

        EObject lv_Keys_2_0 = null;

        EObject lv_MacroCalls_3_0 = null;

        EObject lv_MacroDefines_4_0 = null;

        EObject lv_Textdomains_5_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:975:6: ( ( ( (lv_name_0_0= RULE_DEFINE ) ) ( ( (lv_Tags_1_0= ruleWMLTag ) ) | ( (lv_Keys_2_0= ruleWMLKey ) ) | ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_5_0= ruleWMLTextdomain ) ) )* RULE_ENDDEF ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:976:1: ( ( (lv_name_0_0= RULE_DEFINE ) ) ( ( (lv_Tags_1_0= ruleWMLTag ) ) | ( (lv_Keys_2_0= ruleWMLKey ) ) | ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_5_0= ruleWMLTextdomain ) ) )* RULE_ENDDEF )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:976:1: ( ( (lv_name_0_0= RULE_DEFINE ) ) ( ( (lv_Tags_1_0= ruleWMLTag ) ) | ( (lv_Keys_2_0= ruleWMLKey ) ) | ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_5_0= ruleWMLTextdomain ) ) )* RULE_ENDDEF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:976:2: ( (lv_name_0_0= RULE_DEFINE ) ) ( ( (lv_Tags_1_0= ruleWMLTag ) ) | ( (lv_Keys_2_0= ruleWMLKey ) ) | ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_5_0= ruleWMLTextdomain ) ) )* RULE_ENDDEF
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:976:2: ( (lv_name_0_0= RULE_DEFINE ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:977:1: (lv_name_0_0= RULE_DEFINE )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:977:1: (lv_name_0_0= RULE_DEFINE )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:978:3: lv_name_0_0= RULE_DEFINE
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1539); 

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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1000:2: ( ( (lv_Tags_1_0= ruleWMLTag ) ) | ( (lv_Keys_2_0= ruleWMLKey ) ) | ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) ) | ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_Textdomains_5_0= ruleWMLTextdomain ) ) )*
            loop11:
            do {
                int alt11=6;
                switch ( input.LA(1) ) {
                case 14:
                    {
                    alt11=1;
                    }
                    break;
                case RULE_ID:
                    {
                    alt11=2;
                    }
                    break;
                case 19:
                    {
                    alt11=3;
                    }
                    break;
                case RULE_DEFINE:
                    {
                    alt11=4;
                    }
                    break;
                case RULE_TEXTDOMAIN:
                    {
                    alt11=5;
                    }
                    break;

                }

                switch (alt11) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1000:3: ( (lv_Tags_1_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1000:3: ( (lv_Tags_1_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1001:1: (lv_Tags_1_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1001:1: (lv_Tags_1_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1002:3: lv_Tags_1_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getTagsWMLTagParserRuleCall_1_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLMacroDefine1566);
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1025:6: ( (lv_Keys_2_0= ruleWMLKey ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1025:6: ( (lv_Keys_2_0= ruleWMLKey ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1026:1: (lv_Keys_2_0= ruleWMLKey )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1026:1: (lv_Keys_2_0= ruleWMLKey )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1027:3: lv_Keys_2_0= ruleWMLKey
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getKeysWMLKeyParserRuleCall_1_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLMacroDefine1593);
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1050:6: ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1050:6: ( (lv_MacroCalls_3_0= ruleWMLMacroCall ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1051:1: (lv_MacroCalls_3_0= ruleWMLMacroCall )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1051:1: (lv_MacroCalls_3_0= ruleWMLMacroCall )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1052:3: lv_MacroCalls_3_0= ruleWMLMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getMacroCallsWMLMacroCallParserRuleCall_1_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroDefine1620);
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1075:6: ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1075:6: ( (lv_MacroDefines_4_0= ruleWMLMacroDefine ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1076:1: (lv_MacroDefines_4_0= ruleWMLMacroDefine )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1076:1: (lv_MacroDefines_4_0= ruleWMLMacroDefine )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1077:3: lv_MacroDefines_4_0= ruleWMLMacroDefine
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getMacroDefinesWMLMacroDefineParserRuleCall_1_3_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroDefine_in_ruleWMLMacroDefine1647);
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1100:6: ( (lv_Textdomains_5_0= ruleWMLTextdomain ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1100:6: ( (lv_Textdomains_5_0= ruleWMLTextdomain ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1101:1: (lv_Textdomains_5_0= ruleWMLTextdomain )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1101:1: (lv_Textdomains_5_0= ruleWMLTextdomain )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1102:3: lv_Textdomains_5_0= ruleWMLTextdomain
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getTextdomainsWMLTextdomainParserRuleCall_1_4_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTextdomain_in_ruleWMLMacroDefine1674);
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

            	default :
            	    break loop11;
                }
            } while (true);

            match(input,RULE_ENDDEF,FOLLOW_RULE_ENDDEF_in_ruleWMLMacroDefine1685); 
             
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1136:1: entryRuleWMLTextdomain returns [EObject current=null] : iv_ruleWMLTextdomain= ruleWMLTextdomain EOF ;
    public final EObject entryRuleWMLTextdomain() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTextdomain = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1137:2: (iv_ruleWMLTextdomain= ruleWMLTextdomain EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1138:2: iv_ruleWMLTextdomain= ruleWMLTextdomain EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLTextdomainRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain1720);
            iv_ruleWMLTextdomain=ruleWMLTextdomain();
            _fsp--;

             current =iv_ruleWMLTextdomain; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTextdomain1730); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1145:1: ruleWMLTextdomain returns [EObject current=null] : ( (lv_name_0_0= RULE_TEXTDOMAIN ) ) ;
    public final EObject ruleWMLTextdomain() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1150:6: ( ( (lv_name_0_0= RULE_TEXTDOMAIN ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1151:1: ( (lv_name_0_0= RULE_TEXTDOMAIN ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1151:1: ( (lv_name_0_0= RULE_TEXTDOMAIN ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1152:1: (lv_name_0_0= RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1152:1: (lv_name_0_0= RULE_TEXTDOMAIN )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1153:3: lv_name_0_0= RULE_TEXTDOMAIN
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_ruleWMLTextdomain1771); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1183:1: entryRuleWMLValue returns [EObject current=null] : iv_ruleWMLValue= ruleWMLValue EOF ;
    public final EObject entryRuleWMLValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLValue = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1184:2: (iv_ruleWMLValue= ruleWMLValue EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1185:2: iv_ruleWMLValue= ruleWMLValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLValue_in_entryRuleWMLValue1811);
            iv_ruleWMLValue=ruleWMLValue();
            _fsp--;

             current =iv_ruleWMLValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValue1821); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1192:1: ruleWMLValue returns [EObject current=null] : ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '+' | lv_value_0_4= '~' | lv_value_0_5= RULE_ANY_OTHER ) ) )+ ;
    public final EObject ruleWMLValue() throws RecognitionException {
        EObject current = null;

        Token lv_value_0_1=null;
        Token lv_value_0_2=null;
        Token lv_value_0_3=null;
        Token lv_value_0_4=null;
        Token lv_value_0_5=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1197:6: ( ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '+' | lv_value_0_4= '~' | lv_value_0_5= RULE_ANY_OTHER ) ) )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1198:1: ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '+' | lv_value_0_4= '~' | lv_value_0_5= RULE_ANY_OTHER ) ) )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1198:1: ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '+' | lv_value_0_4= '~' | lv_value_0_5= RULE_ANY_OTHER ) ) )+
            int cnt13=0;
            loop13:
            do {
                int alt13=2;
                switch ( input.LA(1) ) {
                case RULE_ID:
                    {
                    alt13=1;
                    }
                    break;
                case RULE_STRING:
                    {
                    alt13=1;
                    }
                    break;
                case 15:
                    {
                    alt13=1;
                    }
                    break;
                case 20:
                    {
                    alt13=1;
                    }
                    break;
                case RULE_ANY_OTHER:
                    {
                    alt13=1;
                    }
                    break;

                }

                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1199:1: ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '+' | lv_value_0_4= '~' | lv_value_0_5= RULE_ANY_OTHER ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1199:1: ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '+' | lv_value_0_4= '~' | lv_value_0_5= RULE_ANY_OTHER ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1200:1: (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '+' | lv_value_0_4= '~' | lv_value_0_5= RULE_ANY_OTHER )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1200:1: (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '+' | lv_value_0_4= '~' | lv_value_0_5= RULE_ANY_OTHER )
            	    int alt12=5;
            	    switch ( input.LA(1) ) {
            	    case RULE_ID:
            	        {
            	        alt12=1;
            	        }
            	        break;
            	    case RULE_STRING:
            	        {
            	        alt12=2;
            	        }
            	        break;
            	    case 15:
            	        {
            	        alt12=3;
            	        }
            	        break;
            	    case 20:
            	        {
            	        alt12=4;
            	        }
            	        break;
            	    case RULE_ANY_OTHER:
            	        {
            	        alt12=5;
            	        }
            	        break;
            	    default:
            	        NoViableAltException nvae =
            	            new NoViableAltException("1200:1: (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '+' | lv_value_0_4= '~' | lv_value_0_5= RULE_ANY_OTHER )", 12, 0, input);

            	        throw nvae;
            	    }

            	    switch (alt12) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1201:3: lv_value_0_1= RULE_ID
            	            {
            	            lv_value_0_1=(Token)input.LT(1);
            	            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLValue1864); 

            	            			createLeafNode(grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0(), "value"); 
            	            		

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        try {
            	            	       		add(
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
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1222:8: lv_value_0_2= RULE_STRING
            	            {
            	            lv_value_0_2=(Token)input.LT(1);
            	            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLValue1884); 

            	            			createLeafNode(grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1(), "value"); 
            	            		

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        try {
            	            	       		add(
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
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1243:8: lv_value_0_3= '+'
            	            {
            	            lv_value_0_3=(Token)input.LT(1);
            	            match(input,15,FOLLOW_15_in_ruleWMLValue1905); 

            	                    createLeafNode(grammarAccess.getWMLValueAccess().getValuePlusSignKeyword_0_2(), "value"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "value", lv_value_0_3, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 4 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1261:8: lv_value_0_4= '~'
            	            {
            	            lv_value_0_4=(Token)input.LT(1);
            	            match(input,20,FOLLOW_20_in_ruleWMLValue1934); 

            	                    createLeafNode(grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_3(), "value"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "value", lv_value_0_4, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 5 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1279:8: lv_value_0_5= RULE_ANY_OTHER
            	            {
            	            lv_value_0_5=(Token)input.LT(1);
            	            match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_ruleWMLValue1962); 

            	            			createLeafNode(grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_4(), "value"); 
            	            		

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLValueRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"value",
            	            	        		lv_value_0_5, 
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
            	    break;

            	default :
            	    if ( cnt13 >= 1 ) break loop13;
                        EarlyExitException eee =
                            new EarlyExitException(13, input);
                        throw eee;
                }
                cnt13++;
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
    // $ANTLR end ruleWMLValue


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot75 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot85 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLRoot131 = new BitSet(new long[]{0x0000000000084142L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLRoot158 = new BitSet(new long[]{0x0000000000084142L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLRoot185 = new BitSet(new long[]{0x0000000000084142L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_ruleWMLRoot212 = new BitSet(new long[]{0x0000000000084142L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag249 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag259 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_ruleWMLTag294 = new BitSet(new long[]{0x0000000000008010L});
    public static final BitSet FOLLOW_15_in_ruleWMLTag312 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag343 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_ruleWMLTag358 = new BitSet(new long[]{0x00000000000A4150L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLTag380 = new BitSet(new long[]{0x00000000000A4150L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLTag407 = new BitSet(new long[]{0x00000000000A4150L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLTag434 = new BitSet(new long[]{0x00000000000A4150L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLTag461 = new BitSet(new long[]{0x00000000000A4150L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_ruleWMLTag488 = new BitSet(new long[]{0x00000000000A4150L});
    public static final BitSet FOLLOW_17_in_ruleWMLTag500 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag517 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_ruleWMLTag532 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey574 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey584 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKey630 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_ruleWMLKey645 = new BitSet(new long[]{0x000000000018C630L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLKey666 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue706 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue716 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLKeyValue763 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyValue790 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_ruleWMLKeyValue817 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_ruleWMLKeyValue844 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall879 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCall889 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_ruleWMLMacroCall924 = new BitSet(new long[]{0x0000000000100010L});
    public static final BitSet FOLLOW_20_in_ruleWMLMacroCall942 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLMacroCall973 = new BitSet(new long[]{0x0000000000788610L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLMacroCall1002 = new BitSet(new long[]{0x0000000000788610L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_ruleWMLMacroCall1021 = new BitSet(new long[]{0x0000000000788610L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroCall1051 = new BitSet(new long[]{0x0000000000788610L});
    public static final BitSet FOLLOW_21_in_ruleWMLMacroCall1063 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter1099 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroParameter1109 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_ruleWMLMacroParameter1153 = new BitSet(new long[]{0x000000000098C610L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLMacroParameter1176 = new BitSet(new long[]{0x000000000098C610L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLMacroParameter1195 = new BitSet(new long[]{0x000000000098C610L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroParameter1214 = new BitSet(new long[]{0x000000000098C610L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLMacroParameter1233 = new BitSet(new long[]{0x000000000098C610L});
    public static final BitSet FOLLOW_23_in_ruleWMLMacroParameter1247 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode1283 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLLuaCode1293 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_ruleWMLLuaCode1334 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall1374 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLArrayCall1384 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_ruleWMLArrayCall1419 = new BitSet(new long[]{0x0000000000108610L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLArrayCall1440 = new BitSet(new long[]{0x0000000000118610L});
    public static final BitSet FOLLOW_16_in_ruleWMLArrayCall1451 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1487 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine1497 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1539 = new BitSet(new long[]{0x00000000000841D0L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLMacroDefine1566 = new BitSet(new long[]{0x00000000000841D0L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLMacroDefine1593 = new BitSet(new long[]{0x00000000000841D0L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroDefine1620 = new BitSet(new long[]{0x00000000000841D0L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLMacroDefine1647 = new BitSet(new long[]{0x00000000000841D0L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_ruleWMLMacroDefine1674 = new BitSet(new long[]{0x00000000000841D0L});
    public static final BitSet FOLLOW_RULE_ENDDEF_in_ruleWMLMacroDefine1685 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain1720 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTextdomain1730 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_ruleWMLTextdomain1771 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_entryRuleWMLValue1811 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValue1821 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLValue1864 = new BitSet(new long[]{0x0000000000108612L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLValue1884 = new BitSet(new long[]{0x0000000000108612L});
    public static final BitSet FOLLOW_15_in_ruleWMLValue1905 = new BitSet(new long[]{0x0000000000108612L});
    public static final BitSet FOLLOW_20_in_ruleWMLValue1934 = new BitSet(new long[]{0x0000000000108612L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_ruleWMLValue1962 = new BitSet(new long[]{0x0000000000108612L});

}