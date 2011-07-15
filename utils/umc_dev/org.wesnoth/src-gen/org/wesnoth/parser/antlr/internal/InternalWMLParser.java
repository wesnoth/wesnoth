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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_EOL", "RULE_SL_COMMENT", "RULE_DEFINE", "RULE_ENDDEF", "RULE_IFDEF", "RULE_IFNDEF", "RULE_IFHAVE", "RULE_IFNHAVE", "RULE_ELSE", "RULE_ENDIF", "RULE_TEXTDOMAIN", "RULE_LUA_CODE", "RULE_STRING", "RULE_ANY_OTHER", "RULE_WS", "'['", "'+'", "']'", "'[/'", "'='", "'{'", "'./'", "'~'", "'}'", "'_'", "'.'", "'$'", "'/'", "'('", "')'"
    };
    public static final int RULE_LUA_CODE=16;
    public static final int RULE_ID=4;
    public static final int RULE_IFDEF=9;
    public static final int RULE_ANY_OTHER=18;
    public static final int RULE_IFNDEF=10;
    public static final int RULE_EOL=5;
    public static final int RULE_TEXTDOMAIN=15;
    public static final int RULE_IFNHAVE=12;
    public static final int RULE_SL_COMMENT=6;
    public static final int EOF=-1;
    public static final int RULE_STRING=17;
    public static final int RULE_ENDIF=14;
    public static final int RULE_DEFINE=7;
    public static final int RULE_ENDDEF=8;
    public static final int RULE_IFHAVE=11;
    public static final int RULE_WS=19;
    public static final int RULE_ELSE=13;

        public InternalWMLParser(TokenStream input) {
            super(input);
        }
        

    public String[] getTokenNames() { return tokenNames; }
    public String getGrammarFileName() { return "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g"; }



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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:77:1: entryRuleWMLRoot returns [EObject current=null] : iv_ruleWMLRoot= ruleWMLRoot EOF ;
    public final EObject entryRuleWMLRoot() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLRoot = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:78:2: (iv_ruleWMLRoot= ruleWMLRoot EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:79:2: iv_ruleWMLRoot= ruleWMLRoot EOF
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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:86:1: ruleWMLRoot returns [EObject current=null] : ( (lv_Expressions_0_0= ruleWMLRootExpression ) )* ;
    public final EObject ruleWMLRoot() throws RecognitionException {
        EObject current = null;

        EObject lv_Expressions_0_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:91:6: ( ( (lv_Expressions_0_0= ruleWMLRootExpression ) )* )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( (lv_Expressions_0_0= ruleWMLRootExpression ) )*
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( (lv_Expressions_0_0= ruleWMLRootExpression ) )*
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( (LA1_0==RULE_DEFINE||(LA1_0>=RULE_IFDEF && LA1_0<=RULE_IFNHAVE)||LA1_0==RULE_TEXTDOMAIN||LA1_0==20||LA1_0==25) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:93:1: (lv_Expressions_0_0= ruleWMLRootExpression )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:93:1: (lv_Expressions_0_0= ruleWMLRootExpression )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:94:3: lv_Expressions_0_0= ruleWMLRootExpression
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLRootAccess().getExpressionsWMLRootExpressionParserRuleCall_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLRootExpression_in_ruleWMLRoot130);
            	    lv_Expressions_0_0=ruleWMLRootExpression();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Expressions",
            	    	        		lv_Expressions_0_0, 
            	    	        		"WMLRootExpression", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:124:1: entryRuleWMLTag returns [EObject current=null] : iv_ruleWMLTag= ruleWMLTag EOF ;
    public final EObject entryRuleWMLTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTag = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:125:2: (iv_ruleWMLTag= ruleWMLTag EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:126:2: iv_ruleWMLTag= ruleWMLTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLTag_in_entryRuleWMLTag166);
            iv_ruleWMLTag=ruleWMLTag();
            _fsp--;

             current =iv_ruleWMLTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTag176); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:133:1: ruleWMLTag returns [EObject current=null] : ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( (lv_Expressions_4_0= ruleWMLExpression ) )* '[/' ( (lv_endName_6_0= RULE_ID ) ) ']' ) ;
    public final EObject ruleWMLTag() throws RecognitionException {
        EObject current = null;

        Token lv_plus_1_0=null;
        Token lv_name_2_0=null;
        Token lv_endName_6_0=null;
        EObject lv_Expressions_4_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:138:6: ( ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( (lv_Expressions_4_0= ruleWMLExpression ) )* '[/' ( (lv_endName_6_0= RULE_ID ) ) ']' ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:139:1: ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( (lv_Expressions_4_0= ruleWMLExpression ) )* '[/' ( (lv_endName_6_0= RULE_ID ) ) ']' )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:139:1: ( '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( (lv_Expressions_4_0= ruleWMLExpression ) )* '[/' ( (lv_endName_6_0= RULE_ID ) ) ']' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:139:3: '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) ']' ( (lv_Expressions_4_0= ruleWMLExpression ) )* '[/' ( (lv_endName_6_0= RULE_ID ) ) ']'
            {
            match(input,20,FOLLOW_20_in_ruleWMLTag211); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:143:1: ( (lv_plus_1_0= '+' ) )?
            int alt2=2;
            int LA2_0 = input.LA(1);

            if ( (LA2_0==21) ) {
                alt2=1;
            }
            switch (alt2) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:144:1: (lv_plus_1_0= '+' )
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:144:1: (lv_plus_1_0= '+' )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:145:3: lv_plus_1_0= '+'
                    {
                    lv_plus_1_0=(Token)input.LT(1);
                    match(input,21,FOLLOW_21_in_ruleWMLTag229); 

                            createLeafNode(grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0(), "plus"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "plus", lv_plus_1_0, "+", lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }


                    }
                    break;

            }

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:3: ( (lv_name_2_0= RULE_ID ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:165:1: (lv_name_2_0= RULE_ID )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:165:1: (lv_name_2_0= RULE_ID )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:166:3: lv_name_2_0= RULE_ID
            {
            lv_name_2_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag260); 

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

            match(input,22,FOLLOW_22_in_ruleWMLTag275); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3(), null); 
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:192:1: ( (lv_Expressions_4_0= ruleWMLExpression ) )*
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( (LA3_0==RULE_ID||LA3_0==RULE_DEFINE||(LA3_0>=RULE_IFDEF && LA3_0<=RULE_IFNHAVE)||LA3_0==RULE_TEXTDOMAIN||LA3_0==20||LA3_0==25) ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:193:1: (lv_Expressions_4_0= ruleWMLExpression )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:193:1: (lv_Expressions_4_0= ruleWMLExpression )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:194:3: lv_Expressions_4_0= ruleWMLExpression
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getExpressionsWMLExpressionParserRuleCall_4_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLExpression_in_ruleWMLTag296);
            	    lv_Expressions_4_0=ruleWMLExpression();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Expressions",
            	    	        		lv_Expressions_4_0, 
            	    	        		"WMLExpression", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop3;
                }
            } while (true);

            match(input,23,FOLLOW_23_in_ruleWMLTag307); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5(), null); 
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:220:1: ( (lv_endName_6_0= RULE_ID ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:221:1: (lv_endName_6_0= RULE_ID )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:221:1: (lv_endName_6_0= RULE_ID )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:222:3: lv_endName_6_0= RULE_ID
            {
            lv_endName_6_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag324); 

            			createLeafNode(grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0(), "endName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"endName",
            	        		lv_endName_6_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            match(input,22,FOLLOW_22_in_ruleWMLTag339); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:256:1: entryRuleWMLKey returns [EObject current=null] : iv_ruleWMLKey= ruleWMLKey EOF ;
    public final EObject entryRuleWMLKey() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKey = null;


         
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
        	
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:260:2: (iv_ruleWMLKey= ruleWMLKey EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:261:2: iv_ruleWMLKey= ruleWMLKey EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey381);
            iv_ruleWMLKey=ruleWMLKey();
            _fsp--;

             current =iv_ruleWMLKey; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey391); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:271:1: ruleWMLKey returns [EObject current=null] : ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )* ( ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) )+ )* ( ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) ) ) ) ;
    public final EObject ruleWMLKey() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;
        Token lv_eol_7_1=null;
        Token lv_eol_7_2=null;
        EObject lv_value_2_0 = null;

        EObject lv_value_6_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:277:6: ( ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )* ( ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) )+ )* ( ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) ) ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:278:1: ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )* ( ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) )+ )* ( ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) ) ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:278:1: ( ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )* ( ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) )+ )* ( ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:278:2: ( (lv_name_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )* ( ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) )+ )* ( ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:278:2: ( (lv_name_0_0= RULE_ID ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:279:1: (lv_name_0_0= RULE_ID )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:279:1: (lv_name_0_0= RULE_ID )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:280:3: lv_name_0_0= RULE_ID
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLKey437); 

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

            match(input,24,FOLLOW_24_in_ruleWMLKey452); 

                    createLeafNode(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1(), null); 
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:306:1: ( (lv_value_2_0= ruleWMLKeyValue ) )*
            loop4:
            do {
                int alt4=2;
                int LA4_0 = input.LA(1);

                if ( (LA4_0==RULE_ID||(LA4_0>=RULE_LUA_CODE && LA4_0<=RULE_ANY_OTHER)||LA4_0==20||(LA4_0>=25 && LA4_0<=27)||(LA4_0>=29 && LA4_0<=34)) ) {
                    alt4=1;
                }


                switch (alt4) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:307:1: (lv_value_2_0= ruleWMLKeyValue )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:307:1: (lv_value_2_0= ruleWMLKeyValue )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:308:3: lv_value_2_0= ruleWMLKeyValue
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKeyValue_in_ruleWMLKey473);
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:330:3: ( ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) )+ )*
            loop8:
            do {
                int alt8=2;
                int LA8_0 = input.LA(1);

                if ( (LA8_0==RULE_EOL) ) {
                    int LA8_1 = input.LA(2);

                    if ( (LA8_1==21) ) {
                        alt8=1;
                    }


                }
                else if ( (LA8_0==21) ) {
                    alt8=1;
                }


                switch (alt8) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:330:4: ( RULE_EOL )? '+' ( RULE_EOL )? ( (lv_value_6_0= ruleWMLKeyValue ) )+
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:330:4: ( RULE_EOL )?
            	    int alt5=2;
            	    int LA5_0 = input.LA(1);

            	    if ( (LA5_0==RULE_EOL) ) {
            	        alt5=1;
            	    }
            	    switch (alt5) {
            	        case 1 :
            	            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:330:5: RULE_EOL
            	            {
            	            match(input,RULE_EOL,FOLLOW_RULE_EOL_in_ruleWMLKey485); 
            	             
            	                createLeafNode(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_0(), null); 
            	                

            	            }
            	            break;

            	    }

            	    match(input,21,FOLLOW_21_in_ruleWMLKey496); 

            	            createLeafNode(grammarAccess.getWMLKeyAccess().getPlusSignKeyword_3_1(), null); 
            	        
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:338:1: ( RULE_EOL )?
            	    int alt6=2;
            	    int LA6_0 = input.LA(1);

            	    if ( (LA6_0==RULE_EOL) ) {
            	        alt6=1;
            	    }
            	    switch (alt6) {
            	        case 1 :
            	            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:338:2: RULE_EOL
            	            {
            	            match(input,RULE_EOL,FOLLOW_RULE_EOL_in_ruleWMLKey506); 
            	             
            	                createLeafNode(grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_2(), null); 
            	                

            	            }
            	            break;

            	    }

            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:342:3: ( (lv_value_6_0= ruleWMLKeyValue ) )+
            	    int cnt7=0;
            	    loop7:
            	    do {
            	        int alt7=2;
            	        int LA7_0 = input.LA(1);

            	        if ( (LA7_0==RULE_ID||(LA7_0>=RULE_LUA_CODE && LA7_0<=RULE_ANY_OTHER)||LA7_0==20||(LA7_0>=25 && LA7_0<=27)||(LA7_0>=29 && LA7_0<=34)) ) {
            	            alt7=1;
            	        }


            	        switch (alt7) {
            	    	case 1 :
            	    	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:343:1: (lv_value_6_0= ruleWMLKeyValue )
            	    	    {
            	    	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:343:1: (lv_value_6_0= ruleWMLKeyValue )
            	    	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:344:3: lv_value_6_0= ruleWMLKeyValue
            	    	    {
            	    	     
            	    	    	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_3_3_0(), currentNode); 
            	    	    	    
            	    	    pushFollow(FOLLOW_ruleWMLKeyValue_in_ruleWMLKey528);
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
            	    	    break;

            	    	default :
            	    	    if ( cnt7 >= 1 ) break loop7;
            	                EarlyExitException eee =
            	                    new EarlyExitException(7, input);
            	                throw eee;
            	        }
            	        cnt7++;
            	    } while (true);


            	    }
            	    break;

            	default :
            	    break loop8;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:366:5: ( ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:367:1: ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:367:1: ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:368:1: (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:368:1: (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT )
            int alt9=2;
            int LA9_0 = input.LA(1);

            if ( (LA9_0==RULE_EOL) ) {
                alt9=1;
            }
            else if ( (LA9_0==RULE_SL_COMMENT) ) {
                alt9=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("368:1: (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT )", 9, 0, input);

                throw nvae;
            }
            switch (alt9) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:369:3: lv_eol_7_1= RULE_EOL
                    {
                    lv_eol_7_1=(Token)input.LT(1);
                    match(input,RULE_EOL,FOLLOW_RULE_EOL_in_ruleWMLKey550); 

                    			createLeafNode(grammarAccess.getWMLKeyAccess().getEolEOLTerminalRuleCall_4_0_0(), "eol"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"eol",
                    	        		lv_eol_7_1, 
                    	        		"EOL", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:390:8: lv_eol_7_2= RULE_SL_COMMENT
                    {
                    lv_eol_7_2=(Token)input.LT(1);
                    match(input,RULE_SL_COMMENT,FOLLOW_RULE_SL_COMMENT_in_ruleWMLKey570); 

                    			createLeafNode(grammarAccess.getWMLKeyAccess().getEolSL_COMMENTTerminalRuleCall_4_0_1(), "eol"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"eol",
                    	        		lv_eol_7_2, 
                    	        		"SL_COMMENT", 
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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:425:1: entryRuleWMLKeyValue returns [EObject current=null] : iv_ruleWMLKeyValue= ruleWMLKeyValue EOF ;
    public final EObject entryRuleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKeyValue = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:426:2: (iv_ruleWMLKeyValue= ruleWMLKeyValue EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:427:2: iv_ruleWMLKeyValue= ruleWMLKeyValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue618);
            iv_ruleWMLKeyValue=ruleWMLKeyValue();
            _fsp--;

             current =iv_ruleWMLKeyValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue628); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:434:1: ruleWMLKeyValue returns [EObject current=null] : ( ( () ruleWMLValue ) | this_WMLMacroCall_2= ruleWMLMacroCall | this_WMLLuaCode_3= ruleWMLLuaCode | this_WMLArrayCall_4= ruleWMLArrayCall ) ;
    public final EObject ruleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject this_WMLMacroCall_2 = null;

        EObject this_WMLLuaCode_3 = null;

        EObject this_WMLArrayCall_4 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:439:6: ( ( ( () ruleWMLValue ) | this_WMLMacroCall_2= ruleWMLMacroCall | this_WMLLuaCode_3= ruleWMLLuaCode | this_WMLArrayCall_4= ruleWMLArrayCall ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:440:1: ( ( () ruleWMLValue ) | this_WMLMacroCall_2= ruleWMLMacroCall | this_WMLLuaCode_3= ruleWMLLuaCode | this_WMLArrayCall_4= ruleWMLArrayCall )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:440:1: ( ( () ruleWMLValue ) | this_WMLMacroCall_2= ruleWMLMacroCall | this_WMLLuaCode_3= ruleWMLLuaCode | this_WMLArrayCall_4= ruleWMLArrayCall )
            int alt10=4;
            switch ( input.LA(1) ) {
            case RULE_ID:
            case RULE_STRING:
            case RULE_ANY_OTHER:
            case 26:
            case 27:
            case 29:
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
                {
                alt10=1;
                }
                break;
            case 25:
                {
                alt10=2;
                }
                break;
            case RULE_LUA_CODE:
                {
                alt10=3;
                }
                break;
            case 20:
                {
                alt10=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("440:1: ( ( () ruleWMLValue ) | this_WMLMacroCall_2= ruleWMLMacroCall | this_WMLLuaCode_3= ruleWMLLuaCode | this_WMLArrayCall_4= ruleWMLArrayCall )", 10, 0, input);

                throw nvae;
            }

            switch (alt10) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:440:2: ( () ruleWMLValue )
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:440:2: ( () ruleWMLValue )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:440:3: () ruleWMLValue
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:440:3: ()
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:441:5: 
                    {
                     
                            temp=factory.create(grammarAccess.getWMLKeyValueAccess().getWMLKeyValueAction_0_0().getType().getClassifier());
                            current = temp; 
                            temp = null;
                            CompositeNode newNode = createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLKeyValueAction_0_0(), currentNode.getParent());
                        newNode.getChildren().add(currentNode);
                        moveLookaheadInfo(currentNode, newNode);
                        currentNode = newNode; 
                            associateNodeWithAstElement(currentNode, current); 
                        

                    }

                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLKeyValue679);
                    ruleWMLValue();
                    _fsp--;

                     
                            currentNode = currentNode.getParent();
                        

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:461:5: this_WMLMacroCall_2= ruleWMLMacroCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyValue707);
                    this_WMLMacroCall_2=ruleWMLMacroCall();
                    _fsp--;

                     
                            current = this_WMLMacroCall_2; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:471:5: this_WMLLuaCode_3= ruleWMLLuaCode
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLLuaCode_in_ruleWMLKeyValue734);
                    this_WMLLuaCode_3=ruleWMLLuaCode();
                    _fsp--;

                     
                            current = this_WMLLuaCode_3; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:481:5: this_WMLArrayCall_4= ruleWMLArrayCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLArrayCallParserRuleCall_3(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLArrayCall_in_ruleWMLKeyValue761);
                    this_WMLArrayCall_4=ruleWMLArrayCall();
                    _fsp--;

                     
                            current = this_WMLArrayCall_4; 
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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:497:1: entryRuleWMLMacroCall returns [EObject current=null] : iv_ruleWMLMacroCall= ruleWMLMacroCall EOF ;
    public final EObject entryRuleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroCall = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:498:2: (iv_ruleWMLMacroCall= ruleWMLMacroCall EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:499:2: iv_ruleWMLMacroCall= ruleWMLMacroCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall796);
            iv_ruleWMLMacroCall=ruleWMLMacroCall();
            _fsp--;

             current =iv_ruleWMLMacroCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCall806); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:506:1: ruleWMLMacroCall returns [EObject current=null] : ( '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( ( (lv_params_4_0= ruleWMLMacroParameter ) ) | ( (lv_extraMacros_5_0= ruleWMLMacroCall ) ) )* '}' ) ;
    public final EObject ruleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        Token lv_point_1_0=null;
        Token lv_relative_2_0=null;
        Token lv_name_3_0=null;
        AntlrDatatypeRuleToken lv_params_4_0 = null;

        EObject lv_extraMacros_5_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:511:6: ( ( '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( ( (lv_params_4_0= ruleWMLMacroParameter ) ) | ( (lv_extraMacros_5_0= ruleWMLMacroCall ) ) )* '}' ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:512:1: ( '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( ( (lv_params_4_0= ruleWMLMacroParameter ) ) | ( (lv_extraMacros_5_0= ruleWMLMacroCall ) ) )* '}' )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:512:1: ( '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( ( (lv_params_4_0= ruleWMLMacroParameter ) ) | ( (lv_extraMacros_5_0= ruleWMLMacroCall ) ) )* '}' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:512:3: '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( ( (lv_params_4_0= ruleWMLMacroParameter ) ) | ( (lv_extraMacros_5_0= ruleWMLMacroCall ) ) )* '}'
            {
            match(input,25,FOLLOW_25_in_ruleWMLMacroCall841); 

                    createLeafNode(grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:516:1: ( (lv_point_1_0= './' ) )?
            int alt11=2;
            int LA11_0 = input.LA(1);

            if ( (LA11_0==26) ) {
                alt11=1;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:517:1: (lv_point_1_0= './' )
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:517:1: (lv_point_1_0= './' )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:518:3: lv_point_1_0= './'
                    {
                    lv_point_1_0=(Token)input.LT(1);
                    match(input,26,FOLLOW_26_in_ruleWMLMacroCall859); 

                            createLeafNode(grammarAccess.getWMLMacroCallAccess().getPointFullStopSolidusKeyword_1_0(), "point"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "point", lv_point_1_0, "./", lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }


                    }
                    break;

            }

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:537:3: ( (lv_relative_2_0= '~' ) )?
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0==27) ) {
                alt12=1;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:538:1: (lv_relative_2_0= '~' )
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:538:1: (lv_relative_2_0= '~' )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:539:3: lv_relative_2_0= '~'
                    {
                    lv_relative_2_0=(Token)input.LT(1);
                    match(input,27,FOLLOW_27_in_ruleWMLMacroCall891); 

                            createLeafNode(grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0(), "relative"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "relative", lv_relative_2_0, "~", lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }


                    }
                    break;

            }

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:558:3: ( (lv_name_3_0= RULE_ID ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:559:1: (lv_name_3_0= RULE_ID )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:559:1: (lv_name_3_0= RULE_ID )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:560:3: lv_name_3_0= RULE_ID
            {
            lv_name_3_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacroCall922); 

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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:582:2: ( ( (lv_params_4_0= ruleWMLMacroParameter ) ) | ( (lv_extraMacros_5_0= ruleWMLMacroCall ) ) )*
            loop13:
            do {
                int alt13=3;
                int LA13_0 = input.LA(1);

                if ( (LA13_0==RULE_ID||(LA13_0>=RULE_STRING && LA13_0<=RULE_ANY_OTHER)||(LA13_0>=20 && LA13_0<=24)||(LA13_0>=26 && LA13_0<=27)||(LA13_0>=29 && LA13_0<=34)) ) {
                    alt13=1;
                }
                else if ( (LA13_0==25) ) {
                    alt13=2;
                }


                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:582:3: ( (lv_params_4_0= ruleWMLMacroParameter ) )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:582:3: ( (lv_params_4_0= ruleWMLMacroParameter ) )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:583:1: (lv_params_4_0= ruleWMLMacroParameter )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:583:1: (lv_params_4_0= ruleWMLMacroParameter )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:584:3: lv_params_4_0= ruleWMLMacroParameter
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getParamsWMLMacroParameterParserRuleCall_4_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroParameter_in_ruleWMLMacroCall949);
            	    lv_params_4_0=ruleWMLMacroParameter();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"params",
            	    	        		lv_params_4_0, 
            	    	        		"WMLMacroParameter", 
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
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:607:6: ( (lv_extraMacros_5_0= ruleWMLMacroCall ) )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:607:6: ( (lv_extraMacros_5_0= ruleWMLMacroCall ) )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:608:1: (lv_extraMacros_5_0= ruleWMLMacroCall )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:608:1: (lv_extraMacros_5_0= ruleWMLMacroCall )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:609:3: lv_extraMacros_5_0= ruleWMLMacroCall
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getExtraMacrosWMLMacroCallParserRuleCall_4_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroCall976);
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

            match(input,28,FOLLOW_28_in_ruleWMLMacroCall988); 

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


    // $ANTLR start entryRuleWMLArrayCall
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:643:1: entryRuleWMLArrayCall returns [EObject current=null] : iv_ruleWMLArrayCall= ruleWMLArrayCall EOF ;
    public final EObject entryRuleWMLArrayCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLArrayCall = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:644:2: (iv_ruleWMLArrayCall= ruleWMLArrayCall EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:645:2: iv_ruleWMLArrayCall= ruleWMLArrayCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLArrayCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall1024);
            iv_ruleWMLArrayCall=ruleWMLArrayCall();
            _fsp--;

             current =iv_ruleWMLArrayCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLArrayCall1034); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:652:1: ruleWMLArrayCall returns [EObject current=null] : ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' ) ;
    public final EObject ruleWMLArrayCall() throws RecognitionException {
        EObject current = null;

        AntlrDatatypeRuleToken lv_value_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:657:6: ( ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:658:1: ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:658:1: ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:658:3: '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']'
            {
            match(input,20,FOLLOW_20_in_ruleWMLArrayCall1069); 

                    createLeafNode(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:662:1: ( (lv_value_1_0= ruleWMLValue ) )+
            int cnt14=0;
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( (LA14_0==RULE_ID||(LA14_0>=RULE_STRING && LA14_0<=RULE_ANY_OTHER)||(LA14_0>=26 && LA14_0<=27)||(LA14_0>=29 && LA14_0<=34)) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:663:1: (lv_value_1_0= ruleWMLValue )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:663:1: (lv_value_1_0= ruleWMLValue )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:664:3: lv_value_1_0= ruleWMLValue
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLArrayCall1090);
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

            match(input,22,FOLLOW_22_in_ruleWMLArrayCall1101); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:698:1: entryRuleWMLMacroDefine returns [EObject current=null] : iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF ;
    public final EObject entryRuleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroDefine = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:699:2: (iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:700:2: iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroDefineRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1137);
            iv_ruleWMLMacroDefine=ruleWMLMacroDefine();
            _fsp--;

             current =iv_ruleWMLMacroDefine; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroDefine1147); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:707:1: ruleWMLMacroDefine returns [EObject current=null] : ( ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) ) ) ;
    public final EObject ruleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;
        Token lv_endName_2_0=null;
        EObject lv_Expressions_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:712:6: ( ( ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:713:1: ( ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:713:1: ( ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:713:2: ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:713:2: ( (lv_name_0_0= RULE_DEFINE ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:714:1: (lv_name_0_0= RULE_DEFINE )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:714:1: (lv_name_0_0= RULE_DEFINE )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:715:3: lv_name_0_0= RULE_DEFINE
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1189); 

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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:737:2: ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )*
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( (LA15_0==RULE_ID||LA15_0==RULE_DEFINE||(LA15_0>=RULE_IFDEF && LA15_0<=RULE_IFNHAVE)||LA15_0==RULE_TEXTDOMAIN||(LA15_0>=RULE_STRING && LA15_0<=RULE_ANY_OTHER)||LA15_0==20||(LA15_0>=25 && LA15_0<=27)||(LA15_0>=29 && LA15_0<=34)) ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:738:1: (lv_Expressions_1_0= ruleWMLValuedExpression )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:738:1: (lv_Expressions_1_0= ruleWMLValuedExpression )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:739:3: lv_Expressions_1_0= ruleWMLValuedExpression
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValuedExpression_in_ruleWMLMacroDefine1215);
            	    lv_Expressions_1_0=ruleWMLValuedExpression();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Expressions",
            	    	        		lv_Expressions_1_0, 
            	    	        		"WMLValuedExpression", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop15;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:761:3: ( (lv_endName_2_0= RULE_ENDDEF ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:762:1: (lv_endName_2_0= RULE_ENDDEF )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:762:1: (lv_endName_2_0= RULE_ENDDEF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:763:3: lv_endName_2_0= RULE_ENDDEF
            {
            lv_endName_2_0=(Token)input.LT(1);
            match(input,RULE_ENDDEF,FOLLOW_RULE_ENDDEF_in_ruleWMLMacroDefine1233); 

            			createLeafNode(grammarAccess.getWMLMacroDefineAccess().getEndNameENDDEFTerminalRuleCall_2_0(), "endName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLMacroDefineRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"endName",
            	        		lv_endName_2_0, 
            	        		"ENDDEF", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

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


    // $ANTLR start entryRuleWMLPreprocIF
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:793:1: entryRuleWMLPreprocIF returns [EObject current=null] : iv_ruleWMLPreprocIF= ruleWMLPreprocIF EOF ;
    public final EObject entryRuleWMLPreprocIF() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLPreprocIF = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:794:2: (iv_ruleWMLPreprocIF= ruleWMLPreprocIF EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:795:2: iv_ruleWMLPreprocIF= ruleWMLPreprocIF EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLPreprocIFRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLPreprocIF_in_entryRuleWMLPreprocIF1274);
            iv_ruleWMLPreprocIF=ruleWMLPreprocIF();
            _fsp--;

             current =iv_ruleWMLPreprocIF; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLPreprocIF1284); 

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
    // $ANTLR end entryRuleWMLPreprocIF


    // $ANTLR start ruleWMLPreprocIF
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:802:1: ruleWMLPreprocIF returns [EObject current=null] : ( ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( ( (lv_Expressions_1_0= ruleWMLValuedExpression ) ) | ( (lv_Elses_2_0= RULE_ELSE ) ) )* ( (lv_endName_3_0= RULE_ENDIF ) ) ) ;
    public final EObject ruleWMLPreprocIF() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_1=null;
        Token lv_name_0_2=null;
        Token lv_name_0_3=null;
        Token lv_name_0_4=null;
        Token lv_Elses_2_0=null;
        Token lv_endName_3_0=null;
        EObject lv_Expressions_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:807:6: ( ( ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( ( (lv_Expressions_1_0= ruleWMLValuedExpression ) ) | ( (lv_Elses_2_0= RULE_ELSE ) ) )* ( (lv_endName_3_0= RULE_ENDIF ) ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:808:1: ( ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( ( (lv_Expressions_1_0= ruleWMLValuedExpression ) ) | ( (lv_Elses_2_0= RULE_ELSE ) ) )* ( (lv_endName_3_0= RULE_ENDIF ) ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:808:1: ( ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( ( (lv_Expressions_1_0= ruleWMLValuedExpression ) ) | ( (lv_Elses_2_0= RULE_ELSE ) ) )* ( (lv_endName_3_0= RULE_ENDIF ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:808:2: ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( ( (lv_Expressions_1_0= ruleWMLValuedExpression ) ) | ( (lv_Elses_2_0= RULE_ELSE ) ) )* ( (lv_endName_3_0= RULE_ENDIF ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:808:2: ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:809:1: ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:809:1: ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:810:1: (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:810:1: (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE )
            int alt16=4;
            switch ( input.LA(1) ) {
            case RULE_IFDEF:
                {
                alt16=1;
                }
                break;
            case RULE_IFNDEF:
                {
                alt16=2;
                }
                break;
            case RULE_IFHAVE:
                {
                alt16=3;
                }
                break;
            case RULE_IFNHAVE:
                {
                alt16=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("810:1: (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE )", 16, 0, input);

                throw nvae;
            }

            switch (alt16) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:811:3: lv_name_0_1= RULE_IFDEF
                    {
                    lv_name_0_1=(Token)input.LT(1);
                    match(input,RULE_IFDEF,FOLLOW_RULE_IFDEF_in_ruleWMLPreprocIF1328); 

                    			createLeafNode(grammarAccess.getWMLPreprocIFAccess().getNameIFDEFTerminalRuleCall_0_0_0(), "name"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLPreprocIFRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"name",
                    	        		lv_name_0_1, 
                    	        		"IFDEF", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:832:8: lv_name_0_2= RULE_IFNDEF
                    {
                    lv_name_0_2=(Token)input.LT(1);
                    match(input,RULE_IFNDEF,FOLLOW_RULE_IFNDEF_in_ruleWMLPreprocIF1348); 

                    			createLeafNode(grammarAccess.getWMLPreprocIFAccess().getNameIFNDEFTerminalRuleCall_0_0_1(), "name"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLPreprocIFRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"name",
                    	        		lv_name_0_2, 
                    	        		"IFNDEF", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:853:8: lv_name_0_3= RULE_IFHAVE
                    {
                    lv_name_0_3=(Token)input.LT(1);
                    match(input,RULE_IFHAVE,FOLLOW_RULE_IFHAVE_in_ruleWMLPreprocIF1368); 

                    			createLeafNode(grammarAccess.getWMLPreprocIFAccess().getNameIFHAVETerminalRuleCall_0_0_2(), "name"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLPreprocIFRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"name",
                    	        		lv_name_0_3, 
                    	        		"IFHAVE", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:874:8: lv_name_0_4= RULE_IFNHAVE
                    {
                    lv_name_0_4=(Token)input.LT(1);
                    match(input,RULE_IFNHAVE,FOLLOW_RULE_IFNHAVE_in_ruleWMLPreprocIF1388); 

                    			createLeafNode(grammarAccess.getWMLPreprocIFAccess().getNameIFNHAVETerminalRuleCall_0_0_3(), "name"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLPreprocIFRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"name",
                    	        		lv_name_0_4, 
                    	        		"IFNHAVE", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;

            }


            }


            }

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:898:2: ( ( (lv_Expressions_1_0= ruleWMLValuedExpression ) ) | ( (lv_Elses_2_0= RULE_ELSE ) ) )*
            loop17:
            do {
                int alt17=3;
                int LA17_0 = input.LA(1);

                if ( (LA17_0==RULE_ID||LA17_0==RULE_DEFINE||(LA17_0>=RULE_IFDEF && LA17_0<=RULE_IFNHAVE)||LA17_0==RULE_TEXTDOMAIN||(LA17_0>=RULE_STRING && LA17_0<=RULE_ANY_OTHER)||LA17_0==20||(LA17_0>=25 && LA17_0<=27)||(LA17_0>=29 && LA17_0<=34)) ) {
                    alt17=1;
                }
                else if ( (LA17_0==RULE_ELSE) ) {
                    alt17=2;
                }


                switch (alt17) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:898:3: ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:898:3: ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:899:1: (lv_Expressions_1_0= ruleWMLValuedExpression )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:899:1: (lv_Expressions_1_0= ruleWMLValuedExpression )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:900:3: lv_Expressions_1_0= ruleWMLValuedExpression
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLPreprocIFAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValuedExpression_in_ruleWMLPreprocIF1418);
            	    lv_Expressions_1_0=ruleWMLValuedExpression();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLPreprocIFRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Expressions",
            	    	        		lv_Expressions_1_0, 
            	    	        		"WMLValuedExpression", 
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
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:923:6: ( (lv_Elses_2_0= RULE_ELSE ) )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:923:6: ( (lv_Elses_2_0= RULE_ELSE ) )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:924:1: (lv_Elses_2_0= RULE_ELSE )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:924:1: (lv_Elses_2_0= RULE_ELSE )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:925:3: lv_Elses_2_0= RULE_ELSE
            	    {
            	    lv_Elses_2_0=(Token)input.LT(1);
            	    match(input,RULE_ELSE,FOLLOW_RULE_ELSE_in_ruleWMLPreprocIF1441); 

            	    			createLeafNode(grammarAccess.getWMLPreprocIFAccess().getElsesELSETerminalRuleCall_1_1_0(), "Elses"); 
            	    		

            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLPreprocIFRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode, current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Elses",
            	    	        		lv_Elses_2_0, 
            	    	        		"ELSE", 
            	    	        		lastConsumedNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	    

            	    }


            	    }


            	    }
            	    break;

            	default :
            	    break loop17;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:947:4: ( (lv_endName_3_0= RULE_ENDIF ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:948:1: (lv_endName_3_0= RULE_ENDIF )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:948:1: (lv_endName_3_0= RULE_ENDIF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:949:3: lv_endName_3_0= RULE_ENDIF
            {
            lv_endName_3_0=(Token)input.LT(1);
            match(input,RULE_ENDIF,FOLLOW_RULE_ENDIF_in_ruleWMLPreprocIF1465); 

            			createLeafNode(grammarAccess.getWMLPreprocIFAccess().getEndNameENDIFTerminalRuleCall_2_0(), "endName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLPreprocIFRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"endName",
            	        		lv_endName_3_0, 
            	        		"ENDIF", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

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
    // $ANTLR end ruleWMLPreprocIF


    // $ANTLR start entryRuleWMLRootExpression
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:979:1: entryRuleWMLRootExpression returns [EObject current=null] : iv_ruleWMLRootExpression= ruleWMLRootExpression EOF ;
    public final EObject entryRuleWMLRootExpression() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLRootExpression = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:980:2: (iv_ruleWMLRootExpression= ruleWMLRootExpression EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:981:2: iv_ruleWMLRootExpression= ruleWMLRootExpression EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLRootExpressionRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLRootExpression_in_entryRuleWMLRootExpression1506);
            iv_ruleWMLRootExpression=ruleWMLRootExpression();
            _fsp--;

             current =iv_ruleWMLRootExpression; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLRootExpression1516); 

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
    // $ANTLR end entryRuleWMLRootExpression


    // $ANTLR start ruleWMLRootExpression
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:988:1: ruleWMLRootExpression returns [EObject current=null] : (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF ) ;
    public final EObject ruleWMLRootExpression() throws RecognitionException {
        EObject current = null;

        EObject this_WMLTag_0 = null;

        EObject this_WMLMacroCall_1 = null;

        EObject this_WMLMacroDefine_2 = null;

        EObject this_WMLTextdomain_3 = null;

        EObject this_WMLPreprocIF_4 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:993:6: ( (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:994:1: (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:994:1: (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF )
            int alt18=5;
            switch ( input.LA(1) ) {
            case 20:
                {
                alt18=1;
                }
                break;
            case 25:
                {
                alt18=2;
                }
                break;
            case RULE_DEFINE:
                {
                alt18=3;
                }
                break;
            case RULE_TEXTDOMAIN:
                {
                alt18=4;
                }
                break;
            case RULE_IFDEF:
            case RULE_IFNDEF:
            case RULE_IFHAVE:
            case RULE_IFNHAVE:
                {
                alt18=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("994:1: (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF )", 18, 0, input);

                throw nvae;
            }

            switch (alt18) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:995:5: this_WMLTag_0= ruleWMLTag
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLTagParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLRootExpression1563);
                    this_WMLTag_0=ruleWMLTag();
                    _fsp--;

                     
                            current = this_WMLTag_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1005:5: this_WMLMacroCall_1= ruleWMLMacroCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLMacroCallParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLRootExpression1590);
                    this_WMLMacroCall_1=ruleWMLMacroCall();
                    _fsp--;

                     
                            current = this_WMLMacroCall_1; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1015:5: this_WMLMacroDefine_2= ruleWMLMacroDefine
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLMacroDefineParserRuleCall_2(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroDefine_in_ruleWMLRootExpression1617);
                    this_WMLMacroDefine_2=ruleWMLMacroDefine();
                    _fsp--;

                     
                            current = this_WMLMacroDefine_2; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1025:5: this_WMLTextdomain_3= ruleWMLTextdomain
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLTextdomainParserRuleCall_3(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLTextdomain_in_ruleWMLRootExpression1644);
                    this_WMLTextdomain_3=ruleWMLTextdomain();
                    _fsp--;

                     
                            current = this_WMLTextdomain_3; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 5 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1035:5: this_WMLPreprocIF_4= ruleWMLPreprocIF
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLPreprocIFParserRuleCall_4(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLPreprocIF_in_ruleWMLRootExpression1671);
                    this_WMLPreprocIF_4=ruleWMLPreprocIF();
                    _fsp--;

                     
                            current = this_WMLPreprocIF_4; 
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
    // $ANTLR end ruleWMLRootExpression


    // $ANTLR start entryRuleWMLExpression
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1051:1: entryRuleWMLExpression returns [EObject current=null] : iv_ruleWMLExpression= ruleWMLExpression EOF ;
    public final EObject entryRuleWMLExpression() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLExpression = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1052:2: (iv_ruleWMLExpression= ruleWMLExpression EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1053:2: iv_ruleWMLExpression= ruleWMLExpression EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLExpressionRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLExpression_in_entryRuleWMLExpression1706);
            iv_ruleWMLExpression=ruleWMLExpression();
            _fsp--;

             current =iv_ruleWMLExpression; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLExpression1716); 

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
    // $ANTLR end entryRuleWMLExpression


    // $ANTLR start ruleWMLExpression
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1060:1: ruleWMLExpression returns [EObject current=null] : (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey ) ;
    public final EObject ruleWMLExpression() throws RecognitionException {
        EObject current = null;

        EObject this_WMLRootExpression_0 = null;

        EObject this_WMLKey_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1065:6: ( (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1066:1: (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1066:1: (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey )
            int alt19=2;
            int LA19_0 = input.LA(1);

            if ( (LA19_0==RULE_DEFINE||(LA19_0>=RULE_IFDEF && LA19_0<=RULE_IFNHAVE)||LA19_0==RULE_TEXTDOMAIN||LA19_0==20||LA19_0==25) ) {
                alt19=1;
            }
            else if ( (LA19_0==RULE_ID) ) {
                alt19=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("1066:1: (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey )", 19, 0, input);

                throw nvae;
            }
            switch (alt19) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1067:5: this_WMLRootExpression_0= ruleWMLRootExpression
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLExpressionAccess().getWMLRootExpressionParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLRootExpression_in_ruleWMLExpression1763);
                    this_WMLRootExpression_0=ruleWMLRootExpression();
                    _fsp--;

                     
                            current = this_WMLRootExpression_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1077:5: this_WMLKey_1= ruleWMLKey
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLExpressionAccess().getWMLKeyParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLExpression1790);
                    this_WMLKey_1=ruleWMLKey();
                    _fsp--;

                     
                            current = this_WMLKey_1; 
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
    // $ANTLR end ruleWMLExpression


    // $ANTLR start entryRuleWMLValuedExpression
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1093:1: entryRuleWMLValuedExpression returns [EObject current=null] : iv_ruleWMLValuedExpression= ruleWMLValuedExpression EOF ;
    public final EObject entryRuleWMLValuedExpression() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLValuedExpression = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1094:2: (iv_ruleWMLValuedExpression= ruleWMLValuedExpression EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1095:2: iv_ruleWMLValuedExpression= ruleWMLValuedExpression EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLValuedExpressionRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_entryRuleWMLValuedExpression1825);
            iv_ruleWMLValuedExpression=ruleWMLValuedExpression();
            _fsp--;

             current =iv_ruleWMLValuedExpression; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValuedExpression1835); 

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
    // $ANTLR end entryRuleWMLValuedExpression


    // $ANTLR start ruleWMLValuedExpression
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1102:1: ruleWMLValuedExpression returns [EObject current=null] : (this_WMLExpression_0= ruleWMLExpression | ( () ruleWMLValue ) ) ;
    public final EObject ruleWMLValuedExpression() throws RecognitionException {
        EObject current = null;

        EObject this_WMLExpression_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1107:6: ( (this_WMLExpression_0= ruleWMLExpression | ( () ruleWMLValue ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1108:1: (this_WMLExpression_0= ruleWMLExpression | ( () ruleWMLValue ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1108:1: (this_WMLExpression_0= ruleWMLExpression | ( () ruleWMLValue ) )
            int alt20=2;
            switch ( input.LA(1) ) {
            case RULE_DEFINE:
            case RULE_IFDEF:
            case RULE_IFNDEF:
            case RULE_IFHAVE:
            case RULE_IFNHAVE:
            case RULE_TEXTDOMAIN:
            case 20:
            case 25:
                {
                alt20=1;
                }
                break;
            case RULE_ID:
                {
                int LA20_2 = input.LA(2);

                if ( (LA20_2==24) ) {
                    alt20=1;
                }
                else if ( (LA20_2==EOF||LA20_2==RULE_ID||(LA20_2>=RULE_DEFINE && LA20_2<=RULE_TEXTDOMAIN)||(LA20_2>=RULE_STRING && LA20_2<=RULE_ANY_OTHER)||LA20_2==20||(LA20_2>=25 && LA20_2<=27)||(LA20_2>=29 && LA20_2<=34)) ) {
                    alt20=2;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("1108:1: (this_WMLExpression_0= ruleWMLExpression | ( () ruleWMLValue ) )", 20, 2, input);

                    throw nvae;
                }
                }
                break;
            case RULE_STRING:
            case RULE_ANY_OTHER:
            case 26:
            case 27:
            case 29:
            case 30:
            case 31:
            case 32:
            case 33:
            case 34:
                {
                alt20=2;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("1108:1: (this_WMLExpression_0= ruleWMLExpression | ( () ruleWMLValue ) )", 20, 0, input);

                throw nvae;
            }

            switch (alt20) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1109:5: this_WMLExpression_0= ruleWMLExpression
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLValuedExpressionAccess().getWMLExpressionParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLExpression_in_ruleWMLValuedExpression1882);
                    this_WMLExpression_0=ruleWMLExpression();
                    _fsp--;

                     
                            current = this_WMLExpression_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1118:6: ( () ruleWMLValue )
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1118:6: ( () ruleWMLValue )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1118:7: () ruleWMLValue
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1118:7: ()
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1119:5: 
                    {
                     
                            temp=factory.create(grammarAccess.getWMLValuedExpressionAccess().getWMLValuedExpressionAction_1_0().getType().getClassifier());
                            current = temp; 
                            temp = null;
                            CompositeNode newNode = createCompositeNode(grammarAccess.getWMLValuedExpressionAccess().getWMLValuedExpressionAction_1_0(), currentNode.getParent());
                        newNode.getChildren().add(currentNode);
                        moveLookaheadInfo(currentNode, newNode);
                        currentNode = newNode; 
                            associateNodeWithAstElement(currentNode, current); 
                        

                    }

                     
                            currentNode=createCompositeNode(grammarAccess.getWMLValuedExpressionAccess().getWMLValueParserRuleCall_1_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLValuedExpression1913);
                    ruleWMLValue();
                    _fsp--;

                     
                            currentNode = currentNode.getParent();
                        

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
    // $ANTLR end ruleWMLValuedExpression


    // $ANTLR start entryRuleWMLTextdomain
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1145:1: entryRuleWMLTextdomain returns [EObject current=null] : iv_ruleWMLTextdomain= ruleWMLTextdomain EOF ;
    public final EObject entryRuleWMLTextdomain() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTextdomain = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1146:2: (iv_ruleWMLTextdomain= ruleWMLTextdomain EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1147:2: iv_ruleWMLTextdomain= ruleWMLTextdomain EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLTextdomainRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain1949);
            iv_ruleWMLTextdomain=ruleWMLTextdomain();
            _fsp--;

             current =iv_ruleWMLTextdomain; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTextdomain1959); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1154:1: ruleWMLTextdomain returns [EObject current=null] : ( (lv_name_0_0= RULE_TEXTDOMAIN ) ) ;
    public final EObject ruleWMLTextdomain() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1159:6: ( ( (lv_name_0_0= RULE_TEXTDOMAIN ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1160:1: ( (lv_name_0_0= RULE_TEXTDOMAIN ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1160:1: ( (lv_name_0_0= RULE_TEXTDOMAIN ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1161:1: (lv_name_0_0= RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1161:1: (lv_name_0_0= RULE_TEXTDOMAIN )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1162:3: lv_name_0_0= RULE_TEXTDOMAIN
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_ruleWMLTextdomain2000); 

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


    // $ANTLR start entryRuleWMLLuaCode
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1192:1: entryRuleWMLLuaCode returns [EObject current=null] : iv_ruleWMLLuaCode= ruleWMLLuaCode EOF ;
    public final EObject entryRuleWMLLuaCode() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLLuaCode = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1193:2: (iv_ruleWMLLuaCode= ruleWMLLuaCode EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1194:2: iv_ruleWMLLuaCode= ruleWMLLuaCode EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLLuaCodeRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode2040);
            iv_ruleWMLLuaCode=ruleWMLLuaCode();
            _fsp--;

             current =iv_ruleWMLLuaCode; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLLuaCode2050); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1201:1: ruleWMLLuaCode returns [EObject current=null] : ( (lv_value_0_0= RULE_LUA_CODE ) ) ;
    public final EObject ruleWMLLuaCode() throws RecognitionException {
        EObject current = null;

        Token lv_value_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1206:6: ( ( (lv_value_0_0= RULE_LUA_CODE ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1207:1: ( (lv_value_0_0= RULE_LUA_CODE ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1207:1: ( (lv_value_0_0= RULE_LUA_CODE ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1208:1: (lv_value_0_0= RULE_LUA_CODE )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1208:1: (lv_value_0_0= RULE_LUA_CODE )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1209:3: lv_value_0_0= RULE_LUA_CODE
            {
            lv_value_0_0=(Token)input.LT(1);
            match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_ruleWMLLuaCode2091); 

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


    // $ANTLR start entryRuleWMLMacroParameter
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1239:1: entryRuleWMLMacroParameter returns [String current=null] : iv_ruleWMLMacroParameter= ruleWMLMacroParameter EOF ;
    public final String entryRuleWMLMacroParameter() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleWMLMacroParameter = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1240:2: (iv_ruleWMLMacroParameter= ruleWMLMacroParameter EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1241:2: iv_ruleWMLMacroParameter= ruleWMLMacroParameter EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroParameterRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter2132);
            iv_ruleWMLMacroParameter=ruleWMLMacroParameter();
            _fsp--;

             current =iv_ruleWMLMacroParameter.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroParameter2143); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1248:1: ruleWMLMacroParameter returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens ) ;
    public final AntlrDatatypeRuleToken ruleWMLMacroParameter() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        AntlrDatatypeRuleToken this_WMLValue_0 = null;

        AntlrDatatypeRuleToken this_MacroTokens_1 = null;


         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1253:6: ( (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1254:1: (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1254:1: (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens )
            int alt21=2;
            int LA21_0 = input.LA(1);

            if ( (LA21_0==RULE_ID||(LA21_0>=RULE_STRING && LA21_0<=RULE_ANY_OTHER)||(LA21_0>=26 && LA21_0<=27)||(LA21_0>=29 && LA21_0<=34)) ) {
                alt21=1;
            }
            else if ( ((LA21_0>=20 && LA21_0<=24)) ) {
                alt21=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("1254:1: (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens )", 21, 0, input);

                throw nvae;
            }
            switch (alt21) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1255:5: this_WMLValue_0= ruleWMLValue
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getWMLValueParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLMacroParameter2190);
                    this_WMLValue_0=ruleWMLValue();
                    _fsp--;


                    		current.merge(this_WMLValue_0);
                        
                     
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1267:5: this_MacroTokens_1= ruleMacroTokens
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getMacroTokensParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleMacroTokens_in_ruleWMLMacroParameter2223);
                    this_MacroTokens_1=ruleMacroTokens();
                    _fsp--;


                    		current.merge(this_MacroTokens_1);
                        
                     
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
    // $ANTLR end ruleWMLMacroParameter


    // $ANTLR start entryRuleWMLValue
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1285:1: entryRuleWMLValue returns [String current=null] : iv_ruleWMLValue= ruleWMLValue EOF ;
    public final String entryRuleWMLValue() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleWMLValue = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1286:2: (iv_ruleWMLValue= ruleWMLValue EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1287:2: iv_ruleWMLValue= ruleWMLValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLValue_in_entryRuleWMLValue2269);
            iv_ruleWMLValue=ruleWMLValue();
            _fsp--;

             current =iv_ruleWMLValue.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValue2280); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1294:1: ruleWMLValue returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_ID_0= RULE_ID | this_STRING_1= RULE_STRING | kw= '_' | kw= '~' | kw= '.' | kw= './' | kw= '$' | kw= '/' | kw= '(' | kw= ')' | this_ANY_OTHER_10= RULE_ANY_OTHER ) ;
    public final AntlrDatatypeRuleToken ruleWMLValue() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;
        Token this_STRING_1=null;
        Token kw=null;
        Token this_ANY_OTHER_10=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1299:6: ( (this_ID_0= RULE_ID | this_STRING_1= RULE_STRING | kw= '_' | kw= '~' | kw= '.' | kw= './' | kw= '$' | kw= '/' | kw= '(' | kw= ')' | this_ANY_OTHER_10= RULE_ANY_OTHER ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1300:1: (this_ID_0= RULE_ID | this_STRING_1= RULE_STRING | kw= '_' | kw= '~' | kw= '.' | kw= './' | kw= '$' | kw= '/' | kw= '(' | kw= ')' | this_ANY_OTHER_10= RULE_ANY_OTHER )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1300:1: (this_ID_0= RULE_ID | this_STRING_1= RULE_STRING | kw= '_' | kw= '~' | kw= '.' | kw= './' | kw= '$' | kw= '/' | kw= '(' | kw= ')' | this_ANY_OTHER_10= RULE_ANY_OTHER )
            int alt22=11;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt22=1;
                }
                break;
            case RULE_STRING:
                {
                alt22=2;
                }
                break;
            case 29:
                {
                alt22=3;
                }
                break;
            case 27:
                {
                alt22=4;
                }
                break;
            case 30:
                {
                alt22=5;
                }
                break;
            case 26:
                {
                alt22=6;
                }
                break;
            case 31:
                {
                alt22=7;
                }
                break;
            case 32:
                {
                alt22=8;
                }
                break;
            case 33:
                {
                alt22=9;
                }
                break;
            case 34:
                {
                alt22=10;
                }
                break;
            case RULE_ANY_OTHER:
                {
                alt22=11;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("1300:1: (this_ID_0= RULE_ID | this_STRING_1= RULE_STRING | kw= '_' | kw= '~' | kw= '.' | kw= './' | kw= '$' | kw= '/' | kw= '(' | kw= ')' | this_ANY_OTHER_10= RULE_ANY_OTHER )", 22, 0, input);

                throw nvae;
            }

            switch (alt22) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1300:6: this_ID_0= RULE_ID
                    {
                    this_ID_0=(Token)input.LT(1);
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLValue2320); 

                    		current.merge(this_ID_0);
                        
                     
                        createLeafNode(grammarAccess.getWMLValueAccess().getIDTerminalRuleCall_0(), null); 
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1308:10: this_STRING_1= RULE_STRING
                    {
                    this_STRING_1=(Token)input.LT(1);
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLValue2346); 

                    		current.merge(this_STRING_1);
                        
                     
                        createLeafNode(grammarAccess.getWMLValueAccess().getSTRINGTerminalRuleCall_1(), null); 
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1317:2: kw= '_'
                    {
                    kw=(Token)input.LT(1);
                    match(input,29,FOLLOW_29_in_ruleWMLValue2370); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().get_Keyword_2(), null); 
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1324:2: kw= '~'
                    {
                    kw=(Token)input.LT(1);
                    match(input,27,FOLLOW_27_in_ruleWMLValue2389); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getTildeKeyword_3(), null); 
                        

                    }
                    break;
                case 5 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1331:2: kw= '.'
                    {
                    kw=(Token)input.LT(1);
                    match(input,30,FOLLOW_30_in_ruleWMLValue2408); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getFullStopKeyword_4(), null); 
                        

                    }
                    break;
                case 6 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1338:2: kw= './'
                    {
                    kw=(Token)input.LT(1);
                    match(input,26,FOLLOW_26_in_ruleWMLValue2427); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getFullStopSolidusKeyword_5(), null); 
                        

                    }
                    break;
                case 7 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1345:2: kw= '$'
                    {
                    kw=(Token)input.LT(1);
                    match(input,31,FOLLOW_31_in_ruleWMLValue2446); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getDollarSignKeyword_6(), null); 
                        

                    }
                    break;
                case 8 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1352:2: kw= '/'
                    {
                    kw=(Token)input.LT(1);
                    match(input,32,FOLLOW_32_in_ruleWMLValue2465); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getSolidusKeyword_7(), null); 
                        

                    }
                    break;
                case 9 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:2: kw= '('
                    {
                    kw=(Token)input.LT(1);
                    match(input,33,FOLLOW_33_in_ruleWMLValue2484); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getLeftParenthesisKeyword_8(), null); 
                        

                    }
                    break;
                case 10 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1366:2: kw= ')'
                    {
                    kw=(Token)input.LT(1);
                    match(input,34,FOLLOW_34_in_ruleWMLValue2503); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getRightParenthesisKeyword_9(), null); 
                        

                    }
                    break;
                case 11 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1372:10: this_ANY_OTHER_10= RULE_ANY_OTHER
                    {
                    this_ANY_OTHER_10=(Token)input.LT(1);
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_ruleWMLValue2524); 

                    		current.merge(this_ANY_OTHER_10);
                        
                     
                        createLeafNode(grammarAccess.getWMLValueAccess().getANY_OTHERTerminalRuleCall_10(), null); 
                        

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
    // $ANTLR end ruleWMLValue


    // $ANTLR start entryRuleMacroTokens
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1387:1: entryRuleMacroTokens returns [String current=null] : iv_ruleMacroTokens= ruleMacroTokens EOF ;
    public final String entryRuleMacroTokens() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleMacroTokens = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1388:2: (iv_ruleMacroTokens= ruleMacroTokens EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1389:2: iv_ruleMacroTokens= ruleMacroTokens EOF
            {
             currentNode = createCompositeNode(grammarAccess.getMacroTokensRule(), currentNode); 
            pushFollow(FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens2570);
            iv_ruleMacroTokens=ruleMacroTokens();
            _fsp--;

             current =iv_ruleMacroTokens.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleMacroTokens2581); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1396:1: ruleMacroTokens returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (kw= '=' | kw= '[' | kw= ']' | kw= '+' | kw= '[/' ) ;
    public final AntlrDatatypeRuleToken ruleMacroTokens() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1401:6: ( (kw= '=' | kw= '[' | kw= ']' | kw= '+' | kw= '[/' ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1402:1: (kw= '=' | kw= '[' | kw= ']' | kw= '+' | kw= '[/' )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1402:1: (kw= '=' | kw= '[' | kw= ']' | kw= '+' | kw= '[/' )
            int alt23=5;
            switch ( input.LA(1) ) {
            case 24:
                {
                alt23=1;
                }
                break;
            case 20:
                {
                alt23=2;
                }
                break;
            case 22:
                {
                alt23=3;
                }
                break;
            case 21:
                {
                alt23=4;
                }
                break;
            case 23:
                {
                alt23=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("1402:1: (kw= '=' | kw= '[' | kw= ']' | kw= '+' | kw= '[/' )", 23, 0, input);

                throw nvae;
            }

            switch (alt23) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1403:2: kw= '='
                    {
                    kw=(Token)input.LT(1);
                    match(input,24,FOLLOW_24_in_ruleMacroTokens2619); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getMacroTokensAccess().getEqualsSignKeyword_0(), null); 
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1410:2: kw= '['
                    {
                    kw=(Token)input.LT(1);
                    match(input,20,FOLLOW_20_in_ruleMacroTokens2638); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getMacroTokensAccess().getLeftSquareBracketKeyword_1(), null); 
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1417:2: kw= ']'
                    {
                    kw=(Token)input.LT(1);
                    match(input,22,FOLLOW_22_in_ruleMacroTokens2657); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getMacroTokensAccess().getRightSquareBracketKeyword_2(), null); 
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1424:2: kw= '+'
                    {
                    kw=(Token)input.LT(1);
                    match(input,21,FOLLOW_21_in_ruleMacroTokens2676); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getMacroTokensAccess().getPlusSignKeyword_3(), null); 
                        

                    }
                    break;
                case 5 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1431:2: kw= '[/'
                    {
                    kw=(Token)input.LT(1);
                    match(input,23,FOLLOW_23_in_ruleMacroTokens2695); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getMacroTokensAccess().getLeftSquareBracketSolidusKeyword_4(), null); 
                        

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
    // $ANTLR end ruleMacroTokens


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot75 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot85 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_ruleWMLRoot130 = new BitSet(new long[]{0x0000000002109E82L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag166 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag176 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_ruleWMLTag211 = new BitSet(new long[]{0x0000000000200010L});
    public static final BitSet FOLLOW_21_in_ruleWMLTag229 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag260 = new BitSet(new long[]{0x0000000000400000L});
    public static final BitSet FOLLOW_22_in_ruleWMLTag275 = new BitSet(new long[]{0x0000000002909E90L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_ruleWMLTag296 = new BitSet(new long[]{0x0000000002909E90L});
    public static final BitSet FOLLOW_23_in_ruleWMLTag307 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag324 = new BitSet(new long[]{0x0000000000400000L});
    public static final BitSet FOLLOW_22_in_ruleWMLTag339 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey381 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey391 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKey437 = new BitSet(new long[]{0x0000000001000000L});
    public static final BitSet FOLLOW_24_in_ruleWMLKey452 = new BitSet(new long[]{0x00000007EE370070L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLKey473 = new BitSet(new long[]{0x00000007EE370070L});
    public static final BitSet FOLLOW_RULE_EOL_in_ruleWMLKey485 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_21_in_ruleWMLKey496 = new BitSet(new long[]{0x00000007EE170030L});
    public static final BitSet FOLLOW_RULE_EOL_in_ruleWMLKey506 = new BitSet(new long[]{0x00000007EE170010L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLKey528 = new BitSet(new long[]{0x00000007EE370070L});
    public static final BitSet FOLLOW_RULE_EOL_in_ruleWMLKey550 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_SL_COMMENT_in_ruleWMLKey570 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue618 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue628 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLKeyValue679 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyValue707 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_ruleWMLKeyValue734 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_ruleWMLKeyValue761 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall796 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCall806 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_ruleWMLMacroCall841 = new BitSet(new long[]{0x000000000C000010L});
    public static final BitSet FOLLOW_26_in_ruleWMLMacroCall859 = new BitSet(new long[]{0x0000000008000010L});
    public static final BitSet FOLLOW_27_in_ruleWMLMacroCall891 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLMacroCall922 = new BitSet(new long[]{0x00000007FFF60010L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_ruleWMLMacroCall949 = new BitSet(new long[]{0x00000007FFF60010L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroCall976 = new BitSet(new long[]{0x00000007FFF60010L});
    public static final BitSet FOLLOW_28_in_ruleWMLMacroCall988 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall1024 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLArrayCall1034 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_ruleWMLArrayCall1069 = new BitSet(new long[]{0x00000007EC060010L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLArrayCall1090 = new BitSet(new long[]{0x00000007EC460010L});
    public static final BitSet FOLLOW_22_in_ruleWMLArrayCall1101 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1137 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine1147 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1189 = new BitSet(new long[]{0x00000007EE169F90L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_ruleWMLMacroDefine1215 = new BitSet(new long[]{0x00000007EE169F90L});
    public static final BitSet FOLLOW_RULE_ENDDEF_in_ruleWMLMacroDefine1233 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPreprocIF_in_entryRuleWMLPreprocIF1274 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLPreprocIF1284 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFDEF_in_ruleWMLPreprocIF1328 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_IFNDEF_in_ruleWMLPreprocIF1348 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_IFHAVE_in_ruleWMLPreprocIF1368 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_IFNHAVE_in_ruleWMLPreprocIF1388 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_ruleWMLPreprocIF1418 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_ELSE_in_ruleWMLPreprocIF1441 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_ENDIF_in_ruleWMLPreprocIF1465 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_entryRuleWMLRootExpression1506 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRootExpression1516 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLRootExpression1563 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLRootExpression1590 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLRootExpression1617 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_ruleWMLRootExpression1644 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPreprocIF_in_ruleWMLRootExpression1671 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_entryRuleWMLExpression1706 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLExpression1716 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_ruleWMLExpression1763 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLExpression1790 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_entryRuleWMLValuedExpression1825 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValuedExpression1835 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_ruleWMLValuedExpression1882 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLValuedExpression1913 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain1949 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTextdomain1959 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_ruleWMLTextdomain2000 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode2040 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLLuaCode2050 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_ruleWMLLuaCode2091 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter2132 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroParameter2143 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLMacroParameter2190 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_ruleWMLMacroParameter2223 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_entryRuleWMLValue2269 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValue2280 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLValue2320 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLValue2346 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_ruleWMLValue2370 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_ruleWMLValue2389 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_ruleWMLValue2408 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_ruleWMLValue2427 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_ruleWMLValue2446 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_ruleWMLValue2465 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_33_in_ruleWMLValue2484 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_34_in_ruleWMLValue2503 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_ruleWMLValue2524 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens2570 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleMacroTokens2581 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_ruleMacroTokens2619 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_ruleMacroTokens2638 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_ruleMacroTokens2657 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_ruleMacroTokens2676 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_ruleMacroTokens2695 = new BitSet(new long[]{0x0000000000000002L});

}