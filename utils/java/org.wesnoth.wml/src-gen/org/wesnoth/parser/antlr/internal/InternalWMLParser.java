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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_INT", "RULE_STRING", "RULE_ANY_OTHER", "RULE_SL_COMMENT", "RULE_WS", "RULE_EOL", "'['", "'+'", "']'", "'[/'", "'='", "'{'", "'~'", "'}'", "'_'"
    };
    public static final int RULE_ID=4;
    public static final int RULE_STRING=6;
    public static final int RULE_ANY_OTHER=7;
    public static final int RULE_INT=5;
    public static final int RULE_EOL=10;
    public static final int RULE_WS=9;
    public static final int RULE_SL_COMMENT=8;
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:86:1: ruleWMLRoot returns [EObject current=null] : ( ( (lv_tags_0_0= ruleWMLTag ) ) | ( (lv_macroCalls_1_0= ruleWMLAbstractMacroCall ) ) )* ;
    public final EObject ruleWMLRoot() throws RecognitionException {
        EObject current = null;

        EObject lv_tags_0_0 = null;

        EObject lv_macroCalls_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:91:6: ( ( ( (lv_tags_0_0= ruleWMLTag ) ) | ( (lv_macroCalls_1_0= ruleWMLAbstractMacroCall ) ) )* )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( ( (lv_tags_0_0= ruleWMLTag ) ) | ( (lv_macroCalls_1_0= ruleWMLAbstractMacroCall ) ) )*
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( ( (lv_tags_0_0= ruleWMLTag ) ) | ( (lv_macroCalls_1_0= ruleWMLAbstractMacroCall ) ) )*
            loop1:
            do {
                int alt1=3;
                int LA1_0 = input.LA(1);

                if ( (LA1_0==11) ) {
                    alt1=1;
                }
                else if ( (LA1_0==16) ) {
                    alt1=2;
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:117:6: ( (lv_macroCalls_1_0= ruleWMLAbstractMacroCall ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:117:6: ( (lv_macroCalls_1_0= ruleWMLAbstractMacroCall ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:118:1: (lv_macroCalls_1_0= ruleWMLAbstractMacroCall )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:118:1: (lv_macroCalls_1_0= ruleWMLAbstractMacroCall )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:119:3: lv_macroCalls_1_0= ruleWMLAbstractMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getMacroCallsWMLAbstractMacroCallParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLAbstractMacroCall_in_ruleWMLRoot158);
            	    lv_macroCalls_1_0=ruleWMLAbstractMacroCall();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"macroCalls",
            	    	        		lv_macroCalls_1_0, 
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:149:1: entryRuleWMLTag returns [EObject current=null] : iv_ruleWMLTag= ruleWMLTag EOF ;
    public final EObject entryRuleWMLTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:150:2: (iv_ruleWMLTag= ruleWMLTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:151:2: iv_ruleWMLTag= ruleWMLTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLTag_in_entryRuleWMLTag195);
            iv_ruleWMLTag=ruleWMLTag();
            _fsp--;

             current =iv_ruleWMLTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTag205); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:158:1: ruleWMLTag returns [EObject current=null] : ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_keys_5_0= ruleWMLKey ) ) | ( (lv_macroCalls_6_0= ruleWMLAbstractMacroCall ) ) )* '[/' ( (lv_endName_8_0= RULE_ID ) ) ']' ) ;
    public final EObject ruleWMLTag() throws RecognitionException {
        EObject current = null;

        Token lv_plus_1_0=null;
        Token lv_name_2_0=null;
        Token lv_endName_8_0=null;
        EObject lv_tags_4_0 = null;

        EObject lv_keys_5_0 = null;

        EObject lv_macroCalls_6_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:163:6: ( ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_keys_5_0= ruleWMLKey ) ) | ( (lv_macroCalls_6_0= ruleWMLAbstractMacroCall ) ) )* '[/' ( (lv_endName_8_0= RULE_ID ) ) ']' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:1: ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_keys_5_0= ruleWMLKey ) ) | ( (lv_macroCalls_6_0= ruleWMLAbstractMacroCall ) ) )* '[/' ( (lv_endName_8_0= RULE_ID ) ) ']' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:1: ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_keys_5_0= ruleWMLKey ) ) | ( (lv_macroCalls_6_0= ruleWMLAbstractMacroCall ) ) )* '[/' ( (lv_endName_8_0= RULE_ID ) ) ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:3: '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_keys_5_0= ruleWMLKey ) ) | ( (lv_macroCalls_6_0= ruleWMLAbstractMacroCall ) ) )* '[/' ( (lv_endName_8_0= RULE_ID ) ) ']'
            {
            match(input,11,FOLLOW_11_in_ruleWMLTag240); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:168:1: ( (lv_plus_1_0= '+' ) )?
            int alt2=2;
            int LA2_0 = input.LA(1);

            if ( (LA2_0==12) ) {
                alt2=1;
            }
            switch (alt2) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:169:1: (lv_plus_1_0= '+' )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:169:1: (lv_plus_1_0= '+' )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:170:3: lv_plus_1_0= '+'
                    {
                    lv_plus_1_0=(Token)input.LT(1);
                    match(input,12,FOLLOW_12_in_ruleWMLTag258); 

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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:189:3: ( (lv_name_2_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:190:1: (lv_name_2_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:190:1: (lv_name_2_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:191:3: lv_name_2_0= RULE_ID
            {
            lv_name_2_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag289); 

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

            match(input,13,FOLLOW_13_in_ruleWMLTag304); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:217:1: ( ( (lv_tags_4_0= ruleWMLTag ) ) | ( (lv_keys_5_0= ruleWMLKey ) ) | ( (lv_macroCalls_6_0= ruleWMLAbstractMacroCall ) ) )*
            loop3:
            do {
                int alt3=4;
                switch ( input.LA(1) ) {
                case 11:
                    {
                    alt3=1;
                    }
                    break;
                case RULE_ID:
                    {
                    alt3=2;
                    }
                    break;
                case 16:
                    {
                    alt3=3;
                    }
                    break;

                }

                switch (alt3) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:217:2: ( (lv_tags_4_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:217:2: ( (lv_tags_4_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:218:1: (lv_tags_4_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:218:1: (lv_tags_4_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:219:3: lv_tags_4_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTagsWMLTagParserRuleCall_4_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLTag326);
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:242:6: ( (lv_keys_5_0= ruleWMLKey ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:242:6: ( (lv_keys_5_0= ruleWMLKey ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:243:1: (lv_keys_5_0= ruleWMLKey )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:243:1: (lv_keys_5_0= ruleWMLKey )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:244:3: lv_keys_5_0= ruleWMLKey
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getKeysWMLKeyParserRuleCall_4_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLTag353);
            	    lv_keys_5_0=ruleWMLKey();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
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
            	case 3 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:267:6: ( (lv_macroCalls_6_0= ruleWMLAbstractMacroCall ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:267:6: ( (lv_macroCalls_6_0= ruleWMLAbstractMacroCall ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:268:1: (lv_macroCalls_6_0= ruleWMLAbstractMacroCall )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:268:1: (lv_macroCalls_6_0= ruleWMLAbstractMacroCall )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:269:3: lv_macroCalls_6_0= ruleWMLAbstractMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getMacroCallsWMLAbstractMacroCallParserRuleCall_4_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLAbstractMacroCall_in_ruleWMLTag380);
            	    lv_macroCalls_6_0=ruleWMLAbstractMacroCall();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"macroCalls",
            	    	        		lv_macroCalls_6_0, 
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

            	default :
            	    break loop3;
                }
            } while (true);

            match(input,14,FOLLOW_14_in_ruleWMLTag392); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:295:1: ( (lv_endName_8_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:296:1: (lv_endName_8_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:296:1: (lv_endName_8_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:297:3: lv_endName_8_0= RULE_ID
            {
            lv_endName_8_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag409); 

            			createLeafNode(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0(), "endName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"endName",
            	        		lv_endName_8_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            match(input,13,FOLLOW_13_in_ruleWMLTag424); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:331:1: entryRuleWMLKey returns [EObject current=null] : iv_ruleWMLKey= ruleWMLKey EOF ;
    public final EObject entryRuleWMLKey() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKey = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:332:2: (iv_ruleWMLKey= ruleWMLKey EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:333:2: iv_ruleWMLKey= ruleWMLKey EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey460);
            iv_ruleWMLKey=ruleWMLKey();
            _fsp--;

             current =iv_ruleWMLKey; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey470); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:340:1: ruleWMLKey returns [EObject current=null] : ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLAbstractKeyValue ) ) ( '+' ( (lv_extraArgs_4_0= ruleWMLAbstractKeyValue ) ) )* ) ;
    public final EObject ruleWMLKey() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;
        EObject lv_value_2_0 = null;

        EObject lv_extraArgs_4_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:345:6: ( ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLAbstractKeyValue ) ) ( '+' ( (lv_extraArgs_4_0= ruleWMLAbstractKeyValue ) ) )* ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:346:1: ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLAbstractKeyValue ) ) ( '+' ( (lv_extraArgs_4_0= ruleWMLAbstractKeyValue ) ) )* )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:346:1: ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLAbstractKeyValue ) ) ( '+' ( (lv_extraArgs_4_0= ruleWMLAbstractKeyValue ) ) )* )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:346:2: ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLAbstractKeyValue ) ) ( '+' ( (lv_extraArgs_4_0= ruleWMLAbstractKeyValue ) ) )*
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:346:2: ( (lv_name_0_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:347:1: (lv_name_0_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:347:1: (lv_name_0_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:348:3: lv_name_0_0= RULE_ID
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLKey512); 

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

            match(input,15,FOLLOW_15_in_ruleWMLKey527); 

                    createLeafNode(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:374:1: ( (lv_value_2_0= ruleWMLAbstractKeyValue ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:375:1: (lv_value_2_0= ruleWMLAbstractKeyValue )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:375:1: (lv_value_2_0= ruleWMLAbstractKeyValue )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:376:3: lv_value_2_0= ruleWMLAbstractKeyValue
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getValueWMLAbstractKeyValueParserRuleCall_2_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleWMLAbstractKeyValue_in_ruleWMLKey548);
            lv_value_2_0=ruleWMLAbstractKeyValue();
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
            	        		"WMLAbstractKeyValue", 
            	        		currentNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	        currentNode = currentNode.getParent();
            	    

            }


            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:398:2: ( '+' ( (lv_extraArgs_4_0= ruleWMLAbstractKeyValue ) ) )*
            loop4:
            do {
                int alt4=2;
                int LA4_0 = input.LA(1);

                if ( (LA4_0==12) ) {
                    alt4=1;
                }


                switch (alt4) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:398:4: '+' ( (lv_extraArgs_4_0= ruleWMLAbstractKeyValue ) )
            	    {
            	    match(input,12,FOLLOW_12_in_ruleWMLKey559); 

            	            createLeafNode(grammarAccess.getWMLKeyAccess().getPlusSignKeyword_3_0(), null); 
            	        
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:402:1: ( (lv_extraArgs_4_0= ruleWMLAbstractKeyValue ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:403:1: (lv_extraArgs_4_0= ruleWMLAbstractKeyValue )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:403:1: (lv_extraArgs_4_0= ruleWMLAbstractKeyValue )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:404:3: lv_extraArgs_4_0= ruleWMLAbstractKeyValue
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getExtraArgsWMLAbstractKeyValueParserRuleCall_3_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLAbstractKeyValue_in_ruleWMLKey580);
            	    lv_extraArgs_4_0=ruleWMLAbstractKeyValue();
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
            	    	        		"WMLAbstractKeyValue", 
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
            	    break loop4;
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


    // $ANTLR start entryRuleWMLAbstractMacroCall
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:434:1: entryRuleWMLAbstractMacroCall returns [EObject current=null] : iv_ruleWMLAbstractMacroCall= ruleWMLAbstractMacroCall EOF ;
    public final EObject entryRuleWMLAbstractMacroCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLAbstractMacroCall = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:435:2: (iv_ruleWMLAbstractMacroCall= ruleWMLAbstractMacroCall EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:436:2: iv_ruleWMLAbstractMacroCall= ruleWMLAbstractMacroCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLAbstractMacroCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLAbstractMacroCall_in_entryRuleWMLAbstractMacroCall618);
            iv_ruleWMLAbstractMacroCall=ruleWMLAbstractMacroCall();
            _fsp--;

             current =iv_ruleWMLAbstractMacroCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLAbstractMacroCall628); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:443:1: ruleWMLAbstractMacroCall returns [EObject current=null] : (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall ) ;
    public final EObject ruleWMLAbstractMacroCall() throws RecognitionException {
        EObject current = null;

        EObject this_WMLMacroInclude_0 = null;

        EObject this_WMLMacroCall_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:448:6: ( (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:449:1: (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:449:1: (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall )
            int alt5=2;
            int LA5_0 = input.LA(1);

            if ( (LA5_0==16) ) {
                int LA5_1 = input.LA(2);

                if ( (LA5_1==RULE_ID) ) {
                    alt5=2;
                }
                else if ( (LA5_1==17) ) {
                    alt5=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("449:1: (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall )", 5, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("449:1: (this_WMLMacroInclude_0= ruleWMLMacroInclude | this_WMLMacroCall_1= ruleWMLMacroCall )", 5, 0, input);

                throw nvae;
            }
            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:450:5: this_WMLMacroInclude_0= ruleWMLMacroInclude
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLAbstractMacroCallAccess().getWMLMacroIncludeParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroInclude_in_ruleWMLAbstractMacroCall675);
                    this_WMLMacroInclude_0=ruleWMLMacroInclude();
                    _fsp--;

                     
                            current = this_WMLMacroInclude_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:460:5: this_WMLMacroCall_1= ruleWMLMacroCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLAbstractMacroCallAccess().getWMLMacroCallParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLAbstractMacroCall702);
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:476:1: entryRuleWMLMacroInclude returns [EObject current=null] : iv_ruleWMLMacroInclude= ruleWMLMacroInclude EOF ;
    public final EObject entryRuleWMLMacroInclude() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroInclude = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:477:2: (iv_ruleWMLMacroInclude= ruleWMLMacroInclude EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:478:2: iv_ruleWMLMacroInclude= ruleWMLMacroInclude EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroIncludeRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroInclude_in_entryRuleWMLMacroInclude737);
            iv_ruleWMLMacroInclude=ruleWMLMacroInclude();
            _fsp--;

             current =iv_ruleWMLMacroInclude; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroInclude747); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:485:1: ruleWMLMacroInclude returns [EObject current=null] : ( '{' '~' ( (lv_path_2_0= ruleWMLKeyValue ) ) '}' ) ;
    public final EObject ruleWMLMacroInclude() throws RecognitionException {
        EObject current = null;

        AntlrDatatypeRuleToken lv_path_2_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:490:6: ( ( '{' '~' ( (lv_path_2_0= ruleWMLKeyValue ) ) '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:491:1: ( '{' '~' ( (lv_path_2_0= ruleWMLKeyValue ) ) '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:491:1: ( '{' '~' ( (lv_path_2_0= ruleWMLKeyValue ) ) '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:491:3: '{' '~' ( (lv_path_2_0= ruleWMLKeyValue ) ) '}'
            {
            match(input,16,FOLLOW_16_in_ruleWMLMacroInclude782); 

                    createLeafNode(grammarAccess.getWMLMacroIncludeAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            match(input,17,FOLLOW_17_in_ruleWMLMacroInclude792); 

                    createLeafNode(grammarAccess.getWMLMacroIncludeAccess().getTildeKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:499:1: ( (lv_path_2_0= ruleWMLKeyValue ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:500:1: (lv_path_2_0= ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:500:1: (lv_path_2_0= ruleWMLKeyValue )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:501:3: lv_path_2_0= ruleWMLKeyValue
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getWMLMacroIncludeAccess().getPathWMLKeyValueParserRuleCall_2_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleWMLKeyValue_in_ruleWMLMacroInclude813);
            lv_path_2_0=ruleWMLKeyValue();
            _fsp--;


            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLMacroIncludeRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"path",
            	        		lv_path_2_0, 
            	        		"WMLKeyValue", 
            	        		currentNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	        currentNode = currentNode.getParent();
            	    

            }


            }

            match(input,18,FOLLOW_18_in_ruleWMLMacroInclude823); 

                    createLeafNode(grammarAccess.getWMLMacroIncludeAccess().getRightCurlyBracketKeyword_3(), null); 
                

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:535:1: entryRuleWMLMacroCall returns [EObject current=null] : iv_ruleWMLMacroCall= ruleWMLMacroCall EOF ;
    public final EObject entryRuleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroCall = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:536:2: (iv_ruleWMLMacroCall= ruleWMLMacroCall EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:537:2: iv_ruleWMLMacroCall= ruleWMLMacroCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall859);
            iv_ruleWMLMacroCall=ruleWMLMacroCall();
            _fsp--;

             current =iv_ruleWMLMacroCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCall869); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:544:1: ruleWMLMacroCall returns [EObject current=null] : ( '{' ( (lv_name_1_0= RULE_ID ) ) ( (lv_params_2_0= ruleWMLAbstractKeyValue ) )? '}' ) ;
    public final EObject ruleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        Token lv_name_1_0=null;
        EObject lv_params_2_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:549:6: ( ( '{' ( (lv_name_1_0= RULE_ID ) ) ( (lv_params_2_0= ruleWMLAbstractKeyValue ) )? '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:550:1: ( '{' ( (lv_name_1_0= RULE_ID ) ) ( (lv_params_2_0= ruleWMLAbstractKeyValue ) )? '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:550:1: ( '{' ( (lv_name_1_0= RULE_ID ) ) ( (lv_params_2_0= ruleWMLAbstractKeyValue ) )? '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:550:3: '{' ( (lv_name_1_0= RULE_ID ) ) ( (lv_params_2_0= ruleWMLAbstractKeyValue ) )? '}'
            {
            match(input,16,FOLLOW_16_in_ruleWMLMacroCall904); 

                    createLeafNode(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:554:1: ( (lv_name_1_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:555:1: (lv_name_1_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:555:1: (lv_name_1_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:556:3: lv_name_1_0= RULE_ID
            {
            lv_name_1_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacroCall921); 

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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:578:2: ( (lv_params_2_0= ruleWMLAbstractKeyValue ) )?
            int alt6=2;
            int LA6_0 = input.LA(1);

            if ( ((LA6_0>=RULE_ID && LA6_0<=RULE_ANY_OTHER)||LA6_0==16||LA6_0==19) ) {
                alt6=1;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:579:1: (lv_params_2_0= ruleWMLAbstractKeyValue )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:579:1: (lv_params_2_0= ruleWMLAbstractKeyValue )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:580:3: lv_params_2_0= ruleWMLAbstractKeyValue
                    {
                     
                    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getParamsWMLAbstractKeyValueParserRuleCall_2_0(), currentNode); 
                    	    
                    pushFollow(FOLLOW_ruleWMLAbstractKeyValue_in_ruleWMLMacroCall947);
                    lv_params_2_0=ruleWMLAbstractKeyValue();
                    _fsp--;


                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode.getParent(), current);
                    	        }
                    	        try {
                    	       		add(
                    	       			current, 
                    	       			"params",
                    	        		lv_params_2_0, 
                    	        		"WMLAbstractKeyValue", 
                    	        		currentNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	        currentNode = currentNode.getParent();
                    	    

                    }


                    }
                    break;

            }

            match(input,18,FOLLOW_18_in_ruleWMLMacroCall958); 

                    createLeafNode(grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_3(), null); 
                

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


    // $ANTLR start entryRuleWMLAbstractKeyValue
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:614:1: entryRuleWMLAbstractKeyValue returns [EObject current=null] : iv_ruleWMLAbstractKeyValue= ruleWMLAbstractKeyValue EOF ;
    public final EObject entryRuleWMLAbstractKeyValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLAbstractKeyValue = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:615:2: (iv_ruleWMLAbstractKeyValue= ruleWMLAbstractKeyValue EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:616:2: iv_ruleWMLAbstractKeyValue= ruleWMLAbstractKeyValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLAbstractKeyValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLAbstractKeyValue_in_entryRuleWMLAbstractKeyValue994);
            iv_ruleWMLAbstractKeyValue=ruleWMLAbstractKeyValue();
            _fsp--;

             current =iv_ruleWMLAbstractKeyValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLAbstractKeyValue1004); 

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
    // $ANTLR end entryRuleWMLAbstractKeyValue


    // $ANTLR start ruleWMLAbstractKeyValue
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:623:1: ruleWMLAbstractKeyValue returns [EObject current=null] : ( ruleWMLKeyValue | this_WMLMacroCall_1= ruleWMLMacroCall ) ;
    public final EObject ruleWMLAbstractKeyValue() throws RecognitionException {
        EObject current = null;

        EObject this_WMLMacroCall_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:628:6: ( ( ruleWMLKeyValue | this_WMLMacroCall_1= ruleWMLMacroCall ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:629:1: ( ruleWMLKeyValue | this_WMLMacroCall_1= ruleWMLMacroCall )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:629:1: ( ruleWMLKeyValue | this_WMLMacroCall_1= ruleWMLMacroCall )
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( ((LA7_0>=RULE_ID && LA7_0<=RULE_ANY_OTHER)||LA7_0==19) ) {
                alt7=1;
            }
            else if ( (LA7_0==16) ) {
                alt7=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("629:1: ( ruleWMLKeyValue | this_WMLMacroCall_1= ruleWMLMacroCall )", 7, 0, input);

                throw nvae;
            }
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:629:2: ruleWMLKeyValue
                    {
                    pushFollow(FOLLOW_ruleWMLKeyValue_in_ruleWMLAbstractKeyValue1038);
                    ruleWMLKeyValue();
                    _fsp--;


                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:631:5: this_WMLMacroCall_1= ruleWMLMacroCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLAbstractKeyValueAccess().getWMLMacroCallParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLAbstractKeyValue1058);
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
    // $ANTLR end ruleWMLAbstractKeyValue


    // $ANTLR start entryRuleWMLKeyValue
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:647:1: entryRuleWMLKeyValue returns [String current=null] : iv_ruleWMLKeyValue= ruleWMLKeyValue EOF ;
    public final String entryRuleWMLKeyValue() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleWMLKeyValue = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:648:2: (iv_ruleWMLKeyValue= ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:649:2: iv_ruleWMLKeyValue= ruleWMLKeyValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue1094);
            iv_ruleWMLKeyValue=ruleWMLKeyValue();
            _fsp--;

             current =iv_ruleWMLKeyValue.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue1105); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:656:1: ruleWMLKeyValue returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_WMLANYVALUE_0= ruleWMLANYVALUE )+ ;
    public final AntlrDatatypeRuleToken ruleWMLKeyValue() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        AntlrDatatypeRuleToken this_WMLANYVALUE_0 = null;


         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:661:6: ( (this_WMLANYVALUE_0= ruleWMLANYVALUE )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:662:1: (this_WMLANYVALUE_0= ruleWMLANYVALUE )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:662:1: (this_WMLANYVALUE_0= ruleWMLANYVALUE )+
            int cnt8=0;
            loop8:
            do {
                int alt8=2;
                int LA8_0 = input.LA(1);

                if ( (LA8_0==RULE_ID) ) {
                    int LA8_2 = input.LA(2);

                    if ( (LA8_2==EOF||(LA8_2>=RULE_ID && LA8_2<=RULE_ANY_OTHER)||(LA8_2>=11 && LA8_2<=12)||LA8_2==14||LA8_2==16||(LA8_2>=18 && LA8_2<=19)) ) {
                        alt8=1;
                    }


                }
                else if ( ((LA8_0>=RULE_INT && LA8_0<=RULE_ANY_OTHER)||LA8_0==19) ) {
                    alt8=1;
                }


                switch (alt8) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:663:5: this_WMLANYVALUE_0= ruleWMLANYVALUE
            	    {
            	     
            	            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLANYVALUEParserRuleCall(), currentNode); 
            	        
            	    pushFollow(FOLLOW_ruleWMLANYVALUE_in_ruleWMLKeyValue1152);
            	    this_WMLANYVALUE_0=ruleWMLANYVALUE();
            	    _fsp--;


            	    		current.merge(this_WMLANYVALUE_0);
            	        
            	     
            	            currentNode = currentNode.getParent();
            	        

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


    // $ANTLR start entryRuleWMLANYVALUE
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:681:1: entryRuleWMLANYVALUE returns [String current=null] : iv_ruleWMLANYVALUE= ruleWMLANYVALUE EOF ;
    public final String entryRuleWMLANYVALUE() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleWMLANYVALUE = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:682:2: (iv_ruleWMLANYVALUE= ruleWMLANYVALUE EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:683:2: iv_ruleWMLANYVALUE= ruleWMLANYVALUE EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLANYVALUERule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLANYVALUE_in_entryRuleWMLANYVALUE1199);
            iv_ruleWMLANYVALUE=ruleWMLANYVALUE();
            _fsp--;

             current =iv_ruleWMLANYVALUE.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLANYVALUE1210); 

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
    // $ANTLR end entryRuleWMLANYVALUE


    // $ANTLR start ruleWMLANYVALUE
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:690:1: ruleWMLANYVALUE returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_ID_0= RULE_ID | this_INT_1= RULE_INT | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_ANY_OTHER_4= RULE_ANY_OTHER ) ;
    public final AntlrDatatypeRuleToken ruleWMLANYVALUE() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;
        Token this_INT_1=null;
        Token this_STRING_2=null;
        Token this_ANY_OTHER_4=null;
        AntlrDatatypeRuleToken this_TSTRING_3 = null;


         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:695:6: ( (this_ID_0= RULE_ID | this_INT_1= RULE_INT | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_ANY_OTHER_4= RULE_ANY_OTHER ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:696:1: (this_ID_0= RULE_ID | this_INT_1= RULE_INT | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_ANY_OTHER_4= RULE_ANY_OTHER )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:696:1: (this_ID_0= RULE_ID | this_INT_1= RULE_INT | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_ANY_OTHER_4= RULE_ANY_OTHER )
            int alt9=5;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt9=1;
                }
                break;
            case RULE_INT:
                {
                alt9=2;
                }
                break;
            case RULE_STRING:
                {
                alt9=3;
                }
                break;
            case 19:
                {
                alt9=4;
                }
                break;
            case RULE_ANY_OTHER:
                {
                alt9=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("696:1: (this_ID_0= RULE_ID | this_INT_1= RULE_INT | this_STRING_2= RULE_STRING | this_TSTRING_3= ruleTSTRING | this_ANY_OTHER_4= RULE_ANY_OTHER )", 9, 0, input);

                throw nvae;
            }

            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:696:6: this_ID_0= RULE_ID
                    {
                    this_ID_0=(Token)input.LT(1);
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLANYVALUE1250); 

                    		current.merge(this_ID_0);
                        
                     
                        createLeafNode(grammarAccess.getWMLANYVALUEAccess().getIDTerminalRuleCall_0(), null); 
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:704:10: this_INT_1= RULE_INT
                    {
                    this_INT_1=(Token)input.LT(1);
                    match(input,RULE_INT,FOLLOW_RULE_INT_in_ruleWMLANYVALUE1276); 

                    		current.merge(this_INT_1);
                        
                     
                        createLeafNode(grammarAccess.getWMLANYVALUEAccess().getINTTerminalRuleCall_1(), null); 
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:712:10: this_STRING_2= RULE_STRING
                    {
                    this_STRING_2=(Token)input.LT(1);
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLANYVALUE1302); 

                    		current.merge(this_STRING_2);
                        
                     
                        createLeafNode(grammarAccess.getWMLANYVALUEAccess().getSTRINGTerminalRuleCall_2(), null); 
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:721:5: this_TSTRING_3= ruleTSTRING
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLANYVALUEAccess().getTSTRINGParserRuleCall_3(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleTSTRING_in_ruleWMLANYVALUE1335);
                    this_TSTRING_3=ruleTSTRING();
                    _fsp--;


                    		current.merge(this_TSTRING_3);
                        
                     
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:732:10: this_ANY_OTHER_4= RULE_ANY_OTHER
                    {
                    this_ANY_OTHER_4=(Token)input.LT(1);
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_ruleWMLANYVALUE1361); 

                    		current.merge(this_ANY_OTHER_4);
                        
                     
                        createLeafNode(grammarAccess.getWMLANYVALUEAccess().getANY_OTHERTerminalRuleCall_4(), null); 
                        

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
    // $ANTLR end ruleWMLANYVALUE


    // $ANTLR start entryRuleTSTRING
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:747:1: entryRuleTSTRING returns [String current=null] : iv_ruleTSTRING= ruleTSTRING EOF ;
    public final String entryRuleTSTRING() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleTSTRING = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:748:2: (iv_ruleTSTRING= ruleTSTRING EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:749:2: iv_ruleTSTRING= ruleTSTRING EOF
            {
             currentNode = createCompositeNode(grammarAccess.getTSTRINGRule(), currentNode); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING1407);
            iv_ruleTSTRING=ruleTSTRING();
            _fsp--;

             current =iv_ruleTSTRING.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING1418); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:756:1: ruleTSTRING returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (kw= '_' this_STRING_1= RULE_STRING ) ;
    public final AntlrDatatypeRuleToken ruleTSTRING() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        Token this_STRING_1=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:761:6: ( (kw= '_' this_STRING_1= RULE_STRING ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:762:1: (kw= '_' this_STRING_1= RULE_STRING )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:762:1: (kw= '_' this_STRING_1= RULE_STRING )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:763:2: kw= '_' this_STRING_1= RULE_STRING
            {
            kw=(Token)input.LT(1);
            match(input,19,FOLLOW_19_in_ruleTSTRING1456); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getTSTRINGAccess().get_Keyword_0(), null); 
                
            this_STRING_1=(Token)input.LT(1);
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleTSTRING1471); 

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


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot75 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot85 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLRoot131 = new BitSet(new long[]{0x0000000000010802L});
    public static final BitSet FOLLOW_ruleWMLAbstractMacroCall_in_ruleWMLRoot158 = new BitSet(new long[]{0x0000000000010802L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag195 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag205 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_ruleWMLTag240 = new BitSet(new long[]{0x0000000000001010L});
    public static final BitSet FOLLOW_12_in_ruleWMLTag258 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag289 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_13_in_ruleWMLTag304 = new BitSet(new long[]{0x0000000000014810L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLTag326 = new BitSet(new long[]{0x0000000000014810L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLTag353 = new BitSet(new long[]{0x0000000000014810L});
    public static final BitSet FOLLOW_ruleWMLAbstractMacroCall_in_ruleWMLTag380 = new BitSet(new long[]{0x0000000000014810L});
    public static final BitSet FOLLOW_14_in_ruleWMLTag392 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag409 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_13_in_ruleWMLTag424 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey460 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey470 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKey512 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_15_in_ruleWMLKey527 = new BitSet(new long[]{0x00000000000900F0L});
    public static final BitSet FOLLOW_ruleWMLAbstractKeyValue_in_ruleWMLKey548 = new BitSet(new long[]{0x0000000000001002L});
    public static final BitSet FOLLOW_12_in_ruleWMLKey559 = new BitSet(new long[]{0x00000000000900F0L});
    public static final BitSet FOLLOW_ruleWMLAbstractKeyValue_in_ruleWMLKey580 = new BitSet(new long[]{0x0000000000001002L});
    public static final BitSet FOLLOW_ruleWMLAbstractMacroCall_in_entryRuleWMLAbstractMacroCall618 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLAbstractMacroCall628 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroInclude_in_ruleWMLAbstractMacroCall675 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLAbstractMacroCall702 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroInclude_in_entryRuleWMLMacroInclude737 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroInclude747 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_ruleWMLMacroInclude782 = new BitSet(new long[]{0x0000000000020000L});
    public static final BitSet FOLLOW_17_in_ruleWMLMacroInclude792 = new BitSet(new long[]{0x00000000000800F0L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLMacroInclude813 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_ruleWMLMacroInclude823 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall859 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCall869 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_ruleWMLMacroCall904 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLMacroCall921 = new BitSet(new long[]{0x00000000000D00F0L});
    public static final BitSet FOLLOW_ruleWMLAbstractKeyValue_in_ruleWMLMacroCall947 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_ruleWMLMacroCall958 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLAbstractKeyValue_in_entryRuleWMLAbstractKeyValue994 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLAbstractKeyValue1004 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLAbstractKeyValue1038 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLAbstractKeyValue1058 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue1094 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue1105 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLANYVALUE_in_ruleWMLKeyValue1152 = new BitSet(new long[]{0x00000000000800F2L});
    public static final BitSet FOLLOW_ruleWMLANYVALUE_in_entryRuleWMLANYVALUE1199 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLANYVALUE1210 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLANYVALUE1250 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_INT_in_ruleWMLANYVALUE1276 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLANYVALUE1302 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_ruleWMLANYVALUE1335 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_ruleWMLANYVALUE1361 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING1407 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING1418 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_ruleTSTRING1456 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleTSTRING1471 = new BitSet(new long[]{0x0000000000000002L});

}