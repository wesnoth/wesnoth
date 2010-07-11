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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_STRING", "RULE_INT", "RULE_SL_COMMENT", "RULE_WS", "RULE_IDENH", "'{'", "'}'", "'['", "']'", "'[/'", "'='", "'.'"
    };
    public static final int RULE_ID=4;
    public static final int RULE_STRING=5;
    public static final int RULE_IDENH=9;
    public static final int RULE_INT=6;
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

                if ( (LA1_0==12) ) {
                    alt1=1;
                }
                else if ( (LA1_0==10) ) {
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:158:1: ruleWMLMacro returns [EObject current=null] : ( '{' ( (lv_tagcontent_1_0= RULE_ID ) )+ '}' ) ;
    public final EObject ruleWMLMacro() throws RecognitionException {
        EObject current = null;

        Token lv_tagcontent_1_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:163:6: ( ( '{' ( (lv_tagcontent_1_0= RULE_ID ) )+ '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:1: ( '{' ( (lv_tagcontent_1_0= RULE_ID ) )+ '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:1: ( '{' ( (lv_tagcontent_1_0= RULE_ID ) )+ '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:164:3: '{' ( (lv_tagcontent_1_0= RULE_ID ) )+ '}'
            {
            match(input,10,FOLLOW_10_in_ruleWMLMacro240); 

                    createLeafNode(grammarAccess.getWMLMacroAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:168:1: ( (lv_tagcontent_1_0= RULE_ID ) )+
            int cnt2=0;
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( (LA2_0==RULE_ID) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:169:1: (lv_tagcontent_1_0= RULE_ID )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:169:1: (lv_tagcontent_1_0= RULE_ID )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:170:3: lv_tagcontent_1_0= RULE_ID
            	    {
            	    lv_tagcontent_1_0=(Token)input.LT(1);
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacro257); 

            	    			createLeafNode(grammarAccess.getWMLMacroAccess().getTagcontentIDTerminalRuleCall_1_0(), "tagcontent"); 
            	    		

            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLMacroRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode, current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"tagcontent",
            	    	        		lv_tagcontent_1_0, 
            	    	        		"ID", 
            	    	        		lastConsumedNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt2 >= 1 ) break loop2;
                        EarlyExitException eee =
                            new EarlyExitException(2, input);
                        throw eee;
                }
                cnt2++;
            } while (true);

            match(input,11,FOLLOW_11_in_ruleWMLMacro273); 

                    createLeafNode(grammarAccess.getWMLMacroAccess().getRightCurlyBracketKeyword_2(), null); 
                

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:204:1: entryRuleWMLTag returns [EObject current=null] : iv_ruleWMLTag= ruleWMLTag EOF ;
    public final EObject entryRuleWMLTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:205:2: (iv_ruleWMLTag= ruleWMLTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:206:2: iv_ruleWMLTag= ruleWMLTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLTag_in_entryRuleWMLTag309);
            iv_ruleWMLTag=ruleWMLTag();
            _fsp--;

             current =iv_ruleWMLTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTag319); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:213:1: ruleWMLTag returns [EObject current=null] : ( ( (lv_start_0_0= ruleWMLStartTag ) ) ( ( (lv_Ttags_1_0= ruleWMLTag ) ) | ( (lv_Tkeys_2_0= ruleWMLKey ) ) | ( (lv_Tmacros_3_0= ruleWMLMacro ) ) )* ( (lv_end_4_0= ruleWMLEndTag ) ) ) ;
    public final EObject ruleWMLTag() throws RecognitionException {
        EObject current = null;

        EObject lv_start_0_0 = null;

        EObject lv_Ttags_1_0 = null;

        EObject lv_Tkeys_2_0 = null;

        EObject lv_Tmacros_3_0 = null;

        EObject lv_end_4_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:218:6: ( ( ( (lv_start_0_0= ruleWMLStartTag ) ) ( ( (lv_Ttags_1_0= ruleWMLTag ) ) | ( (lv_Tkeys_2_0= ruleWMLKey ) ) | ( (lv_Tmacros_3_0= ruleWMLMacro ) ) )* ( (lv_end_4_0= ruleWMLEndTag ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:219:1: ( ( (lv_start_0_0= ruleWMLStartTag ) ) ( ( (lv_Ttags_1_0= ruleWMLTag ) ) | ( (lv_Tkeys_2_0= ruleWMLKey ) ) | ( (lv_Tmacros_3_0= ruleWMLMacro ) ) )* ( (lv_end_4_0= ruleWMLEndTag ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:219:1: ( ( (lv_start_0_0= ruleWMLStartTag ) ) ( ( (lv_Ttags_1_0= ruleWMLTag ) ) | ( (lv_Tkeys_2_0= ruleWMLKey ) ) | ( (lv_Tmacros_3_0= ruleWMLMacro ) ) )* ( (lv_end_4_0= ruleWMLEndTag ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:219:2: ( (lv_start_0_0= ruleWMLStartTag ) ) ( ( (lv_Ttags_1_0= ruleWMLTag ) ) | ( (lv_Tkeys_2_0= ruleWMLKey ) ) | ( (lv_Tmacros_3_0= ruleWMLMacro ) ) )* ( (lv_end_4_0= ruleWMLEndTag ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:219:2: ( (lv_start_0_0= ruleWMLStartTag ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:220:1: (lv_start_0_0= ruleWMLStartTag )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:220:1: (lv_start_0_0= ruleWMLStartTag )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:221:3: lv_start_0_0= ruleWMLStartTag
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getStartWMLStartTagParserRuleCall_0_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleWMLStartTag_in_ruleWMLTag365);
            lv_start_0_0=ruleWMLStartTag();
            _fsp--;


            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"start",
            	        		lv_start_0_0, 
            	        		"WMLStartTag", 
            	        		currentNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	        currentNode = currentNode.getParent();
            	    

            }


            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:243:2: ( ( (lv_Ttags_1_0= ruleWMLTag ) ) | ( (lv_Tkeys_2_0= ruleWMLKey ) ) | ( (lv_Tmacros_3_0= ruleWMLMacro ) ) )*
            loop3:
            do {
                int alt3=4;
                switch ( input.LA(1) ) {
                case 12:
                    {
                    alt3=1;
                    }
                    break;
                case RULE_ID:
                    {
                    alt3=2;
                    }
                    break;
                case 10:
                    {
                    alt3=3;
                    }
                    break;

                }

                switch (alt3) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:243:3: ( (lv_Ttags_1_0= ruleWMLTag ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:243:3: ( (lv_Ttags_1_0= ruleWMLTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:244:1: (lv_Ttags_1_0= ruleWMLTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:244:1: (lv_Ttags_1_0= ruleWMLTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:245:3: lv_Ttags_1_0= ruleWMLTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTtagsWMLTagParserRuleCall_1_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLTag387);
            	    lv_Ttags_1_0=ruleWMLTag();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Ttags",
            	    	        		lv_Ttags_1_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:268:6: ( (lv_Tkeys_2_0= ruleWMLKey ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:268:6: ( (lv_Tkeys_2_0= ruleWMLKey ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:269:1: (lv_Tkeys_2_0= ruleWMLKey )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:269:1: (lv_Tkeys_2_0= ruleWMLKey )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:270:3: lv_Tkeys_2_0= ruleWMLKey
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTkeysWMLKeyParserRuleCall_1_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLTag414);
            	    lv_Tkeys_2_0=ruleWMLKey();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Tkeys",
            	    	        		lv_Tkeys_2_0, 
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:293:6: ( (lv_Tmacros_3_0= ruleWMLMacro ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:293:6: ( (lv_Tmacros_3_0= ruleWMLMacro ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:294:1: (lv_Tmacros_3_0= ruleWMLMacro )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:294:1: (lv_Tmacros_3_0= ruleWMLMacro )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:295:3: lv_Tmacros_3_0= ruleWMLMacro
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getTmacrosWMLMacroParserRuleCall_1_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacro_in_ruleWMLTag441);
            	    lv_Tmacros_3_0=ruleWMLMacro();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"Tmacros",
            	    	        		lv_Tmacros_3_0, 
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
            	    break loop3;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:317:4: ( (lv_end_4_0= ruleWMLEndTag ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:318:1: (lv_end_4_0= ruleWMLEndTag )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:318:1: (lv_end_4_0= ruleWMLEndTag )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:319:3: lv_end_4_0= ruleWMLEndTag
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getWMLTagAccess().getEndWMLEndTagParserRuleCall_2_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleWMLEndTag_in_ruleWMLTag464);
            lv_end_4_0=ruleWMLEndTag();
            _fsp--;


            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"end",
            	        		lv_end_4_0, 
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


    // $ANTLR start entryRuleWMLStartTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:349:1: entryRuleWMLStartTag returns [EObject current=null] : iv_ruleWMLStartTag= ruleWMLStartTag EOF ;
    public final EObject entryRuleWMLStartTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLStartTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:350:2: (iv_ruleWMLStartTag= ruleWMLStartTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:351:2: iv_ruleWMLStartTag= ruleWMLStartTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLStartTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLStartTag_in_entryRuleWMLStartTag500);
            iv_ruleWMLStartTag=ruleWMLStartTag();
            _fsp--;

             current =iv_ruleWMLStartTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLStartTag510); 

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
    // $ANTLR end entryRuleWMLStartTag


    // $ANTLR start ruleWMLStartTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:358:1: ruleWMLStartTag returns [EObject current=null] : ( '[' ( (lv_tagname_1_0= RULE_ID ) ) ']' ) ;
    public final EObject ruleWMLStartTag() throws RecognitionException {
        EObject current = null;

        Token lv_tagname_1_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:363:6: ( ( '[' ( (lv_tagname_1_0= RULE_ID ) ) ']' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:364:1: ( '[' ( (lv_tagname_1_0= RULE_ID ) ) ']' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:364:1: ( '[' ( (lv_tagname_1_0= RULE_ID ) ) ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:364:3: '[' ( (lv_tagname_1_0= RULE_ID ) ) ']'
            {
            match(input,12,FOLLOW_12_in_ruleWMLStartTag545); 

                    createLeafNode(grammarAccess.getWMLStartTagAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:368:1: ( (lv_tagname_1_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:369:1: (lv_tagname_1_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:369:1: (lv_tagname_1_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:370:3: lv_tagname_1_0= RULE_ID
            {
            lv_tagname_1_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLStartTag562); 

            			createLeafNode(grammarAccess.getWMLStartTagAccess().getTagnameIDTerminalRuleCall_1_0(), "tagname"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getWMLStartTagRule().getType().getClassifier());
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

            match(input,13,FOLLOW_13_in_ruleWMLStartTag577); 

                    createLeafNode(grammarAccess.getWMLStartTagAccess().getRightSquareBracketKeyword_2(), null); 
                

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
    // $ANTLR end ruleWMLStartTag


    // $ANTLR start entryRuleWMLEndTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:404:1: entryRuleWMLEndTag returns [EObject current=null] : iv_ruleWMLEndTag= ruleWMLEndTag EOF ;
    public final EObject entryRuleWMLEndTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLEndTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:405:2: (iv_ruleWMLEndTag= ruleWMLEndTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:406:2: iv_ruleWMLEndTag= ruleWMLEndTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLEndTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLEndTag_in_entryRuleWMLEndTag613);
            iv_ruleWMLEndTag=ruleWMLEndTag();
            _fsp--;

             current =iv_ruleWMLEndTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLEndTag623); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:413:1: ruleWMLEndTag returns [EObject current=null] : ( '[/' ( (lv_tagname_1_0= RULE_ID ) ) ']' ) ;
    public final EObject ruleWMLEndTag() throws RecognitionException {
        EObject current = null;

        Token lv_tagname_1_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:418:6: ( ( '[/' ( (lv_tagname_1_0= RULE_ID ) ) ']' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:419:1: ( '[/' ( (lv_tagname_1_0= RULE_ID ) ) ']' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:419:1: ( '[/' ( (lv_tagname_1_0= RULE_ID ) ) ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:419:3: '[/' ( (lv_tagname_1_0= RULE_ID ) ) ']'
            {
            match(input,14,FOLLOW_14_in_ruleWMLEndTag658); 

                    createLeafNode(grammarAccess.getWMLEndTagAccess().getLeftSquareBracketSolidusKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:423:1: ( (lv_tagname_1_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:424:1: (lv_tagname_1_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:424:1: (lv_tagname_1_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:425:3: lv_tagname_1_0= RULE_ID
            {
            lv_tagname_1_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLEndTag675); 

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

            match(input,13,FOLLOW_13_in_ruleWMLEndTag690); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:459:1: entryRuleWMLKey returns [EObject current=null] : iv_ruleWMLKey= ruleWMLKey EOF ;
    public final EObject entryRuleWMLKey() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKey = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:460:2: (iv_ruleWMLKey= ruleWMLKey EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:461:2: iv_ruleWMLKey= ruleWMLKey EOF
            {
             currentNode = createCompositeNode(grammarAccess.getWMLKeyRule(), currentNode); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey726);
            iv_ruleWMLKey=ruleWMLKey();
            _fsp--;

             current =iv_ruleWMLKey; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey736); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:468:1: ruleWMLKey returns [EObject current=null] : ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( ( (lv_keyValue_2_1= RULE_ID | lv_keyValue_2_2= RULE_STRING | lv_keyValue_2_3= ruleFLOAT ) ) ) ) ;
    public final EObject ruleWMLKey() throws RecognitionException {
        EObject current = null;

        Token lv_keyName_0_0=null;
        Token lv_keyValue_2_1=null;
        Token lv_keyValue_2_2=null;
        AntlrDatatypeRuleToken lv_keyValue_2_3 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:473:6: ( ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( ( (lv_keyValue_2_1= RULE_ID | lv_keyValue_2_2= RULE_STRING | lv_keyValue_2_3= ruleFLOAT ) ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:474:1: ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( ( (lv_keyValue_2_1= RULE_ID | lv_keyValue_2_2= RULE_STRING | lv_keyValue_2_3= ruleFLOAT ) ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:474:1: ( ( (lv_keyName_0_0= RULE_ID ) ) '=' ( ( (lv_keyValue_2_1= RULE_ID | lv_keyValue_2_2= RULE_STRING | lv_keyValue_2_3= ruleFLOAT ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:474:2: ( (lv_keyName_0_0= RULE_ID ) ) '=' ( ( (lv_keyValue_2_1= RULE_ID | lv_keyValue_2_2= RULE_STRING | lv_keyValue_2_3= ruleFLOAT ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:474:2: ( (lv_keyName_0_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:475:1: (lv_keyName_0_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:475:1: (lv_keyName_0_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:476:3: lv_keyName_0_0= RULE_ID
            {
            lv_keyName_0_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLKey778); 

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

            match(input,15,FOLLOW_15_in_ruleWMLKey793); 

                    createLeafNode(grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:502:1: ( ( (lv_keyValue_2_1= RULE_ID | lv_keyValue_2_2= RULE_STRING | lv_keyValue_2_3= ruleFLOAT ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:503:1: ( (lv_keyValue_2_1= RULE_ID | lv_keyValue_2_2= RULE_STRING | lv_keyValue_2_3= ruleFLOAT ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:503:1: ( (lv_keyValue_2_1= RULE_ID | lv_keyValue_2_2= RULE_STRING | lv_keyValue_2_3= ruleFLOAT ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:504:1: (lv_keyValue_2_1= RULE_ID | lv_keyValue_2_2= RULE_STRING | lv_keyValue_2_3= ruleFLOAT )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:504:1: (lv_keyValue_2_1= RULE_ID | lv_keyValue_2_2= RULE_STRING | lv_keyValue_2_3= ruleFLOAT )
            int alt4=3;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt4=1;
                }
                break;
            case RULE_STRING:
                {
                alt4=2;
                }
                break;
            case RULE_INT:
                {
                alt4=3;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("504:1: (lv_keyValue_2_1= RULE_ID | lv_keyValue_2_2= RULE_STRING | lv_keyValue_2_3= ruleFLOAT )", 4, 0, input);

                throw nvae;
            }

            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:505:3: lv_keyValue_2_1= RULE_ID
                    {
                    lv_keyValue_2_1=(Token)input.LT(1);
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLKey812); 

                    			createLeafNode(grammarAccess.getWMLKeyAccess().getKeyValueIDTerminalRuleCall_2_0_0(), "keyValue"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"keyValue",
                    	        		lv_keyValue_2_1, 
                    	        		"ID", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:526:8: lv_keyValue_2_2= RULE_STRING
                    {
                    lv_keyValue_2_2=(Token)input.LT(1);
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLKey832); 

                    			createLeafNode(grammarAccess.getWMLKeyAccess().getKeyValueSTRINGTerminalRuleCall_2_0_1(), "keyValue"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"keyValue",
                    	        		lv_keyValue_2_2, 
                    	        		"STRING", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:547:8: lv_keyValue_2_3= ruleFLOAT
                    {
                     
                    	        currentNode=createCompositeNode(grammarAccess.getWMLKeyAccess().getKeyValueFLOATParserRuleCall_2_0_2(), currentNode); 
                    	    
                    pushFollow(FOLLOW_ruleFLOAT_in_ruleWMLKey856);
                    lv_keyValue_2_3=ruleFLOAT();
                    _fsp--;


                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getWMLKeyRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode.getParent(), current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"keyValue",
                    	        		lv_keyValue_2_3, 
                    	        		"FLOAT", 
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


    // $ANTLR start entryRuleFLOAT
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:579:1: entryRuleFLOAT returns [String current=null] : iv_ruleFLOAT= ruleFLOAT EOF ;
    public final String entryRuleFLOAT() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleFLOAT = null;


         
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();
        	
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:583:2: (iv_ruleFLOAT= ruleFLOAT EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:584:2: iv_ruleFLOAT= ruleFLOAT EOF
            {
             currentNode = createCompositeNode(grammarAccess.getFLOATRule(), currentNode); 
            pushFollow(FOLLOW_ruleFLOAT_in_entryRuleFLOAT902);
            iv_ruleFLOAT=ruleFLOAT();
            _fsp--;

             current =iv_ruleFLOAT.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleFLOAT913); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:594:1: ruleFLOAT returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (this_INT_0= RULE_INT kw= '.' (this_INT_2= RULE_INT )+ ) ;
    public final AntlrDatatypeRuleToken ruleFLOAT() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_INT_0=null;
        Token kw=null;
        Token this_INT_2=null;

         setCurrentLookahead(); resetLookahead(); 
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens();
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:600:6: ( (this_INT_0= RULE_INT kw= '.' (this_INT_2= RULE_INT )+ ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:601:1: (this_INT_0= RULE_INT kw= '.' (this_INT_2= RULE_INT )+ )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:601:1: (this_INT_0= RULE_INT kw= '.' (this_INT_2= RULE_INT )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:601:6: this_INT_0= RULE_INT kw= '.' (this_INT_2= RULE_INT )+
            {
            this_INT_0=(Token)input.LT(1);
            match(input,RULE_INT,FOLLOW_RULE_INT_in_ruleFLOAT957); 

            		current.merge(this_INT_0);
                
             
                createLeafNode(grammarAccess.getFLOATAccess().getINTTerminalRuleCall_0(), null); 
                
            kw=(Token)input.LT(1);
            match(input,16,FOLLOW_16_in_ruleFLOAT975); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getFLOATAccess().getFullStopKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:614:1: (this_INT_2= RULE_INT )+
            int cnt5=0;
            loop5:
            do {
                int alt5=2;
                int LA5_0 = input.LA(1);

                if ( (LA5_0==RULE_INT) ) {
                    alt5=1;
                }


                switch (alt5) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:614:6: this_INT_2= RULE_INT
            	    {
            	    this_INT_2=(Token)input.LT(1);
            	    match(input,RULE_INT,FOLLOW_RULE_INT_in_ruleFLOAT991); 

            	    		current.merge(this_INT_2);
            	        
            	     
            	        createLeafNode(grammarAccess.getFLOATAccess().getINTTerminalRuleCall_2(), null); 
            	        

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


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot75 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot85 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLRoot131 = new BitSet(new long[]{0x0000000000001402L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_ruleWMLRoot158 = new BitSet(new long[]{0x0000000000001402L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_entryRuleWMLMacro195 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacro205 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_10_in_ruleWMLMacro240 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLMacro257 = new BitSet(new long[]{0x0000000000000810L});
    public static final BitSet FOLLOW_11_in_ruleWMLMacro273 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag309 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag319 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLStartTag_in_ruleWMLTag365 = new BitSet(new long[]{0x0000000000005410L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLTag387 = new BitSet(new long[]{0x0000000000005410L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLTag414 = new BitSet(new long[]{0x0000000000005410L});
    public static final BitSet FOLLOW_ruleWMLMacro_in_ruleWMLTag441 = new BitSet(new long[]{0x0000000000005410L});
    public static final BitSet FOLLOW_ruleWMLEndTag_in_ruleWMLTag464 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLStartTag_in_entryRuleWMLStartTag500 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLStartTag510 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_ruleWMLStartTag545 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLStartTag562 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_13_in_ruleWMLStartTag577 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLEndTag_in_entryRuleWMLEndTag613 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLEndTag623 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_ruleWMLEndTag658 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLEndTag675 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_13_in_ruleWMLEndTag690 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey726 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey736 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKey778 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_15_in_ruleWMLKey793 = new BitSet(new long[]{0x0000000000000070L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKey812 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLKey832 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_ruleWMLKey856 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleFLOAT_in_entryRuleFLOAT902 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleFLOAT913 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_INT_in_ruleFLOAT957 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_ruleFLOAT975 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_INT_in_ruleFLOAT991 = new BitSet(new long[]{0x0000000000000042L});

}