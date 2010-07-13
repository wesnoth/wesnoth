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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_STRING", "RULE_IINT", "RULE_SL_COMMENT", "RULE_WS", "'{'", "'_'", "':'", "'-'", "'.'", "'('", "')'", "'='", "'/'", "'}'", "'['", "']'", "'[/'", "' '", "'n'", "'s'", "'w'", "'e'", "'sw'", "'se'", "'ne'", "'nw'", "','", "'~'"
    };
    public static final int RULE_ID=4;
    public static final int RULE_STRING=5;
    public static final int RULE_IINT=6;
    public static final int RULE_WS=8;
    public static final int EOF=-1;
    public static final int RULE_SL_COMMENT=7;

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

                if ( (LA1_0==19) ) {
                    alt1=1;
                }
                else if ( (LA1_0==9) ) {
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:158:1: ruleWMLMacro returns [EObject current=null] : ( '{' ( (lv_name_1_0= RULE_ID ) ) ( ( (lv_value_2_1= RULE_ID | lv_value_2_2= RULE_STRING | lv_value_2_3= '_' | lv_value_2_4= ':' | lv_value_2_5= '-' | lv_value_2_6= '.' | lv_value_2_7= '(' | lv_value_2_8= ')' | lv_value_2_9= '=' | lv_value_2_10= '/' ) ) )* '}' ) ;
    public final EObject ruleWMLMacro() throws RecognitionException {
        EObject current = null;

        Token lv_name_1_0=null;
        Token lv_value_2_1=null;
        Token lv_value_2_2=null;
        Token lv_value_2_3=null;
        Token lv_value_2_4=null;
        Token lv_value_2_5=null;
        Token lv_value_2_6=null;
        Token lv_value_2_7=null;
        Token lv_value_2_8=null;
        Token lv_value_2_9=null;
        Token lv_value_2_10=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:163:6: ( ( '{' ( (lv_name_1_0= RULE_ID ) ) ( ( (lv_value_2_1= RULE_ID | lv_value_2_2= RULE_STRING | lv_value_2_3= '_' | lv_value_2_4= ':' | lv_value_2_5= '-' | lv_value_2_6= '.' | lv_value_2_7= '(' | lv_value_2_8= ')' | lv_value_2_9= '=' | lv_value_2_10= '/' ) ) )* '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:1: ( '{' ( (lv_name_1_0= RULE_ID ) ) ( ( (lv_value_2_1= RULE_ID | lv_value_2_2= RULE_STRING | lv_value_2_3= '_' | lv_value_2_4= ':' | lv_value_2_5= '-' | lv_value_2_6= '.' | lv_value_2_7= '(' | lv_value_2_8= ')' | lv_value_2_9= '=' | lv_value_2_10= '/' ) ) )* '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:1: ( '{' ( (lv_name_1_0= RULE_ID ) ) ( ( (lv_value_2_1= RULE_ID | lv_value_2_2= RULE_STRING | lv_value_2_3= '_' | lv_value_2_4= ':' | lv_value_2_5= '-' | lv_value_2_6= '.' | lv_value_2_7= '(' | lv_value_2_8= ')' | lv_value_2_9= '=' | lv_value_2_10= '/' ) ) )* '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:3: '{' ( (lv_name_1_0= RULE_ID ) ) ( ( (lv_value_2_1= RULE_ID | lv_value_2_2= RULE_STRING | lv_value_2_3= '_' | lv_value_2_4= ':' | lv_value_2_5= '-' | lv_value_2_6= '.' | lv_value_2_7= '(' | lv_value_2_8= ')' | lv_value_2_9= '=' | lv_value_2_10= '/' ) ) )* '}'
            {
            match(input,9,FOLLOW_9_in_ruleWMLMacro240); 

                    createLeafNode(grammarAccess.getWMLMacroAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:168:1: ( (lv_name_1_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:169:1: (lv_name_1_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:169:1: (lv_name_1_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:170:3: lv_name_1_0= RULE_ID
            {
            lv_name_1_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacro257); 

            			createLeafNode(grammarAccess.getWMLMacroAccess().getNameIDTerminalRuleCall_1_0(), "name"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:192:2: ( ( (lv_value_2_1= RULE_ID | lv_value_2_2= RULE_STRING | lv_value_2_3= '_' | lv_value_2_4= ':' | lv_value_2_5= '-' | lv_value_2_6= '.' | lv_value_2_7= '(' | lv_value_2_8= ')' | lv_value_2_9= '=' | lv_value_2_10= '/' ) ) )*
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( ((LA3_0>=RULE_ID && LA3_0<=RULE_STRING)||(LA3_0>=10 && LA3_0<=17)) ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:193:1: ( (lv_value_2_1= RULE_ID | lv_value_2_2= RULE_STRING | lv_value_2_3= '_' | lv_value_2_4= ':' | lv_value_2_5= '-' | lv_value_2_6= '.' | lv_value_2_7= '(' | lv_value_2_8= ')' | lv_value_2_9= '=' | lv_value_2_10= '/' ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:193:1: ( (lv_value_2_1= RULE_ID | lv_value_2_2= RULE_STRING | lv_value_2_3= '_' | lv_value_2_4= ':' | lv_value_2_5= '-' | lv_value_2_6= '.' | lv_value_2_7= '(' | lv_value_2_8= ')' | lv_value_2_9= '=' | lv_value_2_10= '/' ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:194:1: (lv_value_2_1= RULE_ID | lv_value_2_2= RULE_STRING | lv_value_2_3= '_' | lv_value_2_4= ':' | lv_value_2_5= '-' | lv_value_2_6= '.' | lv_value_2_7= '(' | lv_value_2_8= ')' | lv_value_2_9= '=' | lv_value_2_10= '/' )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:194:1: (lv_value_2_1= RULE_ID | lv_value_2_2= RULE_STRING | lv_value_2_3= '_' | lv_value_2_4= ':' | lv_value_2_5= '-' | lv_value_2_6= '.' | lv_value_2_7= '(' | lv_value_2_8= ')' | lv_value_2_9= '=' | lv_value_2_10= '/' )
            	    int alt2=10;
            	    switch ( input.LA(1) ) {
            	    case RULE_ID:
            	        {
            	        alt2=1;
            	        }
            	        break;
            	    case RULE_STRING:
            	        {
            	        alt2=2;
            	        }
            	        break;
            	    case 10:
            	        {
            	        alt2=3;
            	        }
            	        break;
            	    case 11:
            	        {
            	        alt2=4;
            	        }
            	        break;
            	    case 12:
            	        {
            	        alt2=5;
            	        }
            	        break;
            	    case 13:
            	        {
            	        alt2=6;
            	        }
            	        break;
            	    case 14:
            	        {
            	        alt2=7;
            	        }
            	        break;
            	    case 15:
            	        {
            	        alt2=8;
            	        }
            	        break;
            	    case 16:
            	        {
            	        alt2=9;
            	        }
            	        break;
            	    case 17:
            	        {
            	        alt2=10;
            	        }
            	        break;
            	    default:
            	        NoViableAltException nvae =
            	            new NoViableAltException("194:1: (lv_value_2_1= RULE_ID | lv_value_2_2= RULE_STRING | lv_value_2_3= '_' | lv_value_2_4= ':' | lv_value_2_5= '-' | lv_value_2_6= '.' | lv_value_2_7= '(' | lv_value_2_8= ')' | lv_value_2_9= '=' | lv_value_2_10= '/' )", 2, 0, input);

            	        throw nvae;
            	    }

            	    switch (alt2) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:195:3: lv_value_2_1= RULE_ID
            	            {
            	            lv_value_2_1=(Token)input.LT(1);
            	            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacro281); 

            	            			createLeafNode(grammarAccess.getWMLMacroAccess().getValueIDTerminalRuleCall_2_0_0(), "value"); 
            	            		

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"value",
            	            	        		lv_value_2_1, 
            	            	        		"ID", 
            	            	        		lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:216:8: lv_value_2_2= RULE_STRING
            	            {
            	            lv_value_2_2=(Token)input.LT(1);
            	            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLMacro301); 

            	            			createLeafNode(grammarAccess.getWMLMacroAccess().getValueSTRINGTerminalRuleCall_2_0_1(), "value"); 
            	            		

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"value",
            	            	        		lv_value_2_2, 
            	            	        		"STRING", 
            	            	        		lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 3 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:237:8: lv_value_2_3= '_'
            	            {
            	            lv_value_2_3=(Token)input.LT(1);
            	            match(input,10,FOLLOW_10_in_ruleWMLMacro322); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getValue_Keyword_2_0_2(), "value"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "value", lv_value_2_3, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 4 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:255:8: lv_value_2_4= ':'
            	            {
            	            lv_value_2_4=(Token)input.LT(1);
            	            match(input,11,FOLLOW_11_in_ruleWMLMacro351); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getValueColonKeyword_2_0_3(), "value"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "value", lv_value_2_4, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 5 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:273:8: lv_value_2_5= '-'
            	            {
            	            lv_value_2_5=(Token)input.LT(1);
            	            match(input,12,FOLLOW_12_in_ruleWMLMacro380); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getValueHyphenMinusKeyword_2_0_4(), "value"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "value", lv_value_2_5, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 6 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:291:8: lv_value_2_6= '.'
            	            {
            	            lv_value_2_6=(Token)input.LT(1);
            	            match(input,13,FOLLOW_13_in_ruleWMLMacro409); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getValueFullStopKeyword_2_0_5(), "value"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "value", lv_value_2_6, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 7 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:309:8: lv_value_2_7= '('
            	            {
            	            lv_value_2_7=(Token)input.LT(1);
            	            match(input,14,FOLLOW_14_in_ruleWMLMacro438); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getValueLeftParenthesisKeyword_2_0_6(), "value"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "value", lv_value_2_7, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 8 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:327:8: lv_value_2_8= ')'
            	            {
            	            lv_value_2_8=(Token)input.LT(1);
            	            match(input,15,FOLLOW_15_in_ruleWMLMacro467); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getValueRightParenthesisKeyword_2_0_7(), "value"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "value", lv_value_2_8, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 9 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:345:8: lv_value_2_9= '='
            	            {
            	            lv_value_2_9=(Token)input.LT(1);
            	            match(input,16,FOLLOW_16_in_ruleWMLMacro496); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getValueEqualsSignKeyword_2_0_8(), "value"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "value", lv_value_2_9, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 10 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:363:8: lv_value_2_10= '/'
            	            {
            	            lv_value_2_10=(Token)input.LT(1);
            	            match(input,17,FOLLOW_17_in_ruleWMLMacro525); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getValueSolidusKeyword_2_0_9(), "value"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "value", lv_value_2_10, null, lastConsumedNode);
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
            	    break loop3;
                }
            } while (true);

            match(input,18,FOLLOW_18_in_ruleWMLMacro552); 

                    createLeafNode(grammarAccess.getWMLMacroAccess().getRightCurlyBracketKeyword_3(), null); 
                

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:396:1: entryRuleWMLTag returns [EObject current=null] : iv_ruleWMLTag= ruleWMLTag EOF ;
    public final EObject entryRuleWMLTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:397:2: (iv_ruleWMLTag= ruleWMLTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:398:2: iv_ruleWMLTag= ruleWMLTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLTag_in_entryRuleWMLTag588);
            iv_ruleWMLTag=ruleWMLTag();
            _fsp--;

             current =iv_ruleWMLTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTag598); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:405:1: ruleWMLTag returns [EObject current=null] : ( '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* '[/' ( (lv_endName_7_0= RULE_ID ) ) ']' ) ;
    public final EObject ruleWMLTag() throws RecognitionException {
        EObject current = null;

        Token lv_name_1_0=null;
        Token lv_endName_7_0=null;
        EObject lv_Ttags_3_0 = null;

        EObject lv_Tkeys_4_0 = null;

        EObject lv_Tmacros_5_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:410:6: ( ( '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* '[/' ( (lv_endName_7_0= RULE_ID ) ) ']' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:411:1: ( '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* '[/' ( (lv_endName_7_0= RULE_ID ) ) ']' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:411:1: ( '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* '[/' ( (lv_endName_7_0= RULE_ID ) ) ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:411:3: '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* '[/' ( (lv_endName_7_0= RULE_ID ) ) ']'
            {
            match(input,19,FOLLOW_19_in_ruleWMLTag633); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:415:1: ( (lv_name_1_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:416:1: (lv_name_1_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:416:1: (lv_name_1_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:417:3: lv_name_1_0= RULE_ID
            {
            lv_name_1_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag650); 

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

            match(input,20,FOLLOW_20_in_ruleWMLTag665); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_2(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:443:1: ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )*
            loop4:
            do {
                int alt4=4;
                switch ( input.LA(1) ) {
                case 19:
                    {
                    alt4=1;
                    }
                    break;
                case RULE_ID:
                    {
                    alt4=2;
                    }
                    break;
                case 9:
                    {
                    alt4=3;
                    }
                    break;

                }

                switch (alt4) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:443:2: ( (lv_Ttags_3_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:443:2: ( (lv_Ttags_3_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:444:1: (lv_Ttags_3_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:444:1: (lv_Ttags_3_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:445:3: lv_Ttags_3_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTtagsWMLTagParserRuleCall_3_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLTag687);
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:468:6: ( (lv_Tkeys_4_0= ruleWMLKey ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:468:6: ( (lv_Tkeys_4_0= ruleWMLKey ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:469:1: (lv_Tkeys_4_0= ruleWMLKey )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:469:1: (lv_Tkeys_4_0= ruleWMLKey )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:470:3: lv_Tkeys_4_0= ruleWMLKey
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTkeysWMLKeyParserRuleCall_3_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLTag714);
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:493:6: ( (lv_Tmacros_5_0= ruleWMLMacro ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:493:6: ( (lv_Tmacros_5_0= ruleWMLMacro ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:494:1: (lv_Tmacros_5_0= ruleWMLMacro )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:494:1: (lv_Tmacros_5_0= ruleWMLMacro )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:495:3: lv_Tmacros_5_0= ruleWMLMacro
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTmacrosWMLMacroParserRuleCall_3_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacro_in_ruleWMLTag741);
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
            	    break loop4;
                }
            } while (true);

            match(input,21,FOLLOW_21_in_ruleWMLTag753); 

                    createLeafNode(grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_4(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:521:1: ( (lv_endName_7_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:522:1: (lv_endName_7_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:522:1: (lv_endName_7_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:523:3: lv_endName_7_0= RULE_ID
            {
            lv_endName_7_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag770); 

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

            match(input,20,FOLLOW_20_in_ruleWMLTag785); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:557:1: entryRuleWMLKey returns [EObject current=null] : iv_ruleWMLKey= ruleWMLKey EOF ;
    public final EObject entryRuleWMLKey() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKey = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:558:2: (iv_ruleWMLKey= ruleWMLKey EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:559:2: iv_ruleWMLKey= ruleWMLKey EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey821);
            iv_ruleWMLKey=ruleWMLKey();
            _fsp--;

             current =iv_ruleWMLKey; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey831); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:566:1: ruleWMLKey returns [EObject current=null] : ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ) ;
    public final EObject ruleWMLKey() throws RecognitionException {
        EObject current = null;

        Token lv_keyName_0_0=null;
        EObject lv_value_2_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:571:6: ( ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:572:1: ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:572:1: ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:572:2: ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:572:2: ( (lv_keyName_0_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:573:1: (lv_keyName_0_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:573:1: (lv_keyName_0_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:574:3: lv_keyName_0_0= RULE_ID
            {
            lv_keyName_0_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLKey873); 

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

            match(input,16,FOLLOW_16_in_ruleWMLKey888); 

                    createLeafNode(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:600:1: ( (lv_value_2_0= ruleWMLKeyValue ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:601:1: (lv_value_2_0= ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:601:1: (lv_value_2_0= ruleWMLKeyValue )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:602:3: lv_value_2_0= ruleWMLKeyValue
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleWMLKeyValue_in_ruleWMLKey909);
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:632:1: entryRuleWMLKeyValue returns [EObject current=null] : iv_ruleWMLKeyValue= ruleWMLKeyValue EOF ;
    public final EObject entryRuleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKeyValue = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:633:2: (iv_ruleWMLKeyValue= ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:634:2: iv_ruleWMLKeyValue= ruleWMLKeyValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue945);
            iv_ruleWMLKeyValue=ruleWMLKeyValue();
            _fsp--;

             current =iv_ruleWMLKeyValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue955); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:641:1: ruleWMLKeyValue returns [EObject current=null] : ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) ) ;
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
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:646:6: ( ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:647:1: ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:647:1: ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) )
            int alt6=2;
            int LA6_0 = input.LA(1);

            if ( ((LA6_0>=RULE_ID && LA6_0<=RULE_IINT)||LA6_0==10||(LA6_0>=22 && LA6_0<=30)) ) {
                alt6=1;
            }
            else if ( (LA6_0==9) ) {
                alt6=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("647:1: ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) )", 6, 0, input);

                throw nvae;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:647:2: ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:647:2: ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:648:1: ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:648:1: ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:649:1: (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:649:1: (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE )
                    int alt5=9;
                    alt5 = dfa5.predict(input);
                    switch (alt5) {
                        case 1 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:650:3: lv_key1Value_0_1= RULE_ID
                            {
                            lv_key1Value_0_1=(Token)input.LT(1);
                            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLKeyValue999); 

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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:671:8: lv_key1Value_0_2= RULE_STRING
                            {
                            lv_key1Value_0_2=(Token)input.LT(1);
                            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLKeyValue1019); 

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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:692:8: lv_key1Value_0_3= ruleTSTRING
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueTSTRINGParserRuleCall_0_0_2(), currentNode); 
                            	    
                            pushFollow(FOLLOW_ruleTSTRING_in_ruleWMLKeyValue1043);
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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:713:8: lv_key1Value_0_4= ruleFLOAT
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueFLOATParserRuleCall_0_0_3(), currentNode); 
                            	    
                            pushFollow(FOLLOW_ruleFLOAT_in_ruleWMLKeyValue1062);
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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:734:8: lv_key1Value_0_5= RULE_IINT
                            {
                            lv_key1Value_0_5=(Token)input.LT(1);
                            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_ruleWMLKeyValue1077); 

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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:755:8: lv_key1Value_0_6= rulePATH
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValuePATHParserRuleCall_0_0_5(), currentNode); 
                            	    
                            pushFollow(FOLLOW_rulePATH_in_ruleWMLKeyValue1101);
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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:776:8: lv_key1Value_0_7= ruleDIRECTION
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueDIRECTIONParserRuleCall_0_0_6(), currentNode); 
                            	    
                            pushFollow(FOLLOW_ruleDIRECTION_in_ruleWMLKeyValue1120);
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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:797:8: lv_key1Value_0_8= ruleLIST
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueLISTParserRuleCall_0_0_7(), currentNode); 
                            	    
                            pushFollow(FOLLOW_ruleLIST_in_ruleWMLKeyValue1139);
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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:818:8: lv_key1Value_0_9= rulePROGRESSIVE
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValuePROGRESSIVEParserRuleCall_0_0_8(), currentNode); 
                            	    
                            pushFollow(FOLLOW_rulePROGRESSIVE_in_ruleWMLKeyValue1158);
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
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:843:6: ( (lv_key2Value_1_0= ruleWMLMacro ) )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:843:6: ( (lv_key2Value_1_0= ruleWMLMacro ) )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:844:1: (lv_key2Value_1_0= ruleWMLMacro )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:844:1: (lv_key2Value_1_0= ruleWMLMacro )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:845:3: lv_key2Value_1_0= ruleWMLMacro
                    {
                     
                    	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey2ValueWMLMacroParserRuleCall_1_0(), currentNode); 
                    	    
                    pushFollow(FOLLOW_ruleWMLMacro_in_ruleWMLKeyValue1188);
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:875:1: entryRuleFLOAT returns [String current=null] : iv_ruleFLOAT= ruleFLOAT EOF ;
    public final String entryRuleFLOAT() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleFLOAT = null;


         
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();
        	
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:879:2: (iv_ruleFLOAT= ruleFLOAT EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:880:2: iv_ruleFLOAT= ruleFLOAT EOF
            {
             currentNode = createCompositeNode(grammarAccess.getFLOATRule(), currentNode); 
            pushFollow(FOLLOW_ruleFLOAT_in_entryRuleFLOAT1231);
            iv_ruleFLOAT=ruleFLOAT();
            _fsp--;

             current =iv_ruleFLOAT.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleFLOAT1242); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:890:1: ruleFLOAT returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+ ) ;
    public final AntlrDatatypeRuleToken ruleFLOAT() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_IINT_0=null;
        Token kw=null;
        Token this_IINT_2=null;

         setCurrentLookahead(); resetLookahead(); 
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:896:6: ( (this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:897:1: (this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:897:1: (this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:897:6: this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+
            {
            this_IINT_0=(Token)input.LT(1);
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_ruleFLOAT1286); 

            		current.merge(this_IINT_0);
                
             
                createLeafNode(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_0(), null); 
                
            kw=(Token)input.LT(1);
            match(input,13,FOLLOW_13_in_ruleFLOAT1304); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getFLOATAccess().getFullStopKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:910:1: (this_IINT_2= RULE_IINT )+
            int cnt7=0;
            loop7:
            do {
                int alt7=2;
                int LA7_0 = input.LA(1);

                if ( (LA7_0==RULE_IINT) ) {
                    alt7=1;
                }


                switch (alt7) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:910:6: this_IINT_2= RULE_IINT
            	    {
            	    this_IINT_2=(Token)input.LT(1);
            	    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_ruleFLOAT1320); 

            	    		current.merge(this_IINT_2);
            	        
            	     
            	        createLeafNode(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_2(), null); 
            	        

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:928:1: entryRuleTSTRING returns [String current=null] : iv_ruleTSTRING= ruleTSTRING EOF ;
    public final String entryRuleTSTRING() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleTSTRING = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:929:2: (iv_ruleTSTRING= ruleTSTRING EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:930:2: iv_ruleTSTRING= ruleTSTRING EOF
            {
             currentNode = createCompositeNode(grammarAccess.getTSTRINGRule(), currentNode); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING1372);
            iv_ruleTSTRING=ruleTSTRING();
            _fsp--;

             current =iv_ruleTSTRING.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING1383); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:937:1: ruleTSTRING returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING ) ;
    public final AntlrDatatypeRuleToken ruleTSTRING() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        Token this_STRING_3=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:942:6: ( ( ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:1: ( ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:1: ( ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:2: ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:2: ( (kw= ' ' )? kw= '_' (kw= ' ' )? )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:3: (kw= ' ' )? kw= '_' (kw= ' ' )?
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:3: (kw= ' ' )?
            int alt8=2;
            int LA8_0 = input.LA(1);

            if ( (LA8_0==22) ) {
                alt8=1;
            }
            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:944:2: kw= ' '
                    {
                    kw=(Token)input.LT(1);
                    match(input,22,FOLLOW_22_in_ruleTSTRING1423); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_0(), null); 
                        

                    }
                    break;

            }

            kw=(Token)input.LT(1);
            match(input,10,FOLLOW_10_in_ruleTSTRING1438); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getTSTRINGAccess().get_Keyword_0_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:955:1: (kw= ' ' )?
            int alt9=2;
            int LA9_0 = input.LA(1);

            if ( (LA9_0==22) ) {
                alt9=1;
            }
            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:956:2: kw= ' '
                    {
                    kw=(Token)input.LT(1);
                    match(input,22,FOLLOW_22_in_ruleTSTRING1452); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_2(), null); 
                        

                    }
                    break;

            }


            }

            this_STRING_3=(Token)input.LT(1);
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleTSTRING1470); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:976:1: entryRulePATH returns [String current=null] : iv_rulePATH= rulePATH EOF ;
    public final String entryRulePATH() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_rulePATH = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:977:2: (iv_rulePATH= rulePATH EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:978:2: iv_rulePATH= rulePATH EOF
            {
             currentNode = createCompositeNode(grammarAccess.getPATHRule(), currentNode); 
            pushFollow(FOLLOW_rulePATH_in_entryRulePATH1516);
            iv_rulePATH=rulePATH();
            _fsp--;

             current =iv_rulePATH.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePATH1527); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:985:1: rulePATH returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+ ) ;
    public final AntlrDatatypeRuleToken rulePATH() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;
        Token kw=null;
        Token this_ID_3=null;
        Token this_ID_5=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:990:6: ( ( ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:991:1: ( ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:991:1: ( ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:991:2: ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:991:2: ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )*
            loop12:
            do {
                int alt12=2;
                alt12 = dfa12.predict(input);
                switch (alt12) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:991:3: (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:991:3: (this_ID_0= RULE_ID )+
            	    int cnt10=0;
            	    loop10:
            	    do {
            	        int alt10=2;
            	        int LA10_0 = input.LA(1);

            	        if ( (LA10_0==RULE_ID) ) {
            	            alt10=1;
            	        }


            	        switch (alt10) {
            	    	case 1 :
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:991:8: this_ID_0= RULE_ID
            	    	    {
            	    	    this_ID_0=(Token)input.LT(1);
            	    	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rulePATH1569); 

            	    	    		current.merge(this_ID_0);
            	    	        
            	    	     
            	    	        createLeafNode(grammarAccess.getPATHAccess().getIDTerminalRuleCall_0_0(), null); 
            	    	        

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

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:998:3: (kw= '-' | kw= '/' )
            	    int alt11=2;
            	    int LA11_0 = input.LA(1);

            	    if ( (LA11_0==12) ) {
            	        alt11=1;
            	    }
            	    else if ( (LA11_0==17) ) {
            	        alt11=2;
            	    }
            	    else {
            	        NoViableAltException nvae =
            	            new NoViableAltException("998:3: (kw= '-' | kw= '/' )", 11, 0, input);

            	        throw nvae;
            	    }
            	    switch (alt11) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:999:2: kw= '-'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,12,FOLLOW_12_in_rulePATH1590); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getPATHAccess().getHyphenMinusKeyword_0_1_0(), null); 
            	                

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1006:2: kw= '/'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,17,FOLLOW_17_in_rulePATH1609); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getPATHAccess().getSolidusKeyword_0_1_1(), null); 
            	                

            	            }
            	            break;

            	    }


            	    }
            	    break;

            	default :
            	    break loop12;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1011:4: (this_ID_3= RULE_ID )+
            int cnt13=0;
            loop13:
            do {
                int alt13=2;
                int LA13_0 = input.LA(1);

                if ( (LA13_0==RULE_ID) ) {
                    alt13=1;
                }


                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1011:9: this_ID_3= RULE_ID
            	    {
            	    this_ID_3=(Token)input.LT(1);
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rulePATH1628); 

            	    		current.merge(this_ID_3);
            	        
            	     
            	        createLeafNode(grammarAccess.getPATHAccess().getIDTerminalRuleCall_1(), null); 
            	        

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

            kw=(Token)input.LT(1);
            match(input,13,FOLLOW_13_in_rulePATH1648); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getPATHAccess().getFullStopKeyword_2(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1024:1: (this_ID_5= RULE_ID )+
            int cnt14=0;
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( (LA14_0==RULE_ID) ) {
                    int LA14_2 = input.LA(2);

                    if ( (LA14_2==EOF||LA14_2==RULE_ID||LA14_2==9||LA14_2==19||LA14_2==21) ) {
                        alt14=1;
                    }


                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1024:6: this_ID_5= RULE_ID
            	    {
            	    this_ID_5=(Token)input.LT(1);
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rulePATH1664); 

            	    		current.merge(this_ID_5);
            	        
            	     
            	        createLeafNode(grammarAccess.getPATHAccess().getIDTerminalRuleCall_3(), null); 
            	        

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1039:1: entryRuleDIRECTION returns [String current=null] : iv_ruleDIRECTION= ruleDIRECTION EOF ;
    public final String entryRuleDIRECTION() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleDIRECTION = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1040:2: (iv_ruleDIRECTION= ruleDIRECTION EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1041:2: iv_ruleDIRECTION= ruleDIRECTION EOF
            {
             currentNode = createCompositeNode(grammarAccess.getDIRECTIONRule(), currentNode); 
            pushFollow(FOLLOW_ruleDIRECTION_in_entryRuleDIRECTION1712);
            iv_ruleDIRECTION=ruleDIRECTION();
            _fsp--;

             current =iv_ruleDIRECTION.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleDIRECTION1723); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1048:1: ruleDIRECTION returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )? )+ ;
    public final AntlrDatatypeRuleToken ruleDIRECTION() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1053:6: ( ( (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )? )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1054:1: ( (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )? )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1054:1: ( (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )? )+
            int cnt17=0;
            loop17:
            do {
                int alt17=2;
                int LA17_0 = input.LA(1);

                if ( ((LA17_0>=23 && LA17_0<=30)) ) {
                    alt17=1;
                }


                switch (alt17) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1054:2: (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )?
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1054:2: (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' )
            	    int alt15=8;
            	    switch ( input.LA(1) ) {
            	    case 23:
            	        {
            	        alt15=1;
            	        }
            	        break;
            	    case 24:
            	        {
            	        alt15=2;
            	        }
            	        break;
            	    case 25:
            	        {
            	        alt15=3;
            	        }
            	        break;
            	    case 26:
            	        {
            	        alt15=4;
            	        }
            	        break;
            	    case 27:
            	        {
            	        alt15=5;
            	        }
            	        break;
            	    case 28:
            	        {
            	        alt15=6;
            	        }
            	        break;
            	    case 29:
            	        {
            	        alt15=7;
            	        }
            	        break;
            	    case 30:
            	        {
            	        alt15=8;
            	        }
            	        break;
            	    default:
            	        NoViableAltException nvae =
            	            new NoViableAltException("1054:2: (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' )", 15, 0, input);

            	        throw nvae;
            	    }

            	    switch (alt15) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1055:2: kw= 'n'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,23,FOLLOW_23_in_ruleDIRECTION1762); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getNKeyword_0_0(), null); 
            	                

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1062:2: kw= 's'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,24,FOLLOW_24_in_ruleDIRECTION1781); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getSKeyword_0_1(), null); 
            	                

            	            }
            	            break;
            	        case 3 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1069:2: kw= 'w'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,25,FOLLOW_25_in_ruleDIRECTION1800); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getWKeyword_0_2(), null); 
            	                

            	            }
            	            break;
            	        case 4 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1076:2: kw= 'e'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,26,FOLLOW_26_in_ruleDIRECTION1819); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getEKeyword_0_3(), null); 
            	                

            	            }
            	            break;
            	        case 5 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1083:2: kw= 'sw'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,27,FOLLOW_27_in_ruleDIRECTION1838); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getSwKeyword_0_4(), null); 
            	                

            	            }
            	            break;
            	        case 6 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1090:2: kw= 'se'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,28,FOLLOW_28_in_ruleDIRECTION1857); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getSeKeyword_0_5(), null); 
            	                

            	            }
            	            break;
            	        case 7 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1097:2: kw= 'ne'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,29,FOLLOW_29_in_ruleDIRECTION1876); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getNeKeyword_0_6(), null); 
            	                

            	            }
            	            break;
            	        case 8 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1104:2: kw= 'nw'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,30,FOLLOW_30_in_ruleDIRECTION1895); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getNwKeyword_0_7(), null); 
            	                

            	            }
            	            break;

            	    }

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1109:2: (kw= ',' )?
            	    int alt16=2;
            	    int LA16_0 = input.LA(1);

            	    if ( (LA16_0==31) ) {
            	        alt16=1;
            	    }
            	    switch (alt16) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1110:2: kw= ','
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,31,FOLLOW_31_in_ruleDIRECTION1910); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getCommaKeyword_1(), null); 
            	                

            	            }
            	            break;

            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt17 >= 1 ) break loop17;
                        EarlyExitException eee =
                            new EarlyExitException(17, input);
                        throw eee;
                }
                cnt17++;
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1123:1: entryRuleLIST returns [String current=null] : iv_ruleLIST= ruleLIST EOF ;
    public final String entryRuleLIST() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleLIST = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1124:2: (iv_ruleLIST= ruleLIST EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1125:2: iv_ruleLIST= ruleLIST EOF
            {
             currentNode = createCompositeNode(grammarAccess.getLISTRule(), currentNode); 
            pushFollow(FOLLOW_ruleLIST_in_entryRuleLIST1954);
            iv_ruleLIST=ruleLIST();
            _fsp--;

             current =iv_ruleLIST.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleLIST1965); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1132:1: ruleLIST returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+ ) ;
    public final AntlrDatatypeRuleToken ruleLIST() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;
        Token kw=null;
        Token this_ID_2=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1137:6: ( (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1138:1: (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1138:1: (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1138:6: this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+
            {
            this_ID_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleLIST2005); 

            		current.merge(this_ID_0);
                
             
                createLeafNode(grammarAccess.getLISTAccess().getIDTerminalRuleCall_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1145:1: (kw= ',' this_ID_2= RULE_ID )+
            int cnt18=0;
            loop18:
            do {
                int alt18=2;
                int LA18_0 = input.LA(1);

                if ( (LA18_0==31) ) {
                    alt18=1;
                }


                switch (alt18) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1146:2: kw= ',' this_ID_2= RULE_ID
            	    {
            	    kw=(Token)input.LT(1);
            	    match(input,31,FOLLOW_31_in_ruleLIST2024); 

            	            current.merge(kw);
            	            createLeafNode(grammarAccess.getLISTAccess().getCommaKeyword_1_0(), null); 
            	        
            	    this_ID_2=(Token)input.LT(1);
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleLIST2039); 

            	    		current.merge(this_ID_2);
            	        
            	     
            	        createLeafNode(grammarAccess.getLISTAccess().getIDTerminalRuleCall_1_1(), null); 
            	        

            	    }
            	    break;

            	default :
            	    if ( cnt18 >= 1 ) break loop18;
                        EarlyExitException eee =
                            new EarlyExitException(18, input);
                        throw eee;
                }
                cnt18++;
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1166:1: entryRulePROGRESSIVE returns [String current=null] : iv_rulePROGRESSIVE= rulePROGRESSIVE EOF ;
    public final String entryRulePROGRESSIVE() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_rulePROGRESSIVE = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1167:2: (iv_rulePROGRESSIVE= rulePROGRESSIVE EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1168:2: iv_rulePROGRESSIVE= rulePROGRESSIVE EOF
            {
             currentNode = createCompositeNode(grammarAccess.getPROGRESSIVERule(), currentNode); 
            pushFollow(FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE2087);
            iv_rulePROGRESSIVE=rulePROGRESSIVE();
            _fsp--;

             current =iv_rulePROGRESSIVE.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePROGRESSIVE2098); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1175:1: rulePROGRESSIVE returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+ ) ;
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
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1180:6: ( ( (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1181:1: ( (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1181:1: ( (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1181:2: (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1181:2: (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT )
            int alt19=2;
            int LA19_0 = input.LA(1);

            if ( (LA19_0==RULE_IINT) ) {
                int LA19_1 = input.LA(2);

                if ( (LA19_1==13) ) {
                    alt19=2;
                }
                else if ( (LA19_1==11||(LA19_1>=31 && LA19_1<=32)) ) {
                    alt19=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("1181:2: (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT )", 19, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("1181:2: (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT )", 19, 0, input);

                throw nvae;
            }
            switch (alt19) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1181:7: this_IINT_0= RULE_IINT
                    {
                    this_IINT_0=(Token)input.LT(1);
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2139); 

                    		current.merge(this_IINT_0);
                        
                     
                        createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_0_0(), null); 
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1190:5: this_FLOAT_1= ruleFLOAT
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_0_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2172);
                    this_FLOAT_1=ruleFLOAT();
                    _fsp--;


                    		current.merge(this_FLOAT_1);
                        
                     
                            currentNode = currentNode.getParent();
                        

                    }
                    break;

            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1200:2: (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )?
            int alt21=2;
            int LA21_0 = input.LA(1);

            if ( (LA21_0==32) ) {
                alt21=1;
            }
            switch (alt21) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1201:2: kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT )
                    {
                    kw=(Token)input.LT(1);
                    match(input,32,FOLLOW_32_in_rulePROGRESSIVE2192); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_1_0(), null); 
                        
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1206:1: (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT )
                    int alt20=2;
                    int LA20_0 = input.LA(1);

                    if ( (LA20_0==RULE_IINT) ) {
                        int LA20_1 = input.LA(2);

                        if ( (LA20_1==13) ) {
                            alt20=2;
                        }
                        else if ( (LA20_1==11||LA20_1==31) ) {
                            alt20=1;
                        }
                        else {
                            NoViableAltException nvae =
                                new NoViableAltException("1206:1: (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT )", 20, 1, input);

                            throw nvae;
                        }
                    }
                    else {
                        NoViableAltException nvae =
                            new NoViableAltException("1206:1: (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT )", 20, 0, input);

                        throw nvae;
                    }
                    switch (alt20) {
                        case 1 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1206:6: this_IINT_3= RULE_IINT
                            {
                            this_IINT_3=(Token)input.LT(1);
                            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2208); 

                            		current.merge(this_IINT_3);
                                
                             
                                createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_1_1_0(), null); 
                                

                            }
                            break;
                        case 2 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1215:5: this_FLOAT_4= ruleFLOAT
                            {
                             
                                    currentNode=createCompositeNode(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_1_1_1(), currentNode); 
                                
                            pushFollow(FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2241);
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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1225:4: (kw= ':' this_IINT_6= RULE_IINT )?
            int alt22=2;
            int LA22_0 = input.LA(1);

            if ( (LA22_0==11) ) {
                alt22=1;
            }
            switch (alt22) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1226:2: kw= ':' this_IINT_6= RULE_IINT
                    {
                    kw=(Token)input.LT(1);
                    match(input,11,FOLLOW_11_in_rulePROGRESSIVE2263); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_2_0(), null); 
                        
                    this_IINT_6=(Token)input.LT(1);
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2278); 

                    		current.merge(this_IINT_6);
                        
                     
                        createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_2_1(), null); 
                        

                    }
                    break;

            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1238:3: (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+
            int cnt27=0;
            loop27:
            do {
                int alt27=2;
                int LA27_0 = input.LA(1);

                if ( (LA27_0==31) ) {
                    alt27=1;
                }


                switch (alt27) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1239:2: kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )?
            	    {
            	    kw=(Token)input.LT(1);
            	    match(input,31,FOLLOW_31_in_rulePROGRESSIVE2299); 

            	            current.merge(kw);
            	            createLeafNode(grammarAccess.getPROGRESSIVEAccess().getCommaKeyword_3_0(), null); 
            	        
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1244:1: (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT )
            	    int alt23=2;
            	    int LA23_0 = input.LA(1);

            	    if ( (LA23_0==RULE_IINT) ) {
            	        int LA23_1 = input.LA(2);

            	        if ( (LA23_1==13) ) {
            	            alt23=2;
            	        }
            	        else if ( (LA23_1==EOF||LA23_1==RULE_ID||LA23_1==9||LA23_1==11||LA23_1==19||LA23_1==21||(LA23_1>=31 && LA23_1<=32)) ) {
            	            alt23=1;
            	        }
            	        else {
            	            NoViableAltException nvae =
            	                new NoViableAltException("1244:1: (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT )", 23, 1, input);

            	            throw nvae;
            	        }
            	    }
            	    else {
            	        NoViableAltException nvae =
            	            new NoViableAltException("1244:1: (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT )", 23, 0, input);

            	        throw nvae;
            	    }
            	    switch (alt23) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1244:6: this_IINT_8= RULE_IINT
            	            {
            	            this_IINT_8=(Token)input.LT(1);
            	            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2315); 

            	            		current.merge(this_IINT_8);
            	                
            	             
            	                createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_1_0(), null); 
            	                

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1253:5: this_FLOAT_9= ruleFLOAT
            	            {
            	             
            	                    currentNode=createCompositeNode(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_1_1(), currentNode); 
            	                
            	            pushFollow(FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2348);
            	            this_FLOAT_9=ruleFLOAT();
            	            _fsp--;


            	            		current.merge(this_FLOAT_9);
            	                
            	             
            	                    currentNode = currentNode.getParent();
            	                

            	            }
            	            break;

            	    }

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1263:2: (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )?
            	    int alt25=2;
            	    int LA25_0 = input.LA(1);

            	    if ( (LA25_0==32) ) {
            	        alt25=1;
            	    }
            	    switch (alt25) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1264:2: kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT )
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,32,FOLLOW_32_in_rulePROGRESSIVE2368); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_3_2_0(), null); 
            	                
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1269:1: (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT )
            	            int alt24=2;
            	            int LA24_0 = input.LA(1);

            	            if ( (LA24_0==RULE_IINT) ) {
            	                int LA24_1 = input.LA(2);

            	                if ( (LA24_1==13) ) {
            	                    alt24=2;
            	                }
            	                else if ( (LA24_1==EOF||LA24_1==RULE_ID||LA24_1==9||LA24_1==11||LA24_1==19||LA24_1==21||LA24_1==31) ) {
            	                    alt24=1;
            	                }
            	                else {
            	                    NoViableAltException nvae =
            	                        new NoViableAltException("1269:1: (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT )", 24, 1, input);

            	                    throw nvae;
            	                }
            	            }
            	            else {
            	                NoViableAltException nvae =
            	                    new NoViableAltException("1269:1: (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT )", 24, 0, input);

            	                throw nvae;
            	            }
            	            switch (alt24) {
            	                case 1 :
            	                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1269:6: this_IINT_11= RULE_IINT
            	                    {
            	                    this_IINT_11=(Token)input.LT(1);
            	                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2384); 

            	                    		current.merge(this_IINT_11);
            	                        
            	                     
            	                        createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_2_1_0(), null); 
            	                        

            	                    }
            	                    break;
            	                case 2 :
            	                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1278:5: this_FLOAT_12= ruleFLOAT
            	                    {
            	                     
            	                            currentNode=createCompositeNode(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_2_1_1(), currentNode); 
            	                        
            	                    pushFollow(FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2417);
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

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1288:4: (kw= ':' this_IINT_14= RULE_IINT )?
            	    int alt26=2;
            	    int LA26_0 = input.LA(1);

            	    if ( (LA26_0==11) ) {
            	        alt26=1;
            	    }
            	    switch (alt26) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1289:2: kw= ':' this_IINT_14= RULE_IINT
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,11,FOLLOW_11_in_rulePROGRESSIVE2439); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_3_3_0(), null); 
            	                
            	            this_IINT_14=(Token)input.LT(1);
            	            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2454); 

            	            		current.merge(this_IINT_14);
            	                
            	             
            	                createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_3_1(), null); 
            	                

            	            }
            	            break;

            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt27 >= 1 ) break loop27;
                        EarlyExitException eee =
                            new EarlyExitException(27, input);
                        throw eee;
                }
                cnt27++;
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


    protected DFA5 dfa5 = new DFA5(this);
    protected DFA12 dfa12 = new DFA12(this);
    static final String DFA5_eotS =
        "\17\uffff";
    static final String DFA5_eofS =
        "\1\uffff\1\6\2\uffff\1\14\10\uffff\1\16\1\uffff";
    static final String DFA5_minS =
        "\2\4\2\uffff\1\4\2\uffff\1\4\2\uffff\1\6\2\uffff\1\4\1\uffff";
    static final String DFA5_maxS =
        "\1\36\1\37\2\uffff\1\40\2\uffff\1\21\2\uffff\1\6\2\uffff\1\40\1"+
        "\uffff";
    static final String DFA5_acceptS =
        "\2\uffff\1\2\1\3\1\uffff\1\7\1\1\1\uffff\1\6\1\10\1\uffff\1\11\1"+
        "\5\1\uffff\1\4";
    static final String DFA5_specialS =
        "\17\uffff}>";
    static final String[] DFA5_transitionS = {
            "\1\1\1\2\1\4\3\uffff\1\3\13\uffff\1\3\10\5",
            "\1\7\4\uffff\1\6\2\uffff\2\10\3\uffff\1\10\1\uffff\1\6\1\uffff"+
            "\1\6\11\uffff\1\11",
            "",
            "",
            "\1\14\4\uffff\1\14\1\uffff\1\13\1\uffff\1\12\5\uffff\1\14\1"+
            "\uffff\1\14\11\uffff\2\13",
            "",
            "",
            "\1\10\7\uffff\2\10\2\uffff\1\6\1\10",
            "",
            "",
            "\1\15",
            "",
            "",
            "\1\16\1\uffff\1\15\2\uffff\1\16\1\uffff\1\13\7\uffff\1\16\1"+
            "\uffff\1\16\11\uffff\2\13",
            ""
    };

    static final short[] DFA5_eot = DFA.unpackEncodedString(DFA5_eotS);
    static final short[] DFA5_eof = DFA.unpackEncodedString(DFA5_eofS);
    static final char[] DFA5_min = DFA.unpackEncodedStringToUnsignedChars(DFA5_minS);
    static final char[] DFA5_max = DFA.unpackEncodedStringToUnsignedChars(DFA5_maxS);
    static final short[] DFA5_accept = DFA.unpackEncodedString(DFA5_acceptS);
    static final short[] DFA5_special = DFA.unpackEncodedString(DFA5_specialS);
    static final short[][] DFA5_transition;

    static {
        int numStates = DFA5_transitionS.length;
        DFA5_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA5_transition[i] = DFA.unpackEncodedString(DFA5_transitionS[i]);
        }
    }

    class DFA5 extends DFA {

        public DFA5(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 5;
            this.eot = DFA5_eot;
            this.eof = DFA5_eof;
            this.min = DFA5_min;
            this.max = DFA5_max;
            this.accept = DFA5_accept;
            this.special = DFA5_special;
            this.transition = DFA5_transition;
        }
        public String getDescription() {
            return "649:1: (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE )";
        }
    }
    static final String DFA12_eotS =
        "\4\uffff";
    static final String DFA12_eofS =
        "\4\uffff";
    static final String DFA12_minS =
        "\2\4\2\uffff";
    static final String DFA12_maxS =
        "\1\4\1\21\2\uffff";
    static final String DFA12_acceptS =
        "\2\uffff\1\2\1\1";
    static final String DFA12_specialS =
        "\4\uffff}>";
    static final String[] DFA12_transitionS = {
            "\1\1",
            "\1\1\7\uffff\1\3\1\2\3\uffff\1\3",
            "",
            ""
    };

    static final short[] DFA12_eot = DFA.unpackEncodedString(DFA12_eotS);
    static final short[] DFA12_eof = DFA.unpackEncodedString(DFA12_eofS);
    static final char[] DFA12_min = DFA.unpackEncodedStringToUnsignedChars(DFA12_minS);
    static final char[] DFA12_max = DFA.unpackEncodedStringToUnsignedChars(DFA12_maxS);
    static final short[] DFA12_accept = DFA.unpackEncodedString(DFA12_acceptS);
    static final short[] DFA12_special = DFA.unpackEncodedString(DFA12_specialS);
    static final short[][] DFA12_transition;

    static {
        int numStates = DFA12_transitionS.length;
        DFA12_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA12_transition[i] = DFA.unpackEncodedString(DFA12_transitionS[i]);
        }
    }

    class DFA12 extends DFA {

        public DFA12(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 12;
            this.eot = DFA12_eot;
            this.eof = DFA12_eof;
            this.min = DFA12_min;
            this.max = DFA12_max;
            this.accept = DFA12_accept;
            this.special = DFA12_special;
            this.transition = DFA12_transition;
        }
        public String getDescription() {
            return "()* loopback of 991:2: ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )*";
        }
    }
 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot75 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot85 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLRoot131 = new BitSet(new long[]{0x0000000000080202L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_ruleWMLRoot158 = new BitSet(new long[]{0x0000000000080202L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_entryRuleWMLMacro195 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacro205 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_9_in_ruleWMLMacro240 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLMacro257 = new BitSet(new long[]{0x000000000007FC30L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLMacro281 = new BitSet(new long[]{0x000000000007FC30L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLMacro301 = new BitSet(new long[]{0x000000000007FC30L});
    public static final BitSet FOLLOW_10_in_ruleWMLMacro322 = new BitSet(new long[]{0x000000000007FC30L});
    public static final BitSet FOLLOW_11_in_ruleWMLMacro351 = new BitSet(new long[]{0x000000000007FC30L});
    public static final BitSet FOLLOW_12_in_ruleWMLMacro380 = new BitSet(new long[]{0x000000000007FC30L});
    public static final BitSet FOLLOW_13_in_ruleWMLMacro409 = new BitSet(new long[]{0x000000000007FC30L});
    public static final BitSet FOLLOW_14_in_ruleWMLMacro438 = new BitSet(new long[]{0x000000000007FC30L});
    public static final BitSet FOLLOW_15_in_ruleWMLMacro467 = new BitSet(new long[]{0x000000000007FC30L});
    public static final BitSet FOLLOW_16_in_ruleWMLMacro496 = new BitSet(new long[]{0x000000000007FC30L});
    public static final BitSet FOLLOW_17_in_ruleWMLMacro525 = new BitSet(new long[]{0x000000000007FC30L});
    public static final BitSet FOLLOW_18_in_ruleWMLMacro552 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag588 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag598 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_ruleWMLTag633 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag650 = new BitSet(new long[]{0x0000000000100000L});
    public static final BitSet FOLLOW_20_in_ruleWMLTag665 = new BitSet(new long[]{0x0000000000280210L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLTag687 = new BitSet(new long[]{0x0000000000280210L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLTag714 = new BitSet(new long[]{0x0000000000280210L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_ruleWMLTag741 = new BitSet(new long[]{0x0000000000280210L});
    public static final BitSet FOLLOW_21_in_ruleWMLTag753 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag770 = new BitSet(new long[]{0x0000000000100000L});
    public static final BitSet FOLLOW_20_in_ruleWMLTag785 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey821 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey831 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKey873 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_ruleWMLKey888 = new BitSet(new long[]{0x000000007FC00670L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLKey909 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue945 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue955 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKeyValue999 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLKeyValue1019 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_ruleWMLKeyValue1043 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_ruleWMLKeyValue1062 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_ruleWMLKeyValue1077 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_ruleWMLKeyValue1101 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_ruleWMLKeyValue1120 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleLIST_in_ruleWMLKeyValue1139 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_ruleWMLKeyValue1158 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_ruleWMLKeyValue1188 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_entryRuleFLOAT1231 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleFLOAT1242 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_ruleFLOAT1286 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_13_in_ruleFLOAT1304 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_IINT_in_ruleFLOAT1320 = new BitSet(new long[]{0x0000000000000042L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING1372 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING1383 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_ruleTSTRING1423 = new BitSet(new long[]{0x0000000000000400L});
    public static final BitSet FOLLOW_10_in_ruleTSTRING1438 = new BitSet(new long[]{0x0000000000400020L});
    public static final BitSet FOLLOW_22_in_ruleTSTRING1452 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleTSTRING1470 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_entryRulePATH1516 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePATH1527 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rulePATH1569 = new BitSet(new long[]{0x0000000000021010L});
    public static final BitSet FOLLOW_12_in_rulePATH1590 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_17_in_rulePATH1609 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_rulePATH1628 = new BitSet(new long[]{0x0000000000002010L});
    public static final BitSet FOLLOW_13_in_rulePATH1648 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_rulePATH1664 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_entryRuleDIRECTION1712 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleDIRECTION1723 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_ruleDIRECTION1762 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_24_in_ruleDIRECTION1781 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_25_in_ruleDIRECTION1800 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_26_in_ruleDIRECTION1819 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_27_in_ruleDIRECTION1838 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_28_in_ruleDIRECTION1857 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_29_in_ruleDIRECTION1876 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_30_in_ruleDIRECTION1895 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_31_in_ruleDIRECTION1910 = new BitSet(new long[]{0x000000007F800002L});
    public static final BitSet FOLLOW_ruleLIST_in_entryRuleLIST1954 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleLIST1965 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleLIST2005 = new BitSet(new long[]{0x0000000080000000L});
    public static final BitSet FOLLOW_31_in_ruleLIST2024 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleLIST2039 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE2087 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePROGRESSIVE2098 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2139 = new BitSet(new long[]{0x0000000180000800L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2172 = new BitSet(new long[]{0x0000000180000800L});
    public static final BitSet FOLLOW_32_in_rulePROGRESSIVE2192 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2208 = new BitSet(new long[]{0x0000000080000800L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2241 = new BitSet(new long[]{0x0000000080000800L});
    public static final BitSet FOLLOW_11_in_rulePROGRESSIVE2263 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2278 = new BitSet(new long[]{0x0000000080000000L});
    public static final BitSet FOLLOW_31_in_rulePROGRESSIVE2299 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2315 = new BitSet(new long[]{0x0000000180000802L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2348 = new BitSet(new long[]{0x0000000180000802L});
    public static final BitSet FOLLOW_32_in_rulePROGRESSIVE2368 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2384 = new BitSet(new long[]{0x0000000080000802L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2417 = new BitSet(new long[]{0x0000000080000802L});
    public static final BitSet FOLLOW_11_in_rulePROGRESSIVE2439 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2454 = new BitSet(new long[]{0x0000000080000002L});

}