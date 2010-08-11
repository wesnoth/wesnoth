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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_STRING", "RULE_ANY_OTHER", "RULE_DEFINE", "RULE_ENDDEFINE", "RULE_INT", "RULE_TEXTDOMAIN", "RULE_SL_COMMENT", "RULE_WS", "'['", "'+'", "']'", "'[/'", "'{'", "'}'", "'='", "'('", "')'", "'~'", "'/'", "','", "'_'", "'.'", "'-'"
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:86:1: ruleWMLRoot returns [EObject current=null] : ( ( (lv_tags_0_0= ruleWMLTag ) ) | ( (lv_macros_1_0= ruleWMLAbstractMacroCall ) ) | ( (lv_macrosDefines_2_0= ruleWMLMacroDefine ) ) )* ;
    public final EObject ruleWMLRoot() throws RecognitionException {
        EObject current = null;

        EObject lv_tags_0_0 = null;

        EObject lv_macros_1_0 = null;

        EObject lv_macrosDefines_2_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:91:6: ( ( ( (lv_tags_0_0= ruleWMLTag ) ) | ( (lv_macros_1_0= ruleWMLAbstractMacroCall ) ) | ( (lv_macrosDefines_2_0= ruleWMLMacroDefine ) ) )* )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( ( (lv_tags_0_0= ruleWMLTag ) ) | ( (lv_macros_1_0= ruleWMLAbstractMacroCall ) ) | ( (lv_macrosDefines_2_0= ruleWMLMacroDefine ) ) )*
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( ( (lv_tags_0_0= ruleWMLTag ) ) | ( (lv_macros_1_0= ruleWMLAbstractMacroCall ) ) | ( (lv_macrosDefines_2_0= ruleWMLMacroDefine ) ) )*
            loop1:
            do {
                int alt1=4;
                switch ( input.LA(1) ) {
                case 13:
                    {
                    alt1=1;
                    }
                    break;
                case 17:
                    {
                    alt1=2;
                    }
                    break;
                case RULE_DEFINE:
                    {
                    alt1=3;
                    }
                    break;

                }

                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:2: ( (lv_tags_0_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:2: ( (lv_tags_0_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:93:1: (lv_tags_0_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:93:1: (lv_tags_0_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:94:3: lv_tags_0_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getTagsWMLTagParserRuleCall_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLRoot131);
            	    lv_tags_0_0=ruleWMLTag();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"tags",
            	    	        		lv_tags_0_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:117:6: ( (lv_macros_1_0= ruleWMLAbstractMacroCall ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:117:6: ( (lv_macros_1_0= ruleWMLAbstractMacroCall ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:118:1: (lv_macros_1_0= ruleWMLAbstractMacroCall )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:118:1: (lv_macros_1_0= ruleWMLAbstractMacroCall )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:119:3: lv_macros_1_0= ruleWMLAbstractMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getMacrosWMLAbstractMacroCallParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLAbstractMacroCall_in_ruleWMLRoot158);
            	    lv_macros_1_0=ruleWMLAbstractMacroCall();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"macros",
            	    	        		lv_macros_1_0, 
            	    	        		"WMLAbstractMacroCall", 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:142:6: ( (lv_macrosDefines_2_0= ruleWMLMacroDefine ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:142:6: ( (lv_macrosDefines_2_0= ruleWMLMacroDefine ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:143:1: (lv_macrosDefines_2_0= ruleWMLMacroDefine )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:143:1: (lv_macrosDefines_2_0= ruleWMLMacroDefine )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:144:3: lv_macrosDefines_2_0= ruleWMLMacroDefine
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getMacrosDefinesWMLMacroDefineParserRuleCall_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroDefine_in_ruleWMLRoot185);
            	    lv_macrosDefines_2_0=ruleWMLMacroDefine();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"macrosDefines",
            	    	        		lv_macrosDefines_2_0, 
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:174:1: entryRuleWMLTag returns [EObject current=null] : iv_ruleWMLTag= ruleWMLTag EOF ;
    public final EObject entryRuleWMLTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:175:2: (iv_ruleWMLTag= ruleWMLTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:176:2: iv_ruleWMLTag= ruleWMLTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLTag_in_entryRuleWMLTag222);
            iv_ruleWMLTag=ruleWMLTag();
            _fsp--;

             current =iv_ruleWMLTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTag232); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:183:1: ruleWMLTag returns [EObject current=null] : ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_macros_5_0= ruleWMLAbstractMacroCall ) ) | ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) ) | ( (lv_keys_7_0= ruleWMLKey ) ) )* '[/' ( (lv_endName_9_0= RULE_ID ) ) ']' ) ;
    public final EObject ruleWMLTag() throws RecognitionException {
        EObject current = null;

        Token lv_plus_1_0=null;
        Token lv_name_2_0=null;
        Token lv_endName_9_0=null;
        EObject lv_tags_4_0 = null;

        EObject lv_macros_5_0 = null;

        EObject lv_macrosDefines_6_0 = null;

        EObject lv_keys_7_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:188:6: ( ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_macros_5_0= ruleWMLAbstractMacroCall ) ) | ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) ) | ( (lv_keys_7_0= ruleWMLKey ) ) )* '[/' ( (lv_endName_9_0= RULE_ID ) ) ']' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:189:1: ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_macros_5_0= ruleWMLAbstractMacroCall ) ) | ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) ) | ( (lv_keys_7_0= ruleWMLKey ) ) )* '[/' ( (lv_endName_9_0= RULE_ID ) ) ']' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:189:1: ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_macros_5_0= ruleWMLAbstractMacroCall ) ) | ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) ) | ( (lv_keys_7_0= ruleWMLKey ) ) )* '[/' ( (lv_endName_9_0= RULE_ID ) ) ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:189:3: '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_macros_5_0= ruleWMLAbstractMacroCall ) ) | ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) ) | ( (lv_keys_7_0= ruleWMLKey ) ) )* '[/' ( (lv_endName_9_0= RULE_ID ) ) ']'
            {
            match(input,13,FOLLOW_13_in_ruleWMLTag267); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:193:1: ( (lv_plus_1_0= '+' ) )?
            int alt2=2;
            int LA2_0 = input.LA(1);

            if ( (LA2_0==14) ) {
                alt2=1;
            }
            switch (alt2) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:194:1: (lv_plus_1_0= '+' )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:194:1: (lv_plus_1_0= '+' )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:195:3: lv_plus_1_0= '+'
                    {
                    lv_plus_1_0=(Token)input.LT(1);
                    match(input,14,FOLLOW_14_in_ruleWMLTag285); 

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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:214:3: ( (lv_name_2_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:215:1: (lv_name_2_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:215:1: (lv_name_2_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:216:3: lv_name_2_0= RULE_ID
            {
            lv_name_2_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag316); 

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

            match(input,15,FOLLOW_15_in_ruleWMLTag331); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:242:1: ( ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_macros_5_0= ruleWMLAbstractMacroCall ) ) | ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) ) | ( (lv_keys_7_0= ruleWMLKey ) ) )*
            loop3:
            do {
                int alt3=5;
                switch ( input.LA(1) ) {
                case 13:
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

                }

                switch (alt3) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:242:2: ( (lv_tags_4_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:242:2: ( (lv_tags_4_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:243:1: (lv_tags_4_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:243:1: (lv_tags_4_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:244:3: lv_tags_4_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTagsWMLTagParserRuleCall_4_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLTag353);
            	    lv_tags_4_0=ruleWMLTag();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"tags",
            	    	        		lv_tags_4_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:267:6: ( (lv_macros_5_0= ruleWMLAbstractMacroCall ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:267:6: ( (lv_macros_5_0= ruleWMLAbstractMacroCall ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:268:1: (lv_macros_5_0= ruleWMLAbstractMacroCall )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:268:1: (lv_macros_5_0= ruleWMLAbstractMacroCall )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:269:3: lv_macros_5_0= ruleWMLAbstractMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getMacrosWMLAbstractMacroCallParserRuleCall_4_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLAbstractMacroCall_in_ruleWMLTag380);
            	    lv_macros_5_0=ruleWMLAbstractMacroCall();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"macros",
            	    	        		lv_macros_5_0, 
            	    	        		"WMLAbstractMacroCall", 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:292:6: ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:292:6: ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:293:1: (lv_macrosDefines_6_0= ruleWMLMacroDefine )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:293:1: (lv_macrosDefines_6_0= ruleWMLMacroDefine )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:294:3: lv_macrosDefines_6_0= ruleWMLMacroDefine
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getMacrosDefinesWMLMacroDefineParserRuleCall_4_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroDefine_in_ruleWMLTag407);
            	    lv_macrosDefines_6_0=ruleWMLMacroDefine();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"macrosDefines",
            	    	        		lv_macrosDefines_6_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:317:6: ( (lv_keys_7_0= ruleWMLKey ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:317:6: ( (lv_keys_7_0= ruleWMLKey ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:318:1: (lv_keys_7_0= ruleWMLKey )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:318:1: (lv_keys_7_0= ruleWMLKey )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:319:3: lv_keys_7_0= ruleWMLKey
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getKeysWMLKeyParserRuleCall_4_3_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLTag434);
            	    lv_keys_7_0=ruleWMLKey();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"keys",
            	    	        		lv_keys_7_0, 
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

            	default :
            	    break loop3;
                }
            } while (true);

            match(input,16,FOLLOW_16_in_ruleWMLTag446); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:345:1: ( (lv_endName_9_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:346:1: (lv_endName_9_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:346:1: (lv_endName_9_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:347:3: lv_endName_9_0= RULE_ID
            {
            lv_endName_9_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag463); 

            			createLeafNode(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0(), "endName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"endName",
            	        		lv_endName_9_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            match(input,15,FOLLOW_15_in_ruleWMLTag478); 

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


    // $ANTLR start entryRuleWMLAbstractMacroCall
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:381:1: entryRuleWMLAbstractMacroCall returns [EObject current=null] : iv_ruleWMLAbstractMacroCall= ruleWMLAbstractMacroCall EOF ;
    public final EObject entryRuleWMLAbstractMacroCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLAbstractMacroCall = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:382:2: (iv_ruleWMLAbstractMacroCall= ruleWMLAbstractMacroCall EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:383:2: iv_ruleWMLAbstractMacroCall= ruleWMLAbstractMacroCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLAbstractMacroCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLAbstractMacroCall_in_entryRuleWMLAbstractMacroCall514);
            iv_ruleWMLAbstractMacroCall=ruleWMLAbstractMacroCall();
            _fsp--;

             current =iv_ruleWMLAbstractMacroCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLAbstractMacroCall524); 

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
    // $ANTLR end entryRuleWMLAbstractMacroCall


    // $ANTLR start ruleWMLAbstractMacroCall
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:390:1: ruleWMLAbstractMacroCall returns [EObject current=null] : (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall ) ;
    public final EObject ruleWMLAbstractMacroCall() throws RecognitionException {
        EObject current = null;

        EObject this_WMLMacroInclude_0 = null;

        EObject this_WMLMacroCall_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:395:6: ( (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:396:1: (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:396:1: (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall )
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( (LA4_0==17) ) {
                int LA4_1 = input.LA(2);

                if ( (LA4_1==RULE_ID) ) {
                    int LA4_2 = input.LA(3);

                    if ( ((LA4_2>=RULE_ID && LA4_2<=RULE_DEFINE)||LA4_2==13||(LA4_2>=17 && LA4_2<=18)||LA4_2==20||LA4_2==25) ) {
                        alt4=2;
                    }
                    else if ( (LA4_2==23) ) {
                        alt4=1;
                    }
                    else {
                        NoViableAltException nvae =
                            new NoViableAltException("396:1: (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall )", 4, 2, input);

                        throw nvae;
                    }
                }
                else if ( (LA4_1==22||LA4_1==27) ) {
                    alt4=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("396:1: (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall )", 4, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("396:1: (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall )", 4, 0, input);

                throw nvae;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:397:5: this_WMLMacroInclude_0= ruleWMLMacroInclude
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLAbstractMacroCallAccess().getWMLMacroIncludeParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroInclude_in_ruleWMLAbstractMacroCall571);
                    this_WMLMacroInclude_0=ruleWMLMacroInclude();
                    _fsp--;

                     
                            current = this_WMLMacroInclude_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:407:5: this_WMLMacroCall_1= ruleWMLMacroCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLAbstractMacroCallAccess().getWMLMacroCallParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLAbstractMacroCall598);
                    this_WMLMacroCall_1=ruleWMLMacroCall();
                    _fsp--;

                     
                            current = this_WMLMacroCall_1; 
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
    // $ANTLR end ruleWMLAbstractMacroCall


    // $ANTLR start entryRuleWMLMacroInclude
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:423:1: entryRuleWMLMacroInclude returns [EObject current=null] : iv_ruleWMLMacroInclude= ruleWMLMacroInclude EOF ;
    public final EObject entryRuleWMLMacroInclude() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroInclude = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:424:2: (iv_ruleWMLMacroInclude= ruleWMLMacroInclude EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:425:2: iv_ruleWMLMacroInclude= ruleWMLMacroInclude EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroIncludeRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroInclude_in_entryRuleWMLMacroInclude633);
            iv_ruleWMLMacroInclude=ruleWMLMacroInclude();
            _fsp--;

             current =iv_ruleWMLMacroInclude; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroInclude643); 

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
    // $ANTLR end entryRuleWMLMacroInclude


    // $ANTLR start ruleWMLMacroInclude
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:432:1: ruleWMLMacroInclude returns [EObject current=null] : ( '{' ( (lv_name_1_0= ruleWMLPath ) ) '}' ) ;
    public final EObject ruleWMLMacroInclude() throws RecognitionException {
        EObject current = null;

        AntlrDatatypeRuleToken lv_name_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:437:6: ( ( '{' ( (lv_name_1_0= ruleWMLPath ) ) '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:438:1: ( '{' ( (lv_name_1_0= ruleWMLPath ) ) '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:438:1: ( '{' ( (lv_name_1_0= ruleWMLPath ) ) '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:438:3: '{' ( (lv_name_1_0= ruleWMLPath ) ) '}'
            {
            match(input,17,FOLLOW_17_in_ruleWMLMacroInclude678); 

                    createLeafNode(grammarAccess.getWMLMacroIncludeAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:442:1: ( (lv_name_1_0= ruleWMLPath ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:443:1: (lv_name_1_0= ruleWMLPath )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:443:1: (lv_name_1_0= ruleWMLPath )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:444:3: lv_name_1_0= ruleWMLPath
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getWMLMacroIncludeAccess().getNameWMLPathParserRuleCall_1_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleWMLPath_in_ruleWMLMacroInclude699);
            lv_name_1_0=ruleWMLPath();
            _fsp--;


            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLMacroIncludeRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"name",
            	        		lv_name_1_0, 
            	        		"WMLPath", 
            	        		currentNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	        currentNode = currentNode.getParent();
            	    

            }


            }

            match(input,18,FOLLOW_18_in_ruleWMLMacroInclude709); 

                    createLeafNode(grammarAccess.getWMLMacroIncludeAccess().getRightCurlyBracketKeyword_2(), null); 
                

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
    // $ANTLR end ruleWMLMacroInclude


    // $ANTLR start entryRuleWMLMacroCall
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:478:1: entryRuleWMLMacroCall returns [EObject current=null] : iv_ruleWMLMacroCall= ruleWMLMacroCall EOF ;
    public final EObject entryRuleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroCall = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:479:2: (iv_ruleWMLMacroCall= ruleWMLMacroCall EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:480:2: iv_ruleWMLMacroCall= ruleWMLMacroCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall745);
            iv_ruleWMLMacroCall=ruleWMLMacroCall();
            _fsp--;

             current =iv_ruleWMLMacroCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCall755); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:487:1: ruleWMLMacroCall returns [EObject current=null] : ( '{' ( (lv_name_1_0= RULE_ID ) ) ( ( (lv_args_2_1= RULE_ID | lv_args_2_2= RULE_STRING | lv_args_2_3= ruleTSTRING | lv_args_2_4= RULE_ANY_OTHER ) ) )* ( ( (lv_params_3_0= ruleWMLMacroCallParameter ) ) | ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_macros_5_0= ruleWMLMacroCall ) ) | ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) ) | ( (lv_keys_7_0= ruleWMLKey ) ) )* '}' ) ;
    public final EObject ruleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        Token lv_name_1_0=null;
        Token lv_args_2_1=null;
        Token lv_args_2_2=null;
        Token lv_args_2_4=null;
        AntlrDatatypeRuleToken lv_args_2_3 = null;

        AntlrDatatypeRuleToken lv_params_3_0 = null;

        EObject lv_tags_4_0 = null;

        EObject lv_macros_5_0 = null;

        EObject lv_macrosDefines_6_0 = null;

        EObject lv_keys_7_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:492:6: ( ( '{' ( (lv_name_1_0= RULE_ID ) ) ( ( (lv_args_2_1= RULE_ID | lv_args_2_2= RULE_STRING | lv_args_2_3= ruleTSTRING | lv_args_2_4= RULE_ANY_OTHER ) ) )* ( ( (lv_params_3_0= ruleWMLMacroCallParameter ) ) | ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_macros_5_0= ruleWMLMacroCall ) ) | ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) ) | ( (lv_keys_7_0= ruleWMLKey ) ) )* '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:493:1: ( '{' ( (lv_name_1_0= RULE_ID ) ) ( ( (lv_args_2_1= RULE_ID | lv_args_2_2= RULE_STRING | lv_args_2_3= ruleTSTRING | lv_args_2_4= RULE_ANY_OTHER ) ) )* ( ( (lv_params_3_0= ruleWMLMacroCallParameter ) ) | ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_macros_5_0= ruleWMLMacroCall ) ) | ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) ) | ( (lv_keys_7_0= ruleWMLKey ) ) )* '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:493:1: ( '{' ( (lv_name_1_0= RULE_ID ) ) ( ( (lv_args_2_1= RULE_ID | lv_args_2_2= RULE_STRING | lv_args_2_3= ruleTSTRING | lv_args_2_4= RULE_ANY_OTHER ) ) )* ( ( (lv_params_3_0= ruleWMLMacroCallParameter ) ) | ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_macros_5_0= ruleWMLMacroCall ) ) | ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) ) | ( (lv_keys_7_0= ruleWMLKey ) ) )* '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:493:3: '{' ( (lv_name_1_0= RULE_ID ) ) ( ( (lv_args_2_1= RULE_ID | lv_args_2_2= RULE_STRING | lv_args_2_3= ruleTSTRING | lv_args_2_4= RULE_ANY_OTHER ) ) )* ( ( (lv_params_3_0= ruleWMLMacroCallParameter ) ) | ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_macros_5_0= ruleWMLMacroCall ) ) | ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) ) | ( (lv_keys_7_0= ruleWMLKey ) ) )* '}'
            {
            match(input,17,FOLLOW_17_in_ruleWMLMacroCall790); 

                    createLeafNode(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:497:1: ( (lv_name_1_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:498:1: (lv_name_1_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:498:1: (lv_name_1_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:499:3: lv_name_1_0= RULE_ID
            {
            lv_name_1_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacroCall807); 

            			createLeafNode(grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_1_0(), "name"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:521:2: ( ( (lv_args_2_1= RULE_ID | lv_args_2_2= RULE_STRING | lv_args_2_3= ruleTSTRING | lv_args_2_4= RULE_ANY_OTHER ) ) )*
            loop6:
            do {
                int alt6=2;
                int LA6_0 = input.LA(1);

                if ( (LA6_0==RULE_ID) ) {
                    int LA6_2 = input.LA(2);

                    if ( ((LA6_2>=RULE_ID && LA6_2<=RULE_DEFINE)||LA6_2==13||(LA6_2>=17 && LA6_2<=18)||LA6_2==20||LA6_2==25) ) {
                        alt6=1;
                    }


                }
                else if ( ((LA6_0>=RULE_STRING && LA6_0<=RULE_ANY_OTHER)||LA6_0==25) ) {
                    alt6=1;
                }


                switch (alt6) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:522:1: ( (lv_args_2_1= RULE_ID | lv_args_2_2= RULE_STRING | lv_args_2_3= ruleTSTRING | lv_args_2_4= RULE_ANY_OTHER ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:522:1: ( (lv_args_2_1= RULE_ID | lv_args_2_2= RULE_STRING | lv_args_2_3= ruleTSTRING | lv_args_2_4= RULE_ANY_OTHER ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:523:1: (lv_args_2_1= RULE_ID | lv_args_2_2= RULE_STRING | lv_args_2_3= ruleTSTRING | lv_args_2_4= RULE_ANY_OTHER )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:523:1: (lv_args_2_1= RULE_ID | lv_args_2_2= RULE_STRING | lv_args_2_3= ruleTSTRING | lv_args_2_4= RULE_ANY_OTHER )
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
            	    case 25:
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
            	            new NoViableAltException("523:1: (lv_args_2_1= RULE_ID | lv_args_2_2= RULE_STRING | lv_args_2_3= ruleTSTRING | lv_args_2_4= RULE_ANY_OTHER )", 5, 0, input);

            	        throw nvae;
            	    }

            	    switch (alt5) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:524:3: lv_args_2_1= RULE_ID
            	            {
            	            lv_args_2_1=(Token)input.LT(1);
            	            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacroCall831); 

            	            			createLeafNode(grammarAccess.getWMLMacroCallAccess().getArgsIDTerminalRuleCall_2_0_0(), "args"); 
            	            		

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"args",
            	            	        		lv_args_2_1, 
            	            	        		"ID", 
            	            	        		lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:545:8: lv_args_2_2= RULE_STRING
            	            {
            	            lv_args_2_2=(Token)input.LT(1);
            	            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLMacroCall851); 

            	            			createLeafNode(grammarAccess.getWMLMacroCallAccess().getArgsSTRINGTerminalRuleCall_2_0_1(), "args"); 
            	            		

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"args",
            	            	        		lv_args_2_2, 
            	            	        		"STRING", 
            	            	        		lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 3 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:566:8: lv_args_2_3= ruleTSTRING
            	            {
            	             
            	            	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getArgsTSTRINGParserRuleCall_2_0_2(), currentNode); 
            	            	    
            	            pushFollow(FOLLOW_ruleTSTRING_in_ruleWMLMacroCall875);
            	            lv_args_2_3=ruleTSTRING();
            	            _fsp--;


            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"args",
            	            	        		lv_args_2_3, 
            	            	        		"TSTRING", 
            	            	        		currentNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	        currentNode = currentNode.getParent();
            	            	    

            	            }
            	            break;
            	        case 4 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:587:8: lv_args_2_4= RULE_ANY_OTHER
            	            {
            	            lv_args_2_4=(Token)input.LT(1);
            	            match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_ruleWMLMacroCall890); 

            	            			createLeafNode(grammarAccess.getWMLMacroCallAccess().getArgsANY_OTHERTerminalRuleCall_2_0_3(), "args"); 
            	            		

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"args",
            	            	        		lv_args_2_4, 
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
            	    break loop6;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:611:3: ( ( (lv_params_3_0= ruleWMLMacroCallParameter ) ) | ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_macros_5_0= ruleWMLMacroCall ) ) | ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) ) | ( (lv_keys_7_0= ruleWMLKey ) ) )*
            loop7:
            do {
                int alt7=6;
                switch ( input.LA(1) ) {
                case 20:
                    {
                    alt7=1;
                    }
                    break;
                case 13:
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

                }

                switch (alt7) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:611:4: ( (lv_params_3_0= ruleWMLMacroCallParameter ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:611:4: ( (lv_params_3_0= ruleWMLMacroCallParameter ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:612:1: (lv_params_3_0= ruleWMLMacroCallParameter )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:612:1: (lv_params_3_0= ruleWMLMacroCallParameter )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:613:3: lv_params_3_0= ruleWMLMacroCallParameter
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getParamsWMLMacroCallParameterParserRuleCall_3_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_ruleWMLMacroCall921);
            	    lv_params_3_0=ruleWMLMacroCallParameter();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"params",
            	    	        		lv_params_3_0, 
            	    	        		"WMLMacroCallParameter", 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:636:6: ( (lv_tags_4_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:636:6: ( (lv_tags_4_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:637:1: (lv_tags_4_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:637:1: (lv_tags_4_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:638:3: lv_tags_4_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getTagsWMLTagParserRuleCall_3_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLMacroCall948);
            	    lv_tags_4_0=ruleWMLTag();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"tags",
            	    	        		lv_tags_4_0, 
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
            	case 3 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:661:6: ( (lv_macros_5_0= ruleWMLMacroCall ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:661:6: ( (lv_macros_5_0= ruleWMLMacroCall ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:662:1: (lv_macros_5_0= ruleWMLMacroCall )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:662:1: (lv_macros_5_0= ruleWMLMacroCall )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:663:3: lv_macros_5_0= ruleWMLMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getMacrosWMLMacroCallParserRuleCall_3_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroCall975);
            	    lv_macros_5_0=ruleWMLMacroCall();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"macros",
            	    	        		lv_macros_5_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:686:6: ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:686:6: ( (lv_macrosDefines_6_0= ruleWMLMacroDefine ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:687:1: (lv_macrosDefines_6_0= ruleWMLMacroDefine )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:687:1: (lv_macrosDefines_6_0= ruleWMLMacroDefine )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:688:3: lv_macrosDefines_6_0= ruleWMLMacroDefine
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getMacrosDefinesWMLMacroDefineParserRuleCall_3_3_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroDefine_in_ruleWMLMacroCall1002);
            	    lv_macrosDefines_6_0=ruleWMLMacroDefine();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"macrosDefines",
            	    	        		lv_macrosDefines_6_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:711:6: ( (lv_keys_7_0= ruleWMLKey ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:711:6: ( (lv_keys_7_0= ruleWMLKey ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:712:1: (lv_keys_7_0= ruleWMLKey )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:712:1: (lv_keys_7_0= ruleWMLKey )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:713:3: lv_keys_7_0= ruleWMLKey
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getKeysWMLKeyParserRuleCall_3_4_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLMacroCall1029);
            	    lv_keys_7_0=ruleWMLKey();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"keys",
            	    	        		lv_keys_7_0, 
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

            	default :
            	    break loop7;
                }
            } while (true);

            match(input,18,FOLLOW_18_in_ruleWMLMacroCall1041); 

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


    // $ANTLR start entryRuleWMLMacroDefine
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:747:1: entryRuleWMLMacroDefine returns [EObject current=null] : iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF ;
    public final EObject entryRuleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroDefine = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:748:2: (iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:749:2: iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroDefineRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1077);
            iv_ruleWMLMacroDefine=ruleWMLMacroDefine();
            _fsp--;

             current =iv_ruleWMLMacroDefine; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroDefine1087); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:756:1: ruleWMLMacroDefine returns [EObject current=null] : ( RULE_DEFINE ( ( (lv_params_1_0= ruleWMLMacroCallParameter ) ) | ( (lv_tags_2_0= ruleWMLTag ) ) | ( (lv_macros_3_0= ruleWMLMacroCall ) ) | ( (lv_macrosDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_keys_5_0= ruleWMLKey ) ) )* RULE_ENDDEFINE ) ;
    public final EObject ruleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        AntlrDatatypeRuleToken lv_params_1_0 = null;

        EObject lv_tags_2_0 = null;

        EObject lv_macros_3_0 = null;

        EObject lv_macrosDefines_4_0 = null;

        EObject lv_keys_5_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:761:6: ( ( RULE_DEFINE ( ( (lv_params_1_0= ruleWMLMacroCallParameter ) ) | ( (lv_tags_2_0= ruleWMLTag ) ) | ( (lv_macros_3_0= ruleWMLMacroCall ) ) | ( (lv_macrosDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_keys_5_0= ruleWMLKey ) ) )* RULE_ENDDEFINE ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:762:1: ( RULE_DEFINE ( ( (lv_params_1_0= ruleWMLMacroCallParameter ) ) | ( (lv_tags_2_0= ruleWMLTag ) ) | ( (lv_macros_3_0= ruleWMLMacroCall ) ) | ( (lv_macrosDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_keys_5_0= ruleWMLKey ) ) )* RULE_ENDDEFINE )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:762:1: ( RULE_DEFINE ( ( (lv_params_1_0= ruleWMLMacroCallParameter ) ) | ( (lv_tags_2_0= ruleWMLTag ) ) | ( (lv_macros_3_0= ruleWMLMacroCall ) ) | ( (lv_macrosDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_keys_5_0= ruleWMLKey ) ) )* RULE_ENDDEFINE )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:762:2: RULE_DEFINE ( ( (lv_params_1_0= ruleWMLMacroCallParameter ) ) | ( (lv_tags_2_0= ruleWMLTag ) ) | ( (lv_macros_3_0= ruleWMLMacroCall ) ) | ( (lv_macrosDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_keys_5_0= ruleWMLKey ) ) )* RULE_ENDDEFINE
            {
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1121); 
             
                createLeafNode(grammarAccess.getWMLMacroDefineAccess().getDEFINETerminalRuleCall_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:766:1: ( ( (lv_params_1_0= ruleWMLMacroCallParameter ) ) | ( (lv_tags_2_0= ruleWMLTag ) ) | ( (lv_macros_3_0= ruleWMLMacroCall ) ) | ( (lv_macrosDefines_4_0= ruleWMLMacroDefine ) ) | ( (lv_keys_5_0= ruleWMLKey ) ) )*
            loop8:
            do {
                int alt8=6;
                switch ( input.LA(1) ) {
                case 20:
                    {
                    alt8=1;
                    }
                    break;
                case 13:
                    {
                    alt8=2;
                    }
                    break;
                case 17:
                    {
                    alt8=3;
                    }
                    break;
                case RULE_DEFINE:
                    {
                    alt8=4;
                    }
                    break;
                case RULE_ID:
                    {
                    alt8=5;
                    }
                    break;

                }

                switch (alt8) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:766:2: ( (lv_params_1_0= ruleWMLMacroCallParameter ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:766:2: ( (lv_params_1_0= ruleWMLMacroCallParameter ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:767:1: (lv_params_1_0= ruleWMLMacroCallParameter )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:767:1: (lv_params_1_0= ruleWMLMacroCallParameter )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:768:3: lv_params_1_0= ruleWMLMacroCallParameter
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getParamsWMLMacroCallParameterParserRuleCall_1_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_ruleWMLMacroDefine1142);
            	    lv_params_1_0=ruleWMLMacroCallParameter();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"params",
            	    	        		lv_params_1_0, 
            	    	        		"WMLMacroCallParameter", 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:791:6: ( (lv_tags_2_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:791:6: ( (lv_tags_2_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:792:1: (lv_tags_2_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:792:1: (lv_tags_2_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:793:3: lv_tags_2_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getTagsWMLTagParserRuleCall_1_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLMacroDefine1169);
            	    lv_tags_2_0=ruleWMLTag();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"tags",
            	    	        		lv_tags_2_0, 
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
            	case 3 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:816:6: ( (lv_macros_3_0= ruleWMLMacroCall ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:816:6: ( (lv_macros_3_0= ruleWMLMacroCall ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:817:1: (lv_macros_3_0= ruleWMLMacroCall )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:817:1: (lv_macros_3_0= ruleWMLMacroCall )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:818:3: lv_macros_3_0= ruleWMLMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getMacrosWMLMacroCallParserRuleCall_1_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroDefine1196);
            	    lv_macros_3_0=ruleWMLMacroCall();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"macros",
            	    	        		lv_macros_3_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:841:6: ( (lv_macrosDefines_4_0= ruleWMLMacroDefine ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:841:6: ( (lv_macrosDefines_4_0= ruleWMLMacroDefine ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:842:1: (lv_macrosDefines_4_0= ruleWMLMacroDefine )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:842:1: (lv_macrosDefines_4_0= ruleWMLMacroDefine )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:843:3: lv_macrosDefines_4_0= ruleWMLMacroDefine
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getMacrosDefinesWMLMacroDefineParserRuleCall_1_3_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroDefine_in_ruleWMLMacroDefine1223);
            	    lv_macrosDefines_4_0=ruleWMLMacroDefine();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"macrosDefines",
            	    	        		lv_macrosDefines_4_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:866:6: ( (lv_keys_5_0= ruleWMLKey ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:866:6: ( (lv_keys_5_0= ruleWMLKey ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:867:1: (lv_keys_5_0= ruleWMLKey )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:867:1: (lv_keys_5_0= ruleWMLKey )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:868:3: lv_keys_5_0= ruleWMLKey
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getKeysWMLKeyParserRuleCall_1_4_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLMacroDefine1250);
            	    lv_keys_5_0=ruleWMLKey();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"keys",
            	    	        		lv_keys_5_0, 
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

            	default :
            	    break loop8;
                }
            } while (true);

            match(input,RULE_ENDDEFINE,FOLLOW_RULE_ENDDEFINE_in_ruleWMLMacroDefine1261); 
             
                createLeafNode(grammarAccess.getWMLMacroDefineAccess().getENDDEFINETerminalRuleCall_2(), null); 
                

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


    // $ANTLR start entryRuleWMLKey
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:904:1: entryRuleWMLKey returns [EObject current=null] : iv_ruleWMLKey= ruleWMLKey EOF ;
    public final EObject entryRuleWMLKey() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKey = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:905:2: (iv_ruleWMLKey= ruleWMLKey EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:906:2: iv_ruleWMLKey= ruleWMLKey EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey1298);
            iv_ruleWMLKey=ruleWMLKey();
            _fsp--;

             current =iv_ruleWMLKey; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey1308); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:913:1: ruleWMLKey returns [EObject current=null] : ( ( (lv_name_0_0= ruleWMLIDList ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ( '+' ( (lv_extraArgs_4_0= ruleWMLKeyExtraArgs ) ) )* ) ;
    public final EObject ruleWMLKey() throws RecognitionException {
        EObject current = null;

        AntlrDatatypeRuleToken lv_name_0_0 = null;

        EObject lv_value_2_0 = null;

        EObject lv_extraArgs_4_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:918:6: ( ( ( (lv_name_0_0= ruleWMLIDList ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ( '+' ( (lv_extraArgs_4_0= ruleWMLKeyExtraArgs ) ) )* ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:919:1: ( ( (lv_name_0_0= ruleWMLIDList ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ( '+' ( (lv_extraArgs_4_0= ruleWMLKeyExtraArgs ) ) )* )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:919:1: ( ( (lv_name_0_0= ruleWMLIDList ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ( '+' ( (lv_extraArgs_4_0= ruleWMLKeyExtraArgs ) ) )* )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:919:2: ( (lv_name_0_0= ruleWMLIDList ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ( '+' ( (lv_extraArgs_4_0= ruleWMLKeyExtraArgs ) ) )*
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:919:2: ( (lv_name_0_0= ruleWMLIDList ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:920:1: (lv_name_0_0= ruleWMLIDList )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:920:1: (lv_name_0_0= ruleWMLIDList )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:921:3: lv_name_0_0= ruleWMLIDList
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getNameWMLIDListParserRuleCall_0_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleWMLIDList_in_ruleWMLKey1354);
            lv_name_0_0=ruleWMLIDList();
            _fsp--;


            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"name",
            	        		lv_name_0_0, 
            	        		"WMLIDList", 
            	        		currentNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	        currentNode = currentNode.getParent();
            	    

            }


            }

            match(input,19,FOLLOW_19_in_ruleWMLKey1364); 

                    createLeafNode(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:947:1: ( (lv_value_2_0= ruleWMLKeyValue ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:948:1: (lv_value_2_0= ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:948:1: (lv_value_2_0= ruleWMLKeyValue )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:949:3: lv_value_2_0= ruleWMLKeyValue
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleWMLKeyValue_in_ruleWMLKey1385);
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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:971:2: ( '+' ( (lv_extraArgs_4_0= ruleWMLKeyExtraArgs ) ) )*
            loop9:
            do {
                int alt9=2;
                int LA9_0 = input.LA(1);

                if ( (LA9_0==14) ) {
                    alt9=1;
                }


                switch (alt9) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:971:4: '+' ( (lv_extraArgs_4_0= ruleWMLKeyExtraArgs ) )
            	    {
            	    match(input,14,FOLLOW_14_in_ruleWMLKey1396); 

            	            createLeafNode(grammarAccess.getWMLKeyAccess().getPlusSignKeyword_3_0(), null); 
            	        
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:975:1: ( (lv_extraArgs_4_0= ruleWMLKeyExtraArgs ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:976:1: (lv_extraArgs_4_0= ruleWMLKeyExtraArgs )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:976:1: (lv_extraArgs_4_0= ruleWMLKeyExtraArgs )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:977:3: lv_extraArgs_4_0= ruleWMLKeyExtraArgs
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getExtraArgsWMLKeyExtraArgsParserRuleCall_3_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKeyExtraArgs_in_ruleWMLKey1417);
            	    lv_extraArgs_4_0=ruleWMLKeyExtraArgs();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"extraArgs",
            	    	        		lv_extraArgs_4_0, 
            	    	        		"WMLKeyExtraArgs", 
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
            	    break loop9;
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
    // $ANTLR end ruleWMLKey


    // $ANTLR start entryRuleWMLKeyExtraArgs
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1007:1: entryRuleWMLKeyExtraArgs returns [EObject current=null] : iv_ruleWMLKeyExtraArgs= ruleWMLKeyExtraArgs EOF ;
    public final EObject entryRuleWMLKeyExtraArgs() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKeyExtraArgs = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1008:2: (iv_ruleWMLKeyExtraArgs= ruleWMLKeyExtraArgs EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1009:2: iv_ruleWMLKeyExtraArgs= ruleWMLKeyExtraArgs EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyExtraArgsRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKeyExtraArgs_in_entryRuleWMLKeyExtraArgs1455);
            iv_ruleWMLKeyExtraArgs=ruleWMLKeyExtraArgs();
            _fsp--;

             current =iv_ruleWMLKeyExtraArgs; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyExtraArgs1465); 

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
    // $ANTLR end entryRuleWMLKeyExtraArgs


    // $ANTLR start ruleWMLKeyExtraArgs
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1016:1: ruleWMLKeyExtraArgs returns [EObject current=null] : (this_WMLMacroCall_0= ruleWMLMacroCall | RULE_STRING | ruleTSTRING ) ;
    public final EObject ruleWMLKeyExtraArgs() throws RecognitionException {
        EObject current = null;

        EObject this_WMLMacroCall_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1021:6: ( (this_WMLMacroCall_0= ruleWMLMacroCall | RULE_STRING | ruleTSTRING ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1022:1: (this_WMLMacroCall_0= ruleWMLMacroCall | RULE_STRING | ruleTSTRING )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1022:1: (this_WMLMacroCall_0= ruleWMLMacroCall | RULE_STRING | ruleTSTRING )
            int alt10=3;
            switch ( input.LA(1) ) {
            case 17:
                {
                alt10=1;
                }
                break;
            case RULE_STRING:
                {
                alt10=2;
                }
                break;
            case 25:
                {
                alt10=3;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("1022:1: (this_WMLMacroCall_0= ruleWMLMacroCall | RULE_STRING | ruleTSTRING )", 10, 0, input);

                throw nvae;
            }

            switch (alt10) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1023:5: this_WMLMacroCall_0= ruleWMLMacroCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyExtraArgsAccess().getWMLMacroCallParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyExtraArgs1512);
                    this_WMLMacroCall_0=ruleWMLMacroCall();
                    _fsp--;

                     
                            current = this_WMLMacroCall_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1032:6: RULE_STRING
                    {
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLKeyExtraArgs1526); 
                     
                        createLeafNode(grammarAccess.getWMLKeyExtraArgsAccess().getSTRINGTerminalRuleCall_1(), null); 
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1037:6: ruleTSTRING
                    {
                    pushFollow(FOLLOW_ruleTSTRING_in_ruleWMLKeyExtraArgs1540);
                    ruleTSTRING();
                    _fsp--;


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
    // $ANTLR end ruleWMLKeyExtraArgs


    // $ANTLR start entryRuleWMLMacroCallParameter
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1045:1: entryRuleWMLMacroCallParameter returns [String current=null] : iv_ruleWMLMacroCallParameter= ruleWMLMacroCallParameter EOF ;
    public final String entryRuleWMLMacroCallParameter() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleWMLMacroCallParameter = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1046:2: (iv_ruleWMLMacroCallParameter= ruleWMLMacroCallParameter EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1047:2: iv_ruleWMLMacroCallParameter= ruleWMLMacroCallParameter EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroCallParameterRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_entryRuleWMLMacroCallParameter1569);
            iv_ruleWMLMacroCallParameter=ruleWMLMacroCallParameter();
            _fsp--;

             current =iv_ruleWMLMacroCallParameter.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCallParameter1580); 

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
    // $ANTLR end entryRuleWMLMacroCallParameter


    // $ANTLR start ruleWMLMacroCallParameter
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1054:1: ruleWMLMacroCallParameter returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (kw= '(' (this_ID_1= RULE_ID | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_FILE_4= ruleFILE ) kw= ')' ) ;
    public final AntlrDatatypeRuleToken ruleWMLMacroCallParameter() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        Token this_ID_1=null;
        Token this_STRING_2=null;
        AntlrDatatypeRuleToken this_TSTRING_3 = null;

        AntlrDatatypeRuleToken this_FILE_4 = null;


         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1059:6: ( (kw= '(' (this_ID_1= RULE_ID | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_FILE_4= ruleFILE ) kw= ')' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1060:1: (kw= '(' (this_ID_1= RULE_ID | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_FILE_4= ruleFILE ) kw= ')' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1060:1: (kw= '(' (this_ID_1= RULE_ID | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_FILE_4= ruleFILE ) kw= ')' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1061:2: kw= '(' (this_ID_1= RULE_ID | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_FILE_4= ruleFILE ) kw= ')'
            {
            kw=(Token)input.LT(1);
            match(input,20,FOLLOW_20_in_ruleWMLMacroCallParameter1618); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getWMLMacroCallParameterAccess().getLeftParenthesisKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1066:1: (this_ID_1= RULE_ID | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_FILE_4= ruleFILE )
            int alt11=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                int LA11_1 = input.LA(2);

                if ( (LA11_1==RULE_ID||(LA11_1>=26 && LA11_1<=27)) ) {
                    alt11=4;
                }
                else if ( (LA11_1==21) ) {
                    alt11=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("1066:1: (this_ID_1= RULE_ID | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_FILE_4= ruleFILE )", 11, 1, input);

                    throw nvae;
                }
                }
                break;
            case RULE_STRING:
                {
                alt11=2;
                }
                break;
            case 25:
                {
                alt11=3;
                }
                break;
            case 27:
                {
                alt11=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("1066:1: (this_ID_1= RULE_ID | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_FILE_4= ruleFILE )", 11, 0, input);

                throw nvae;
            }

            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1066:6: this_ID_1= RULE_ID
                    {
                    this_ID_1=(Token)input.LT(1);
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacroCallParameter1634); 

                    		current.merge(this_ID_1);
                        
                     
                        createLeafNode(grammarAccess.getWMLMacroCallParameterAccess().getIDTerminalRuleCall_1_0(), null); 
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1074:10: this_STRING_2= RULE_STRING
                    {
                    this_STRING_2=(Token)input.LT(1);
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLMacroCallParameter1660); 

                    		current.merge(this_STRING_2);
                        
                     
                        createLeafNode(grammarAccess.getWMLMacroCallParameterAccess().getSTRINGTerminalRuleCall_1_1(), null); 
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1083:5: this_TSTRING_3= ruleTSTRING
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLMacroCallParameterAccess().getTSTRINGParserRuleCall_1_2(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleTSTRING_in_ruleWMLMacroCallParameter1693);
                    this_TSTRING_3=ruleTSTRING();
                    _fsp--;


                    		current.merge(this_TSTRING_3);
                        
                     
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1095:5: this_FILE_4= ruleFILE
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLMacroCallParameterAccess().getFILEParserRuleCall_1_3(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleFILE_in_ruleWMLMacroCallParameter1726);
                    this_FILE_4=ruleFILE();
                    _fsp--;


                    		current.merge(this_FILE_4);
                        
                     
                            currentNode = currentNode.getParent();
                        

                    }
                    break;

            }

            kw=(Token)input.LT(1);
            match(input,21,FOLLOW_21_in_ruleWMLMacroCallParameter1745); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getWMLMacroCallParameterAccess().getRightParenthesisKeyword_2(), null); 
                

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
    // $ANTLR end ruleWMLMacroCallParameter


    // $ANTLR start entryRuleWMLKeyValue
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1119:1: entryRuleWMLKeyValue returns [EObject current=null] : iv_ruleWMLKeyValue= ruleWMLKeyValue EOF ;
    public final EObject entryRuleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKeyValue = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1120:2: (iv_ruleWMLKeyValue= ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1121:2: iv_ruleWMLKeyValue= ruleWMLKeyValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue1785);
            iv_ruleWMLKeyValue=ruleWMLKeyValue();
            _fsp--;

             current =iv_ruleWMLKeyValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue1795); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1128:1: ruleWMLKeyValue returns [EObject current=null] : ( ruleWMLINTList | ruleWMLIDList | this_WMLMacroCall_2= ruleWMLMacroCall | RULE_STRING | ruleTSTRING | ruleWMLPath | ruleFILE ) ;
    public final EObject ruleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject this_WMLMacroCall_2 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1133:6: ( ( ruleWMLINTList | ruleWMLIDList | this_WMLMacroCall_2= ruleWMLMacroCall | RULE_STRING | ruleTSTRING | ruleWMLPath | ruleFILE ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1134:1: ( ruleWMLINTList | ruleWMLIDList | this_WMLMacroCall_2= ruleWMLMacroCall | RULE_STRING | ruleTSTRING | ruleWMLPath | ruleFILE )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1134:1: ( ruleWMLINTList | ruleWMLIDList | this_WMLMacroCall_2= ruleWMLMacroCall | RULE_STRING | ruleTSTRING | ruleWMLPath | ruleFILE )
            int alt12=7;
            switch ( input.LA(1) ) {
            case RULE_INT:
                {
                alt12=1;
                }
                break;
            case RULE_ID:
                {
                switch ( input.LA(2) ) {
                case 23:
                    {
                    alt12=6;
                    }
                    break;
                case EOF:
                case RULE_DEFINE:
                case RULE_ENDDEFINE:
                case 13:
                case 14:
                case 16:
                case 17:
                case 18:
                case 20:
                case 24:
                    {
                    alt12=2;
                    }
                    break;
                case RULE_ID:
                    {
                    int LA12_9 = input.LA(3);

                    if ( (LA12_9==19||LA12_9==24) ) {
                        alt12=2;
                    }
                    else if ( (LA12_9==RULE_ID||(LA12_9>=26 && LA12_9<=27)) ) {
                        alt12=7;
                    }
                    else {
                        NoViableAltException nvae =
                            new NoViableAltException("1134:1: ( ruleWMLINTList | ruleWMLIDList | this_WMLMacroCall_2= ruleWMLMacroCall | RULE_STRING | ruleTSTRING | ruleWMLPath | ruleFILE )", 12, 9, input);

                        throw nvae;
                    }
                    }
                    break;
                case 26:
                case 27:
                    {
                    alt12=7;
                    }
                    break;
                default:
                    NoViableAltException nvae =
                        new NoViableAltException("1134:1: ( ruleWMLINTList | ruleWMLIDList | this_WMLMacroCall_2= ruleWMLMacroCall | RULE_STRING | ruleTSTRING | ruleWMLPath | ruleFILE )", 12, 2, input);

                    throw nvae;
                }

                }
                break;
            case 17:
                {
                alt12=3;
                }
                break;
            case RULE_STRING:
                {
                alt12=4;
                }
                break;
            case 25:
                {
                alt12=5;
                }
                break;
            case 22:
                {
                alt12=6;
                }
                break;
            case 27:
                {
                int LA12_7 = input.LA(2);

                if ( (LA12_7==RULE_ID||(LA12_7>=26 && LA12_7<=27)) ) {
                    alt12=7;
                }
                else if ( (LA12_7==23) ) {
                    alt12=6;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("1134:1: ( ruleWMLINTList | ruleWMLIDList | this_WMLMacroCall_2= ruleWMLMacroCall | RULE_STRING | ruleTSTRING | ruleWMLPath | ruleFILE )", 12, 7, input);

                    throw nvae;
                }
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("1134:1: ( ruleWMLINTList | ruleWMLIDList | this_WMLMacroCall_2= ruleWMLMacroCall | RULE_STRING | ruleTSTRING | ruleWMLPath | ruleFILE )", 12, 0, input);

                throw nvae;
            }

            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1134:2: ruleWMLINTList
                    {
                    pushFollow(FOLLOW_ruleWMLINTList_in_ruleWMLKeyValue1829);
                    ruleWMLINTList();
                    _fsp--;


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1135:6: ruleWMLIDList
                    {
                    pushFollow(FOLLOW_ruleWMLIDList_in_ruleWMLKeyValue1836);
                    ruleWMLIDList();
                    _fsp--;


                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1137:5: this_WMLMacroCall_2= ruleWMLMacroCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_2(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyValue1856);
                    this_WMLMacroCall_2=ruleWMLMacroCall();
                    _fsp--;

                     
                            current = this_WMLMacroCall_2; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1146:6: RULE_STRING
                    {
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLKeyValue1870); 
                     
                        createLeafNode(grammarAccess.getWMLKeyValueAccess().getSTRINGTerminalRuleCall_3(), null); 
                        

                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1151:6: ruleTSTRING
                    {
                    pushFollow(FOLLOW_ruleTSTRING_in_ruleWMLKeyValue1884);
                    ruleTSTRING();
                    _fsp--;


                    }
                    break;
                case 6 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1152:6: ruleWMLPath
                    {
                    pushFollow(FOLLOW_ruleWMLPath_in_ruleWMLKeyValue1891);
                    ruleWMLPath();
                    _fsp--;


                    }
                    break;
                case 7 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1153:6: ruleFILE
                    {
                    pushFollow(FOLLOW_ruleFILE_in_ruleWMLKeyValue1898);
                    ruleFILE();
                    _fsp--;


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


    // $ANTLR start entryRuleWMLPath
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1161:1: entryRuleWMLPath returns [String current=null] : iv_ruleWMLPath= ruleWMLPath EOF ;
    public final String entryRuleWMLPath() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleWMLPath = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1162:2: (iv_ruleWMLPath= ruleWMLPath EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1163:2: iv_ruleWMLPath= ruleWMLPath EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLPathRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLPath_in_entryRuleWMLPath1927);
            iv_ruleWMLPath=ruleWMLPath();
            _fsp--;

             current =iv_ruleWMLPath.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLPath1938); 

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
    // $ANTLR end entryRuleWMLPath


    // $ANTLR start ruleWMLPath
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1170:1: ruleWMLPath returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( (kw= '~' )? this_PATH_ID_1= rulePATH_ID (kw= '/' this_PATH_ID_3= rulePATH_ID )+ (this_FILE_4= ruleFILE )? (kw= '/' )? ) ;
    public final AntlrDatatypeRuleToken ruleWMLPath() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        AntlrDatatypeRuleToken this_PATH_ID_1 = null;

        AntlrDatatypeRuleToken this_PATH_ID_3 = null;

        AntlrDatatypeRuleToken this_FILE_4 = null;


         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1175:6: ( ( (kw= '~' )? this_PATH_ID_1= rulePATH_ID (kw= '/' this_PATH_ID_3= rulePATH_ID )+ (this_FILE_4= ruleFILE )? (kw= '/' )? ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1176:1: ( (kw= '~' )? this_PATH_ID_1= rulePATH_ID (kw= '/' this_PATH_ID_3= rulePATH_ID )+ (this_FILE_4= ruleFILE )? (kw= '/' )? )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1176:1: ( (kw= '~' )? this_PATH_ID_1= rulePATH_ID (kw= '/' this_PATH_ID_3= rulePATH_ID )+ (this_FILE_4= ruleFILE )? (kw= '/' )? )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1176:2: (kw= '~' )? this_PATH_ID_1= rulePATH_ID (kw= '/' this_PATH_ID_3= rulePATH_ID )+ (this_FILE_4= ruleFILE )? (kw= '/' )?
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1176:2: (kw= '~' )?
            int alt13=2;
            int LA13_0 = input.LA(1);

            if ( (LA13_0==22) ) {
                alt13=1;
            }
            switch (alt13) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1177:2: kw= '~'
                    {
                    kw=(Token)input.LT(1);
                    match(input,22,FOLLOW_22_in_ruleWMLPath1977); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLPathAccess().getTildeKeyword_0(), null); 
                        

                    }
                    break;

            }

             
                    currentNode=createCompositeNode(grammarAccess.getWMLPathAccess().getPATH_IDParserRuleCall_1(), currentNode); 
                
            pushFollow(FOLLOW_rulePATH_ID_in_ruleWMLPath2001);
            this_PATH_ID_1=rulePATH_ID();
            _fsp--;


            		current.merge(this_PATH_ID_1);
                
             
                    currentNode = currentNode.getParent();
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1193:1: (kw= '/' this_PATH_ID_3= rulePATH_ID )+
            int cnt14=0;
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( (LA14_0==23) ) {
                    int LA14_2 = input.LA(2);

                    if ( (LA14_2==RULE_ID) ) {
                        int LA14_3 = input.LA(3);

                        if ( (LA14_3==EOF||LA14_3==RULE_ID||(LA14_3>=RULE_DEFINE && LA14_3<=RULE_ENDDEFINE)||(LA14_3>=13 && LA14_3<=14)||(LA14_3>=16 && LA14_3<=18)||LA14_3==20||LA14_3==23||LA14_3==27) ) {
                            alt14=1;
                        }


                    }
                    else if ( (LA14_2==27) ) {
                        alt14=1;
                    }


                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1194:2: kw= '/' this_PATH_ID_3= rulePATH_ID
            	    {
            	    kw=(Token)input.LT(1);
            	    match(input,23,FOLLOW_23_in_ruleWMLPath2020); 

            	            current.merge(kw);
            	            createLeafNode(grammarAccess.getWMLPathAccess().getSolidusKeyword_2_0(), null); 
            	        
            	     
            	            currentNode=createCompositeNode(grammarAccess.getWMLPathAccess().getPATH_IDParserRuleCall_2_1(), currentNode); 
            	        
            	    pushFollow(FOLLOW_rulePATH_ID_in_ruleWMLPath2042);
            	    this_PATH_ID_3=rulePATH_ID();
            	    _fsp--;


            	    		current.merge(this_PATH_ID_3);
            	        
            	     
            	            currentNode = currentNode.getParent();
            	        

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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1210:3: (this_FILE_4= ruleFILE )?
            int alt15=2;
            int LA15_0 = input.LA(1);

            if ( (LA15_0==RULE_ID) ) {
                int LA15_1 = input.LA(2);

                if ( (LA15_1==RULE_ID||(LA15_1>=26 && LA15_1<=27)) ) {
                    alt15=1;
                }
            }
            else if ( (LA15_0==27) ) {
                alt15=1;
            }
            switch (alt15) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1211:5: this_FILE_4= ruleFILE
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLPathAccess().getFILEParserRuleCall_3(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleFILE_in_ruleWMLPath2072);
                    this_FILE_4=ruleFILE();
                    _fsp--;


                    		current.merge(this_FILE_4);
                        
                     
                            currentNode = currentNode.getParent();
                        

                    }
                    break;

            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1221:3: (kw= '/' )?
            int alt16=2;
            int LA16_0 = input.LA(1);

            if ( (LA16_0==23) ) {
                alt16=1;
            }
            switch (alt16) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1222:2: kw= '/'
                    {
                    kw=(Token)input.LT(1);
                    match(input,23,FOLLOW_23_in_ruleWMLPath2093); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLPathAccess().getSolidusKeyword_4(), null); 
                        

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
    // $ANTLR end ruleWMLPath


    // $ANTLR start entryRuleWMLIDList
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1235:1: entryRuleWMLIDList returns [String current=null] : iv_ruleWMLIDList= ruleWMLIDList EOF ;
    public final String entryRuleWMLIDList() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleWMLIDList = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1236:2: (iv_ruleWMLIDList= ruleWMLIDList EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1237:2: iv_ruleWMLIDList= ruleWMLIDList EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLIDListRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLIDList_in_entryRuleWMLIDList2136);
            iv_ruleWMLIDList=ruleWMLIDList();
            _fsp--;

             current =iv_ruleWMLIDList.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLIDList2147); 

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
    // $ANTLR end entryRuleWMLIDList


    // $ANTLR start ruleWMLIDList
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1244:1: ruleWMLIDList returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )* ) ;
    public final AntlrDatatypeRuleToken ruleWMLIDList() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;
        Token kw=null;
        Token this_ID_2=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1249:6: ( (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )* ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1250:1: (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )* )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1250:1: (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )* )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1250:6: this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )*
            {
            this_ID_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLIDList2187); 

            		current.merge(this_ID_0);
                
             
                createLeafNode(grammarAccess.getWMLIDListAccess().getIDTerminalRuleCall_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1257:1: (kw= ',' this_ID_2= RULE_ID )*
            loop17:
            do {
                int alt17=2;
                int LA17_0 = input.LA(1);

                if ( (LA17_0==24) ) {
                    alt17=1;
                }


                switch (alt17) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1258:2: kw= ',' this_ID_2= RULE_ID
            	    {
            	    kw=(Token)input.LT(1);
            	    match(input,24,FOLLOW_24_in_ruleWMLIDList2206); 

            	            current.merge(kw);
            	            createLeafNode(grammarAccess.getWMLIDListAccess().getCommaKeyword_1_0(), null); 
            	        
            	    this_ID_2=(Token)input.LT(1);
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLIDList2221); 

            	    		current.merge(this_ID_2);
            	        
            	     
            	        createLeafNode(grammarAccess.getWMLIDListAccess().getIDTerminalRuleCall_1_1(), null); 
            	        

            	    }
            	    break;

            	default :
            	    break loop17;
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
    // $ANTLR end ruleWMLIDList


    // $ANTLR start entryRuleWMLINTList
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1278:1: entryRuleWMLINTList returns [String current=null] : iv_ruleWMLINTList= ruleWMLINTList EOF ;
    public final String entryRuleWMLINTList() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleWMLINTList = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1279:2: (iv_ruleWMLINTList= ruleWMLINTList EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1280:2: iv_ruleWMLINTList= ruleWMLINTList EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLINTListRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLINTList_in_entryRuleWMLINTList2269);
            iv_ruleWMLINTList=ruleWMLINTList();
            _fsp--;

             current =iv_ruleWMLINTList.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLINTList2280); 

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
    // $ANTLR end entryRuleWMLINTList


    // $ANTLR start ruleWMLINTList
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1287:1: ruleWMLINTList returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_INT_0= RULE_INT (kw= ',' this_INT_2= RULE_INT )* ) ;
    public final AntlrDatatypeRuleToken ruleWMLINTList() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_INT_0=null;
        Token kw=null;
        Token this_INT_2=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1292:6: ( (this_INT_0= RULE_INT (kw= ',' this_INT_2= RULE_INT )* ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1293:1: (this_INT_0= RULE_INT (kw= ',' this_INT_2= RULE_INT )* )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1293:1: (this_INT_0= RULE_INT (kw= ',' this_INT_2= RULE_INT )* )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1293:6: this_INT_0= RULE_INT (kw= ',' this_INT_2= RULE_INT )*
            {
            this_INT_0=(Token)input.LT(1);
            match(input,RULE_INT,FOLLOW_RULE_INT_in_ruleWMLINTList2320); 

            		current.merge(this_INT_0);
                
             
                createLeafNode(grammarAccess.getWMLINTListAccess().getINTTerminalRuleCall_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1300:1: (kw= ',' this_INT_2= RULE_INT )*
            loop18:
            do {
                int alt18=2;
                int LA18_0 = input.LA(1);

                if ( (LA18_0==24) ) {
                    alt18=1;
                }


                switch (alt18) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1301:2: kw= ',' this_INT_2= RULE_INT
            	    {
            	    kw=(Token)input.LT(1);
            	    match(input,24,FOLLOW_24_in_ruleWMLINTList2339); 

            	            current.merge(kw);
            	            createLeafNode(grammarAccess.getWMLINTListAccess().getCommaKeyword_1_0(), null); 
            	        
            	    this_INT_2=(Token)input.LT(1);
            	    match(input,RULE_INT,FOLLOW_RULE_INT_in_ruleWMLINTList2354); 

            	    		current.merge(this_INT_2);
            	        
            	     
            	        createLeafNode(grammarAccess.getWMLINTListAccess().getINTTerminalRuleCall_1_1(), null); 
            	        

            	    }
            	    break;

            	default :
            	    break loop18;
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
    // $ANTLR end ruleWMLINTList


    // $ANTLR start entryRuleTSTRING
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1321:1: entryRuleTSTRING returns [String current=null] : iv_ruleTSTRING= ruleTSTRING EOF ;
    public final String entryRuleTSTRING() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleTSTRING = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1322:2: (iv_ruleTSTRING= ruleTSTRING EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1323:2: iv_ruleTSTRING= ruleTSTRING EOF
            {
             currentNode = createCompositeNode(grammarAccess.getTSTRINGRule(), currentNode); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING2402);
            iv_ruleTSTRING=ruleTSTRING();
            _fsp--;

             current =iv_ruleTSTRING.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING2413); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1330:1: ruleTSTRING returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (kw= '_' this_STRING_1= RULE_STRING ) ;
    public final AntlrDatatypeRuleToken ruleTSTRING() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        Token this_STRING_1=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1335:6: ( (kw= '_' this_STRING_1= RULE_STRING ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1336:1: (kw= '_' this_STRING_1= RULE_STRING )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1336:1: (kw= '_' this_STRING_1= RULE_STRING )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1337:2: kw= '_' this_STRING_1= RULE_STRING
            {
            kw=(Token)input.LT(1);
            match(input,25,FOLLOW_25_in_ruleTSTRING2451); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getTSTRINGAccess().get_Keyword_0(), null); 
                
            this_STRING_1=(Token)input.LT(1);
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleTSTRING2466); 

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


    // $ANTLR start entryRuleFILE
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:1: entryRuleFILE returns [String current=null] : iv_ruleFILE= ruleFILE EOF ;
    public final String entryRuleFILE() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleFILE = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1358:2: (iv_ruleFILE= ruleFILE EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:2: iv_ruleFILE= ruleFILE EOF
            {
             currentNode = createCompositeNode(grammarAccess.getFILERule(), currentNode); 
            pushFollow(FOLLOW_ruleFILE_in_entryRuleFILE2512);
            iv_ruleFILE=ruleFILE();
            _fsp--;

             current =iv_ruleFILE.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleFILE2523); 

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
    // $ANTLR end entryRuleFILE


    // $ANTLR start ruleFILE
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1366:1: ruleFILE returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( (this_PATH_ID_0= rulePATH_ID )+ kw= '.' this_ID_2= RULE_ID ) ;
    public final AntlrDatatypeRuleToken ruleFILE() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        Token this_ID_2=null;
        AntlrDatatypeRuleToken this_PATH_ID_0 = null;


         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1371:6: ( ( (this_PATH_ID_0= rulePATH_ID )+ kw= '.' this_ID_2= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1372:1: ( (this_PATH_ID_0= rulePATH_ID )+ kw= '.' this_ID_2= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1372:1: ( (this_PATH_ID_0= rulePATH_ID )+ kw= '.' this_ID_2= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1372:2: (this_PATH_ID_0= rulePATH_ID )+ kw= '.' this_ID_2= RULE_ID
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1372:2: (this_PATH_ID_0= rulePATH_ID )+
            int cnt19=0;
            loop19:
            do {
                int alt19=2;
                int LA19_0 = input.LA(1);

                if ( (LA19_0==RULE_ID||LA19_0==27) ) {
                    alt19=1;
                }


                switch (alt19) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1373:5: this_PATH_ID_0= rulePATH_ID
            	    {
            	     
            	            currentNode=createCompositeNode(grammarAccess.getFILEAccess().getPATH_IDParserRuleCall_0(), currentNode); 
            	        
            	    pushFollow(FOLLOW_rulePATH_ID_in_ruleFILE2571);
            	    this_PATH_ID_0=rulePATH_ID();
            	    _fsp--;


            	    		current.merge(this_PATH_ID_0);
            	        
            	     
            	            currentNode = currentNode.getParent();
            	        

            	    }
            	    break;

            	default :
            	    if ( cnt19 >= 1 ) break loop19;
                        EarlyExitException eee =
                            new EarlyExitException(19, input);
                        throw eee;
                }
                cnt19++;
            } while (true);

            kw=(Token)input.LT(1);
            match(input,26,FOLLOW_26_in_ruleFILE2591); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getFILEAccess().getFullStopKeyword_1(), null); 
                
            this_ID_2=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleFILE2606); 

            		current.merge(this_ID_2);
                
             
                createLeafNode(grammarAccess.getFILEAccess().getIDTerminalRuleCall_2(), null); 
                

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
    // $ANTLR end ruleFILE


    // $ANTLR start entryRulePATH_ID
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1404:1: entryRulePATH_ID returns [String current=null] : iv_rulePATH_ID= rulePATH_ID EOF ;
    public final String entryRulePATH_ID() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_rulePATH_ID = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1405:2: (iv_rulePATH_ID= rulePATH_ID EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1406:2: iv_rulePATH_ID= rulePATH_ID EOF
            {
             currentNode = createCompositeNode(grammarAccess.getPATH_IDRule(), currentNode); 
            pushFollow(FOLLOW_rulePATH_ID_in_entryRulePATH_ID2652);
            iv_rulePATH_ID=rulePATH_ID();
            _fsp--;

             current =iv_rulePATH_ID.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePATH_ID2663); 

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
    // $ANTLR end entryRulePATH_ID


    // $ANTLR start rulePATH_ID
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1413:1: rulePATH_ID returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_ID_0= RULE_ID | kw= '-' ) ;
    public final AntlrDatatypeRuleToken rulePATH_ID() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;
        Token kw=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1418:6: ( (this_ID_0= RULE_ID | kw= '-' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1419:1: (this_ID_0= RULE_ID | kw= '-' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1419:1: (this_ID_0= RULE_ID | kw= '-' )
            int alt20=2;
            int LA20_0 = input.LA(1);

            if ( (LA20_0==RULE_ID) ) {
                alt20=1;
            }
            else if ( (LA20_0==27) ) {
                alt20=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("1419:1: (this_ID_0= RULE_ID | kw= '-' )", 20, 0, input);

                throw nvae;
            }
            switch (alt20) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1419:6: this_ID_0= RULE_ID
                    {
                    this_ID_0=(Token)input.LT(1);
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_rulePATH_ID2703); 

                    		current.merge(this_ID_0);
                        
                     
                        createLeafNode(grammarAccess.getPATH_IDAccess().getIDTerminalRuleCall_0(), null); 
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1428:2: kw= '-'
                    {
                    kw=(Token)input.LT(1);
                    match(input,27,FOLLOW_27_in_rulePATH_ID2727); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getPATH_IDAccess().getHyphenMinusKeyword_1(), null); 
                        

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
    // $ANTLR end rulePATH_ID


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot75 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot85 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLRoot131 = new BitSet(new long[]{0x0000000000022082L});
    public static final BitSet FOLLOW_ruleWMLAbstractMacroCall_in_ruleWMLRoot158 = new BitSet(new long[]{0x0000000000022082L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLRoot185 = new BitSet(new long[]{0x0000000000022082L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag222 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag232 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_13_in_ruleWMLTag267 = new BitSet(new long[]{0x0000000000004010L});
    public static final BitSet FOLLOW_14_in_ruleWMLTag285 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag316 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_15_in_ruleWMLTag331 = new BitSet(new long[]{0x0000000000032090L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLTag353 = new BitSet(new long[]{0x0000000000032090L});
    public static final BitSet FOLLOW_ruleWMLAbstractMacroCall_in_ruleWMLTag380 = new BitSet(new long[]{0x0000000000032090L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLTag407 = new BitSet(new long[]{0x0000000000032090L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLTag434 = new BitSet(new long[]{0x0000000000032090L});
    public static final BitSet FOLLOW_16_in_ruleWMLTag446 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag463 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_15_in_ruleWMLTag478 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLAbstractMacroCall_in_entryRuleWMLAbstractMacroCall514 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLAbstractMacroCall524 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroInclude_in_ruleWMLAbstractMacroCall571 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLAbstractMacroCall598 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroInclude_in_entryRuleWMLMacroInclude633 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroInclude643 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_ruleWMLMacroInclude678 = new BitSet(new long[]{0x0000000008400010L});
    public static final BitSet FOLLOW_ruleWMLPath_in_ruleWMLMacroInclude699 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_ruleWMLMacroInclude709 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall745 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCall755 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_ruleWMLMacroCall790 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLMacroCall807 = new BitSet(new long[]{0x00000000021620F0L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLMacroCall831 = new BitSet(new long[]{0x00000000021620F0L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLMacroCall851 = new BitSet(new long[]{0x00000000021620F0L});
    public static final BitSet FOLLOW_ruleTSTRING_in_ruleWMLMacroCall875 = new BitSet(new long[]{0x00000000021620F0L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_ruleWMLMacroCall890 = new BitSet(new long[]{0x00000000021620F0L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_ruleWMLMacroCall921 = new BitSet(new long[]{0x0000000000162090L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLMacroCall948 = new BitSet(new long[]{0x0000000000162090L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroCall975 = new BitSet(new long[]{0x0000000000162090L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLMacroCall1002 = new BitSet(new long[]{0x0000000000162090L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLMacroCall1029 = new BitSet(new long[]{0x0000000000162090L});
    public static final BitSet FOLLOW_18_in_ruleWMLMacroCall1041 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1077 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine1087 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1121 = new BitSet(new long[]{0x0000000000122190L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_ruleWMLMacroDefine1142 = new BitSet(new long[]{0x0000000000122190L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLMacroDefine1169 = new BitSet(new long[]{0x0000000000122190L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroDefine1196 = new BitSet(new long[]{0x0000000000122190L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLMacroDefine1223 = new BitSet(new long[]{0x0000000000122190L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLMacroDefine1250 = new BitSet(new long[]{0x0000000000122190L});
    public static final BitSet FOLLOW_RULE_ENDDEFINE_in_ruleWMLMacroDefine1261 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey1298 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey1308 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLIDList_in_ruleWMLKey1354 = new BitSet(new long[]{0x0000000000080000L});
    public static final BitSet FOLLOW_19_in_ruleWMLKey1364 = new BitSet(new long[]{0x000000000A420230L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLKey1385 = new BitSet(new long[]{0x0000000000004002L});
    public static final BitSet FOLLOW_14_in_ruleWMLKey1396 = new BitSet(new long[]{0x0000000002020020L});
    public static final BitSet FOLLOW_ruleWMLKeyExtraArgs_in_ruleWMLKey1417 = new BitSet(new long[]{0x0000000000004002L});
    public static final BitSet FOLLOW_ruleWMLKeyExtraArgs_in_entryRuleWMLKeyExtraArgs1455 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyExtraArgs1465 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyExtraArgs1512 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLKeyExtraArgs1526 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_ruleWMLKeyExtraArgs1540 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_entryRuleWMLMacroCallParameter1569 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCallParameter1580 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_ruleWMLMacroCallParameter1618 = new BitSet(new long[]{0x000000000A000030L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLMacroCallParameter1634 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLMacroCallParameter1660 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_ruleTSTRING_in_ruleWMLMacroCallParameter1693 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_ruleFILE_in_ruleWMLMacroCallParameter1726 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_21_in_ruleWMLMacroCallParameter1745 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue1785 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue1795 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLINTList_in_ruleWMLKeyValue1829 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLIDList_in_ruleWMLKeyValue1836 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyValue1856 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLKeyValue1870 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_ruleWMLKeyValue1884 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPath_in_ruleWMLKeyValue1891 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFILE_in_ruleWMLKeyValue1898 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPath_in_entryRuleWMLPath1927 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLPath1938 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_ruleWMLPath1977 = new BitSet(new long[]{0x0000000008000010L});
    public static final BitSet FOLLOW_rulePATH_ID_in_ruleWMLPath2001 = new BitSet(new long[]{0x0000000000800000L});
    public static final BitSet FOLLOW_23_in_ruleWMLPath2020 = new BitSet(new long[]{0x0000000008000010L});
    public static final BitSet FOLLOW_rulePATH_ID_in_ruleWMLPath2042 = new BitSet(new long[]{0x0000000008800012L});
    public static final BitSet FOLLOW_ruleFILE_in_ruleWMLPath2072 = new BitSet(new long[]{0x0000000000800002L});
    public static final BitSet FOLLOW_23_in_ruleWMLPath2093 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLIDList_in_entryRuleWMLIDList2136 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLIDList2147 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLIDList2187 = new BitSet(new long[]{0x0000000001000002L});
    public static final BitSet FOLLOW_24_in_ruleWMLIDList2206 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLIDList2221 = new BitSet(new long[]{0x0000000001000002L});
    public static final BitSet FOLLOW_ruleWMLINTList_in_entryRuleWMLINTList2269 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLINTList2280 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_INT_in_ruleWMLINTList2320 = new BitSet(new long[]{0x0000000001000002L});
    public static final BitSet FOLLOW_24_in_ruleWMLINTList2339 = new BitSet(new long[]{0x0000000000000200L});
    public static final BitSet FOLLOW_RULE_INT_in_ruleWMLINTList2354 = new BitSet(new long[]{0x0000000001000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING2402 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING2413 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_ruleTSTRING2451 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleTSTRING2466 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFILE_in_entryRuleFILE2512 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleFILE2523 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_ID_in_ruleFILE2571 = new BitSet(new long[]{0x000000000C000010L});
    public static final BitSet FOLLOW_26_in_ruleFILE2591 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleFILE2606 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_ID_in_entryRulePATH_ID2652 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePATH_ID2663 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rulePATH_ID2703 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_rulePATH_ID2727 = new BitSet(new long[]{0x0000000000000002L});

}