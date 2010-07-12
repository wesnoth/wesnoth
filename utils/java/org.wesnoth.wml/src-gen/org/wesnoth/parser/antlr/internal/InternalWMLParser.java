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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:158:1: ruleWMLMacro returns [EObject current=null] : ( '{' ( (lv_macroName_1_0= RULE_ID ) ) ( ( (lv_tagcontent_2_1= RULE_ID | lv_tagcontent_2_2= RULE_STRING | lv_tagcontent_2_3= '_' | lv_tagcontent_2_4= ':' | lv_tagcontent_2_5= '-' | lv_tagcontent_2_6= '.' | lv_tagcontent_2_7= '(' | lv_tagcontent_2_8= ')' | lv_tagcontent_2_9= '=' | lv_tagcontent_2_10= '/' ) ) )* '}' ) ;
    public final EObject ruleWMLMacro() throws RecognitionException {
        EObject current = null;

        Token lv_macroName_1_0=null;
        Token lv_tagcontent_2_1=null;
        Token lv_tagcontent_2_2=null;
        Token lv_tagcontent_2_3=null;
        Token lv_tagcontent_2_4=null;
        Token lv_tagcontent_2_5=null;
        Token lv_tagcontent_2_6=null;
        Token lv_tagcontent_2_7=null;
        Token lv_tagcontent_2_8=null;
        Token lv_tagcontent_2_9=null;
        Token lv_tagcontent_2_10=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:163:6: ( ( '{' ( (lv_macroName_1_0= RULE_ID ) ) ( ( (lv_tagcontent_2_1= RULE_ID | lv_tagcontent_2_2= RULE_STRING | lv_tagcontent_2_3= '_' | lv_tagcontent_2_4= ':' | lv_tagcontent_2_5= '-' | lv_tagcontent_2_6= '.' | lv_tagcontent_2_7= '(' | lv_tagcontent_2_8= ')' | lv_tagcontent_2_9= '=' | lv_tagcontent_2_10= '/' ) ) )* '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:1: ( '{' ( (lv_macroName_1_0= RULE_ID ) ) ( ( (lv_tagcontent_2_1= RULE_ID | lv_tagcontent_2_2= RULE_STRING | lv_tagcontent_2_3= '_' | lv_tagcontent_2_4= ':' | lv_tagcontent_2_5= '-' | lv_tagcontent_2_6= '.' | lv_tagcontent_2_7= '(' | lv_tagcontent_2_8= ')' | lv_tagcontent_2_9= '=' | lv_tagcontent_2_10= '/' ) ) )* '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:1: ( '{' ( (lv_macroName_1_0= RULE_ID ) ) ( ( (lv_tagcontent_2_1= RULE_ID | lv_tagcontent_2_2= RULE_STRING | lv_tagcontent_2_3= '_' | lv_tagcontent_2_4= ':' | lv_tagcontent_2_5= '-' | lv_tagcontent_2_6= '.' | lv_tagcontent_2_7= '(' | lv_tagcontent_2_8= ')' | lv_tagcontent_2_9= '=' | lv_tagcontent_2_10= '/' ) ) )* '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:3: '{' ( (lv_macroName_1_0= RULE_ID ) ) ( ( (lv_tagcontent_2_1= RULE_ID | lv_tagcontent_2_2= RULE_STRING | lv_tagcontent_2_3= '_' | lv_tagcontent_2_4= ':' | lv_tagcontent_2_5= '-' | lv_tagcontent_2_6= '.' | lv_tagcontent_2_7= '(' | lv_tagcontent_2_8= ')' | lv_tagcontent_2_9= '=' | lv_tagcontent_2_10= '/' ) ) )* '}'
            {
            match(input,9,FOLLOW_9_in_ruleWMLMacro240); 

                    createLeafNode(grammarAccess.getWMLMacroAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:168:1: ( (lv_macroName_1_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:169:1: (lv_macroName_1_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:169:1: (lv_macroName_1_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:170:3: lv_macroName_1_0= RULE_ID
            {
            lv_macroName_1_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacro257); 

            			createLeafNode(grammarAccess.getWMLMacroAccess().getMacroNameIDTerminalRuleCall_1_0(), "macroName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"macroName",
            	        		lv_macroName_1_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:192:2: ( ( (lv_tagcontent_2_1= RULE_ID | lv_tagcontent_2_2= RULE_STRING | lv_tagcontent_2_3= '_' | lv_tagcontent_2_4= ':' | lv_tagcontent_2_5= '-' | lv_tagcontent_2_6= '.' | lv_tagcontent_2_7= '(' | lv_tagcontent_2_8= ')' | lv_tagcontent_2_9= '=' | lv_tagcontent_2_10= '/' ) ) )*
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( ((LA3_0>=RULE_ID && LA3_0<=RULE_STRING)||(LA3_0>=10 && LA3_0<=17)) ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:193:1: ( (lv_tagcontent_2_1= RULE_ID | lv_tagcontent_2_2= RULE_STRING | lv_tagcontent_2_3= '_' | lv_tagcontent_2_4= ':' | lv_tagcontent_2_5= '-' | lv_tagcontent_2_6= '.' | lv_tagcontent_2_7= '(' | lv_tagcontent_2_8= ')' | lv_tagcontent_2_9= '=' | lv_tagcontent_2_10= '/' ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:193:1: ( (lv_tagcontent_2_1= RULE_ID | lv_tagcontent_2_2= RULE_STRING | lv_tagcontent_2_3= '_' | lv_tagcontent_2_4= ':' | lv_tagcontent_2_5= '-' | lv_tagcontent_2_6= '.' | lv_tagcontent_2_7= '(' | lv_tagcontent_2_8= ')' | lv_tagcontent_2_9= '=' | lv_tagcontent_2_10= '/' ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:194:1: (lv_tagcontent_2_1= RULE_ID | lv_tagcontent_2_2= RULE_STRING | lv_tagcontent_2_3= '_' | lv_tagcontent_2_4= ':' | lv_tagcontent_2_5= '-' | lv_tagcontent_2_6= '.' | lv_tagcontent_2_7= '(' | lv_tagcontent_2_8= ')' | lv_tagcontent_2_9= '=' | lv_tagcontent_2_10= '/' )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:194:1: (lv_tagcontent_2_1= RULE_ID | lv_tagcontent_2_2= RULE_STRING | lv_tagcontent_2_3= '_' | lv_tagcontent_2_4= ':' | lv_tagcontent_2_5= '-' | lv_tagcontent_2_6= '.' | lv_tagcontent_2_7= '(' | lv_tagcontent_2_8= ')' | lv_tagcontent_2_9= '=' | lv_tagcontent_2_10= '/' )
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
            	            new NoViableAltException("194:1: (lv_tagcontent_2_1= RULE_ID | lv_tagcontent_2_2= RULE_STRING | lv_tagcontent_2_3= '_' | lv_tagcontent_2_4= ':' | lv_tagcontent_2_5= '-' | lv_tagcontent_2_6= '.' | lv_tagcontent_2_7= '(' | lv_tagcontent_2_8= ')' | lv_tagcontent_2_9= '=' | lv_tagcontent_2_10= '/' )", 2, 0, input);

            	        throw nvae;
            	    }

            	    switch (alt2) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:195:3: lv_tagcontent_2_1= RULE_ID
            	            {
            	            lv_tagcontent_2_1=(Token)input.LT(1);
            	            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacro281); 

            	            			createLeafNode(grammarAccess.getWMLMacroAccess().getTagcontentIDTerminalRuleCall_2_0_0(), "tagcontent"); 
            	            		

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"tagcontent",
            	            	        		lv_tagcontent_2_1, 
            	            	        		"ID", 
            	            	        		lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:216:8: lv_tagcontent_2_2= RULE_STRING
            	            {
            	            lv_tagcontent_2_2=(Token)input.LT(1);
            	            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLMacro301); 

            	            			createLeafNode(grammarAccess.getWMLMacroAccess().getTagcontentSTRINGTerminalRuleCall_2_0_1(), "tagcontent"); 
            	            		

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        try {
            	            	       		add(
            	            	       			current, 
            	            	       			"tagcontent",
            	            	        		lv_tagcontent_2_2, 
            	            	        		"STRING", 
            	            	        		lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 3 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:237:8: lv_tagcontent_2_3= '_'
            	            {
            	            lv_tagcontent_2_3=(Token)input.LT(1);
            	            match(input,10,FOLLOW_10_in_ruleWMLMacro322); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getTagcontent_Keyword_2_0_2(), "tagcontent"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "tagcontent", lv_tagcontent_2_3, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 4 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:255:8: lv_tagcontent_2_4= ':'
            	            {
            	            lv_tagcontent_2_4=(Token)input.LT(1);
            	            match(input,11,FOLLOW_11_in_ruleWMLMacro351); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getTagcontentColonKeyword_2_0_3(), "tagcontent"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "tagcontent", lv_tagcontent_2_4, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 5 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:273:8: lv_tagcontent_2_5= '-'
            	            {
            	            lv_tagcontent_2_5=(Token)input.LT(1);
            	            match(input,12,FOLLOW_12_in_ruleWMLMacro380); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getTagcontentHyphenMinusKeyword_2_0_4(), "tagcontent"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "tagcontent", lv_tagcontent_2_5, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 6 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:291:8: lv_tagcontent_2_6= '.'
            	            {
            	            lv_tagcontent_2_6=(Token)input.LT(1);
            	            match(input,13,FOLLOW_13_in_ruleWMLMacro409); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getTagcontentFullStopKeyword_2_0_5(), "tagcontent"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "tagcontent", lv_tagcontent_2_6, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 7 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:309:8: lv_tagcontent_2_7= '('
            	            {
            	            lv_tagcontent_2_7=(Token)input.LT(1);
            	            match(input,14,FOLLOW_14_in_ruleWMLMacro438); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getTagcontentLeftParenthesisKeyword_2_0_6(), "tagcontent"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "tagcontent", lv_tagcontent_2_7, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 8 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:327:8: lv_tagcontent_2_8= ')'
            	            {
            	            lv_tagcontent_2_8=(Token)input.LT(1);
            	            match(input,15,FOLLOW_15_in_ruleWMLMacro467); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getTagcontentRightParenthesisKeyword_2_0_7(), "tagcontent"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "tagcontent", lv_tagcontent_2_8, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 9 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:345:8: lv_tagcontent_2_9= '='
            	            {
            	            lv_tagcontent_2_9=(Token)input.LT(1);
            	            match(input,16,FOLLOW_16_in_ruleWMLMacro496); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getTagcontentEqualsSignKeyword_2_0_8(), "tagcontent"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "tagcontent", lv_tagcontent_2_9, null, lastConsumedNode);
            	            	        } catch (ValueConverterException vce) {
            	            				handleValueConverterException(vce);
            	            	        }
            	            	    

            	            }
            	            break;
            	        case 10 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:363:8: lv_tagcontent_2_10= '/'
            	            {
            	            lv_tagcontent_2_10=(Token)input.LT(1);
            	            match(input,17,FOLLOW_17_in_ruleWMLMacro525); 

            	                    createLeafNode(grammarAccess.getWMLMacroAccess().getTagcontentSolidusKeyword_2_0_9(), "tagcontent"); 
            	                

            	            	        if (current==null) {
            	            	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	            	            associateNodeWithAstElement(currentNode, current);
            	            	        }
            	            	        
            	            	        try {
            	            	       		add(current, "tagcontent", lv_tagcontent_2_10, null, lastConsumedNode);
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:405:1: ruleWMLTag returns [EObject current=null] : ( '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* ( (lv_end_6_0= ruleWMLEndTag ) ) ) ;
    public final EObject ruleWMLTag() throws RecognitionException {
        EObject current = null;

        Token lv_name_1_0=null;
        EObject lv_Ttags_3_0 = null;

        EObject lv_Tkeys_4_0 = null;

        EObject lv_Tmacros_5_0 = null;

        EObject lv_end_6_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:410:6: ( ( '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* ( (lv_end_6_0= ruleWMLEndTag ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:411:1: ( '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* ( (lv_end_6_0= ruleWMLEndTag ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:411:1: ( '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* ( (lv_end_6_0= ruleWMLEndTag ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:411:3: '[' ( (lv_name_1_0= RULE_ID ) ) ']' ( ( (lv_Ttags_3_0= ruleWMLTag ) ) | ( (lv_Tkeys_4_0= ruleWMLKey ) ) | ( (lv_Tmacros_5_0= ruleWMLMacro ) ) )* ( (lv_end_6_0= ruleWMLEndTag ) )
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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:517:4: ( (lv_end_6_0= ruleWMLEndTag ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:518:1: (lv_end_6_0= ruleWMLEndTag )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:518:1: (lv_end_6_0= ruleWMLEndTag )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:519:3: lv_end_6_0= ruleWMLEndTag
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getEndWMLEndTagParserRuleCall_4_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleWMLEndTag_in_ruleWMLTag764);
            lv_end_6_0=ruleWMLEndTag();
            _fsp--;


            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"end",
            	        		lv_end_6_0, 
            	        		"WMLEndTag", 
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
    // $ANTLR end ruleWMLTag


    // $ANTLR start entryRuleWMLEndTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:549:1: entryRuleWMLEndTag returns [EObject current=null] : iv_ruleWMLEndTag= ruleWMLEndTag EOF ;
    public final EObject entryRuleWMLEndTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLEndTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:550:2: (iv_ruleWMLEndTag= ruleWMLEndTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:551:2: iv_ruleWMLEndTag= ruleWMLEndTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLEndTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLEndTag_in_entryRuleWMLEndTag800);
            iv_ruleWMLEndTag=ruleWMLEndTag();
            _fsp--;

             current =iv_ruleWMLEndTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLEndTag810); 

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
    // $ANTLR end entryRuleWMLEndTag


    // $ANTLR start ruleWMLEndTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:558:1: ruleWMLEndTag returns [EObject current=null] : ( '[/' ( (lv_tagname_1_0= RULE_ID ) ) ']' ) ;
    public final EObject ruleWMLEndTag() throws RecognitionException {
        EObject current = null;

        Token lv_tagname_1_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:563:6: ( ( '[/' ( (lv_tagname_1_0= RULE_ID ) ) ']' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:564:1: ( '[/' ( (lv_tagname_1_0= RULE_ID ) ) ']' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:564:1: ( '[/' ( (lv_tagname_1_0= RULE_ID ) ) ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:564:3: '[/' ( (lv_tagname_1_0= RULE_ID ) ) ']'
            {
            match(input,21,FOLLOW_21_in_ruleWMLEndTag845); 

                    createLeafNode(grammarAccess.getWMLEndTagAccess().getLeftSquareBracketSolidusKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:568:1: ( (lv_tagname_1_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:569:1: (lv_tagname_1_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:569:1: (lv_tagname_1_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:570:3: lv_tagname_1_0= RULE_ID
            {
            lv_tagname_1_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLEndTag862); 

            			createLeafNode(grammarAccess.getWMLEndTagAccess().getTagnameIDTerminalRuleCall_1_0(), "tagname"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLEndTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"tagname",
            	        		lv_tagname_1_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            match(input,20,FOLLOW_20_in_ruleWMLEndTag877); 

                    createLeafNode(grammarAccess.getWMLEndTagAccess().getRightSquareBracketKeyword_2(), null); 
                

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
    // $ANTLR end ruleWMLEndTag


    // $ANTLR start entryRuleWMLKey
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:604:1: entryRuleWMLKey returns [EObject current=null] : iv_ruleWMLKey= ruleWMLKey EOF ;
    public final EObject entryRuleWMLKey() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKey = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:605:2: (iv_ruleWMLKey= ruleWMLKey EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:606:2: iv_ruleWMLKey= ruleWMLKey EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey913);
            iv_ruleWMLKey=ruleWMLKey();
            _fsp--;

             current =iv_ruleWMLKey; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey923); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:613:1: ruleWMLKey returns [EObject current=null] : ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ) ;
    public final EObject ruleWMLKey() throws RecognitionException {
        EObject current = null;

        Token lv_keyName_0_0=null;
        EObject lv_value_2_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:618:6: ( ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:619:1: ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:619:1: ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:619:2: ( (lv_keyName_0_0= RULE_ID ) ) '=' ( (lv_value_2_0= ruleWMLKeyValue ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:619:2: ( (lv_keyName_0_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:620:1: (lv_keyName_0_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:620:1: (lv_keyName_0_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:621:3: lv_keyName_0_0= RULE_ID
            {
            lv_keyName_0_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLKey965); 

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

            match(input,16,FOLLOW_16_in_ruleWMLKey980); 

                    createLeafNode(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:647:1: ( (lv_value_2_0= ruleWMLKeyValue ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:648:1: (lv_value_2_0= ruleWMLKeyValue )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:648:1: (lv_value_2_0= ruleWMLKeyValue )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:649:3: lv_value_2_0= ruleWMLKeyValue
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getValueWMLKeyValueParserRuleCall_2_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleWMLKeyValue_in_ruleWMLKey1001);
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:679:1: entryRuleWMLKeyValue returns [EObject current=null] : iv_ruleWMLKeyValue= ruleWMLKeyValue EOF ;
    public final EObject entryRuleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKeyValue = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:680:2: (iv_ruleWMLKeyValue= ruleWMLKeyValue EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:681:2: iv_ruleWMLKeyValue= ruleWMLKeyValue EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyValueRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue1037);
            iv_ruleWMLKeyValue=ruleWMLKeyValue();
            _fsp--;

             current =iv_ruleWMLKeyValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue1047); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:688:1: ruleWMLKeyValue returns [EObject current=null] : ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) ) ;
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
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:693:6: ( ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:694:1: ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:694:1: ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) )
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
                    new NoViableAltException("694:1: ( ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) ) | ( (lv_key2Value_1_0= ruleWMLMacro ) ) )", 6, 0, input);

                throw nvae;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:694:2: ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:694:2: ( ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) ) )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:695:1: ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:695:1: ( (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE ) )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:696:1: (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:696:1: (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE )
                    int alt5=9;
                    alt5 = dfa5.predict(input);
                    switch (alt5) {
                        case 1 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:697:3: lv_key1Value_0_1= RULE_ID
                            {
                            lv_key1Value_0_1=(Token)input.LT(1);
                            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLKeyValue1091); 

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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:718:8: lv_key1Value_0_2= RULE_STRING
                            {
                            lv_key1Value_0_2=(Token)input.LT(1);
                            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLKeyValue1111); 

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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:739:8: lv_key1Value_0_3= ruleTSTRING
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueTSTRINGParserRuleCall_0_0_2(), currentNode); 
                            	    
                            pushFollow(FOLLOW_ruleTSTRING_in_ruleWMLKeyValue1135);
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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:760:8: lv_key1Value_0_4= ruleFLOAT
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueFLOATParserRuleCall_0_0_3(), currentNode); 
                            	    
                            pushFollow(FOLLOW_ruleFLOAT_in_ruleWMLKeyValue1154);
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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:781:8: lv_key1Value_0_5= RULE_IINT
                            {
                            lv_key1Value_0_5=(Token)input.LT(1);
                            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_ruleWMLKeyValue1169); 

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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:802:8: lv_key1Value_0_6= rulePATH
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValuePATHParserRuleCall_0_0_5(), currentNode); 
                            	    
                            pushFollow(FOLLOW_rulePATH_in_ruleWMLKeyValue1193);
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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:823:8: lv_key1Value_0_7= ruleDIRECTION
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueDIRECTIONParserRuleCall_0_0_6(), currentNode); 
                            	    
                            pushFollow(FOLLOW_ruleDIRECTION_in_ruleWMLKeyValue1212);
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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:844:8: lv_key1Value_0_8= ruleLIST
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValueLISTParserRuleCall_0_0_7(), currentNode); 
                            	    
                            pushFollow(FOLLOW_ruleLIST_in_ruleWMLKeyValue1231);
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
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:865:8: lv_key1Value_0_9= rulePROGRESSIVE
                            {
                             
                            	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey1ValuePROGRESSIVEParserRuleCall_0_0_8(), currentNode); 
                            	    
                            pushFollow(FOLLOW_rulePROGRESSIVE_in_ruleWMLKeyValue1250);
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
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:890:6: ( (lv_key2Value_1_0= ruleWMLMacro ) )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:890:6: ( (lv_key2Value_1_0= ruleWMLMacro ) )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:891:1: (lv_key2Value_1_0= ruleWMLMacro )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:891:1: (lv_key2Value_1_0= ruleWMLMacro )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:892:3: lv_key2Value_1_0= ruleWMLMacro
                    {
                     
                    	        currentNode=createCompositeNode(grammarAccess.getWMLKeyValueAccess().getKey2ValueWMLMacroParserRuleCall_1_0(), currentNode); 
                    	    
                    pushFollow(FOLLOW_ruleWMLMacro_in_ruleWMLKeyValue1280);
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:922:1: entryRuleFLOAT returns [String current=null] : iv_ruleFLOAT= ruleFLOAT EOF ;
    public final String entryRuleFLOAT() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleFLOAT = null;


         
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();
        	
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:926:2: (iv_ruleFLOAT= ruleFLOAT EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:927:2: iv_ruleFLOAT= ruleFLOAT EOF
            {
             currentNode = createCompositeNode(grammarAccess.getFLOATRule(), currentNode); 
            pushFollow(FOLLOW_ruleFLOAT_in_entryRuleFLOAT1323);
            iv_ruleFLOAT=ruleFLOAT();
            _fsp--;

             current =iv_ruleFLOAT.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleFLOAT1334); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:937:1: ruleFLOAT returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+ ) ;
    public final AntlrDatatypeRuleToken ruleFLOAT() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_IINT_0=null;
        Token kw=null;
        Token this_IINT_2=null;

         setCurrentLookahead(); resetLookahead(); 
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:943:6: ( (this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:944:1: (this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:944:1: (this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:944:6: this_IINT_0= RULE_IINT kw= '.' (this_IINT_2= RULE_IINT )+
            {
            this_IINT_0=(Token)input.LT(1);
            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_ruleFLOAT1378); 

            		current.merge(this_IINT_0);
                
             
                createLeafNode(grammarAccess.getFLOATAccess().getIINTTerminalRuleCall_0(), null); 
                
            kw=(Token)input.LT(1);
            match(input,13,FOLLOW_13_in_ruleFLOAT1396); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getFLOATAccess().getFullStopKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:957:1: (this_IINT_2= RULE_IINT )+
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:957:6: this_IINT_2= RULE_IINT
            	    {
            	    this_IINT_2=(Token)input.LT(1);
            	    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_ruleFLOAT1412); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:975:1: entryRuleTSTRING returns [String current=null] : iv_ruleTSTRING= ruleTSTRING EOF ;
    public final String entryRuleTSTRING() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleTSTRING = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:976:2: (iv_ruleTSTRING= ruleTSTRING EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:977:2: iv_ruleTSTRING= ruleTSTRING EOF
            {
             currentNode = createCompositeNode(grammarAccess.getTSTRINGRule(), currentNode); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING1464);
            iv_ruleTSTRING=ruleTSTRING();
            _fsp--;

             current =iv_ruleTSTRING.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING1475); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:984:1: ruleTSTRING returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING ) ;
    public final AntlrDatatypeRuleToken ruleTSTRING() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        Token this_STRING_3=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:989:6: ( ( ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:990:1: ( ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:990:1: ( ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:990:2: ( (kw= ' ' )? kw= '_' (kw= ' ' )? ) this_STRING_3= RULE_STRING
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:990:2: ( (kw= ' ' )? kw= '_' (kw= ' ' )? )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:990:3: (kw= ' ' )? kw= '_' (kw= ' ' )?
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:990:3: (kw= ' ' )?
            int alt8=2;
            int LA8_0 = input.LA(1);

            if ( (LA8_0==22) ) {
                alt8=1;
            }
            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:991:2: kw= ' '
                    {
                    kw=(Token)input.LT(1);
                    match(input,22,FOLLOW_22_in_ruleTSTRING1515); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_0(), null); 
                        

                    }
                    break;

            }

            kw=(Token)input.LT(1);
            match(input,10,FOLLOW_10_in_ruleTSTRING1530); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getTSTRINGAccess().get_Keyword_0_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1002:1: (kw= ' ' )?
            int alt9=2;
            int LA9_0 = input.LA(1);

            if ( (LA9_0==22) ) {
                alt9=1;
            }
            switch (alt9) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1003:2: kw= ' '
                    {
                    kw=(Token)input.LT(1);
                    match(input,22,FOLLOW_22_in_ruleTSTRING1544); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getTSTRINGAccess().getSpaceKeyword_0_2(), null); 
                        

                    }
                    break;

            }


            }

            this_STRING_3=(Token)input.LT(1);
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleTSTRING1562); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1023:1: entryRulePATH returns [String current=null] : iv_rulePATH= rulePATH EOF ;
    public final String entryRulePATH() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_rulePATH = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1024:2: (iv_rulePATH= rulePATH EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1025:2: iv_rulePATH= rulePATH EOF
            {
             currentNode = createCompositeNode(grammarAccess.getPATHRule(), currentNode); 
            pushFollow(FOLLOW_rulePATH_in_entryRulePATH1608);
            iv_rulePATH=rulePATH();
            _fsp--;

             current =iv_rulePATH.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePATH1619); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1032:1: rulePATH returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+ ) ;
    public final AntlrDatatypeRuleToken rulePATH() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;
        Token kw=null;
        Token this_ID_3=null;
        Token this_ID_5=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1037:6: ( ( ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1038:1: ( ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1038:1: ( ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1038:2: ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )* (this_ID_3= RULE_ID )+ kw= '.' (this_ID_5= RULE_ID )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1038:2: ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )*
            loop12:
            do {
                int alt12=2;
                alt12 = dfa12.predict(input);
                switch (alt12) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1038:3: (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1038:3: (this_ID_0= RULE_ID )+
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
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1038:8: this_ID_0= RULE_ID
            	    	    {
            	    	    this_ID_0=(Token)input.LT(1);
            	    	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rulePATH1661); 

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

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1045:3: (kw= '-' | kw= '/' )
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
            	            new NoViableAltException("1045:3: (kw= '-' | kw= '/' )", 11, 0, input);

            	        throw nvae;
            	    }
            	    switch (alt11) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1046:2: kw= '-'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,12,FOLLOW_12_in_rulePATH1682); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getPATHAccess().getHyphenMinusKeyword_0_1_0(), null); 
            	                

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1053:2: kw= '/'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,17,FOLLOW_17_in_rulePATH1701); 

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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1058:4: (this_ID_3= RULE_ID )+
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1058:9: this_ID_3= RULE_ID
            	    {
            	    this_ID_3=(Token)input.LT(1);
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rulePATH1720); 

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
            match(input,13,FOLLOW_13_in_rulePATH1740); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getPATHAccess().getFullStopKeyword_2(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1071:1: (this_ID_5= RULE_ID )+
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1071:6: this_ID_5= RULE_ID
            	    {
            	    this_ID_5=(Token)input.LT(1);
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_rulePATH1756); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1086:1: entryRuleDIRECTION returns [String current=null] : iv_ruleDIRECTION= ruleDIRECTION EOF ;
    public final String entryRuleDIRECTION() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleDIRECTION = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1087:2: (iv_ruleDIRECTION= ruleDIRECTION EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1088:2: iv_ruleDIRECTION= ruleDIRECTION EOF
            {
             currentNode = createCompositeNode(grammarAccess.getDIRECTIONRule(), currentNode); 
            pushFollow(FOLLOW_ruleDIRECTION_in_entryRuleDIRECTION1804);
            iv_ruleDIRECTION=ruleDIRECTION();
            _fsp--;

             current =iv_ruleDIRECTION.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleDIRECTION1815); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1095:1: ruleDIRECTION returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )? )+ ;
    public final AntlrDatatypeRuleToken ruleDIRECTION() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1100:6: ( ( (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )? )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1101:1: ( (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )? )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1101:1: ( (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )? )+
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1101:2: (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' ) (kw= ',' )?
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1101:2: (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' )
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
            	            new NoViableAltException("1101:2: (kw= 'n' | kw= 's' | kw= 'w' | kw= 'e' | kw= 'sw' | kw= 'se' | kw= 'ne' | kw= 'nw' )", 15, 0, input);

            	        throw nvae;
            	    }

            	    switch (alt15) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1102:2: kw= 'n'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,23,FOLLOW_23_in_ruleDIRECTION1854); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getNKeyword_0_0(), null); 
            	                

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1109:2: kw= 's'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,24,FOLLOW_24_in_ruleDIRECTION1873); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getSKeyword_0_1(), null); 
            	                

            	            }
            	            break;
            	        case 3 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1116:2: kw= 'w'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,25,FOLLOW_25_in_ruleDIRECTION1892); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getWKeyword_0_2(), null); 
            	                

            	            }
            	            break;
            	        case 4 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1123:2: kw= 'e'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,26,FOLLOW_26_in_ruleDIRECTION1911); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getEKeyword_0_3(), null); 
            	                

            	            }
            	            break;
            	        case 5 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1130:2: kw= 'sw'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,27,FOLLOW_27_in_ruleDIRECTION1930); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getSwKeyword_0_4(), null); 
            	                

            	            }
            	            break;
            	        case 6 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1137:2: kw= 'se'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,28,FOLLOW_28_in_ruleDIRECTION1949); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getSeKeyword_0_5(), null); 
            	                

            	            }
            	            break;
            	        case 7 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1144:2: kw= 'ne'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,29,FOLLOW_29_in_ruleDIRECTION1968); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getNeKeyword_0_6(), null); 
            	                

            	            }
            	            break;
            	        case 8 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1151:2: kw= 'nw'
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,30,FOLLOW_30_in_ruleDIRECTION1987); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getDIRECTIONAccess().getNwKeyword_0_7(), null); 
            	                

            	            }
            	            break;

            	    }

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1156:2: (kw= ',' )?
            	    int alt16=2;
            	    int LA16_0 = input.LA(1);

            	    if ( (LA16_0==31) ) {
            	        alt16=1;
            	    }
            	    switch (alt16) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1157:2: kw= ','
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,31,FOLLOW_31_in_ruleDIRECTION2002); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1170:1: entryRuleLIST returns [String current=null] : iv_ruleLIST= ruleLIST EOF ;
    public final String entryRuleLIST() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleLIST = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1171:2: (iv_ruleLIST= ruleLIST EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1172:2: iv_ruleLIST= ruleLIST EOF
            {
             currentNode = createCompositeNode(grammarAccess.getLISTRule(), currentNode); 
            pushFollow(FOLLOW_ruleLIST_in_entryRuleLIST2046);
            iv_ruleLIST=ruleLIST();
            _fsp--;

             current =iv_ruleLIST.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleLIST2057); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1179:1: ruleLIST returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+ ) ;
    public final AntlrDatatypeRuleToken ruleLIST() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;
        Token kw=null;
        Token this_ID_2=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1184:6: ( (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1185:1: (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1185:1: (this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1185:6: this_ID_0= RULE_ID (kw= ',' this_ID_2= RULE_ID )+
            {
            this_ID_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleLIST2097); 

            		current.merge(this_ID_0);
                
             
                createLeafNode(grammarAccess.getLISTAccess().getIDTerminalRuleCall_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1192:1: (kw= ',' this_ID_2= RULE_ID )+
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1193:2: kw= ',' this_ID_2= RULE_ID
            	    {
            	    kw=(Token)input.LT(1);
            	    match(input,31,FOLLOW_31_in_ruleLIST2116); 

            	            current.merge(kw);
            	            createLeafNode(grammarAccess.getLISTAccess().getCommaKeyword_1_0(), null); 
            	        
            	    this_ID_2=(Token)input.LT(1);
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleLIST2131); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1213:1: entryRulePROGRESSIVE returns [String current=null] : iv_rulePROGRESSIVE= rulePROGRESSIVE EOF ;
    public final String entryRulePROGRESSIVE() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_rulePROGRESSIVE = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1214:2: (iv_rulePROGRESSIVE= rulePROGRESSIVE EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1215:2: iv_rulePROGRESSIVE= rulePROGRESSIVE EOF
            {
             currentNode = createCompositeNode(grammarAccess.getPROGRESSIVERule(), currentNode); 
            pushFollow(FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE2179);
            iv_rulePROGRESSIVE=rulePROGRESSIVE();
            _fsp--;

             current =iv_rulePROGRESSIVE.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRulePROGRESSIVE2190); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1222:1: rulePROGRESSIVE returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : ( (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+ ) ;
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
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1227:6: ( ( (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1228:1: ( (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1228:1: ( (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1228:2: (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT ) (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )? (kw= ':' this_IINT_6= RULE_IINT )? (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1228:2: (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT )
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
                        new NoViableAltException("1228:2: (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT )", 19, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("1228:2: (this_IINT_0= RULE_IINT | this_FLOAT_1= ruleFLOAT )", 19, 0, input);

                throw nvae;
            }
            switch (alt19) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1228:7: this_IINT_0= RULE_IINT
                    {
                    this_IINT_0=(Token)input.LT(1);
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2231); 

                    		current.merge(this_IINT_0);
                        
                     
                        createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_0_0(), null); 
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1237:5: this_FLOAT_1= ruleFLOAT
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_0_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2264);
                    this_FLOAT_1=ruleFLOAT();
                    _fsp--;


                    		current.merge(this_FLOAT_1);
                        
                     
                            currentNode = currentNode.getParent();
                        

                    }
                    break;

            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1247:2: (kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT ) )?
            int alt21=2;
            int LA21_0 = input.LA(1);

            if ( (LA21_0==32) ) {
                alt21=1;
            }
            switch (alt21) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1248:2: kw= '~' (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT )
                    {
                    kw=(Token)input.LT(1);
                    match(input,32,FOLLOW_32_in_rulePROGRESSIVE2284); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_1_0(), null); 
                        
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1253:1: (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT )
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
                                new NoViableAltException("1253:1: (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT )", 20, 1, input);

                            throw nvae;
                        }
                    }
                    else {
                        NoViableAltException nvae =
                            new NoViableAltException("1253:1: (this_IINT_3= RULE_IINT | this_FLOAT_4= ruleFLOAT )", 20, 0, input);

                        throw nvae;
                    }
                    switch (alt20) {
                        case 1 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1253:6: this_IINT_3= RULE_IINT
                            {
                            this_IINT_3=(Token)input.LT(1);
                            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2300); 

                            		current.merge(this_IINT_3);
                                
                             
                                createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_1_1_0(), null); 
                                

                            }
                            break;
                        case 2 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1262:5: this_FLOAT_4= ruleFLOAT
                            {
                             
                                    currentNode=createCompositeNode(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_1_1_1(), currentNode); 
                                
                            pushFollow(FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2333);
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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1272:4: (kw= ':' this_IINT_6= RULE_IINT )?
            int alt22=2;
            int LA22_0 = input.LA(1);

            if ( (LA22_0==11) ) {
                alt22=1;
            }
            switch (alt22) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1273:2: kw= ':' this_IINT_6= RULE_IINT
                    {
                    kw=(Token)input.LT(1);
                    match(input,11,FOLLOW_11_in_rulePROGRESSIVE2355); 

                            current.merge(kw);
                            createLeafNode(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_2_0(), null); 
                        
                    this_IINT_6=(Token)input.LT(1);
                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2370); 

                    		current.merge(this_IINT_6);
                        
                     
                        createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_2_1(), null); 
                        

                    }
                    break;

            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1285:3: (kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )? )+
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1286:2: kw= ',' (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT ) (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )? (kw= ':' this_IINT_14= RULE_IINT )?
            	    {
            	    kw=(Token)input.LT(1);
            	    match(input,31,FOLLOW_31_in_rulePROGRESSIVE2391); 

            	            current.merge(kw);
            	            createLeafNode(grammarAccess.getPROGRESSIVEAccess().getCommaKeyword_3_0(), null); 
            	        
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1291:1: (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT )
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
            	                new NoViableAltException("1291:1: (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT )", 23, 1, input);

            	            throw nvae;
            	        }
            	    }
            	    else {
            	        NoViableAltException nvae =
            	            new NoViableAltException("1291:1: (this_IINT_8= RULE_IINT | this_FLOAT_9= ruleFLOAT )", 23, 0, input);

            	        throw nvae;
            	    }
            	    switch (alt23) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1291:6: this_IINT_8= RULE_IINT
            	            {
            	            this_IINT_8=(Token)input.LT(1);
            	            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2407); 

            	            		current.merge(this_IINT_8);
            	                
            	             
            	                createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_1_0(), null); 
            	                

            	            }
            	            break;
            	        case 2 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1300:5: this_FLOAT_9= ruleFLOAT
            	            {
            	             
            	                    currentNode=createCompositeNode(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_1_1(), currentNode); 
            	                
            	            pushFollow(FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2440);
            	            this_FLOAT_9=ruleFLOAT();
            	            _fsp--;


            	            		current.merge(this_FLOAT_9);
            	                
            	             
            	                    currentNode = currentNode.getParent();
            	                

            	            }
            	            break;

            	    }

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1310:2: (kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT ) )?
            	    int alt25=2;
            	    int LA25_0 = input.LA(1);

            	    if ( (LA25_0==32) ) {
            	        alt25=1;
            	    }
            	    switch (alt25) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1311:2: kw= '~' (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT )
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,32,FOLLOW_32_in_rulePROGRESSIVE2460); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getPROGRESSIVEAccess().getTildeKeyword_3_2_0(), null); 
            	                
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1316:1: (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT )
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
            	                        new NoViableAltException("1316:1: (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT )", 24, 1, input);

            	                    throw nvae;
            	                }
            	            }
            	            else {
            	                NoViableAltException nvae =
            	                    new NoViableAltException("1316:1: (this_IINT_11= RULE_IINT | this_FLOAT_12= ruleFLOAT )", 24, 0, input);

            	                throw nvae;
            	            }
            	            switch (alt24) {
            	                case 1 :
            	                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1316:6: this_IINT_11= RULE_IINT
            	                    {
            	                    this_IINT_11=(Token)input.LT(1);
            	                    match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2476); 

            	                    		current.merge(this_IINT_11);
            	                        
            	                     
            	                        createLeafNode(grammarAccess.getPROGRESSIVEAccess().getIINTTerminalRuleCall_3_2_1_0(), null); 
            	                        

            	                    }
            	                    break;
            	                case 2 :
            	                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1325:5: this_FLOAT_12= ruleFLOAT
            	                    {
            	                     
            	                            currentNode=createCompositeNode(grammarAccess.getPROGRESSIVEAccess().getFLOATParserRuleCall_3_2_1_1(), currentNode); 
            	                        
            	                    pushFollow(FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2509);
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

            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1335:4: (kw= ':' this_IINT_14= RULE_IINT )?
            	    int alt26=2;
            	    int LA26_0 = input.LA(1);

            	    if ( (LA26_0==11) ) {
            	        alt26=1;
            	    }
            	    switch (alt26) {
            	        case 1 :
            	            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1336:2: kw= ':' this_IINT_14= RULE_IINT
            	            {
            	            kw=(Token)input.LT(1);
            	            match(input,11,FOLLOW_11_in_rulePROGRESSIVE2531); 

            	                    current.merge(kw);
            	                    createLeafNode(grammarAccess.getPROGRESSIVEAccess().getColonKeyword_3_3_0(), null); 
            	                
            	            this_IINT_14=(Token)input.LT(1);
            	            match(input,RULE_IINT,FOLLOW_RULE_IINT_in_rulePROGRESSIVE2546); 

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
        "\1\uffff\1\10\2\uffff\1\13\10\uffff\1\16\1\uffff";
    static final String DFA5_minS =
        "\2\4\2\uffff\1\4\2\uffff\1\4\2\uffff\1\6\2\uffff\1\4\1\uffff";
    static final String DFA5_maxS =
        "\1\36\1\37\2\uffff\1\40\2\uffff\1\21\2\uffff\1\6\2\uffff\1\40\1"+
        "\uffff";
    static final String DFA5_acceptS =
        "\2\uffff\1\2\1\3\1\uffff\1\7\1\6\1\uffff\1\1\1\10\1\uffff\1\5\1"+
        "\11\1\uffff\1\4";
    static final String DFA5_specialS =
        "\17\uffff}>";
    static final String[] DFA5_transitionS = {
            "\1\1\1\2\1\4\3\uffff\1\3\13\uffff\1\3\10\5",
            "\1\7\4\uffff\1\10\2\uffff\2\6\3\uffff\1\6\1\uffff\1\10\1\uffff"+
            "\1\10\11\uffff\1\11",
            "",
            "",
            "\1\13\4\uffff\1\13\1\uffff\1\14\1\uffff\1\12\5\uffff\1\13\1"+
            "\uffff\1\13\11\uffff\2\14",
            "",
            "",
            "\1\6\7\uffff\2\6\2\uffff\1\10\1\6",
            "",
            "",
            "\1\15",
            "",
            "",
            "\1\16\1\uffff\1\15\2\uffff\1\16\1\uffff\1\14\7\uffff\1\16\1"+
            "\uffff\1\16\11\uffff\2\14",
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
            return "696:1: (lv_key1Value_0_1= RULE_ID | lv_key1Value_0_2= RULE_STRING | lv_key1Value_0_3= ruleTSTRING | lv_key1Value_0_4= ruleFLOAT | lv_key1Value_0_5= RULE_IINT | lv_key1Value_0_6= rulePATH | lv_key1Value_0_7= ruleDIRECTION | lv_key1Value_0_8= ruleLIST | lv_key1Value_0_9= rulePROGRESSIVE )";
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
            return "()* loopback of 1038:2: ( (this_ID_0= RULE_ID )+ (kw= '-' | kw= '/' ) )*";
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
    public static final BitSet FOLLOW_ruleWMLEndTag_in_ruleWMLTag764 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLEndTag_in_entryRuleWMLEndTag800 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLEndTag810 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_ruleWMLEndTag845 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLEndTag862 = new BitSet(new long[]{0x0000000000100000L});
    public static final BitSet FOLLOW_20_in_ruleWMLEndTag877 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey913 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey923 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKey965 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_ruleWMLKey980 = new BitSet(new long[]{0x000000007FC00670L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLKey1001 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue1037 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue1047 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKeyValue1091 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLKeyValue1111 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_ruleWMLKeyValue1135 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_ruleWMLKeyValue1154 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_ruleWMLKeyValue1169 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_ruleWMLKeyValue1193 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_ruleWMLKeyValue1212 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleLIST_in_ruleWMLKeyValue1231 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_ruleWMLKeyValue1250 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_ruleWMLKeyValue1280 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_entryRuleFLOAT1323 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleFLOAT1334 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_ruleFLOAT1378 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_13_in_ruleFLOAT1396 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_IINT_in_ruleFLOAT1412 = new BitSet(new long[]{0x0000000000000042L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING1464 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING1475 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_ruleTSTRING1515 = new BitSet(new long[]{0x0000000000000400L});
    public static final BitSet FOLLOW_10_in_ruleTSTRING1530 = new BitSet(new long[]{0x0000000000400020L});
    public static final BitSet FOLLOW_22_in_ruleTSTRING1544 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleTSTRING1562 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePATH_in_entryRulePATH1608 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePATH1619 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_rulePATH1661 = new BitSet(new long[]{0x0000000000021010L});
    public static final BitSet FOLLOW_12_in_rulePATH1682 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_17_in_rulePATH1701 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_rulePATH1720 = new BitSet(new long[]{0x0000000000002010L});
    public static final BitSet FOLLOW_13_in_rulePATH1740 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_rulePATH1756 = new BitSet(new long[]{0x0000000000000012L});
    public static final BitSet FOLLOW_ruleDIRECTION_in_entryRuleDIRECTION1804 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleDIRECTION1815 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_ruleDIRECTION1854 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_24_in_ruleDIRECTION1873 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_25_in_ruleDIRECTION1892 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_26_in_ruleDIRECTION1911 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_27_in_ruleDIRECTION1930 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_28_in_ruleDIRECTION1949 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_29_in_ruleDIRECTION1968 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_30_in_ruleDIRECTION1987 = new BitSet(new long[]{0x00000000FF800002L});
    public static final BitSet FOLLOW_31_in_ruleDIRECTION2002 = new BitSet(new long[]{0x000000007F800002L});
    public static final BitSet FOLLOW_ruleLIST_in_entryRuleLIST2046 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleLIST2057 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleLIST2097 = new BitSet(new long[]{0x0000000080000000L});
    public static final BitSet FOLLOW_31_in_ruleLIST2116 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleLIST2131 = new BitSet(new long[]{0x0000000080000002L});
    public static final BitSet FOLLOW_rulePROGRESSIVE_in_entryRulePROGRESSIVE2179 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePROGRESSIVE2190 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2231 = new BitSet(new long[]{0x0000000180000800L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2264 = new BitSet(new long[]{0x0000000180000800L});
    public static final BitSet FOLLOW_32_in_rulePROGRESSIVE2284 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2300 = new BitSet(new long[]{0x0000000080000800L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2333 = new BitSet(new long[]{0x0000000080000800L});
    public static final BitSet FOLLOW_11_in_rulePROGRESSIVE2355 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2370 = new BitSet(new long[]{0x0000000080000000L});
    public static final BitSet FOLLOW_31_in_rulePROGRESSIVE2391 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2407 = new BitSet(new long[]{0x0000000180000802L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2440 = new BitSet(new long[]{0x0000000180000802L});
    public static final BitSet FOLLOW_32_in_rulePROGRESSIVE2460 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2476 = new BitSet(new long[]{0x0000000080000802L});
    public static final BitSet FOLLOW_ruleFLOAT_in_rulePROGRESSIVE2509 = new BitSet(new long[]{0x0000000080000802L});
    public static final BitSet FOLLOW_11_in_rulePROGRESSIVE2531 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_IINT_in_rulePROGRESSIVE2546 = new BitSet(new long[]{0x0000000080000002L});

}