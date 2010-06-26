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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_PATH", "RULE_STRING", "RULE_IDLIST", "RULE_SL_COMMENT", "RULE_WS", "RULE_ANY_OTHER", "'#textdomain '", "'{'", "'}'", "'['", "']'", "'/'", "'+'", "'='", "'_'", "'~'"
    };
    public static final int RULE_ID=4;
    public static final int RULE_STRING=6;
    public static final int RULE_IDLIST=7;
    public static final int RULE_ANY_OTHER=10;
    public static final int RULE_PATH=5;
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
        	return "Root";	
       	}
       	
       	@Override
       	protected WMLGrammarAccess getGrammarAccess() {
       		return grammarAccess;
       	}



    // $ANTLR start entryRuleRoot
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:77:1: entryRuleRoot returns [EObject current=null] : iv_ruleRoot= ruleRoot EOF ;
    public final EObject entryRuleRoot() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleRoot = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:78:2: (iv_ruleRoot= ruleRoot EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:79:2: iv_ruleRoot= ruleRoot EOF
            {
             currentNode = createCompositeNode(grammarAccess.getRootRule(), currentNode); 
            pushFollow(FOLLOW_ruleRoot_in_entryRuleRoot75);
            iv_ruleRoot=ruleRoot();
            _fsp--;

             current =iv_ruleRoot; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleRoot85); 

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
    // $ANTLR end entryRuleRoot


    // $ANTLR start ruleRoot
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:86:1: ruleRoot returns [EObject current=null] : ( ( (lv_textdomains_0_0= ruleTextDomain ) )* ( (lv_preproc_1_0= rulePreprocessor ) )* ( (lv_roots_2_0= ruleRootType ) )* ) ;
    public final EObject ruleRoot() throws RecognitionException {
        EObject current = null;

        EObject lv_textdomains_0_0 = null;

        EObject lv_preproc_1_0 = null;

        EObject lv_roots_2_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:91:6: ( ( ( (lv_textdomains_0_0= ruleTextDomain ) )* ( (lv_preproc_1_0= rulePreprocessor ) )* ( (lv_roots_2_0= ruleRootType ) )* ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( ( (lv_textdomains_0_0= ruleTextDomain ) )* ( (lv_preproc_1_0= rulePreprocessor ) )* ( (lv_roots_2_0= ruleRootType ) )* )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( ( (lv_textdomains_0_0= ruleTextDomain ) )* ( (lv_preproc_1_0= rulePreprocessor ) )* ( (lv_roots_2_0= ruleRootType ) )* )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:2: ( (lv_textdomains_0_0= ruleTextDomain ) )* ( (lv_preproc_1_0= rulePreprocessor ) )* ( (lv_roots_2_0= ruleRootType ) )*
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:2: ( (lv_textdomains_0_0= ruleTextDomain ) )*
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( (LA1_0==11) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:93:1: (lv_textdomains_0_0= ruleTextDomain )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:93:1: (lv_textdomains_0_0= ruleTextDomain )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:94:3: lv_textdomains_0_0= ruleTextDomain
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getRootAccess().getTextdomainsTextDomainParserRuleCall_0_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleTextDomain_in_ruleRoot131);
            	    lv_textdomains_0_0=ruleTextDomain();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"textdomains",
            	    	        		lv_textdomains_0_0, 
            	    	        		"TextDomain", 
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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:116:3: ( (lv_preproc_1_0= rulePreprocessor ) )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( (LA2_0==12) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:117:1: (lv_preproc_1_0= rulePreprocessor )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:117:1: (lv_preproc_1_0= rulePreprocessor )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:118:3: lv_preproc_1_0= rulePreprocessor
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getRootAccess().getPreprocPreprocessorParserRuleCall_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_rulePreprocessor_in_ruleRoot153);
            	    lv_preproc_1_0=rulePreprocessor();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"preproc",
            	    	        		lv_preproc_1_0, 
            	    	        		"Preprocessor", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:140:3: ( (lv_roots_2_0= ruleRootType ) )*
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( (LA3_0==14) ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:141:1: (lv_roots_2_0= ruleRootType )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:141:1: (lv_roots_2_0= ruleRootType )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:142:3: lv_roots_2_0= ruleRootType
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getRootAccess().getRootsRootTypeParserRuleCall_2_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleRootType_in_ruleRoot175);
            	    lv_roots_2_0=ruleRootType();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getRootRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"roots",
            	    	        		lv_roots_2_0, 
            	    	        		"RootType", 
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
    // $ANTLR end ruleRoot


    // $ANTLR start entryRuleTextDomain
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:172:1: entryRuleTextDomain returns [EObject current=null] : iv_ruleTextDomain= ruleTextDomain EOF ;
    public final EObject entryRuleTextDomain() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleTextDomain = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:173:2: (iv_ruleTextDomain= ruleTextDomain EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:174:2: iv_ruleTextDomain= ruleTextDomain EOF
            {
             currentNode = createCompositeNode(grammarAccess.getTextDomainRule(), currentNode); 
            pushFollow(FOLLOW_ruleTextDomain_in_entryRuleTextDomain212);
            iv_ruleTextDomain=ruleTextDomain();
            _fsp--;

             current =iv_ruleTextDomain; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTextDomain222); 

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
    // $ANTLR end entryRuleTextDomain


    // $ANTLR start ruleTextDomain
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:181:1: ruleTextDomain returns [EObject current=null] : ( '#textdomain ' ( (lv_DomainName_1_0= RULE_ID ) ) ) ;
    public final EObject ruleTextDomain() throws RecognitionException {
        EObject current = null;

        Token lv_DomainName_1_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:186:6: ( ( '#textdomain ' ( (lv_DomainName_1_0= RULE_ID ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:187:1: ( '#textdomain ' ( (lv_DomainName_1_0= RULE_ID ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:187:1: ( '#textdomain ' ( (lv_DomainName_1_0= RULE_ID ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:187:3: '#textdomain ' ( (lv_DomainName_1_0= RULE_ID ) )
            {
            match(input,11,FOLLOW_11_in_ruleTextDomain257); 

                    createLeafNode(grammarAccess.getTextDomainAccess().getTextdomainKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:191:1: ( (lv_DomainName_1_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:192:1: (lv_DomainName_1_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:192:1: (lv_DomainName_1_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:193:3: lv_DomainName_1_0= RULE_ID
            {
            lv_DomainName_1_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleTextDomain274); 

            			createLeafNode(grammarAccess.getTextDomainAccess().getDomainNameIDTerminalRuleCall_1_0(), "DomainName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getTextDomainRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"DomainName",
            	        		lv_DomainName_1_0, 
            	        		"ID", 
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
    // $ANTLR end ruleTextDomain


    // $ANTLR start entryRulePreprocessor
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:223:1: entryRulePreprocessor returns [EObject current=null] : iv_rulePreprocessor= rulePreprocessor EOF ;
    public final EObject entryRulePreprocessor() throws RecognitionException {
        EObject current = null;

        EObject iv_rulePreprocessor = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:224:2: (iv_rulePreprocessor= rulePreprocessor EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:225:2: iv_rulePreprocessor= rulePreprocessor EOF
            {
             currentNode = createCompositeNode(grammarAccess.getPreprocessorRule(), currentNode); 
            pushFollow(FOLLOW_rulePreprocessor_in_entryRulePreprocessor315);
            iv_rulePreprocessor=rulePreprocessor();
            _fsp--;

             current =iv_rulePreprocessor; 
            match(input,EOF,FOLLOW_EOF_in_entryRulePreprocessor325); 

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
    // $ANTLR end entryRulePreprocessor


    // $ANTLR start rulePreprocessor
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:232:1: rulePreprocessor returns [EObject current=null] : (this_Macro_0= ruleMacro | this_PathInclude_1= rulePathInclude ) ;
    public final EObject rulePreprocessor() throws RecognitionException {
        EObject current = null;

        EObject this_Macro_0 = null;

        EObject this_PathInclude_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:237:6: ( (this_Macro_0= ruleMacro | this_PathInclude_1= rulePathInclude ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:238:1: (this_Macro_0= ruleMacro | this_PathInclude_1= rulePathInclude )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:238:1: (this_Macro_0= ruleMacro | this_PathInclude_1= rulePathInclude )
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( (LA4_0==12) ) {
                int LA4_1 = input.LA(2);

                if ( (LA4_1==RULE_PATH||LA4_1==20) ) {
                    alt4=2;
                }
                else if ( (LA4_1==RULE_ID) ) {
                    alt4=1;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("238:1: (this_Macro_0= ruleMacro | this_PathInclude_1= rulePathInclude )", 4, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("238:1: (this_Macro_0= ruleMacro | this_PathInclude_1= rulePathInclude )", 4, 0, input);

                throw nvae;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:239:5: this_Macro_0= ruleMacro
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getPreprocessorAccess().getMacroParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleMacro_in_rulePreprocessor372);
                    this_Macro_0=ruleMacro();
                    _fsp--;

                     
                            current = this_Macro_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:249:5: this_PathInclude_1= rulePathInclude
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getPreprocessorAccess().getPathIncludeParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_rulePathInclude_in_rulePreprocessor399);
                    this_PathInclude_1=rulePathInclude();
                    _fsp--;

                     
                            current = this_PathInclude_1; 
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
    // $ANTLR end rulePreprocessor


    // $ANTLR start entryRuleMacro
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:265:1: entryRuleMacro returns [EObject current=null] : iv_ruleMacro= ruleMacro EOF ;
    public final EObject entryRuleMacro() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleMacro = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:266:2: (iv_ruleMacro= ruleMacro EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:267:2: iv_ruleMacro= ruleMacro EOF
            {
             currentNode = createCompositeNode(grammarAccess.getMacroRule(), currentNode); 
            pushFollow(FOLLOW_ruleMacro_in_entryRuleMacro434);
            iv_ruleMacro=ruleMacro();
            _fsp--;

             current =iv_ruleMacro; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleMacro444); 

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
    // $ANTLR end entryRuleMacro


    // $ANTLR start ruleMacro
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:274:1: ruleMacro returns [EObject current=null] : ( '{' ( (lv_macroContent_1_0= RULE_ID ) )+ '}' ) ;
    public final EObject ruleMacro() throws RecognitionException {
        EObject current = null;

        Token lv_macroContent_1_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:279:6: ( ( '{' ( (lv_macroContent_1_0= RULE_ID ) )+ '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:280:1: ( '{' ( (lv_macroContent_1_0= RULE_ID ) )+ '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:280:1: ( '{' ( (lv_macroContent_1_0= RULE_ID ) )+ '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:280:3: '{' ( (lv_macroContent_1_0= RULE_ID ) )+ '}'
            {
            match(input,12,FOLLOW_12_in_ruleMacro479); 

                    createLeafNode(grammarAccess.getMacroAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:284:1: ( (lv_macroContent_1_0= RULE_ID ) )+
            int cnt5=0;
            loop5:
            do {
                int alt5=2;
                int LA5_0 = input.LA(1);

                if ( (LA5_0==RULE_ID) ) {
                    alt5=1;
                }


                switch (alt5) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:285:1: (lv_macroContent_1_0= RULE_ID )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:285:1: (lv_macroContent_1_0= RULE_ID )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:286:3: lv_macroContent_1_0= RULE_ID
            	    {
            	    lv_macroContent_1_0=(Token)input.LT(1);
            	    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleMacro496); 

            	    			createLeafNode(grammarAccess.getMacroAccess().getMacroContentIDTerminalRuleCall_1_0(), "macroContent"); 
            	    		

            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getMacroRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode, current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"macroContent",
            	    	        		lv_macroContent_1_0, 
            	    	        		"ID", 
            	    	        		lastConsumedNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	    

            	    }


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

            match(input,13,FOLLOW_13_in_ruleMacro512); 

                    createLeafNode(grammarAccess.getMacroAccess().getRightCurlyBracketKeyword_2(), null); 
                

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
    // $ANTLR end ruleMacro


    // $ANTLR start entryRulePathInclude
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:320:1: entryRulePathInclude returns [EObject current=null] : iv_rulePathInclude= rulePathInclude EOF ;
    public final EObject entryRulePathInclude() throws RecognitionException {
        EObject current = null;

        EObject iv_rulePathInclude = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:321:2: (iv_rulePathInclude= rulePathInclude EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:322:2: iv_rulePathInclude= rulePathInclude EOF
            {
             currentNode = createCompositeNode(grammarAccess.getPathIncludeRule(), currentNode); 
            pushFollow(FOLLOW_rulePathInclude_in_entryRulePathInclude548);
            iv_rulePathInclude=rulePathInclude();
            _fsp--;

             current =iv_rulePathInclude; 
            match(input,EOF,FOLLOW_EOF_in_entryRulePathInclude558); 

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
    // $ANTLR end entryRulePathInclude


    // $ANTLR start rulePathInclude
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:329:1: rulePathInclude returns [EObject current=null] : ( '{' ( ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) ) ) '}' ) ;
    public final EObject rulePathInclude() throws RecognitionException {
        EObject current = null;

        Token lv_path_1_2=null;
        AntlrDatatypeRuleToken lv_path_1_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:334:6: ( ( '{' ( ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) ) ) '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:335:1: ( '{' ( ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) ) ) '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:335:1: ( '{' ( ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) ) ) '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:335:3: '{' ( ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) ) ) '}'
            {
            match(input,12,FOLLOW_12_in_rulePathInclude593); 

                    createLeafNode(grammarAccess.getPathIncludeAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:339:1: ( ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:340:1: ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:340:1: ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:341:1: (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:341:1: (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH )
            int alt6=2;
            int LA6_0 = input.LA(1);

            if ( (LA6_0==20) ) {
                alt6=1;
            }
            else if ( (LA6_0==RULE_PATH) ) {
                alt6=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("341:1: (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH )", 6, 0, input);

                throw nvae;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:342:3: lv_path_1_1= ruleHOMEPATH
                    {
                     
                    	        currentNode=createCompositeNode(grammarAccess.getPathIncludeAccess().getPathHOMEPATHParserRuleCall_1_0_0(), currentNode); 
                    	    
                    pushFollow(FOLLOW_ruleHOMEPATH_in_rulePathInclude616);
                    lv_path_1_1=ruleHOMEPATH();
                    _fsp--;


                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getPathIncludeRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode.getParent(), current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"path",
                    	        		lv_path_1_1, 
                    	        		"HOMEPATH", 
                    	        		currentNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	        currentNode = currentNode.getParent();
                    	    

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:363:8: lv_path_1_2= RULE_PATH
                    {
                    lv_path_1_2=(Token)input.LT(1);
                    match(input,RULE_PATH,FOLLOW_RULE_PATH_in_rulePathInclude631); 

                    			createLeafNode(grammarAccess.getPathIncludeAccess().getPathPATHTerminalRuleCall_1_0_1(), "path"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getPathIncludeRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"path",
                    	        		lv_path_1_2, 
                    	        		"PATH", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;

            }


            }


            }

            match(input,13,FOLLOW_13_in_rulePathInclude649); 

                    createLeafNode(grammarAccess.getPathIncludeAccess().getRightCurlyBracketKeyword_2(), null); 
                

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
    // $ANTLR end rulePathInclude


    // $ANTLR start entryRuleRootType
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:399:1: entryRuleRootType returns [EObject current=null] : iv_ruleRootType= ruleRootType EOF ;
    public final EObject entryRuleRootType() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleRootType = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:400:2: (iv_ruleRootType= ruleRootType EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:401:2: iv_ruleRootType= ruleRootType EOF
            {
             currentNode = createCompositeNode(grammarAccess.getRootTypeRule(), currentNode); 
            pushFollow(FOLLOW_ruleRootType_in_entryRuleRootType685);
            iv_ruleRootType=ruleRootType();
            _fsp--;

             current =iv_ruleRootType; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleRootType695); 

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
    // $ANTLR end entryRuleRootType


    // $ANTLR start ruleRootType
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:408:1: ruleRootType returns [EObject current=null] : ( '[' ( (lv_startTag_1_0= ruleRootTag ) ) ']' ( (lv_subTypes_3_0= ruleRootType ) )* ( (lv_at_4_0= ruleAttributes ) )* ( (lv_okpreproc_5_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_8_0= ruleRootTag ) ) ']' ) ;
    public final EObject ruleRootType() throws RecognitionException {
        EObject current = null;

        EObject lv_startTag_1_0 = null;

        EObject lv_subTypes_3_0 = null;

        EObject lv_at_4_0 = null;

        EObject lv_okpreproc_5_0 = null;

        EObject lv_endTag_8_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:413:6: ( ( '[' ( (lv_startTag_1_0= ruleRootTag ) ) ']' ( (lv_subTypes_3_0= ruleRootType ) )* ( (lv_at_4_0= ruleAttributes ) )* ( (lv_okpreproc_5_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_8_0= ruleRootTag ) ) ']' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:414:1: ( '[' ( (lv_startTag_1_0= ruleRootTag ) ) ']' ( (lv_subTypes_3_0= ruleRootType ) )* ( (lv_at_4_0= ruleAttributes ) )* ( (lv_okpreproc_5_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_8_0= ruleRootTag ) ) ']' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:414:1: ( '[' ( (lv_startTag_1_0= ruleRootTag ) ) ']' ( (lv_subTypes_3_0= ruleRootType ) )* ( (lv_at_4_0= ruleAttributes ) )* ( (lv_okpreproc_5_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_8_0= ruleRootTag ) ) ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:414:3: '[' ( (lv_startTag_1_0= ruleRootTag ) ) ']' ( (lv_subTypes_3_0= ruleRootType ) )* ( (lv_at_4_0= ruleAttributes ) )* ( (lv_okpreproc_5_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_8_0= ruleRootTag ) ) ']'
            {
            match(input,14,FOLLOW_14_in_ruleRootType730); 

                    createLeafNode(grammarAccess.getRootTypeAccess().getLeftSquareBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:418:1: ( (lv_startTag_1_0= ruleRootTag ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:419:1: (lv_startTag_1_0= ruleRootTag )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:419:1: (lv_startTag_1_0= ruleRootTag )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:420:3: lv_startTag_1_0= ruleRootTag
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getRootTypeAccess().getStartTagRootTagParserRuleCall_1_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleRootTag_in_ruleRootType751);
            lv_startTag_1_0=ruleRootTag();
            _fsp--;


            	        if (current==null) {
            	            current = factory.create(grammarAccess.getRootTypeRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"startTag",
            	        		lv_startTag_1_0, 
            	        		"RootTag", 
            	        		currentNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	        currentNode = currentNode.getParent();
            	    

            }


            }

            match(input,15,FOLLOW_15_in_ruleRootType761); 

                    createLeafNode(grammarAccess.getRootTypeAccess().getRightSquareBracketKeyword_2(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:446:1: ( (lv_subTypes_3_0= ruleRootType ) )*
            loop7:
            do {
                int alt7=2;
                int LA7_0 = input.LA(1);

                if ( (LA7_0==14) ) {
                    int LA7_2 = input.LA(2);

                    if ( (LA7_2==16) ) {
                        int LA7_3 = input.LA(3);

                        if ( (LA7_3==RULE_ID) ) {
                            int LA7_5 = input.LA(4);

                            if ( (LA7_5==15) ) {
                                alt7=1;
                            }


                        }


                    }
                    else if ( (LA7_2==RULE_ID||LA7_2==17) ) {
                        alt7=1;
                    }


                }


                switch (alt7) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:447:1: (lv_subTypes_3_0= ruleRootType )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:447:1: (lv_subTypes_3_0= ruleRootType )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:448:3: lv_subTypes_3_0= ruleRootType
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getRootTypeAccess().getSubTypesRootTypeParserRuleCall_3_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleRootType_in_ruleRootType782);
            	    lv_subTypes_3_0=ruleRootType();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getRootTypeRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"subTypes",
            	    	        		lv_subTypes_3_0, 
            	    	        		"RootType", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop7;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:470:3: ( (lv_at_4_0= ruleAttributes ) )*
            loop8:
            do {
                int alt8=2;
                int LA8_0 = input.LA(1);

                if ( (LA8_0==RULE_ID) ) {
                    alt8=1;
                }


                switch (alt8) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:471:1: (lv_at_4_0= ruleAttributes )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:471:1: (lv_at_4_0= ruleAttributes )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:472:3: lv_at_4_0= ruleAttributes
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getRootTypeAccess().getAtAttributesParserRuleCall_4_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleAttributes_in_ruleRootType804);
            	    lv_at_4_0=ruleAttributes();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getRootTypeRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"at",
            	    	        		lv_at_4_0, 
            	    	        		"Attributes", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop8;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:494:3: ( (lv_okpreproc_5_0= rulePreprocessor ) )*
            loop9:
            do {
                int alt9=2;
                int LA9_0 = input.LA(1);

                if ( (LA9_0==12) ) {
                    alt9=1;
                }


                switch (alt9) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:495:1: (lv_okpreproc_5_0= rulePreprocessor )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:495:1: (lv_okpreproc_5_0= rulePreprocessor )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:496:3: lv_okpreproc_5_0= rulePreprocessor
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getRootTypeAccess().getOkpreprocPreprocessorParserRuleCall_5_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_rulePreprocessor_in_ruleRootType826);
            	    lv_okpreproc_5_0=rulePreprocessor();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getRootTypeRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		add(
            	    	       			current, 
            	    	       			"okpreproc",
            	    	        		lv_okpreproc_5_0, 
            	    	        		"Preprocessor", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop9;
                }
            } while (true);

            match(input,14,FOLLOW_14_in_ruleRootType837); 

                    createLeafNode(grammarAccess.getRootTypeAccess().getLeftSquareBracketKeyword_6(), null); 
                
            match(input,16,FOLLOW_16_in_ruleRootType847); 

                    createLeafNode(grammarAccess.getRootTypeAccess().getSolidusKeyword_7(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:526:1: ( (lv_endTag_8_0= ruleRootTag ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:527:1: (lv_endTag_8_0= ruleRootTag )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:527:1: (lv_endTag_8_0= ruleRootTag )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:528:3: lv_endTag_8_0= ruleRootTag
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getRootTypeAccess().getEndTagRootTagParserRuleCall_8_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleRootTag_in_ruleRootType868);
            lv_endTag_8_0=ruleRootTag();
            _fsp--;


            	        if (current==null) {
            	            current = factory.create(grammarAccess.getRootTypeRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"endTag",
            	        		lv_endTag_8_0, 
            	        		"RootTag", 
            	        		currentNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	        currentNode = currentNode.getParent();
            	    

            }


            }

            match(input,15,FOLLOW_15_in_ruleRootType878); 

                    createLeafNode(grammarAccess.getRootTypeAccess().getRightSquareBracketKeyword_9(), null); 
                

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
    // $ANTLR end ruleRootType


    // $ANTLR start entryRuleRootTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:562:1: entryRuleRootTag returns [EObject current=null] : iv_ruleRootTag= ruleRootTag EOF ;
    public final EObject entryRuleRootTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleRootTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:563:2: (iv_ruleRootTag= ruleRootTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:564:2: iv_ruleRootTag= ruleRootTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getRootTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleRootTag_in_entryRuleRootTag914);
            iv_ruleRootTag=ruleRootTag();
            _fsp--;

             current =iv_ruleRootTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleRootTag924); 

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
    // $ANTLR end entryRuleRootTag


    // $ANTLR start ruleRootTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:571:1: ruleRootTag returns [EObject current=null] : (this_SimpleTag_0= ruleSimpleTag | this_AddedTag_1= ruleAddedTag ) ;
    public final EObject ruleRootTag() throws RecognitionException {
        EObject current = null;

        EObject this_SimpleTag_0 = null;

        EObject this_AddedTag_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:576:6: ( (this_SimpleTag_0= ruleSimpleTag | this_AddedTag_1= ruleAddedTag ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:577:1: (this_SimpleTag_0= ruleSimpleTag | this_AddedTag_1= ruleAddedTag )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:577:1: (this_SimpleTag_0= ruleSimpleTag | this_AddedTag_1= ruleAddedTag )
            int alt10=2;
            int LA10_0 = input.LA(1);

            if ( (LA10_0==RULE_ID||LA10_0==16) ) {
                alt10=1;
            }
            else if ( (LA10_0==17) ) {
                alt10=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("577:1: (this_SimpleTag_0= ruleSimpleTag | this_AddedTag_1= ruleAddedTag )", 10, 0, input);

                throw nvae;
            }
            switch (alt10) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:578:5: this_SimpleTag_0= ruleSimpleTag
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getRootTagAccess().getSimpleTagParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleSimpleTag_in_ruleRootTag971);
                    this_SimpleTag_0=ruleSimpleTag();
                    _fsp--;

                     
                            current = this_SimpleTag_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:588:5: this_AddedTag_1= ruleAddedTag
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getRootTagAccess().getAddedTagParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleAddedTag_in_ruleRootTag998);
                    this_AddedTag_1=ruleAddedTag();
                    _fsp--;

                     
                            current = this_AddedTag_1; 
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
    // $ANTLR end ruleRootTag


    // $ANTLR start entryRuleSimpleTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:604:1: entryRuleSimpleTag returns [EObject current=null] : iv_ruleSimpleTag= ruleSimpleTag EOF ;
    public final EObject entryRuleSimpleTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleSimpleTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:605:2: (iv_ruleSimpleTag= ruleSimpleTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:606:2: iv_ruleSimpleTag= ruleSimpleTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getSimpleTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleSimpleTag_in_entryRuleSimpleTag1033);
            iv_ruleSimpleTag=ruleSimpleTag();
            _fsp--;

             current =iv_ruleSimpleTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleSimpleTag1043); 

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
    // $ANTLR end entryRuleSimpleTag


    // $ANTLR start ruleSimpleTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:613:1: ruleSimpleTag returns [EObject current=null] : ( ( (lv_endTag_0_0= '/' ) )? ( (lv_tagName_1_0= ruleRootTagsList ) ) ) ;
    public final EObject ruleSimpleTag() throws RecognitionException {
        EObject current = null;

        Token lv_endTag_0_0=null;
        AntlrDatatypeRuleToken lv_tagName_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:618:6: ( ( ( (lv_endTag_0_0= '/' ) )? ( (lv_tagName_1_0= ruleRootTagsList ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:619:1: ( ( (lv_endTag_0_0= '/' ) )? ( (lv_tagName_1_0= ruleRootTagsList ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:619:1: ( ( (lv_endTag_0_0= '/' ) )? ( (lv_tagName_1_0= ruleRootTagsList ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:619:2: ( (lv_endTag_0_0= '/' ) )? ( (lv_tagName_1_0= ruleRootTagsList ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:619:2: ( (lv_endTag_0_0= '/' ) )?
            int alt11=2;
            int LA11_0 = input.LA(1);

            if ( (LA11_0==16) ) {
                alt11=1;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:620:1: (lv_endTag_0_0= '/' )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:620:1: (lv_endTag_0_0= '/' )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:621:3: lv_endTag_0_0= '/'
                    {
                    lv_endTag_0_0=(Token)input.LT(1);
                    match(input,16,FOLLOW_16_in_ruleSimpleTag1086); 

                            createLeafNode(grammarAccess.getSimpleTagAccess().getEndTagSolidusKeyword_0_0(), "endTag"); 
                        

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getSimpleTagRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        
                    	        try {
                    	       		set(current, "endTag", true, "/", lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }


                    }
                    break;

            }

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:640:3: ( (lv_tagName_1_0= ruleRootTagsList ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:641:1: (lv_tagName_1_0= ruleRootTagsList )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:641:1: (lv_tagName_1_0= ruleRootTagsList )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:642:3: lv_tagName_1_0= ruleRootTagsList
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getSimpleTagAccess().getTagNameRootTagsListParserRuleCall_1_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleRootTagsList_in_ruleSimpleTag1121);
            lv_tagName_1_0=ruleRootTagsList();
            _fsp--;


            	        if (current==null) {
            	            current = factory.create(grammarAccess.getSimpleTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"tagName",
            	        		lv_tagName_1_0, 
            	        		"RootTagsList", 
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
    // $ANTLR end ruleSimpleTag


    // $ANTLR start entryRuleAddedTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:672:1: entryRuleAddedTag returns [EObject current=null] : iv_ruleAddedTag= ruleAddedTag EOF ;
    public final EObject entryRuleAddedTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleAddedTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:673:2: (iv_ruleAddedTag= ruleAddedTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:674:2: iv_ruleAddedTag= ruleAddedTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getAddedTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleAddedTag_in_entryRuleAddedTag1157);
            iv_ruleAddedTag=ruleAddedTag();
            _fsp--;

             current =iv_ruleAddedTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleAddedTag1167); 

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
    // $ANTLR end entryRuleAddedTag


    // $ANTLR start ruleAddedTag
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:681:1: ruleAddedTag returns [EObject current=null] : ( '+' ( (lv_tagName_1_0= ruleRootTagsList ) ) ) ;
    public final EObject ruleAddedTag() throws RecognitionException {
        EObject current = null;

        AntlrDatatypeRuleToken lv_tagName_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:686:6: ( ( '+' ( (lv_tagName_1_0= ruleRootTagsList ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:687:1: ( '+' ( (lv_tagName_1_0= ruleRootTagsList ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:687:1: ( '+' ( (lv_tagName_1_0= ruleRootTagsList ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:687:3: '+' ( (lv_tagName_1_0= ruleRootTagsList ) )
            {
            match(input,17,FOLLOW_17_in_ruleAddedTag1202); 

                    createLeafNode(grammarAccess.getAddedTagAccess().getPlusSignKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:691:1: ( (lv_tagName_1_0= ruleRootTagsList ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:692:1: (lv_tagName_1_0= ruleRootTagsList )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:692:1: (lv_tagName_1_0= ruleRootTagsList )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:693:3: lv_tagName_1_0= ruleRootTagsList
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getAddedTagAccess().getTagNameRootTagsListParserRuleCall_1_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleRootTagsList_in_ruleAddedTag1223);
            lv_tagName_1_0=ruleRootTagsList();
            _fsp--;


            	        if (current==null) {
            	            current = factory.create(grammarAccess.getAddedTagRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode.getParent(), current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"tagName",
            	        		lv_tagName_1_0, 
            	        		"RootTagsList", 
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
    // $ANTLR end ruleAddedTag


    // $ANTLR start entryRuleRootTagsList
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:723:1: entryRuleRootTagsList returns [String current=null] : iv_ruleRootTagsList= ruleRootTagsList EOF ;
    public final String entryRuleRootTagsList() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleRootTagsList = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:724:2: (iv_ruleRootTagsList= ruleRootTagsList EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:725:2: iv_ruleRootTagsList= ruleRootTagsList EOF
            {
             currentNode = createCompositeNode(grammarAccess.getRootTagsListRule(), currentNode); 
            pushFollow(FOLLOW_ruleRootTagsList_in_entryRuleRootTagsList1260);
            iv_ruleRootTagsList=ruleRootTagsList();
            _fsp--;

             current =iv_ruleRootTagsList.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleRootTagsList1271); 

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
    // $ANTLR end entryRuleRootTagsList


    // $ANTLR start ruleRootTagsList
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:732:1: ruleRootTagsList returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : this_ID_0= RULE_ID ;
    public final AntlrDatatypeRuleToken ruleRootTagsList() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:737:6: (this_ID_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:738:5: this_ID_0= RULE_ID
            {
            this_ID_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleRootTagsList1310); 

            		current.merge(this_ID_0);
                
             
                createLeafNode(grammarAccess.getRootTagsListAccess().getIDTerminalRuleCall(), null); 
                

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
    // $ANTLR end ruleRootTagsList


    // $ANTLR start entryRuleAttributes
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:753:1: entryRuleAttributes returns [EObject current=null] : iv_ruleAttributes= ruleAttributes EOF ;
    public final EObject entryRuleAttributes() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleAttributes = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:754:2: (iv_ruleAttributes= ruleAttributes EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:755:2: iv_ruleAttributes= ruleAttributes EOF
            {
             currentNode = createCompositeNode(grammarAccess.getAttributesRule(), currentNode); 
            pushFollow(FOLLOW_ruleAttributes_in_entryRuleAttributes1354);
            iv_ruleAttributes=ruleAttributes();
            _fsp--;

             current =iv_ruleAttributes; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleAttributes1364); 

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
    // $ANTLR end entryRuleAttributes


    // $ANTLR start ruleAttributes
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:762:1: ruleAttributes returns [EObject current=null] : ( ( (lv_attrName_0_0= RULE_ID ) ) '=' ( ( (lv_attrValue_2_1= ruleTSTRING | lv_attrValue_2_2= RULE_STRING | lv_attrValue_2_3= RULE_PATH | lv_attrValue_2_4= RULE_ID | lv_attrValue_2_5= RULE_IDLIST ) ) ) ) ;
    public final EObject ruleAttributes() throws RecognitionException {
        EObject current = null;

        Token lv_attrName_0_0=null;
        Token lv_attrValue_2_2=null;
        Token lv_attrValue_2_3=null;
        Token lv_attrValue_2_4=null;
        Token lv_attrValue_2_5=null;
        AntlrDatatypeRuleToken lv_attrValue_2_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:767:6: ( ( ( (lv_attrName_0_0= RULE_ID ) ) '=' ( ( (lv_attrValue_2_1= ruleTSTRING | lv_attrValue_2_2= RULE_STRING | lv_attrValue_2_3= RULE_PATH | lv_attrValue_2_4= RULE_ID | lv_attrValue_2_5= RULE_IDLIST ) ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:768:1: ( ( (lv_attrName_0_0= RULE_ID ) ) '=' ( ( (lv_attrValue_2_1= ruleTSTRING | lv_attrValue_2_2= RULE_STRING | lv_attrValue_2_3= RULE_PATH | lv_attrValue_2_4= RULE_ID | lv_attrValue_2_5= RULE_IDLIST ) ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:768:1: ( ( (lv_attrName_0_0= RULE_ID ) ) '=' ( ( (lv_attrValue_2_1= ruleTSTRING | lv_attrValue_2_2= RULE_STRING | lv_attrValue_2_3= RULE_PATH | lv_attrValue_2_4= RULE_ID | lv_attrValue_2_5= RULE_IDLIST ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:768:2: ( (lv_attrName_0_0= RULE_ID ) ) '=' ( ( (lv_attrValue_2_1= ruleTSTRING | lv_attrValue_2_2= RULE_STRING | lv_attrValue_2_3= RULE_PATH | lv_attrValue_2_4= RULE_ID | lv_attrValue_2_5= RULE_IDLIST ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:768:2: ( (lv_attrName_0_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:769:1: (lv_attrName_0_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:769:1: (lv_attrName_0_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:770:3: lv_attrName_0_0= RULE_ID
            {
            lv_attrName_0_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleAttributes1406); 

            			createLeafNode(grammarAccess.getAttributesAccess().getAttrNameIDTerminalRuleCall_0_0(), "attrName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getAttributesRule().getType().getClassifier());
            	            associateNodeWithAstElement(currentNode, current);
            	        }
            	        try {
            	       		set(
            	       			current, 
            	       			"attrName",
            	        		lv_attrName_0_0, 
            	        		"ID", 
            	        		lastConsumedNode);
            	        } catch (ValueConverterException vce) {
            				handleValueConverterException(vce);
            	        }
            	    

            }


            }

            match(input,18,FOLLOW_18_in_ruleAttributes1421); 

                    createLeafNode(grammarAccess.getAttributesAccess().getEqualsSignKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:796:1: ( ( (lv_attrValue_2_1= ruleTSTRING | lv_attrValue_2_2= RULE_STRING | lv_attrValue_2_3= RULE_PATH | lv_attrValue_2_4= RULE_ID | lv_attrValue_2_5= RULE_IDLIST ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:797:1: ( (lv_attrValue_2_1= ruleTSTRING | lv_attrValue_2_2= RULE_STRING | lv_attrValue_2_3= RULE_PATH | lv_attrValue_2_4= RULE_ID | lv_attrValue_2_5= RULE_IDLIST ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:797:1: ( (lv_attrValue_2_1= ruleTSTRING | lv_attrValue_2_2= RULE_STRING | lv_attrValue_2_3= RULE_PATH | lv_attrValue_2_4= RULE_ID | lv_attrValue_2_5= RULE_IDLIST ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:798:1: (lv_attrValue_2_1= ruleTSTRING | lv_attrValue_2_2= RULE_STRING | lv_attrValue_2_3= RULE_PATH | lv_attrValue_2_4= RULE_ID | lv_attrValue_2_5= RULE_IDLIST )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:798:1: (lv_attrValue_2_1= ruleTSTRING | lv_attrValue_2_2= RULE_STRING | lv_attrValue_2_3= RULE_PATH | lv_attrValue_2_4= RULE_ID | lv_attrValue_2_5= RULE_IDLIST )
            int alt12=5;
            switch ( input.LA(1) ) {
            case 19:
                {
                alt12=1;
                }
                break;
            case RULE_STRING:
                {
                alt12=2;
                }
                break;
            case RULE_PATH:
                {
                alt12=3;
                }
                break;
            case RULE_ID:
                {
                alt12=4;
                }
                break;
            case RULE_IDLIST:
                {
                alt12=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("798:1: (lv_attrValue_2_1= ruleTSTRING | lv_attrValue_2_2= RULE_STRING | lv_attrValue_2_3= RULE_PATH | lv_attrValue_2_4= RULE_ID | lv_attrValue_2_5= RULE_IDLIST )", 12, 0, input);

                throw nvae;
            }

            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:799:3: lv_attrValue_2_1= ruleTSTRING
                    {
                     
                    	        currentNode=createCompositeNode(grammarAccess.getAttributesAccess().getAttrValueTSTRINGParserRuleCall_2_0_0(), currentNode); 
                    	    
                    pushFollow(FOLLOW_ruleTSTRING_in_ruleAttributes1444);
                    lv_attrValue_2_1=ruleTSTRING();
                    _fsp--;


                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getAttributesRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode.getParent(), current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"attrValue",
                    	        		lv_attrValue_2_1, 
                    	        		"TSTRING", 
                    	        		currentNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	        currentNode = currentNode.getParent();
                    	    

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:820:8: lv_attrValue_2_2= RULE_STRING
                    {
                    lv_attrValue_2_2=(Token)input.LT(1);
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleAttributes1459); 

                    			createLeafNode(grammarAccess.getAttributesAccess().getAttrValueSTRINGTerminalRuleCall_2_0_1(), "attrValue"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getAttributesRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"attrValue",
                    	        		lv_attrValue_2_2, 
                    	        		"STRING", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:841:8: lv_attrValue_2_3= RULE_PATH
                    {
                    lv_attrValue_2_3=(Token)input.LT(1);
                    match(input,RULE_PATH,FOLLOW_RULE_PATH_in_ruleAttributes1479); 

                    			createLeafNode(grammarAccess.getAttributesAccess().getAttrValuePATHTerminalRuleCall_2_0_2(), "attrValue"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getAttributesRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"attrValue",
                    	        		lv_attrValue_2_3, 
                    	        		"PATH", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:862:8: lv_attrValue_2_4= RULE_ID
                    {
                    lv_attrValue_2_4=(Token)input.LT(1);
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleAttributes1499); 

                    			createLeafNode(grammarAccess.getAttributesAccess().getAttrValueIDTerminalRuleCall_2_0_3(), "attrValue"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getAttributesRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"attrValue",
                    	        		lv_attrValue_2_4, 
                    	        		"ID", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:883:8: lv_attrValue_2_5= RULE_IDLIST
                    {
                    lv_attrValue_2_5=(Token)input.LT(1);
                    match(input,RULE_IDLIST,FOLLOW_RULE_IDLIST_in_ruleAttributes1519); 

                    			createLeafNode(grammarAccess.getAttributesAccess().getAttrValueIDLISTTerminalRuleCall_2_0_4(), "attrValue"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getAttributesRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"attrValue",
                    	        		lv_attrValue_2_5, 
                    	        		"IDLIST", 
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
        }
        return current;
    }
    // $ANTLR end ruleAttributes


    // $ANTLR start entryRuleTSTRING
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:915:1: entryRuleTSTRING returns [String current=null] : iv_ruleTSTRING= ruleTSTRING EOF ;
    public final String entryRuleTSTRING() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleTSTRING = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:916:2: (iv_ruleTSTRING= ruleTSTRING EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:917:2: iv_ruleTSTRING= ruleTSTRING EOF
            {
             currentNode = createCompositeNode(grammarAccess.getTSTRINGRule(), currentNode); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING1564);
            iv_ruleTSTRING=ruleTSTRING();
            _fsp--;

             current =iv_ruleTSTRING.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING1575); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:924:1: ruleTSTRING returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (kw= '_' this_STRING_1= RULE_STRING ) ;
    public final AntlrDatatypeRuleToken ruleTSTRING() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        Token this_STRING_1=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:929:6: ( (kw= '_' this_STRING_1= RULE_STRING ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:930:1: (kw= '_' this_STRING_1= RULE_STRING )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:930:1: (kw= '_' this_STRING_1= RULE_STRING )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:931:2: kw= '_' this_STRING_1= RULE_STRING
            {
            kw=(Token)input.LT(1);
            match(input,19,FOLLOW_19_in_ruleTSTRING1613); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getTSTRINGAccess().get_Keyword_0(), null); 
                
            this_STRING_1=(Token)input.LT(1);
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleTSTRING1628); 

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


    // $ANTLR start entryRuleHOMEPATH
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:951:1: entryRuleHOMEPATH returns [String current=null] : iv_ruleHOMEPATH= ruleHOMEPATH EOF ;
    public final String entryRuleHOMEPATH() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleHOMEPATH = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:952:2: (iv_ruleHOMEPATH= ruleHOMEPATH EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:953:2: iv_ruleHOMEPATH= ruleHOMEPATH EOF
            {
             currentNode = createCompositeNode(grammarAccess.getHOMEPATHRule(), currentNode); 
            pushFollow(FOLLOW_ruleHOMEPATH_in_entryRuleHOMEPATH1674);
            iv_ruleHOMEPATH=ruleHOMEPATH();
            _fsp--;

             current =iv_ruleHOMEPATH.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleHOMEPATH1685); 

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
    // $ANTLR end entryRuleHOMEPATH


    // $ANTLR start ruleHOMEPATH
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:960:1: ruleHOMEPATH returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (kw= '~' this_PATH_1= RULE_PATH ) ;
    public final AntlrDatatypeRuleToken ruleHOMEPATH() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        Token this_PATH_1=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:965:6: ( (kw= '~' this_PATH_1= RULE_PATH ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:966:1: (kw= '~' this_PATH_1= RULE_PATH )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:966:1: (kw= '~' this_PATH_1= RULE_PATH )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:967:2: kw= '~' this_PATH_1= RULE_PATH
            {
            kw=(Token)input.LT(1);
            match(input,20,FOLLOW_20_in_ruleHOMEPATH1723); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getHOMEPATHAccess().getTildeKeyword_0(), null); 
                
            this_PATH_1=(Token)input.LT(1);
            match(input,RULE_PATH,FOLLOW_RULE_PATH_in_ruleHOMEPATH1738); 

            		current.merge(this_PATH_1);
                
             
                createLeafNode(grammarAccess.getHOMEPATHAccess().getPATHTerminalRuleCall_1(), null); 
                

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
    // $ANTLR end ruleHOMEPATH


 

    public static final BitSet FOLLOW_ruleRoot_in_entryRuleRoot75 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleRoot85 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTextDomain_in_ruleRoot131 = new BitSet(new long[]{0x0000000000005802L});
    public static final BitSet FOLLOW_rulePreprocessor_in_ruleRoot153 = new BitSet(new long[]{0x0000000000005002L});
    public static final BitSet FOLLOW_ruleRootType_in_ruleRoot175 = new BitSet(new long[]{0x0000000000004002L});
    public static final BitSet FOLLOW_ruleTextDomain_in_entryRuleTextDomain212 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTextDomain222 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_ruleTextDomain257 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleTextDomain274 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePreprocessor_in_entryRulePreprocessor315 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePreprocessor325 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacro_in_rulePreprocessor372 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePathInclude_in_rulePreprocessor399 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacro_in_entryRuleMacro434 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleMacro444 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_ruleMacro479 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleMacro496 = new BitSet(new long[]{0x0000000000002010L});
    public static final BitSet FOLLOW_13_in_ruleMacro512 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePathInclude_in_entryRulePathInclude548 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePathInclude558 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rulePathInclude593 = new BitSet(new long[]{0x0000000000100020L});
    public static final BitSet FOLLOW_ruleHOMEPATH_in_rulePathInclude616 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_RULE_PATH_in_rulePathInclude631 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_13_in_rulePathInclude649 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootType_in_entryRuleRootType685 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleRootType695 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_ruleRootType730 = new BitSet(new long[]{0x0000000000030010L});
    public static final BitSet FOLLOW_ruleRootTag_in_ruleRootType751 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_15_in_ruleRootType761 = new BitSet(new long[]{0x0000000000005010L});
    public static final BitSet FOLLOW_ruleRootType_in_ruleRootType782 = new BitSet(new long[]{0x0000000000005010L});
    public static final BitSet FOLLOW_ruleAttributes_in_ruleRootType804 = new BitSet(new long[]{0x0000000000005010L});
    public static final BitSet FOLLOW_rulePreprocessor_in_ruleRootType826 = new BitSet(new long[]{0x0000000000005000L});
    public static final BitSet FOLLOW_14_in_ruleRootType837 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_ruleRootType847 = new BitSet(new long[]{0x0000000000030010L});
    public static final BitSet FOLLOW_ruleRootTag_in_ruleRootType868 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_15_in_ruleRootType878 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootTag_in_entryRuleRootTag914 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleRootTag924 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleSimpleTag_in_ruleRootTag971 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleAddedTag_in_ruleRootTag998 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleSimpleTag_in_entryRuleSimpleTag1033 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleSimpleTag1043 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_ruleSimpleTag1086 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_ruleRootTagsList_in_ruleSimpleTag1121 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleAddedTag_in_entryRuleAddedTag1157 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleAddedTag1167 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_ruleAddedTag1202 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_ruleRootTagsList_in_ruleAddedTag1223 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootTagsList_in_entryRuleRootTagsList1260 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleRootTagsList1271 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleRootTagsList1310 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleAttributes_in_entryRuleAttributes1354 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleAttributes1364 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleAttributes1406 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_ruleAttributes1421 = new BitSet(new long[]{0x00000000000800F0L});
    public static final BitSet FOLLOW_ruleTSTRING_in_ruleAttributes1444 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleAttributes1459 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_PATH_in_ruleAttributes1479 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleAttributes1499 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IDLIST_in_ruleAttributes1519 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING1564 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING1575 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_ruleTSTRING1613 = new BitSet(new long[]{0x0000000000000040L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleTSTRING1628 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleHOMEPATH_in_entryRuleHOMEPATH1674 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleHOMEPATH1685 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_ruleHOMEPATH1723 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_PATH_in_ruleHOMEPATH1738 = new BitSet(new long[]{0x0000000000000002L});

}