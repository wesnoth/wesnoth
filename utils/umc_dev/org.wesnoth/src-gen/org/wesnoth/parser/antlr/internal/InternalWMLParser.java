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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:506:1: ruleWMLMacroCall returns [EObject current=null] : ( '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( (lv_Parameters_4_0= ruleWMLMacroCallParameter ) )* '}' ) ;
    public final EObject ruleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        Token lv_point_1_0=null;
        Token lv_relative_2_0=null;
        Token lv_name_3_0=null;
        EObject lv_Parameters_4_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:511:6: ( ( '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( (lv_Parameters_4_0= ruleWMLMacroCallParameter ) )* '}' ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:512:1: ( '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( (lv_Parameters_4_0= ruleWMLMacroCallParameter ) )* '}' )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:512:1: ( '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( (lv_Parameters_4_0= ruleWMLMacroCallParameter ) )* '}' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:512:3: '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( (lv_Parameters_4_0= ruleWMLMacroCallParameter ) )* '}'
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:582:2: ( (lv_Parameters_4_0= ruleWMLMacroCallParameter ) )*
            loop13:
            do {
                int alt13=2;
                int LA13_0 = input.LA(1);

                if ( (LA13_0==RULE_ID||(LA13_0>=RULE_STRING && LA13_0<=RULE_ANY_OTHER)||(LA13_0>=20 && LA13_0<=27)||(LA13_0>=29 && LA13_0<=34)) ) {
                    alt13=1;
                }


                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:583:1: (lv_Parameters_4_0= ruleWMLMacroCallParameter )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:583:1: (lv_Parameters_4_0= ruleWMLMacroCallParameter )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:584:3: lv_Parameters_4_0= ruleWMLMacroCallParameter
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroCallAccess().getParametersWMLMacroCallParameterParserRuleCall_4_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_ruleWMLMacroCall948);
            	    lv_Parameters_4_0=ruleWMLMacroCallParameter();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroCallRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Parameters",
            	    	        		lv_Parameters_4_0, 
            	    	        		"WMLMacroCallParameter", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop13;
                }
            } while (true);

            match(input,28,FOLLOW_28_in_ruleWMLMacroCall959); 

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


    // $ANTLR start entryRuleWMLMacroCallParameter
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:618:1: entryRuleWMLMacroCallParameter returns [EObject current=null] : iv_ruleWMLMacroCallParameter= ruleWMLMacroCallParameter EOF ;
    public final EObject entryRuleWMLMacroCallParameter() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroCallParameter = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:619:2: (iv_ruleWMLMacroCallParameter= ruleWMLMacroCallParameter EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:620:2: iv_ruleWMLMacroCallParameter= ruleWMLMacroCallParameter EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroCallParameterRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_entryRuleWMLMacroCallParameter995);
            iv_ruleWMLMacroCallParameter=ruleWMLMacroCallParameter();
            _fsp--;

             current =iv_ruleWMLMacroCallParameter; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCallParameter1005); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:627:1: ruleWMLMacroCallParameter returns [EObject current=null] : ( ( () ruleWMLMacroParameter ) | this_WMLMacroCall_2= ruleWMLMacroCall ) ;
    public final EObject ruleWMLMacroCallParameter() throws RecognitionException {
        EObject current = null;

        EObject this_WMLMacroCall_2 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:632:6: ( ( ( () ruleWMLMacroParameter ) | this_WMLMacroCall_2= ruleWMLMacroCall ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:633:1: ( ( () ruleWMLMacroParameter ) | this_WMLMacroCall_2= ruleWMLMacroCall )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:633:1: ( ( () ruleWMLMacroParameter ) | this_WMLMacroCall_2= ruleWMLMacroCall )
            int alt14=2;
            int LA14_0 = input.LA(1);

            if ( (LA14_0==RULE_ID||(LA14_0>=RULE_STRING && LA14_0<=RULE_ANY_OTHER)||(LA14_0>=20 && LA14_0<=24)||(LA14_0>=26 && LA14_0<=27)||(LA14_0>=29 && LA14_0<=34)) ) {
                alt14=1;
            }
            else if ( (LA14_0==25) ) {
                alt14=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("633:1: ( ( () ruleWMLMacroParameter ) | this_WMLMacroCall_2= ruleWMLMacroCall )", 14, 0, input);

                throw nvae;
            }
            switch (alt14) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:633:2: ( () ruleWMLMacroParameter )
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:633:2: ( () ruleWMLMacroParameter )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:633:3: () ruleWMLMacroParameter
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:633:3: ()
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:634:5: 
                    {
                     
                            temp=factory.create(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroCallParameterAction_0_0().getType().getClassifier());
                            current = temp; 
                            temp = null;
                            CompositeNode newNode = createCompositeNode(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroCallParameterAction_0_0(), currentNode.getParent());
                        newNode.getChildren().add(currentNode);
                        moveLookaheadInfo(currentNode, newNode);
                        currentNode = newNode; 
                            associateNodeWithAstElement(currentNode, current); 
                        

                    }

                     
                            currentNode=createCompositeNode(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroParameterParserRuleCall_0_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroParameter_in_ruleWMLMacroCallParameter1056);
                    ruleWMLMacroParameter();
                    _fsp--;

                     
                            currentNode = currentNode.getParent();
                        

                    }


                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:654:5: this_WMLMacroCall_2= ruleWMLMacroCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroCallParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroCallParameter1084);
                    this_WMLMacroCall_2=ruleWMLMacroCall();
                    _fsp--;

                     
                            current = this_WMLMacroCall_2; 
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
    // $ANTLR end ruleWMLMacroCallParameter


    // $ANTLR start entryRuleWMLArrayCall
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:670:1: entryRuleWMLArrayCall returns [EObject current=null] : iv_ruleWMLArrayCall= ruleWMLArrayCall EOF ;
    public final EObject entryRuleWMLArrayCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLArrayCall = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:671:2: (iv_ruleWMLArrayCall= ruleWMLArrayCall EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:672:2: iv_ruleWMLArrayCall= ruleWMLArrayCall EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLArrayCallRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall1119);
            iv_ruleWMLArrayCall=ruleWMLArrayCall();
            _fsp--;

             current =iv_ruleWMLArrayCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLArrayCall1129); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:679:1: ruleWMLArrayCall returns [EObject current=null] : ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' ) ;
    public final EObject ruleWMLArrayCall() throws RecognitionException {
        EObject current = null;

        AntlrDatatypeRuleToken lv_value_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:684:6: ( ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:685:1: ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:685:1: ( '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:685:3: '[' ( (lv_value_1_0= ruleWMLValue ) )+ ']'
            {
            match(input,20,FOLLOW_20_in_ruleWMLArrayCall1164); 

                    createLeafNode(grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:689:1: ( (lv_value_1_0= ruleWMLValue ) )+
            int cnt15=0;
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( (LA15_0==RULE_ID||(LA15_0>=RULE_STRING && LA15_0<=RULE_ANY_OTHER)||(LA15_0>=26 && LA15_0<=27)||(LA15_0>=29 && LA15_0<=34)) ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:690:1: (lv_value_1_0= ruleWMLValue )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:690:1: (lv_value_1_0= ruleWMLValue )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:691:3: lv_value_1_0= ruleWMLValue
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLArrayCall1185);
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
            	    if ( cnt15 >= 1 ) break loop15;
                        EarlyExitException eee =
                            new EarlyExitException(15, input);
                        throw eee;
                }
                cnt15++;
            } while (true);

            match(input,22,FOLLOW_22_in_ruleWMLArrayCall1196); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:725:1: entryRuleWMLMacroDefine returns [EObject current=null] : iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF ;
    public final EObject entryRuleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroDefine = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:726:2: (iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:727:2: iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroDefineRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1232);
            iv_ruleWMLMacroDefine=ruleWMLMacroDefine();
            _fsp--;

             current =iv_ruleWMLMacroDefine; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroDefine1242); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:734:1: ruleWMLMacroDefine returns [EObject current=null] : ( ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) ) ) ;
    public final EObject ruleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;
        Token lv_endName_2_0=null;
        EObject lv_Expressions_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:739:6: ( ( ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:740:1: ( ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:740:1: ( ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:740:2: ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:740:2: ( (lv_name_0_0= RULE_DEFINE ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:741:1: (lv_name_0_0= RULE_DEFINE )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:741:1: (lv_name_0_0= RULE_DEFINE )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:742:3: lv_name_0_0= RULE_DEFINE
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1284); 

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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:764:2: ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )*
            loop16:
            do {
                int alt16=2;
                int LA16_0 = input.LA(1);

                if ( (LA16_0==RULE_ID||LA16_0==RULE_DEFINE||(LA16_0>=RULE_IFDEF && LA16_0<=RULE_IFNHAVE)||LA16_0==RULE_TEXTDOMAIN||(LA16_0>=RULE_STRING && LA16_0<=RULE_ANY_OTHER)||LA16_0==20||(LA16_0>=25 && LA16_0<=27)||(LA16_0>=29 && LA16_0<=34)) ) {
                    alt16=1;
                }


                switch (alt16) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:765:1: (lv_Expressions_1_0= ruleWMLValuedExpression )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:765:1: (lv_Expressions_1_0= ruleWMLValuedExpression )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:766:3: lv_Expressions_1_0= ruleWMLValuedExpression
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLMacroDefineAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValuedExpression_in_ruleWMLMacroDefine1310);
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
            	    break loop16;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:788:3: ( (lv_endName_2_0= RULE_ENDDEF ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:789:1: (lv_endName_2_0= RULE_ENDDEF )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:789:1: (lv_endName_2_0= RULE_ENDDEF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:790:3: lv_endName_2_0= RULE_ENDDEF
            {
            lv_endName_2_0=(Token)input.LT(1);
            match(input,RULE_ENDDEF,FOLLOW_RULE_ENDDEF_in_ruleWMLMacroDefine1328); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:820:1: entryRuleWMLPreprocIF returns [EObject current=null] : iv_ruleWMLPreprocIF= ruleWMLPreprocIF EOF ;
    public final EObject entryRuleWMLPreprocIF() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLPreprocIF = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:821:2: (iv_ruleWMLPreprocIF= ruleWMLPreprocIF EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:822:2: iv_ruleWMLPreprocIF= ruleWMLPreprocIF EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLPreprocIFRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLPreprocIF_in_entryRuleWMLPreprocIF1369);
            iv_ruleWMLPreprocIF=ruleWMLPreprocIF();
            _fsp--;

             current =iv_ruleWMLPreprocIF; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLPreprocIF1379); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:829:1: ruleWMLPreprocIF returns [EObject current=null] : ( ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+ )? ( (lv_endName_4_0= RULE_ENDIF ) ) ) ;
    public final EObject ruleWMLPreprocIF() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_1=null;
        Token lv_name_0_2=null;
        Token lv_name_0_3=null;
        Token lv_name_0_4=null;
        Token lv_Elses_2_0=null;
        Token lv_endName_4_0=null;
        EObject lv_Expressions_1_0 = null;

        EObject lv_ElseExpressions_3_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:834:6: ( ( ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+ )? ( (lv_endName_4_0= RULE_ENDIF ) ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:835:1: ( ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+ )? ( (lv_endName_4_0= RULE_ENDIF ) ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:835:1: ( ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+ )? ( (lv_endName_4_0= RULE_ENDIF ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:835:2: ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+ )? ( (lv_endName_4_0= RULE_ENDIF ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:835:2: ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:836:1: ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:836:1: ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:837:1: (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:837:1: (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE )
            int alt17=4;
            switch ( input.LA(1) ) {
            case RULE_IFDEF:
                {
                alt17=1;
                }
                break;
            case RULE_IFNDEF:
                {
                alt17=2;
                }
                break;
            case RULE_IFHAVE:
                {
                alt17=3;
                }
                break;
            case RULE_IFNHAVE:
                {
                alt17=4;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("837:1: (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE )", 17, 0, input);

                throw nvae;
            }

            switch (alt17) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:838:3: lv_name_0_1= RULE_IFDEF
                    {
                    lv_name_0_1=(Token)input.LT(1);
                    match(input,RULE_IFDEF,FOLLOW_RULE_IFDEF_in_ruleWMLPreprocIF1423); 

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
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:859:8: lv_name_0_2= RULE_IFNDEF
                    {
                    lv_name_0_2=(Token)input.LT(1);
                    match(input,RULE_IFNDEF,FOLLOW_RULE_IFNDEF_in_ruleWMLPreprocIF1443); 

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
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:880:8: lv_name_0_3= RULE_IFHAVE
                    {
                    lv_name_0_3=(Token)input.LT(1);
                    match(input,RULE_IFHAVE,FOLLOW_RULE_IFHAVE_in_ruleWMLPreprocIF1463); 

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
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:901:8: lv_name_0_4= RULE_IFNHAVE
                    {
                    lv_name_0_4=(Token)input.LT(1);
                    match(input,RULE_IFNHAVE,FOLLOW_RULE_IFNHAVE_in_ruleWMLPreprocIF1483); 

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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:925:2: ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )*
            loop18:
            do {
                int alt18=2;
                int LA18_0 = input.LA(1);

                if ( (LA18_0==RULE_ID||LA18_0==RULE_DEFINE||(LA18_0>=RULE_IFDEF && LA18_0<=RULE_IFNHAVE)||LA18_0==RULE_TEXTDOMAIN||(LA18_0>=RULE_STRING && LA18_0<=RULE_ANY_OTHER)||LA18_0==20||(LA18_0>=25 && LA18_0<=27)||(LA18_0>=29 && LA18_0<=34)) ) {
                    alt18=1;
                }


                switch (alt18) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:926:1: (lv_Expressions_1_0= ruleWMLValuedExpression )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:926:1: (lv_Expressions_1_0= ruleWMLValuedExpression )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:927:3: lv_Expressions_1_0= ruleWMLValuedExpression
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLPreprocIFAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValuedExpression_in_ruleWMLPreprocIF1512);
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
            	    break;

            	default :
            	    break loop18;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:949:3: ( ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+ )?
            int alt20=2;
            int LA20_0 = input.LA(1);

            if ( (LA20_0==RULE_ELSE) ) {
                alt20=1;
            }
            switch (alt20) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:949:4: ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:949:4: ( (lv_Elses_2_0= RULE_ELSE ) )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:950:1: (lv_Elses_2_0= RULE_ELSE )
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:950:1: (lv_Elses_2_0= RULE_ELSE )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:951:3: lv_Elses_2_0= RULE_ELSE
                    {
                    lv_Elses_2_0=(Token)input.LT(1);
                    match(input,RULE_ELSE,FOLLOW_RULE_ELSE_in_ruleWMLPreprocIF1531); 

                    			createLeafNode(grammarAccess.getWMLPreprocIFAccess().getElsesELSETerminalRuleCall_2_0_0(), "Elses"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLPreprocIFRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
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

                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:973:2: ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+
                    int cnt19=0;
                    loop19:
                    do {
                        int alt19=2;
                        int LA19_0 = input.LA(1);

                        if ( (LA19_0==RULE_ID||LA19_0==RULE_DEFINE||(LA19_0>=RULE_IFDEF && LA19_0<=RULE_IFNHAVE)||LA19_0==RULE_TEXTDOMAIN||(LA19_0>=RULE_STRING && LA19_0<=RULE_ANY_OTHER)||LA19_0==20||(LA19_0>=25 && LA19_0<=27)||(LA19_0>=29 && LA19_0<=34)) ) {
                            alt19=1;
                        }


                        switch (alt19) {
                    	case 1 :
                    	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:974:1: (lv_ElseExpressions_3_0= ruleWMLValuedExpression )
                    	    {
                    	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:974:1: (lv_ElseExpressions_3_0= ruleWMLValuedExpression )
                    	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:975:3: lv_ElseExpressions_3_0= ruleWMLValuedExpression
                    	    {
                    	     
                    	    	        currentNode=createCompositeNode(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsWMLValuedExpressionParserRuleCall_2_1_0(), currentNode); 
                    	    	    
                    	    pushFollow(FOLLOW_ruleWMLValuedExpression_in_ruleWMLPreprocIF1557);
                    	    lv_ElseExpressions_3_0=ruleWMLValuedExpression();
                    	    _fsp--;


                    	    	        if (current==null) {
                    	    	            current = factory.create(grammarAccess.getWMLPreprocIFRule().getType().getClassifier());
                    	    	            associateNodeWithAstElement(currentNode.getParent(), current);
                    	    	        }
                    	    	        try {
                    	    	       		add(
                    	    	       			current, 
                    	    	       			"ElseExpressions",
                    	    	        		lv_ElseExpressions_3_0, 
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
                    	    if ( cnt19 >= 1 ) break loop19;
                                EarlyExitException eee =
                                    new EarlyExitException(19, input);
                                throw eee;
                        }
                        cnt19++;
                    } while (true);


                    }
                    break;

            }

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:997:5: ( (lv_endName_4_0= RULE_ENDIF ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:998:1: (lv_endName_4_0= RULE_ENDIF )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:998:1: (lv_endName_4_0= RULE_ENDIF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:999:3: lv_endName_4_0= RULE_ENDIF
            {
            lv_endName_4_0=(Token)input.LT(1);
            match(input,RULE_ENDIF,FOLLOW_RULE_ENDIF_in_ruleWMLPreprocIF1577); 

            			createLeafNode(grammarAccess.getWMLPreprocIFAccess().getEndNameENDIFTerminalRuleCall_3_0(), "endName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLPreprocIFRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"endName",
            	        		lv_endName_4_0, 
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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1029:1: entryRuleWMLRootExpression returns [EObject current=null] : iv_ruleWMLRootExpression= ruleWMLRootExpression EOF ;
    public final EObject entryRuleWMLRootExpression() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLRootExpression = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1030:2: (iv_ruleWMLRootExpression= ruleWMLRootExpression EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1031:2: iv_ruleWMLRootExpression= ruleWMLRootExpression EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLRootExpressionRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLRootExpression_in_entryRuleWMLRootExpression1618);
            iv_ruleWMLRootExpression=ruleWMLRootExpression();
            _fsp--;

             current =iv_ruleWMLRootExpression; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLRootExpression1628); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1038:1: ruleWMLRootExpression returns [EObject current=null] : (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF ) ;
    public final EObject ruleWMLRootExpression() throws RecognitionException {
        EObject current = null;

        EObject this_WMLTag_0 = null;

        EObject this_WMLMacroCall_1 = null;

        EObject this_WMLMacroDefine_2 = null;

        EObject this_WMLTextdomain_3 = null;

        EObject this_WMLPreprocIF_4 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1043:6: ( (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1044:1: (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1044:1: (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF )
            int alt21=5;
            switch ( input.LA(1) ) {
            case 20:
                {
                alt21=1;
                }
                break;
            case 25:
                {
                alt21=2;
                }
                break;
            case RULE_DEFINE:
                {
                alt21=3;
                }
                break;
            case RULE_TEXTDOMAIN:
                {
                alt21=4;
                }
                break;
            case RULE_IFDEF:
            case RULE_IFNDEF:
            case RULE_IFHAVE:
            case RULE_IFNHAVE:
                {
                alt21=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("1044:1: (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF )", 21, 0, input);

                throw nvae;
            }

            switch (alt21) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1045:5: this_WMLTag_0= ruleWMLTag
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLTagParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLRootExpression1675);
                    this_WMLTag_0=ruleWMLTag();
                    _fsp--;

                     
                            current = this_WMLTag_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1055:5: this_WMLMacroCall_1= ruleWMLMacroCall
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLMacroCallParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLRootExpression1702);
                    this_WMLMacroCall_1=ruleWMLMacroCall();
                    _fsp--;

                     
                            current = this_WMLMacroCall_1; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1065:5: this_WMLMacroDefine_2= ruleWMLMacroDefine
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLMacroDefineParserRuleCall_2(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroDefine_in_ruleWMLRootExpression1729);
                    this_WMLMacroDefine_2=ruleWMLMacroDefine();
                    _fsp--;

                     
                            current = this_WMLMacroDefine_2; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1075:5: this_WMLTextdomain_3= ruleWMLTextdomain
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLTextdomainParserRuleCall_3(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLTextdomain_in_ruleWMLRootExpression1756);
                    this_WMLTextdomain_3=ruleWMLTextdomain();
                    _fsp--;

                     
                            current = this_WMLTextdomain_3; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 5 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1085:5: this_WMLPreprocIF_4= ruleWMLPreprocIF
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLPreprocIFParserRuleCall_4(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLPreprocIF_in_ruleWMLRootExpression1783);
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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1101:1: entryRuleWMLExpression returns [EObject current=null] : iv_ruleWMLExpression= ruleWMLExpression EOF ;
    public final EObject entryRuleWMLExpression() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLExpression = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1102:2: (iv_ruleWMLExpression= ruleWMLExpression EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1103:2: iv_ruleWMLExpression= ruleWMLExpression EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLExpressionRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLExpression_in_entryRuleWMLExpression1818);
            iv_ruleWMLExpression=ruleWMLExpression();
            _fsp--;

             current =iv_ruleWMLExpression; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLExpression1828); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1110:1: ruleWMLExpression returns [EObject current=null] : (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey ) ;
    public final EObject ruleWMLExpression() throws RecognitionException {
        EObject current = null;

        EObject this_WMLRootExpression_0 = null;

        EObject this_WMLKey_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1115:6: ( (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1116:1: (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1116:1: (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey )
            int alt22=2;
            int LA22_0 = input.LA(1);

            if ( (LA22_0==RULE_DEFINE||(LA22_0>=RULE_IFDEF && LA22_0<=RULE_IFNHAVE)||LA22_0==RULE_TEXTDOMAIN||LA22_0==20||LA22_0==25) ) {
                alt22=1;
            }
            else if ( (LA22_0==RULE_ID) ) {
                alt22=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("1116:1: (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey )", 22, 0, input);

                throw nvae;
            }
            switch (alt22) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1117:5: this_WMLRootExpression_0= ruleWMLRootExpression
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLExpressionAccess().getWMLRootExpressionParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLRootExpression_in_ruleWMLExpression1875);
                    this_WMLRootExpression_0=ruleWMLRootExpression();
                    _fsp--;

                     
                            current = this_WMLRootExpression_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1127:5: this_WMLKey_1= ruleWMLKey
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLExpressionAccess().getWMLKeyParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLExpression1902);
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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1143:1: entryRuleWMLValuedExpression returns [EObject current=null] : iv_ruleWMLValuedExpression= ruleWMLValuedExpression EOF ;
    public final EObject entryRuleWMLValuedExpression() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLValuedExpression = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1144:2: (iv_ruleWMLValuedExpression= ruleWMLValuedExpression EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1145:2: iv_ruleWMLValuedExpression= ruleWMLValuedExpression EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLValuedExpressionRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_entryRuleWMLValuedExpression1937);
            iv_ruleWMLValuedExpression=ruleWMLValuedExpression();
            _fsp--;

             current =iv_ruleWMLValuedExpression; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValuedExpression1947); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1152:1: ruleWMLValuedExpression returns [EObject current=null] : (this_WMLExpression_0= ruleWMLExpression | ( () ruleWMLValue ) ) ;
    public final EObject ruleWMLValuedExpression() throws RecognitionException {
        EObject current = null;

        EObject this_WMLExpression_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1157:6: ( (this_WMLExpression_0= ruleWMLExpression | ( () ruleWMLValue ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1158:1: (this_WMLExpression_0= ruleWMLExpression | ( () ruleWMLValue ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1158:1: (this_WMLExpression_0= ruleWMLExpression | ( () ruleWMLValue ) )
            int alt23=2;
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
                alt23=1;
                }
                break;
            case RULE_ID:
                {
                int LA23_2 = input.LA(2);

                if ( (LA23_2==24) ) {
                    alt23=1;
                }
                else if ( (LA23_2==EOF||LA23_2==RULE_ID||(LA23_2>=RULE_DEFINE && LA23_2<=RULE_TEXTDOMAIN)||(LA23_2>=RULE_STRING && LA23_2<=RULE_ANY_OTHER)||LA23_2==20||(LA23_2>=25 && LA23_2<=27)||(LA23_2>=29 && LA23_2<=34)) ) {
                    alt23=2;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("1158:1: (this_WMLExpression_0= ruleWMLExpression | ( () ruleWMLValue ) )", 23, 2, input);

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
                alt23=2;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("1158:1: (this_WMLExpression_0= ruleWMLExpression | ( () ruleWMLValue ) )", 23, 0, input);

                throw nvae;
            }

            switch (alt23) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1159:5: this_WMLExpression_0= ruleWMLExpression
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLValuedExpressionAccess().getWMLExpressionParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLExpression_in_ruleWMLValuedExpression1994);
                    this_WMLExpression_0=ruleWMLExpression();
                    _fsp--;

                     
                            current = this_WMLExpression_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1168:6: ( () ruleWMLValue )
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1168:6: ( () ruleWMLValue )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1168:7: () ruleWMLValue
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1168:7: ()
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1169:5: 
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
                        
                    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLValuedExpression2025);
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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1195:1: entryRuleWMLTextdomain returns [EObject current=null] : iv_ruleWMLTextdomain= ruleWMLTextdomain EOF ;
    public final EObject entryRuleWMLTextdomain() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTextdomain = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1196:2: (iv_ruleWMLTextdomain= ruleWMLTextdomain EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1197:2: iv_ruleWMLTextdomain= ruleWMLTextdomain EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLTextdomainRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain2061);
            iv_ruleWMLTextdomain=ruleWMLTextdomain();
            _fsp--;

             current =iv_ruleWMLTextdomain; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTextdomain2071); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1204:1: ruleWMLTextdomain returns [EObject current=null] : ( (lv_name_0_0= RULE_TEXTDOMAIN ) ) ;
    public final EObject ruleWMLTextdomain() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1209:6: ( ( (lv_name_0_0= RULE_TEXTDOMAIN ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1210:1: ( (lv_name_0_0= RULE_TEXTDOMAIN ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1210:1: ( (lv_name_0_0= RULE_TEXTDOMAIN ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1211:1: (lv_name_0_0= RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1211:1: (lv_name_0_0= RULE_TEXTDOMAIN )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1212:3: lv_name_0_0= RULE_TEXTDOMAIN
            {
            lv_name_0_0=(Token)input.LT(1);
            match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_ruleWMLTextdomain2112); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1242:1: entryRuleWMLLuaCode returns [EObject current=null] : iv_ruleWMLLuaCode= ruleWMLLuaCode EOF ;
    public final EObject entryRuleWMLLuaCode() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLLuaCode = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1243:2: (iv_ruleWMLLuaCode= ruleWMLLuaCode EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1244:2: iv_ruleWMLLuaCode= ruleWMLLuaCode EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLLuaCodeRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode2152);
            iv_ruleWMLLuaCode=ruleWMLLuaCode();
            _fsp--;

             current =iv_ruleWMLLuaCode; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLLuaCode2162); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1251:1: ruleWMLLuaCode returns [EObject current=null] : ( (lv_value_0_0= RULE_LUA_CODE ) ) ;
    public final EObject ruleWMLLuaCode() throws RecognitionException {
        EObject current = null;

        Token lv_value_0_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1256:6: ( ( (lv_value_0_0= RULE_LUA_CODE ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1257:1: ( (lv_value_0_0= RULE_LUA_CODE ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1257:1: ( (lv_value_0_0= RULE_LUA_CODE ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1258:1: (lv_value_0_0= RULE_LUA_CODE )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1258:1: (lv_value_0_0= RULE_LUA_CODE )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1259:3: lv_value_0_0= RULE_LUA_CODE
            {
            lv_value_0_0=(Token)input.LT(1);
            match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_ruleWMLLuaCode2203); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1289:1: entryRuleWMLMacroParameter returns [String current=null] : iv_ruleWMLMacroParameter= ruleWMLMacroParameter EOF ;
    public final String entryRuleWMLMacroParameter() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleWMLMacroParameter = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1290:2: (iv_ruleWMLMacroParameter= ruleWMLMacroParameter EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1291:2: iv_ruleWMLMacroParameter= ruleWMLMacroParameter EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLMacroParameterRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter2244);
            iv_ruleWMLMacroParameter=ruleWMLMacroParameter();
            _fsp--;

             current =iv_ruleWMLMacroParameter.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroParameter2255); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1298:1: ruleWMLMacroParameter returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens ) ;
    public final AntlrDatatypeRuleToken ruleWMLMacroParameter() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        AntlrDatatypeRuleToken this_WMLValue_0 = null;

        AntlrDatatypeRuleToken this_MacroTokens_1 = null;


         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1303:6: ( (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1304:1: (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1304:1: (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens )
            int alt24=2;
            int LA24_0 = input.LA(1);

            if ( (LA24_0==RULE_ID||(LA24_0>=RULE_STRING && LA24_0<=RULE_ANY_OTHER)||(LA24_0>=26 && LA24_0<=27)||(LA24_0>=29 && LA24_0<=34)) ) {
                alt24=1;
            }
            else if ( ((LA24_0>=20 && LA24_0<=24)) ) {
                alt24=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("1304:1: (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens )", 24, 0, input);

                throw nvae;
            }
            switch (alt24) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1305:5: this_WMLValue_0= ruleWMLValue
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getWMLValueParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLMacroParameter2302);
                    this_WMLValue_0=ruleWMLValue();
                    _fsp--;


                    		current.merge(this_WMLValue_0);
                        
                     
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1317:5: this_MacroTokens_1= ruleMacroTokens
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getWMLMacroParameterAccess().getMacroTokensParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleMacroTokens_in_ruleWMLMacroParameter2335);
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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1335:1: entryRuleWMLValue returns [String current=null] : iv_ruleWMLValue= ruleWMLValue EOF ;
    public final String entryRuleWMLValue() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleWMLValue = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1336:2: (iv_ruleWMLValue= ruleWMLValue EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1337:2: iv_ruleWMLValue= ruleWMLValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLValue_in_entryRuleWMLValue2381);
            iv_ruleWMLValue=ruleWMLValue();
            _fsp--;

             current =iv_ruleWMLValue.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValue2392); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1344:1: ruleWMLValue returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_ID_0= RULE_ID | this_STRING_1= RULE_STRING | kw= '_' | kw= '~' | kw= '.' | kw= './' | kw= '$' | kw= '/' | kw= '(' | kw= ')' | this_ANY_OTHER_10= RULE_ANY_OTHER ) ;
    public final AntlrDatatypeRuleToken ruleWMLValue() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;
        Token this_STRING_1=null;
        Token kw=null;
        Token this_ANY_OTHER_10=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:6: ( (this_ID_0= RULE_ID | this_STRING_1= RULE_STRING | kw= '_' | kw= '~' | kw= '.' | kw= './' | kw= '$' | kw= '/' | kw= '(' | kw= ')' | this_ANY_OTHER_10= RULE_ANY_OTHER ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1350:1: (this_ID_0= RULE_ID | this_STRING_1= RULE_STRING | kw= '_' | kw= '~' | kw= '.' | kw= './' | kw= '$' | kw= '/' | kw= '(' | kw= ')' | this_ANY_OTHER_10= RULE_ANY_OTHER )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1350:1: (this_ID_0= RULE_ID | this_STRING_1= RULE_STRING | kw= '_' | kw= '~' | kw= '.' | kw= './' | kw= '$' | kw= '/' | kw= '(' | kw= ')' | this_ANY_OTHER_10= RULE_ANY_OTHER )
            int alt25=11;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt25=1;
                }
                break;
            case RULE_STRING:
                {
                alt25=2;
                }
                break;
            case 29:
                {
                alt25=3;
                }
                break;
            case 27:
                {
                alt25=4;
                }
                break;
            case 30:
                {
                alt25=5;
                }
                break;
            case 26:
                {
                alt25=6;
                }
                break;
            case 31:
                {
                alt25=7;
                }
                break;
            case 32:
                {
                alt25=8;
                }
                break;
            case 33:
                {
                alt25=9;
                }
                break;
            case 34:
                {
                alt25=10;
                }
                break;
            case RULE_ANY_OTHER:
                {
                alt25=11;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("1350:1: (this_ID_0= RULE_ID | this_STRING_1= RULE_STRING | kw= '_' | kw= '~' | kw= '.' | kw= './' | kw= '$' | kw= '/' | kw= '(' | kw= ')' | this_ANY_OTHER_10= RULE_ANY_OTHER )", 25, 0, input);

                throw nvae;
            }

            switch (alt25) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1350:6: this_ID_0= RULE_ID
                    {
                    this_ID_0=(Token)input.LT(1);
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLValue2432); 

                    		current.merge(this_ID_0);
                        
                     
                        createLeafNode(grammarAccess.getWMLValueAccess().getIDTerminalRuleCall_0(), null); 
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1358:10: this_STRING_1= RULE_STRING
                    {
                    this_STRING_1=(Token)input.LT(1);
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLValue2458); 

                    		current.merge(this_STRING_1);
                        
                     
                        createLeafNode(grammarAccess.getWMLValueAccess().getSTRINGTerminalRuleCall_1(), null); 
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:2: kw= '_'
                    {
                    kw=(Token)input.LT(1);
                    match(input,29,FOLLOW_29_in_ruleWMLValue2482); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().get_Keyword_2(), null); 
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1374:2: kw= '~'
                    {
                    kw=(Token)input.LT(1);
                    match(input,27,FOLLOW_27_in_ruleWMLValue2501); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getTildeKeyword_3(), null); 
                        

                    }
                    break;
                case 5 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1381:2: kw= '.'
                    {
                    kw=(Token)input.LT(1);
                    match(input,30,FOLLOW_30_in_ruleWMLValue2520); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getFullStopKeyword_4(), null); 
                        

                    }
                    break;
                case 6 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1388:2: kw= './'
                    {
                    kw=(Token)input.LT(1);
                    match(input,26,FOLLOW_26_in_ruleWMLValue2539); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getFullStopSolidusKeyword_5(), null); 
                        

                    }
                    break;
                case 7 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1395:2: kw= '$'
                    {
                    kw=(Token)input.LT(1);
                    match(input,31,FOLLOW_31_in_ruleWMLValue2558); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getDollarSignKeyword_6(), null); 
                        

                    }
                    break;
                case 8 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1402:2: kw= '/'
                    {
                    kw=(Token)input.LT(1);
                    match(input,32,FOLLOW_32_in_ruleWMLValue2577); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getSolidusKeyword_7(), null); 
                        

                    }
                    break;
                case 9 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1409:2: kw= '('
                    {
                    kw=(Token)input.LT(1);
                    match(input,33,FOLLOW_33_in_ruleWMLValue2596); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getLeftParenthesisKeyword_8(), null); 
                        

                    }
                    break;
                case 10 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1416:2: kw= ')'
                    {
                    kw=(Token)input.LT(1);
                    match(input,34,FOLLOW_34_in_ruleWMLValue2615); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getWMLValueAccess().getRightParenthesisKeyword_9(), null); 
                        

                    }
                    break;
                case 11 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1422:10: this_ANY_OTHER_10= RULE_ANY_OTHER
                    {
                    this_ANY_OTHER_10=(Token)input.LT(1);
                    match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_ruleWMLValue2636); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1437:1: entryRuleMacroTokens returns [String current=null] : iv_ruleMacroTokens= ruleMacroTokens EOF ;
    public final String entryRuleMacroTokens() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleMacroTokens = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1438:2: (iv_ruleMacroTokens= ruleMacroTokens EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1439:2: iv_ruleMacroTokens= ruleMacroTokens EOF
            {
             currentNode = createCompositeNode(grammarAccess.getMacroTokensRule(), currentNode); 
            pushFollow(FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens2682);
            iv_ruleMacroTokens=ruleMacroTokens();
            _fsp--;

             current =iv_ruleMacroTokens.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleMacroTokens2693); 

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
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1446:1: ruleMacroTokens returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (kw= '=' | kw= '[' | kw= ']' | kw= '+' | kw= '[/' ) ;
    public final AntlrDatatypeRuleToken ruleMacroTokens() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1451:6: ( (kw= '=' | kw= '[' | kw= ']' | kw= '+' | kw= '[/' ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1452:1: (kw= '=' | kw= '[' | kw= ']' | kw= '+' | kw= '[/' )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1452:1: (kw= '=' | kw= '[' | kw= ']' | kw= '+' | kw= '[/' )
            int alt26=5;
            switch ( input.LA(1) ) {
            case 24:
                {
                alt26=1;
                }
                break;
            case 20:
                {
                alt26=2;
                }
                break;
            case 22:
                {
                alt26=3;
                }
                break;
            case 21:
                {
                alt26=4;
                }
                break;
            case 23:
                {
                alt26=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("1452:1: (kw= '=' | kw= '[' | kw= ']' | kw= '+' | kw= '[/' )", 26, 0, input);

                throw nvae;
            }

            switch (alt26) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1453:2: kw= '='
                    {
                    kw=(Token)input.LT(1);
                    match(input,24,FOLLOW_24_in_ruleMacroTokens2731); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getMacroTokensAccess().getEqualsSignKeyword_0(), null); 
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1460:2: kw= '['
                    {
                    kw=(Token)input.LT(1);
                    match(input,20,FOLLOW_20_in_ruleMacroTokens2750); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getMacroTokensAccess().getLeftSquareBracketKeyword_1(), null); 
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1467:2: kw= ']'
                    {
                    kw=(Token)input.LT(1);
                    match(input,22,FOLLOW_22_in_ruleMacroTokens2769); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getMacroTokensAccess().getRightSquareBracketKeyword_2(), null); 
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1474:2: kw= '+'
                    {
                    kw=(Token)input.LT(1);
                    match(input,21,FOLLOW_21_in_ruleMacroTokens2788); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getMacroTokensAccess().getPlusSignKeyword_3(), null); 
                        

                    }
                    break;
                case 5 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1481:2: kw= '[/'
                    {
                    kw=(Token)input.LT(1);
                    match(input,23,FOLLOW_23_in_ruleMacroTokens2807); 

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
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_ruleWMLMacroCall948 = new BitSet(new long[]{0x00000007FFF60010L});
    public static final BitSet FOLLOW_28_in_ruleWMLMacroCall959 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_entryRuleWMLMacroCallParameter995 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCallParameter1005 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_ruleWMLMacroCallParameter1056 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroCallParameter1084 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall1119 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLArrayCall1129 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_ruleWMLArrayCall1164 = new BitSet(new long[]{0x00000007EC060010L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLArrayCall1185 = new BitSet(new long[]{0x00000007EC460010L});
    public static final BitSet FOLLOW_22_in_ruleWMLArrayCall1196 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1232 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine1242 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1284 = new BitSet(new long[]{0x00000007EE169F90L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_ruleWMLMacroDefine1310 = new BitSet(new long[]{0x00000007EE169F90L});
    public static final BitSet FOLLOW_RULE_ENDDEF_in_ruleWMLMacroDefine1328 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPreprocIF_in_entryRuleWMLPreprocIF1369 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLPreprocIF1379 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFDEF_in_ruleWMLPreprocIF1423 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_IFNDEF_in_ruleWMLPreprocIF1443 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_IFHAVE_in_ruleWMLPreprocIF1463 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_IFNHAVE_in_ruleWMLPreprocIF1483 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_ruleWMLPreprocIF1512 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_ELSE_in_ruleWMLPreprocIF1531 = new BitSet(new long[]{0x00000007EE169E90L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_ruleWMLPreprocIF1557 = new BitSet(new long[]{0x00000007EE16DE90L});
    public static final BitSet FOLLOW_RULE_ENDIF_in_ruleWMLPreprocIF1577 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_entryRuleWMLRootExpression1618 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRootExpression1628 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLRootExpression1675 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLRootExpression1702 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLRootExpression1729 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_ruleWMLRootExpression1756 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPreprocIF_in_ruleWMLRootExpression1783 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_entryRuleWMLExpression1818 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLExpression1828 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_ruleWMLExpression1875 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLExpression1902 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_entryRuleWMLValuedExpression1937 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValuedExpression1947 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_ruleWMLValuedExpression1994 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLValuedExpression2025 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain2061 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTextdomain2071 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_ruleWMLTextdomain2112 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode2152 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLLuaCode2162 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_ruleWMLLuaCode2203 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter2244 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroParameter2255 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLMacroParameter2302 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_ruleWMLMacroParameter2335 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_entryRuleWMLValue2381 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValue2392 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLValue2432 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLValue2458 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_ruleWMLValue2482 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_ruleWMLValue2501 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_ruleWMLValue2520 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_ruleWMLValue2539 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_ruleWMLValue2558 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_ruleWMLValue2577 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_33_in_ruleWMLValue2596 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_34_in_ruleWMLValue2615 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_ruleWMLValue2636 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens2682 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleMacroTokens2693 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_ruleMacroTokens2731 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_ruleMacroTokens2750 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_ruleMacroTokens2769 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_ruleMacroTokens2788 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_ruleMacroTokens2807 = new BitSet(new long[]{0x0000000000000002L});

}