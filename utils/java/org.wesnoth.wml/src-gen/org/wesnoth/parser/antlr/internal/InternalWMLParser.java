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
        "<invalid>", "<EOR>", "<DOWN>", "<UP>", "RULE_ID", "RULE_PATH", "RULE_IDLIST", "RULE_STRING", "RULE_SL_COMMENT", "RULE_WS", "RULE_ANY_OTHER", "'#textdomain '", "'{'", "'}'", "'['", "']'", "'/'", "'+'", "'='", "'_'", "'~'"
    };
    public static final int RULE_ID=4;
    public static final int RULE_STRING=7;
    public static final int RULE_IDLIST=6;
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:86:1: ruleRoot returns [EObject current=null] : ( ( ( ( ({...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ ) ) | ({...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ ) ) | ({...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ ) ) )* ) ) ) ;
    public final EObject ruleRoot() throws RecognitionException {
        EObject current = null;

        EObject lv_textdomains_1_0 = null;

        EObject lv_preproc_2_0 = null;

        EObject lv_roots_3_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:91:6: ( ( ( ( ( ({...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ ) ) | ({...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ ) ) | ({...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ ) ) )* ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( ( ( ( ({...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ ) ) | ({...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ ) ) | ({...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ ) ) )* ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:92:1: ( ( ( ( ({...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ ) ) | ({...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ ) ) | ({...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ ) ) )* ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:94:1: ( ( ( ({...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ ) ) | ({...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ ) ) | ({...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ ) ) )* ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:94:1: ( ( ( ({...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ ) ) | ({...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ ) ) | ({...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ ) ) )* ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:95:2: ( ( ({...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ ) ) | ({...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ ) ) | ({...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ ) ) )* )
            {
             
            	  getUnorderedGroupHelper().enter(grammarAccess.getRootAccess().getUnorderedGroup());
            	
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:98:2: ( ( ({...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ ) ) | ({...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ ) ) | ({...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ ) ) )* )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:99:3: ( ({...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ ) ) | ({...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ ) ) | ({...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ ) ) )*
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:99:3: ( ({...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ ) ) | ({...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ ) ) | ({...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ ) ) )*
            loop4:
            do {
                int alt4=4;
                int LA4_0 = input.LA(1);

                if ( LA4_0 ==11 && getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 0) ) {
                    alt4=1;
                }
                else if ( LA4_0 ==12 && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)) ) {
                    int LA4_3 = input.LA(2);

                    if ( LA4_3 ==RULE_ID && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) ) {
                        int LA4_6 = input.LA(3);

                        if ( LA4_6 ==13 && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)) ) {
                            int LA4_9 = input.LA(4);

                            if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) {
                                alt4=2;
                            }
                            else if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ) {
                                alt4=3;
                            }


                        }


                    }
                    else if ( LA4_3 ==20 && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) ) {
                        int LA4_7 = input.LA(3);

                        if ( LA4_7 ==RULE_PATH && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)) ) {
                            int LA4_10 = input.LA(4);

                            if ( LA4_10 ==13 && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)) ) {
                                int LA4_11 = input.LA(5);

                                if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) {
                                    alt4=2;
                                }
                                else if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ) {
                                    alt4=3;
                                }


                            }


                        }


                    }
                    else if ( LA4_3 ==RULE_PATH && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)) ) {
                        int LA4_8 = input.LA(3);

                        if ( LA4_8 ==13 && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)) ) {
                            int LA4_11 = input.LA(4);

                            if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) {
                                alt4=2;
                            }
                            else if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ) {
                                alt4=3;
                            }


                        }


                    }


                }
                else if ( LA4_0 ==14 && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) ) ) {
                    alt4=3;
                }
                else if ( LA4_0 ==RULE_ID && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) ) ) {
                    alt4=3;
                }


                switch (alt4) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:101:4: ({...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:101:4: ({...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:102:5: {...}? => ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ )
            	    {
            	    if ( ! getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 0) ) {
            	        throw new FailedPredicateException(input, "ruleRoot", "getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 0)");
            	    }
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:102:99: ( ( (lv_textdomains_1_0= ruleTextDomain ) )+ )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:103:6: ( (lv_textdomains_1_0= ruleTextDomain ) )+
            	    {
            	     
            	    	 				  getUnorderedGroupHelper().select(grammarAccess.getRootAccess().getUnorderedGroup(), 0);
            	    	 				
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:106:6: ( (lv_textdomains_1_0= ruleTextDomain ) )+
            	    int cnt1=0;
            	    loop1:
            	    do {
            	        int alt1=2;
            	        int LA1_0 = input.LA(1);

            	        if ( (LA1_0==11) ) {
            	            int LA1_2 = input.LA(2);

            	            if ( ! getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 0) ) {
            	                alt1=1;
            	            }


            	        }


            	        switch (alt1) {
            	    	case 1 :
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:107:1: (lv_textdomains_1_0= ruleTextDomain )
            	    	    {
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:107:1: (lv_textdomains_1_0= ruleTextDomain )
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:108:3: lv_textdomains_1_0= ruleTextDomain
            	    	    {
            	    	     
            	    	    	        currentNode=createCompositeNode(grammarAccess.getRootAccess().getTextdomainsTextDomainParserRuleCall_0_0(), currentNode); 
            	    	    	    
            	    	    pushFollow(FOLLOW_ruleTextDomain_in_ruleRoot172);
            	    	    lv_textdomains_1_0=ruleTextDomain();
            	    	    _fsp--;


            	    	    	        if (current==null) {
            	    	    	            current = factory.create(grammarAccess.getRootRule().getType().getClassifier());
            	    	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	    	        }
            	    	    	        try {
            	    	    	       		add(
            	    	    	       			current, 
            	    	    	       			"textdomains",
            	    	    	        		lv_textdomains_1_0, 
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
            	    	    if ( cnt1 >= 1 ) break loop1;
            	                EarlyExitException eee =
            	                    new EarlyExitException(1, input);
            	                throw eee;
            	        }
            	        cnt1++;
            	    } while (true);

            	     
            	    	 				  getUnorderedGroupHelper().returnFromSelection(grammarAccess.getRootAccess().getUnorderedGroup());
            	    	 				

            	    }


            	    }


            	    }
            	    break;
            	case 2 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:137:4: ({...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:137:4: ({...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:138:5: {...}? => ( ( (lv_preproc_2_0= rulePreprocessor ) )+ )
            	    {
            	    if ( ! getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) {
            	        throw new FailedPredicateException(input, "ruleRoot", "getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)");
            	    }
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:138:99: ( ( (lv_preproc_2_0= rulePreprocessor ) )+ )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:139:6: ( (lv_preproc_2_0= rulePreprocessor ) )+
            	    {
            	     
            	    	 				  getUnorderedGroupHelper().select(grammarAccess.getRootAccess().getUnorderedGroup(), 1);
            	    	 				
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:142:6: ( (lv_preproc_2_0= rulePreprocessor ) )+
            	    int cnt2=0;
            	    loop2:
            	    do {
            	        int alt2=2;
            	        int LA2_0 = input.LA(1);

            	        if ( LA2_0 ==12 && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||! ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||! ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) ||! ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||! ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) ||! ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||! ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||! ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||! (getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)) ||! ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1) ) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)) ) {
            	            int LA2_3 = input.LA(2);

            	            if ( ! (getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)) ) {
            	                alt2=1;
            	            }


            	        }


            	        switch (alt2) {
            	    	case 1 :
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:143:1: (lv_preproc_2_0= rulePreprocessor )
            	    	    {
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:143:1: (lv_preproc_2_0= rulePreprocessor )
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:144:3: lv_preproc_2_0= rulePreprocessor
            	    	    {
            	    	     
            	    	    	        currentNode=createCompositeNode(grammarAccess.getRootAccess().getPreprocPreprocessorParserRuleCall_1_0(), currentNode); 
            	    	    	    
            	    	    pushFollow(FOLLOW_rulePreprocessor_in_ruleRoot244);
            	    	    lv_preproc_2_0=rulePreprocessor();
            	    	    _fsp--;


            	    	    	        if (current==null) {
            	    	    	            current = factory.create(grammarAccess.getRootRule().getType().getClassifier());
            	    	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	    	        }
            	    	    	        try {
            	    	    	       		add(
            	    	    	       			current, 
            	    	    	       			"preproc",
            	    	    	        		lv_preproc_2_0, 
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
            	    	    if ( cnt2 >= 1 ) break loop2;
            	                EarlyExitException eee =
            	                    new EarlyExitException(2, input);
            	                throw eee;
            	        }
            	        cnt2++;
            	    } while (true);

            	     
            	    	 				  getUnorderedGroupHelper().returnFromSelection(grammarAccess.getRootAccess().getUnorderedGroup());
            	    	 				

            	    }


            	    }


            	    }
            	    break;
            	case 3 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:173:4: ({...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:173:4: ({...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:174:5: {...}? => ( ( (lv_roots_3_0= ruleRootType ) )+ )
            	    {
            	    if ( ! getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) ) {
            	        throw new FailedPredicateException(input, "ruleRoot", "getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2)");
            	    }
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:174:99: ( ( (lv_roots_3_0= ruleRootType ) )+ )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:175:6: ( (lv_roots_3_0= ruleRootType ) )+
            	    {
            	     
            	    	 				  getUnorderedGroupHelper().select(grammarAccess.getRootAccess().getUnorderedGroup(), 2);
            	    	 				
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:178:6: ( (lv_roots_3_0= ruleRootType ) )+
            	    int cnt3=0;
            	    loop3:
            	    do {
            	        int alt3=2;
            	        int LA3_0 = input.LA(1);

            	        if ( LA3_0 ==12 && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)) ) {
            	            int LA3_3 = input.LA(2);

            	            if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ) {
            	                alt3=1;
            	            }


            	        }
            	        else if ( LA3_0 ==14 && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0)) ) {
            	            int LA3_4 = input.LA(2);

            	            if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) ) {
            	                alt3=1;
            	            }


            	        }
            	        else if ( LA3_0 ==RULE_ID && getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) ) {
            	            int LA3_5 = input.LA(2);

            	            if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) ) {
            	                alt3=1;
            	            }


            	        }


            	        switch (alt3) {
            	    	case 1 :
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:179:1: (lv_roots_3_0= ruleRootType )
            	    	    {
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:179:1: (lv_roots_3_0= ruleRootType )
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:180:3: lv_roots_3_0= ruleRootType
            	    	    {
            	    	     
            	    	    	        currentNode=createCompositeNode(grammarAccess.getRootAccess().getRootsRootTypeParserRuleCall_2_0(), currentNode); 
            	    	    	    
            	    	    pushFollow(FOLLOW_ruleRootType_in_ruleRoot316);
            	    	    lv_roots_3_0=ruleRootType();
            	    	    _fsp--;


            	    	    	        if (current==null) {
            	    	    	            current = factory.create(grammarAccess.getRootRule().getType().getClassifier());
            	    	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	    	        }
            	    	    	        try {
            	    	    	       		add(
            	    	    	       			current, 
            	    	    	       			"roots",
            	    	    	        		lv_roots_3_0, 
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
            	    	    if ( cnt3 >= 1 ) break loop3;
            	                EarlyExitException eee =
            	                    new EarlyExitException(3, input);
            	                throw eee;
            	        }
            	        cnt3++;
            	    } while (true);

            	     
            	    	 				  getUnorderedGroupHelper().returnFromSelection(grammarAccess.getRootAccess().getUnorderedGroup());
            	    	 				

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

             
            	  getUnorderedGroupHelper().leave(grammarAccess.getRootAccess().getUnorderedGroup());
            	

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:224:1: entryRuleTextDomain returns [EObject current=null] : iv_ruleTextDomain= ruleTextDomain EOF ;
    public final EObject entryRuleTextDomain() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleTextDomain = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:225:2: (iv_ruleTextDomain= ruleTextDomain EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:226:2: iv_ruleTextDomain= ruleTextDomain EOF
            {
             currentNode = createCompositeNode(grammarAccess.getTextDomainRule(), currentNode); 
            pushFollow(FOLLOW_ruleTextDomain_in_entryRuleTextDomain391);
            iv_ruleTextDomain=ruleTextDomain();
            _fsp--;

             current =iv_ruleTextDomain; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTextDomain401); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:233:1: ruleTextDomain returns [EObject current=null] : ( '#textdomain ' ( (lv_DomainName_1_0= RULE_ID ) ) ) ;
    public final EObject ruleTextDomain() throws RecognitionException {
        EObject current = null;

        Token lv_DomainName_1_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:238:6: ( ( '#textdomain ' ( (lv_DomainName_1_0= RULE_ID ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:239:1: ( '#textdomain ' ( (lv_DomainName_1_0= RULE_ID ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:239:1: ( '#textdomain ' ( (lv_DomainName_1_0= RULE_ID ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:239:3: '#textdomain ' ( (lv_DomainName_1_0= RULE_ID ) )
            {
            match(input,11,FOLLOW_11_in_ruleTextDomain436); 

                    createLeafNode(grammarAccess.getTextDomainAccess().getTextdomainKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:243:1: ( (lv_DomainName_1_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:244:1: (lv_DomainName_1_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:244:1: (lv_DomainName_1_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:245:3: lv_DomainName_1_0= RULE_ID
            {
            lv_DomainName_1_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleTextDomain453); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:275:1: entryRulePreprocessor returns [EObject current=null] : iv_rulePreprocessor= rulePreprocessor EOF ;
    public final EObject entryRulePreprocessor() throws RecognitionException {
        EObject current = null;

        EObject iv_rulePreprocessor = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:276:2: (iv_rulePreprocessor= rulePreprocessor EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:277:2: iv_rulePreprocessor= rulePreprocessor EOF
            {
             currentNode = createCompositeNode(grammarAccess.getPreprocessorRule(), currentNode); 
            pushFollow(FOLLOW_rulePreprocessor_in_entryRulePreprocessor494);
            iv_rulePreprocessor=rulePreprocessor();
            _fsp--;

             current =iv_rulePreprocessor; 
            match(input,EOF,FOLLOW_EOF_in_entryRulePreprocessor504); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:284:1: rulePreprocessor returns [EObject current=null] : (this_Macro_0= ruleMacro | this_PathInclude_1= rulePathInclude ) ;
    public final EObject rulePreprocessor() throws RecognitionException {
        EObject current = null;

        EObject this_Macro_0 = null;

        EObject this_PathInclude_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:289:6: ( (this_Macro_0= ruleMacro | this_PathInclude_1= rulePathInclude ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:290:1: (this_Macro_0= ruleMacro | this_PathInclude_1= rulePathInclude )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:290:1: (this_Macro_0= ruleMacro | this_PathInclude_1= rulePathInclude )
            int alt5=2;
            int LA5_0 = input.LA(1);

            if ( (LA5_0==12) ) {
                int LA5_1 = input.LA(2);

                if ( (LA5_1==RULE_ID) ) {
                    alt5=1;
                }
                else if ( (LA5_1==RULE_PATH||LA5_1==20) ) {
                    alt5=2;
                }
                else {
                    NoViableAltException nvae =
                        new NoViableAltException("290:1: (this_Macro_0= ruleMacro | this_PathInclude_1= rulePathInclude )", 5, 1, input);

                    throw nvae;
                }
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("290:1: (this_Macro_0= ruleMacro | this_PathInclude_1= rulePathInclude )", 5, 0, input);

                throw nvae;
            }
            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:291:5: this_Macro_0= ruleMacro
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getPreprocessorAccess().getMacroParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleMacro_in_rulePreprocessor551);
                    this_Macro_0=ruleMacro();
                    _fsp--;

                     
                            current = this_Macro_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:301:5: this_PathInclude_1= rulePathInclude
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getPreprocessorAccess().getPathIncludeParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_rulePathInclude_in_rulePreprocessor578);
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:317:1: entryRuleMacro returns [EObject current=null] : iv_ruleMacro= ruleMacro EOF ;
    public final EObject entryRuleMacro() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleMacro = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:318:2: (iv_ruleMacro= ruleMacro EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:319:2: iv_ruleMacro= ruleMacro EOF
            {
             currentNode = createCompositeNode(grammarAccess.getMacroRule(), currentNode); 
            pushFollow(FOLLOW_ruleMacro_in_entryRuleMacro613);
            iv_ruleMacro=ruleMacro();
            _fsp--;

             current =iv_ruleMacro; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleMacro623); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:326:1: ruleMacro returns [EObject current=null] : ( '{' ( (lv_macroName_1_0= RULE_ID ) ) '}' ) ;
    public final EObject ruleMacro() throws RecognitionException {
        EObject current = null;

        Token lv_macroName_1_0=null;

         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:331:6: ( ( '{' ( (lv_macroName_1_0= RULE_ID ) ) '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:332:1: ( '{' ( (lv_macroName_1_0= RULE_ID ) ) '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:332:1: ( '{' ( (lv_macroName_1_0= RULE_ID ) ) '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:332:3: '{' ( (lv_macroName_1_0= RULE_ID ) ) '}'
            {
            match(input,12,FOLLOW_12_in_ruleMacro658); 

                    createLeafNode(grammarAccess.getMacroAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:336:1: ( (lv_macroName_1_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:337:1: (lv_macroName_1_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:337:1: (lv_macroName_1_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:338:3: lv_macroName_1_0= RULE_ID
            {
            lv_macroName_1_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleMacro675); 

            			createLeafNode(grammarAccess.getMacroAccess().getMacroNameIDTerminalRuleCall_1_0(), "macroName"); 
            		

            	        if (current==null) {
            	            current = factory.create(grammarAccess.getMacroRule().getType().getClassifier());
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

            match(input,13,FOLLOW_13_in_ruleMacro690); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:372:1: entryRulePathInclude returns [EObject current=null] : iv_rulePathInclude= rulePathInclude EOF ;
    public final EObject entryRulePathInclude() throws RecognitionException {
        EObject current = null;

        EObject iv_rulePathInclude = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:373:2: (iv_rulePathInclude= rulePathInclude EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:374:2: iv_rulePathInclude= rulePathInclude EOF
            {
             currentNode = createCompositeNode(grammarAccess.getPathIncludeRule(), currentNode); 
            pushFollow(FOLLOW_rulePathInclude_in_entryRulePathInclude726);
            iv_rulePathInclude=rulePathInclude();
            _fsp--;

             current =iv_rulePathInclude; 
            match(input,EOF,FOLLOW_EOF_in_entryRulePathInclude736); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:381:1: rulePathInclude returns [EObject current=null] : ( '{' ( ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) ) ) '}' ) ;
    public final EObject rulePathInclude() throws RecognitionException {
        EObject current = null;

        Token lv_path_1_2=null;
        AntlrDatatypeRuleToken lv_path_1_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:386:6: ( ( '{' ( ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) ) ) '}' ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:387:1: ( '{' ( ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) ) ) '}' )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:387:1: ( '{' ( ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) ) ) '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:387:3: '{' ( ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) ) ) '}'
            {
            match(input,12,FOLLOW_12_in_rulePathInclude771); 

                    createLeafNode(grammarAccess.getPathIncludeAccess().getLeftCurlyBracketKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:391:1: ( ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:392:1: ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:392:1: ( (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:393:1: (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:393:1: (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH )
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
                    new NoViableAltException("393:1: (lv_path_1_1= ruleHOMEPATH | lv_path_1_2= RULE_PATH )", 6, 0, input);

                throw nvae;
            }
            switch (alt6) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:394:3: lv_path_1_1= ruleHOMEPATH
                    {
                     
                    	        currentNode=createCompositeNode(grammarAccess.getPathIncludeAccess().getPathHOMEPATHParserRuleCall_1_0_0(), currentNode); 
                    	    
                    pushFollow(FOLLOW_ruleHOMEPATH_in_rulePathInclude794);
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
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:415:8: lv_path_1_2= RULE_PATH
                    {
                    lv_path_1_2=(Token)input.LT(1);
                    match(input,RULE_PATH,FOLLOW_RULE_PATH_in_rulePathInclude809); 

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

            match(input,13,FOLLOW_13_in_rulePathInclude827); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:451:1: entryRuleRootType returns [EObject current=null] : iv_ruleRootType= ruleRootType EOF ;
    public final EObject entryRuleRootType() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleRootType = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:452:2: (iv_ruleRootType= ruleRootType EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:453:2: iv_ruleRootType= ruleRootType EOF
            {
             currentNode = createCompositeNode(grammarAccess.getRootTypeRule(), currentNode); 
            pushFollow(FOLLOW_ruleRootType_in_entryRuleRootType863);
            iv_ruleRootType=ruleRootType();
            _fsp--;

             current =iv_ruleRootType; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleRootType873); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:460:1: ruleRootType returns [EObject current=null] : ( ( ( ( ({...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) ) ) | ({...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ ) ) | ({...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) ) ) )+ {...}?) ) ) ;
    public final EObject ruleRootType() throws RecognitionException {
        EObject current = null;

        EObject lv_startTag_2_0 = null;

        EObject lv_subTypes_4_0 = null;

        EObject lv_at_5_0 = null;

        EObject lv_okpreproc_6_0 = null;

        EObject lv_endTag_9_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:465:6: ( ( ( ( ( ({...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) ) ) | ({...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ ) ) | ({...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) ) ) )+ {...}?) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:466:1: ( ( ( ( ({...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) ) ) | ({...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ ) ) | ({...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) ) ) )+ {...}?) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:466:1: ( ( ( ( ({...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) ) ) | ({...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ ) ) | ({...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) ) ) )+ {...}?) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:468:1: ( ( ( ({...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) ) ) | ({...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ ) ) | ({...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) ) ) )+ {...}?) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:468:1: ( ( ( ({...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) ) ) | ({...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ ) ) | ({...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) ) ) )+ {...}?) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:469:2: ( ( ({...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) ) ) | ({...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ ) ) | ({...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) ) ) )+ {...}?)
            {
             
            	  getUnorderedGroupHelper().enter(grammarAccess.getRootTypeAccess().getUnorderedGroup());
            	
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:472:2: ( ( ({...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) ) ) | ({...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ ) ) | ({...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) ) ) )+ {...}?)
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:473:3: ( ({...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) ) ) | ({...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ ) ) | ({...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) ) ) )+ {...}?
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:473:3: ( ({...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) ) ) | ({...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ ) ) | ({...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) ) ) )+
            int cnt10=0;
            loop10:
            do {
                int alt10=4;
                int LA10_0 = input.LA(1);

                if ( LA10_0 ==12 && (getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)) ) {
                    int LA10_3 = input.LA(2);

                    if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ) {
                        alt10=3;
                    }


                }
                else if ( LA10_0 ==14 && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0)) ) {
                    int LA10_4 = input.LA(2);

                    if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) ) {
                        alt10=1;
                    }
                    else if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ) {
                        alt10=3;
                    }


                }
                else if ( LA10_0 ==RULE_ID && getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) ) {
                    int LA10_5 = input.LA(2);

                    if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) ) {
                        alt10=2;
                    }


                }


                switch (alt10) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:475:4: ({...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:475:4: ({...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:476:5: {...}? => ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) )
            	    {
            	    if ( ! getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) ) {
            	        throw new FailedPredicateException(input, "ruleRootType", "getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0)");
            	    }
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:476:103: ( ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:477:6: ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* )
            	    {
            	     
            	    	 				  getUnorderedGroupHelper().select(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0);
            	    	 				
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:480:6: ( '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )* )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:480:8: '[' ( (lv_startTag_2_0= ruleRootTag ) ) ']' ( (lv_subTypes_4_0= ruleRootType ) )*
            	    {
            	    match(input,14,FOLLOW_14_in_ruleRootType950); 

            	            createLeafNode(grammarAccess.getRootTypeAccess().getLeftSquareBracketKeyword_0_0(), null); 
            	        
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:484:1: ( (lv_startTag_2_0= ruleRootTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:485:1: (lv_startTag_2_0= ruleRootTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:485:1: (lv_startTag_2_0= ruleRootTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:486:3: lv_startTag_2_0= ruleRootTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getRootTypeAccess().getStartTagRootTagParserRuleCall_0_1_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleRootTag_in_ruleRootType971);
            	    lv_startTag_2_0=ruleRootTag();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getRootTypeRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		set(
            	    	       			current, 
            	    	       			"startTag",
            	    	        		lv_startTag_2_0, 
            	    	        		"RootTag", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }

            	    match(input,15,FOLLOW_15_in_ruleRootType981); 

            	            createLeafNode(grammarAccess.getRootTypeAccess().getRightSquareBracketKeyword_0_2(), null); 
            	        
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:512:1: ( (lv_subTypes_4_0= ruleRootType ) )*
            	    loop7:
            	    do {
            	        int alt7=2;
            	        int LA7_0 = input.LA(1);

            	        if ( LA7_0 ==12 && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 1)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)) ) {
            	            int LA7_3 = input.LA(2);

            	            if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ) {
            	                alt7=1;
            	            }


            	        }
            	        else if ( LA7_0 ==14 && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0)|| getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) || getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0)) ) {
            	            int LA7_4 = input.LA(2);

            	            if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 0) ) {
            	                alt7=1;
            	            }


            	        }
            	        else if ( LA7_0 ==RULE_ID && getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) ) {
            	            int LA7_5 = input.LA(2);

            	            if ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) ) {
            	                alt7=1;
            	            }


            	        }


            	        switch (alt7) {
            	    	case 1 :
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:513:1: (lv_subTypes_4_0= ruleRootType )
            	    	    {
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:513:1: (lv_subTypes_4_0= ruleRootType )
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:514:3: lv_subTypes_4_0= ruleRootType
            	    	    {
            	    	     
            	    	    	        currentNode=createCompositeNode(grammarAccess.getRootTypeAccess().getSubTypesRootTypeParserRuleCall_0_3_0(), currentNode); 
            	    	    	    
            	    	    pushFollow(FOLLOW_ruleRootType_in_ruleRootType1002);
            	    	    lv_subTypes_4_0=ruleRootType();
            	    	    _fsp--;


            	    	    	        if (current==null) {
            	    	    	            current = factory.create(grammarAccess.getRootTypeRule().getType().getClassifier());
            	    	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	    	        }
            	    	    	        try {
            	    	    	       		add(
            	    	    	       			current, 
            	    	    	       			"subTypes",
            	    	    	        		lv_subTypes_4_0, 
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


            	    }

            	     
            	    	 				  getUnorderedGroupHelper().returnFromSelection(grammarAccess.getRootTypeAccess().getUnorderedGroup());
            	    	 				

            	    }


            	    }


            	    }
            	    break;
            	case 2 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:543:4: ({...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:543:4: ({...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:544:5: {...}? => ( ( (lv_at_5_0= ruleAttributes ) )+ )
            	    {
            	    if ( ! getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) ) {
            	        throw new FailedPredicateException(input, "ruleRootType", "getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1)");
            	    }
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:544:103: ( ( (lv_at_5_0= ruleAttributes ) )+ )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:545:6: ( (lv_at_5_0= ruleAttributes ) )+
            	    {
            	     
            	    	 				  getUnorderedGroupHelper().select(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1);
            	    	 				
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:548:6: ( (lv_at_5_0= ruleAttributes ) )+
            	    int cnt8=0;
            	    loop8:
            	    do {
            	        int alt8=2;
            	        int LA8_0 = input.LA(1);

            	        if ( LA8_0 ==RULE_ID && ( getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) ||! getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1)) ) {
            	            int LA8_5 = input.LA(2);

            	            if ( ! ( getUnorderedGroupHelper().canLeave(grammarAccess.getRootTypeAccess().getUnorderedGroup()) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootAccess().getUnorderedGroup(), 2) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) || getUnorderedGroupHelper().canLeave(grammarAccess.getRootTypeAccess().getUnorderedGroup()) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) || getUnorderedGroupHelper().canLeave(grammarAccess.getRootTypeAccess().getUnorderedGroup()) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1) ||getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1)|| getUnorderedGroupHelper().canLeave(grammarAccess.getRootTypeAccess().getUnorderedGroup()) &&getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 1)) ) {
            	                alt8=1;
            	            }


            	        }


            	        switch (alt8) {
            	    	case 1 :
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:549:1: (lv_at_5_0= ruleAttributes )
            	    	    {
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:549:1: (lv_at_5_0= ruleAttributes )
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:550:3: lv_at_5_0= ruleAttributes
            	    	    {
            	    	     
            	    	    	        currentNode=createCompositeNode(grammarAccess.getRootTypeAccess().getAtAttributesParserRuleCall_1_0(), currentNode); 
            	    	    	    
            	    	    pushFollow(FOLLOW_ruleAttributes_in_ruleRootType1075);
            	    	    lv_at_5_0=ruleAttributes();
            	    	    _fsp--;


            	    	    	        if (current==null) {
            	    	    	            current = factory.create(grammarAccess.getRootTypeRule().getType().getClassifier());
            	    	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	    	        }
            	    	    	        try {
            	    	    	       		add(
            	    	    	       			current, 
            	    	    	       			"at",
            	    	    	        		lv_at_5_0, 
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
            	    	    if ( cnt8 >= 1 ) break loop8;
            	                EarlyExitException eee =
            	                    new EarlyExitException(8, input);
            	                throw eee;
            	        }
            	        cnt8++;
            	    } while (true);

            	     
            	    	 				  getUnorderedGroupHelper().returnFromSelection(grammarAccess.getRootTypeAccess().getUnorderedGroup());
            	    	 				

            	    }


            	    }


            	    }
            	    break;
            	case 3 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:579:4: ({...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) ) )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:579:4: ({...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:580:5: {...}? => ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) )
            	    {
            	    if ( ! getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2) ) {
            	        throw new FailedPredicateException(input, "ruleRootType", "getUnorderedGroupHelper().canSelect(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2)");
            	    }
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:580:103: ( ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:581:6: ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' )
            	    {
            	     
            	    	 				  getUnorderedGroupHelper().select(grammarAccess.getRootTypeAccess().getUnorderedGroup(), 2);
            	    	 				
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:584:6: ( ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']' )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:584:7: ( (lv_okpreproc_6_0= rulePreprocessor ) )* '[' '/' ( (lv_endTag_9_0= ruleRootTag ) ) ']'
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:584:7: ( (lv_okpreproc_6_0= rulePreprocessor ) )*
            	    loop9:
            	    do {
            	        int alt9=2;
            	        int LA9_0 = input.LA(1);

            	        if ( (LA9_0==12) ) {
            	            alt9=1;
            	        }


            	        switch (alt9) {
            	    	case 1 :
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:585:1: (lv_okpreproc_6_0= rulePreprocessor )
            	    	    {
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:585:1: (lv_okpreproc_6_0= rulePreprocessor )
            	    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:586:3: lv_okpreproc_6_0= rulePreprocessor
            	    	    {
            	    	     
            	    	    	        currentNode=createCompositeNode(grammarAccess.getRootTypeAccess().getOkpreprocPreprocessorParserRuleCall_2_0_0(), currentNode); 
            	    	    	    
            	    	    pushFollow(FOLLOW_rulePreprocessor_in_ruleRootType1148);
            	    	    lv_okpreproc_6_0=rulePreprocessor();
            	    	    _fsp--;


            	    	    	        if (current==null) {
            	    	    	            current = factory.create(grammarAccess.getRootTypeRule().getType().getClassifier());
            	    	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	    	        }
            	    	    	        try {
            	    	    	       		add(
            	    	    	       			current, 
            	    	    	       			"okpreproc",
            	    	    	        		lv_okpreproc_6_0, 
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

            	    match(input,14,FOLLOW_14_in_ruleRootType1159); 

            	            createLeafNode(grammarAccess.getRootTypeAccess().getLeftSquareBracketKeyword_2_1(), null); 
            	        
            	    match(input,16,FOLLOW_16_in_ruleRootType1169); 

            	            createLeafNode(grammarAccess.getRootTypeAccess().getSolidusKeyword_2_2(), null); 
            	        
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:616:1: ( (lv_endTag_9_0= ruleRootTag ) )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:617:1: (lv_endTag_9_0= ruleRootTag )
            	    {
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:617:1: (lv_endTag_9_0= ruleRootTag )
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:618:3: lv_endTag_9_0= ruleRootTag
            	    {
            	     
            	    	        currentNode=createCompositeNode(grammarAccess.getRootTypeAccess().getEndTagRootTagParserRuleCall_2_3_0(), currentNode); 
            	    	    
            	    pushFollow(FOLLOW_ruleRootTag_in_ruleRootType1190);
            	    lv_endTag_9_0=ruleRootTag();
            	    _fsp--;


            	    	        if (current==null) {
            	    	            current = factory.create(grammarAccess.getRootTypeRule().getType().getClassifier());
            	    	            associateNodeWithAstElement(currentNode.getParent(), current);
            	    	        }
            	    	        try {
            	    	       		set(
            	    	       			current, 
            	    	       			"endTag",
            	    	        		lv_endTag_9_0, 
            	    	        		"RootTag", 
            	    	        		currentNode);
            	    	        } catch (ValueConverterException vce) {
            	    				handleValueConverterException(vce);
            	    	        }
            	    	        currentNode = currentNode.getParent();
            	    	    

            	    }


            	    }

            	    match(input,15,FOLLOW_15_in_ruleRootType1200); 

            	            createLeafNode(grammarAccess.getRootTypeAccess().getRightSquareBracketKeyword_2_4(), null); 
            	        

            	    }

            	     
            	    	 				  getUnorderedGroupHelper().returnFromSelection(grammarAccess.getRootTypeAccess().getUnorderedGroup());
            	    	 				

            	    }


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

            if ( ! getUnorderedGroupHelper().canLeave(grammarAccess.getRootTypeAccess().getUnorderedGroup()) ) {
                throw new FailedPredicateException(input, "ruleRootType", "getUnorderedGroupHelper().canLeave(grammarAccess.getRootTypeAccess().getUnorderedGroup())");
            }

            }


            }

             
            	  getUnorderedGroupHelper().leave(grammarAccess.getRootTypeAccess().getUnorderedGroup());
            	

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:667:1: entryRuleRootTag returns [EObject current=null] : iv_ruleRootTag= ruleRootTag EOF ;
    public final EObject entryRuleRootTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleRootTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:668:2: (iv_ruleRootTag= ruleRootTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:669:2: iv_ruleRootTag= ruleRootTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getRootTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleRootTag_in_entryRuleRootTag1281);
            iv_ruleRootTag=ruleRootTag();
            _fsp--;

             current =iv_ruleRootTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleRootTag1291); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:676:1: ruleRootTag returns [EObject current=null] : (this_SimpleTag_0= ruleSimpleTag | this_AddedTag_1= ruleAddedTag ) ;
    public final EObject ruleRootTag() throws RecognitionException {
        EObject current = null;

        EObject this_SimpleTag_0 = null;

        EObject this_AddedTag_1 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:681:6: ( (this_SimpleTag_0= ruleSimpleTag | this_AddedTag_1= ruleAddedTag ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:682:1: (this_SimpleTag_0= ruleSimpleTag | this_AddedTag_1= ruleAddedTag )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:682:1: (this_SimpleTag_0= ruleSimpleTag | this_AddedTag_1= ruleAddedTag )
            int alt11=2;
            int LA11_0 = input.LA(1);

            if ( (LA11_0==RULE_ID||LA11_0==16) ) {
                alt11=1;
            }
            else if ( (LA11_0==17) ) {
                alt11=2;
            }
            else {
                NoViableAltException nvae =
                    new NoViableAltException("682:1: (this_SimpleTag_0= ruleSimpleTag | this_AddedTag_1= ruleAddedTag )", 11, 0, input);

                throw nvae;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:683:5: this_SimpleTag_0= ruleSimpleTag
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getRootTagAccess().getSimpleTagParserRuleCall_0(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleSimpleTag_in_ruleRootTag1338);
                    this_SimpleTag_0=ruleSimpleTag();
                    _fsp--;

                     
                            current = this_SimpleTag_0; 
                            currentNode = currentNode.getParent();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:693:5: this_AddedTag_1= ruleAddedTag
                    {
                     
                            currentNode=createCompositeNode(grammarAccess.getRootTagAccess().getAddedTagParserRuleCall_1(), currentNode); 
                        
                    pushFollow(FOLLOW_ruleAddedTag_in_ruleRootTag1365);
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:709:1: entryRuleSimpleTag returns [EObject current=null] : iv_ruleSimpleTag= ruleSimpleTag EOF ;
    public final EObject entryRuleSimpleTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleSimpleTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:710:2: (iv_ruleSimpleTag= ruleSimpleTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:711:2: iv_ruleSimpleTag= ruleSimpleTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getSimpleTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleSimpleTag_in_entryRuleSimpleTag1400);
            iv_ruleSimpleTag=ruleSimpleTag();
            _fsp--;

             current =iv_ruleSimpleTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleSimpleTag1410); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:718:1: ruleSimpleTag returns [EObject current=null] : ( ( (lv_endTag_0_0= '/' ) )? ( (lv_tagName_1_0= ruleRootTagsList ) ) ) ;
    public final EObject ruleSimpleTag() throws RecognitionException {
        EObject current = null;

        Token lv_endTag_0_0=null;
        AntlrDatatypeRuleToken lv_tagName_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:723:6: ( ( ( (lv_endTag_0_0= '/' ) )? ( (lv_tagName_1_0= ruleRootTagsList ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:724:1: ( ( (lv_endTag_0_0= '/' ) )? ( (lv_tagName_1_0= ruleRootTagsList ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:724:1: ( ( (lv_endTag_0_0= '/' ) )? ( (lv_tagName_1_0= ruleRootTagsList ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:724:2: ( (lv_endTag_0_0= '/' ) )? ( (lv_tagName_1_0= ruleRootTagsList ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:724:2: ( (lv_endTag_0_0= '/' ) )?
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0==16) ) {
                alt12=1;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:725:1: (lv_endTag_0_0= '/' )
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:725:1: (lv_endTag_0_0= '/' )
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:726:3: lv_endTag_0_0= '/'
                    {
                    lv_endTag_0_0=(Token)input.LT(1);
                    match(input,16,FOLLOW_16_in_ruleSimpleTag1453); 

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

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:745:3: ( (lv_tagName_1_0= ruleRootTagsList ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:746:1: (lv_tagName_1_0= ruleRootTagsList )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:746:1: (lv_tagName_1_0= ruleRootTagsList )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:747:3: lv_tagName_1_0= ruleRootTagsList
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getSimpleTagAccess().getTagNameRootTagsListParserRuleCall_1_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleRootTagsList_in_ruleSimpleTag1488);
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:777:1: entryRuleAddedTag returns [EObject current=null] : iv_ruleAddedTag= ruleAddedTag EOF ;
    public final EObject entryRuleAddedTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleAddedTag = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:778:2: (iv_ruleAddedTag= ruleAddedTag EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:779:2: iv_ruleAddedTag= ruleAddedTag EOF
            {
             currentNode = createCompositeNode(grammarAccess.getAddedTagRule(), currentNode); 
            pushFollow(FOLLOW_ruleAddedTag_in_entryRuleAddedTag1524);
            iv_ruleAddedTag=ruleAddedTag();
            _fsp--;

             current =iv_ruleAddedTag; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleAddedTag1534); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:786:1: ruleAddedTag returns [EObject current=null] : ( '+' ( (lv_tagName_1_0= ruleRootTagsList ) ) ) ;
    public final EObject ruleAddedTag() throws RecognitionException {
        EObject current = null;

        AntlrDatatypeRuleToken lv_tagName_1_0 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:791:6: ( ( '+' ( (lv_tagName_1_0= ruleRootTagsList ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:792:1: ( '+' ( (lv_tagName_1_0= ruleRootTagsList ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:792:1: ( '+' ( (lv_tagName_1_0= ruleRootTagsList ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:792:3: '+' ( (lv_tagName_1_0= ruleRootTagsList ) )
            {
            match(input,17,FOLLOW_17_in_ruleAddedTag1569); 

                    createLeafNode(grammarAccess.getAddedTagAccess().getPlusSignKeyword_0(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:796:1: ( (lv_tagName_1_0= ruleRootTagsList ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:797:1: (lv_tagName_1_0= ruleRootTagsList )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:797:1: (lv_tagName_1_0= ruleRootTagsList )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:798:3: lv_tagName_1_0= ruleRootTagsList
            {
             
            	        currentNode=createCompositeNode(grammarAccess.getAddedTagAccess().getTagNameRootTagsListParserRuleCall_1_0(), currentNode); 
            	    
            pushFollow(FOLLOW_ruleRootTagsList_in_ruleAddedTag1590);
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:828:1: entryRuleRootTagsList returns [String current=null] : iv_ruleRootTagsList= ruleRootTagsList EOF ;
    public final String entryRuleRootTagsList() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleRootTagsList = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:829:2: (iv_ruleRootTagsList= ruleRootTagsList EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:830:2: iv_ruleRootTagsList= ruleRootTagsList EOF
            {
             currentNode = createCompositeNode(grammarAccess.getRootTagsListRule(), currentNode); 
            pushFollow(FOLLOW_ruleRootTagsList_in_entryRuleRootTagsList1627);
            iv_ruleRootTagsList=ruleRootTagsList();
            _fsp--;

             current =iv_ruleRootTagsList.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleRootTagsList1638); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:837:1: ruleRootTagsList returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : this_ID_0= RULE_ID ;
    public final AntlrDatatypeRuleToken ruleRootTagsList() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token this_ID_0=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:842:6: (this_ID_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:843:5: this_ID_0= RULE_ID
            {
            this_ID_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleRootTagsList1677); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:858:1: entryRuleAttributes returns [EObject current=null] : iv_ruleAttributes= ruleAttributes EOF ;
    public final EObject entryRuleAttributes() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleAttributes = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:859:2: (iv_ruleAttributes= ruleAttributes EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:860:2: iv_ruleAttributes= ruleAttributes EOF
            {
             currentNode = createCompositeNode(grammarAccess.getAttributesRule(), currentNode); 
            pushFollow(FOLLOW_ruleAttributes_in_entryRuleAttributes1721);
            iv_ruleAttributes=ruleAttributes();
            _fsp--;

             current =iv_ruleAttributes; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleAttributes1731); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:867:1: ruleAttributes returns [EObject current=null] : ( ( (lv_attrName_0_0= RULE_ID ) ) '=' ( ( (lv_attrValue_2_1= RULE_ID | lv_attrValue_2_2= RULE_IDLIST | lv_attrValue_2_3= ruleTSTRING | lv_attrValue_2_4= RULE_STRING | lv_attrValue_2_5= RULE_PATH ) ) ) ) ;
    public final EObject ruleAttributes() throws RecognitionException {
        EObject current = null;

        Token lv_attrName_0_0=null;
        Token lv_attrValue_2_1=null;
        Token lv_attrValue_2_2=null;
        Token lv_attrValue_2_4=null;
        Token lv_attrValue_2_5=null;
        AntlrDatatypeRuleToken lv_attrValue_2_3 = null;


         EObject temp=null; setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:872:6: ( ( ( (lv_attrName_0_0= RULE_ID ) ) '=' ( ( (lv_attrValue_2_1= RULE_ID | lv_attrValue_2_2= RULE_IDLIST | lv_attrValue_2_3= ruleTSTRING | lv_attrValue_2_4= RULE_STRING | lv_attrValue_2_5= RULE_PATH ) ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:873:1: ( ( (lv_attrName_0_0= RULE_ID ) ) '=' ( ( (lv_attrValue_2_1= RULE_ID | lv_attrValue_2_2= RULE_IDLIST | lv_attrValue_2_3= ruleTSTRING | lv_attrValue_2_4= RULE_STRING | lv_attrValue_2_5= RULE_PATH ) ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:873:1: ( ( (lv_attrName_0_0= RULE_ID ) ) '=' ( ( (lv_attrValue_2_1= RULE_ID | lv_attrValue_2_2= RULE_IDLIST | lv_attrValue_2_3= ruleTSTRING | lv_attrValue_2_4= RULE_STRING | lv_attrValue_2_5= RULE_PATH ) ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:873:2: ( (lv_attrName_0_0= RULE_ID ) ) '=' ( ( (lv_attrValue_2_1= RULE_ID | lv_attrValue_2_2= RULE_IDLIST | lv_attrValue_2_3= ruleTSTRING | lv_attrValue_2_4= RULE_STRING | lv_attrValue_2_5= RULE_PATH ) ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:873:2: ( (lv_attrName_0_0= RULE_ID ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:874:1: (lv_attrName_0_0= RULE_ID )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:874:1: (lv_attrName_0_0= RULE_ID )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:875:3: lv_attrName_0_0= RULE_ID
            {
            lv_attrName_0_0=(Token)input.LT(1);
            match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleAttributes1773); 

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

            match(input,18,FOLLOW_18_in_ruleAttributes1788); 

                    createLeafNode(grammarAccess.getAttributesAccess().getEqualsSignKeyword_1(), null); 
                
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:901:1: ( ( (lv_attrValue_2_1= RULE_ID | lv_attrValue_2_2= RULE_IDLIST | lv_attrValue_2_3= ruleTSTRING | lv_attrValue_2_4= RULE_STRING | lv_attrValue_2_5= RULE_PATH ) ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:902:1: ( (lv_attrValue_2_1= RULE_ID | lv_attrValue_2_2= RULE_IDLIST | lv_attrValue_2_3= ruleTSTRING | lv_attrValue_2_4= RULE_STRING | lv_attrValue_2_5= RULE_PATH ) )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:902:1: ( (lv_attrValue_2_1= RULE_ID | lv_attrValue_2_2= RULE_IDLIST | lv_attrValue_2_3= ruleTSTRING | lv_attrValue_2_4= RULE_STRING | lv_attrValue_2_5= RULE_PATH ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:903:1: (lv_attrValue_2_1= RULE_ID | lv_attrValue_2_2= RULE_IDLIST | lv_attrValue_2_3= ruleTSTRING | lv_attrValue_2_4= RULE_STRING | lv_attrValue_2_5= RULE_PATH )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:903:1: (lv_attrValue_2_1= RULE_ID | lv_attrValue_2_2= RULE_IDLIST | lv_attrValue_2_3= ruleTSTRING | lv_attrValue_2_4= RULE_STRING | lv_attrValue_2_5= RULE_PATH )
            int alt13=5;
            switch ( input.LA(1) ) {
            case RULE_ID:
                {
                alt13=1;
                }
                break;
            case RULE_IDLIST:
                {
                alt13=2;
                }
                break;
            case 19:
                {
                alt13=3;
                }
                break;
            case RULE_STRING:
                {
                alt13=4;
                }
                break;
            case RULE_PATH:
                {
                alt13=5;
                }
                break;
            default:
                NoViableAltException nvae =
                    new NoViableAltException("903:1: (lv_attrValue_2_1= RULE_ID | lv_attrValue_2_2= RULE_IDLIST | lv_attrValue_2_3= ruleTSTRING | lv_attrValue_2_4= RULE_STRING | lv_attrValue_2_5= RULE_PATH )", 13, 0, input);

                throw nvae;
            }

            switch (alt13) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:904:3: lv_attrValue_2_1= RULE_ID
                    {
                    lv_attrValue_2_1=(Token)input.LT(1);
                    match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleAttributes1807); 

                    			createLeafNode(grammarAccess.getAttributesAccess().getAttrValueIDTerminalRuleCall_2_0_0(), "attrValue"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getAttributesRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"attrValue",
                    	        		lv_attrValue_2_1, 
                    	        		"ID", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 2 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:925:8: lv_attrValue_2_2= RULE_IDLIST
                    {
                    lv_attrValue_2_2=(Token)input.LT(1);
                    match(input,RULE_IDLIST,FOLLOW_RULE_IDLIST_in_ruleAttributes1827); 

                    			createLeafNode(grammarAccess.getAttributesAccess().getAttrValueIDLISTTerminalRuleCall_2_0_1(), "attrValue"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getAttributesRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"attrValue",
                    	        		lv_attrValue_2_2, 
                    	        		"IDLIST", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 3 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:946:8: lv_attrValue_2_3= ruleTSTRING
                    {
                     
                    	        currentNode=createCompositeNode(grammarAccess.getAttributesAccess().getAttrValueTSTRINGParserRuleCall_2_0_2(), currentNode); 
                    	    
                    pushFollow(FOLLOW_ruleTSTRING_in_ruleAttributes1851);
                    lv_attrValue_2_3=ruleTSTRING();
                    _fsp--;


                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getAttributesRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode.getParent(), current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"attrValue",
                    	        		lv_attrValue_2_3, 
                    	        		"TSTRING", 
                    	        		currentNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	        currentNode = currentNode.getParent();
                    	    

                    }
                    break;
                case 4 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:967:8: lv_attrValue_2_4= RULE_STRING
                    {
                    lv_attrValue_2_4=(Token)input.LT(1);
                    match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleAttributes1866); 

                    			createLeafNode(grammarAccess.getAttributesAccess().getAttrValueSTRINGTerminalRuleCall_2_0_3(), "attrValue"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getAttributesRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"attrValue",
                    	        		lv_attrValue_2_4, 
                    	        		"STRING", 
                    	        		lastConsumedNode);
                    	        } catch (ValueConverterException vce) {
                    				handleValueConverterException(vce);
                    	        }
                    	    

                    }
                    break;
                case 5 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:988:8: lv_attrValue_2_5= RULE_PATH
                    {
                    lv_attrValue_2_5=(Token)input.LT(1);
                    match(input,RULE_PATH,FOLLOW_RULE_PATH_in_ruleAttributes1886); 

                    			createLeafNode(grammarAccess.getAttributesAccess().getAttrValuePATHTerminalRuleCall_2_0_4(), "attrValue"); 
                    		

                    	        if (current==null) {
                    	            current = factory.create(grammarAccess.getAttributesRule().getType().getClassifier());
                    	            associateNodeWithAstElement(currentNode, current);
                    	        }
                    	        try {
                    	       		set(
                    	       			current, 
                    	       			"attrValue",
                    	        		lv_attrValue_2_5, 
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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1020:1: entryRuleTSTRING returns [String current=null] : iv_ruleTSTRING= ruleTSTRING EOF ;
    public final String entryRuleTSTRING() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleTSTRING = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1021:2: (iv_ruleTSTRING= ruleTSTRING EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1022:2: iv_ruleTSTRING= ruleTSTRING EOF
            {
             currentNode = createCompositeNode(grammarAccess.getTSTRINGRule(), currentNode); 
            pushFollow(FOLLOW_ruleTSTRING_in_entryRuleTSTRING1931);
            iv_ruleTSTRING=ruleTSTRING();
            _fsp--;

             current =iv_ruleTSTRING.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleTSTRING1942); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1029:1: ruleTSTRING returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (kw= '_' this_STRING_1= RULE_STRING ) ;
    public final AntlrDatatypeRuleToken ruleTSTRING() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        Token this_STRING_1=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1034:6: ( (kw= '_' this_STRING_1= RULE_STRING ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1035:1: (kw= '_' this_STRING_1= RULE_STRING )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1035:1: (kw= '_' this_STRING_1= RULE_STRING )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1036:2: kw= '_' this_STRING_1= RULE_STRING
            {
            kw=(Token)input.LT(1);
            match(input,19,FOLLOW_19_in_ruleTSTRING1980); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getTSTRINGAccess().get_Keyword_0(), null); 
                
            this_STRING_1=(Token)input.LT(1);
            match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleTSTRING1995); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1056:1: entryRuleHOMEPATH returns [String current=null] : iv_ruleHOMEPATH= ruleHOMEPATH EOF ;
    public final String entryRuleHOMEPATH() throws RecognitionException {
        String current = null;

        AntlrDatatypeRuleToken iv_ruleHOMEPATH = null;


        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1057:2: (iv_ruleHOMEPATH= ruleHOMEPATH EOF )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1058:2: iv_ruleHOMEPATH= ruleHOMEPATH EOF
            {
             currentNode = createCompositeNode(grammarAccess.getHOMEPATHRule(), currentNode); 
            pushFollow(FOLLOW_ruleHOMEPATH_in_entryRuleHOMEPATH2041);
            iv_ruleHOMEPATH=ruleHOMEPATH();
            _fsp--;

             current =iv_ruleHOMEPATH.getText(); 
            match(input,EOF,FOLLOW_EOF_in_entryRuleHOMEPATH2052); 

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
    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1065:1: ruleHOMEPATH returns [AntlrDatatypeRuleToken current=new AntlrDatatypeRuleToken()] : (kw= '~' this_PATH_1= RULE_PATH ) ;
    public final AntlrDatatypeRuleToken ruleHOMEPATH() throws RecognitionException {
        AntlrDatatypeRuleToken current = new AntlrDatatypeRuleToken();

        Token kw=null;
        Token this_PATH_1=null;

         setCurrentLookahead(); resetLookahead(); 
            
        try {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1070:6: ( (kw= '~' this_PATH_1= RULE_PATH ) )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1071:1: (kw= '~' this_PATH_1= RULE_PATH )
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1071:1: (kw= '~' this_PATH_1= RULE_PATH )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1072:2: kw= '~' this_PATH_1= RULE_PATH
            {
            kw=(Token)input.LT(1);
            match(input,20,FOLLOW_20_in_ruleHOMEPATH2090); 

                    current.merge(kw);
                    createLeafNode(grammarAccess.getHOMEPATHAccess().getTildeKeyword_0(), null); 
                
            this_PATH_1=(Token)input.LT(1);
            match(input,RULE_PATH,FOLLOW_RULE_PATH_in_ruleHOMEPATH2105); 

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
    public static final BitSet FOLLOW_ruleTextDomain_in_ruleRoot172 = new BitSet(new long[]{0x0000000000005812L});
    public static final BitSet FOLLOW_rulePreprocessor_in_ruleRoot244 = new BitSet(new long[]{0x0000000000005812L});
    public static final BitSet FOLLOW_ruleRootType_in_ruleRoot316 = new BitSet(new long[]{0x0000000000005812L});
    public static final BitSet FOLLOW_ruleTextDomain_in_entryRuleTextDomain391 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTextDomain401 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_11_in_ruleTextDomain436 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleTextDomain453 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePreprocessor_in_entryRulePreprocessor494 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePreprocessor504 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacro_in_rulePreprocessor551 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePathInclude_in_rulePreprocessor578 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacro_in_entryRuleMacro613 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleMacro623 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_ruleMacro658 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleMacro675 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_13_in_ruleMacro690 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_rulePathInclude_in_entryRulePathInclude726 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRulePathInclude736 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_12_in_rulePathInclude771 = new BitSet(new long[]{0x0000000000100020L});
    public static final BitSet FOLLOW_ruleHOMEPATH_in_rulePathInclude794 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_RULE_PATH_in_rulePathInclude809 = new BitSet(new long[]{0x0000000000002000L});
    public static final BitSet FOLLOW_13_in_rulePathInclude827 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootType_in_entryRuleRootType863 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleRootType873 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_14_in_ruleRootType950 = new BitSet(new long[]{0x0000000000030010L});
    public static final BitSet FOLLOW_ruleRootTag_in_ruleRootType971 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_15_in_ruleRootType981 = new BitSet(new long[]{0x0000000000005012L});
    public static final BitSet FOLLOW_ruleRootType_in_ruleRootType1002 = new BitSet(new long[]{0x0000000000005012L});
    public static final BitSet FOLLOW_ruleAttributes_in_ruleRootType1075 = new BitSet(new long[]{0x0000000000005012L});
    public static final BitSet FOLLOW_rulePreprocessor_in_ruleRootType1148 = new BitSet(new long[]{0x0000000000005000L});
    public static final BitSet FOLLOW_14_in_ruleRootType1159 = new BitSet(new long[]{0x0000000000010000L});
    public static final BitSet FOLLOW_16_in_ruleRootType1169 = new BitSet(new long[]{0x0000000000030010L});
    public static final BitSet FOLLOW_ruleRootTag_in_ruleRootType1190 = new BitSet(new long[]{0x0000000000008000L});
    public static final BitSet FOLLOW_15_in_ruleRootType1200 = new BitSet(new long[]{0x0000000000005012L});
    public static final BitSet FOLLOW_ruleRootTag_in_entryRuleRootTag1281 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleRootTag1291 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleSimpleTag_in_ruleRootTag1338 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleAddedTag_in_ruleRootTag1365 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleSimpleTag_in_entryRuleSimpleTag1400 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleSimpleTag1410 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_16_in_ruleSimpleTag1453 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_ruleRootTagsList_in_ruleSimpleTag1488 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleAddedTag_in_entryRuleAddedTag1524 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleAddedTag1534 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_17_in_ruleAddedTag1569 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_ruleRootTagsList_in_ruleAddedTag1590 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleRootTagsList_in_entryRuleRootTagsList1627 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleRootTagsList1638 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleRootTagsList1677 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleAttributes_in_entryRuleAttributes1721 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleAttributes1731 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleAttributes1773 = new BitSet(new long[]{0x0000000000040000L});
    public static final BitSet FOLLOW_18_in_ruleAttributes1788 = new BitSet(new long[]{0x00000000000800F0L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleAttributes1807 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IDLIST_in_ruleAttributes1827 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_ruleAttributes1851 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleAttributes1866 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_PATH_in_ruleAttributes1886 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleTSTRING_in_entryRuleTSTRING1931 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleTSTRING1942 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_19_in_ruleTSTRING1980 = new BitSet(new long[]{0x0000000000000080L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleTSTRING1995 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleHOMEPATH_in_entryRuleHOMEPATH2041 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleHOMEPATH2052 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_ruleHOMEPATH2090 = new BitSet(new long[]{0x0000000000000020L});
    public static final BitSet FOLLOW_RULE_PATH_in_ruleHOMEPATH2105 = new BitSet(new long[]{0x0000000000000002L});

}
